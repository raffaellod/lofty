/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include "signal_dispatcher.hxx"
#include "../thread-impl.hxx"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_API_MACH
   #include <cstdlib> // std::abort()
   // TODO: use Mach threads instead of pthreads for the exception-catching thread.
   #include <pthread.h>

   // Mach reference: <http://web.mit.edu/darwin/src/modules/xnu/osfmk/man/>.
   #include <mach/mach.h>
   #include <mach/mach_traps.h>


   /*! Handles a kernel-reported thread exception. This is exposed by Mach, but for some reason not declared
   in any system headers. See <http://web.mit.edu/darwin/src/modules/xnu/osfmk/man/exc_server.html>.

   @param request_msg_header
      Pointer to the request message.
   @param reply_msg_header
      Pointer to the message that will receive the reply.
   @return
      true if the message was handled and catch_exception_raise() (defined in this file) was called, or false
      if the message had nothing to do with exceptions.
   */
   extern "C" ::boolean_t exc_server(
      ::mach_msg_header_t * request_msg_header, ::mach_msg_header_t * reply_msg_header
   );

   /*! Called by exc_server() when the latter is passed an exception message, giving the process a way to do
   something about it. What we do is change the next instruction in the faulting thread to
   exception::throw_common_type().

   @param exceptions_port
      ?
   @param thread_port
      Faulting thread.
   @param task_port
      ?
   @param exc_type
      Type of exception.
   @param exc_codes
      Pointer to an array of machine-dependent exception codes (sub-types).
   @param exc_codes_size
      Count of elements in the array pointed to by exc_codes.
   @return
      One of the KERN_* constants.
   */
   extern "C" ::kern_return_t LOFTY_SYM catch_exception_raise(
      ::mach_port_t exceptions_port, ::mach_port_t thread_port, ::mach_port_t task_port,
      ::exception_type_t exc_type, ::exception_data_t exc_codes, ::mach_msg_type_number_t exc_codes_size
   ) {
      LOFTY_UNUSED_ARG(exceptions_port);
      LOFTY_UNUSED_ARG(task_port);

   #if LOFTY_HOST_ARCH_X86_64
      typedef ::x86_exception_state64_t arch_exception_state_t;
      typedef ::x86_thread_state64_t    arch_thread_state_t;
      #ifdef EXCEPTION_STATE
         #undef EXCEPTION_STATE
      #endif
      static ::thread_state_flavor_t const EXCEPTION_STATE = x86_EXCEPTION_STATE64;
      static ::thread_state_flavor_t const THREAD_STATE    = x86_THREAD_STATE64;
      static ::mach_msg_type_number_t const EXCEPTION_STATE_COUNT = x86_EXCEPTION_STATE64_COUNT;
      static ::mach_msg_type_number_t const THREAD_STATE_COUNT    = x86_THREAD_STATE64_COUNT;
   #else
      #error "TODO: HOST_ARCH"
   #endif

      // Read the exception and convert it into a known C++ type.
      lofty::exception::common_type::enum_type x_type;
      std::intptr_t arg0 = 0, arg1 = 0;
      {
         arch_exception_state_t exc_state;
         // On input this is a word count, but on output it’s an element count.
         ::mach_msg_type_number_t exception_states_size = EXCEPTION_STATE_COUNT;
         if (::thread_get_state(
            thread_port, EXCEPTION_STATE, reinterpret_cast< ::thread_state_t>(&exc_state),
            &exception_states_size
         ) != KERN_SUCCESS) {
            return KERN_FAILURE;
         }

         switch (exc_type) {
            case EXC_BAD_ACCESS:
               x_type = lofty::exception::common_type::memory_bad_pointer;
   #if LOFTY_HOST_ARCH_X86_64
               arg0 = static_cast<std::intptr_t>(exc_state.__faultvaddr);
   #else
      #error "TODO: HOST_ARCH"
   #endif
               break;

            case EXC_BAD_INSTRUCTION:
               // TODO: use a better exception class.
               x_type = lofty::exception::common_type::memory_bad_pointer_alignment;
   #if LOFTY_HOST_ARCH_X86_64
               arg0 = static_cast<std::intptr_t>(exc_state.__faultvaddr);
   #else
      #error "TODO: HOST_ARCH"
   #endif
               break;

            case EXC_ARITHMETIC:
               x_type = lofty::exception::common_type::math_arithmetic_error;
               if (exc_codes_size) {
                  // TODO: can there be more than one exception code passed to a single call?
                  switch (exc_codes[0]) {
   #if LOFTY_HOST_ARCH_X86_64
                     case EXC_I386_DIV:
                        x_type = lofty::exception::common_type::math_division_by_zero;
                        break;
/*
                     case EXC_I386_INTO:
                        x_type = lofty::exception::common_type::math_arithmetic_error;
                        break;
                     case EXC_I386_NOEXT:
                        x_type = lofty::exception::common_type::math_arithmetic_error;
                        break;
                     case EXC_I386_EXTOVR:
                        x_type = lofty::exception::common_type::math_arithmetic_error;
                        break;
                     case EXC_I386_EXTERR:
                        x_type = lofty::exception::common_type::math_arithmetic_error;
                        break;
                     case EXC_I386_EMERR:
                        x_type = lofty::exception::common_type::math_arithmetic_error;
                        break;
                     case EXC_I386_BOUND:
                        x_type = lofty::exception::common_type::math_arithmetic_error;
                        break;
                     case EXC_I386_SSEEXTERR:
                        x_type = lofty::exception::common_type::math_arithmetic_error;
                        break;
*/
   #else
      #error "TODO: HOST_ARCH"
   #endif
                  }
               }
               break;

            default:
               // Should never happen.
               return KERN_FAILURE;
         }
      }

      /* Change the address at which thread_port is executing: manipulate the thread state to emulate a
      function call to exception::throw_common_type(). */

      // Obtain the faulting thread’s state.
      arch_thread_state_t thread_state;
      // On input this is a word count, but on output it’s an element count.
      ::mach_msg_type_number_t thread_states_size = THREAD_STATE_COUNT;
      if (::thread_get_state(
         thread_port, THREAD_STATE, reinterpret_cast< ::thread_state_t>(&thread_state), &thread_states_size
      ) != KERN_SUCCESS) {
         return KERN_FAILURE;
      }

      // Manipulate the thread state to emulate a call to exception::throw_common_type().
      lofty::exception::inject_in_context(x_type, arg0, arg1, &thread_state);

      // Update the faulting thread’s state.
      if (::thread_set_state(
         thread_port, THREAD_STATE, reinterpret_cast< ::thread_state_t>(&thread_state), thread_states_size
      ) != KERN_SUCCESS) {
         return KERN_FAILURE;
      }
      return KERN_SUCCESS;
   }
