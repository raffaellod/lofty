/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with Abaclade. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#include <abaclade/coroutine.hxx>
#include <abaclade/process.hxx>
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

void to_str_backend<source_location>::set_format(str const & sFormat) {
   ABC_TRACE_FUNC(this, sFormat);

   auto it(sFormat.cbegin());

   // Add parsing of the format string here.

   // If we still have any characters, they are garbage.
   if (it != sFormat.cend()) {
      ABC_THROW(syntax_error, (
         ABC_SL("unexpected character"), sFormat, static_cast<unsigned>(it - sFormat.cbegin())
      ));
   }
}

void to_str_backend<source_location>::write(
   source_location const & srcloc, io::text::writer * ptwOut
) {
   ABC_TRACE_FUNC(this/*, srcloc*/, ptwOut);

   ptwOut->write(str(external_buffer, srcloc.file_path()));
   ptwOut->write(ABC_SL(":"));
   ptwOut->write(srcloc.line_number());
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

exception::exception() :
   m_pszWhat("abc::exception"),
   m_pszSourceFunction(nullptr),
   m_bInFlight(false) {
}
exception::exception(exception const & x) :
   m_pszWhat(x.m_pszWhat),
   m_pszSourceFunction(x.m_pszSourceFunction),
   m_srcloc(x.m_srcloc),
   m_bInFlight(x.m_bInFlight) {
   // See @ref stack-tracing.
   if (m_bInFlight) {
      detail::scope_trace::trace_writer_addref();
   }
}

/*virtual*/ exception::~exception() {
   // See @ref stack-tracing.
   if (m_bInFlight) {
      detail::scope_trace::trace_writer_release();
   }
}

exception & exception::operator=(exception const & x) {
   m_pszWhat = x.m_pszWhat;
   m_pszSourceFunction = x.m_pszSourceFunction;
   m_srcloc = x.m_srcloc;
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
   return *this;
}

void exception::_before_throw(source_location const & srcloc, char_t const * pszFunction) {
   m_pszSourceFunction = pszFunction;
   m_srcloc = srcloc;
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
   // Make sure the stack is aligned.
   if (iStackPtr & 0xf) {
      --pStack;
   }
   /* Load the arguments into rdi/rsi/rdx (Mach, POSIX), push the address of the current
   instruction. */
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
   source_location srcloc(ABC_SL("source_not_available"), 0);
   static char_t const sc_szInternal[] = ABC_SL("<internal>");
   static char_t const sc_szOS[] = ABC_SL("<OS error reporting>");

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
                  _ABC_THROW_FROM(srcloc, sc_szInternal, app_execution_interruption, ());
               case common_type::app_exit_interruption:
                  _ABC_THROW_FROM(srcloc, sc_szInternal, app_exit_interruption, ());
               case common_type::execution_interruption:
                  _ABC_THROW_FROM(srcloc, sc_szInternal, execution_interruption, ());
               case common_type::user_forced_interruption:
                  _ABC_THROW_FROM(srcloc, sc_szInternal, user_forced_interruption, ());
               default:
                  // Silence compiler warnings.
                  break;
            }
         }
         break;
      case common_type::arithmetic_error:
         _ABC_THROW_FROM(srcloc, sc_szOS, arithmetic_error, ());
      case common_type::division_by_zero_error:
         _ABC_THROW_FROM(srcloc, sc_szOS, division_by_zero_error, ());
      case common_type::floating_point_error:
         _ABC_THROW_FROM(srcloc, sc_szOS, floating_point_error, ());
      case common_type::memory_access_error:
         _ABC_THROW_FROM(
            srcloc, sc_szOS, memory::access_error, (reinterpret_cast<void const *>(iArg0))
         );
      case common_type::memory_address_error:
         _ABC_THROW_FROM(
            srcloc, sc_szOS, memory::address_error, (reinterpret_cast<void const *>(iArg0))
         );
      case common_type::null_pointer_error:
         _ABC_THROW_FROM(srcloc, sc_szOS, memory::null_pointer_error, ());
      case common_type::overflow_error:
         _ABC_THROW_FROM(srcloc, sc_szOS, overflow_error, ());
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

char const * exception::what() const {
   return m_pszWhat;
}

