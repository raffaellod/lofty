/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_THREAD_HXX
#define _LOFTY_THREAD_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/coroutine.hxx>

#if LOFTY_HOST_API_POSIX
   #include <pthread.h>
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! @page threads Threads
Asynchronous code execution via OS-provided preemptive multithreading.

Lofty provides augmented alternatives to std::thread and std::this_thread: lofty::thread and  
lofty::this_thread, respectively. In addition to every feature offered by std::thread and std::this_thread,
Lofty’s classes provide integration with coroutines (see @ref coroutines) and a predictable interruption/
termination model.

In programs based on Lofty, the POSIX signals SIGINT and SIGTERM are always only delivered to the main thread,
and converted into C++ exceptions; if the main thread does not block them and the exceptions escape
lofty::app::main(), Lofty will proceed to cleanly terminate all other threads in the process by interrupting
them with an appropriate exception type on their earliest interruption point (see @ref interruption-points).

If a non-main thread throws an exception and does not catch it, an exception will be thrown in the main thread
as soon as the main thread reaches an interruption point, leading to a behavior similar to what happens upon
receiving a SIGTERM in the main thread.

A thread, after its instantiation, must be either joined using its join() method, or be allowed to run freely,
using its detach() method. However, when the main thread terminates, all detached threads will receive an
exception at their earliest interruption point. */

//! Thread of program execution. Replacement for std::thread supporting cooperation with lofty::coroutine.
class LOFTY_SYM thread : public noncopyable {
public:
   //! OS-dependent type for unique thread IDs.
#if LOFTY_HOST_API_DARWIN
   typedef std::uint64_t id_type;
#elif LOFTY_HOST_API_FREEBSD
   typedef int id_type;
#elif LOFTY_HOST_API_LINUX
   typedef int id_type;
#elif LOFTY_HOST_API_WIN32
   typedef ::DWORD id_type;
#else
   #error "TODO: HOST_API"
#endif

   //! Thread implementation.
   class impl;

   //! Underlying OS-dependent ID/handle type.
#if LOFTY_HOST_API_POSIX
   typedef ::pthread_t native_handle_type;
#elif LOFTY_HOST_API_WIN32
   typedef ::HANDLE native_handle_type;
#else
   #error "TODO: HOST_API"
#endif

public:
   //! Default constructor.
   thread() {
   }

   /*! Constructor that immediately runs a function as a new thread.

   @param main_fn
      Function that will act as the entry point for a new thread to be started immediately.
   */
   explicit thread(_std::function<void ()> main_fn);

   /*! Move constructor.

   @param src
      Source object.
   */
   thread(thread && src) :
      pimpl(_std::move(src.pimpl)) {
   }

   //! Destructor.
   ~thread();

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   thread & operator=(thread && src) {
      pimpl = _std::move(src.pimpl);
      return *this;
   }

   /*! Releases the OS-dependent ID/handle, making *this reference no thread and invalidating the value
   returned by native_handle(). */
   void detach() {
      pimpl.reset();
   }

   /*! Returns a process-wide unique ID for the thread.

   @return
      Unique ID representing the thread.
   */
   id_type id() const;

   //! Interrupts the thread by throwing a lofty::execution_interruption instance in it.
   void interrupt();

   //! Waits for the thread to terminate.
   void join();

   /*! Returns true if calling join() on the object is allowed.

   @return
      true if the object is in a joinable state, or false otherwise.
   */
   bool joinable() const {
      return pimpl != nullptr;
   }

   /*! Returns the underlying ID/handle type.

   @return
      OS-dependent ID/handle.
   */
   native_handle_type native_handle() const;

private:
   //! Pointer to the implementation instance.
   _std::shared_ptr<impl> pimpl;
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <>
class LOFTY_SYM to_text_ostream<thread> : public to_text_ostream<thread::id_type> {
public:
   /*! Changes the output format.

   @param format
      Formatting options.
   */
   void set_format(str const & format);

   /*! Writes a thread’s identifier, applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(thread const & src, io::text::ostream * dst);
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Functions that can only affect the current thread; replacement for std::this_thread.
namespace this_thread {}

} //namespace lofty

namespace lofty { namespace this_thread {

/*! Attaches a coroutine scheduler to the current thread, and performs and necessary initialization required
for the current thread to run coroutines.

@param coro_sched
   Scheduler to attach. If omitted, a new coroutine::scheduler will be created.
@return
   Coroutine scheduler associated to this thread. If coro_sched was non-nullptr, this is the same as
   coro_sched.
*/
LOFTY_SYM _std::shared_ptr<coroutine::scheduler> const & attach_coroutine_scheduler(
   _std::shared_ptr<coroutine::scheduler> coro_sched = nullptr
);

/*! Returns the coroutine scheduler associated to the current thread, if any.

@return
   Coroutine scheduler associated to this thread. May be nullptr if attach_coroutine_scheduler() was never
   called for the current thread.
*/
LOFTY_SYM _std::shared_ptr<coroutine::scheduler> const & coroutine_scheduler();

//! Removes the current thread’s coroutine scheduler, if any.
LOFTY_SYM void detach_coroutine_scheduler();

/*! Returns a process-wide unique ID for the current thread.

@return
   Unique ID representing the current thread.
*/
LOFTY_SYM thread::id_type id();

#if LOFTY_HOST_API_WIN32
/*! Performs a ::WaitForSingleObject() while being interruptible by lofty::thread::interrupt().

@param h
   Handle to wait for.
@param timeout_millisecs
   Time after which the wait will be interrupted and the wait aborted.
*/
LOFTY_SYM void interruptible_wait_for_single_object(::HANDLE handle, unsigned timeout_millisecs);
#endif

/*! Declares an interruption point, allowing the calling thread to act on any pending interruptions. See
@ref interruption-points for more information. */
LOFTY_SYM void interruption_point();

/*! Begins running scheduled coroutines on the current thread. Only returns after every coroutine scheduled on
the same thread or scheduler returns. */
LOFTY_SYM void run_coroutines();

/*! Suspends execution of the current thread for at least the specified duration.

@param millisecs
   Duration for which the current thread should not execute, in milliseconds.
*/
LOFTY_SYM void sleep_for_ms(unsigned millisecs);

/*! Suspends execution of the current thread until an asynchronous I/O operation completes.

@param fd
   File descriptor that the calling coroutine is waiting for I/O on.
@param write
   true if the coroutine is waiting to write to fd, or false if it’s waiting to read from it.
@param timeout_millisecs
   Time after which the wait will be interrupted and the I/O operation deemed failed.
@param ovl
   (Win32 only) Pointer to the lofty::io::overlapped object that is being used for the asynchronous I/O
   operation.
*/
LOFTY_SYM void sleep_until_fd_ready(
   io::filedesc_t fd, bool write, unsigned timeout_millisecs
#if LOFTY_HOST_API_WIN32
   , io::overlapped * ovl
#endif
);

}} //namespace lofty::this_thread

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_THREAD_HXX
