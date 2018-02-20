/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/coroutine.hxx>
#include <lofty/defer_to_scope_end.hxx>
#include <lofty/numeric.hxx>
#include "coroutine-scheduler.hxx"

#if LOFTY_HOST_API_POSIX
   #include <errno.h> // EINTR errno
   #include <signal.h> // SIGSTKSZ
   #include <ucontext.h>
   #if LOFTY_HOST_API_BSD
      #include <sys/types.h>
      #include <sys/event.h>
      #include <sys/time.h>
   #elif LOFTY_HOST_API_LINUX
      #include <sys/epoll.h>
      #include <sys/eventfd.h>
      #include <sys/timerfd.h>
      #include <unistd.h> // read() write()
   #endif
#endif
#ifdef COMPLEMAKE_USING_VALGRIND
   #include <valgrind/valgrind.h>
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

class coroutine::impl : public noncopyable {
private:
   friend class coroutine;

public:
   /*! Constructor

   @param main_fn
      Initial value for inner_main_fn.
   */
   impl(_std::function<void ()> main_fn) :
#if LOFTY_HOST_API_POSIX
      stack(SIGSTKSZ),
#elif LOFTY_HOST_API_WIN32
      fiber_(nullptr),
      blocking_ovl(nullptr),
#endif
#ifdef COMPLEMAKE_USING_VALGRIND
      valgrind_stack_id(VALGRIND_STACK_REGISTER(
         stack.get(), static_cast<std::int8_t *>(stack.get()) + stack.size()
      )),
#endif
      blocking_event_id(0),
      blocking_fd(io::filedesc_t_null),
      blocking_time_millisecs(0),
      pending_x_type(exception::common_type::none),
      join_event_ptr(nullptr),
      inner_main_fn(_std::move(main_fn)) {
#if LOFTY_HOST_API_POSIX
      // TODO: use ::mprotect() to setup a guard page for the stack.
   #if LOFTY_HOST_API_DARWIN && LOFTY_HOST_CXX_CLANG
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #endif
      if (::getcontext(&uctx) < 0) {
         exception::throw_os_error();
      }
      uctx.uc_stack.ss_sp = static_cast<char *>(stack.get());
      uctx.uc_stack.ss_size = stack.size();
      uctx.uc_link = nullptr;
      ::makecontext(&uctx, reinterpret_cast<void (*)()>(&outer_main), 1, this);
   #if LOFTY_HOST_API_DARWIN && LOFTY_HOST_CXX_CLANG
      #pragma clang diagnostic pop
   #endif
#elif LOFTY_HOST_API_WIN32
      fiber_ = ::CreateFiber(0, &outer_main, this);
#endif
   }

   //! Destructor.
   ~impl() {
#ifdef COMPLEMAKE_USING_VALGRIND
      VALGRIND_STACK_DEREGISTER(valgrind_stack_id);
#endif
#if LOFTY_HOST_API_WIN32
      if (fiber_) {
         ::DeleteFiber(fiber_);
      }
#endif
   }

#if LOFTY_HOST_API_WIN32
   /*! Returns the internal fiber pointer.

   @return
      Pointer to the coroutine’s fiber.
   */
   void * fiber() {
      return fiber_;
   }
#endif

   /*! Injects the requested type of exception in the coroutine.

   @param this_pimpl
      Shared pointer to *this.
   @param x_type
      Type of exception to inject.
   */
   void inject_exception(_std::shared_ptr<impl> const & this_pimpl, exception::common_type x_type) {
      /* Avoid interrupting the coroutine if there’s already a pending interruption (expected_x_type != none).
      This is not meant to prevent multiple concurrent interruptions (@see interruption-points); this is
      analogous to lofty::thread::interrupt() not trying to prevent multiple concurrent interruptions. In this
      scenario, the compare-and-swap below would succeed, but the coroutine might terminate before
      find_coroutine_to_activate() got to running it (and it would, eventually, since we call add_ready() for
      that), which would be bad. */
      auto expected_x_type = exception::common_type::none;
      if (pending_x_type.compare_exchange_strong(expected_x_type, x_type.base())) {
         /* Mark this coroutine as ready, so it will be scheduler before the scheduler tries to wait for it to
         be unblocked. */
         // TODO: sanity check to avoid scheduling a coroutine twice!
         this_thread::coroutine_scheduler()->add_ready(this_pimpl);
      }
   }

   /*! Called right after each time the coroutine resumes execution and on each interruption point defined by
   this_coroutine::interruption_point(), this will throw an exception of the type specified by
   pending_x_type. */
   void interruption_point() {
      /* This load/store is multithread-safe: the coroutine can only be executing on one thread at a time, and
      the “if” condition being true means that coroutine::interrupt() is preventing other threads from
      changing pending_x_type until we reset it to none. */
      auto x_type = pending_x_type.load();
      if (x_type != exception::common_type::none) {
         pending_x_type.store(exception::common_type::none/*, _std::memory_order_relaxed*/);
         exception::throw_common_type(x_type, 0, 0);
      }
   }

   //! Implementation of the waiting aspect of lofty::coroutine::join().
   void join() {
      event join_event, * curr_join_event_ptr = nullptr;
      if (join_event_ptr.compare_exchange_strong(curr_join_event_ptr, &join_event)) {
         join_event.wait();
      }
      /* If the exchange failed, join_event_ptr has already been assigned a non-nullptr value. Whatever the
      reason, this means the coroutine has already been joined, so it’s okay to just not block. */
   }

