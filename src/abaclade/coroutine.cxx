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

#if ABC_HOST_API_POSIX
   #include <errno.h> // EINTR errno
   #if ABC_HOST_API_DARWIN
      #define _XOPEN_SOURCE
   #endif
   #include <ucontext.h>
   #if ABC_HOST_API_BSD
      #include <sys/types.h>
      #include <sys/event.h>
      #include <sys/time.h>
   #elif ABC_HOST_API_LINUX
      #include <sys/epoll.h>
   #endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::coroutine

namespace abc {

class coroutine::context : public noncopyable {
public:
   context(std::function<void ()> fnMain) :
      m_fnInnerMain(std::move(fnMain)) {
   }

#if ABC_HOST_API_BSD
   void reset(::ucontext_t * puctxReturn) {
      if (::getcontext(&m_uctx) < 0) {
         exception::throw_os_error();
      }
      m_uctx.uc_stack.ss_sp = reinterpret_cast<char *>(&m_aiStack);
      m_uctx.uc_stack.ss_size = sizeof m_aiStack;
      m_uctx.uc_link = puctxReturn;
      ::makecontext(&m_uctx, reinterpret_cast<void (*)()>(&outer_main), 1, this);
   }
#elif ABC_HOST_API_LINUX //if ABC_HOST_API_BSD
   void reset(::ucontext_t * puctxReturn) {
      if (::getcontext(&m_uctx) < 0) {
         exception::throw_os_error();
      }
      m_uctx.uc_stack.ss_sp = &m_aiStack;
      m_uctx.uc_stack.ss_size = sizeof m_aiStack;
      m_uctx.uc_link = puctxReturn;
      ::makecontext(&m_uctx, reinterpret_cast<void (*)()>(&outer_main), 1, this);
   }
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_BSD … elif ABC_HOST_API_LINUX
   #error "TODO: HOST_API"
#else //if ABC_HOST_API_BSD … elif ABC_HOST_API_LINUX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_BSD … elif ABC_HOST_API_LINUX … elif ABC_HOST_API_WIN32 … else

private:
   static void outer_main(void * p) {
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
   }

private:
#if ABC_HOST_API_BSD
public:
   ::ucontext_t m_uctx;
private:
   // TODO: use MINSIGSTKSZ.
   abc::max_align_t m_aiStack[1024];
#elif ABC_HOST_API_LINUX //if ABC_HOST_API_BSD
public:
   ::ucontext_t m_uctx;
private:
   // TODO: use MINSIGSTKSZ.
   abc::max_align_t m_aiStack[1024];
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_BSD … elif ABC_HOST_API_LINUX
   #error "TODO: HOST_API"
#else //if ABC_HOST_API_BSD … elif ABC_HOST_API_LINUX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_BSD … elif ABC_HOST_API_LINUX … elif ABC_HOST_API_WIN32 … else
   std::function<void ()> m_fnInnerMain;
};


coroutine::coroutine() {
}
/*explicit*/ coroutine::coroutine(std::function<void ()> fnMain) :
   m_pctx(std::make_shared<coroutine::context>(std::move(fnMain))) {
}

coroutine::~coroutine() {
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::coroutine_scheduler

namespace abc {

#if ABC_HOST_API_BSD

class coroutine_scheduler_impl : public coroutine_scheduler {
public:
   //! Constructor.
   coroutine_scheduler_impl() :
      m_fdKqueue(::kqueue()) {
      if (!m_fdKqueue) {
         exception::throw_os_error();
      }
      // TODO: set close-on-exec. ::kqueue1() may be available on some systems; investigate that.
   }

   //! Destructor.
   virtual ~coroutine_scheduler_impl() {
      // TODO: verify that m_listStartingCoros and m_mapBlockedCoros are empty.
   }

   //! See coroutine_scheduler::run().
   virtual void run() override {
      while ((m_pcoroctxActive = find_coroutine_to_activate())) {
         if (::swapcontext(&m_uctxReturn, &m_pcoroctxActive->m_uctx) < 0) {
            /* TODO: in case of errors, which should never happen here since all coroutines have the
            same stack size, inject a stack overflow exception in *m_pcoroctxActive. */
         }
      }
      // Release the last coroutine.
      m_pcoroctxActive.reset();
   }

   //! See coroutine_scheduler::yield_while_async_pending().
   virtual void yield_while_async_pending(io::filedesc const & fd, bool bWrite) override {
      // Add fd to the epoll as a new event source.
      struct ::kevent ke;
      ke.ident = static_cast<std::uintptr_t>(fd.get());
      // Use EV_ONESHOT to avoid waking up multiple threads for the same fd becoming ready.
      ke.flags = EV_ADD | EV_ONESHOT | EV_EOF ;
      ke.filter = bWrite ? EVFILT_WRITE : EVFILT_READ;
      if (::kevent(m_fdKqueue.get(), &ke, 1, nullptr, 0, nullptr) < 0) {
         exception::throw_os_error();
      }
      // Deactivate the current coroutine and find one to activate instead.
      coroutine::context * pcoroctxActive = m_pcoroctxActive.get();
      auto itBlockedCoro(
         m_mapBlockedCoros.add_or_assign(fd.get(), std::move(m_pcoroctxActive)).first
      );
      try {
         m_pcoroctxActive = find_coroutine_to_activate();
      } catch (...) {
         // If anything went wrong, restore the coroutine that was active.
         m_pcoroctxActive = m_mapBlockedCoros.extract(itBlockedCoro);
         throw;
      }
      /* If the context changed, i.e. the coroutine that’s ready to run is not the one that was
      active, switch to it. */
      if (m_pcoroctxActive.get() != pcoroctxActive) {
         if (::swapcontext(&pcoroctxActive->m_uctx, &m_pcoroctxActive->m_uctx) < 0) {
            /* TODO: in case of errors, which should never happen here since all coroutines have the
            same stack size, inject a stack overflow exception in *m_pcoroctxActive. */
         }
      }
   }

private:
   std::shared_ptr<coroutine::context> find_coroutine_to_activate() {
      // This loop will only repeat in case of EINTR during ::epoll_wait().
      for (;;) {
         if (m_listStartingCoros) {
            // There are coroutines that haven’t had a chance to run; remove and schedule the first.
            auto pcoroctx(m_listStartingCoros.pop_front());
            // TODO: verify how this behaves in a multithreaded scenario.
            pcoroctx->reset(&m_uctxReturn);
            return std::move(pcoroctx);
         } else if (m_mapBlockedCoros) {
            // There are blocked coroutines; wait for the first one to become ready again.
            struct ::kevent ke;
            int cReadyEvents = ::kevent(m_fdKqueue.get(), nullptr, 0, &ke, 1, nullptr);
            // 0 won’t really be returned; possible values are either 1 or -1.
            if (cReadyEvents < 0) {
               int iErr = errno;
               if (iErr == EINTR) {
                  continue;
               }
               exception::throw_os_error(iErr);
            }
            /* Find which coroutine was waiting for ke, remove it from m_mapBlockedCoros, and return
            it. */
            return m_mapBlockedCoros.extract(static_cast<int>(ke.ident));
         } else {
            return nullptr;
         }
      }
   }

private:
   //! File descriptor of the internal epoll.
   io::filedesc m_fdKqueue;
   //! Coroutines that are blocked on a fd wait.
   collections::map<int, std::shared_ptr<coroutine::context>> m_mapBlockedCoros;
   // Coroutine context that every coroutine eventually returns to.
   ::ucontext_t m_uctxReturn;
};

#elif ABC_HOST_API_LINUX //if ABC_HOST_API_BSD

class coroutine_scheduler_impl : public coroutine_scheduler {
public:
   //! Constructor.
   coroutine_scheduler_impl() :
      m_fdEpoll(::epoll_create1(EPOLL_CLOEXEC)) {
      if (!m_fdEpoll) {
         exception::throw_os_error();
      }
   }

   //! Destructor.
   virtual ~coroutine_scheduler_impl() {
      // TODO: verify that m_listStartingCoros and m_mapBlockedCoros are empty.
   }

   //! See coroutine_scheduler::run().
   virtual void run() override {
      while ((m_pcoroctxActive = find_coroutine_to_activate())) {
         if (::swapcontext(&m_uctxReturn, &m_pcoroctxActive->m_uctx) < 0) {
            /* TODO: in case of errors, which should never happen here since all coroutines have the
            same stack size, inject a stack overflow exception in *m_pcoroctxActive. */
         }
      }
      // Release the last coroutine.
      m_pcoroctxActive.reset();
   }

   //! See coroutine_scheduler::yield_while_async_pending().
   virtual void yield_while_async_pending(io::filedesc const & fd, bool bWrite) override {
      // Add fd to the epoll as a new event source.
      ::epoll_event ee;
      ee.data.fd = fd.get();
      /* Use EPOLLONESHOT to avoid waking up multiple threads for the same fd becoming ready. This
      means we’d need to then rearm it in find_coroutine_to_activate() when it becomes ready, but
      we’ll remove it instead. */
      ee.events = EPOLLONESHOT | (bWrite ? EPOLLOUT : EPOLLIN) | EPOLLPRI;
      if (::epoll_ctl(m_fdEpoll.get(), EPOLL_CTL_ADD, fd.get(), &ee) < 0) {
         exception::throw_os_error();
      }
      // Deactivate the current coroutine and find one to activate instead.
      coroutine::context * pcoroctxActive = m_pcoroctxActive.get();
      auto itBlockedCoro(
         m_mapBlockedCoros.add_or_assign(fd.get(), std::move(m_pcoroctxActive)).first
      );
      try {
         m_pcoroctxActive = find_coroutine_to_activate();
      } catch (...) {
         // If anything went wrong, restore the coroutine that was active.
         m_pcoroctxActive = m_mapBlockedCoros.extract(itBlockedCoro);
         throw;
      }
      /* If the context changed, i.e. the coroutine that’s ready to run is not the one that was
      active, switch to it. */
      if (m_pcoroctxActive.get() != pcoroctxActive) {
         if (::swapcontext(&pcoroctxActive->m_uctx, &m_pcoroctxActive->m_uctx) < 0) {
            /* TODO: in case of errors, which should never happen here since all coroutines have the
            same stack size, inject a stack overflow exception in *m_pcoroctxActive. */
         }
      }
   }

private:
   std::shared_ptr<coroutine::context> find_coroutine_to_activate() {
      // This loop will only repeat in case of EINTR during ::epoll_wait().
      for (;;) {
         if (m_listStartingCoros) {
            // There are coroutines that haven’t had a chance to run; remove and schedule the first.
            auto pcoroctx(m_listStartingCoros.pop_front());
            // TODO: verify how this behaves in a multithreaded scenario.
            pcoroctx->reset(&m_uctxReturn);
            return std::move(pcoroctx);
         } else if (m_mapBlockedCoros) {
            // There are blocked coroutines; wait for the first one to become ready again.
            ::epoll_event eeReady;
            int cReadyFds = ::epoll_wait(m_fdEpoll.get(), &eeReady, 1, -1);
            // 0 won’t really be returned; possible values are either 1 or -1.
            if (cReadyFds < 0) {
               int iErr = errno;
               if (iErr == EINTR) {
                  continue;
               }
               exception::throw_os_error(iErr);
            }
            /* Remove this event source from the epoll. Ignore errors, since we wouldn’t know what
            to do aobut them. */
            ::epoll_ctl(m_fdEpoll.get(), EPOLL_CTL_DEL, eeReady.data.fd, nullptr);
            /* Find which coroutine was waiting for ke, remove it from m_mapBlockedCoros, and return
            it. */
            return m_mapBlockedCoros.extract(static_cast<int>(eeReady.data.fd));
         } else {
            return nullptr;
         }
      }
   }

private:
   //! File descriptor of the internal epoll.
   io::filedesc m_fdEpoll;
   //! Coroutines that are blocked on a fd wait.
   collections::map<int, std::shared_ptr<coroutine::context>> m_mapBlockedCoros;
   // Coroutine context that every coroutine eventually returns to.
   ::ucontext_t m_uctxReturn;
};

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_BSD … elif ABC_HOST_API_LINUX
   #error "TODO: HOST_API"
#else //if ABC_HOST_API_BSD … elif ABC_HOST_API_LINUX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_BSD … elif ABC_HOST_API_LINUX … elif ABC_HOST_API_WIN32 … else

thread_local_value<std::shared_ptr<coroutine_scheduler>> coroutine_scheduler::sm_pcorosched;

coroutine_scheduler::coroutine_scheduler() :
   m_pcoroctxActive(nullptr) {
}

coroutine_scheduler::~coroutine_scheduler() {
}

void coroutine_scheduler::add_coroutine(coroutine const & coro) {
   // Add the coroutine to those ready to start.
   m_listStartingCoros.push_back(coro.m_pctx);
}

/*static*/ coroutine_scheduler & coroutine_scheduler::attach_to_current_thread(
   std::shared_ptr<coroutine_scheduler> pcorosched /*= nullptr*/
) {
   if (sm_pcorosched.operator std::shared_ptr<coroutine_scheduler> const &()) {
      // The current thread already has a coroutine scheduler.
      // TODO: use a better exception class.
      ABC_THROW(generic_error, ());
   }
   if (!pcorosched) {
      pcorosched = std::make_shared<coroutine_scheduler_impl>();
   }
   return *(
      sm_pcorosched = std::move(pcorosched)
   ).operator std::shared_ptr<coroutine_scheduler> const &();
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
