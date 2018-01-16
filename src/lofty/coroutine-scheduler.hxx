/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_COROUTINE_SCHEDULER_HXX
#define _LOFTY_COROUTINE_SCHEDULER_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/coroutine.hxx>
#include <lofty/collections/hash_map.hxx>
#include <lofty/collections/queue.hxx>
#include <lofty/collections/trie_ordered_multimap.hxx>
#include <lofty/event.hxx>
#include <lofty/thread.hxx>

#if LOFTY_HOST_API_POSIX
   #include <ucontext.h>
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

class coroutine::scheduler : public noncopyable {
private:
   friend id_type this_coroutine::id();
   friend void this_coroutine::interruption_point();

public:
   //! Type of the id of a waitable event.
   typedef event::id_type event_id_t;
   /*! Integer type large enough to represent a time duration in milliseconds with a magnitude sufficient for
   scheduling coroutines. */
   typedef std::uint32_t time_duration_t;
   //! Integer type large enough to represent a point in time with resolution of one millisecond.
   typedef std::uint64_t time_point_t;

private:
   union fd_io_key {
#if LOFTY_HOST_API_BSD
      typedef void * pack_t;
#elif LOFTY_HOST_API_LINUX
      typedef std::uint64_t pack_t;
#elif LOFTY_HOST_API_WIN32
      typedef ::ULONG_PTR pack_t;
#endif
      struct s_t {
         io::filedesc_t fd;
         bool write;
      } s;
      pack_t pack;
   };

public:
   //! Constructor.
   scheduler();

   //! Destructor.
   ~scheduler();

   /*! Adds a coroutine to those ready to run. Ready coroutines take precedence over coroutines that were
   known to be blocked but might be ready on the next find_coroutine_to_activate() invocation.

   @param coro_pimpl
      Pointer to a coroutine (implementation) that’s ready to execute.
   */
   void add_ready(_std::shared_ptr<impl> coro_pimpl);

   /*! Allows other coroutines to run while a delay and/or an asynchronous I/O operation completes, as an
   alternative to blocking while waiting for its completion.

   @param millisecs
      If event_id or fd is provided, then this is the time after which the wait will be interrupted and the
      wait deemed failed, resulting in an exception of type lofty::io::timeout. Otherwise, this is the time
      after which the blocking will end, with no exceptions thrown.
   @param event_id
      Id of an event to wait for, or 0 to not wait for an event.
   @param fd
      File descriptor that the calling coroutine is waiting for I/O on, or lofty::io::filedesc_t_null to not
      wait on I/O.
   @param write
      true if the coroutine is waiting to write to fd, or false if it’s waiting to read from it.
   @param ovl
      (Win32 only) Pointer to the lofty::io::overlapped object that is being used for the asynchronous I/O
      operation.
   */
   void block_active(
      unsigned millisecs, event_id_t event_id, io::filedesc_t fd, bool write
#if LOFTY_HOST_API_WIN32
      , io::overlapped * ovl
#endif
   );

   /*! Coroutine-based implementation of lofty::event::event().

   @return
      Id of the newly-created event.
   */
   event_id_t create_event();

   /*! Coroutine-based implementation of lofty::event::~event().

   @param event_id
      Id of the event to discard.
   */
   void discard_event(event_id_t event_id);

#if LOFTY_HOST_API_WIN32
   /*! Returns the internal IOCP.

   @return
      File descriptor of the internal IOCP.
   */
   io::filedesc_t iocp() const {
      return engine_fd.get();
   }
#endif

   /*! Switches context to the current thread’s own context.

   @param x_type
      Type of exception that escaped the coroutine function, or exception::common_type::none if the function
      returned normally.
   */
   void return_to_scheduler(exception::common_type x_type);

   /*! Begins scheduling and running coroutines on the current thread. Only returns after every coroutine
   added with add_coroutine() returns. */
   void run();

   /*! Coroutine-based implementation of lofty::event::trigger().

   @param event_id
      Id of the event to trigger.
   */
   void trigger_event(event_id_t event_id);

private:
#if LOFTY_HOST_API_LINUX || LOFTY_HOST_API_WIN32
   /*! Arms the internal timer responsible for all time-based waits.

   @param millisecs
      Time in which the timer should fire.
   */
   void arm_timer(time_duration_t millisecs) const;

   /*! Arms the internal timer so that it fires as requested by the next sleeping coroutine. If there are no
   sleeping coroutines, the timer will be disabled. */
   void arm_timer_for_next_sleep_end() const;

   /*! Returns the current time.

   @return
      Current time.
   */
   static time_point_t current_time();
#endif

   /*! Finds a coroutine ready to execute; if none are, but there are blocked coroutines, it blocks the
   current thread until one of them becomes ready.

   @return
      Pointer to a coroutine (implementation) that’s ready to execute.
   */
   _std::shared_ptr<impl> find_coroutine_to_activate();

   /*! Repeatedly finds and runs coroutines that are ready to execute.

   @param interrupting_all
      If true, the loop won’t check for changes to interruption_reason_x_type, assuming that it was
      already != none when the method was called.
   */
   void coroutine_scheduling_loop(bool interrupting_all = false);