   /*! Returns a pointer to the coroutine’s coroutine_local_storage object.

   @return
      Pointer to the coroutine’s crls member.
   */
   _pvt::coroutine_local_storage * local_storage_ptr() {
      return &crls;
   }

#if LOFTY_HOST_API_POSIX
   /*! Returns a pointer to the coroutine’s context.

   @return
      Pointer to the context.
   */
   ::ucontext_t * ucontext_ptr() {
      return &uctx;
   }
#endif

private:
   /*! Lower-level wrapper for the coroutine function passed to coroutine::coroutine().

   @param p
      this.
   */
   static void
#if LOFTY_HOST_API_WIN32
      WINAPI
#endif
   outer_main(void * p);

private:
#if LOFTY_HOST_API_POSIX
   //! Context for the coroutine.
   ::ucontext_t uctx;
   //! Pointer to the memory chunk used as stack.
   memory::pages_ptr stack;
#elif LOFTY_HOST_API_WIN32
   //! Fiber for the coroutine.
   void * fiber_;
   //! Pointer to the OVERLAPPED struct for the outstanding I/O operation on blocking_fd.
   _std::atomic<io::overlapped *> blocking_ovl;
#else
   #error "TODO: HOST_API"
#endif
#ifdef COMPLEMAKE_USING_VALGRIND
   //! Identifier assigned by Valgrind to this coroutine’s stack.
   unsigned valgrind_stack_id;
#endif
   //! Id of the event that is actively blocking the coroutine.
   _std::atomic<scheduler::event_id_t> blocking_event_id;
   //! File descriptor that is actively blocking the coroutine.
   _std::atomic<io::filedesc_t> blocking_fd;
   //! Delay that is currently blocking the coroutine, or timeout for blocking_event_id or blocking_fd.
   _std::atomic<unsigned> blocking_time_millisecs;
   /*! Every time the coroutine is scheduled or returns from an interruption point, this is checked for
   pending exceptions to be injected. */
   _std::atomic<exception::common_type::enum_type> pending_x_type;
   /*! Event triggered after inner_main_fn returns; only non-nullptr while join() is called, or == ~0 after
   the thread passes the point at which it could’ve triggered it (to avoid pointlessly waiting for it). Note
   that this uses the scheduler of the thread calling join(), not the scheduler running inner_main_fn. */
   _std::atomic<event *> join_event_ptr;
   //! Function to be executed in the coroutine.
   _std::function<void ()> inner_main_fn;
   //! Local storage for the coroutine.
   _pvt::coroutine_local_storage crls;
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

coroutine::coroutine() {
}
/*explicit*/ coroutine::coroutine(_std::function<void ()> main_fn) :
   pimpl(_std::make_shared<impl>(_std::move(main_fn))) {
   this_thread::attach_coroutine_scheduler()->add_ready(pimpl);
}

coroutine::~coroutine() {
}

coroutine::id_type coroutine::id() const {
   return reinterpret_cast<id_type>(pimpl.get());
}

void coroutine::interrupt() {
   pimpl->inject_exception(pimpl, exception::common_type::execution_interruption);
}

void coroutine::join() {
   if (!pimpl) {
      // TODO: use a better exception class.
      LOFTY_THROW(argument_error, ());
   }
   // Empty pimpl immediately; this will also make joinable() return false.
   auto pimpl_(_std::move(pimpl));
   pimpl_->join();
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

void to_text_ostream<coroutine>::set_format(str const & format) {
   auto itr(format.cbegin());

   // Add parsing of the format string here.

   throw_on_unused_streaming_format_chars(itr, format);
}

void to_text_ostream<coroutine>::write(coroutine const & src, io::text::ostream * dst) {
   dst->write(LOFTY_SL("CRID:"));
   if (auto id = src.id()) {
      to_text_ostream<decltype(id)>::write(id, dst);
   } else {
      dst->write(LOFTY_SL("-"));
   }
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

thread_local_value<_std::shared_ptr<coroutine::impl>> coroutine::scheduler::active_coro_pimpl;
#if LOFTY_HOST_API_POSIX
thread_local_value< ::ucontext_t *> coroutine::scheduler::default_return_uctx /*= nullptr*/;
#elif LOFTY_HOST_API_WIN32
thread_local_value<void *> coroutine::scheduler::return_fiber /*= nullptr*/;
#endif

coroutine::scheduler::scheduler() :
#if LOFTY_HOST_API_BSD
   // CLOEXEC behavior is implicit.
   engine_fd(::kqueue()),
#elif LOFTY_HOST_API_LINUX
   engine_fd(::epoll_create1(EPOLL_CLOEXEC)),
#elif LOFTY_HOST_API_WIN32
   engine_fd(::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0)),
   non_iocp_events_thread_handle(nullptr),
   stop_non_iocp_events_thread(false),
#endif
   last_created_event_id(0),
   interruption_reason_x_type(exception::common_type::none) {
   if (!engine_fd) {
      exception::throw_os_error();
   }
}

coroutine::scheduler::~scheduler() {
   // TODO: verify that ready_coros_queue and coros_blocked_by_fd (and coros_blocked_by_timer_ke…) are empty.
#if LOFTY_HOST_API_WIN32
   if (non_iocp_events_thread_handle) {
      stop_non_iocp_events_thread.store(true);
      // Wake the thread up one last time to let it know that it’s over.
      arm_timer(0);
      ::ReleaseSemaphore(event_semaphore_fd.get(), 1, nullptr);
      ::WaitForSingleObject(non_iocp_events_thread_handle, INFINITE);
      ::CloseHandle(non_iocp_events_thread_handle);
   }
#endif
}

void coroutine::scheduler::add_ready(_std::shared_ptr<impl> coro_pimpl) {
//   _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
   ready_coros_queue.push_back(_std::move(coro_pimpl));
}

#if LOFTY_HOST_API_LINUX || LOFTY_HOST_API_WIN32
void coroutine::scheduler::arm_timer(time_duration_t millisecs) const {
   /* Since setting the timeout to 0 disables the timer, we’ll set it to the smallest delay possible instead.
   The resolution of the timer is much greater than milliseconds, so the requested sleep duration will be
   essentially honored. */
   #if LOFTY_HOST_API_LINUX
      ::itimerspec sleep_end;
      if (millisecs == 0) {
         // See comment above.
         sleep_end.it_value.tv_sec  = 0;
         sleep_end.it_value.tv_nsec = 1;
      } else {
         sleep_end.it_value.tv_sec  =  millisecs / 1000;
         sleep_end.it_value.tv_nsec = (millisecs % 1000) * 1000000;
      }
      sleep_end.it_interval.tv_sec  = 0;
      sleep_end.it_interval.tv_nsec = 0;
      if (::timerfd_settime(timer_fd.get(), 0, &sleep_end, nullptr) < 0) {
         exception::throw_os_error();
      }
   #elif LOFTY_HOST_API_WIN32
      ::LARGE_INTEGER nanosec_hundreds;
      // Set a relative time (negative) to make the time counting monotonic.
      if (millisecs == 0) {
         // See comment in the beginning of this method.
         nanosec_hundreds.QuadPart = -1;
      } else {
         nanosec_hundreds.QuadPart = static_cast<std::int64_t>(
            static_cast<std::uint64_t>(millisecs)
         ) * -10000;
      }
      if (!::SetWaitableTimer(timer_fd.get(), &nanosec_hundreds, 0, nullptr, nullptr, false)) {
         exception::throw_os_error();
      }
   #endif
}

void coroutine::scheduler::arm_timer_for_next_sleep_end() const {
   if (coros_blocked_by_timer_fd) {
      // Calculate the time at which the earliest sleep end should occur.
      time_point_t now = current_time(), sleep_end = coros_blocked_by_timer_fd.front().key;
      time_duration_t sleep;
      if (now < sleep_end) {
         sleep = static_cast<time_duration_t>(sleep_end - now);
      } else {
         // The timer should’ve already fired by now.
         sleep = 0;
      }
      arm_timer(sleep);
   } else {
      // Stop the timer.
   #if LOFTY_HOST_API_LINUX
      ::itimerspec sleep_end;
      memory::clear(&sleep_end);
      if (::timerfd_settime(timer_fd.get(), 0, &sleep_end, nullptr) < 0) {
         exception::throw_os_error();
      }
   #elif LOFTY_HOST_API_WIN32
      if (!::CancelWaitableTimer(timer_fd.get())) {
         exception::throw_os_error();
      }
   #endif
   }
}
#endif

void coroutine::scheduler::block_active(
   unsigned millisecs, event_id_t event_id, io::filedesc_t fd, bool write
#if LOFTY_HOST_API_WIN32
   , io::overlapped * ovl
#endif
) {
   // TODO: handle millisecs == 0 as a timer-less yield.
   /* TODO: when adding both an event/fd and a timer, there’s a race condition when multiple threads share the
   same scheduler: if the timeout lapses and the coroutine is activated before the fd is removed from the
   waited-on pool, a different thread might wake to serve the fd becoming ready, resuming the coroutine a
   second time. This can be avoided with an atomic “being activated” flag in coroutine::impl. */
   fd_io_key fdiok;
   fdiok.pack = 0;
   fdiok.s.fd = fd;
   fdiok.s.write = write;
   _std::shared_ptr<impl> coro_pimpl(active_coro_pimpl);

   if (event_id) {
      {
//         _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
         coros_blocked_by_event.add_or_assign(event_id, coro_pimpl);
      }
      coro_pimpl->blocking_event_id = event_id;
   }
   LOFTY_DEFER_TO_SCOPE_END(
      /* If the coroutine still thinks it’s blocked upon resuming, the event is still active and must be
      disconnected from it. */
      if (event_id && coro_pimpl->blocking_event_id) {
         coro_pimpl->blocking_event_id = 0;
//         _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
         coros_blocked_by_event.remove(event_id);
      }
   );
#if LOFTY_HOST_API_BSD
   struct ::kevent fd_ke;
   if (fd != io::filedesc_t_null) {
      fd_ke.ident = static_cast<std::uintptr_t>(fd);
      fd_ke.filter = write ? EVFILT_WRITE : EVFILT_READ;
      // Use EV_ONESHOT to avoid waking up multiple threads for the same fd becoming ready.
      fd_ke.flags = EV_ADD | EV_ONESHOT | EV_EOF;
      fd_ke.udata = fdiok.pack;
      if (::kevent(engine_fd.get(), &fd_ke, 1, nullptr, 0, nullptr) < 0) {
         exception::throw_os_error();
      }
      {
//         _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
         coros_blocked_by_fd.add_or_assign(fdiok.pack, coro_pimpl);
      }
      coro_pimpl->blocking_fd = fd;
   }
   LOFTY_DEFER_TO_SCOPE_END(
      /* If the coroutine still thinks it’s blocked upon resuming, the event is still active and must be
      removed. */
      if (fd != io::filedesc_t_null && coro_pimpl->blocking_fd != io::filedesc_t_null) {
         fd_ke.flags = EV_DELETE;
         ::kevent(engine_fd.get(), &fd_ke, 1, nullptr, 0, nullptr);
         coro_pimpl->blocking_fd = io::filedesc_t_null;
//         _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
         coros_blocked_by_fd.remove(fdiok.pack);
      }
   );
   struct ::kevent timer_ke;
   if (millisecs) {
      timer_ke.ident = reinterpret_cast<std::uintptr_t>(coro_pimpl.get());
      timer_ke.filter = EVFILT_TIMER;
      // Use EV_ONESHOT to avoid waking up multiple threads for the same fd becoming ready.
      timer_ke.flags = EV_ADD | EV_ONESHOT;
      timer_ke.data = millisecs;
      // Use the default time unit, milliseconds.
      timer_ke.fflags = 0;
      if (::kevent(engine_fd.get(), &timer_ke, 1, nullptr, 0, nullptr) < 0) {
         exception::throw_os_error();
      }
      {
//      _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
         coros_blocked_by_timer_ke.add_or_assign(timer_ke.ident, coro_pimpl);
      }
      coro_pimpl->blocking_time_millisecs = millisecs;
   }
   LOFTY_DEFER_TO_SCOPE_END(
      /* If the coroutine still thinks it’s blocked upon resuming, the event is still active and must be
      removed. */
      if (millisecs && coro_pimpl->blocking_time_millisecs) {
         timer_ke.flags = EV_DELETE;
         ::kevent(engine_fd.get(), &timer_ke, 1, nullptr, 0, nullptr);
//         _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
         coros_blocked_by_timer_ke.remove(timer_ke.ident);
      }
   );
#elif LOFTY_HOST_API_LINUX || LOFTY_HOST_API_WIN32
   if (fd != io::filedesc_t_null) {
   #if LOFTY_HOST_API_LINUX
      ::epoll_event ee;
      ee.data.u64 = fdiok.pack;
      /* Use EPOLLONESHOT to avoid waking up multiple threads for the same fd becoming ready. This means we’d
      need to then rearm it in find_coroutine_to_activate() when it becomes ready, but we’ll remove it
      instead. */
      ee.events = EPOLLONESHOT | EPOLLPRI | (write ? EPOLLOUT : EPOLLIN);
      if (::epoll_ctl(engine_fd.get(), EPOLL_CTL_ADD, fd, &ee) < 0) {
         exception::throw_os_error();
      }
   #elif LOFTY_HOST_API_WIN32
      /* TODO: ensure bind_to_this_coroutine_scheduler_iocp() has been called on fd. There’s nothing we can do
      about that here, since it’s a non-repeatable operation. */
   #endif
      {
//         _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
         coros_blocked_by_fd.add_or_assign(fdiok.pack, coro_pimpl);
      }
      coro_pimpl->blocking_fd = fd;
   #if LOFTY_HOST_API_WIN32
      coro_pimpl->blocking_ovl = ovl;
   #endif
   }
   #if LOFTY_HOST_API_LINUX
   LOFTY_DEFER_TO_SCOPE_END(
      if (fd != io::filedesc_t_null) {
         // See comment on creation for why we unconditionally remove this.
         ::epoll_ctl(engine_fd.get(), EPOLL_CTL_DEL, fd, nullptr);
         /* If the coroutine still thinks it’s blocked upon resuming, the event is still active and must be
         removed. */
         if (coro_pimpl->blocking_fd != io::filedesc_t_null) {
            coro_pimpl->blocking_fd = io::filedesc_t_null;
//            _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
            coros_blocked_by_fd.remove(fdiok.pack);
         }
      }
   );
   #elif LOFTY_HOST_API_WIN32
   LOFTY_DEFER_TO_SCOPE_END(
      /* If the coroutine still thinks it’s blocked upon resuming, the event is still active and must be
      removed. */
      if (fd != io::filedesc_t_null && coro_pimpl->blocking_fd != io::filedesc_t_null) {
         coro_pimpl->blocking_fd = io::filedesc_t_null;
         /* Cancel the pending I/O operation. Note that this will cancel ALL pending I/O on the fd, not just
         this one; this shouldn’t be a problem because if we’re cancelling this I/O it’s most likely due to
         timeout, and it makes sense to abort all I/O for the fd once one I/O operation on it times out. */
         ::CancelIo(fd);
//         _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
         coros_blocked_by_fd.remove(fdiok.pack);
      }
   );
   #endif
   decltype(coros_blocked_by_timer_fd)::iterator timer_block_itr;
   if (millisecs) {
      if (!timer_fd) {
         // No timer infrastructure yet; set it up now.
   #if LOFTY_HOST_API_LINUX
         timer_fd = io::filedesc(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC));
         if (!timer_fd) {
            exception::throw_os_error();
         }
         ::epoll_event ee;
         memory::clear(&ee.data);
         ee.data.fd = timer_fd.get();
         /* Use EPOLLET to avoid waking up multiple threads for each firing of the timer. If when the timer
         fires there will be multiple coroutines to activate (unlikely), we’ll manually rearm the timer to
         wake up more threads (or wake up the same threads repeatedly) until all coroutines are activated. */
         ee.events = EPOLLET | EPOLLIN;
         if (::epoll_ctl(engine_fd.get(), EPOLL_CTL_ADD, timer_fd.get(), &ee) < 0) {
            exception::throw_os_error();
         }
   #elif LOFTY_HOST_API_WIN32
         setup_non_iocp_events();
   #endif
      }
      {
//      _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
         /* Add the timeout to the timers map, then rearm the timer to ensure the new timeout is accounted
         for. */
         timer_block_itr = coros_blocked_by_timer_fd.add(current_time() + millisecs, coro_pimpl);
         arm_timer_for_next_sleep_end();
      }
      coro_pimpl->blocking_time_millisecs = millisecs;
   }
   LOFTY_DEFER_TO_SCOPE_END(
      /* If the coroutine still thinks it’s blocked upon resuming, the timer is still active and must be
      removed. */
      if (millisecs && coro_pimpl->blocking_time_millisecs) {
//         _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
         coros_blocked_by_timer_fd.remove(timer_block_itr);
         arm_timer_for_next_sleep_end();
      }
   );
#else
   #error "TODO: HOST_API"
#endif

   /* Now that the coroutine is associated to the specified blockers, deactivate it, then switch back to the
   thread’s own context and have it wait for a ready coroutine. */
   active_coro_pimpl.reset();
   switch_to_scheduler(coro_pimpl.get());
   // After returning from that, active_coro_pimpl == coro_pimpl again.

   if (millisecs && !coro_pimpl->blocking_time_millisecs && (
      (event_id && coro_pimpl->blocking_event_id) ||
      (fd != io::filedesc_t_null && coro_pimpl->blocking_fd != io::filedesc_t_null)
   )) {
      /* The coroutine blocked on a wait with a timeout, and the wait is still in progress while the timeout
      expired: convert the timeout into the appropriate type of exception. */
      LOFTY_THROW(io::timeout, ());
   }
}

void coroutine::scheduler::coroutine_scheduling_loop(bool interrupting_all /*= false*/) {
   _std::shared_ptr<impl> & active_coro_pimpl_ = active_coro_pimpl;
   _pvt::coroutine_local_storage * default_crls, ** current_crls;
   _pvt::coroutine_local_storage::get_default_and_current_pointers(&default_crls, &current_crls);
#if LOFTY_HOST_API_POSIX
   ::ucontext_t * return_uctx = default_return_uctx.get();
#endif
   while ((active_coro_pimpl_ = find_coroutine_to_activate())) {
      // Swap the coroutine_local_storage pointer for this thread with that of the active coroutine.
      *current_crls = active_coro_pimpl_->local_storage_ptr();
#if LOFTY_HOST_API_POSIX
      int ret;
#endif
      {
         // Afterwards, restore the coroutine_local_storage pointer for this thread.
         LOFTY_DEFER_TO_SCOPE_END(*current_crls = default_crls);
         // Switch the current thread’s context to the active coroutine’s.
#if LOFTY_HOST_API_POSIX
   #if LOFTY_HOST_API_DARWIN && LOFTY_HOST_CXX_CLANG
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #endif
         ret = ::swapcontext(return_uctx, active_coro_pimpl_->ucontext_ptr());
   #if LOFTY_HOST_API_DARWIN && LOFTY_HOST_CXX_CLANG
      #pragma clang diagnostic pop
   #endif
#elif LOFTY_HOST_API_WIN32
         ::SwitchToFiber(active_coro_pimpl_->fiber());
#else
   #error "TODO: HOST_API"
#endif
      }
#if LOFTY_HOST_API_POSIX
      if (ret < 0) {
         /* TODO: only a stack-related ENOMEM is possible, so throw a stack overflow exception
         (*active_coro_pimpl has a problem, not return_uctx). */
      }
#endif
      /* If a coroutine (in this or another thread) leaked an uncaught exception, terminate all coroutines and
      eventually this very thread. */
      if (!interrupting_all && interruption_reason_x_type.load() != exception::common_type::none) {
         interrupt_all();
         break;
      }
   }
}

coroutine::scheduler::event_id_t coroutine::scheduler::create_event() {
   event_id_t event_id;
   {
//      _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
      // TODO: handle overflow of last_created_event_id.
      event_id = ++last_created_event_id;
   }
#if LOFTY_HOST_API_BSD
   struct ::kevent ke;
   ke.ident = event_id;
   ke.filter = EVFILT_USER;
   ke.flags = EV_ADD;
   ke.fflags = 0;
   if (::kevent(engine_fd.get(), &ke, 1, nullptr, 0, nullptr) < 0) {
      exception::throw_os_error();
   }
#elif LOFTY_HOST_API_LINUX || LOFTY_HOST_API_WIN32
   if (!event_semaphore_fd) {
      // No event infrastructure yet; set it up now.
   #if LOFTY_HOST_API_LINUX
      /* We don’t use EFD_SEMAPHORE because with EPOLLET, it will only wake up a single thread once even if
      the semaphore count is >1. In non-semaphore mode, the one thread being awoken can activate all unblocked
      coroutines, and schedule the first one. */
      event_semaphore_fd = io::filedesc(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC));
      if (!event_semaphore_fd) {
         exception::throw_os_error();
      }
      ::epoll_event ee;
      memory::clear(&ee.data);
      ee.data.fd = event_semaphore_fd.get();
      // Use EPOLLET to avoid waking up multiple threads for each firing of the semaphore.
      ee.events = EPOLLET | EPOLLIN;
      if (::epoll_ctl(engine_fd.get(), EPOLL_CTL_ADD, event_semaphore_fd.get(), &ee) < 0) {
         exception::throw_os_error();
      }
   #elif LOFTY_HOST_API_WIN32
      setup_non_iocp_events();
   #endif
   }
#else
   #error "TODO: HOST_API"
#endif
   return event_id;
}