#elif LOFTY_HOST_API_POSIX
   #include <cstdlib> // std::abort()
   #include <ucontext.h> // ucontext_t
#endif

namespace lofty { namespace _pvt {

#if LOFTY_HOST_API_POSIX
   #if !LOFTY_HOST_API_MACH
//! Fault signals that we can translate into C++ exceptions.
static int const fault_signals[] = {
  SIGBUS,  // Bus error (bad memory access) (POSIX.1-2001).
   SIGFPE,  // Floating point exception (POSIX.1-1990).
// SIGILL,  // Illegal Instruction (POSIX.1-1990).
   SIGSEGV  // Invalid memory reference (POSIX.1-1990).
};
   #endif
//! Signals that are redundant with errno values; we prefer errno to signals.
static int const ignored_signals[] = {
   SIGPIPE  // Broken pipe: write to pipe with no readers (POSIX.1-1990).
};
//! Interruption signals that we can translate into C++ exceptions.
static int const interruption_signals[] = {
   SIGINT,  // Interrupt from keyboard (POSIX.1-1990).
// SIGQUIT, // Quit from keyboard (POSIX.1-1990).
   SIGTERM  // Termination signal (POSIX.1-1990).
};
#endif

signal_dispatcher * signal_dispatcher::this_instance = nullptr;

signal_dispatcher::signal_dispatcher() :
#if LOFTY_HOST_API_POSIX
   thread_interruption_signal_(
   #if LOFTY_HOST_API_DARWIN
      // SIGRT* not available.
      SIGUSR1
   #else
      SIGRTMIN + 1
   #endif
   ) {
#elif LOFTY_HOST_API_WIN32
   // Install the translator of Win32 structured exceptions into C++ exceptions.
   default_se_translator_fn(::_set_se_translator(&fault_se_translator)) {
#endif
   this_instance = this;
#if LOFTY_HOST_API_POSIX
   // Block unwanted signals. These are not restored at the end; we really mean “unwanted”.
   LOFTY_FOR_EACH(int signal, ignored_signals) {
      ::signal(signal, SIG_IGN);
   }
   struct ::sigaction sa;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = SA_SIGINFO;
   // Setup interruption signal handlers.
   sa.sa_sigaction = &interruption_signal_handler;
   LOFTY_FOR_EACH(int signal, interruption_signals) {
      ::sigaction(signal, &sa, nullptr);
   }
   sa.sa_sigaction = &thread_interruption_signal_handler;
   ::sigaction(thread_interruption_signal_, &sa, nullptr);
#endif
#if LOFTY_HOST_API_MACH
   ::mach_port_t proc_port = ::mach_task_self();
   // Allocate a right-less port to listen for exceptions.
   if (::mach_port_allocate(proc_port, MACH_PORT_RIGHT_RECEIVE, &exceptions_port) == KERN_SUCCESS) {
      // Assign rights to the port.
      if (::mach_port_insert_right(
         proc_port, exceptions_port, exceptions_port, MACH_MSG_TYPE_MAKE_SEND
      ) == KERN_SUCCESS) {
         // Start the thread that will catch exceptions from all the others.
         if (::pthread_create(&exception_handler_thread, nullptr, &exception_handler, this) == 0) {
            // Now that the handler thread is running, set the process-wide exception port.
            if (::task_set_exception_ports(
               proc_port, EXC_MASK_BAD_ACCESS | EXC_MASK_BAD_INSTRUCTION | EXC_MASK_ARITHMETIC,
               exceptions_port, EXCEPTION_DEFAULT, MACHINE_THREAD_STATE
            ) == KERN_SUCCESS) {
               // All good.
            }
         }
      }
   }
#elif LOFTY_HOST_API_POSIX
   // Setup fault signal handlers.
   sa.sa_sigaction = &fault_signal_handler;
   LOFTY_FOR_EACH(int signal, fault_signals) {
      ::sigaction(signal, &sa, nullptr);
   }
#elif LOFTY_HOST_API_WIN32
   ::SetConsoleCtrlHandler(&console_ctrl_event_translator, true);
#endif
}

signal_dispatcher::~signal_dispatcher() {
#if LOFTY_HOST_API_POSIX
   // Restore the default signal handler for the interruption signals.
   ::signal(thread_interruption_signal_, SIG_DFL);
   LOFTY_FOR_EACH(int signal, interruption_signals) {
      ::signal(signal, SIG_DFL);
   }
   #if LOFTY_HOST_API_MACH
      // Tell exception_handler_thread to stop, then wait for it to do so.
      ::mach_msg_header_t header;
      header.msgh_bits = MACH_MSGH_BITS_SET_PORTS(MACH_MSG_TYPE_MAKE_SEND, 0, 0);
      header.msgh_remote_port = exceptions_port;
      header.msgh_local_port = MACH_PORT_NULL;
      header.msgh_size = sizeof header;
      if (::mach_msg(
         &header, MACH_SEND_MSG, sizeof header, 0 /*not receiving*/,
         MACH_PORT_NULL /*no reply*/, 10 /*ms timeout*/, MACH_PORT_NULL
      ) == MACH_MSG_SUCCESS) {
         ::pthread_join(exception_handler_thread, nullptr);
      }
   #else
      // Restore the default signal handler for the fault signals.
      LOFTY_FOR_EACH(int signal, fault_signals) {
         ::signal(signal, SIG_DFL);
      }
   #endif
#elif LOFTY_HOST_API_WIN32
   ::_set_se_translator(default_se_translator_fn);
#endif
   this_instance = nullptr;
}

#if LOFTY_HOST_API_POSIX
   /*static*/ void signal_dispatcher::interruption_signal_handler(int signal, ::siginfo_t * si, void * ctx) {
      LOFTY_UNUSED_ARG(si);
      LOFTY_UNUSED_ARG(ctx);

      exception::common_type::enum_type x_type;
      if (signal == SIGINT) {
         x_type = exception::common_type::user_forced_interruption;
      } else if (signal == SIGTERM) {
         x_type = exception::common_type::execution_interruption;
      } else {
         // Should never happen.
         std::abort();
      }
      this_instance->main_thread->inject_exception(x_type, false);
   }

   /*static*/ void signal_dispatcher::thread_interruption_signal_handler(
      int signal, ::siginfo_t * si, void * ctx
   ) {
      LOFTY_UNUSED_ARG(signal);
      LOFTY_UNUSED_ARG(si);
      LOFTY_UNUSED_ARG(ctx);
      // Nothing to do here.
   }
#endif

#if LOFTY_HOST_API_MACH
   /*static*/ void * signal_dispatcher::exception_handler(void * p) {
      signal_dispatcher * this_ptr = static_cast<signal_dispatcher *>(p);
      struct {
         ::mach_msg_header_t header;
         // Testing on x86-64 shows that an exception message has size=76, so this will be more than sufficient.
         std::uint8_t data[256];
      } msg, reply;
      for (;;) {
         // Block to read from the exception port.
         if (::mach_msg(
            &msg.header, MACH_RCV_MSG, 0 /*not sending*/, sizeof msg,
            this_ptr->exceptions_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL
         ) != MACH_MSG_SUCCESS) {
            // Ignore a receive failure.
            continue;
         }
         if (msg.header.msgh_remote_port == MACH_PORT_NULL) {
            // Termination message sent by the main thread.
            return nullptr;
         }
         // This will call our catch_exception_raise() and produce a reply for us to send.
         if (::exc_server(&msg.header, &reply.header)) {
            ::mach_msg(
               &reply.header, MACH_SEND_MSG, reply.header.msgh_size, 0 /*not receiving*/,
               MACH_PORT_NULL /*no reply*/, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL
            );
         }
      }
   }
#elif LOFTY_HOST_API_POSIX
   /*static*/ void signal_dispatcher::fault_signal_handler(int signal, ::siginfo_t * si, void * ctx) {
      /* Don’t let external programs mess with us: if the source is not the kernel, ignore the error.
      POSIX.1-2008 states that:

         “Historically, an si_code value of less than or equal to zero indicated that the signal was generated
         by a process via the kill() function, and values of si_code that provided additional information for
         implementation-generated signals, such as SIGFPE or SIGSEGV, were all positive. […] if si_code is
         less than or equal to zero, the signal was generated by a process. However, since POSIX.1b did not
         specify that SI_USER (or SI_QUEUE) had a value less than or equal to zero, it is not true that when
         the signal is generated by a process, the value of si_code will always be less than or equal to zero.
         XSI applications should check whether si_code is SI_USER or SI_QUEUE in addition to checking whether
         it is less than or equal to zero.”

      So we do exactly that – except we skip checking for SI_USER and SI_QUEUE at this point because they
      don’t apply to many signals this handler takes care of. */
      if (si->si_code <= 0) {
         return;
      }

      exception::common_type::enum_type x_type = exception::common_type::none;
      std::intptr_t arg0 = 0, arg1 = 0;
      switch (signal) {
         case SIGBUS:
            /* There aren’t many codes here that are safe to handle; most of them indicate that there is some
            major memory corruption going on, and in that case we really don’t want to keep on going – even
            the code to throw an exception could be compromised. */
            switch (si->si_code) {
               case BUS_ADRALN: // Invalid address alignment.
                  x_type = exception::common_type::memory_bad_pointer_alignment;
                  arg0 = reinterpret_cast<std::intptr_t>(si->si_addr);
                  break;
            }
            break;

         case SIGFPE:
            switch (si->si_code) {
               case FPE_INTDIV: // Integer divide by zero.
                  x_type = exception::common_type::math_division_by_zero;
                  break;
               case FPE_INTOVF: // Integer overflow.
                  x_type = exception::common_type::math_overflow;
                  break;
               case FPE_FLTDIV: // Floating-point divide by zero.
               case FPE_FLTOVF: // Floating-point overflow.
               case FPE_FLTUND: // Floating-point underflow.
               case FPE_FLTRES: // Floating-point inexact result.
               case FPE_FLTINV: // Floating-point invalid operation.
               case FPE_FLTSUB: // Subscript out of range.
                  x_type = exception::common_type::math_floating_point_error;
                  break;
               default:
                  /* At the time of writing, the above case labels don’t leave out any values, but that’s not
                  necessarily going to be true in 5 years, so… */
                  x_type = exception::common_type::math_arithmetic_error;
                  break;
            }
            break;

         case SIGSEGV:
            x_type = exception::common_type::memory_bad_pointer;
            arg0 = reinterpret_cast<std::intptr_t>(si->si_addr);
            break;
      }
      if (x_type != exception::common_type::none) {
         // Inject the selected exception type in the faulting thread.
         exception::inject_in_context(x_type, arg0, arg1, ctx);
      } else {
         // Deal with cases not covered above.
         std::abort();
      }
   }
#elif LOFTY_HOST_API_WIN32
   /*static*/ ::BOOL WINAPI signal_dispatcher::console_ctrl_event_translator(::DWORD ctrl_event) {
      exception::common_type::enum_type x_type;
      switch (ctrl_event) {
         case CTRL_BREAK_EVENT:
         case CTRL_C_EVENT:
         case CTRL_LOGOFF_EVENT:
         case CTRL_SHUTDOWN_EVENT:
            x_type = exception::common_type::user_forced_interruption;
            break;
         case CTRL_CLOSE_EVENT:
            // Clicking on the X is considered a normal way of terminating a process.
            x_type = exception::common_type::process_exit;
            break;
         default:
            return false;
      }
      this_instance->main_thread->inject_exception(x_type);
      return true;
   }

   /*static*/ void LOFTY_STL_CALLCONV signal_dispatcher::fault_se_translator(
      unsigned code, ::_EXCEPTION_POINTERS * sx_info
   ) {
      exception::common_type::enum_type x_type = exception::common_type::none;
      std::intptr_t arg0 = 0, arg1 = 0;
      switch (code) {
         case EXCEPTION_ACCESS_VIOLATION:
            /* Attempt to read from or write to an inaccessible address.
            ExceptionInformation[0] contains a read-write flag that indicates the type of operation that
            caused the access violation. If this value is zero, the thread attempted to read the inaccessible
            data. If this value is 1, the thread attempted to write to an inaccessible address. If this value
            is 8, the thread caused a user-mode data execution prevention (DEP) violation.
            ExceptionInformation[1] specifies the virtual address of the inaccessible data. */
            x_type = exception::common_type::memory_bad_pointer;
            arg0 = static_cast<std::intptr_t>(sx_info->ExceptionRecord->ExceptionInformation[1]);
            break;

//       case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            /* Attempt to access an array element that is out of bounds, and the underlying hardware supports
            bounds checking. */
//          break;

         case EXCEPTION_DATATYPE_MISALIGNMENT:
            // Attempt to read or write data that is misaligned on hardware that requires alignment.
            x_type = exception::common_type::memory_bad_pointer_alignment;
            arg0 = reinterpret_cast<std::intptr_t>(nullptr);
            break;

         case EXCEPTION_FLT_DENORMAL_OPERAND:
            /* An operand in a floating-point operation is too small to represent as a standard floating-point
            value. */
            // Fall through.
         case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            // Attempt to divide a floating-point value by a floating-point divisor of zero.
            // Fall through.
         case EXCEPTION_FLT_INEXACT_RESULT:
            // The result of a floating-point operation cannot be represented exactly as a decimal fraction.
            // Fall through.
         case EXCEPTION_FLT_INVALID_OPERATION:
            // Other floating-point exception.
            // Fall through.
         case EXCEPTION_FLT_OVERFLOW:
            /* The exponent of a floating-point operation is greater than the magnitude allowed by the
            corresponding type. */
            // Fall through.
         case EXCEPTION_FLT_STACK_CHECK:
            // The stack overflowed or underflowed as a result of a floating-point operation.
            // Fall through.
         case EXCEPTION_FLT_UNDERFLOW:
            /* The exponent of a floating-point operation is less than the magnitude allowed by the
            corresponding type. */
            x_type = exception::common_type::math_floating_point_error;
            break;

         case EXCEPTION_ILLEGAL_INSTRUCTION:
            // Attempt to execute an invalid instruction.
            break;

         case EXCEPTION_IN_PAGE_ERROR:
            /* Attempt to access a page that was not present, and the system was unable to load the page. For
            example, this exception might occur if a network connection is lost while running a program over
            the network. */
            break;

         case EXCEPTION_INT_DIVIDE_BY_ZERO:
            // The thread attempted to divide an integer value by an integer divisor of zero.
            x_type = exception::common_type::math_division_by_zero;
            break;

         case EXCEPTION_INT_OVERFLOW:
            /* The result of an integer operation caused a carry out of the most significant bit of the
            result. */
            x_type = exception::common_type::math_overflow;
            break;

         case EXCEPTION_PRIV_INSTRUCTION:
            // Attempt to execute an instruction whose operation is not allowed in the current machine mode.
            break;

         case EXCEPTION_STACK_OVERFLOW:
            // The thread used up its stack.
            break;
      }
      if (x_type != exception::common_type::none) {
         exception::throw_common_type(x_type, arg0, arg1);
      }
   }

   /*static*/ void signal_dispatcher::init_for_current_thread() {
      // Install the SEH translator, without saving the original.
      ::_set_se_translator(&fault_se_translator);
   }
#else
   #error "TODO: HOST_API"
#endif

void signal_dispatcher::main_thread_started() {
   main_thread = _std::make_shared<thread::impl>(nullptr);
}

void signal_dispatcher::main_thread_terminated(exception::common_type x_type) {
   /* Note: at this point, a correct program should have no other threads running. As a courtesy, Lofty will
   prevent the process from terminating while threads are still running, by ensuring that all Lofty-managed
   threads are joined before termination; however, app::main() returning when known_threads.size() > 0 should
   be considered an exception (and a bug) rather than the rule. */

   // Make this thread uninterruptible by other threads.
   main_thread->terminating_.store(true);

   _std::unique_lock<_std::mutex> lock(known_threads_mutex);
   // Signal every other thread to terminate.
   LOFTY_FOR_EACH(auto kv, known_threads) {
      kv.value->inject_exception(x_type);
   }
   /* Wait for all threads to terminate; as they do, they’ll invoke nonmain_thread_terminated() and have
   themselves removed from known_threads. We can’t join() them here, since they might be joining amongst
   themselves in some application-defined order, and we can’t join the same thread more than once (at least
   under POSIX). */
   while (known_threads) {
      lock.unlock();
      // Yes, we just sleep. Remember, this should not really happen (see the note above).
      this_thread::sleep_for_ms(1);
      // Re-lock the mutex and check again.
      lock.lock();
   }
}

void signal_dispatcher::nonmain_thread_started(_std::shared_ptr<thread::impl> const & thread_pimpl) {
   _std::lock_guard<_std::mutex> lock(known_threads_mutex);
   known_threads.add_or_assign(thread_pimpl.get(), thread_pimpl);
}

void signal_dispatcher::nonmain_thread_terminated(thread::impl * thread_pimpl, bool uncaught_exception) {
   // Remove the thread from the bookkeeping list.
   {
      _std::lock_guard<_std::mutex> lock(known_threads_mutex);
      known_threads.remove(thread_pimpl);
   }
   /* If the thread was terminated by an exception making it all the way out of the thread function, all other
   threads must terminate as well. Achieve this by “forwarding” the exception to the main thread, so that its
   termination will in turn cause the termination of all other threads. */
   if (uncaught_exception) {
      /* TODO: use a more specific exception subclass of execution_interruption, such as
      “other_thread_execution_interrupted”. */
      main_thread->inject_exception(exception::common_type::execution_interruption);
   }
}

}} //namespace lofty::_pvt
