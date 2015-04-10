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

// #include <abaclade.hxx> already done in exception-fault_converter.cxx.

// TODO: use Mach threads instead of pthreads for the exception-catching thread.
#include <pthread.h> // pthread_create()

// Mach reference: <http://web.mit.edu/darwin/src/modules/xnu/osfmk/man/>.
#include <mach/mach.h> // boolean_t mach_msg_header_t
#include <mach/mach_traps.h> // mach_task_self()


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::exception::fault_converter

/*! Handles a kernel-reported thread exception. This is exposed by Mach, but for some reason not
declared in any system headers. See <http://web.mit.edu/darwin/src/modules/xnu/osfmk/man/
exc_server.html>.

@param pmsghRequest
   Pointer to the request message.
@param pmsghReply
   Pointer to the message that will receive the reply.
@return
   true if the message was handled and catch_exception_raise() (defined in this file) was called, or
   false if the message had nothing to do with exceptions.
*/
extern "C" ::boolean_t exc_server(
   ::mach_msg_header_t * pmsghRequest, ::mach_msg_header_t * pmsghReply
);

/*! Called by exc_server() when the latter is passed an exception message, giving the process a way
to do something about it. What we do is change the next instruction in the faulting thread to
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
            // TODO: better exception type.
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
   exception::inject_in_context(inj, iArg0, iArg1, thrst);

   // Update the faulting thread’s state.
   if (::thread_set_state(
      mpThread, sc_tsfThread, reinterpret_cast< ::thread_state_t>(&thrst), iThreadStatesCount
   ) != KERN_SUCCESS) {
      return KERN_FAILURE;
   }
   return KERN_SUCCESS;
}

namespace {

//! Thread in charge of handling exceptions for all the other threads.
::pthread_t g_thrExcHandler;
//! Port through which we ask the kernel to communicate exceptions to this process.
::mach_port_t g_mpExceptions;

void * exception_handler_thread(void *) {
   for (;;) {
      /* The exact definition of these structs is in the kernel’s sources; thankfully all we need to
      do with them is pass them around, so just define them as BLOBs and hope that they’re large
      enough. */
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
         &msg.msgh, MACH_RCV_MSG | MACH_RCV_LARGE, 0, sizeof msg, g_mpExceptions,
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

} //namespace

namespace abc {

exception::fault_converter::fault_converter() {
   ::mach_port_t mpThisProc = ::mach_task_self();
   // Allocate a right-less port to listen for exceptions.
   if (::mach_port_allocate(mpThisProc, MACH_PORT_RIGHT_RECEIVE, &g_mpExceptions) == KERN_SUCCESS) {
      // Assign rights to the port.
      if (::mach_port_insert_right(
         mpThisProc, g_mpExceptions, g_mpExceptions, MACH_MSG_TYPE_MAKE_SEND
      ) == KERN_SUCCESS) {
         // Start the thread that will catch exceptions from all the others.
         if (::pthread_create(&g_thrExcHandler, nullptr, exception_handler_thread, nullptr) == 0) {
            // Now that the handler thread is running, set the process-wide exception port.
            if (::task_set_exception_ports(
               mpThisProc,
               EXC_MASK_BAD_ACCESS | EXC_MASK_BAD_INSTRUCTION | EXC_MASK_ARITHMETIC,
               g_mpExceptions, EXCEPTION_DEFAULT, MACHINE_THREAD_STATE
            ) == KERN_SUCCESS) {
               // All good.
            }
         }
      }
   }
}

exception::fault_converter::~fault_converter() {
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
