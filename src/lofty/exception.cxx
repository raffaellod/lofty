/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/coroutine.hxx>
#include <lofty/coroutine_local.hxx>
#include <lofty/exception.hxx>
#include <lofty/io/text.hxx>
#include <lofty/io/text/str.hxx>
#include <lofty/logging.hxx>
#include <lofty/math.hxx>
#include <lofty/memory.hxx>
#include <lofty/process.hxx>
#include <lofty/_std/exception.hxx>
#include <lofty/_std/utility.hxx>
#include <lofty/text.hxx>
#include <lofty/text/char_ptr_to_str_adapter.hxx>
#include <lofty/text/str.hxx>
#include <lofty/thread.hxx>
#include "thread-impl.hxx"
#include <cstdlib> // std::abort()
#if LOFTY_HOST_API_POSIX
   #include <errno.h> // E* errno
   #if LOFTY_HOST_API_MACH
      #include <mach/mach.h>
   #else
      #include <ucontext.h>
   #endif
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*explicit*/ argument_error::argument_error(errint_t err_ /*= 0*/) :
   generic_error(err_ ? err_ :
#if LOFTY_HOST_API_POSIX
      EINVAL
#else
      0
#endif
   ) {
}

argument_error::argument_error(argument_error const & src) :
   generic_error(src) {
}

/*virtual*/ argument_error::~argument_error() LOFTY_STL_NOEXCEPT_TRUE() {
}

argument_error & argument_error::operator=(argument_error const & src) {
   generic_error::operator=(src);
   return *this;
}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

coroutine_local_value<bool> assertion_error::reentering /*= false*/;

/*static*/ void assertion_error::_assertion_failed(
   source_file_address const & source_file_addr, text::str const & expr, text::str const & msg
) {
   if (!reentering) {
      reentering = true;
      try {
         io::text::stderr->print(
            LOFTY_SL("Assertion failed: {} ( {} ) in file {}: in function {}\n"),
            msg, expr, source_file_addr.file_address(), source_file_addr.function()
         );
      } catch (...) {
         reentering = false;
         throw;
      }
      reentering = false;
   }
   LOFTY_THROW(assertion_error, ());
}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*explicit*/ domain_error::domain_error(errint_t err_ /*= 0*/) :
   generic_error(err_ ? err_ :
#if LOFTY_HOST_API_POSIX
      EDOM
#else
      0
#endif
   ) {
}

domain_error::domain_error(domain_error const & src) :
   generic_error(src) {
}

/*virtual*/ domain_error::~domain_error() LOFTY_STL_NOEXCEPT_TRUE() {
}

domain_error & domain_error::operator=(domain_error const & src) {
   generic_error::operator=(src);
   return *this;
}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

execution_interruption::execution_interruption(/*source?*/) {
}

execution_interruption::execution_interruption(execution_interruption const & src) :
   exception(src) {
}

/*virtual*/ execution_interruption::~execution_interruption() LOFTY_STL_NOEXCEPT_TRUE() {
}

execution_interruption & execution_interruption::operator=(execution_interruption const & src) {
   exception::operator=(src);
   return *this;
}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

exception::exception() :
   in_flight(false),
   what_buf_available(LOFTY_COUNTOF(what_buf) - 1 /*NUL*/) {
   what_buf[0] = '\0';
}

exception::exception(exception const & src) :
   source_file_addr(src.source_file_addr),
   in_flight(src.in_flight),
   what_buf_available(src.what_buf_available) {
   // See @ref stack-tracing.
   if (in_flight) {
      logging::_pvt::scope_trace::trace_ostream_addref();
   }
   memory::copy<char>(what_buf, src.what_buf, LOFTY_COUNTOF(what_buf) - what_buf_available);
}

/*virtual*/ exception::~exception() LOFTY_STL_NOEXCEPT_TRUE() {
   // See @ref stack-tracing.
   if (in_flight) {
      logging::_pvt::scope_trace::trace_ostream_release();
   }
}

exception & exception::operator=(exception const & src) {
   source_file_addr = src.source_file_addr;
   /* Adopt the src’s in-flight status. See @ref stack-tracing. If the in-flight status is not changing, avoid
   the pointless (and dangerous, if done in this sequence – it could delete the trace ostream if *this was the
   last reference to it) release()/addref(). */
   if (in_flight != src.in_flight) {
      if (in_flight) {
         logging::_pvt::scope_trace::trace_ostream_release();
      }
      in_flight = src.in_flight;
      if (in_flight) {
         logging::_pvt::scope_trace::trace_ostream_addref();
      }
   }
   what_buf_available = src.what_buf_available;
   memory::copy<char>(what_buf, src.what_buf, LOFTY_COUNTOF(what_buf) - what_buf_available);
   return *this;
}