#if LOFTY_HOST_API_LINUX || LOFTY_HOST_API_WIN32
/*static*/ coroutine::scheduler::time_point_t coroutine::scheduler::current_time() {
   time_point_t now;
   #if LOFTY_HOST_API_LINUX
      ::timespec now_ts;
      ::clock_gettime(CLOCK_MONOTONIC, &now_ts);
      now  = static_cast<time_point_t>(now_ts.tv_sec) * 1000;
      now += static_cast<time_point_t>(now_ts.tv_nsec / 1000000);
   #elif LOFTY_HOST_API_WIN32
      static ::LARGE_INTEGER frequency = {{0, 0}};
      if (frequency.QuadPart == 0) {
         ::QueryPerformanceFrequency(&frequency);
      }
      ::LARGE_INTEGER now_li;
      ::QueryPerformanceCounter(&now_li);
      // TODO: handle wrap-around by keeping a “last now” and adding something if now < “last now”.
      now = now_li.QuadPart * 1000ull / frequency.QuadPart;
   #endif
   return now;
}
#endif

void coroutine::scheduler::discard_event(event_id_t event_id) {
   /* TODO: ensure that a possible blocked coroutine gets released with a timeout or other exception.
   auto blocked_coro(coros_blocked_by_event.find(event_id));
   if (blocked_coro != coros_blocked_by_event.cend()) { … */
#if LOFTY_HOST_API_BSD
   struct ::kevent ke;
   ke.ident = event_id;
   ke.filter = EVFILT_USER;
   ke.flags = EV_DELETE;
   ke.fflags = 0;
   if (::kevent(engine_fd.get(), &ke, 1, nullptr, 0, nullptr) < 0) {
      exception::throw_os_error();
   }
#endif
}

