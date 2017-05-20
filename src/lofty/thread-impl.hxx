/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_THREAD_IMPL_HXX
#define _LOFTY_THREAD_IMPL_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/coroutine.hxx>
#include <lofty/thread.hxx>
#include "_pvt/signal_dispatcher.hxx"

#if LOFTY_HOST_API_POSIX
   #include <errno.h> // EINTR errno
   #include <signal.h>
   #include <sys/poll.h>
   #if LOFTY_HOST_API_DARWIN
      #include <dispatch/dispatch.h>
   #else
      #include <semaphore.h>
   #endif
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Event that can be waited for. Not compatible with coroutines, since it doesn’t yield to a
coroutine::scheduler. */
// TODO: make this a non-coroutine-friendly general-purpose event.
namespace lofty { namespace _pvt {

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
#if LOFTY_HOST_API_DARWIN
   //! Underlying dispatch semaphore.
   ::dispatch_semaphore_t disp_sem;
#elif LOFTY_HOST_API_POSIX
   //! Underlying POSIX semaphore.
   ::sem_t sem;
#elif LOFTY_HOST_API_WIN32
   //! Underlying event.
   ::HANDLE event;
#else
   #error "TODO: HOST_API"
#endif
};

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace this_thread {

/*! Returns a pointer to the impl instance for the calling thread.

@return
   Pointer to the impl instance for the calling thread, or nullptr if the thread is not managed by Lofty.
*/
thread::impl * get_impl();

}} //namespace lofty::this_thread

namespace lofty {

class thread::impl {
private:
   friend id_type thread::id() const;
   friend native_handle_type thread::native_handle() const;
   friend impl * this_thread::get_impl();
   friend void this_thread::interruption_point();
   friend void _pvt::signal_dispatcher::main_thread_terminated(exception::common_type x_type);

public:
   /*! Constructor.

   @param main_fn
      Initial value for inner_main_fn.
   */
   explicit impl(_std::function<void ()> main_fn);

   //! Constructor used to instantiate an impl for the main thread.
   explicit impl(std::nullptr_t);

   //! Destructor.
   ~impl();

   /*! Returns a pointer to the coroutine scheduler associated to the thread, if any.

   @return
      Pointer to the thread’s coroutine scheduler.
   */
   _std::shared_ptr<coroutine::scheduler> & coroutine_scheduler() {
      return coro_sched;
   }

   /*! Injects the requested type of exception in the thread.

   @param x_type
      Type of exception to inject.
   @param send_signal
      (POSIX only) If true (default), a signal will be raised to ensure that the thread is unblocked from any
      syscalls; if false, no signal will be raised.
   */
   void inject_exception(
      exception::common_type x_type
#if LOFTY_HOST_API_POSIX
      , bool send_signal = true
#endif
   );

#if LOFTY_HOST_API_WIN32
   /*! Returns the handle used to interrupt wait functions.

   @return
      Event handle.
   */
   ::HANDLE interruption_event_handle() const {
      return interruption_event;
   }
#endif

   //! Implementation of the waiting aspect of lofty::thread::join().
   void join();

   /*! Creates a thread to run outer_main().

   @param this_pimpl_ptr
      Pointer by which outer_main() will get a reference (as in refcount) to *this, preventing it from being
      deallocated while still running.
   */
   void start(_std::shared_ptr<impl> * this_pimpl_ptr);

   /*! Returns true if the thread is terminating, which means that it’s running Lofty threading code instead
   of application code.

   @return
      true if the thread is terminating, or false otherwise.
   */
   bool terminating() const {
      return terminating_.load();
   }

private:
   /*! Lower-level wrapper for the thread function passed to the constructor. Under POSIX, this is also needed
   to get the thread ID and store it in the impl instance.

   @param p
      Pointer to a shared_ptr to *this.
   @return
      Unused.
   */
#if LOFTY_HOST_API_POSIX
   static void * outer_main(void * p);
#elif LOFTY_HOST_API_WIN32
   static ::DWORD WINAPI outer_main(void * p);
#else
   #error "TODO: HOST_API"
#endif

private:
   //! OS-dependent ID/handle.
   native_handle_type handle;
#if LOFTY_HOST_API_POSIX
   //! OS-dependent ID for use with OS-specific API (pthread_*_np() functions and other native API).
   id_type id;
#elif LOFTY_HOST_API_WIN32
   /*! Handle that all ::WaitFor*() function calls must include to achieve something simlar to POSIX
   asynchronous signal delivery (when signals interrupt syscalls, making them return EINTR). */
   ::HANDLE interruption_event;
#endif
   /*! Pointer to an event used by the new thread to report to its parent that it has started. Only
   non-nullptr during the execution of start(). */
   _pvt::simple_event * started_event_ptr;
   /*! Every time the thread returns from an interruption point, this is checked for pending exceptions to be
   injected. */
   _std::atomic<exception::common_type::enum_type> pending_x_type;
   /*! true if the thread is terminating, i.e. running Lofty threading code, or false if it’s still running
   application code. */
   _std::atomic<bool> terminating_;
   //! Function to be executed in the thread.
   _std::function<void ()> inner_main_fn;
   //! Pointer to the thread’s coroutine scheduler, if any.
   _std::shared_ptr<coroutine::scheduler> coro_sched;

   //! Allows a thread to access its impl instance.
   static thread_local_value<impl *> pimpl_via_tls;
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_THREAD_IMPL_HXX
