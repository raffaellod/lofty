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

#ifndef _ABACLADE_EXCEPTION_FAULT_CONVERTER_HXX
#define _ABACLADE_EXCEPTION_FAULT_CONVERTER_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#if ABC_HOST_API_MACH
   // Mach reference: <http://web.mit.edu/darwin/src/modules/xnu/osfmk/man/>.
   #include <mach/mach.h> // mach_port_t
   #include <pthread.h>
#elif ABC_HOST_API_POSIX
   #include <signal.h> // sigaction sig*()
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::exception::fault_converter

namespace abc {

class exception::fault_converter : public noncopyable {
public:
   //! Constructor.
   fault_converter();

   //! Destructor.
   ~fault_converter();

#if ABC_HOST_API_WIN32
   /*! Initializes the fault converter for the current thread. Every thread but the first calls
   this. */
   static void init_for_current_thread();
#endif

private:
#if ABC_HOST_API_MACH
   //! Handles exceptions for every thread. Runs in its own thread.
   static void * exception_handler_thread(void *);
#elif ABC_HOST_API_POSIX
   /*! Translates POSIX signals into C++ exceptions, whenever possible. This works by injecting the
   stack frame of a call to throw_common_type(), and then returning, ending processing of the
   signal. Execution will resume from throw_common_type(), which creates the appearance of a C++
   exception being thrown at the location of the offending instruction, without calling any of the
   (many) functions that are forbidden in a signal handler.

   @param iSignal
      Signal number for which the function is being called.
   @param psi
      Additional information on the signal.
   @param pctx
      Thread context. This is used to manipulate the stack of the thread to inject a call frame.
   */
   static void fault_signal_handler(int iSignal, ::siginfo_t * psi, void * pctx);
#elif ABC_HOST_API_WIN32
   static void ABC_STL_CALLCONV fault_se_translator(
      unsigned iCode, ::_EXCEPTION_POINTERS * pxpInfo
   );
#endif

private:
#if ABC_HOST_API_MACH
   //! Port through which we ask the kernel to communicate exceptions to this process.
   ::mach_port_t m_mpExceptions;
   //! Thread in charge of handling exceptions for all the other threads.
   ::pthread_t m_thrExcHandler;
#elif ABC_HOST_API_POSIX
   //! Signals that we can translate into C++ exceptions.
   static int const smc_aiHandledSignals[];
#elif ABC_HOST_API_WIN32
   //! Structured Exception translator on program startup.
   ::_se_translator_function m_setfDefault;
#endif
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_EXCEPTION_FAULT_CONVERTER_HXX