void exception::_before_throw(source_file_address const & source_file_addr_) {
   source_file_addr = source_file_addr_;
   /* Clear any old trace ostream buffer and create a new one with *this as its only reference. See
   @ref stack-tracing. */
   logging::_pvt::scope_trace::trace_ostream_clear();
   logging::_pvt::scope_trace::trace_ostream_addref();
   in_flight = true;
}

#if LOFTY_HOST_API_MACH || LOFTY_HOST_API_POSIX
/*static*/ void exception::inject_in_context(
   common_type x_type, std::intptr_t arg0, std::intptr_t arg1, void * os_context
) {
   /* Abstract away the differences in the context structures across different OSes and architectures by
   defining references to each register needed below. */
#if LOFTY_HOST_API_MACH
   #if LOFTY_HOST_ARCH_X86_64
      ::x86_thread_state64_t * thread_state = static_cast< ::x86_thread_state64_t *>(os_context);
      typedef std::uint64_t reg_t;
      reg_t & code_ptr_reg = thread_state->__rip, & stack_ptr_reg = thread_state->__rsp;
      reg_t & rdi = thread_state->__rdi, & rsi = thread_state->__rsi, & rdx = thread_state->__rdx;
   #endif
#elif LOFTY_HOST_API_POSIX
   auto & ucontext = static_cast< ::ucontext_t *>(os_context)->uc_mcontext;
   #if LOFTY_HOST_ARCH_ARM
      #if LOFTY_HOST_API_LINUX
         typedef unsigned long reg_t;
         reg_t & code_ptr_reg = ucontext.arm_pc, & stack_ptr_reg = ucontext.arm_sp, & lr = ucontext.arm_lr;
         reg_t & r0 = ucontext.arm_r0, & r1 = ucontext.arm_r1, & r2 = ucontext.arm_r2;
      #endif
   #elif LOFTY_HOST_ARCH_I386
      #if LOFTY_HOST_API_LINUX
         typedef int reg_t;
         reg_t & code_ptr_reg = ucontext.gregs[REG_EIP], & stack_ptr_reg = ucontext.gregs[REG_ESP];
      #elif LOFTY_HOST_API_FREEBSD
         typedef std::uint32_t reg_t;
         reg_t & code_ptr_reg = ucontext.mc_eip, & stack_ptr_reg = ucontext.mc_esp;
      #endif
   #elif LOFTY_HOST_ARCH_X86_64
      #if LOFTY_HOST_API_LINUX
         typedef long long reg_t;
         reg_t & code_ptr_reg = ucontext.gregs[REG_RIP], & stack_ptr_reg = ucontext.gregs[REG_RSP];
         reg_t & rdi = ucontext.gregs[REG_RDI], & rsi = ucontext.gregs[REG_RSI];
         reg_t & rdx = ucontext.gregs[REG_RDX];
      #elif LOFTY_HOST_API_FREEBSD
         typedef long reg_t;
         reg_t & code_ptr_reg = ucontext.mc_rip, & stack_ptr_reg = ucontext.mc_rsp;
         reg_t & rdi = ucontext.mc_rdi, & rsi = ucontext.mc_rsi, & rdx = ucontext.mc_rdx;
      #endif
   #endif
#else
   #error "TODO: HOST_API"
#endif

   // Emulate a 3-argument function call to throw_common_type().
#if LOFTY_HOST_ARCH_X86_64
   // Make sure the stack is aligned. Seems unnecessary?
   //stack_ptr_reg &= ~reg_t(0xf);
#endif
   reg_t * stack_ptr = reinterpret_cast<reg_t *>(stack_ptr_reg);
#if LOFTY_HOST_ARCH_ARM
   // Load the arguments into r0-2, push lr and replace it with the address of the current instruction.
   r0 = static_cast<reg_t>(x_type.base());
   r1 = static_cast<reg_t>(arg0);
   r2 = static_cast<reg_t>(arg1);
   *--stack_ptr = lr;
   lr = code_ptr_reg;
#elif LOFTY_HOST_ARCH_I386
   // Push the arguments onto the stack, push the address of the current instruction.
   *--stack_ptr = static_cast<reg_t>(arg1);
   *--stack_ptr = static_cast<reg_t>(arg0);
   *--stack_ptr = static_cast<reg_t>(x_type.base());
   *--stack_ptr = code_ptr_reg;
#elif LOFTY_HOST_ARCH_X86_64
   // Load the arguments into rdi/rsi/rdx, push the address of the current instruction.
   rdi = static_cast<reg_t>(x_type.base());
   rsi = static_cast<reg_t>(arg0);
   rdx = static_cast<reg_t>(arg1);
   *--stack_ptr = code_ptr_reg;
   // Stack alignment to 16 bytes is done by the callee with push rbp.
#else
   #error "TODO: HOST_ARCH"
#endif
   // Jump to the start of throw_common_type().
   code_ptr_reg = reinterpret_cast<reg_t>(&throw_common_type);
   stack_ptr_reg = reinterpret_cast<reg_t>(stack_ptr);
}
#endif //if LOFTY_HOST_API_MACH || LOFTY_HOST_API_POSIX