_std::shared_ptr<coroutine::impl> coroutine::scheduler::find_coroutine_to_activate() {
   _std::shared_ptr<coroutine::impl> coro_pimpl;
   // This loop will only repeat in case of EINTR from the blocking-wait API.
   /* TODO: if the epoll/kqueue/IOCP is shared by several threads and one thread receives and removes the last
   event source from it, what happens to the remaining threads?
   a) We could send a no-op signal (SIGCONT?) to all threads using this scheduler, to make the wait function
      return EINTR;
   b) We could have a single event source for each scheduler with the semantics of “event sources changed”, in
      edge-triggered mode so it wakes all waiting threads once, at once. */
   for (;;) {
      {
//         _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
         if (ready_coros_queue) {
            // There are coroutines that are ready to run; remove and return the first.
            coro_pimpl = ready_coros_queue.pop_front();
            break;
         } else if (
            !coros_blocked_by_fd && !coros_blocked_by_event
#if LOFTY_HOST_API_BSD
            && !coros_blocked_by_timer_ke
#elif LOFTY_HOST_API_LINUX || LOFTY_HOST_API_WIN32
            && !coros_blocked_by_timer_fd
#endif
         ) {
            this_thread::interruption_point();
            // No coroutines.
            coro_pimpl = nullptr;
            break;
         }
      }
      /* TODO: FIXME: coros_add_remove_mutex does not protect against race conditions for the “any coroutines
      left?” case. */

      // There are blocked coroutines; wait for the first one to become ready again.
      fd_io_key fdiok;
#if LOFTY_HOST_API_BSD
      struct ::kevent ke;
      if (::kevent(engine_fd.get(), nullptr, 0, &ke, 1, nullptr) < 0) {
         int err = errno;
         /* TODO: EINTR is not a reliable way to interrupt a thread’s ::kevent() call when multiple threads
         share the same coroutine::scheduler. */
         if (err == EINTR) {
            this_thread::interruption_point();
            continue;
         }
         exception::throw_os_error(err);
      }
      // TODO: understand how EV_ERROR works.
      /*if (ke.flags & EV_ERROR) {
         exception::throw_os_error(ke.data);
      }*/
//      _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
      if (ke.filter == EVFILT_TIMER) {
         coro_pimpl = coros_blocked_by_timer_ke.pop(ke.ident);
         // Make the coroutine aware that it’s no longer waiting for the timer.
         coro_pimpl->blocking_time_millisecs = 0;
         break;
      } else if (ke.filter == EVFILT_USER) {
         auto blocked_coro(coros_blocked_by_event.find(ke.ident));
         if (blocked_coro == coros_blocked_by_event.cend()) {
            // The event must’ve been triggered with no coroutines waiting for it.
            continue;
         }
         coro_pimpl = _std::move(blocked_coro->value);
         coros_blocked_by_event.remove(blocked_coro);
         // Make the coroutine aware that it’s no longer waiting for the event.
         coro_pimpl->blocking_event_id = 0;
         break;
      }
      // Otherwise it’s a file descriptor event.
      fdiok.pack = ke.udata;
#elif LOFTY_HOST_API_LINUX || LOFTY_HOST_API_WIN32
   #if LOFTY_HOST_API_LINUX
      ::epoll_event ee;
      if (::epoll_wait(engine_fd.get(), &ee, 1, -1) < 0) {
         int err = errno;
         /* TODO: EINTR is not a reliable way to interrupt a thread’s ::epoll_wait() call when multiple
         threads share the same coroutine::scheduler. This is a problem for Win32 as well (see below), so it
         probably needs a shared solution. */
         if (err == EINTR) {
            this_thread::interruption_point();
            continue;
         }
         exception::throw_os_error(err);
      }
      fdiok.pack = ee.data.u64;
   #elif LOFTY_HOST_API_WIN32
      ::DWORD transferred_byte_size;
      ::OVERLAPPED * ovl;
      if (!::GetQueuedCompletionStatus(
         engine_fd.get(), &transferred_byte_size, &fdiok.pack, &ovl, INFINITE
      )) {
         /* Distinguish between IOCP failures and I/O failures by also checking whether an OVERLAPPED pointer
         was returned. */
         if (!ovl) {
            exception::throw_os_error();
         }
      }

      /* A completion reported on the IOCP itself is used by Lofty to emulate EINTR; see
      lofty::thread::impl::inject_exception(). While we could use a dedicated handle for this purpose, the
      IOCP reporting a completion about itself kind of makes sense. */
      /* TODO: this is not a reliable way to interrupt a thread’s ::GetQueuedCompletionStatus() call when
      multiple threads share the same coroutine::scheduler. This is a problem for POSIX as well (see above),
      so it probably needs a shared solution. */
      if (fdiok.s.fd == engine_fd.get()) {
         this_thread::interruption_point();
         continue;
      }
   #endif
//      _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
      if (fdiok.s.fd == timer_fd.get()) {
         // Pop the coroutine that should run now, and rearm the timer if necessary.
         coro_pimpl = coros_blocked_by_timer_fd.pop_front().value;
         if (coros_blocked_by_timer_fd) {
            arm_timer_for_next_sleep_end();
         }
         // Make the coroutine aware that it’s no longer waiting for the timer.
         coro_pimpl->blocking_time_millisecs = 0;
         break;
      } else if (fdiok.s.fd == event_semaphore_fd.get()) {
   #if LOFTY_HOST_API_LINUX
         /* Reset the semaphore, tracking how many coroutines we need to unblock. This could’ve been made
         easier by EFD_SEMAPHORE, but unfortunately that is broken with EPOLLET (see other comment in this
         file). */
         std::uint64_t unblock_count;
         while (::read(event_semaphore_fd.get(), &unblock_count, sizeof unblock_count) < 0) {
            int err = errno;
            if (err != EINTR) {
               /* This is probably bad, but there’s nothing we can do about it here. Maybe log it? At least we
               make sure that unblock_count gets a known value. */
               unblock_count = 0;
               break;
            }
            this_thread::interruption_point();
         }
   #endif
         coro_pimpl = unblock_by_first_event();
         if (!coro_pimpl) {
            continue;
         }
   #if LOFTY_HOST_API_LINUX
         // Move to ready all other coroutines that this thread was woken up for.
         while (--unblock_count > 0) {
            auto other_coro_pimpl(unblock_by_first_event());
            if (!other_coro_pimpl) {
               break;
            }
            ready_coros_queue.push_back(_std::move(other_coro_pimpl));
         }
   #endif
         break;
      }
#else
   #error "TODO: HOST_API"
#endif
      // Remove and return the coroutine that was waiting for this file descriptor.
      auto blocked_coro(coros_blocked_by_fd.find(fdiok.pack));
      if (blocked_coro != coros_blocked_by_fd.cend()) {
         /* Note (WIN32 BUG?)
         Empirical evidence shows that at this point ovl might not be a valid pointer, even if the completion
         key (fd) returned was a valid Lofty-owned handle. I could not find any explanation for this, but as a
         workaround, each coroutine carries a pointer to the OVERLAPPED it’s waiting on, and we can check
         whether GetOverlappedResult() reports ERROR_IO_INCOMPLETE for it to avoid resuming the coroutine in
         those cases.
         Spurious notifications seem to occur only with sockets:
         •  TCP: when after a completed overlapped read operation, a new overlapped read is requested and
            ReadFile() returns ERROR_IO_PENDING;
         •  UDP: when WSASendTo() fails due to ICMP reporting that the remote server is gone, WSARecvFrom()
            will fail for several different reasons. */
#if LOFTY_HOST_API_WIN32
         if (blocked_coro->value->blocking_ovl.load()->get_result() == ERROR_IO_INCOMPLETE) {
            continue;
         }
#endif
         coro_pimpl = coros_blocked_by_fd.pop(blocked_coro);
         // Make the coroutine aware that it’s no longer waiting for I/O.
         coro_pimpl->blocking_fd = io::filedesc_t_null;
#if LOFTY_HOST_API_WIN32
         coro_pimpl->blocking_ovl = nullptr;
#endif
         break;
      }
      // Else ignore this notification for an event that nobody was waiting for.
      /* TODO: in a Win32 multithreaded scenario, the IOCP notification might arrive to a thread before the
      coroutine blocked itself (on another thread) for the event, due to associating the fd with the IOCP
      before blocking, which is unavoidable and necessary. To address this, requeue the event so it gets
      another chance at being processed. It may be necessary to keep a list of handles that should be held
      until a coroutine blocks for them. */
   }
   return _std::move(coro_pimpl);
}

