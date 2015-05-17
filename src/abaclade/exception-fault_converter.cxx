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
   throw_common_type().

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
      function call to throw_common_type(). */

      // Obtain the faulting thread’s state.
      arch_thread_state_t thrst;
      // On input this is a word count, but on output it’s an element count.
      ::mach_msg_type_number_t iThreadStatesCount = sc_cThreadStateWords;
      if (::thread_get_state(
         mpThread, sc_tsfThread, reinterpret_cast< ::thread_state_t>(&thrst), &iThreadStatesCount
      ) != KERN_SUCCESS) {
         return KERN_FAILURE;
      }

      // Manipulate the thread state to emulate a call to throw_common_type().
      abc::exception::inject_in_context(xct, iArg0, iArg1, &thrst);

      // Update the faulting thread’s state.
      if (::thread_set_state(
         mpThread, sc_tsfThread, reinterpret_cast< ::thread_state_t>(&thrst), iThreadStatesCount
      ) != KERN_SUCCESS) {
         return KERN_FAILURE;
      }
      return KERN_SUCCESS;
   }

   namespace abc {

   exception::fault_converter::fault_converter() {
      ::mach_port_t mpThisProc = ::mach_task_self();
      // Allocate a right-less port to listen for exceptions.
      if (::mach_port_allocate(
         mpThisProc, MACH_PORT_RIGHT_RECEIVE, &m_mpExceptions) == KERN_SUCCESS
      ) {
         // Assign rights to the port.
         if (::mach_port_insert_right(
            mpThisProc, m_mpExceptions, m_mpExceptions, MACH_MSG_TYPE_MAKE_SEND
         ) == KERN_SUCCESS) {
            // Start the thread that will catch exceptions from all the others.
            if (::pthread_create(&m_thrExcHandler, nullptr, exception_handler_thread, this) == 0) {
               // Now that the handler thread is running, set the process-wide exception port.
               if (::task_set_exception_ports(
                  mpThisProc,
                  EXC_MASK_BAD_ACCESS | EXC_MASK_BAD_INSTRUCTION | EXC_MASK_ARITHMETIC,
                  m_mpExceptions, EXCEPTION_DEFAULT, MACHINE_THREAD_STATE
               ) == KERN_SUCCESS) {
                  // All good.
               }
            }
         }
      }
   }

   exception::fault_converter::~fault_converter() {
   }

   /*static*/ void * exception::fault_converter::exception_handler_thread(void * p) {
      fault_converter * pxfcThis = static_cast<fault_converter *>(p);
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
            &msg.msgh, MACH_RCV_MSG | MACH_RCV_LARGE, 0, sizeof msg, pxfcThis->m_mpExceptions,
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
      SIGBUS,  // Bus error (bad memory access) (POSIX.1-2001).
      SIGFPE,  // Floating point exception (POSIX.1-1990).
//    SIGILL,  // Illegal Instruction (POSIX.1-1990).
      SIGSEGV  // Invalid memory reference (POSIX.1-1990).
   };

   exception::fault_converter::fault_converter() {
      // Setup handlers for the signals in smc_aiHandledSignals.
      struct ::sigaction sa;
      sa.sa_sigaction = &fault_signal_handler;
      sigemptyset(&sa.sa_mask);
      sa.sa_flags = SA_SIGINFO;
      ABC_FOR_EACH(int iSignal, smc_aiHandledSignals) {
         ::sigaction(iSignal, &sa, nullptr);
      }
   }

   exception::fault_converter::~fault_converter() {
      // Restore the default signal handlers.
      ABC_FOR_EACH(int iSignal, smc_aiHandledSignals) {
         ::signal(iSignal, SIG_DFL);
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

      common_type::enum_type xct = common_type::none;
      std::intptr_t iArg0 = 0, iArg1 = 0;
      switch (iSignal) {
         case SIGBUS:
            /* There aren’t many codes here that are safe to handle; most of them indicate that
            there is some major memory corruption going on, and in that case we really don’t want to
            keep on going – even the code to throw an exception could be compromised. */
            switch (psi->si_code) {
               case BUS_ADRALN: // Invalid address alignment.
                  xct = common_type::memory_access_error;
                  iArg0 = reinterpret_cast<std::intptr_t>(psi->si_addr);
                  break;
            }
            break;

         case SIGFPE:
            switch (psi->si_code) {
               case FPE_INTDIV: // Integer divide by zero.
                  xct = common_type::division_by_zero_error;
                  break;
               case FPE_INTOVF: // Integer overflow.
                  xct = common_type::overflow_error;
                  break;
               case FPE_FLTDIV: // Floating-point divide by zero.
               case FPE_FLTOVF: // Floating-point overflow.
               case FPE_FLTUND: // Floating-point underflow.
               case FPE_FLTRES: // Floating-point inexact result.
               case FPE_FLTINV: // Floating-point invalid operation.
               case FPE_FLTSUB: // Subscript out of range.
                  xct = common_type::floating_point_error;
                  break;
               default:
                  /* At the time of writing, the above case labels don’t leave out any values, but
                  that’s not necessarily going to be true in 5 years, so… */
                  xct = common_type::arithmetic_error;
                  break;
            }
            break;

         case SIGSEGV:
            if (psi->si_addr == nullptr) {
               xct = common_type::null_pointer_error;
            } else {
               xct = common_type::memory_address_error;
               iArg0 = reinterpret_cast<std::intptr_t>(psi->si_addr);
            }
            break;
      }
      if (xct != common_type::none) {
         // Inject the selected exception type in the faulting thread.
         inject_in_context(xct, iArg0, iArg1, pctx);
      } else {
         // Deal with cases not covered above.
         std::abort();
      }
   }

   } //namespace abc

#elif ABC_HOST_API_WIN32

   namespace abc {

   exception::fault_converter::fault_converter() :
      // Install the translator of Win32 structured exceptions into C++ exceptions.
      m_setfDefault(::_set_se_translator(&fault_se_translator)) {
   }

   exception::fault_converter::~fault_converter() {
      ::_set_se_translator(m_setfDefault);
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
               throw_common_type(common_type::null_pointer_error, 0, 0);
            } else {
               throw_common_type(
                  common_type::memory_address_error, reinterpret_cast<std::intptr_t>(pAddr), 0
               );
            }
         }

//       case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            /* Attempt to access an array element that is out of bounds, and the underlying hardware
            supports bounds checking. */
//          break;

         case EXCEPTION_DATATYPE_MISALIGNMENT:
            // Attempt to read or write data that is misaligned on hardware that requires alignment.
            throw_common_type(
               common_type::memory_access_error, reinterpret_cast<std::intptr_t>(nullptr), 0
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
            throw_common_type(common_type::floating_point_error, 0, 0);

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
            throw_common_type(common_type::division_by_zero_error, 0, 0);

         case EXCEPTION_INT_OVERFLOW:
            /* The result of an integer operation caused a carry out of the most significant bit of
            the result. */
            throw_common_type(common_type::overflow_error, 0, 0);

         case EXCEPTION_PRIV_INSTRUCTION:
            /* Attempt to execute an instruction whose operation is not allowed in the current
            machine mode. */
            break;

         case EXCEPTION_STACK_OVERFLOW:
            // The thread used up its stack.
            break;
      }
   }

   /*static*/ void exception::fault_converter::init_for_current_thread() {
      // Install the SEH translator, without saving the original.
      ::_set_se_translator(&fault_se_translator);
   }

   } //namespace abc

#else
   #error "TODO: HOST_API"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
