/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2016 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#include <abaclade/coroutine.hxx>
#include <abaclade/defer_to_scope_end.hxx>
#include <abaclade/numeric.hxx>
#include "coroutine-scheduler.hxx"

#if ABC_HOST_API_POSIX
   #if ABC_HOST_API_DARWIN
      #define _XOPEN_SOURCE
   #endif
   #include <errno.h> // EINTR errno
   #include <signal.h> // SIGSTKSZ
   #include <ucontext.h>
   #if ABC_HOST_API_BSD
      #include <sys/types.h>
      #include <sys/event.h>
      #include <sys/time.h>
   #elif ABC_HOST_API_LINUX
      #include <sys/epoll.h>
      #include <sys/timerfd.h>
   #endif
#endif
#ifdef ABAMAKE_USING_VALGRIND
   #include <valgrind/valgrind.h>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

class coroutine::impl : public noncopyable {
private:
   friend class coroutine;

public:
   /*! Constructor

   @param fnMain
      Initial value for m_fnInnerMain.
   */
   impl(_std::function<void ()> fnMain) :
#if ABC_HOST_API_POSIX
      m_pStack(SIGSTKSZ),
#elif ABC_HOST_API_WIN32
      m_pfbr(nullptr),
#endif
#ifdef ABAMAKE_USING_VALGRIND
      m_iValgrindStackId(VALGRIND_STACK_REGISTER(
         m_pStack.get(), static_cast<std::int8_t *>(m_pStack.get()) + m_pStack.size()
      )),
#endif
      m_xctPending(exception::common_type::none),
      m_fnInnerMain(_std::move(fnMain)) {
#if ABC_HOST_API_POSIX
      // TODO: use ::mprotect() to setup a guard page for the stack.
   #if ABC_HOST_API_DARWIN && ABC_HOST_CXX_CLANG
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #endif
      if (::getcontext(&m_uctx) < 0) {
         exception::throw_os_error();
      }
      m_uctx.uc_stack.ss_sp = static_cast<char *>(m_pStack.get());
      m_uctx.uc_stack.ss_size = m_pStack.size();
      m_uctx.uc_link = nullptr;
      ::makecontext(&m_uctx, reinterpret_cast<void (*)()>(&outer_main), 1, this);
   #if ABC_HOST_API_DARWIN && ABC_HOST_CXX_CLANG
      #pragma clang diagnostic pop
   #endif
#elif ABC_HOST_API_WIN32
      m_pfbr = ::CreateFiber(0, &outer_main, this);
#endif
   }

   //! Destructor.
   ~impl() {
#ifdef ABAMAKE_USING_VALGRIND
      VALGRIND_STACK_DEREGISTER(m_iValgrindStackId);
#endif
#if ABC_HOST_API_WIN32
      if (m_pfbr) {
         ::DeleteFiber(m_pfbr);
      }
#endif
   }

#if ABC_HOST_API_WIN32
   /*! Returns the internal fiber pointer.

   @return
      Pointer to the coroutine’s fiber.
   */
   void * fiber() {
      return m_pfbr;
   }
#endif

   /*! Injects the requested type of exception in the coroutine.

   @param pimplThis
      Shared pointer to *this.
   @param xct
      Type of exception to inject.
   */
   void inject_exception(_std::shared_ptr<impl> const & pimplThis, exception::common_type xct) {
      ABC_TRACE_FUNC(this, pimplThis, xct);

      /* Avoid interrupting the coroutine if there’s already a pending interruption (xctExpected !=
      none).
      This is not meant to prevent multiple concurrent interruptions, with a second interruption
      occurring after a first one has been thrown; this is analogous to abc::thread::interrupt() not
      trying to prevent multiple concurrent interruptions. In this scenario, the compare-and-swap
      below would succeed, but the coroutine might terminate before find_coroutine_to_activate() got
      to running it (and it would, eventually, since we call add_ready() for that), which would be
      bad. */
      auto xctExpected = exception::common_type::none;
      if (m_xctPending.compare_exchange_strong(xctExpected, xct.base())) {
         /* Mark this coroutine as ready, so it will be scheduler before the scheduler tries to wait
         for it to be unblocked. */
         // TODO: sanity check to avoid scheduling a coroutine twice!
         this_thread::coroutine_scheduler()->add_ready(pimplThis);
      }
   }

   /*! Called right after each time the coroutine resumes execution and on each interruption point
   defined by this_coroutine::interruption_point(), this will throw an exception of the type
   specified by m_xctPending. */
   void interruption_point() {
      /* This load/store is multithread-safe: the coroutine can only be executing on one thread at a
      time, and the “if” condition being true means that coroutine::interrupt() is preventing other
      threads from changing m_xctPending until we reset it to none. */
      auto xct = m_xctPending.load();
      if (xct != exception::common_type::none) {
         m_xctPending.store(exception::common_type::none/*, _std::memory_order_relaxed*/);
         exception::throw_common_type(xct, 0, 0);
      }
   }