void coroutine::scheduler::interrupt_all() {
   // Interrupt all coroutines using pending_x_type.
   auto x_type = interruption_reason_x_type.load();
   {
      /* TODO: using a different locking pattern, this work could be split across multiple threads, in case
      multiple are associated to this scheduler. */
//      _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
      LOFTY_FOR_EACH(auto kv, coros_blocked_by_fd) {
         kv.value->inject_exception(kv.value, x_type);
      }
#if LOFTY_HOST_API_BSD
      LOFTY_FOR_EACH(auto kv, coros_blocked_by_timer_ke) {
         kv.value->inject_exception(kv.value, x_type);
      }
#elif LOFTY_HOST_API_LINUX || LOFTY_HOST_API_WIN32
      LOFTY_FOR_EACH(auto kv, coros_blocked_by_timer_fd) {
         kv.value->inject_exception(kv.value, x_type);
      }
#endif
      /* TODO: coroutines currently running on other threads associated to this scheduler won’t have been
      interrupted by the above loops; they need to be stopped by interrupting the threads that are running
      them. */
   }
   /* Run all coroutines. Since they’ve all just been added to ready_coros_queue, they’ll all run and handle
   the interruption request, leaving the epoll/kqueue/IOCP empty, so the latter won’t be checked at all. */
   /* TODO: document that scheduling a new coroutine at this point should be avoided because it breaks the
   interruption guarantee. Maybe actively prevent new coroutines from being scheduled? */
   coroutine_scheduling_loop(true);
}

