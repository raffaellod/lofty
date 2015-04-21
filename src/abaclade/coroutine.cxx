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

#include "coroutine-scheduler.hxx"

#include <abaclade.hxx>
#include <abaclade/coroutine.hxx>

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
// abc::coroutine::context

namespace abc {

class coroutine::context : public noncopyable {
private:
   friend class coroutine;

public:
   /*! Constructor

   @param fnMain
      Initial value for m_fnInnerMain.
   */
   context(std::function<void ()> fnMain) :
#if ABC_HOST_API_POSIX
      m_pStack(SIGSTKSZ),
#endif
#ifdef ABAMAKE_USING_VALGRIND
      m_iValgrindStackId(VALGRIND_STACK_REGISTER(
         m_pStack.get(), static_cast<std::int8_t *>(m_pStack.get()) + m_pStack.size()
      )),
#endif
      m_injResumeException(exception::injectable::none),
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
   ~context() {
#ifdef ABAMAKE_USING_VALGRIND
      VALGRIND_STACK_DEREGISTER(m_iValgrindStackId);
#endif
   }

   /*! Returns a pointer to the coroutine’s coroutine_local_storage object.

   @return
      Pointer to the context’s m_crls member.
   */
   detail::coroutine_local_storage * local_storage_ptr() {
      return &m_crls;
   }

