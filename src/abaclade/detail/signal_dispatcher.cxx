/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014, 2015
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
#include "signal_dispatcher.hxx"
#include "../thread-impl.hxx"


////////////////////////////////////////////////////////////////////////////////////////////////////

#if ABC_HOST_API_MACH
   #include <cstdlib> // std::abort()
   // TODO: use Mach threads instead of pthreads for the exception-catching thread.
   #include <pthread.h>

   // Mach reference: <http://web.mit.edu/darwin/src/modules/xnu/osfmk/man/>.
   #include <mach/mach.h>
   #include <mach/mach_traps.h>


   /*! Handles a kernel-reported thread exception. This is exposed by Mach, but for some reason not
   declared in any system headers. See <http://web.mit.edu/darwin/src/modules/xnu/osfmk/man/
   exc_server.html>.

   @param pmsghRequest
      Pointer to the request message.
   @param pmsghReply
      Pointer to the message that will receive the reply.
   @return
      true if the message was handled and catch_exception_raise() (defined in this file) was called,
      or false if the message had nothing to do with exceptions.
   */
   extern "C" ::boolean_t exc_server(
      ::mach_msg_header_t * pmsghRequest, ::mach_msg_header_t * pmsghReply
   );

   /*! Called by exc_server() when the latter is passed an exception message, giving the process a
   way to do something about it. What we do is change the next instruction in the faulting thread to
   exception::throw_common_type().

   @param mpExceptions
      ?
   @param mpThread
      Faulting thread.
   @param mpTask
      ?
   @param exctype
      Type of exception.
   @param piExcCodes
      Pointer to an array of machine-dependent exception codes (sub-types).
   @param cExcCodes
      Count of elements in the array pointed to by piExcCodes.
   @return
      One of the KERN_* constants.
   */
   extern "C" ::kern_return_t ABACLADE_SYM catch_exception_raise(
      ::mach_port_t mpExceptions, ::mach_port_t mpThread, ::mach_port_t mpTask,
      ::exception_type_t exctype, ::exception_data_t piExcCodes, ::mach_msg_type_number_t cExcCodes
   ) {
      ABC_UNUSED_ARG(mpExceptions);
      ABC_UNUSED_ARG(mpTask);

   #if ABC_HOST_ARCH_X86_64
      typedef ::x86_exception_state64_t arch_exception_state_t;
      typedef ::x86_thread_state64_t    arch_thread_state_t;
      static ::thread_state_flavor_t const sc_tsfException = x86_EXCEPTION_STATE64;
      static ::thread_state_flavor_t const sc_tsfThread    = x86_THREAD_STATE64;
      static ::mach_msg_type_number_t const sc_cExceptionStateWords = x86_EXCEPTION_STATE64_COUNT;
      static ::mach_msg_type_number_t const sc_cThreadStateWords    = x86_THREAD_STATE64_COUNT;
   #else
      #error "TODO: HOST_ARCH"
   #endif

      // Read the exception and convert it into a known C++ type.
      abc::exception::common_type::enum_type xct;
      std::intptr_t iArg0 = 0, iArg1 = 0;
      {
         arch_exception_state_t excst;
         // On input this is a word count, but on output it’s an element count.
         ::mach_msg_type_number_t iExceptionStatesCount = sc_cExceptionStateWords;
         if (::thread_get_state(
            mpThread, sc_tsfException, reinterpret_cast< ::thread_state_t>(&excst),
            &iExceptionStatesCount
         ) != KERN_SUCCESS) {
            return KERN_FAILURE;
         }

         switch (exctype) {
            case EXC_BAD_ACCESS:
   #if ABC_HOST_ARCH_X86_64
               iArg0 = static_cast<std::intptr_t>(excst.__faultvaddr);
   #else
      #error "TODO: HOST_ARCH"
   #endif
               if (iArg0 == 0) {
                  xct = abc::exception::common_type::null_pointer_error;
               } else {
                  xct = abc::exception::common_type::memory_address_error;
               }
               break;

            case EXC_BAD_INSTRUCTION:
   #if ABC_HOST_ARCH_X86_64
               iArg0 = static_cast<std::intptr_t>(excst.__faultvaddr);
   #else
      #error "TODO: HOST_ARCH"
   #endif
               // TODO: use a better exception class.
               xct = abc::exception::common_type::memory_access_error;
               break;

            case EXC_ARITHMETIC:
               xct = abc::exception::common_type::arithmetic_error;
               if (cExcCodes) {
                  // TODO: can there be more than one exception code passed to a single call?
                  switch (piExcCodes[0]) {
   #if ABC_HOST_ARCH_X86_64
                     case EXC_I386_DIV:
                        xct = abc::exception::common_type::division_by_zero_error;
                        break;
/*
                     case EXC_I386_INTO:
                        xct = abc::exception::common_type::arithmetic_error;
                        break;
                     case EXC_I386_NOEXT:
                        xct = abc::exception::common_type::arithmetic_error;
                        break;
                     case EXC_I386_EXTOVR:
                        xct = abc::exception::common_type::arithmetic_error;
                        break;
                     case EXC_I386_EXTERR:
                        xct = abc::exception::common_type::arithmetic_error;
                        break;
                     case EXC_I386_EMERR:
                        xct = abc::exception::common_type::arithmetic_error;
                        break;
                     case EXC_I386_BOUND:
                        xct = abc::exception::common_type::arithmetic_error;
                        break;
                     case EXC_I386_SSEEXTERR:
                        xct = abc::exception::common_type::arithmetic_error;
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

      /* Change the address at which mpThread is executing: manipulate the thread state to emulate a
      function call to exception::throw_common_type(). */

      // Obtain the faulting thread’s state.
      arch_thread_state_t thrst;
      // On input this is a word count, but on output it’s an element count.
      ::mach_msg_type_number_t iThreadStatesCount = sc_cThreadStateWords;
      if (::thread_get_state(
         mpThread, sc_tsfThread, reinterpret_cast< ::thread_state_t>(&thrst), &iThreadStatesCount
      ) != KERN_SUCCESS) {
         return KERN_FAILURE;
      }

      // Manipulate the thread state to emulate a call to exception::throw_common_type().
      abc::exception::inject_in_context(xct, iArg0, iArg1, &thrst);

      // Update the faulting thread’s state.
      if (::thread_set_state(
         mpThread, sc_tsfThread, reinterpret_cast< ::thread_state_t>(&thrst), iThreadStatesCount
      ) != KERN_SUCCESS) {
         return KERN_FAILURE;
      }
      return KERN_SUCCESS;
   }
#elif ABC_HOST_API_POSIX
   #include <cstdlib> // std::abort()
   #include <ucontext.h> // ucontext_t
#endif

namespace abc { namespace detail {

#if ABC_HOST_API_POSIX
int const signal_dispatcher::smc_aiHandledSignals[] = {
   SIGBUS,  // Bus error (bad memory access) (POSIX.1-2001).
   SIGFPE,  // Floating point exception (POSIX.1-1990).
// SIGILL,  // Illegal Instruction (POSIX.1-1990).
   SIGSEGV  // Invalid memory reference (POSIX.1-1990).
};
#endif
signal_dispatcher * signal_dispatcher::sm_pInst = nullptr;

signal_dispatcher::signal_dispatcher() :
#if ABC_HOST_API_POSIX
   mc_iInterruptionSignal(
   #if ABC_HOST_API_DARWIN
      // SIGRT* not available.
      SIGUSR1
   #else
      SIGRTMIN + 1
   #endif
   ) {
#elif ABC_HOST_API_WIN32
   // Install the translator of Win32 structured exceptions into C++ exceptions.
   m_setfDefault(::_set_se_translator(&fault_se_translator)) {
#endif
   sm_pInst = this;
#if ABC_HOST_API_MACH
   ::mach_port_t mpThisProc = ::mach_task_self();
   // Allocate a right-less port to listen for exceptions.
   if (::mach_port_allocate(mpThisProc, MACH_PORT_RIGHT_RECEIVE, &m_mpExceptions) == KERN_SUCCESS) {
      // Assign rights to the port.
      if (::mach_port_insert_right(
         mpThisProc, m_mpExceptions, m_mpExceptions, MACH_MSG_TYPE_MAKE_SEND
      ) == KERN_SUCCESS) {
         // Start the thread that will catch exceptions from all the others.
         if (::pthread_create(&m_thrExcHandler, nullptr, exception_handler_thread, this) == 0) {
            // Now that the handler thread is running, set the process-wide exception port.
            if (::task_set_exception_ports(
               mpThisProc, EXC_MASK_BAD_ACCESS | EXC_MASK_BAD_INSTRUCTION | EXC_MASK_ARITHMETIC,
               m_mpExceptions, EXCEPTION_DEFAULT, MACHINE_THREAD_STATE
            ) == KERN_SUCCESS) {
               // All good.
            }
         }
      }
   }
#elif ABC_HOST_API_POSIX
   struct ::sigaction sa;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = SA_SIGINFO;
   // Setup interruption signal handlers.
   sa.sa_sigaction = &thread::impl::interruption_signal_handler;
   ::sigaction(mc_iInterruptionSignal, &sa, nullptr);
   ::sigaction(SIGINT,                 &sa, nullptr);
   ::sigaction(SIGTERM,                &sa, nullptr);
   // Setup fault signal handlers.
   sa.sa_sigaction = &fault_signal_handler;
   ABC_FOR_EACH(int iSignal, smc_aiHandledSignals) {
      ::sigaction(iSignal, &sa, nullptr);
   }
#endif
}

signal_dispatcher::~signal_dispatcher() {
#if ABC_HOST_API_MACH
   // TODO: stop m_thrExcHandler.
#elif ABC_HOST_API_POSIX
   // Restore the default signal handlers.
   ABC_FOR_EACH(int iSignal, smc_aiHandledSignals) {
      ::signal(iSignal, SIG_DFL);
   }
   ::signal(SIGINT,                 SIG_DFL);
   ::signal(SIGTERM,                SIG_DFL);
   ::signal(mc_iInterruptionSignal, SIG_DFL);
#elif ABC_HOST_API_WIN32
   ::_set_se_translator(m_setfDefault);
#endif
   sm_pInst = nullptr;
}

#if ABC_HOST_API_MACH

   /*static*/ void * signal_dispatcher::exception_handler_thread(void * p) {
      signal_dispatcher * pesdThis = static_cast<signal_dispatcher *>(p);
      for (;;) {
         /* The exact definition of these structs is in the kernel’s sources; thankfully all we need
         to do with them is pass them around, so just define them as BLOBs and hope that they’re
         large enough. */
         struct {
            ::mach_msg_header_t msgh;
            ::mach_msg_body_t msgb;
            std::uint8_t abData[1024];
         } msg;
         struct {
            ::mach_msg_header_t msgh;
            std::uint8_t abData[1024];
         } reply;

         // Block to read from the exception port.
         if (::mach_msg(
            &msg.msgh, MACH_RCV_MSG | MACH_RCV_LARGE, 0, sizeof msg, pesdThis->m_mpExceptions,
            MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL
         ) != MACH_MSG_SUCCESS) {
            std::abort();
         }

         // Handle the received message by having exc_server() call our catch_exception_raise().
         if (::exc_server(&msg.msgh, &reply.msgh)) {
            // exc_server() created a reply for the message, send it.
            if (::mach_msg(
               &reply.msgh, MACH_SEND_MSG, reply.msgh.msgh_size, 0, MACH_PORT_NULL,
               MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL
            ) != MACH_MSG_SUCCESS) {
               std::abort();
            }
         }
      }
   }

#elif ABC_HOST_API_POSIX

   /*static*/ void signal_dispatcher::fault_signal_handler(
      int iSignal, ::siginfo_t * psi, void * pctx
   ) {
      /* Don’t let external programs mess with us: if the source is not the kernel, ignore the
      error. POSIX.1-2008 states that:

         “Historically, an si_code value of less than or equal to zero indicated that the signal was
         generated by a process via the kill() function, and values of si_code that provided
         additional information for implementation-generated signals, such as SIGFPE or SIGSEGV,
         were all positive. […] if si_code is less than or equal to zero, the signal was generated
         by a process. However, since POSIX.1b did not specify that SI_USER (or SI_QUEUE) had a
         value less than or equal to zero, it is not true that when the signal is generated by a
         process, the value of si_code will always be less than or equal to zero. XSI applications
         should check whether si_code is SI_USER or SI_QUEUE in addition to checking whether it is
         less than or equal to zero.”

      So we do exactly that – except we skip checking for SI_USER and SI_QUEUE at this point because
      they don’t apply to many signals this handler takes care of. */
      if (psi->si_code <= 0) {
         return;
      }

      exception::common_type::enum_type xct = exception::common_type::none;
      std::intptr_t iArg0 = 0, iArg1 = 0;
      switch (iSignal) {
         case SIGBUS:
            /* There aren’t many codes here that are safe to handle; most of them indicate that
            there is some major memory corruption going on, and in that case we really don’t want to
            keep on going – even the code to throw an exception could be compromised. */
            switch (psi->si_code) {
               case BUS_ADRALN: // Invalid address alignment.
                  xct = exception::common_type::memory_access_error;
                  iArg0 = reinterpret_cast<std::intptr_t>(psi->si_addr);
                  break;
            }
            break;

         case SIGFPE:
            switch (psi->si_code) {
               case FPE_INTDIV: // Integer divide by zero.
                  xct = exception::common_type::division_by_zero_error;
                  break;
               case FPE_INTOVF: // Integer overflow.
                  xct = exception::common_type::overflow_error;
                  break;
               case FPE_FLTDIV: // Floating-point divide by zero.
               case FPE_FLTOVF: // Floating-point overflow.
               case FPE_FLTUND: // Floating-point underflow.
               case FPE_FLTRES: // Floating-point inexact result.
               case FPE_FLTINV: // Floating-point invalid operation.
               case FPE_FLTSUB: // Subscript out of range.
                  xct = exception::common_type::floating_point_error;
                  break;
               default:
                  /* At the time of writing, the above case labels don’t leave out any values, but
                  that’s not necessarily going to be true in 5 years, so… */
                  xct = exception::common_type::arithmetic_error;
                  break;
            }
            break;

         case SIGSEGV:
            if (psi->si_addr == nullptr) {
               xct = exception::common_type::null_pointer_error;
            } else {
               xct = exception::common_type::memory_address_error;
               iArg0 = reinterpret_cast<std::intptr_t>(psi->si_addr);
            }
            break;
      }
      if (xct != exception::common_type::none) {
         // Inject the selected exception type in the faulting thread.
         exception::inject_in_context(xct, iArg0, iArg1, pctx);
      } else {
         // Deal with cases not covered above.
         std::abort();
      }
   }

#elif ABC_HOST_API_WIN32

   /*static*/ ::BOOL WINAPI signal_dispatcher::console_ctrl_event_translator(::DWORD iCtrlEvent) {
      switch (iCtrlEvent) {
         case CTRL_C_EVENT:
            break;
         case CTRL_BREAK_EVENT:
            break;
         case CTRL_CLOSE_EVENT:
            break;
         case CTRL_LOGOFF_EVENT:
            break;
         case CTRL_SHUTDOWN_EVENT:
            break;
      }
      return true;
   }

   /*static*/ void ABC_STL_CALLCONV signal_dispatcher::fault_se_translator(
      unsigned iCode, ::_EXCEPTION_POINTERS * pxpInfo
   ) {
      switch (iCode) {
         case EXCEPTION_ACCESS_VIOLATION: {
            /* Attempt to read from or write to an inaccessible address.
            ExceptionInformation[0] contains a read-write flag that indicates the type of operation
            that caused the access violation. If this value is zero, the thread attempted to read
            the inaccessible data. If this value is 1, the thread attempted to write to an
            inaccessible address. If this value is 8, the thread caused a user-mode data execution
            prevention (DEP) violation.
            ExceptionInformation[1] specifies the virtual address of the inaccessible data. */
            void const * pAddr = reinterpret_cast<void const *>(
               pxpInfo->ExceptionRecord->ExceptionInformation[1]
            );
            if (pAddr == nullptr) {
               exception::throw_common_type(exception::common_type::null_pointer_error, 0, 0);
            } else {
               exception::throw_common_type(
                  exception::common_type::memory_address_error,
                  reinterpret_cast<std::intptr_t>(pAddr), 0
               );
            }
         }

//       case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            /* Attempt to access an array element that is out of bounds, and the underlying hardware
            supports bounds checking. */
//          break;

         case EXCEPTION_DATATYPE_MISALIGNMENT:
            // Attempt to read or write data that is misaligned on hardware that requires alignment.
            exception::throw_common_type(
               exception::common_type::memory_access_error,
               reinterpret_cast<std::intptr_t>(nullptr), 0
            );

         case EXCEPTION_FLT_DENORMAL_OPERAND:
            /* An operand in a floating-point operation is too small to represent as a standard
            floating-point value. */
            // Fall through.
         case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            // Attempt to divide a floating-point value by a floating-point divisor of zero.
            // Fall through.
         case EXCEPTION_FLT_INEXACT_RESULT:
            /* The result of a floating-point operation cannot be represented exactly as a decimal
            fraction. */
            // Fall through.
         case EXCEPTION_FLT_INVALID_OPERATION:
            // Other floating-point exception.
            // Fall through.
         case EXCEPTION_FLT_OVERFLOW:
            /* The exponent of a floating-point operation is greater than the magnitude allowed by
            the corresponding type. */
            // Fall through.
         case EXCEPTION_FLT_STACK_CHECK:
            // The stack overflowed or underflowed as a result of a floating-point operation.
            // Fall through.
         case EXCEPTION_FLT_UNDERFLOW:
            /* The exponent of a floating-point operation is less than the magnitude allowed by the
            corresponding type. */
            exception::throw_common_type(exception::common_type::floating_point_error, 0, 0);

         case EXCEPTION_ILLEGAL_INSTRUCTION:
            // Attempt to execute an invalid instruction.
            break;

         case EXCEPTION_IN_PAGE_ERROR:
            /* Attempt to access a page that was not present, and the system was unable to load the
            page. For example, this exception might occur if a network connection is lost while
            running a program over the network. */
            break;

         case EXCEPTION_INT_DIVIDE_BY_ZERO:
            // The thread attempted to divide an integer value by an integer divisor of zero.
            exception::throw_common_type(exception::common_type::division_by_zero_error, 0, 0);

         case EXCEPTION_INT_OVERFLOW:
            /* The result of an integer operation caused a carry out of the most significant bit of
            the result. */
            exception::throw_common_type(exception::common_type::overflow_error, 0, 0);

         case EXCEPTION_PRIV_INSTRUCTION:
            /* Attempt to execute an instruction whose operation is not allowed in the current
            machine mode. */
            break;

         case EXCEPTION_STACK_OVERFLOW:
            // The thread used up its stack.
            break;
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
   m_pthrimplMain = std::make_shared<thread::impl>(nullptr);
}

void signal_dispatcher::main_thread_terminated(exception::common_type xct) {
   /* Note: at this point, a correct program should have no other threads running. As a courtesy,
   Abaclade will prevent the process from terminating while threads are still running, by ensuring
   that all Abaclade-managed threads are joined before termination; however, app::main() returning
   when m_hmThreads.size() > 0 should be considered an exception (and a bug) rather than the
   rule. */

   // Make this thread uninterruptible by other threads.
   m_pthrimplMain->m_bTerminating.store(true);

   std::unique_lock<std::mutex> lock(m_mtxThreads);
   // Signal every other thread to terminate.
   ABC_FOR_EACH(auto kv, m_hmThreads) {
      kv.value->inject_exception(xct);
   }
   /* Wait for all threads to terminate; as they do, they’ll invoke nonmain_thread_terminated() and
   have themselves removed from m_hmThreads. We can’t join() them here, since they might be joining
   amongst themselves in some application-defined order, and we can’t join the same thread more than
   once (at least in POSIX). */
   while (!m_hmThreads.empty()) {
      lock.unlock();
      // Yes, we just sleep. Remember, this should not really happen (see the note above).
      this_thread::sleep_for_ms(1);
      // Re-lock the mutex and check again.
      lock.lock();
   }
}

void signal_dispatcher::nonmain_thread_started(std::shared_ptr<thread::impl> const & pthrimpl) {
   std::lock_guard<std::mutex> lock(m_mtxThreads);
   m_hmThreads.add_or_assign(pthrimpl.get(), pthrimpl);
}

void signal_dispatcher::nonmain_thread_terminated(
   thread::impl * pthrimpl, bool bUncaughtException
) {
   // Remove the thread from the bookkeeping list.
   {
      std::lock_guard<std::mutex> lock(m_mtxThreads);
      m_hmThreads.remove(pthrimpl);
   }
   /* If the thread was terminated by an exception making it all the way out of the thread function,
   all other threads must terminate as well. Achieve this by “forwarding” the exception to the main
   thread, so that its termination will in turn cause the termination of all other threads. */
   if (bUncaughtException) {
      /* TODO: use a more specific exception subclass of execution_interruption, such as
      “other_thread_execution_interrupted”. */
      m_pthrimplMain->inject_exception(exception::common_type::execution_interruption);
   }
}

}} //namespace abc::detail