   /*! Returns a pointer to the coroutine’s coroutine_local_storage object.

   @return
      Pointer to the coroutine’s m_crls member.
   */
   detail::coroutine_local_storage * local_storage_ptr() {
      return &m_crls;
   }

#if ABC_HOST_API_POSIX
   /*! Returns a pointer to the coroutine’s context.

   @return
      Pointer to the context.
   */
   ::ucontext_t * ucontext_ptr() {
      return &m_uctx;
   }
#endif

private:
   /*! Lower-level wrapper for the coroutine function passed to coroutine::coroutine().

   @param p
      this.
   */
   static void
#if ABC_HOST_API_WIN32
      WINAPI
#endif
   outer_main(void * p);

private:
#if ABC_HOST_API_POSIX
   //! Context for the coroutine.
   ::ucontext_t m_uctx;
   //! Pointer to the memory chunk used as stack.
   memory::pages_ptr m_pStack;
#elif ABC_HOST_API_WIN32
   //! Fiber for the coroutine.
   void * m_pfbr;
#else
   #error "TODO: HOST_API"
#endif
#ifdef ABAMAKE_USING_VALGRIND
   //! Identifier assigned by Valgrind to this coroutine’s stack.
   unsigned m_iValgrindStackId;
#endif
   /*! Every time the coroutine is scheduled or returns from an interruption point, this is checked
   for pending exceptions to be injected. */
   _std::atomic<exception::common_type::enum_type> m_xctPending;
   //! Function to be executed in the coroutine.
   _std::function<void ()> m_fnInnerMain;
   //! Local storage for the coroutine.
   detail::coroutine_local_storage m_crls;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

coroutine::coroutine() {
}
/*explicit*/ coroutine::coroutine(_std::function<void ()> fnMain) :
   m_pimpl(_std::make_shared<impl>(_std::move(fnMain))) {
   this_thread::attach_coroutine_scheduler()->add_ready(m_pimpl);
}

coroutine::~coroutine() {
}

coroutine::id_type coroutine::id() const {
   return reinterpret_cast<id_type>(m_pimpl.get());
}

void coroutine::interrupt() {
   ABC_TRACE_FUNC(this);

   m_pimpl->inject_exception(m_pimpl, exception::common_type::execution_interruption);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

to_str_backend<coroutine>::to_str_backend() {
}

to_str_backend<coroutine>::~to_str_backend() {
}

void to_str_backend<coroutine>::set_format(str const & sFormat) {
   ABC_TRACE_FUNC(this, sFormat);

   auto it(sFormat.cbegin());

   // Add parsing of the format string here.

   // If we still have any characters, they are garbage.
   if (it != sFormat.cend()) {
      ABC_THROW(syntax_error, (
         ABC_SL("unexpected character"), sFormat, static_cast<unsigned>(it - sFormat.cbegin())
      ));
   }
}

void to_str_backend<coroutine>::write(coroutine const & coro, io::text::ostream * ptos) {
   ABC_TRACE_FUNC(this/*, coro*/, ptos);

   if (coroutine::id_type id = coro.id()) {
      m_tsbStr.write(str(ABC_SL("CRID:")), ptos);
      m_tsbId.write(id, ptos);
   } else {
      m_tsbStr.write(str(ABC_SL("CRID:-")), ptos);
   }
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

thread_local_value<_std::shared_ptr<coroutine::impl>> coroutine::scheduler::sm_pcoroimplActive;
#if ABC_HOST_API_POSIX
thread_local_value< ::ucontext_t *> coroutine::scheduler::sm_puctxReturn /*= nullptr*/;
#elif ABC_HOST_API_WIN32
thread_local_value<void *> coroutine::scheduler::sm_pfbrReturn /*= nullptr*/;
#endif

coroutine::scheduler::scheduler() :
#if ABC_HOST_API_BSD
   m_fdKqueue(::kqueue()),
#elif ABC_HOST_API_LINUX
   m_fdEpoll(::epoll_create1(EPOLL_CLOEXEC)),
#elif ABC_HOST_API_WIN32
   m_fdIocp(::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0)),
   m_hTimerThread(nullptr),
   m_bTimerThreadEnd(false),
#endif
   m_xctInterruptionReason(exception::common_type::none) {
#if ABC_HOST_API_BSD
   if (!m_fdKqueue) {
      exception::throw_os_error();
   }
   /* Note that at this point there’s no hack that will ensure a fork()/exec() from another thread
   won’t leak the file descriptor. That’s the whole point of NetBSD’s kqueue1(). */
   m_fdKqueue.set_close_on_exec(true);
#elif ABC_HOST_API_LINUX
   if (!m_fdEpoll) {
      exception::throw_os_error();
   }
#elif ABC_HOST_API_WIN32
   if (!m_fdIocp) {
      exception::throw_os_error();
   }
#endif
}

coroutine::scheduler::~scheduler() {
   /* TODO: verify that m_qReadyCoros and m_hmCorosBlockedByFD (and m_hmCorosBlockedByTimer…) are
   empty. */
#if ABC_HOST_API_WIN32
   if (m_hTimerThread) {
      m_bTimerThreadEnd.store(true);
      // Wake the thread up one last time to let it know that it’s over.
      arm_timer(0);
      ::WaitForSingleObject(m_hTimerThread, INFINITE);
      ::CloseHandle(m_hTimerThread);
   }
#endif
}

void coroutine::scheduler::add_ready(_std::shared_ptr<impl> pcoroimpl) {
   ABC_TRACE_FUNC(this, pcoroimpl);

//   _std::lock_guard<_std::mutex> lock(m_mtxCorosAddRemove);
   m_qReadyCoros.push_back(_std::move(pcoroimpl));
}

#if ABC_HOST_API_LINUX || ABC_HOST_API_WIN32
void coroutine::scheduler::arm_timer(time_duration_t tdMillisecs) const {
   /* Since setting the timeout to 0 disables the timer, we’ll set it to the smallest delay possible
   instead. The resolution of the timer is much greater than milliseconds, so the requested sleep
   duration will be essentially honored. */
   #if ABC_HOST_API_LINUX
      ::itimerspec itsSleepEnd;
      if (tdMillisecs == 0) {
         // See comment above.
         itsSleepEnd.it_value.tv_sec  = 0;
         itsSleepEnd.it_value.tv_nsec = 1;
      } else {
         itsSleepEnd.it_value.tv_sec  =  tdMillisecs / 1000;
         itsSleepEnd.it_value.tv_nsec = (tdMillisecs % 1000) * 1000000;
      }
      itsSleepEnd.it_interval.tv_sec  = 0;
      itsSleepEnd.it_interval.tv_nsec = 0;
      if (::timerfd_settime(m_fdTimer.get(), 0, &itsSleepEnd, nullptr) < 0) {
         exception::throw_os_error();
      }
   #elif ABC_HOST_API_WIN32
      ::LARGE_INTEGER i100nanosecs;
      // Set a relative time (negative) to make the time counting monotonic.
      if (tdMillisecs == 0) {
         // See comment in the beginning of this method.
         i100nanosecs.QuadPart = -1;
      } else {
         i100nanosecs.QuadPart = static_cast<std::int64_t>(
            static_cast<std::uint64_t>(tdMillisecs)
         ) * -10000;
      }
      if (!::SetWaitableTimer(m_fdTimer.get(), &i100nanosecs, 0, nullptr, nullptr, false)) {
         exception::throw_os_error();
      }
   #endif
}

void coroutine::scheduler::arm_timer_for_next_sleep_end() const {
   if (m_tommCorosBlockedByTimer) {
      // Calculate the time at which the earliest sleep end should occur.
      time_point_t tpNow = current_time(), tpSleepEnd = m_tommCorosBlockedByTimer.front().key;
      time_duration_t tdSleep;
      if (tpNow < tpSleepEnd) {
         tdSleep = static_cast<time_duration_t>(tpSleepEnd - tpNow);
      } else {
         // The timer should’ve already fired by now.
         tdSleep = 0;
      }
      arm_timer(tdSleep);
   } else {
      // Stop the timer.
   #if ABC_HOST_API_LINUX
      ::itimerspec itsSleepEnd;
      memory::clear(&itsSleepEnd);
      if (::timerfd_settime(m_fdTimer.get(), 0, &itsSleepEnd, nullptr) < 0) {
         exception::throw_os_error();
      }
   #elif ABC_HOST_API_WIN32
      if (!::CancelWaitableTimer(m_fdTimer.get())) {
         exception::throw_os_error();
      }
   #endif
   }
}
#endif

void coroutine::scheduler::block_active_for_ms(unsigned iMillisecs) {
   ABC_TRACE_FUNC(this, iMillisecs);

   // TODO: handle iMillisecs == 0 as a timer-less yield.

#if ABC_HOST_API_BSD
   impl * pcoroimpl = sm_pcoroimplActive.get();
   struct ::kevent ke;
   ke.ident = reinterpret_cast<std::uintptr_t>(pcoroimpl);
   // Use EV_ONESHOT to avoid waking up multiple threads for the same fd becoming ready.
   ke.flags = EV_ADD | EV_ONESHOT;
   ke.filter = EVFILT_TIMER;
#if ABC_HOST_API_DARWIN
   ke.fflags = NOTE_USECONDS;
   ke.data = iMillisecs * 1000u;
#else
   ke.data = iMillisecs;
#endif
   ::timespec ts = { 0, 0 };
   if (::kevent(m_fdKqueue.get(), &ke, 1, nullptr, 0, &ts) < 0) {
      exception::throw_os_error();
   }
   // Deactivate the current coroutine and find one to activate instead.
   {
//      _std::lock_guard<_std::mutex> lock(m_mtxCorosAddRemove);
      m_hmCorosBlockedByTimer.add_or_assign(ke.ident, _std::move(sm_pcoroimplActive));
   }
   try {
      // Switch back to the thread’s own context and have it wait for a ready coroutine.
      switch_to_scheduler(pcoroimpl);
   } catch (...) {
      // If anything went wrong or the coroutine was terminated, remove the timer.
      {
//         _std::lock_guard<_std::mutex> lock(m_mtxCorosAddRemove);
         m_hmCorosBlockedByTimer.remove(ke.ident);
      }
      ke.flags = EV_DELETE;
      ::kevent(m_fdKqueue.get(), &ke, 1, nullptr, 0, &ts);
      throw;
   }
#elif ABC_HOST_API_LINUX || ABC_HOST_API_WIN32
   if (!m_fdTimer) {
      // No timer infrastructure yet; set it up now.
   #if ABC_HOST_API_LINUX
      m_fdTimer = io::filedesc(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC));
      if (!m_fdTimer) {
         exception::throw_os_error();
      }
      ::epoll_event ee;
      memory::clear(&ee.data);
      ee.data.fd = m_fdTimer.get();
      /* Use EPOLLET to avoid waking up multiple threads for each firing of the timer. If when the
      timer fires there will be multiple coroutines to activate (unlikely), we’ll manually rearm the
      timer to wake up more threads (or wake up the same threads repeatedly) until all coroutines
      are activated. */
      ee.events = EPOLLET | EPOLLIN;
      if (::epoll_ctl(m_fdEpoll.get(), EPOLL_CTL_ADD, m_fdTimer.get(), &ee) < 0) {
         exception::throw_os_error();
      }
   #elif ABC_HOST_API_WIN32
      m_fdTimer = io::filedesc(::CreateWaitableTimer(nullptr, false, nullptr));
      if (!m_fdTimer) {
         exception::throw_os_error();
      }
      /* Create a thread that will wait for the timer to fire and post each firing to the IOCP,
      effectively emulating a timerfd. */
      m_hTimerThread = ::CreateThread(nullptr, 0, &timer_thread_static, this, 0, nullptr);
      if (!m_hTimerThread) {
         exception::throw_os_error();
      }
   #endif
   }
   // Calculate the time at which this timer should fire.
   time_point_t tpSleepEndMillisecs = current_time() + iMillisecs;

   // Extract the next timeout from the map.
   time_point_t tpNextSleepEnd;
   if (m_tommCorosBlockedByTimer) {
      tpNextSleepEnd = m_tommCorosBlockedByTimer.front().key;
   } else {
      tpNextSleepEnd = numeric::max<time_point_t>::value;
   }

   // Move the active coroutine to the map.
   auto it(m_tommCorosBlockedByTimer.add(tpSleepEndMillisecs, _std::move(sm_pcoroimplActive)));
   try {
      // If the calculated time is sooner than the next timeout, rearm the timer.
      if (tpSleepEndMillisecs < tpNextSleepEnd) {
         arm_timer(iMillisecs);
      }
      // Switch back to the thread’s own context and have it wait for a ready coroutine.
      switch_to_scheduler(it->value.get());
   } catch (...) {
      /* Remove the coroutine from the map of blocked ones and rearm the timer if there are sleepers
      left. */
      m_tommCorosBlockedByTimer.remove(it);
      arm_timer_for_next_sleep_end();
      throw;
   }
#else
   #error "TODO: HOST_API"
#endif
}

void coroutine::scheduler::block_active_until_fd_ready(
   io::filedesc_t fd, bool bWrite
#if ABC_HOST_API_WIN32
   , io::overlapped * povl
#endif
) {
#if ABC_HOST_API_WIN32
   ABC_TRACE_FUNC(this, fd, bWrite, povl);
#else
   ABC_TRACE_FUNC(this, fd, bWrite);
#endif

   // Add fd as a new event source.
#if ABC_HOST_API_BSD
   struct ::kevent ke;
   ke.ident = static_cast<std::uintptr_t>(fd);
   // Use EV_ONESHOT to avoid waking up multiple threads for the same fd becoming ready.
   ke.flags = EV_ADD | EV_ONESHOT | EV_EOF;
   ke.filter = bWrite ? EVFILT_WRITE : EVFILT_READ;
   ::timespec ts = { 0, 0 };
   if (::kevent(m_fdKqueue.get(), &ke, 1, nullptr, 0, &ts) < 0) {
      exception::throw_os_error();
   }
#elif ABC_HOST_API_LINUX
   ::epoll_event ee;
   memory::clear(&ee.data);
   ee.data.fd = fd;
   /* Use EPOLLONESHOT to avoid waking up multiple threads for the same fd becoming ready. This
   means we’d need to then rearm it in find_coroutine_to_activate() when it becomes ready, but we’ll
   remove it instead. */
   ee.events = EPOLLONESHOT | EPOLLPRI | (bWrite ? EPOLLOUT : EPOLLIN);
   if (::epoll_ctl(m_fdEpoll.get(), EPOLL_CTL_ADD, fd, &ee) < 0) {
      exception::throw_os_error();
   }
   auto deferred1(defer_to_scope_end([this, fd] () {
      // Remove fd from the epoll. Ignore errors since we wouldn’t know what to do about them.
      ::epoll_ctl(m_fdEpoll.get(), EPOLL_CTL_DEL, fd, nullptr);
   }));
#elif ABC_HOST_API_WIN32
   // TODO: ensure bind_to_this_coroutine_scheduler_iocp() has been called on fd.
   // This may repeat in case of spurious notifications by the IOCP for fd (WIN32 BUG?).
   do {
#else
   #error "TODO: HOST_API"
#endif
      // Deactivate the current coroutine and find one to activate instead.
      impl * pcoroimpl;
      {
//         _std::lock_guard<_std::mutex> lock(m_mtxCorosAddRemove);
         pcoroimpl = _std::get<0>(m_hmCorosBlockedByFD.add_or_assign(
            fd, _std::move(sm_pcoroimplActive)
         ))->value.get();
      }
      try {
         // Switch back to the thread’s own context and have it wait for a ready coroutine.
         switch_to_scheduler(pcoroimpl);
      } catch (...) {
#if ABC_HOST_API_WIN32
         /* Cancel the pending I/O operation. Note that this will cancel ALL pending I/O on the
         file, not just this one. */
         ::CancelIo(fd);
#endif
         // Remove the coroutine from the map of blocked ones.
//         _std::lock_guard<_std::mutex> lock(m_mtxCorosAddRemove);
         m_hmCorosBlockedByFD.remove(fd);
         throw;
      }
      // Under Linux, deferred1 will remove the now-inactive event for fd.
#if ABC_HOST_API_WIN32
   } while (povl->get_result() == ERROR_IO_INCOMPLETE);
#endif
}

void coroutine::scheduler::coroutine_scheduling_loop(bool bInterruptingAll /*= false*/) {
   _std::shared_ptr<impl> & pcoroimplActive = sm_pcoroimplActive;
   detail::coroutine_local_storage * pcrlsDefault, ** ppcrlsCurrent;
   detail::coroutine_local_storage::get_default_and_current_pointers(&pcrlsDefault, &ppcrlsCurrent);
#if ABC_HOST_API_POSIX
   ::ucontext_t * puctxReturn = sm_puctxReturn.get();
#endif
   while ((pcoroimplActive = find_coroutine_to_activate())) {
      // Swap the coroutine_local_storage pointer for this thread with that of the active coroutine.
      *ppcrlsCurrent = pcoroimplActive->local_storage_ptr();
#if ABC_HOST_API_POSIX
      int iRet;
#endif
      {
         auto deferred1(defer_to_scope_end([ppcrlsCurrent, pcrlsDefault] () {
            // Restore the coroutine_local_storage pointer for this thread.
            *ppcrlsCurrent = pcrlsDefault;
         }));
         // Switch the current thread’s context to the active coroutine’s.
#if ABC_HOST_API_POSIX
   #if ABC_HOST_API_DARWIN && ABC_HOST_CXX_CLANG
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #endif
         iRet = ::swapcontext(puctxReturn, pcoroimplActive->ucontext_ptr());
   #if ABC_HOST_API_DARWIN && ABC_HOST_CXX_CLANG
      #pragma clang diagnostic pop
   #endif
#elif ABC_HOST_API_WIN32
         ::SwitchToFiber(pcoroimplActive->fiber());
#else
   #error "TODO: HOST_API"
#endif
         // deferred1 will restore the coroutine_local_storage pointer for this thread.
      }
#if ABC_HOST_API_POSIX
      if (iRet < 0) {
         /* TODO: only a stack-related ENOMEM is possible, so throw a stack overflow exception
         (*sm_pcoroimplActive has a problem, not uctxReturn). */
      }
#endif
      /* If a coroutine (in this or anothre thread) leaked an uncaught exception, terminate all
      coroutines and eventually this very thread. */
      if (!bInterruptingAll && m_xctInterruptionReason.load() != exception::common_type::none) {
         interrupt_all();
         break;
      }
   }
}

#if ABC_HOST_API_LINUX || ABC_HOST_API_WIN32
/*static*/ coroutine::scheduler::time_point_t coroutine::scheduler::current_time() {
   time_point_t tpNow;
   #if ABC_HOST_API_LINUX
      ::timespec tsNow;
      ::clock_gettime(CLOCK_MONOTONIC, &tsNow);
      tpNow  = static_cast<time_point_t>(tsNow.tv_sec) * 1000;
      tpNow += static_cast<time_point_t>(tsNow.tv_nsec / 1000000);
   #elif ABC_HOST_API_WIN32
      static ::LARGE_INTEGER s_iFreq = {{0, 0}};
      if (s_iFreq.QuadPart == 0) {
         ::QueryPerformanceFrequency(&s_iFreq);
      }
      ::LARGE_INTEGER iNow;
      ::QueryPerformanceCounter(&iNow);
      // TODO: handle wrap-around by keeping tpLast and adding something if tpNow < tpLast.
      tpNow = iNow.QuadPart * 1000ull / s_iFreq.QuadPart;
   #endif
   return tpNow;
}
#endif

_std::shared_ptr<coroutine::impl> coroutine::scheduler::find_coroutine_to_activate() {
   ABC_TRACE_FUNC(this);

   // This loop will only repeat in case of EINTR from the blocking-wait API.
   /* TODO: if the epoll/kqueue/IOCP is shared by several threads and one thread receives and
   removes the last event source from it, what happens to the remaining threads?
   a) We could send a no-op signal (SIGCONT?) to all threads using this scheduler, to make the wait
      function return EINTR;
   b) We could have a single event source for each scheduler with the semantics of “event sources
      changed”, in edge-triggered mode so it wakes all waiting threads once, at once. */
   for (;;) {
      {
//         _std::lock_guard<_std::mutex> lock(m_mtxCorosAddRemove);
         if (m_qReadyCoros) {
            // There are coroutines that are ready to run; remove and return the first.
            return m_qReadyCoros.pop_front();
         } else if (
            !m_hmCorosBlockedByFD
#if ABC_HOST_API_BSD
            && !m_hmCorosBlockedByTimer
#elif ABC_HOST_API_LINUX || ABC_HOST_API_WIN32
            && !m_tommCorosBlockedByTimer
#endif
         ) {
            this_thread::interruption_point();
            return nullptr;
         }
      }
      /* TODO: FIXME: m_mtxCorosAddRemove does not protect against race conditions for the “any
      coroutines left?” case. */

      // There are blocked coroutines; wait for the first one to become ready again.
#if ABC_HOST_API_BSD
      struct ::kevent ke;
      if (::kevent(m_fdKqueue.get(), nullptr, 0, &ke, 1, nullptr) < 0) {
         int iErr = errno;
         /* TODO: EINTR is not a reliable way to interrupt a thread’s ::kevent() call when multiple
         threads share the same coroutine::scheduler. */
         if (iErr == EINTR) {
            this_thread::interruption_point();
            continue;
         }
         exception::throw_os_error(iErr);
      }
      // TODO: understand how EV_ERROR works.
      /*if (ke.flags & EV_ERROR) {
         exception::throw_os_error(ke.data);
      }*/
//      _std::lock_guard<_std::mutex> lock(m_mtxCorosAddRemove);
      if (ke.filter == EVFILT_TIMER) {
         // Remove and return the coroutine that was waiting for this timer.
         return m_hmCorosBlockedByTimer.pop(ke.ident);
      }
      io::filedesc_t fd = static_cast<io::filedesc_t>(ke.ident);
#elif ABC_HOST_API_LINUX || ABC_HOST_API_WIN32
   #if ABC_HOST_API_LINUX
      ::epoll_event ee;
      if (::epoll_wait(m_fdEpoll.get(), &ee, 1, -1) < 0) {
         int iErr = errno;
         /* TODO: EINTR is not a reliable way to interrupt a thread’s ::epoll_wait() call when
         multiple threads share the same coroutine::scheduler. */
         if (iErr == EINTR) {
            this_thread::interruption_point();
            continue;
         }
         exception::throw_os_error(iErr);
      }
      io::filedesc_t fd = ee.data.fd;
   #elif ABC_HOST_API_WIN32
      ::DWORD cbTransferred;
      ::ULONG_PTR iCompletionKey;
      ::OVERLAPPED * povl;
      if (!::GetQueuedCompletionStatus(
         m_fdIocp.get(), &cbTransferred, &iCompletionKey, &povl, INFINITE
      )) {
         /* Distinguish between IOCP failures and I/O failures by also checking whether an
         OVERLAPPED pointer was returned. */
         if (!povl) {
            exception::throw_os_error();
         }
      }
      io::filedesc_t fd = reinterpret_cast< ::HANDLE>(iCompletionKey);
      /* Note (WIN32 BUG?)
      Empirical evidence shows that at this point, povl might not be a valid pointer, even if the
      completion key (fd) returned was a valid Abaclade-owned handle. I could not find any
      explanation for this, but at least the caller of sleep_until_fd_ready() will be able to detect
      the spurious notification by GetOverlappedResult() setting the last error to
      ERROR_IO_INCOMPLETE.
      Spurious notifications seem to occur predictably with sockets when, after a completed
      overlapped read, a new overlapped read is requested and ReadFile() return ERROR_IO_PENDING. */

      // A completion reported on the IOCP itself is used by Abaclade to emulate EINTR.
      /* TODO: this is not a reliable way to interrupt a thread’s ::GetQueuedCompletionStatus() call
      when multiple threads share the same coroutine::scheduler. */
      if (fd == m_fdIocp.get()) {
         this_thread::interruption_point();
         continue;
      }
   #endif
//      _std::lock_guard<_std::mutex> lock(m_mtxCorosAddRemove);
      if (fd == m_fdTimer.get()) {
         // Pop the coroutine that should run now, and rearm the timer if necessary.
         auto kv(m_tommCorosBlockedByTimer.pop_front());
         if (m_tommCorosBlockedByTimer) {
            arm_timer_for_next_sleep_end();
         }
         // Return the coroutine that was waiting for the timer.
         return _std::move(kv.value);
      }
#else
   #error "TODO: HOST_API"
#endif
      // Remove and return the coroutine that was waiting for this file descriptor.
      auto itBlockedCoro(m_hmCorosBlockedByFD.find(fd));
      if (itBlockedCoro != m_hmCorosBlockedByFD.cend()) {
         return m_hmCorosBlockedByFD.pop(itBlockedCoro);
      }
      // Else ignore this notification for an event that nobody was waiting for.
      /* TODO: in a Win32 multithreaded scenario, the IOCP notification might arrive to a thread
      before the coroutine blocked itself (on another thread) for the event due, to associating the
      fd with the IOCP before blocking, which is unavoidable and necessary.
      To address this, requeue the event so it gets another chance at being processed. It may be
      necessary to keep a list of handles that should be helf until a coroutine blocks for them. */
   }
}

void coroutine::scheduler::interrupt_all() {
   // Interrupt all coroutines using m_xctPending.
   auto xctInterruptionReason = m_xctInterruptionReason.load();
   {
      /* TODO: using a different locking pattern, this work could be split across multiple threads,
      in case multiple are associated to this scheduler. */
//      _std::lock_guard<_std::mutex> lock(m_mtxCorosAddRemove);
      ABC_FOR_EACH(auto kv, m_hmCorosBlockedByFD) {
         kv.value->inject_exception(kv.value, xctInterruptionReason);
      }
#if ABC_HOST_API_BSD
      ABC_FOR_EACH(auto kv, m_hmCorosBlockedByTimer) {
         kv.value->inject_exception(kv.value, xctInterruptionReason);
      }
#elif ABC_HOST_API_LINUX || ABC_HOST_API_WIN32
      ABC_FOR_EACH(auto kv, m_tommCorosBlockedByTimer) {
         kv.value->inject_exception(kv.value, xctInterruptionReason);
      }
#endif
      /* TODO: coroutines currently running on other threads associated to this scheduler won’t have
      been interrupted by the above loops; they need to be stopped by interrupting the threads that
      are running them. */
   }
   /* Run all coroutines. Since they’ve all just been added to m_qReadyCoros, they’ll all run and
   handle the interruption request, leaving the epoll/kqueue/IOCP empty, so the latter won’t be
   checked at all. */
   /* TODO: document that scheduling a new coroutine at this point should be avoided because it
   breaks the interruption guarantee. Maybe actively prevent new coroutines from being scheduled? */
   coroutine_scheduling_loop(true);
}

void coroutine::scheduler::interrupt_all(exception::common_type xctReason) {
   /* Try to set m_xctInterruptionReason; if that doesn’t happen, it’s because it was already set to
   none, in which case we still go ahead and get to interrupting all coroutines. */
   auto xctExpected = exception::common_type::none;
   m_xctInterruptionReason.compare_exchange_strong(xctExpected, xctReason.base());
   interrupt_all();
}

void coroutine::scheduler::return_to_scheduler(exception::common_type xct) {
   /* Only the first uncaught exception in a coroutine can succeed at triggering termination of all
   coroutines. */
   auto xctExpected = exception::common_type::none;
   m_xctInterruptionReason.compare_exchange_strong(xctExpected, xct.base());

#if ABC_HOST_API_POSIX
   #if ABC_HOST_API_DARWIN && ABC_HOST_CXX_CLANG
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #endif
   ::setcontext(sm_puctxReturn.get());
   #if ABC_HOST_API_DARWIN && ABC_HOST_CXX_CLANG
      #pragma clang diagnostic pop
   #endif
   // Assume ::setcontext() is always successful, in which case it never returns.
   // TODO: maybe issue warning/abort in case ::setcontext() does return?
#elif ABC_HOST_API_WIN32
   ::SwitchToFiber(sm_pfbrReturn.get());
#else
   #error "TODO: HOST_API"
#endif
}

void coroutine::scheduler::run() {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_API_POSIX
   ::ucontext_t uctxReturn;
   sm_puctxReturn = &uctxReturn;
   auto deferred1(defer_to_scope_end([] () {
      sm_puctxReturn = nullptr;
   }));
#elif ABC_HOST_API_WIN32
   void * pfbr = ::ConvertThreadToFiber(nullptr);
   if (!pfbr) {
      exception::throw_os_error();
   }
   auto deferred1(defer_to_scope_end([] () {
      ::ConvertFiberToThread();
   }));
   sm_pfbrReturn = pfbr;
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
   // Under POSIX, deferred1 will reset sm_puctxReturn to nullptr.
   // Under Win32, deferred1 will convert the current fiber back into a thread.
}

void coroutine::scheduler::switch_to_scheduler(impl * pcoroimplLastActive) {
#if ABC_HOST_API_POSIX
   #if ABC_HOST_API_DARWIN && ABC_HOST_CXX_CLANG
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #endif
   if (::swapcontext(pcoroimplLastActive->ucontext_ptr(), sm_puctxReturn.get()) < 0) {
   #if ABC_HOST_API_DARWIN && ABC_HOST_CXX_CLANG
      #pragma clang diagnostic pop
   #endif
      /* TODO: only a stack-related ENOMEM is possible, so throw a stack overflow exception
      (*sm_puctxReturn has a problem, not *sm_pcoroimplActive). */
   }
#elif ABC_HOST_API_WIN32
   ::SwitchToFiber(sm_pfbrReturn.get());
#else
   #error "TODO: HOST_API"
#endif
   // Now that we’re back to the coroutine, check for any pending interruptions.
   pcoroimplLastActive->interruption_point();
}

#if ABC_HOST_API_WIN32
void coroutine::scheduler::timer_thread() {
   do {
      if (::WaitForSingleObject(m_fdTimer.get(), INFINITE) == WAIT_OBJECT_0) {
         ::PostQueuedCompletionStatus(
            m_fdIocp.get(), 0, reinterpret_cast< ::ULONG_PTR>(m_fdTimer.get()), nullptr
         );
      }
   } while (!m_bTimerThreadEnd.load());
}

/*static*/ ::DWORD WINAPI coroutine::scheduler::timer_thread_static(void * pThis) {
   try {
      static_cast<scheduler *>(pThis)->timer_thread();
   } catch (...) {
      return 1;
   }
   return 0;
}
#endif


// Now this can be defined.

/*static*/ void coroutine::impl::outer_main(void * p) {
   impl * pimplThis = static_cast<impl *>(p);
   // Assume for now that m_fnInnerMain will return without exceptions.
   exception::common_type xct = exception::common_type::none;
   try {
      pimplThis->m_fnInnerMain();
   } catch (_std::exception const & x) {
      exception::write_with_scope_trace(nullptr, &x);
      xct = exception::execution_interruption_to_common_type(&x);
   } catch (...) {
      exception::write_with_scope_trace();
      xct = exception::execution_interruption_to_common_type();
   }
   this_thread::coroutine_scheduler()->return_to_scheduler(xct);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace this_coroutine {

coroutine::id_type id() {
   return reinterpret_cast<coroutine::id_type>(coroutine::scheduler::sm_pcoroimplActive.get());
}

void interruption_point() {
   if (
      _std::shared_ptr<coroutine::impl> const & pcoroimplActive =
         coroutine::scheduler::sm_pcoroimplActive
   ) {
      pcoroimplActive->interruption_point();
   }
   this_thread::interruption_point();
}

void sleep_for_ms(unsigned iMillisecs) {
   if (auto & pcorosched = this_thread::coroutine_scheduler()) {
      pcorosched->block_active_for_ms(iMillisecs);
   } else {
      this_thread::sleep_for_ms(iMillisecs);
   }
}

void sleep_until_fd_ready(
   io::filedesc_t fd, bool bWrite
#if ABC_HOST_API_WIN32
   , io::overlapped * povl
#endif
) {
   if (auto & pcorosched = this_thread::coroutine_scheduler()) {
      pcorosched->block_active_until_fd_ready(
         fd, bWrite
#if ABC_HOST_API_WIN32
         , povl
#endif
      );
   } else {
      this_thread::sleep_until_fd_ready(
         fd, bWrite
#if ABC_HOST_API_WIN32
         , povl
#endif
      );
   }
}

}} //namespace abc::this_coroutine