/*static*/ void exception::throw_common_type(
   common_type::enum_type x_type, std::intptr_t arg0, std::intptr_t arg1
) {
   static _pvt::source_file_address_data const internal_source_file_addr_data = {
      LOFTY_SL("<internal>"),           { LOFTY_SL("source_not_available"), 0 }
   };
   static _pvt::source_file_address_data const os_source_file_addr_data = {
      LOFTY_SL("<OS error reporting>"), { LOFTY_SL("source_not_available"), 0 }
   };
   auto const & internal_source_file_addr = *source_file_address::from_data(&internal_source_file_addr_data);
   auto const &       os_source_file_addr = *source_file_address::from_data(&      os_source_file_addr_data);

   LOFTY_UNUSED_ARG(arg1);
   switch (x_type) {
      case common_type::execution_interruption:
      case common_type::process_exit:
      case common_type::process_interruption:
      case common_type::user_forced_interruption:
         /* Check if the thread is already terminating, and avoid throwing an interruption exception if the
         thread is terminating anyway. This check is safe because terminating can only be written to by the
         current thread. */
         if (!this_thread::get_impl()->terminating()) {
            switch (x_type) {
               case common_type::execution_interruption:
                  LOFTY_THROW_FROM(internal_source_file_addr, execution_interruption, ());
               case common_type::process_exit:
                  LOFTY_THROW_FROM(internal_source_file_addr, process_exit, ());
               case common_type::process_interruption:
                  LOFTY_THROW_FROM(internal_source_file_addr, process_interruption, ());
               case common_type::user_forced_interruption:
                  LOFTY_THROW_FROM(internal_source_file_addr, user_forced_interruption, ());
               default:
                  // Silence compiler warnings.
                  break;
            }
         }
         break;
      case common_type::math_arithmetic_error:
         LOFTY_THROW_FROM(os_source_file_addr, math::arithmetic_error, ());
      case common_type::math_division_by_zero:
         LOFTY_THROW_FROM(os_source_file_addr, math::division_by_zero, ());
      case common_type::math_floating_point_error:
         LOFTY_THROW_FROM(os_source_file_addr, math::floating_point_error, ());
      case common_type::math_overflow:
         LOFTY_THROW_FROM(os_source_file_addr, math::overflow, ());
      case common_type::memory_bad_pointer:
         LOFTY_THROW_FROM(os_source_file_addr, memory::bad_pointer, (reinterpret_cast<void const *>(arg0)));
      case common_type::memory_bad_pointer_alignment:
         LOFTY_THROW_FROM(
            os_source_file_addr, memory::bad_pointer_alignment, (reinterpret_cast<void const *>(arg0))
         );
      default:
         // Unexpected exception type. Should never happen.
         std::abort();
   }
}

/*static*/ exception::common_type exception::execution_interruption_to_common_type(
   _std::exception const * x /*= nullptr */
) {
   if (x) {
      // The order of the dynamic_casts matters, since some are subclasses of others.
      if (dynamic_cast<process_interruption const *>(x)) {
         return common_type::process_interruption;
      } else if (dynamic_cast<process_exit const *>(x)) {
         return common_type::process_exit;
      } else if (dynamic_cast<user_forced_interruption const *>(x)) {
         return common_type::user_forced_interruption;
      } else if (dynamic_cast<execution_interruption const *>(x)) {
         return common_type::execution_interruption;
      }
   }
   // Not an execution_interruption subclass, or not even an std::exception (nullptr was passed).
   // TODO: use a more specific common_type, such as other_coroutine/thread_execution_interrupted.
   return common_type::execution_interruption;
}

/*virtual*/ const char * exception::what() const LOFTY_STL_NOEXCEPT_TRUE() /*override*/ {
   return what_buf;
}

