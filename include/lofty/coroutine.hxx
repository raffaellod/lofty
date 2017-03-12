/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_COROUTINE_HXX
#define _LOFTY_COROUTINE_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/io.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! @page coroutines Coroutines
Asynchronous code execution via cooperative multithreading.

Lofty supports asynchronous code execution via preemptive multithreading (see @ref threads) and via
coroutines, a form of cooperative multithreading alternative to the confusing “callbacks waterfall” pattern.

Just like threads are created by instantiating a lofty::thread object with a function to run in the new
thread, coroutines are created by instantiating a lofty::coroutine object with a function to run in the new
coroutine. Unlike threads, coroutines won’t necessarily start executing immediately.

Upon instantiation, coroutines are scheduled to run on the current thread’s lofty::coroutine::scheduler
instance; if none was attached with lofty::this_thread::attach_coroutine_scheduler(), a new instance is
created and attached to the current thread.

Once one or more coroutines have been instantiated and implicitly scheduled to run, it’s the application’s
responsibility to give control to the scheduler by invoking lofty::this_thread::run_coroutines() on at least
one of the threads attached to that scheduler.

If an exception escapes from a coroutine, the scheduler that was running it will terminate any other 
coroutines associated to it, and will then proceed to throw a similar exception in the containing thread,
possibly leading to the termination of the entire process (see @ref threads).

If a thread is interrupted by an exception while executing lofty::coroutine::scheduler code, the scheduler
will terminate every coroutine associated to it, and then throw a similar exception to the caller of
lofty::this_thread::run_coroutines(), eventually leading to the effect described above. */

/*! @page interruption-points Interruption points

@ref coroutines and @ref threads in Lofty are safely interruptible using a built-in mechanism.

Calling lofty::coroutine::interrupt() or lofty::thread::interrupt() will cause the object on which they’re
called to receive an exception of type lofty::execution_interruption, which will be thrown at the earliest
time the coroutine or thread calls lofty::this_coroutine::interruption_point() or
lofty::this_thread::interruption_point(), respectively.

Interruption points are used to dispatch other kinds of exceptions, such as those originating from external
inputs, such as Ctrl+C. The thrown exception in these cases will be of a subclass of 
lofty::execution_interruption.

The following functions and methods implicitly define an interruption point:
•  lofty::this_thread::sleep_for_ms() / lofty::this_coroutine::sleep_for_ms();
•  lofty::this_thread::sleep_until_fd_ready() / lofty::this_coroutine::sleep_until_fd_ready();
•  All I/O operations performed on lofty::io file-based stream classes;
•  All I/O operations in lofty::net classes.

Interruption points should be avoided in destructors or catch blocks, since interruption points may recurse: a
first interruption will cause an exception to be thrown in a coroutine/thread once detected by
lofty::this_coroutine::interruption_point(); if a second interruption occurs in that same coroutine/thread,
and the code executed during the flight of the first exception (either due to stack unwinding or by explicit
catch blocks) causes another call to interruption_point(), a second exception will be thrown in that
coroutine/thread, resulting in a nested exception if in the context of a catch block or a call to std::abort()
if in the context of a destructor. */

/*! Subroutine for use in non-preemptive multitasking, enabling asynchronous I/O in most lofty::io classes.
See @ref coroutines for more information. */
class LOFTY_SYM coroutine : public noncopyable {
public:
   //! Type of the unique coroutine IDs.
   typedef std::intptr_t id_type;
   //! Coroutine implementation.
   class impl;
   //! Schedules coroutine execution.
   class scheduler;

public:
   //! Default constructor.
   coroutine();

   /*! Constructor that immediately schedules a function to be executed as a coroutine.

   @param main_fn
      Function to invoke once the coroutine is first scheduled.
   */
   explicit coroutine(_std::function<void ()> main_fn);

   /*! Move constructor.

   @param coro
      Source object.
   */
   coroutine(coroutine && coro) :
      pimpl(_std::move(coro.pimpl)) {
   }

   //! Destructor.
   ~coroutine();

   /*! Move-assignment operator.

   @param coro
      Source object.
   @return
      *this.
   */
   coroutine & operator=(coroutine && coro) {
      pimpl = _std::move(coro.pimpl);
      return *this;
   }

   /*! Returns a process-wide unique ID for the coroutine.

   @return
      Unique ID representing the coroutine.
   */
   id_type id() const;

   /*! Interrupts the coroutine by throwing a lofty::execution_interruption instance in it.

   Interruption will occur as soon as the target coroutine (*this) performs a call to
   this_coroutine::sleep_for_ms() or other coroutine sleep functions; if the coroutine is already blocked on
   such call, the effect will be immediate from the coroutine’s point of view.

   Interruption will only occur when the scheduler is able to schedule the target coroutine; in a
   single-threaded scheduler case, this means that a coroutine calling interrupt() on another coroutine should
   then follow with a coroutine sleep function call to allow the target coroutine to be scheduled and
   interrupted as requested. */
   void interrupt();

private:
   //! Pointer to the implementation instance.
   _std::shared_ptr<impl> pimpl;
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <>
class LOFTY_SYM to_text_ostream<coroutine> : public to_text_ostream<coroutine::id_type> {
public:
   /*! Changes the output format.

   @param format
      Formatting options.
   */
   void set_format(str const & format);

   /*! Writes a coroutine’s identifier, applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(coroutine const & src, io::text::ostream * dst);
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Functions that can only affect the current coroutine; coroutine counterpart to this_thread.
namespace this_coroutine {}

} //namespace lofty

namespace lofty { namespace this_coroutine {

/*! Returns a process-wide unique ID for the current coroutine.

@return
   Unique ID representing the current coroutine.
*/
LOFTY_SYM coroutine::id_type id();

/*! Declares an interruption point, allowing the calling thread or coroutine to act on any pending
interruptions. See @ref interruption-points for more information. */
LOFTY_SYM void interruption_point();

/*! Suspends execution of the current coroutine for at least the specified duration.

@param millisecs
   Duration for which the current coroutine should not execute, in milliseconds.
*/
LOFTY_SYM void sleep_for_ms(unsigned millisecs);

/*! Suspends execution of the current coroutine until an asynchronous I/O operation completes.

@param fd
   File descriptor that the calling coroutine is waiting for I/O on.
@param write
   true if the coroutine is waiting to write to fd, or false if it’s waiting to read from it.
@param ovl
   (Win32 only) Pointer to the lofty::io::overlapped object that is being used for the asynchronous I/O
   operation.
*/
LOFTY_SYM void sleep_until_fd_ready(
   io::filedesc_t fd, bool write
#if LOFTY_HOST_API_WIN32
   , io::overlapped * ovl
#endif
);

}} //namespace lofty::this_coroutine

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_COROUTINE_HXX