   //! Interrupts with interruption_reason_x_type any coroutines associated to the scheduler.
   void interrupt_all();

   /*! Interrupts any coroutines associated to the scheduler. If there’s no previous reason to interrupt all
   coroutines (i.e. if interruption_reason_x_type == none), reason_x_type will be used as the reason.

   @param reason_x_type
      Reason for interruption.
   */
   void interrupt_all(exception::common_type reason_x_type);

   /*! Switches context from the coroutine context pointed to by last_active_coro_pimpl to the current
   thread’s own context.

   @param last_active_coro_pimpl
      Pointer to the coroutine (implementation) that is being inactivated.
   */
   void switch_to_scheduler(impl * last_active_coro_pimpl);

#if LOFTY_HOST_API_WIN32
   //! Waits for timer_fd to fire, posting each firing to the IOCP.
   void timer_thread();

   /*! Invokes coro_sched->timer_thread().

   @param coro_sched
      this.
   @return
      0 if no errors occurred, or 1 if an exception was caught.
   */
   static ::DWORD WINAPI timer_thread_static(void * coro_sched);
#endif

#if LOFTY_HOST_API_LINUX || LOFTY_HOST_API_WIN32
   /*! Unblocks the first coroutine blocked by an event.

   @return
      Pointer to the coroutine’s impl, or nullptr if no coroutines could be unblocked.
   */
   _std::shared_ptr<impl> unblock_by_first_event();
#endif

private:
   //! File descriptor of the internal kqueue (BSD) / epoll (Linux) / IOCP (Win32).
   io::filedesc engine_fd;
#if LOFTY_HOST_API_BSD
   /*! Coroutines that are blocked on a timer wait. The keys are the same as the values, but this can’t be
   changed into a set<shared_ptr<impl>> because we need it to hold a strong reference to the coroutine
   implementation while allowing lookups without having a shared_ptr. */
   collections::hash_map<std::uintptr_t, _std::shared_ptr<impl>> coros_blocked_by_timer_ke;
#elif LOFTY_HOST_API_WIN32
   //! Thread that translates events from event_semaphore_fd into IOCP completions.
   ::HANDLE event_semaphore_thread_handle;
   //! Flag used to tell the event thread to stop looping.
   _std::atomic<bool> stop_event_semaphore_thread;
   //! Thread that translates events from timer_fd into IOCP completions.
   ::HANDLE timer_thread_handle;
   //! Flag used to tell the timer thread to stop looping.
   _std::atomic<bool> stop_timer_thread;
#endif
#if LOFTY_HOST_API_LINUX || LOFTY_HOST_API_WIN32
   //! List of events that need to be processed on the next time the event semaphore unblocks a thread.
   collections::queue<event_id_t> ready_events_queue;
   //! Map of timeouts, in milliseconds, and their associated coroutines.
   collections::trie_ordered_multimap<time_point_t, _std::shared_ptr<impl>> coros_blocked_by_timer_fd;
   //! Semaphore responsible for every event wait.
   io::filedesc event_semaphore_fd;
   //! Timer responsible for every timed wait.
   io::filedesc timer_fd;
#endif
   /*! Coroutines that are blocked on an event wait. Unlike other coros_blocked_by_* members, this one always
   contains all known events; if a value/pointer is nullptr, we’re just keeping track of the event id, but no
   coroutine is currently waiting for that event. */
   collections::hash_map<event_id_t, _std::shared_ptr<impl>> coros_blocked_by_event;
   //! Coroutines that are blocked on a fd wait.
   collections::hash_map<fd_io_key::pack_t, _std::shared_ptr<impl>> coros_blocked_by_fd;
   /*! List of coroutines that are ready to run. Includes coroutines that have been scheduled, but have not
   been started yet. */
   collections::queue<_std::shared_ptr<impl>> ready_coros_queue;
   //! Governs access to ready_coros_queue, coros_blocked_by_fd and other “blocked by” maps/sets.
   _std::mutex coros_add_remove_mutex;
   //! Id of the last event created.
   event_id_t last_created_event_id;
   /*! Set to anything other than exception::common_type::none if a coroutine leaks an uncaught exception, or
   if the scheduler throws an exception while not running coroutines. Once one of these events happens, every
   thread running the scheduler will start interrupting coroutines with this type of exception. */
   _std::atomic<exception::common_type::enum_type> interruption_reason_x_type;

   //! Pointer to the active (current) coroutine, or nullptr if none is active.
   static thread_local_value<_std::shared_ptr<impl>> active_coro_pimpl;
#if LOFTY_HOST_API_POSIX
   //! Pointer to the original context of every thread running a coroutine scheduler.
   static thread_local_value< ::ucontext_t *> default_return_uctx;
#elif LOFTY_HOST_API_WIN32
   //! Handle to the original fiber of every thread running a coroutine scheduler.
   static thread_local_value<void *> return_fiber;
#endif
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_COROUTINE_SCHEDULER_HXX