/*virtual*/ void exception::write_extended_info(io::text::writer * ptwOut) const {
   ABC_UNUSED_ARG(ptwOut);
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
      ptwOut->print(
         ABC_SL("Exception in PID:{},TID:{}"), this_process::id(), this_thread::id()
      );
      if (auto crid =  this_coroutine::id()) {
         ptwOut->print(ABC_SL(",CRID:{}"), crid);
      }
      ptwOut->print(ABC_SL(": {}\n"), text::char_ptr_to_str_adapter(pstdx->what()));
      pabcx = dynamic_cast<exception const *>(pstdx);
      if (pabcx) {
         try {
            ptwOut->write(ABC_SL("Extended information:"));
            pabcx->write_extended_info(ptwOut);
            ptwOut->write_line();
         } catch (...) {
            /* The exception is not rethrown because we don’t want exception details to interfere
            with the display of the (more important) exception information. */
            // FIXME: EXC-SWALLOW
         }
      }
   } else {
      pabcx = nullptr;
   }

   ptwOut->write(ABC_SL("Stack trace (most recent call first):\n"));
   if (pabcx) {
      // Frame 0 is the location of the ABC_THROW() statement.
      ptwOut->print(
         ABC_SL("#0 {} at {}\n"), str(external_buffer, pabcx->m_pszSourceFunction), pabcx->m_srcloc
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

destructing_unfinalized_object::destructing_unfinalized_object() {
   m_pszWhat = "abc::destructing_unfinalized_object";
}

destructing_unfinalized_object::destructing_unfinalized_object(
   destructing_unfinalized_object const & x
) :
   exception(x),
#if ABC_HOST_UTF > 8
   m_vchWhat(x.m_vchWhat),
   m_sWhat(x.m_sWhat) {
   m_pszWhat = reinterpret_cast<char const *>(m_vchWhat.data());
#else
   m_sWhat(x.m_sWhat) {
   m_pszWhat = m_sWhat.c_str();
#endif
}

/*virtual*/ destructing_unfinalized_object::~destructing_unfinalized_object() {
}

destructing_unfinalized_object & destructing_unfinalized_object::operator=(
   destructing_unfinalized_object const & x
) {
#if ABC_HOST_UTF > 8
   m_vchWhat = x.m_vchWhat;
   m_sWhat = x.m_sWhat;
   m_pszWhat = reinterpret_cast<char const *>(m_vchWhat.data());
#else
   m_sWhat = x.m_sWhat;
   m_pszWhat = m_sWhat.c_str();
#endif
   return *this;
}

void destructing_unfinalized_object::init(void const * pObj, _std::type_info const & ti) {
   m_sWhat = str(ABC_SL("object being destructed={} @ {}")).format(ti, pObj);
#if ABC_HOST_UTF > 8
   // TODO: abc::text::encoding::ascii.
   m_vchWhat = m_sWhat.encode(text::encoding::utf8, true);
   m_pszWhat = reinterpret_cast<char const *>(m_vchWhat.data());
#else
   m_pszWhat = m_sWhat.c_str();
#endif
}

/*virtual*/ void destructing_unfinalized_object::write_extended_info(
   io::text::writer * ptwOut
) const /*override*/ {
   exception::write_extended_info(ptwOut);
   ptwOut->print(ABC_SL(" {}"), m_sWhat);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

execution_interruption::execution_interruption(/*source?*/) {
   m_pszWhat = "abc::execution_interruption";
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

app_execution_interruption::app_execution_interruption() {
   m_pszWhat = "abc::app_execution_interruption";
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
   m_pszWhat = "abc::app_exit_interruption";
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

user_forced_interruption::user_forced_interruption() {
   m_pszWhat = "abc::user_forced_interruption";
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

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

argument_error::argument_error() :
   generic_error() {
   m_pszWhat = "abc::argument_error";
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

void argument_error::init(errint_t err /*= 0*/) {
   generic_error::init(err ? err :
#if ABC_HOST_API_POSIX
      EINVAL
#else
      0
#endif
   );
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

arithmetic_error::arithmetic_error() :
   generic_error() {
   m_pszWhat = "abc::arithmetic_error";
}

arithmetic_error::arithmetic_error(arithmetic_error const & x) :
   generic_error(x) {
}

/*virtual*/ arithmetic_error::~arithmetic_error() {
}

arithmetic_error & arithmetic_error::operator=(arithmetic_error const & x) {
   generic_error::operator=(x);
   return *this;
}

void arithmetic_error::init(errint_t err /*= 0*/) {
   generic_error::init(err);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

coroutine_local_value<bool> assertion_error::sm_bReentering /*= false*/;

/*static*/ void assertion_error::_assertion_failed(
   source_location const & srcloc, str const & sFunction, str const & sExpr, str const & sMsg
) {
   if (!sm_bReentering) {
      sm_bReentering = true;
      try {
         io::text::stderr->print(
            ABC_SL("Assertion failed: {} ( {} ) in file {}: in function {}\n"),
            sMsg, sExpr, srcloc, sFunction
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

buffer_error::buffer_error() :
   generic_error() {
   m_pszWhat = "abc::buffer_error";
}

buffer_error::buffer_error(buffer_error const & x) :
   generic_error(x) {
}

/*virtual*/ buffer_error::~buffer_error() {
}

buffer_error & buffer_error::operator=(buffer_error const & x) {
   generic_error::operator=(x);
   return *this;
}

void buffer_error::init(errint_t err /*= 0*/) {
   generic_error::init(err);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

division_by_zero_error::division_by_zero_error() :
   generic_error(),
   arithmetic_error() {
   m_pszWhat = "abc::division_by_zero_error";
}

division_by_zero_error::division_by_zero_error(division_by_zero_error const & x) :
   generic_error(x),
   arithmetic_error(x) {
}

/*virtual*/ division_by_zero_error::~division_by_zero_error() {
}

division_by_zero_error & division_by_zero_error::operator=(division_by_zero_error const & x) {
   arithmetic_error::operator=(x);
   return *this;
}

void division_by_zero_error::init(errint_t err /*= 0*/) {
   arithmetic_error::init(err);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

domain_error::domain_error() :
   generic_error() {
   m_pszWhat = "abc::domain_error";
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

void domain_error::init(errint_t err /*= 0*/) {
   generic_error::init(err ? err :
#if ABC_HOST_API_POSIX
      EDOM
#else
      0
#endif
   );
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

floating_point_error::floating_point_error() :
   generic_error(),
   arithmetic_error() {
   m_pszWhat = "abc::floating_point_error";
}

floating_point_error::floating_point_error(floating_point_error const & x) :
   generic_error(x),
   arithmetic_error(x) {
}

/*virtual*/ floating_point_error::~floating_point_error() {
}

floating_point_error & floating_point_error::operator=(floating_point_error const & x) {
   arithmetic_error::operator=(x);
   return *this;
}

void floating_point_error::init(errint_t err /*= 0*/) {
   arithmetic_error::init(err);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

generic_error::generic_error() :
   exception() {
   m_pszWhat = "abc::generic_error";
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

/*virtual*/ void generic_error::write_extended_info(io::text::writer * ptwOut) const /*override*/ {
   exception::write_extended_info(ptwOut);
   if (m_err) {
      ptwOut->print(ABC_SL(" OS error={}"), m_err);
   }
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

index_error::index_error() :
   lookup_error() {
   m_pszWhat = "abc::index_error";
}
index_error::index_error(index_error const & x) :
   generic_error(x),
   lookup_error(x),
   m_iInvalid(x.m_iInvalid),
   m_iMin(x.m_iMin),
   m_iMax(x.m_iMax),
   m_bMinMaxProvided(x.m_bMinMaxProvided) {
}

/*virtual*/ index_error::~index_error() {
}

index_error & index_error::operator=(index_error const & x) {
   lookup_error::operator=(x);
   m_iInvalid = x.m_iInvalid;
   m_iMin = x.m_iMin;
   m_iMax = x.m_iMax;
   m_bMinMaxProvided = x.m_bMinMaxProvided;
   return *this;
}

void index_error::init(std::ptrdiff_t iInvalid, errint_t err /*= 0*/) {
   lookup_error::init(err);
   m_iInvalid = iInvalid;
   m_bMinMaxProvided = false;
}

void index_error::init(
   std::ptrdiff_t iInvalid, std::ptrdiff_t iMin, std::ptrdiff_t iMax, errint_t err /*= 0*/
) {
   lookup_error::init(err);
   m_iInvalid = iInvalid;
   m_iMin = iMin;
   m_iMax = iMax;
   m_bMinMaxProvided = true;
}

/*virtual*/ void index_error::write_extended_info(io::text::writer * ptwOut) const /*override*/ {
   lookup_error::write_extended_info(ptwOut);
   str sFormat;
   if (m_bMinMaxProvided) {
      sFormat = ABC_SL(" invalid index={0}; valid range=[{1}, {2}]");
   } else {
      sFormat = ABC_SL(" invalid index={0}");
   }
   ptwOut->print(sFormat, m_iInvalid, m_iMin, m_iMax);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

iterator_error::iterator_error() :
   generic_error() {
   m_pszWhat = "abc::iterator_error";
}

iterator_error::iterator_error(iterator_error const & x) :
   generic_error(x) {
}

/*virtual*/ iterator_error::~iterator_error() {
}

iterator_error & iterator_error::operator=(iterator_error const & x) {
   generic_error::operator=(x);
   return *this;
}

void iterator_error::init(errint_t err /*= 0*/) {
   generic_error::init(err);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

key_error::key_error() :
   generic_error(),
   lookup_error() {
   m_pszWhat = "abc::key_error";
}

key_error::key_error(key_error const & x) :
   generic_error(x),
   lookup_error(x) {
}

/*virtual*/ key_error::~key_error() {
}

key_error & key_error::operator=(key_error const & x) {
   lookup_error::operator=(x);
   return *this;
}

void key_error::init(errint_t err /*= 0*/) {
   lookup_error::init(err);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

lookup_error::lookup_error() :
   generic_error() {
   m_pszWhat = "abc::lookup_error";
}

lookup_error::lookup_error(lookup_error const & x) :
   generic_error(x) {
}

/*virtual*/ lookup_error::~lookup_error() {
}

lookup_error & lookup_error::operator=(lookup_error const & x) {
   generic_error::operator=(x);
   return *this;
}

void lookup_error::init(errint_t err /*= 0*/) {
   generic_error::init(err);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

network_error::network_error() :
   generic_error() {
   m_pszWhat = "abc::network_error";
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

void network_error::init(errint_t err /*= 0*/) {
   generic_error::init(err);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

not_implemented_error::not_implemented_error() :
   generic_error() {
   m_pszWhat = "abc::not_implemented_error";
}

not_implemented_error::not_implemented_error(not_implemented_error const & x) :
   generic_error(x) {
}

/*virtual*/ not_implemented_error::~not_implemented_error() {
}

not_implemented_error & not_implemented_error::operator=(not_implemented_error const & x) {
   generic_error::operator=(x);
   return *this;
}

void not_implemented_error::init(errint_t err /*= 0*/) {
   generic_error::init(err);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

overflow_error::overflow_error() :
   generic_error(),
   arithmetic_error() {
   m_pszWhat = "abc::overflow_error";
}

overflow_error::overflow_error(overflow_error const & x) :
   generic_error(x),
   arithmetic_error(x) {
}

/*virtual*/ overflow_error::~overflow_error() {
}

overflow_error & overflow_error::operator=(overflow_error const & x) {
   arithmetic_error::operator=(x);
   return *this;
}

void overflow_error::init(errint_t err /*= 0*/) {
   arithmetic_error::init(err ? err :
#if ABC_HOST_API_POSIX
      EOVERFLOW
#else
      0
#endif
   );
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

security_error::security_error() :
   generic_error() {
   m_pszWhat = "abc::security_error";
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

void security_error::init(errint_t err /*= 0*/) {
   generic_error::init(err);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

syntax_error::syntax_error() :
   generic_error() {
   m_pszWhat = "abc::syntax_error";
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

void syntax_error::init(
   str const & sDescription /*= str::empty*/, str const & sSource /*= str::empty*/,
   unsigned iChar /*= 0*/, unsigned iLine /*= 0*/, errint_t err /*= 0*/
) {
   generic_error::init(err);
   m_sDescription = sDescription;
   m_sSource = sSource;
   m_iChar = iChar;
   m_iLine = iLine;
}

/*virtual*/ void syntax_error::write_extended_info(io::text::writer * ptwOut) const /*override*/ {
   generic_error::write_extended_info(ptwOut);
   str sFormat;
   if (m_sSource) {
      if (m_iChar) {
         if (m_iLine) {
            sFormat = ABC_SL(" {0} in {1}:{2}:{3}");
         } else {
            sFormat = ABC_SL(" {0} in expression \"{1}\", character {3}");
         }
      } else {
         if (m_iLine) {
            sFormat = ABC_SL(" {0} in {1}:{2}");
         } else {
            sFormat = ABC_SL(" {0} in expression \"{1}\"");
         }
      }
   } else {
      if (m_iChar) {
         if (m_iLine) {
            sFormat = ABC_SL(" {0} in <input>:{2}:{3}");
         } else {
            sFormat = ABC_SL(" {0} in <expression>, character {3}");
         }
      } else {
         if (m_iLine) {
            sFormat = ABC_SL(" {0} in <input>:{2}");
         } else {
            sFormat = ABC_SL(" {0}");
         }
      }
   }

   ptwOut->print(sFormat, m_sDescription, m_sSource, m_iLine, m_iChar);
}

} //namespace abc