void coroutine::scheduler::interrupt_all(exception::common_type reason_x_type) {
   /* Try to set interruption_reason_x_type; if that doesn’t happen, it’s because it was already set to none,
   in which case we still go ahead and get to interrupting all coroutines. */
   auto expected_x_type = exception::common_type::none;
   interruption_reason_x_type.compare_exchange_strong(expected_x_type, reason_x_type.base());
   interrupt_all();
}

#if LOFTY_HOST_API_WIN32
void coroutine::scheduler::non_iocp_events_thread() {
   ::HANDLE const handles[2] = { event_semaphore_fd.get(), timer_fd.get() };
   do {
      ::DWORD ret = ::WaitForMultipleObjects(
         LOFTY_COUNTOF(handles), handles, false /*wait for one*/, INFINITE
      );
      if (/*ret >= WAIT_OBJECT_0 &&*/ ret < WAIT_OBJECT_0 + LOFTY_COUNTOF(handles)) {
         ::PostQueuedCompletionStatus(
            engine_fd.get(), 0, reinterpret_cast< ::ULONG_PTR>(handles[ret - WAIT_OBJECT_0]), nullptr
         );
      }
   } while (!stop_non_iocp_events_thread.load());
}

/*static*/ ::DWORD WINAPI coroutine::scheduler::non_iocp_events_thread_static(void * coro_sched) {
   try {
      static_cast<scheduler *>(coro_sched)->non_iocp_events_thread();
   } catch (...) {
      return 1;
   }
   return 0;
}
#endif

