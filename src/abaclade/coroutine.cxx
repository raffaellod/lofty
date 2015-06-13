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

#include <abaclade.hxx>
#include <abaclade/coroutine.hxx>
#include <abaclade/defer_to_scope_end.hxx>
#include "coroutine-scheduler.hxx"

#include <atomic>

#if ABC_HOST_API_POSIX
   #if ABC_HOST_API_DARWIN
      #define _XOPEN_SOURCE
   #endif
   #include <errno.h> // EINTR errno
   #include <signal.h> // SIGSTKSZ
   #include <sys/poll.h>
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
// abc::coroutine::impl

namespace abc {

class coroutine::impl : public noncopyable {
private:
   friend class coroutine;

public:
   /*! Constructor

   @param fnMain
      Initial value for m_fnInnerMain.
   */
   impl(std::function<void ()> fnMain) :
#if ABC_HOST_API_POSIX
      m_pStack(SIGSTKSZ),
#elif ABC_HOST_API_WIN32
      m_pfbr(::CreateFiber(0, &outer_main, this)),
#endif
#ifdef ABAMAKE_USING_VALGRIND
      m_iValgrindStackId(VALGRIND_STACK_REGISTER(
         m_pStack.get(), static_cast<std::int8_t *>(m_pStack.get()) + m_pStack.size()
      )),
#endif
      m_xctPending(exception::common_type::none),
      m_fnInnerMain(std::move(fnMain)),
      m_crls(false /*don’t automatically install as the current thread’s TLS instance*/) {
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

   /*! Injects the requested type of exception in the thread.

   @param pimplThis
      Shared pointer to *this.
   @param xct
      Type of exception to inject.
   */
   void inject_exception(std::shared_ptr<impl> const & pimplThis, exception::common_type xct) {
      ABC_TRACE_FUNC(this);

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

   /*! Returns a pointer to the coroutine’s coroutine_local_storage object.

   @return
      Pointer to the coroutine’s m_crls member.
   */
   detail::coroutine_local_storage * local_storage_ptr() {
      return &m_crls;
   }

   /*! Called right after each time the coroutine resumes execution, this will throw an exception of
   the type specified by m_xctPending. This kind of exceptions are injected by other coroutines or
   other Abaclade implementation code. */
   void throw_if_any_pending_exception() {
      /* This load/store is multithread-safe: the coroutine can only be executing on one thread at a
      time, and the “if” condition being true means that coroutine::interrupt() is preventing other
      threads from changing m_xctPending until we reset it to none. */
      auto xct = m_xctPending.load();
      if (xct != exception::common_type::none) {
         m_xctPending.store(exception::common_type::none, std::memory_order_relaxed);
         exception::throw_common_type(xct, 0, 0);
      }
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
   static void outer_main(void * p);

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
   //! Every time the coroutine is scheduled, this is checked for pending exceptions to be injected.
   std::atomic<exception::common_type::enum_type> m_xctPending;
   //! Function to be executed in the coroutine.
   std::function<void ()> m_fnInnerMain;
   //! Local storage for the coroutine.
   detail::coroutine_local_storage m_crls;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::coroutine

namespace abc {

coroutine::coroutine() {
}
/*explicit*/ coroutine::coroutine(std::function<void ()> fnMain) :
   m_pimpl(std::make_shared<impl>(std::move(fnMain))) {
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
// abc::to_str_backend – specialization for abc::coroutine

namespace abc {

to_str_backend<coroutine>::to_str_backend() {
}

to_str_backend<coroutine>::~to_str_backend() {
}

void to_str_backend<coroutine>::set_format(istr const & sFormat) {
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

void to_str_backend<coroutine>::write(coroutine const & coro, io::text::writer * ptwOut) {
   ABC_TRACE_FUNC(this/*, coro*/, ptwOut);

   if (coroutine::id_type id = coro.id()) {
      m_tsbStr.write(istr(ABC_SL("CRID:")), ptwOut);
      m_tsbId.write(id, ptwOut);
   } else {
      m_tsbStr.write(istr(ABC_SL("CRID:-")), ptwOut);
   }
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::coroutine::scheduler

namespace abc {

thread_local_value<std::shared_ptr<coroutine::impl>> coroutine::scheduler::sm_pcoroimplActive;
thread_local_value<std::shared_ptr<coroutine::scheduler>> coroutine::scheduler::sm_pcorosched;
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
#endif
   m_xctInterruptionReason(exception::common_type::none) {
   ABC_TRACE_FUNC(this);

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
   /* TODO: verify that m_listReadyCoros and m_mapCorosBlockedByFD (and m_mapCorosBlockedByTimer…)
   are empty. */
}

void coroutine::scheduler::add_ready(std::shared_ptr<impl> pcoroimpl) {
   ABC_TRACE_FUNC(this, pcoroimpl);

//   std::lock_guard<std::mutex> lock(m_mtxCorosAddRemove);
   m_listReadyCoros.push_back(std::move(pcoroimpl));
}

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
//      std::lock_guard<std::mutex> lock(m_mtxCorosAddRemove);
      m_mapCorosBlockedByTimer.add_or_assign(ke.ident, std::move(sm_pcoroimplActive));
   }
   try {
      // Switch back to the thread’s own context and have it wait for a ready coroutine.
      switch_to_scheduler(pcoroimpl);
   } catch (...) {
      // If anything went wrong or the coroutine was terminated, remove the timer.
      {
//         std::lock_guard<std::mutex> lock(m_mtxCorosAddRemove);
         m_mapCorosBlockedByTimer.remove(ke.ident);
      }
      ke.flags = EV_DELETE;
      ::kevent(m_fdKqueue.get(), &ke, 1, nullptr, 0, &ts);
      throw;
   }
#elif ABC_HOST_API_LINUX
   // TODO: use a pool of inactive timers instead of creating and destroying them each time.
   io::filedesc fd(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC));
   if (!fd) {
      exception::throw_os_error();
   }
   // Arm the timer.
   ::itimerspec its;
   its.it_interval.tv_sec = 0;
   its.it_interval.tv_nsec = 0;
   its.it_value.tv_sec = static_cast< ::time_t>(iMillisecs / 1000u);
   its.it_value.tv_nsec = static_cast<long>(
      static_cast<unsigned long>(iMillisecs % 1000u) * 1000000u
   );
   if (::timerfd_settime(fd.get(), 0, &its, nullptr) < 0) {
      exception::throw_os_error();
   }
   // This timer is now active (save exceptions – see catch (...) below).
   io::filedesc_t fdCopy = fd.get();
   m_mapActiveTimers.add_or_assign(fdCopy, std::move(fd));
   auto deferred1(defer_to_scope_end([this, fdCopy] () {
      // Remove the timer from the set of active ones.
      // TODO: recycle the timer, putting it back in the pool of inactive timers.
      m_mapActiveTimers.remove(fdCopy);
   }));
   // At this point the timer is just a file descriptor that we’ll be waiting to read from.
   block_active_until_fd_ready(fdCopy, false);
   // deferred1 will remove fdCopy from m_mapActiveTimers.
#else
   #error "TODO: HOST_API"
#endif
}

void coroutine::scheduler::block_active_until_fd_ready(io::filedesc_t fd, bool bWrite) {
   ABC_TRACE_FUNC(this, fd, bWrite);

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
   if (!::CreateIoCompletionPort(fd, m_fdIocp.get(), reinterpret_cast< ::ULONG_PTR>(fd), 0)) {
      exception::throw_os_error();
   }
#else
   #error "TODO: HOST_API"
#endif
   // Deactivate the current coroutine and find one to activate instead.
   impl * pcoroimpl;
   {
//      std::lock_guard<std::mutex> lock(m_mtxCorosAddRemove);
      pcoroimpl = m_mapCorosBlockedByFD.add_or_assign(
         fd, std::move(sm_pcoroimplActive)
      ).first->value.get();
   }
   try {
      // Switch back to the thread’s own context and have it wait for a ready coroutine.
      switch_to_scheduler(pcoroimpl);
   } catch (...) {
#if ABC_HOST_API_WIN32
      /* Cancel the pending I/O operation. Note that this will cancel ALL pending I/O on the file,
      not just this one. */
      ::CancelIo(fd, nullptr);
#endif
      // Remove the coroutine from the map of blocked ones.
//      std::lock_guard<std::mutex> lock(m_mtxCorosAddRemove);
      m_mapCorosBlockedByFD.remove(fd);
      throw;
   }
   // Under Linux, deferred1 will remove the now-inactive event for fd.
}

void coroutine::scheduler::coroutine_scheduling_loop(bool bInterruptingAll /*= false*/) {
   std::shared_ptr<impl> & pcoroimplActive = sm_pcoroimplActive;
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
         auto deferred2(defer_to_scope_end([ppcrlsCurrent, pcrlsDefault] () {
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
         // deferred2 will restore the coroutine_local_storage pointer for this thread.
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

std::shared_ptr<coroutine::impl> coroutine::scheduler::find_coroutine_to_activate() {
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
//         std::lock_guard<std::mutex> lock(m_mtxCorosAddRemove);
         if (m_listReadyCoros) {
            // There are coroutines that are ready to run; remove and return the first.
            return m_listReadyCoros.pop_front();
         } else if (
            !m_mapCorosBlockedByFD
#if ABC_HOST_API_BSD
            && !m_mapCorosBlockedByTimer
#endif
         ) {
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
         if (iErr == EINTR) {
            continue;
         }
         exception::throw_os_error(iErr);
      }
      // TODO: understand how EV_ERROR works.
      /*if (ke.flags & EV_ERROR) {
         exception::throw_os_error(ke.data);
      }*/
//      std::lock_guard<std::mutex> lock(m_mtxCorosAddRemove);
      if (ke.filter == EVFILT_TIMER) {
         // Remove and return the coroutine that was waiting for this timer.
         return m_mapCorosBlockedByTimer.extract(ke.ident);
      } else {
         // Remove and return the coroutine that was waiting for this file descriptor.
         return m_mapCorosBlockedByFD.extract(static_cast<io::filedesc_t>(ke.ident));
      }
#elif ABC_HOST_API_LINUX
      ::epoll_event ee;
      if (::epoll_wait(m_fdEpoll.get(), &ee, 1, -1) < 0) {
         int iErr = errno;
         if (iErr == EINTR) {
            continue;
         }
         exception::throw_os_error(iErr);
      }
      // Remove and return the coroutine that was waiting for this file descriptor.
//      std::lock_guard<std::mutex> lock(m_mtxCorosAddRemove);
      return m_mapCorosBlockedByFD.extract(ee.data.fd);
#elif ABC_HOST_API_WIN32
      ::DWORD cbTransferred;
      ::ULONG_PTR iCompletionKey;
      ::OVERLAPPED * povl;
      if (!::GetQueuedCompletionStatus(
         m_fdIocp.get(), &cbTransferred, &iCompletionKey, &povl, INFINITE
      )) {
         exception::throw_os_error(iErr);
      }
      // Remove and return the coroutine that was waiting for this handle.
//      std::lock_guard<std::mutex> lock(m_mtxCorosAddRemove);
      return m_mapCorosBlockedByFD.extract(reinterpret_cast< ::HANDLE>(iCompletionKey));
#else
   #error "TODO: HOST_API"
#endif
   }
}

void coroutine::scheduler::interrupt_all() {
   // Interrupt all coroutines using m_xctPending.
   auto xctInterruptionReason = m_xctInterruptionReason.load();
   {
      /* TODO: using a different locking pattern, this work could be split across multiple threads,
      in case multiple are associated to this scheduler. */
//      std::lock_guard<std::mutex> lock(m_mtxCorosAddRemove);
      ABC_FOR_EACH(auto kv, m_mapCorosBlockedByFD) {
         kv.value->inject_exception(kv.value, xctInterruptionReason);
      }
#if ABC_HOST_API_BSD
      ABC_FOR_EACH(auto kv, m_mapCorosBlockedByTimer) {
         kv.value->inject_exception(kv.value, xctInterruptionReason);
      }
#endif
      /* TODO: coroutines currently running on other threads associated to this scheduler won’t have
      been interrupted by the above loops; they need to be stopped by interrupting the threads that
      are running them. */
   }
   /* Run all coroutines. Since they’ve all just been added to m_listReadyCoros, they’ll all run and
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
   {
      void * pfbr = ::ConvertThreadToFiber(nullptr);
      if (!pfbr) {
         exception::throw_os_error();
      }
      sm_pfbrReturn = pfbr;
   }
#else
   #error "TODO: HOST_API"
#endif
   try {
      coroutine_scheduling_loop();
   } catch (std::exception const & x) {
      interrupt_all(exception::execution_interruption_to_common_type(&x));
      throw;
   } catch (...) {
      interrupt_all(exception::execution_interruption_to_common_type());
      throw;
   }
   // Under POSIX, deferred1 will reset sm_puctxReturn to nullptr.
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
   // Now that we’re back to the coroutine, check for any resume exceptions.
   pcoroimplLastActive->throw_if_any_pending_exception();
}


// Now this can be defined.

/*static*/ void coroutine::impl::outer_main(void * p) {
   impl * pimplThis = static_cast<impl *>(p);
   // Assume for now that m_fnInnerMain will return without exceptions.
   exception::common_type xct = exception::common_type::none;
   try {
      pimplThis->m_fnInnerMain();
   } catch (std::exception const & x) {
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
// abc::this_coroutine

namespace abc {
namespace this_coroutine {

coroutine::id_type id() {
   return reinterpret_cast<coroutine::id_type>(coroutine::scheduler::sm_pcoroimplActive.get());
}

void sleep_for_ms(unsigned iMillisecs) {
   if (auto & pcorosched = this_thread::coroutine_scheduler()) {
      pcorosched->block_active_for_ms(iMillisecs);
   } else {
      this_thread::sleep_for_ms(iMillisecs);
   }
}

void sleep_until_fd_ready(io::filedesc_t fd, bool bWrite) {
   if (auto & pcorosched = this_thread::coroutine_scheduler()) {
      pcorosched->block_active_until_fd_ready(fd, bWrite);
   } else {
      // No coroutine scheduler, so we have to block-wait for fd.
#if ABC_HOST_API_POSIX
      ::pollfd pfd;
      pfd.fd = fd;
      pfd.events = (bWrite ? POLLIN : POLLOUT) | POLLPRI;
      while (::poll(&pfd, 1, -1) < 0) {
         int iErr = errno;
         if (iErr == EINTR) {
            break;
         }
         exception::throw_os_error(iErr);
      }
      if (pfd.revents & (POLLERR | POLLNVAL)) {
         // TODO: how should POLLERR and POLLNVAL be handled?
      }
      /* Consider POLLHUP as input, so that ::read() can return 0 bytes. This helps mitigate the
      considerable differences among poll(2) implementations documented at
      <http://www.greenend.org.uk/rjk/tech/poll.html>, and Linux happens to be one of those who set
      *only* POLLHUP on a pipe with no open write fds. */
      if (pfd.revents & (bWrite ? POLLOUT : POLLIN | POLLHUP)) {
         if (pfd.revents & POLLPRI) {
            // TODO: anything special about “high priority data”?
         }
      } else {
         // TODO: what to do if ::poll() returned but no meaningful bits are set in pfd.revents?
      }
#elif ABC_HOST_API_WIN32
      if (::WaitForSingleObject(fd, INFINITE) != WAIT_OBJECT_0) {
         exception::throw_os_error();
      }
#else
   #error "TODO: HOST_API"
#endif
   }
}

} //namespace this_coroutine
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