io::text::char_ptr_ostream exception::what_ostream() {
   std::size_t chars_used = LOFTY_COUNTOF(what_buf) - 1 /*NUL*/ - what_buf_available;
   io::text::char_ptr_ostream ostream(what_buf + chars_used, &what_buf_available);
   // If the what() string has already been written to, add a separator before a new write.
   if (chars_used > 0) {
      ostream.write(LOFTY_SL("; "));
   }
   return _std::move(ostream);
}

/*static*/ void exception::write_with_scope_trace(
   io::text::ostream * dst /*= nullptr*/, _std::exception const * std_x /*= nullptr*/
) {
   if (!dst) {
      dst = io::text::stderr.get();
   }
   exception const * x;
   if (std_x) {
      // We have an std::exception: print its what() and check if it’s also a lofty::exception.
      dst->print(LOFTY_SL("Exception in PID:{} TID:{}"), this_process::id(), this_thread::id());
      if (auto coro_id =  this_coroutine::id()) {
         dst->print(LOFTY_SL(" CRID:{}"), coro_id);
      }
      dst->print(LOFTY_SL("\n{}: {}\n"), typeid(*std_x), text::char_ptr_to_str_adapter(std_x->what()));
      x = dynamic_cast<exception const *>(std_x);
   } else {
      x = nullptr;
   }

   dst->write(LOFTY_SL("Stack trace (most recent call first):\n"));
   if (x) {
      // Frame 0 is the location of the LOFTY_THROW() statement.
      dst->print(
         LOFTY_SL("#0 {} at {}\n"),
         text::str(external_buffer, x->source_file_addr.function()), x->source_file_addr.file_address()
      );
   }
   // Write the scope/stack trace collected via LOFTY_TRACE_*().
   dst->write(logging::_pvt::scope_trace::get_trace_ostream()->get_str());
   // Append any scope_trace instances that haven’t been destructed yet.
   logging::_pvt::scope_trace::write_list(dst);
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*explicit*/ generic_error::generic_error(errint_t err_ /*= 0*/) :
   err(err_) {
   if (err) {
      what_ostream().print(LOFTY_SL("OS error={}"), err);
   }
}

generic_error::generic_error(generic_error const & src) :
   exception(src),
   err(src.err) {
}

/*virtual*/ generic_error::~generic_error() LOFTY_STL_NOEXCEPT_TRUE() {
}

generic_error & generic_error::operator=(generic_error const & src) {
   exception::operator=(src);
   err = src.err;
   return *this;
}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*explicit*/ network_error::network_error(errint_t err_ /*= 0*/) :
   generic_error(err_) {
}

network_error::network_error(network_error const & src) :
   generic_error(src) {
}

/*virtual*/ network_error::~network_error() LOFTY_STL_NOEXCEPT_TRUE() {
}

network_error & network_error::operator=(network_error const & src) {
   generic_error::operator=(src);
   return *this;
}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

process_exit::process_exit() {
}

process_exit::process_exit(process_exit const & src) :
   execution_interruption(src) {
}

/*virtual*/ process_exit::~process_exit() LOFTY_STL_NOEXCEPT_TRUE() {
}

process_exit & process_exit::operator=(process_exit const & src) {
   execution_interruption::operator=(src);
   return *this;
}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

process_interruption::process_interruption() {
}

process_interruption::process_interruption(process_interruption const & src) :
   execution_interruption(src) {
}

/*virtual*/ process_interruption::~process_interruption() LOFTY_STL_NOEXCEPT_TRUE() {
}

process_interruption & process_interruption::operator=(process_interruption const & src) {
   execution_interruption::operator=(src);
   return *this;
}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*explicit*/ security_error::security_error(errint_t err_ /*= 0*/) :
   generic_error(err_) {
}

security_error::security_error(security_error const & src) :
   generic_error(src) {
}

/*virtual*/ security_error::~security_error() LOFTY_STL_NOEXCEPT_TRUE() {
}

security_error & security_error::operator=(security_error const & src) {
   generic_error::operator=(src);
   return *this;
}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

user_forced_interruption::user_forced_interruption() {
}

user_forced_interruption::user_forced_interruption(user_forced_interruption const & src) :
   process_interruption(src) {
}

/*virtual*/ user_forced_interruption::~user_forced_interruption() LOFTY_STL_NOEXCEPT_TRUE() {
}

user_forced_interruption & user_forced_interruption::operator=(user_forced_interruption const & src) {
   process_interruption::operator=(src);
   return *this;
}

}
