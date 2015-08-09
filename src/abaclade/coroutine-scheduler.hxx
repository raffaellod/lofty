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

#ifndef _ABACLADE_COROUTINE_SCHEDULER_HXX
#define _ABACLADE_COROUTINE_SCHEDULER_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/coroutine.hxx>
#include <abaclade/collections/hash_map.hxx>
#include <abaclade/collections/queue.hxx>
#include <abaclade/collections/trie_ordered_multimap.hxx>
#include <abaclade/thread.hxx>

#include <mutex>

#if ABC_HOST_API_POSIX
   #if ABC_HOST_API_DARWIN
      #define _XOPEN_SOURCE
   #endif
   #include <ucontext.h>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

class coroutine::scheduler : public noncopyable {
private:
   friend id_type this_coroutine::id();
   friend void this_coroutine::interruption_point();

public:
   /*! Integer type large enough to represent a time duration in milliseconds with a magnitude
   sufficient for scheduling coroutines. */
   typedef std::uint32_t time_duration_t;
   //! Integer type large enough to represent a point in time with resolution of one millisecond.
   typedef std::uint64_t time_point_t;

public:
   //! Constructor.
   scheduler();

   //! Destructor.
   ~scheduler();

   /*! Adds a coroutine to those ready to run. Ready coroutines take precedence over coroutines that
   were known to be blocked but might be ready on the next find_coroutine_to_activate() invocation.

   @param pcoroimpl
      Pointer to a coroutine (implementation) that’s ready to execute.
   */
   void add_ready(std::shared_ptr<impl> pcoroimpl);

   /*! Allows other coroutines to run, preventing the calling coroutine from being rescheduled until
   at least iMillisecs milliseconds have passed.

   @param iMillisecs
      Minimum duration for which to yield to other coroutines.
   */
   void block_active_for_ms(unsigned iMillisecs);

   /*! Allows other coroutines to run while the asynchronous I/O operation completes, as an
   alternative to blocking while waiting for its completion.

   @param fd
      File descriptor that the calling coroutine is waiting for I/O on.
   @param bWrite
      true if the coroutine is waiting to write to fd, or false if it’s waiting to read from it.
   @param phCurrentIocp
      (Win32 only) Checked on input and updated on output, allows to ensure that fd is only
      associated to a single IOCP at a time.
   */
   void block_active_until_fd_ready(
      io::filedesc_t fd, bool bWrite
#if ABC_HOST_API_WIN32
      , ::HANDLE * phCurrentIocp
#endif
   );

#if ABC_HOST_API_WIN32
   /*! Returns the internal IOCP.

   @return
   File descriptor of the internal IOCP.
   */
   io::filedesc_t iocp() const {
      return m_fdIocp.get();
   }
#endif

   /*! Switches context to the current thread’s own context.

   @param xct
      Type of exception that escaped the coroutine function, or exception::common_type::none if the
      function returned normally.
   */
   void return_to_scheduler(exception::common_type xct);

   /*! Begins scheduling and running coroutines on the current thread. Only returns after every
   coroutine added with add_coroutine() returns. */
   void run();

private:
#if ABC_HOST_API_LINUX || ABC_HOST_API_WIN32
   /*! Arms the internal timer responsible for all time-based waits.

   @param tdMillisecs
      Time in which the timer should fire.
   */
   void arm_timer(time_duration_t tdMillisecs) const;

   /*! Arms the internal timer so that it fires as requested by the next sleeping coroutine. If
   there are no sleeping coroutines, the timer will be disabled. */
   void arm_timer_for_next_sleep_end() const;

   /*! Returns the current time.

   @return
      Current time.
   */
   static time_point_t current_time();
#endif

   /*! Finds a coroutine ready to execute; if none are, but there are blocked coroutines, it blocks
   the current thread until one of them becomes ready.

   @return
      Pointer to a coroutine (implementation) that’s ready to execute.
   */
   std::shared_ptr<impl> find_coroutine_to_activate();

