/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#include <abaclade/coroutine.hxx>
#include <abaclade/io/text.hxx>
#include <abaclade/math.hxx>
#include <abaclade/process.hxx>
#include <abaclade/text.hxx>
#include <abaclade/text/char_ptr_to_str_adapter.hxx>
#include <abaclade/thread.hxx>
#include "thread-impl.hxx"

#include <cstdlib> // std::abort()
#if ABC_HOST_API_POSIX
   #include <errno.h> // E* errno
   #if ABC_HOST_API_MACH
      #include <mach/mach.h>
   #else
      #include <ucontext.h>
   #endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

app_execution_interruption::app_execution_interruption() {
}

app_execution_interruption::app_execution_interruption(app_execution_interruption const & x) :
   execution_interruption(x) {
}

/*virtual*/ app_execution_interruption::~app_execution_interruption() {
}

app_execution_interruption & app_execution_interruption::operator=(
   app_execution_interruption const & x
) {
   execution_interruption::operator=(x);
   return *this;
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

app_exit_interruption::app_exit_interruption() {
}

app_exit_interruption::app_exit_interruption(app_exit_interruption const & x) :
   execution_interruption(x) {
}

/*virtual*/ app_exit_interruption::~app_exit_interruption() {
}

app_exit_interruption & app_exit_interruption::operator=(app_exit_interruption const & x) {
   execution_interruption::operator=(x);
   return *this;
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*explicit*/ argument_error::argument_error(errint_t err /*= 0*/) :
   generic_error(err ? err :
#if ABC_HOST_API_POSIX
      EINVAL
#else
      0
#endif
   ) {
}

argument_error::argument_error(argument_error const & x) :
   generic_error(x) {
}

/*virtual*/ argument_error::~argument_error() {
}

argument_error & argument_error::operator=(argument_error const & x) {
   generic_error::operator=(x);
   return *this;
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

coroutine_local_value<bool> assertion_error::sm_bReentering /*= false*/;

/*static*/ void assertion_error::_assertion_failed(
   source_file_address const & sfa, str const & sExpr, str const & sMsg
) {
   if (!sm_bReentering) {
      sm_bReentering = true;
      try {
         io::text::stderr->print(
            ABC_SL("Assertion failed: {} ( {} ) in file {}: in function {}\n"),
            sMsg, sExpr, sfa.file_address(), sfa.function()
         );
      } catch (...) {
         sm_bReentering = false;
         throw;
      }
      sm_bReentering = false;
   }
   ABC_THROW(assertion_error, ());
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

destructing_unfinalized_object::destructing_unfinalized_object(
   destructing_unfinalized_object const & x
) :
   exception(x) {
}

/*virtual*/ destructing_unfinalized_object::~destructing_unfinalized_object() {
}

destructing_unfinalized_object & destructing_unfinalized_object::operator=(
   destructing_unfinalized_object const & x
) {
   exception::operator=(x);
   return *this;
}

void destructing_unfinalized_object::write_what(void const * pObj, _std::type_info const & ti) {
   what_writer().print(
      ABC_SL("instance of {} @ {} being destructed before finalize() was invoked on it"), ti, pObj
   );
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*explicit*/ domain_error::domain_error(errint_t err /*= 0*/) :
   generic_error(err ? err :
#if ABC_HOST_API_POSIX
      EDOM
#else
      0
#endif
   ) {
}

domain_error::domain_error(domain_error const & x) :
   generic_error(x) {
}

/*virtual*/ domain_error::~domain_error() {
}

domain_error & domain_error::operator=(domain_error const & x) {
   generic_error::operator=(x);
   return *this;
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

execution_interruption::execution_interruption(/*source?*/) {
}

execution_interruption::execution_interruption(execution_interruption const & x) :
   exception(x) {
}

/*virtual*/ execution_interruption::~execution_interruption() {
}

execution_interruption & execution_interruption::operator=(execution_interruption const & x) {
   exception::operator=(x);
   return *this;
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

exception::exception() :
   m_bInFlight(false),
   m_cchWhatAvailable(ABC_COUNTOF(m_szWhat) - 1 /*NUL*/) {
   m_szWhat[0] = '\0';
}

exception::exception(exception const & x) :
   m_sfa(x.m_sfa),
   m_bInFlight(x.m_bInFlight),
   m_cchWhatAvailable(x.m_cchWhatAvailable) {
   // See @ref stack-tracing.
   if (m_bInFlight) {
      detail::scope_trace::trace_writer_addref();
   }
   memory::copy<char>(m_szWhat, x.m_szWhat, ABC_COUNTOF(m_szWhat) - m_cchWhatAvailable);
}

/*virtual*/ exception::~exception() {
   // See @ref stack-tracing.
   if (m_bInFlight) {
      detail::scope_trace::trace_writer_release();
   }
}

exception & exception::operator=(exception const & x) {
   m_sfa = x.m_sfa;
   /* Adopt the source’s in-flight status. See @ref stack-tracing. If the in-flight status is not
   changing, avoid the pointless (and dangerous, if done in this sequence – it could delete the
   trace writer if *this was the last reference to it) release()/addref(). */
   if (m_bInFlight != x.m_bInFlight) {
      if (m_bInFlight) {
         detail::scope_trace::trace_writer_release();
      }
      m_bInFlight = x.m_bInFlight;
      if (m_bInFlight) {
         detail::scope_trace::trace_writer_addref();
      }
   }
   m_cchWhatAvailable = x.m_cchWhatAvailable;
   memory::copy<char>(m_szWhat, x.m_szWhat, ABC_COUNTOF(m_szWhat) - m_cchWhatAvailable);
   return *this;
}

void exception::_before_throw(source_file_address const & sfa) {
   m_sfa = sfa;
   /* Clear any old trace writer buffer and create a new one with *this as its only reference. See
   @ref stack-tracing. */
   detail::scope_trace::trace_writer_clear();
   detail::scope_trace::trace_writer_addref();
   m_bInFlight = true;
}

#if ABC_HOST_API_MACH || ABC_HOST_API_POSIX
/*static*/ void exception::inject_in_context(
   common_type xct, std::intptr_t iArg0, std::intptr_t iArg1, void * pvctx
) {
   /* Abstract away the differences in the context structures across different OSes and
   architectures by defining references to each register needed below. */
#if ABC_HOST_API_MACH
   #if ABC_HOST_ARCH_X86_64
      ::x86_thread_state64_t * pthrst = static_cast< ::x86_thread_state64_t *>(pvctx);
      typedef std::uint64_t reg_t;
      reg_t & iCodePtr = pthrst->__rip, & iStackPtr = pthrst->__rsp;
      reg_t & rdi = pthrst->__rdi, & rsi = pthrst->__rsi, & rdx = pthrst->__rdx;
   #endif
#elif ABC_HOST_API_POSIX
   auto & mctx = static_cast< ::ucontext_t *>(pvctx)->uc_mcontext;
   #if ABC_HOST_ARCH_ARM
      #if ABC_HOST_API_LINUX
         typedef long reg_t;
         reg_t & iCodePtr = mctx.arm_pc, & iStackPtr = mctx.arm_sp, & lr = mctx.arm_lr;
         reg_t & r0 = mctx.arm_r0, & r1 = mctx.arm_r1, & r2 = mctx.arm_r2;
      #endif
   #elif ABC_HOST_ARCH_I386
      #if ABC_HOST_API_LINUX
         typedef long reg_t;
         reg_t & iCodePtr = mctx.gregs[REG_EIP], & iStackPtr = mctx.gregs[REG_ESP];
      #elif ABC_HOST_API_FREEBSD
         typedef std::uint32_t reg_t;
         reg_t & iCodePtr = mctx.mc_eip, & iStackPtr = mctx.mc_esp;
      #endif
   #elif ABC_HOST_ARCH_X86_64
      #if ABC_HOST_API_LINUX
         typedef long long reg_t;
         reg_t & iCodePtr = mctx.gregs[REG_RIP], & iStackPtr = mctx.gregs[REG_RSP];
         reg_t & rdi = mctx.gregs[REG_RDI], & rsi = mctx.gregs[REG_RSI];
         reg_t & rdx = mctx.gregs[REG_RDX];
      #elif ABC_HOST_API_FREEBSD
         typedef long reg_t;
         reg_t & iCodePtr = mctx.mc_rip, & iStackPtr = mctx.mc_rsp;
         reg_t & rdi = mctx.mc_rdi, & rsi = mctx.mc_rsi, & rdx = mctx.mc_rdx;
      #endif
   #endif
#else
   #error "TODO: HOST_API"
#endif

   // Emulate a 3-argument function call to throw_common_type().
#if ABC_HOST_ARCH_X86_64
   // Make sure the stack is aligned. Seems unnecessary?
   //iStackPtr &= ~reg_t(0xf);
#endif
   reg_t * pStack = reinterpret_cast<reg_t *>(iStackPtr);
#if ABC_HOST_ARCH_ARM
   /* Load the arguments into r0-2, push lr and replace it with the address of the current
   instruction. */
   r0 = static_cast<reg_t>(xct.base());
   r1 = static_cast<reg_t>(iArg0);
   r2 = static_cast<reg_t>(iArg1);
   *--pStack = lr;
   lr = iCodePtr;
#elif ABC_HOST_ARCH_I386
   // Push the arguments onto the stack, push the address of the current instruction.
   *--pStack = static_cast<reg_t>(iArg1);
   *--pStack = static_cast<reg_t>(iArg0);
   *--pStack = static_cast<reg_t>(xct.base());
   *--pStack = iCodePtr;
#elif ABC_HOST_ARCH_X86_64
   // Load the arguments into rdi/rsi/rdx, push the address of the current instruction.
   rdi = static_cast<reg_t>(xct.base());
   rsi = static_cast<reg_t>(iArg0);
   rdx = static_cast<reg_t>(iArg1);
   *--pStack = iCodePtr;
   // Stack alignment to 16 bytes is done by the callee with push rbp.
#else
   #error "TODO: HOST_ARCH"
#endif
   // Jump to the start of throw_common_type().
   iCodePtr = reinterpret_cast<reg_t>(&throw_common_type);
   iStackPtr = reinterpret_cast<reg_t>(pStack);
}
#endif //if ABC_HOST_API_MACH || ABC_HOST_API_POSIX

/*static*/ void exception::throw_common_type(
   common_type::enum_type xct, std::intptr_t iArg0, std::intptr_t iArg1
) {
   static detail::source_file_address_data const sc_sfadInternal = {
      ABC_SL("<internal>"),           { ABC_SL("source_not_available"), 0 }
   };
   static detail::source_file_address_data const sc_sfadOS = {
      ABC_SL("<OS error reporting>"), { ABC_SL("source_not_available"), 0 }
   };
   source_file_address const & sfaInternal = *source_file_address::from_data(&sc_sfadInternal);
   source_file_address const & sfaOS       = *source_file_address::from_data(&sc_sfadOS      );

   ABC_UNUSED_ARG(iArg1);
   switch (xct) {
      case common_type::app_execution_interruption:
      case common_type::app_exit_interruption:
      case common_type::execution_interruption:
      case common_type::user_forced_interruption:
         /* Check if the thread is already terminating, and avoid throwing an interruption exception
         if the thread is terminating anyway. This check is safe because m_bTerminating can only be
         written to by the current thread. */
         if (!this_thread::get_impl()->terminating()) {
            switch (xct) {
               case common_type::app_execution_interruption:
                  ABC_THROW_FROM(sfaInternal, app_execution_interruption, ());
               case common_type::app_exit_interruption:
                  ABC_THROW_FROM(sfaInternal, app_exit_interruption, ());
               case common_type::execution_interruption:
                  ABC_THROW_FROM(sfaInternal, execution_interruption, ());
               case common_type::user_forced_interruption:
                  ABC_THROW_FROM(sfaInternal, user_forced_interruption, ());
               default:
                  // Silence compiler warnings.
                  break;
            }
         }
         break;
      case common_type::math_arithmetic_error:
         ABC_THROW_FROM(sfaOS, math::arithmetic_error, ());
      case common_type::math_division_by_zero:
         ABC_THROW_FROM(sfaOS, math::division_by_zero, ());
      case common_type::math_floating_point_error:
         ABC_THROW_FROM(sfaOS, math::floating_point_error, ());
      case common_type::math_overflow:
         ABC_THROW_FROM(sfaOS, math::overflow, ());
      case common_type::memory_bad_pointer:
         ABC_THROW_FROM(sfaOS, memory::bad_pointer, (reinterpret_cast<void const *>(iArg0)));
      case common_type::memory_bad_pointer_alignment:
         ABC_THROW_FROM(
            sfaOS, memory::bad_pointer_alignment, (reinterpret_cast<void const *>(iArg0))
         );
      default:
         // Unexpected exception type. Should never happen.
         std::abort();
   }
}

/*static*/ exception::common_type exception::execution_interruption_to_common_type(
   _std::exception const * px /*= nullptr */
) {
   if (px) {
      // The order of the dynamic_casts matters, since some are subclasses of others.
      if (dynamic_cast<app_execution_interruption const *>(px)) {
         return common_type::app_execution_interruption;
      } else if (dynamic_cast<app_exit_interruption const *>(px)) {
         return common_type::app_exit_interruption;
      } else if (dynamic_cast<user_forced_interruption const *>(px)) {
         return common_type::user_forced_interruption;
      } else if (dynamic_cast<execution_interruption const *>(px)) {
         return common_type::execution_interruption;
      }
   }
   // Not an execution_interruption subclass, or not even an std::exception (nullptr was passed).
   // TODO: use a more specific common_type, such as other_coroutine/thread_execution_interrupted.
   return common_type::execution_interruption;
}

/*virtual*/ const char * exception::what() const ABC_STL_NOEXCEPT_TRUE() /*override*/ {
   return m_szWhat;
}

io::text::char_ptr_writer exception::what_writer() {
   std::size_t cchUsed = ABC_COUNTOF(m_szWhat) - 1 /*NUL*/ - m_cchWhatAvailable;
   io::text::char_ptr_writer cpw(m_szWhat + cchUsed, &m_cchWhatAvailable);
   // If the what() string has already been written to, add a separator before a new write.
   if (cchUsed > 0) {
      cpw.write(ABC_SL("; "));
   }
   return _std::move(cpw);
}

/*static*/ void exception::write_with_scope_trace(
   io::text::writer * ptwOut /*= nullptr*/, _std::exception const * pstdx /*= nullptr*/
) {
   if (!ptwOut) {
      ptwOut = io::text::stderr.get();
   }
   exception const * pabcx;
   if (pstdx) {
      // We have an std::exception: print its what() and check if it’s also an abc::exception.
      ptwOut->print(ABC_SL("Exception in PID:{} TID:{}"), this_process::id(), this_thread::id());
      if (auto crid =  this_coroutine::id()) {
         ptwOut->print(ABC_SL(" CRID:{}"), crid);
      }
      ptwOut->print(
         ABC_SL("\n{}: {}\n"), typeid(*pstdx), text::char_ptr_to_str_adapter(pstdx->what())
      );
      pabcx = dynamic_cast<exception const *>(pstdx);
   } else {
      pabcx = nullptr;
   }

   ptwOut->write(ABC_SL("Stack trace (most recent call first):\n"));
   if (pabcx) {
      // Frame 0 is the location of the ABC_THROW() statement.
      ptwOut->print(
         ABC_SL("#0 {} at {}\n"),
         str(external_buffer, pabcx->m_sfa.function()), pabcx->m_sfa.file_address()
      );
   }
   // Print the scope/stack trace collected via ABC_TRACE_FUNC().
   ptwOut->write(detail::scope_trace::get_trace_writer()->get_str());
   // Append any scope_trace instances that haven’t been destructed yet.
   detail::scope_trace::write_list(ptwOut);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*explicit*/ generic_error::generic_error(errint_t err /*= 0*/) :
   m_err(err) {
   if (m_err) {
      what_writer().print(ABC_SL("OS error={}"), m_err);
   }
}

generic_error::generic_error(generic_error const & x) :
   exception(x),
   m_err(x.m_err) {
}

/*virtual*/ generic_error::~generic_error() {
}

generic_error & generic_error::operator=(generic_error const & x) {
   exception::operator=(x);
   m_err = x.m_err;
   return *this;
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*explicit*/ network_error::network_error(errint_t err /*= 0*/) :
   generic_error(err) {
}

network_error::network_error(network_error const & x) :
   generic_error(x) {
}

/*virtual*/ network_error::~network_error() {
}

network_error & network_error::operator=(network_error const & x) {
   generic_error::operator=(x);
   return *this;
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*explicit*/ security_error::security_error(errint_t err /*= 0*/) :
   generic_error(err) {
}

security_error::security_error(security_error const & x) :
   generic_error(x) {
}

/*virtual*/ security_error::~security_error() {
}

security_error & security_error::operator=(security_error const & x) {
   generic_error::operator=(x);
   return *this;
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*explicit*/ syntax_error::syntax_error(
   str const & sDescription, str const & sSource /*= str::empty*/, unsigned iChar /*= 0*/,
   unsigned iLine /*= 0*/, errint_t err /*= 0*/
) :
   generic_error(err),
   m_sDescription(sDescription),
   m_sSource(sSource),
   m_iChar(iChar),
   m_iLine(iLine) {

   str sFormat;
   if (m_sSource) {
      if (m_iChar) {
         if (m_iLine) {
            sFormat = ABC_SL("{0} in {1}:{2}:{3}");
         } else {
            sFormat = ABC_SL("{0} in expression \"{1}\", character {3}");
         }
      } else {
         if (m_iLine) {
            sFormat = ABC_SL("{0} in {1}:{2}");
         } else {
            sFormat = ABC_SL("{0} in expression \"{1}\"");
         }
      }
   } else {
      if (m_iChar) {
         if (m_iLine) {
            sFormat = ABC_SL("{0} in <input>:{2}:{3}");
         } else {
            sFormat = ABC_SL("{0} in <expression>, character {3}");
         }
      } else {
         if (m_iLine) {
            sFormat = ABC_SL("{0} in <input>:{2}");
         } else {
            sFormat = ABC_SL("{0}");
         }
      }
   }
   what_writer().print(sFormat, m_sDescription, m_sSource, m_iLine, m_iChar);
}

syntax_error::syntax_error(syntax_error const & x) :
   generic_error(x),
   m_sDescription(x.m_sDescription),
   m_sSource(x.m_sSource),
   m_iChar(x.m_iChar),
   m_iLine(x.m_iLine) {
}

/*virtual*/ syntax_error::~syntax_error() {
}

syntax_error & syntax_error::operator=(syntax_error const & x) {
   generic_error::operator=(x);
   m_sDescription = x.m_sDescription;
   m_sSource = x.m_sSource;
   m_iChar = x.m_iChar;
   m_iLine = x.m_iLine;
   return *this;
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

user_forced_interruption::user_forced_interruption() {
}

user_forced_interruption::user_forced_interruption(user_forced_interruption const & x) :
   app_execution_interruption(x) {
}

/*virtual*/ user_forced_interruption::~user_forced_interruption() {
}

user_forced_interruption & user_forced_interruption::operator=(user_forced_interruption const & x) {
   app_execution_interruption::operator=(x);
   return *this;
}

} //namespace abc
