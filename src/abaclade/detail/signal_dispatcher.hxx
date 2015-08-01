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

#ifndef _ABACLADE_DETAIL_SIGNAL_DISPATCHER_HXX
#define _ABACLADE_DETAIL_SIGNAL_DISPATCHER_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/collections/hash_map.hxx>
#include <abaclade/thread.hxx>

#include <mutex>

#if ABC_HOST_API_MACH
   // Mach reference: <http://web.mit.edu/darwin/src/modules/xnu/osfmk/man/>.
   #include <mach/mach.h> // mach_port_t
   #include <pthread.h>
#elif ABC_HOST_API_POSIX
   #include <signal.h> // sigaction sig*()
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

/*! Dispatches non-C++ signals to the process’ threads. It establishes, and restores upon
destruction, special-case handlers to convert non-C++ synchronous error events (Mach exceptions,
POSIX signals, Win32 Structured Exceptions) and termination signals (more POSIX signals, Win32
CTRL_*_EVENTs) into C++ exceptions.

This class keeps track of all threads managed by Abaclade to distribute signals among them and
verify that they all terminate at the end of a program.

This class is a singleton instantiated by abc::app. */
class signal_dispatcher {
public:
   //! Default constructor.
   signal_dispatcher();

   //! Destructor.
   ~signal_dispatcher();

#if ABC_HOST_API_WIN32
   /*! Initializes the fault converter for the current thread. Every thread but the first calls
   this. */
   static void init_for_current_thread();
#endif

   /*! Returns a pointer to the singleton instance.

   @return
      Pointer to the only instance of this class.
   */
   static signal_dispatcher & instance() {
      return *sm_psd;
   }

#if ABC_HOST_API_POSIX
   /*! Returns the signal number to be used to interrupt a thread so that it can process any pending
   exceptions.

   @param return
      Signal number.
   */
   int thread_interruption_signal() const {
      return mc_iThreadInterruptionSignal;
   }
#endif

   //! Initializes internal data for the main thread.
   void main_thread_started();

   /*! Registers the termination of the program’s abc::app::main() overload.

   @param xct
      Type of exception that escaped the program’s main(), or exception::common_type::none if main()
      returned normally.
   */
   void main_thread_terminated(exception::common_type xct);

   /*! Registers a new thread as running.

   @param pthrimpl
      Pointer to the abc::thread::impl instance running the thread.
   */
   void nonmain_thread_started(std::shared_ptr<thread::impl> const & pthrimpl);

   /*! Registers a non-main thread as no longer running.

   @param pthrimpl
      Pointer to the abc::thread::impl instance running the thread.
   @param bUncaughtException
      true if an exception escaped the thread’s function and was only blocked by abc::thread::impl.
   */
   void nonmain_thread_terminated(thread::impl * pthrimpl, bool bUncaughtException);

private:
#if ABC_HOST_API_POSIX
   /*! Handles the Abaclade-defined signal used to interrupt any thread, by doing nothing. This
   allows to break out of a syscall with EINTR, so the code following the interrupted call can check
   abc::thread::impl::m_xctPending.

   @param iSignal
      Signal number for which the function is being called.
   @param psi
      Additional information on the signal.
   @param pctx
      Thread context.
   */
   static void thread_interruption_signal_handler(int iSignal, ::siginfo_t * psi, void * pctx);
#endif

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

   /*! Handles SIGINT and SIGTERM for the main thread, injecting an appropriate exception type in
   the thread’s context.

   @param iSignal
      Signal number for which the function is being called.
   @param psi
      Additional information on the signal.
   @param pctx
      Thread context. Used to manipulate the stack of the thread to inject a function call.
   */
   static void interruption_signal_handler(int iSignal, ::siginfo_t * psi, void * pctx);
#elif ABC_HOST_API_WIN32
   /*! Translates Win32 console eventstructured exceptions into C++ exceptions.

   @param iCtrlEvent
      CTRL_*_EVENT.
   @return
       true to indicate that the message was processed, or false to execute the next handler.
   */
   static ::BOOL WINAPI console_ctrl_event_translator(::DWORD iCtrlEvent);

   /*! Translates Win32 structured exceptions into C++ exceptions.

   @param iCode
      Exception code.
   @param pxpInfo
      Structured exception information.
   */
   static void ABC_STL_CALLCONV fault_se_translator(
      unsigned iCode, ::_EXCEPTION_POINTERS * pxpInfo
   );
#endif

private:
#if ABC_HOST_API_POSIX
   //! Signal number to be used to interrupt threads.
   int const mc_iThreadInterruptionSignal;
#endif
#if ABC_HOST_API_MACH
   //! Port through which we ask the kernel to communicate exceptions to this process.
   ::mach_port_t m_mpExceptions;
   //! Thread in charge of handling exceptions for all the other threads.
   ::pthread_t m_thrExcHandler;
#elif ABC_HOST_API_WIN32
   //! Structured Exception translator on program startup.
   ::_se_translator_function m_setfDefault;
#endif
   /*! Pointer to an incomplete abc::thread::impl instance that’s used to control the main (default)
   thread of the process. */
   // TODO: instantiate this lazily, only if needed.
   std::shared_ptr<thread::impl> m_pthrimplMain;
   //! Governs access to m_hmThreads.
   std::mutex m_mtxThreads;
   //! Tracks all threads running in the process except *m_pthrimplMain.
   // TODO: make this a hash_set instead of a hash_map.
   collections::hash_map<thread::impl *, std::shared_ptr<thread::impl>> m_hmThreads;

   //! Pointer to the singleton instance.
   static signal_dispatcher * sm_psd;
#if ABC_HOST_API_POSIX
   //! Fault signals that we can translate into C++ exceptions.
   static int const smc_aiFaultSignals[];
   //! Interruption signals that we can translate into C++ exceptions.
   static int const smc_aiInterruptionSignals[];
#endif
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_DETAIL_SIGNAL_DISPATCHER_HXX
