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

#ifndef _ABACLADE_COROUTINE_HXX
#define _ABACLADE_COROUTINE_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::coroutine

namespace abc {

/*! @page coroutines Coroutines
Asynchronous code execution via cooperative multithreading.

Abaclade supports asynchronous code execution via preemptive multithreading (see @ref threads) and
via coroutines, a form of cooperative multithreading alternative to the confusing “callbacks
waterfall” pattern.

Just like threads are created by instantiating an abc::thread object with a function to run in the
new thread, coroutines are created by instantiating an abc::coroutine object with a function to run
in the new coroutine. Unlike threads, coroutines won’t necessarily start executing immediately.

Upon instantiation, coroutines are scheduled to run on the current thread’s
abc::coroutine::scheduler instance; if none was attached with
abc::this_thread::attach_coroutine_scheduler(), a new instance is created and attached to the
current thread.

Once one or more coroutines have been instantiated and implicitly scheduled to run, it’s the
application’s responsibility to give control to the scheduler by invoking
abc::this_thread::run_coroutines() on at least one of the threads attached to that scheduler.

If an exception escapes from a coroutine, the scheduler that was running it will terminate any other
coroutines associated to it, and will then proceed to rethrow the exception in the containing
thread, possibly leading to the termination of the entire process (see @ref threads).

If a thread is interrupted by an exception while executing abc::coroutine::scheduler code, the
scheduler will terminate every coroutine associated to it, and then rethrow the exception to the
caller of abc::this_thread::run_coroutines(), eventually leading to the effect described above.
*/

/*! Subroutine for use in non-preemptive multitasking, enabling asynchronous I/O in most abc::io
classes. See @ref coroutines for more information.
*/
class ABACLADE_SYM coroutine : public noncopyable {
public:
   //! Type of the unique coroutine IDs.
   typedef std::intptr_t id_type;
   //! Coroutine implementation.
   class impl;
   //! Schedules coroutine execution.
   class scheduler;

public:
   /*! Constructor.

   @param fnMain
      Function to invoke once the coroutine is first scheduled.
   @param coro
      Source object.
   */
   coroutine();
   explicit coroutine(std::function<void ()> fnMain);
   coroutine(coroutine && coro) :
      m_pimpl(std::move(coro.m_pimpl)) {
   }

   //! Destructor.
   ~coroutine();

   /*! Assignment operator.

   @param coro
      Source object.
   @return
      *this.
   */
   coroutine & operator=(coroutine && coro) {
      m_pimpl = std::move(coro.m_pimpl);
      return *this;
   }

   /*! Returns a process-wide unique ID for the coroutine.

   @return
      Unique ID representing the coroutine.
   */
   id_type id() const;

   /*! Interrupts the coroutine by throwing an abc::execution_interruption instance in it.

   Interruption will occur as soon as the target coroutine (*this) performs a call to
   this_coroutine::sleep_for_ms() or other coroutine sleep functions; if the coroutine is already
   blocked on such call, the effect will be immediate from the coroutine’s point of view.

   Interruption will only occur when the scheduler is able to schedule the target coroutine; in a
   single-threaded scheduler case, this means that a coroutine calling interrupt() on another
   coroutine should then follow with a coroutine sleep function call to allow the target coroutine
   to be scheduled and interrupted as requested. */
   void interrupt();

private:
   //! Pointer to the implementation instance.
   std::shared_ptr<impl> m_pimpl;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specialization for abc::coroutine

namespace abc {

template <>
class ABACLADE_SYM to_str_backend<coroutine> {
public:
   //! Constructor.
   to_str_backend();

   //! Destructor.
   ~to_str_backend();

   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(istr const & sFormat);

   /*! Writes a string, applying the formatting options.

   @param op
      Path to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(coroutine const & coro, io::text::writer * ptwOut);

protected:
   //! Backend used to write strings.
   to_str_backend<istr> m_tsbStr;
   //! Backend used to write coroutine ID.
   to_str_backend<coroutine::id_type> m_tsbId;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::this_coroutine

namespace abc {
//! Functions that can only affect the current coroutine. Coroutine counterpart to abc::this_thread.
namespace this_coroutine {

/*! Returns a process-wide unique ID for the current coroutine.

@return
   Unique ID representing the current coroutine.
*/
ABACLADE_SYM coroutine::id_type id();

/*! Suspends execution of the current coroutine for at least the specified duration.

@param iMillisecs
   Duration for which the current coroutine should not execute, in milliseconds.
*/
ABACLADE_SYM void sleep_for_ms(unsigned iMillisecs);

/*! Suspends execution of the current coroutine until an asynchronous I/O operation completes.

@param fd
   File descriptor that the calling coroutine is waiting for I/O on.
@param bWrite
   true if the coroutine is waiting to write to fd, or false if it’s waiting to read from it.
*/
ABACLADE_SYM void sleep_until_fd_ready(io::filedesc_t fd, bool bWrite);
inline void sleep_until_fd_ready(io::filedesc const & fd, bool bWrite) {
   sleep_until_fd_ready(fd.get(), bWrite);
}

} //namespace this_coroutine
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COROUTINE_HXX