   /*! Repeatedly finds and runs coroutines that are ready to execute.

   @param bInterruptingAll
      If true, the loop won’t check for changes to m_xctInterruptionReason, assuming that it was
      already != none when the method was called.
   */
   void coroutine_scheduling_loop(bool bInterruptingAll = false);

   //! Interrupts with m_xctInterruptionReason any coroutines associated to the scheduler.
   void interrupt_all();

   /*! Interrupts any coroutines associated to the scheduler. If there’s no previous reason to
   interrupt all coroutines (i.e. if m_xctInterruptionReason == none), xctReason will be used as the
   reason.

   @param xctReason
      Reason for interruption.
   */
   void interrupt_all(exception::common_type xctReason);

   /*! Switches context from the coroutine context pointed to by pcoroimplLastActive to the current
   thread’s own context.

   @param pcoroimplLastActive
      Pointer to the coroutine (implementation) that is being inactivated.
   */
   void switch_to_scheduler(impl * pcoroimplLastActive);

#if ABC_HOST_API_WIN32
   //! Waits for m_fdTimer to fire, posting each firing to the IOCP.
   void timer_thread();

   /*! Invokes pThis->timer_thread().

   @param pThis
      this.
   @return
      0 if no errors occurred, or 1 if an exception was caught.
   */
   static ::DWORD WINAPI timer_thread_static(void * pThis);
#endif

private:
#if ABC_HOST_API_BSD
   //! File descriptor of the internal kqueue.
   io::filedesc m_fdKqueue;
   /*! Coroutines that are blocked on a timer wait. The keys are the same as the values, but this
   can’t be changed into a set<shared_ptr<impl>> because we need it to hold a strong reference to
   the coroutine implementation while allowing lookups without having a shared_ptr. */
   collections::hash_map<std::uintptr_t, std::shared_ptr<impl>> m_hmCorosBlockedByTimer;
#elif ABC_HOST_API_LINUX
   //! File descriptor of the internal epoll.
   io::filedesc m_fdEpoll;
#elif ABC_HOST_API_WIN32
   //! File descriptor of the internal IOCP.
   io::filedesc m_fdIocp;
   //! Thread that translates events from m_fdTimer into IOCP completions.
   ::HANDLE m_hTimerThread;
   std::atomic<bool> m_bTimerThreadEnd;
#else
   #error "TODO: HOST_API"
#endif
#if ABC_HOST_API_LINUX || ABC_HOST_API_WIN32
   //! Map of timeouts, in milliseconds, and their associated coroutines.
   collections::trie_ordered_multimap<
      time_point_t, std::shared_ptr<impl>
   > m_tommCorosBlockedByTimer;
   //! Timer responsible for every timed wait.
   io::filedesc m_fdTimer;
#endif
   //! Coroutines that are blocked on a fd wait.
   collections::hash_map<io::filedesc_t, std::shared_ptr<impl>> m_hmCorosBlockedByFD;
   /*! List of coroutines that are ready to run. Includes coroutines that have been scheduled, but
   have not been started yet. */
   collections::queue<std::shared_ptr<impl>> m_qReadyCoros;
   //! Governs access to m_qReadyCoros, m_hmCorosBlockedByFD and other “blocked by” maps/sets.
   std::mutex m_mtxCorosAddRemove;
   /*! Set to anything other than exception::common_type::none if a coroutine leaks an uncaught
   exception, or if the scheduler throws an exception while not running coroutines. Once one of
   these events happens, every thread running the scheduler will start interrupting coroutines with
   this type of exception. */
   std::atomic<exception::common_type::enum_type> m_xctInterruptionReason;

   //! Pointer to the active (current) coroutine, or nullptr if none is active.
   static thread_local_value<std::shared_ptr<impl>> sm_pcoroimplActive;
#if ABC_HOST_API_POSIX
   //! Pointer to the original context of every thread running a coroutine scheduler.
   static thread_local_value< ::ucontext_t *> sm_puctxReturn;
#elif ABC_HOST_API_WIN32
   //! Handle to the original fiber of every thread running a coroutine scheduler.
   static thread_local_value<void *> sm_pfbrReturn;
#endif
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COROUTINE_SCHEDULER_HXX