void coroutine::scheduler::return_to_scheduler(exception::common_type x_type) {
   /* Only the first uncaught exception in a coroutine can succeed at triggering termination of all
   coroutines. */
   auto expected_x_type = exception::common_type::none;
   interruption_reason_x_type.compare_exchange_strong(expected_x_type, x_type.base());

#if LOFTY_HOST_API_POSIX
   #if LOFTY_HOST_API_DARWIN && LOFTY_HOST_CXX_CLANG
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #endif
   ::setcontext(default_return_uctx.get());
   #if LOFTY_HOST_API_DARWIN && LOFTY_HOST_CXX_CLANG
      #pragma clang diagnostic pop
   #endif
   // Assume ::setcontext() is always successful, in which case it never returns.
   // TODO: maybe issue warning/abort in case ::setcontext() does return?
#elif LOFTY_HOST_API_WIN32
   ::SwitchToFiber(return_fiber.get());
#else
   #error "TODO: HOST_API"
#endif
}

void coroutine::scheduler::run() {
#if LOFTY_HOST_API_POSIX
   ::ucontext_t thread_uctx;
   default_return_uctx = &thread_uctx;
   LOFTY_DEFER_TO_SCOPE_END(default_return_uctx = nullptr);
#elif LOFTY_HOST_API_WIN32
   void * pfbr = ::ConvertThreadToFiber(nullptr);
   if (!pfbr) {
      exception::throw_os_error();
   }
   LOFTY_DEFER_TO_SCOPE_END(::ConvertFiberToThread());
   return_fiber = pfbr;
#else
   #error "TODO: HOST_API"
#endif
   try {
      coroutine_scheduling_loop();
   } catch (_std::exception const & x) {
      interrupt_all(exception::execution_interruption_to_common_type(&x));
      throw;
   } catch (...) {
      interrupt_all(exception::execution_interruption_to_common_type());
      throw;
   }
}

#if LOFTY_HOST_API_WIN32
void coroutine::scheduler::setup_non_iocp_events() {
   event_semaphore_fd = io::filedesc(::CreateSemaphore(nullptr, 0, numeric::max< ::LONG>::value, nullptr));
   if (!event_semaphore_fd) {
      exception::throw_os_error();
   }
   timer_fd = io::filedesc(::CreateWaitableTimer(nullptr, false /*automatic reset*/, nullptr));
   if (!timer_fd) {
      exception::throw_os_error();
   }
   /* Create a thread that will wait for the handles above to fire and post each firing to the IOCP,
   effectively emulating an eventfd and a timerfd. */
   non_iocp_events_thread_handle = ::CreateThread(
      nullptr, 0, &non_iocp_events_thread_static, this, 0, nullptr
   );
   if (!non_iocp_events_thread_handle) {
      exception::throw_os_error();
   }
}
#endif

