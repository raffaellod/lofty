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
#include "exception-fault_converter.hxx"


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::exception::fault_converter

#if ABC_HOST_API_MACH
   #include <cstdlib> // std::abort()
   // TODO: use Mach threads instead of pthreads for the exception-catching thread.
   #include <pthread.h>

   // Mach reference: <http://web.mit.edu/darwin/src/modules/xnu/osfmk/man/>.
   #include <mach/mach.h> // boolean_t mach_msg_header_t
   #include <mach/mach_traps.h> // mach_task_self()


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
   throw_injected_exception().

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
      abc::exception::injectable::enum_type inj;
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
                  inj = abc::exception::injectable::null_pointer_error;
               } else {
                  inj = abc::exception::injectable::memory_address_error;
               }
               break;

            case EXC_BAD_INSTRUCTION:
   #if ABC_HOST_ARCH_X86_64
               iArg0 = static_cast<std::intptr_t>(excst.__faultvaddr);
   #else
      #error "TODO: HOST_ARCH"
   #endif
               // TODO: use a better exception class.
               inj = abc::exception::injectable::memory_access_error;
               break;

            case EXC_ARITHMETIC:
               inj = abc::exception::injectable::arithmetic_error;
               if (cExcCodes) {
                  // TODO: can there be more than one exception code passed to a single call?
                  switch (piExcCodes[0]) {
   #if ABC_HOST_ARCH_X86_64
                     case EXC_I386_DIV:
                        inj = abc::exception::injectable::division_by_zero_error;
                        break;
/*
                     case EXC_I386_INTO:
                        inj = abc::exception::injectable::arithmetic_error;
                        break;
                     case EXC_I386_NOEXT:
                        inj = abc::exception::injectable::arithmetic_error;
                        break;
                     case EXC_I386_EXTOVR:
                        inj = abc::exception::injectable::arithmetic_error;
                        break;
                     case EXC_I386_EXTERR:
                        inj = abc::exception::injectable::arithmetic_error;
                        break;
                     case EXC_I386_EMERR:
                        inj = abc::exception::injectable::arithmetic_error;
                        break;
                     case EXC_I386_BOUND:
                        inj = abc::exception::injectable::arithmetic_error;
                        break;
                     case EXC_I386_SSEEXTERR:
                        inj = abc::exception::injectable::arithmetic_error;
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
      function call to throw_injected_exception(). */

      // Obtain the faulting thread’s state.
      arch_thread_state_t thrst;
      // On input this is a word count, but on output it’s an element count.
      ::mach_msg_type_number_t iThreadStatesCount = sc_cThreadStateWords;
      if (::thread_get_state(
         mpThread, sc_tsfThread, reinterpret_cast< ::thread_state_t>(&thrst), &iThreadStatesCount
      ) != KERN_SUCCESS) {
         return KERN_FAILURE;
      }

      // Manipulate the thread state to emulate a call to throw_injected_exception().
      abc::exception::inject_in_context(inj, iArg0, iArg1, &thrst);

      // Update the faulting thread’s state.
      if (::thread_set_state(
         mpThread, sc_tsfThread, reinterpret_cast< ::thread_state_t>(&thrst), iThreadStatesCount
      ) != KERN_SUCCESS) {
         return KERN_FAILURE;
      }
      return KERN_SUCCESS;
   }

   namespace abc {

   ::pthread_t exception::fault_converter::sm_thrExcHandler;
   ::mach_port_t exception::fault_converter::sm_mpExceptions;

   exception::fault_converter::fault_converter() {
      ::mach_port_t mpThisProc = ::mach_task_self();
      // Allocate a right-less port to listen for exceptions.
      if (::mach_port_allocate(
         mpThisProc, MACH_PORT_RIGHT_RECEIVE, &sm_mpExceptions) == KERN_SUCCESS
      ) {
         // Assign rights to the port.
         if (::mach_port_insert_right(
            mpThisProc, sm_mpExceptions, sm_mpExceptions, MACH_MSG_TYPE_MAKE_SEND
         ) == KERN_SUCCESS) {
            // Start the thread that will catch exceptions from all the others.
            if (::pthread_create(
               &sm_thrExcHandler, nullptr, exception_handler_thread, nullptr
            ) == 0) {
               // Now that the handler thread is running, set the process-wide exception port.
               if (::task_set_exception_ports(
                  mpThisProc,
                  EXC_MASK_BAD_ACCESS | EXC_MASK_BAD_INSTRUCTION | EXC_MASK_ARITHMETIC,
                  sm_mpExceptions, EXCEPTION_DEFAULT, MACHINE_THREAD_STATE
               ) == KERN_SUCCESS) {
                  // All good.
               }
            }
         }
      }
   }

   exception::fault_converter::~fault_converter() {
   }

   /*static*/ void * exception::fault_converter::exception_handler_thread(void *) {
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
            &msg.msgh, MACH_RCV_MSG | MACH_RCV_LARGE, 0, sizeof msg, sm_mpExceptions,
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

   } //namespace abc

#elif ABC_HOST_API_POSIX
   #include <cstdlib> // std::abort()

   #include <ucontext.h> // ucontext_t


   namespace abc {

   int const exception::fault_converter::smc_aiHandledSignals[] = {
//    Signal (Default action) Description (standard).
//    SIGABRT, // (Core) Abort signal from abort(3) (POSIX.1-1990).
//    SIGALRM, // (Term) Timer signal from alarm(2) (POSIX.1-1990).
      SIGBUS,  // (Core) Bus error (bad memory access) (POSIX.1-2001).
//    SIGCHLD, // (Ign ) Child stopped or terminated (POSIX.1-1990).
//    SIGCONT, // (Cont) Continue if stopped (POSIX.1-1990).
      SIGFPE,  // (Core) Floating point exception (POSIX.1-1990).
/*    SIGHUP,  // (Term) Hangup on controlling terminal or death of controlling process
               // (POSIX.1-1990). */
//    SIGILL,  // (Core) Illegal Instruction (POSIX.1-1990).
//    SIGINT,  // (Term) Interrupt from keyboard (POSIX.1-1990).
//    SIGPIPE, // (Term) Broken pipe: write to pipe with no readers (POSIX.1-1990).
//    SIGPROF, // (Term) Profiling timer expired (POSIX.1-2001).
//    SIGQUIT, // (Core) Quit from keyboard (POSIX.1-1990).
      SIGSEGV  // (Core) Invalid memory reference (POSIX.1-1990).
//    SIGTERM  // (Term) Termination signal (POSIX.1-1990).
//    SIGTRAP  // (Core) Trace/breakpoint trap (POSIX.1-2001).
//    SIGTSTP  // (Stop) Stop typed at terminal (POSIX.1-1990).
//    SIGTTIN  // (Stop) Terminal input for background process (POSIX.1-1990).
//    SIGTTOU  // (Stop) Terminal output for background process (POSIX.1-1990).
//    SIGUSR1  // (Term) User-defined signal 1 (POSIX.1-1990).
//    SIGUSR2  // (Term) User-defined signal 2 (POSIX.1-1990).
   };
   struct ::sigaction exception::fault_converter::sm_asaDefault[ABC_COUNTOF(smc_aiHandledSignals)];

   exception::fault_converter::fault_converter() {
      // Setup handlers for the signals in smc_aiHandledSignals.
      struct ::sigaction saNew;
      saNew.sa_sigaction = &fault_signal_handler;
      sigemptyset(&saNew.sa_mask);
      /* SA_SIGINFO (POSIX.1-2001) provides the handler with more information about the signal,
      which we use to generate more precise exceptions. */
      saNew.sa_flags = SA_SIGINFO;
      for (std::size_t i = ABC_COUNTOF(smc_aiHandledSignals); i-- > 0; ) {
         ::sigaction(smc_aiHandledSignals[i], &saNew, &sm_asaDefault[i]);
      }
   }

   exception::fault_converter::~fault_converter() {
      // Restore the saved signal handlers.
      for (std::size_t i = ABC_COUNTOF(smc_aiHandledSignals); i-- > 0; ) {
         ::sigaction(smc_aiHandledSignals[i], &sm_asaDefault[i], nullptr);
      }
   }

   /*static*/ void exception::fault_converter::fault_signal_handler(
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

      injectable::enum_type inj;
      std::intptr_t iArg0 = 0, iArg1 = 0;
      switch (iSignal) {
         case SIGBUS:
            /* There aren’t many codes here that are safe to handle; most of them indicate that
            there is some major memory corruption going on, and in that case we really don’t want to
            keep on going – even the code to throw an exception could be compromised. */
            switch (psi->si_code) {
               case BUS_ADRALN: // Invalid address alignment.
                  inj = injectable::memory_access_error;
                  iArg0 = reinterpret_cast<std::intptr_t>(psi->si_addr);
                  break;
               default:
                  std::abort();
            }
            break;

         case SIGFPE:
            switch (psi->si_code) {
               case FPE_INTDIV: // Integer divide by zero.
                  inj = injectable::division_by_zero_error;
                  break;
               case FPE_INTOVF: // Integer overflow.
                  inj = injectable::overflow_error;
                  break;
               case FPE_FLTDIV: // Floating-point divide by zero.
               case FPE_FLTOVF: // Floating-point overflow.
               case FPE_FLTUND: // Floating-point underflow.
               case FPE_FLTRES: // Floating-point inexact result.
               case FPE_FLTINV: // Floating-point invalid operation.
               case FPE_FLTSUB: // Subscript out of range.
                  inj = injectable::floating_point_error;
                  break;
               default:
                  /* At the time of writing, the above case labels don’t leave out any values, but
                  that’s not necessarily going to be true in 5 years, so… */
                  inj = injectable::arithmetic_error;
                  break;
            }
            break;

         case SIGSEGV:
            if (psi->si_addr == nullptr) {
               inj = injectable::null_pointer_error;
            } else {
               inj = injectable::memory_address_error;
               iArg0 = reinterpret_cast<std::intptr_t>(psi->si_addr);
            }
            break;

         default:
            /* Handle all unrecognized cases here. Since here we only handle signals for which the
            default actions is a core dump, calling abort (which sends SIGABRT, also causing a core
            dump) is the same as invoking the default action. */
            std::abort();
      }

      /* Change the address at which the thread will resume execution: manipulate the thread context
      to emulate a function call to throw_injected_exception(). */
      inject_in_context(inj, iArg0, iArg1, pctx);
   }

   } //namespace abc

#elif ABC_HOST_API_WIN32

   namespace abc {

   ::_se_translator_function exception::fault_converter::sm_setfDefault;

   exception::fault_converter::fault_converter() {
      // Install the translator of Win32 structured exceptions into C++ exceptions.
      sm_setfDefault = ::_set_se_translator(&fault_se_translator);
   }

   exception::fault_converter::~fault_converter() {
      ::_set_se_translator(sm_setfDefault);
   }

   /*static*/ void exception::fault_converter::init_for_current_thread() {
      // Install the SEH translator, without saving the original.
      ::_set_se_translator(&fault_se_translator);
   }

   /*static*/ void ABC_STL_CALLCONV exception::fault_converter::fault_se_translator(
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
               throw_injected_exception(injectable::null_pointer_error, 0, 0);
            } else {
               throw_injected_exception(
                  injectable::memory_address_error, reinterpret_cast<std::intptr_t>(pAddr), 0
               );
            }
         }

//       case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            /* Attempt to access an array element that is out of bounds, and the underlying hardware
            supports bounds checking. */
//          break;

         case EXCEPTION_DATATYPE_MISALIGNMENT:
            // Attempt to read or write data that is misaligned on hardware that requires alignment.
            throw_injected_exception(
               injectable::memory_access_error, reinterpret_cast<std::intptr_t>(nullptr), 0
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
            throw_injected_exception(injectable::floating_point_error, 0, 0);

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
            throw_injected_exception(injectable::division_by_zero_error, 0, 0);

         case EXCEPTION_INT_OVERFLOW:
            /* The result of an integer operation caused a carry out of the most significant bit of
            the result. */
            throw_injected_exception(injectable::overflow_error, 0, 0);

         case EXCEPTION_PRIV_INSTRUCTION:
            /* Attempt to execute an instruction whose operation is not allowed in the current
            machine mode. */
            break;

         case EXCEPTION_STACK_OVERFLOW:
            // The thread used up its stack.
            break;
      }
   }

   } //namespace abc

#else
   #error "TODO: HOST_API"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