   /*! Called right after each time the coroutine resumes execution, this will throw an exception of
   the type specified by m_injResumeException. This kind of exceptions are injected by other
   coroutines or other Abaclade implementation code. */
   void throw_if_any_resume_exception() const {
      if (m_injResumeException != exception::injectable::none) {
         exception::throw_injected_exception(m_injResumeException, 0, 0);
      }
   }

#if ABC_HOST_API_POSIX
   /*! Returns a pointer to the internal ::ucontext_t.

   @return
      Pointer to the context’s ::ucontext_t member.
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
   //! Low-level context for the coroutine.
   ::ucontext_t m_uctx;
   //! Pointer to the memory chunk used as stack.
   memory::pages_ptr m_pStack;
#elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#else
   #error "TODO: HOST_API"
#endif
#ifdef ABAMAKE_USING_VALGRIND
   //! Identifier assigned by Valgrind to this context’s stack.
   unsigned m_iValgrindStackId;
#endif
   std::atomic<exception::injectable::enum_type> m_injResumeException;
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
   m_pctx(std::make_shared<coroutine::context>(std::move(fnMain))) {
   this_thread::attach_coroutine_scheduler()->add_ready(m_pctx);
}

coroutine::~coroutine() {
}

coroutine::id_type coroutine::id() const {
   return reinterpret_cast<id_type>(m_pctx.get());
}

void coroutine::interrupt() {
   ABC_TRACE_FUNC(this);

   m_pctx->m_injResumeException = exception::injectable::execution_interruption;
   /* Mark this coroutine as ready, so it will be scheduler before the scheduler tries to wait for
   it to be unblocked. */
   // TODO: multithread-proofing.
   // TODO: sanity check to avoid scheduling a coroutine twice!
   this_thread::get_coroutine_scheduler()->add_ready(m_pctx);
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

thread_local_value<std::shared_ptr<coroutine::context>> coroutine::scheduler::sm_pcoroctxActive;
thread_local_value<std::shared_ptr<coroutine::scheduler>> coroutine::scheduler::sm_pcorosched;
thread_local_value< ::ucontext_t *> coroutine::scheduler::sm_puctxReturn /*= nullptr*/;

coroutine::scheduler::scheduler() :
#if ABC_HOST_API_BSD
   m_fdKqueue(::kqueue()) {
#elif ABC_HOST_API_LINUX
   m_fdEpoll(::epoll_create1(EPOLL_CLOEXEC)) {
#endif
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
#endif
}

coroutine::scheduler::~scheduler() {
   // TODO: verify that m_listReadyCoros and m_mapCorosBlockedByFD are empty.
}

void coroutine::scheduler::add_ready(std::shared_ptr<coroutine::context> pcoroctx) {
   ABC_TRACE_FUNC(this, pcoroctx);

   m_listReadyCoros.push_back(std::move(pcoroctx));
}

void coroutine::scheduler::block_active_for_ms(unsigned iMillisecs) {
   ABC_TRACE_FUNC(this, iMillisecs);

   // TODO: handle iMillisecs == 0 as a timer-less yield.

#if ABC_HOST_API_BSD
   coroutine::context * pcoroctxActive = sm_pcoroctxActive.get();
   struct ::kevent ke;
   ke.ident = reinterpret_cast<std::uintptr_t>(pcoroctxActive);
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
   auto itThisCoro(
      m_mapCorosBlockedByTimer.add_or_assign(ke.ident, std::move(sm_pcoroctxActive)).first
   );
   try {
      // Switch back to the thread’s own context and have it wait for a ready coroutine.
      switch_to_scheduler(itThisCoro->value.get());
   } catch (...) {
      // If anything went wrong or the coroutine was terminated, remove the timer.
      m_mapCorosBlockedByTimer.remove(ke.ident);
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
   // At this point the timer is just a file descriptor that we’ll be waiting to read from.
   try {
      block_active_until_fd_ready(fdCopy, false);
   } catch (...) {
      // Remove the timer from the set of active ones.
      // TODO: recycle the timer, putting it back in the pool of inactive timers.
      // TODO: move this code to a “defer” lambda.
      m_mapActiveTimers.remove(fdCopy);
      throw;
   }
   // Remove the timer from the set of active ones.
   // TODO: recycle the timer, putting it back in the pool of inactive timers.
   // TODO: move this code to a “defer” lambda.
   m_mapActiveTimers.remove(fdCopy);
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
#else
   #error "TODO: HOST_API"
#endif
   // Deactivate the current coroutine and find one to activate instead.
   auto itThisCoro(m_mapCorosBlockedByFD.add_or_assign(fd, std::move(sm_pcoroctxActive)).first);
   try {
      // Switch back to the thread’s own context and have it wait for a ready coroutine.
      switch_to_scheduler(itThisCoro->value.get());
   } catch (...) {
#if ABC_HOST_API_LINUX
      // Remove fd from the epoll. Ignore errors since we wouldn’t know what to do about them.
      // TODO: move this code to a “defer” lambda.
      ::epoll_ctl(m_fdEpoll.get(), EPOLL_CTL_DEL, fd, nullptr);
#endif
      // Remove the coroutine from the map of blocked ones.
      m_mapCorosBlockedByFD.remove(fd);
      throw;
   }
#if ABC_HOST_API_LINUX
   // Remove fd from the epoll. Ignore errors since we wouldn’t know what to do about them.
   // TODO: move this code to a “defer” lambda.
   ::epoll_ctl(m_fdEpoll.get(), EPOLL_CTL_DEL, fd, nullptr);
#endif
}

std::shared_ptr<coroutine::context> coroutine::scheduler::find_coroutine_to_activate() {
   ABC_TRACE_FUNC(this);

   // This loop will only repeat in case of EINTR from the blocking-wait API.
   /* TODO: if the epoll/kqueue is shared by several threads and one thread receives and removes the
   last event source from it, what happens to the remaining threads?
   a) We could send a no-op signal (SIGCONT?) to all threads using this scheduler, to make the wait
      function return EINTR;
   b) We could have a single event source for each scheduler with the semantics of “event sources
      changed”, in edge-triggered mode so it wakes all waiting threads once, at once. */
   for (;;) {
      if (m_listReadyCoros) {
         // There are coroutines that are ready to run; remove and return the first.
         return m_listReadyCoros.pop_front();
      } else if (
         m_mapCorosBlockedByFD
#if ABC_HOST_API_BSD
         || m_mapCorosBlockedByTimer
#endif
      ) {
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
         if (ke.filter == EVFILT_TIMER) {
            // Remove and return the coroutine that was waiting for the timer.
            return m_mapCorosBlockedByTimer.extract(ke.ident);
         } else {
            // Remove and return the coroutine that was waiting for the file descriptor.
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
         // Remove and return the coroutine that was waiting for the file descriptor.
         return m_mapCorosBlockedByFD.extract(ee.data.fd);
#else
   #error "TODO: HOST_API"
#endif
      } else {
         return nullptr;
      }
   }
}

void coroutine::scheduler::return_to_scheduler() {
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
}

void coroutine::scheduler::run() {
   ABC_TRACE_FUNC(this);

   ::ucontext_t uctxReturn;
   sm_puctxReturn = &uctxReturn;
   std::shared_ptr<coroutine::context> & pcoroctxActive = sm_pcoroctxActive;
   detail::coroutine_local_storage * pcrlsDefault, ** ppcrlsCurrent;
   detail::coroutine_local_storage::get_default_and_current_pointers(&pcrlsDefault, &ppcrlsCurrent);
   try {
      while ((pcoroctxActive = find_coroutine_to_activate())) {
         /* Swap the coroutine_local_storage pointer for this thread with that of the active
         coroutine. */
         *ppcrlsCurrent = pcoroctxActive->local_storage_ptr();
         // Switch the current thread’s context to the active coroutine’s.
#if ABC_HOST_API_DARWIN && ABC_HOST_CXX_CLANG
   #pragma clang diagnostic push
   #pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
         int iRet = ::swapcontext(&uctxReturn, pcoroctxActive->ucontext_ptr());
#if ABC_HOST_API_DARWIN && ABC_HOST_CXX_CLANG
   #pragma clang diagnostic pop
#endif
         // Restore the coroutine_local_storage pointer for this thread.
         *ppcrlsCurrent = pcrlsDefault;
         if (iRet < 0) {
            /* TODO: only a stack-related ENOMEM is possible, so throw a stack overflow exception
            (*sm_pcoroctxActive has a problem, not uctxReturn). */
         }
      }
   } catch (...) {
      sm_puctxReturn = nullptr;
      throw;
   }
   sm_puctxReturn = nullptr;
}

void coroutine::scheduler::switch_to_scheduler(coroutine::context * pcoroctxLastActive) {
   #if ABC_HOST_API_DARWIN && ABC_HOST_CXX_CLANG
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #endif
   if (::swapcontext(pcoroctxLastActive->ucontext_ptr(), sm_puctxReturn.get()) < 0) {
   #if ABC_HOST_API_DARWIN && ABC_HOST_CXX_CLANG
      #pragma clang diagnostic pop
   #endif
      /* TODO: only a stack-related ENOMEM is possible, so throw a stack overflow exception
      (*sm_puctxReturn has a problem, not *sm_pcoroctxActive). */
   }
   // Now that we’re back to the coroutine, check for any resume exceptions.
   pcoroctxLastActive->throw_if_any_resume_exception();
}


// Now this can be defined.

/*static*/ void coroutine::context::outer_main(void * p) {
   context * pctx = static_cast<context *>(p);
   try {
      pctx->m_fnInnerMain();
   } catch (std::exception const & x) {
      exception::write_with_scope_trace(nullptr, &x);
      // TODO: maybe support “moving” the exception to the return coroutine context?
   } catch (...) {
      exception::write_with_scope_trace();
      // TODO: maybe support “moving” the exception to the return coroutine context?
   }
   this_thread::get_coroutine_scheduler()->return_to_scheduler();
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::this_coroutine

namespace abc {
namespace this_coroutine {

coroutine::id_type id() {
   return reinterpret_cast<coroutine::id_type>(coroutine::scheduler::sm_pcoroctxActive.get());
}

void sleep_for_ms(unsigned iMillisecs) {
   if (auto & pcorosched = this_thread::get_coroutine_scheduler()) {
      pcorosched->block_active_for_ms(iMillisecs);
   } else {
      this_thread::sleep_for_ms(iMillisecs);
   }
}

void sleep_until_fd_ready(io::filedesc_t fd, bool bWrite) {
   if (auto & pcorosched = this_thread::get_coroutine_scheduler()) {
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
