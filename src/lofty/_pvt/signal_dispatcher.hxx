/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY__PVT_SIGNAL_DISPATCHER_HXX
#define _LOFTY__PVT_SIGNAL_DISPATCHER_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/collections/hash_map.hxx>
#include <lofty/thread.hxx>

#if LOFTY_HOST_API_MACH
   // Mach reference: <http://web.mit.edu/darwin/src/modules/xnu/osfmk/man/>.
   #include <mach/mach.h> // mach_port_t
   #include <pthread.h>
#endif
#if LOFTY_HOST_API_POSIX
   #include <signal.h> // sigaction siginfo_t sig*()
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

/*! Dispatches non-C++ signals to the process’ threads. It establishes, and restores upon destruction,
special-case handlers to convert non-C++ synchronous error events (Mach exceptions, POSIX signals, Win32
Structured Exceptions) and termination signals (more POSIX signals, Win32 CTRL_*_EVENTs) into C++ exceptions.

This class keeps track of all threads managed by Lofty to distribute signals among them and verify that they
all terminate at the end of a program.

This class is a singleton instantiated by lofty::app. */
class signal_dispatcher {
public:
   //! Default constructor.
   signal_dispatcher();

   //! Destructor.
   ~signal_dispatcher();

#if LOFTY_HOST_API_WIN32
   //! Initializes the fault converter for the current thread. Every thread but the first calls this.
   static void init_for_current_thread();
#endif

   /*! Returns a pointer to the singleton instance.

   @return
      Pointer to the only instance of this class.
   */
   static signal_dispatcher & instance() {
      return *this_instance;
   }

#if LOFTY_HOST_API_POSIX
   /*! Handles SIGINT and SIGTERM for the main thread, injecting an appropriate exception type in the thread’s
   context.

   @param signal
      Signal number for which the function is being called.
   @param si
      Additional information on the signal.
   @param ctx
      Thread context. Used to manipulate the stack of the thread to inject a function call.
   */
   static void interruption_signal_handler(int signal, ::siginfo_t * si, void * ctx);

   /*! Returns the signal number to be used to interrupt a thread so that it can process any pending
   exceptions.

   @param return
      Signal number.
   */
   int thread_interruption_signal() const {
      return thread_interruption_signal_;
   }
#endif

   //! Initializes internal data for the main thread.
   void main_thread_started();

   /*! Registers the termination of the program’s lofty::app::main() overload.

   @param xct
      Type of exception that escaped the program’s main(), or exception::common_type::none if main() returned
      normally.
   */
   void main_thread_terminated(exception::common_type xct);

   /*! Registers a new thread as running.

   @param thread_pimpl
      Pointer to the lofty::thread::impl instance running the thread.
   */
   void nonmain_thread_started(_std::shared_ptr<thread::impl> const & thread_pimpl);

   /*! Registers a non-main thread as no longer running.

   @param thread_pimpl
      Pointer to the lofty::thread::impl instance running the thread.
   @param uncaught_exception
      true if an exception escaped the thread’s function and was only blocked by lofty::thread::impl.
   */
   void nonmain_thread_terminated(thread::impl * thread_pimpl, bool uncaught_exception);

private:
#if LOFTY_HOST_API_POSIX
   /*! Handles the Lofty-defined signal used to interrupt any thread, by doing nothing. This allows to break
   out of a syscall with EINTR, so the code following the interrupted call can check
   lofty::thread::impl::pending_x_type.

   @param signal
      Signal number for which the function is being called.
   @param si
      Additional information on the signal.
   @param ctx
      Thread context.
   */
   static void thread_interruption_signal_handler(int signal, ::siginfo_t * si, void * ctx);
#endif

#if LOFTY_HOST_API_MACH
   //! Handles exceptions for every thread. Runs in its own thread.
   static void * exception_handler_thread(void *);
#elif LOFTY_HOST_API_POSIX
   /*! Translates POSIX signals into C++ exceptions, whenever possible. This works by injecting the stack
   frame of a call to throw_common_type(), and then returning, ending processing of the signal. Execution will
   resume from throw_common_type(), which creates the appearance of a C++ exception being thrown at the
   location of the offending instruction, without calling any of the (many) functions that are forbidden in a
   signal handler.

   @param signal
      Signal number for which the function is being called.
   @param si
      Additional information on the signal.
   @param ctx
      Thread context. This is used to manipulate the stack of the thread to inject a call frame.
   */
   static void fault_signal_handler(int signal, ::siginfo_t * si, void * ctx);

#elif LOFTY_HOST_API_WIN32
   /*! Translates Win32 console events into C++ exceptions.

   @param ctrl_event
      CTRL_*_EVENT.
   @return
       true to indicate that the message was processed, or false to execute the next handler.
   */
   static ::BOOL WINAPI console_ctrl_event_translator(::DWORD ctrl_event);

   /*! Translates Win32 structured exceptions into C++ exceptions.

   @param code
      Exception code.
   @param sx_info
      Structured exception information.
   */
   static void LOFTY_STL_CALLCONV fault_se_translator(unsigned code, ::_EXCEPTION_POINTERS * sx_info);
#endif

private:
#if LOFTY_HOST_API_POSIX
   //! Signal number to be used to interrupt threads.
   int const thread_interruption_signal_;
#endif
#if LOFTY_HOST_API_MACH
   //! Port through which we ask the kernel to communicate exceptions to this process.
   ::mach_port_t exceptions_port;
   //! Thread in charge of handling exceptions for all the other threads.
   ::pthread_t exception_handler_thread;
#elif LOFTY_HOST_API_WIN32
   //! Structured Exception translator on program startup.
   ::_se_translator_function default_se_translator_fn;
#endif
   /*! Pointer to an incomplete lofty::thread::impl instance that’s used to control the main (default) thread
   of the process. */
   // TODO: instantiate this lazily, only if needed.
   _std::shared_ptr<thread::impl> main_thread;
   //! Governs access to known_threads.
   _std::mutex known_threads_mutex;
   //! Tracks all threads running in the process except *main_thread.
   // TODO: make this a hash_set instead of a hash_map.
   collections::hash_map<thread::impl *, _std::shared_ptr<thread::impl>> known_threads;

   //! Pointer to the singleton instance.
   static signal_dispatcher * this_instance;
#if LOFTY_HOST_API_POSIX
   //! Fault signals that we can translate into C++ exceptions.
   static int const fault_signals[];
   //! Signals that are redundant with errno values; we prefer errno to signals.
   static int const ignored_signals[];
   //! Interruption signals that we can translate into C++ exceptions.
   static int const interruption_signals[];
#endif
};

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY__PVT_SIGNAL_DISPATCHER_HXX