void coroutine::scheduler::switch_to_scheduler(impl * last_active_coro_pimpl) {
#if LOFTY_HOST_API_POSIX
   #if LOFTY_HOST_API_DARWIN && LOFTY_HOST_CXX_CLANG
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #endif
   if (::swapcontext(last_active_coro_pimpl->ucontext_ptr(), default_return_uctx.get()) < 0) {
   #if LOFTY_HOST_API_DARWIN && LOFTY_HOST_CXX_CLANG
      #pragma clang diagnostic pop
   #endif
      /* TODO: only a stack-related ENOMEM is possible, so throw a stack overflow exception
      (*default_return_uctx has a problem, not *active_coro_pimpl). */
   }
#elif LOFTY_HOST_API_WIN32
   ::SwitchToFiber(return_fiber.get());
#else
   #error "TODO: HOST_API"
#endif
   // Now that we’re back to the coroutine, check for any pending interruptions.
   last_active_coro_pimpl->interruption_point();
}

void coroutine::scheduler::trigger_event(event_id_t event_id) {
#if LOFTY_HOST_API_BSD
   struct ::kevent ke;
   ke.ident = event_id;
   ke.filter = EVFILT_USER;
   // Re-enable the event, and use EV_DISPATCH to only wake up a single thread and the disable the event.
   ke.flags = EV_ENABLE | EV_DISPATCH;
   ke.fflags = NOTE_TRIGGER;
   if (::kevent(engine_fd.get(), &ke, 1, nullptr, 0, nullptr) < 0) {
      exception::throw_os_error();
   }
#elif LOFTY_HOST_API_LINUX || LOFTY_HOST_API_WIN32
   {
//      _std::lock_guard<_std::mutex> lock(coros_add_remove_mutex);
      ready_events_queue.push_back(event_id);
   }
   #if LOFTY_HOST_API_LINUX
      std::uint64_t one = 1;
      ::write(event_semaphore_fd.get(), &one, sizeof one);
   #elif LOFTY_HOST_API_WIN32
      ::ReleaseSemaphore(event_semaphore_fd.get(), 1, nullptr);
   #endif
#else
   #error "TODO: HOST_API"
#endif
}

#if LOFTY_HOST_API_LINUX || LOFTY_HOST_API_WIN32
_std::shared_ptr<coroutine::impl> coroutine::scheduler::unblock_by_first_event() {
   if (!ready_events_queue) {
      // This is probably bad, but there’s nothing we can do about it here. Maybe log it?
      return nullptr;
   }
   auto event_id = ready_events_queue.pop_front();
   auto blocked_coro(coros_blocked_by_event.find(event_id));
   if (blocked_coro == coros_blocked_by_event.cend()) {
      // The event must’ve been triggered with no coroutines waiting for it.
      return nullptr;
   }
   auto coro_pimpl(_std::move(blocked_coro->value));
   coros_blocked_by_event.remove(blocked_coro);
   // Make the coroutine aware that it’s no longer waiting for the event.
   coro_pimpl->blocking_event_id = 0;
   return _std::move(coro_pimpl);
}
#endif

// Now this can be defined.

/*static*/ void coroutine::impl::outer_main(void * p) {
   auto this_pimpl = static_cast<impl *>(p);
   exception::common_type x_type;
   try {
      this_pimpl->inner_main_fn();
      x_type = exception::common_type::none;
   } catch (_std::exception const & x) {
      exception::write_with_scope_trace(nullptr, &x);
      x_type = exception::execution_interruption_to_common_type(&x);
   } catch (...) {
      exception::write_with_scope_trace();
      x_type = exception::execution_interruption_to_common_type();
   }

   /* Try to replace a nullptr with ~nullptr. If the exchange fails, curr_join_event_ptr will receive a
   pointer to an event to trigger. */
   event * curr_join_event_ptr = nullptr;
   if (!this_pimpl->join_event_ptr.compare_exchange_strong(
      curr_join_event_ptr, reinterpret_cast<event *>(~std::uintptr_t(0))
   )) {
      curr_join_event_ptr->trigger();
   }

   this_thread::coroutine_scheduler()->return_to_scheduler(x_type);
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace this_coroutine {

coroutine::id_type id() {
   return reinterpret_cast<coroutine::id_type>(coroutine::scheduler::active_coro_pimpl.get());
}

void interruption_point() {
   if (
      _std::shared_ptr<coroutine::impl> const & active_coro_pimpl_ = coroutine::scheduler::active_coro_pimpl
   ) {
      active_coro_pimpl_->interruption_point();
   }
   this_thread::interruption_point();
}

void sleep_for_ms(unsigned millisecs) {
   if (auto & coro_sched = this_thread::coroutine_scheduler()) {
      coro_sched->block_active(
         millisecs, 0 /*no event*/, io::filedesc_t_null, false /*read – N/A*/
#if LOFTY_HOST_API_WIN32
         , nullptr
#endif
      );
   } else {
      this_thread::sleep_for_ms(millisecs);
   }
}

void sleep_until_fd_ready(
   io::filedesc_t fd, bool write, unsigned timeout_millisecs
#if LOFTY_HOST_API_WIN32
   , io::overlapped * ovl
#endif
) {
   if (auto & coro_sched = this_thread::coroutine_scheduler()) {
      coro_sched->block_active(
         timeout_millisecs, 0 /*no event*/, fd, write
#if LOFTY_HOST_API_WIN32
         , ovl
#endif
      );
   } else {
      this_thread::sleep_until_fd_ready(
         fd, write, timeout_millisecs
#if LOFTY_HOST_API_WIN32
         , ovl
#endif
      );
   }
}

}} //namespace lofty::this_coroutine

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

coroutine_local_storage_registrar::data_members coroutine_local_storage_registrar::data_members_ =
   LOFTY__PVT_CONTEXT_LOCAL_STORAGE_REGISTRAR_INITIALIZER;

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

coroutine_local_storage::coroutine_local_storage() :
   context_local_storage_impl(&coroutine_local_storage_registrar::instance()) {
}

coroutine_local_storage::~coroutine_local_storage() {
   unsigned remaining_attempts = 10;
   bool any_destructed;
   do {
      any_destructed = destruct_vars(coroutine_local_storage_registrar::instance());
   } while (--remaining_attempts > 0 && any_destructed);
}

}} //namespace lofty::_pvt
