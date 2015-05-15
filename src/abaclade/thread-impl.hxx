/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015
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

#ifndef _ABACLADE_THREAD_IMPL_HXX
#define _ABACLADE_THREAD_IMPL_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/thread.hxx>
#include "thread-tracker.hxx"

#include <atomic>

#if ABC_HOST_API_POSIX
   #include <signal.h>
   #if ABC_HOST_API_DARWIN
      #include <dispatch/dispatch.h>
   #else
      #include <semaphore.h>
   #endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::simple_event

/*! Event that can be waited for. Not compatible with coroutines, since it doesn’t yield to a
coroutine::scheduler. */
// TODO: make this a non-coroutine-friendly general-purpose event.
namespace abc {
namespace detail {

class simple_event : public noncopyable {
public:
   //! Constructor.
   simple_event();

   //! Destructor.
   ~simple_event();

   //! Raises the event.
   void raise();

   //! Waits for the event to be raised by another thread.
   void wait();

private:
#if ABC_HOST_API_DARWIN
   //! Underlying dispatch semaphore.
   ::dispatch_semaphore_t m_dsem;
#elif ABC_HOST_API_POSIX
   //! Underlying POSIX semaphore.
   ::sem_t m_sem;
#elif ABC_HOST_API_WIN32
   //! Underlying event.
   ::HANDLE m_hEvent;
#else
   #error "TODO: HOST_API"
#endif
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::thread::impl

namespace abc {

namespace this_thread {

/*! Returns a pointer to the impl instance for the calling thread.

@return
   Pointer to the impl instance for the calling thread, or nullptr if the thread is not managed by
   Abaclade.
*/
thread::impl * get_impl();

} //namespace this_thread

class thread::impl {
private:
   friend id_type thread::id() const;
   friend native_handle_type thread::native_handle() const;
   friend impl * this_thread::get_impl();
   friend void tracker::main_thread_terminated(exception::common_type xct);

public:
   /*! Constructor

   @param fnMain
      Initial value for m_fnInnerMain.
   */
   impl(std::function<void ()> fnMain);
   // This overload is used to instantiate an impl for the main thread.
   impl(std::nullptr_t);

   //! Destructor.
   ~impl();

   /*! Injects the requested type of exception in the thread.

   @param xct
      Type of exception to inject.
   */
   void inject_exception(exception::common_type xct);

#if ABC_HOST_API_POSIX
   /*! Handles SIGINT and SIGTERM for the main thread, as well as the Abaclade-defined signal used
   to interrupt any thread, injecting an appropriate exception type in the thread’s context.

   @param iSignal
      Signal number for which the function is being called.
   @param psi
      Additional information on the signal.
   @param pctx
      Thread context. This is used to manipulate the stack of the thread to inject a call frame.
   */
   static void interruption_signal_handler(int iSignal, ::siginfo_t * psi, void * pctx);
#endif

   //! Implementation of the waiting aspect of abc::thread::join().
   void join();

   /*! Creates a thread to run outer_main().

   @param ppimplThis
      Pointer by which outer_main() will get a reference (as in refcount) to *this, preventing it
      from being deallocated while still running.
   */
   void start(std::shared_ptr<impl> * ppimplThis);

   /*! Returns true if the thread is terminating, which means that it’s running Abaclade threading
   code instead of application code.

   @return
      true if the thread is terminating, or false otherwise.
   */
   bool terminating() const {
      return m_bTerminating.load();
   }

private:
   /*! Lower-level wrapper for the thread function passed to the constructor. Under POSIX, this is
   also needed to get the thread ID and store it in the impl instance.

   @param p
      Pointer to a shared_ptr to *this.
   @return
      Unused.
   */
#if ABC_HOST_API_POSIX
   static void * outer_main(void * p);
#elif ABC_HOST_API_WIN32
   static ::DWORD WINAPI outer_main(void * p);
#else
   #error "TODO: HOST_API"
#endif

private:
   //! OS-dependent ID/handle.
   native_handle_type m_h;
#if ABC_HOST_API_POSIX
   //! OS-dependent ID for use with OS-specific API (pthread_*_np() functions and other native API).
   id_type m_id;
#endif
   /*! Pointer to an event used by the new thread to report to its parent that it has started. Only
   non-nullptr during the execution of start(). */
   detail::simple_event * m_pseStarted;
   /*! true if the thread is terminating, i.e. running Abaclade threading code, or false if it’s
   still running application code. */
   std::atomic<bool> m_bTerminating;
   //! Function to be executed in the thread.
   std::function<void ()> m_fnInnerMain;
   //! Allows a thread to access its impl instance.
   static thread_local_value<impl *> sm_pimplThis;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_THREAD_IMPL_HXX
