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
#include <abaclade/collections/map.hxx>

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
   #ifdef ABAMAKE_USING_VALGRIND
      #include <valgrind/valgrind.h>
   #endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::coroutine

namespace abc {

class coroutine::context : public noncopyable {
public:
   /*! Constructor

   @param fnMain
      Initial value for m_fnInnerMain.
   */
   context(std::function<void ()> fnMain) :
#if ABC_HOST_API_POSIX
      m_pStack(SIGSTKSZ),
   #ifdef ABAMAKE_USING_VALGRIND
      m_iValgrindStackId(VALGRIND_STACK_REGISTER(
         m_pStack.get(), static_cast<std::int8_t *>(m_pStack.get()) + m_pStack.size()
      )),
   #endif
#endif
      m_fnInnerMain(std::move(fnMain)),
      m_crls(false) {
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

   // Destructor.
   ~context() {
#ifdef ABAMAKE_USING_VALGRIND
      VALGRIND_STACK_DEREGISTER(m_iValgrindStackId);
#endif
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

   /*! Returns a pointer to the coroutine’s coroutine_local_storage object.

   @return
      Pointer to the context’s m_crls member.
   */
   detail::coroutine_local_storage * local_storage_ptr() {
      return &m_crls;
   }

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
   #ifdef ABAMAKE_USING_VALGRIND
   unsigned m_iValgrindStackId;
   #endif
#elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#else
   #error "TODO: HOST_API"
#endif
   //! Function to be executed in the coroutine.
   std::function<void ()> m_fnInnerMain;
   //! Local storage for the coroutine.
   detail::coroutine_local_storage m_crls;
};


coroutine::coroutine() {
}
/*explicit*/ coroutine::coroutine(std::function<void ()> fnMain) :
   m_pctx(std::make_shared<coroutine::context>(std::move(fnMain))) {
}

coroutine::~coroutine() {
}

coroutine::id_type coroutine::id() const {
   return reinterpret_cast<id_type>(m_pctx.get());
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
// abc::coroutine_scheduler

namespace abc {

namespace detail {

#if ABC_HOST_API_POSIX

class coroutine_scheduler_impl : public coroutine_scheduler {
public:
   //! Constructor.
   coroutine_scheduler_impl() :
#if ABC_HOST_API_BSD
      m_fdKqueue(::kqueue()) {
      ABC_TRACE_FUNC(this);

      if (!m_fdKqueue) {
         exception::throw_os_error();
      }
      // TODO: set close-on-exec. ::kqueue1() may be available on some systems; investigate that.
#elif ABC_HOST_API_LINUX
      m_fdEpoll(::epoll_create1(EPOLL_CLOEXEC)) {
      ABC_TRACE_FUNC(this);

      if (!m_fdEpoll) {
         exception::throw_os_error();
      }
#else
   #error "TODO: HOST_API"
#endif
   }

   //! Destructor.
   virtual ~coroutine_scheduler_impl() {
      // TODO: verify that m_listStartingCoros and m_mapCorosBlockedByFD are empty.
   }

   //! Switches context to the current thread’s own context.
   void return_to_scheduler() {
   #if ABC_HOST_API_DARWIN && ABC_HOST_CXX_CLANG
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #endif
      ::setcontext(sm_puctxReturn.get());
   #if ABC_HOST_API_DARWIN && ABC_HOST_CXX_CLANG
      #pragma clang diagnostic pop
   #endif
      // ::setcontext() never returns if successful, so if we’re still here something went wrong.
      exception::throw_os_error();
   }

   //! See coroutine_scheduler::run().
   virtual void run() override {
      ABC_TRACE_FUNC(this);

      ::ucontext_t uctxReturn;
      sm_puctxReturn = &uctxReturn;
      std::shared_ptr<coroutine::context> & pcoroctxActive = sm_pcoroctxActive;
      coroutine_local_storage * pcrlsDefault = &coroutine_local_storage::sm_crls.get();
      coroutine_local_storage *& pcrlsCurrent = coroutine_local_storage::sm_pcrls;
      try {
         while ((pcoroctxActive = find_coroutine_to_activate())) {
            /* Swap the coroutine_local_storage pointer for this thread with that of the active
            coroutine. */
            pcrlsCurrent = pcoroctxActive->local_storage_ptr();
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
            pcrlsCurrent = pcrlsDefault;
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

   //! See coroutine_scheduler::yield_for().
   virtual void yield_for(unsigned iMillisecs) override {
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
      auto itActiveTimer(m_mapActiveTimers.add_or_assign(fdCopy, std::move(fd)).first);
      // At this point the timer is just a file descriptor that we’ll be waiting to read from.
      try {
         yield_while_async_pending(fdCopy, false);
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

   //! See coroutine_scheduler::yield_while_async_pending().
   virtual void yield_while_async_pending(io::filedesc_t fd, bool bWrite) override {
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
      means we’d need to then rearm it in find_coroutine_to_activate() when it becomes ready, but
      we’ll remove it instead. */
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
         // Remove the coroutine from the map of blocked ones.
         m_mapCorosBlockedByFD.remove(fd);
         throw;
      }
   }

private:
   /*! Finds a coroutine ready to execute; if none are, but there are blocked coroutines, it blocks
   the current thread until one of them becomes ready.

   @return
      Pointer to the context of a coroutine that’s ready to execute.
   */
   std::shared_ptr<coroutine::context> find_coroutine_to_activate() {
      ABC_TRACE_FUNC(this);

      // This loop will only repeat in case of EINTR from the blocking-wait API.
      for (;;) {
         if (m_listStartingCoros) {
            // There are coroutines that haven’t had a chance to run; remove and return the first.
            return m_listStartingCoros.pop_front();
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
               return m_mapCorosBlockedByFD.extract(static_cast<int>(ke.ident));
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
            // Remove fd from the epoll. Ignore errors since we wouldn’t know what to do about them.
            ::epoll_ctl(m_fdEpoll.get(), EPOLL_CTL_DEL, ee.data.fd, nullptr);
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

   /*! Switches context from the coroutine context pointed to by pcoroctxLastActive to the current
   thread’s own context.

   @param pcoroctxLastActive
      Pointer to the coroutine context that is being inactivated.
   */
   void switch_to_scheduler(coroutine::context * pcoroctxLastActive) {
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
   }

private:
#if ABC_HOST_API_BSD
   //! File descriptor of the internal kqueue.
   io::filedesc m_fdKqueue;
   /*! Coroutines that are blocked on a timer wait. The keys are the same as the values, but this
   can’t be changed into a set<shared_ptr<context>> because we need it to hold a strong reference to
   the coroutine context while allowing lookups without having a shared_ptr. */
   collections::map<std::uintptr_t, std::shared_ptr<coroutine::context>> m_mapCorosBlockedByTimer;
#elif ABC_HOST_API_LINUX
   //! File descriptor of the internal epoll.
   io::filedesc m_fdEpoll;
   /*! Timers currently being waited for. The key is the same as the value, but this can’t be
   changed into a set<io::filedesc> until io::filedesc is hashable. */
   collections::map<io::filedesc_t, io::filedesc> m_mapActiveTimers;
#else
   #error "TODO: HOST_API"
#endif
   //! Coroutines that are blocked on a fd wait.
   collections::map<io::filedesc_t, std::shared_ptr<coroutine::context>> m_mapCorosBlockedByFD;
   //! Pointer to the context of every thread running a coroutine_scheduler.
   static thread_local_value< ::ucontext_t *> sm_puctxReturn;
};

thread_local_value< ::ucontext_t *> coroutine_scheduler_impl::sm_puctxReturn /*= nullptr*/;

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   #error "TODO: HOST_API"
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else

} //namespace detail

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
   static_cast<detail::coroutine_scheduler_impl *>(
      this_thread::get_coroutine_scheduler().get()
   )->return_to_scheduler();
}


thread_local_value<std::shared_ptr<coroutine::context>> coroutine_scheduler::sm_pcoroctxActive;
thread_local_value<std::shared_ptr<coroutine_scheduler>> coroutine_scheduler::sm_pcorosched;

coroutine_scheduler::coroutine_scheduler() {
}

coroutine_scheduler::~coroutine_scheduler() {
}

void coroutine_scheduler::add(coroutine const & coro) {
   ABC_TRACE_FUNC(this);

   // Add the coroutine to those ready to start.
   m_listStartingCoros.push_back(coro.m_pctx);
}

namespace this_thread {

std::shared_ptr<coroutine_scheduler> const & attach_coroutine_scheduler(
   std::shared_ptr<coroutine_scheduler> pcorosched /*= nullptr*/
) {
   ABC_TRACE_FUNC(pcorosched);

   if (coroutine_scheduler::sm_pcorosched) {
      // The current thread already has a coroutine scheduler.
      // TODO: use a better exception class.
      ABC_THROW(generic_error, ());
   }
   if (!pcorosched) {
      pcorosched = std::make_shared<detail::coroutine_scheduler_impl>();
   }
   return (coroutine_scheduler::sm_pcorosched = std::move(pcorosched));
}

} //namespace this_thread

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
