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
   #include <errno.h> // EINTR errno
   #if ABC_HOST_API_DARWIN
      #define _XOPEN_SOURCE
   #endif
   #include <signal.h> // SIGSTKSZ
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
   /*! Constructor

   @param fnMain
      Initial value for m_fnInnerMain.
   */
   context(std::function<void ()> fnMain) :
      m_fnInnerMain(std::move(fnMain)) {
   }

#if ABC_HOST_API_POSIX
   /*! Reinitializes the context for execution.

   @param puctxReturn
      Context to return to upon termination.
   */
   void reset(::ucontext_t * puctxReturn) {
      ABC_TRACE_FUNC(this, puctxReturn);

      if (::getcontext(&m_uctx) < 0) {
         exception::throw_os_error();
      }
      m_uctx.uc_stack.ss_sp = reinterpret_cast<char *>(&m_aiStack);
      m_uctx.uc_stack.ss_size = sizeof m_aiStack;
      m_uctx.uc_link = puctxReturn;
      ::makecontext(&m_uctx, reinterpret_cast<void (*)()>(&outer_main), 1, this);
   }
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   #error "TODO: HOST_API"
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else

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
      *this.
   */
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
#if ABC_HOST_API_POSIX
   //! Low-level context for the coroutine.
   ::ucontext_t m_uctx;
   //! Memory chunk used as stack.
   abc::max_align_t m_aiStack[ABC_ALIGNED_SIZE(SIGSTKSZ)];
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   #error "TODO: HOST_API"
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
   //! Function to be executed in the coroutine.
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
      // TODO: verify that m_listStartingCoros and m_mapBlockedCoros are empty.
   }

   //! See coroutine_scheduler::run().
   virtual void run() override {
      ABC_TRACE_FUNC(this);

      while ((m_pcoroctxActive = find_coroutine_to_activate())) {
         if (::swapcontext(&m_uctxReturn, m_pcoroctxActive->ucontext_ptr()) < 0) {
            /* TODO: in case of errors, which should never happen here since all coroutines have the
            same stack size, inject a stack overflow exception in *m_pcoroctxActive. */
         }
      }
      // Release the last coroutine.
      m_pcoroctxActive.reset();
   }

   //! See coroutine_scheduler::yield_while_async_pending().
   virtual void yield_while_async_pending(io::filedesc const & fd, bool bWrite) override {
      ABC_TRACE_FUNC(this, /*fd, */bWrite);

      // Add fd as a new event source.
#if ABC_HOST_API_BSD
      struct ::kevent ke;
      ke.ident = static_cast<std::uintptr_t>(fd.get());
      // Use EV_ONESHOT to avoid waking up multiple threads for the same fd becoming ready.
      ke.flags = EV_ADD | EV_ONESHOT | EV_EOF ;
      ke.filter = bWrite ? EVFILT_WRITE : EVFILT_READ;
      if (::kevent(m_fdKqueue.get(), &ke, 1, nullptr, 0, nullptr) < 0) {
         exception::throw_os_error();
      }
#elif ABC_HOST_API_LINUX
      ::epoll_event ee;
      ee.data.fd = fd.get();
      /* Use EPOLLONESHOT to avoid waking up multiple threads for the same fd becoming ready. This
      means we’d need to then rearm it in find_coroutine_to_activate() when it becomes ready, but
      we’ll remove it instead. */
      ee.events = EPOLLONESHOT | EPOLLPRI | (bWrite ? EPOLLOUT : EPOLLIN);
      if (::epoll_ctl(m_fdEpoll.get(), EPOLL_CTL_ADD, fd.get(), &ee) < 0) {
         exception::throw_os_error();
      }
#else
   #error "TODO: HOST_API"
#endif
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
         if (::swapcontext(pcoroctxActive->ucontext_ptr(), m_pcoroctxActive->ucontext_ptr()) < 0) {
            /* TODO: in case of errors, which should never happen here since all coroutines have the
            same stack size, inject a stack overflow exception in *m_pcoroctxActive. */
         }
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
            // There are coroutines that haven’t had a chance to run; remove and schedule the first.
            auto pcoroctx(m_listStartingCoros.pop_front());
            /* TODO: verify how this behaves in a multithreaded scenario: which thread should this
            coroutine return to? */
            pcoroctx->reset(&m_uctxReturn);
            return std::move(pcoroctx);
         } else if (m_mapBlockedCoros) {
            // There are blocked coroutines; wait for the first one to become ready again.
#if ABC_HOST_API_BSD
            struct ::kevent ke;
            if (::kevent(m_fdKqueue.get(), nullptr, 0, &ke, 1, nullptr) < 0) {
#elif ABC_HOST_API_LINUX
            ::epoll_event ee;
            if (::epoll_wait(m_fdEpoll.get(), &ee, 1, -1) < 0) {
#else
   #error "TODO: HOST_API"
#endif
               int iErr = errno;
               if (iErr == EINTR) {
                  continue;
               }
               exception::throw_os_error(iErr);
            }
            io::filedesc_t fd;
#if ABC_HOST_API_BSD
            fd = static_cast<int>(ke.ident);
#elif ABC_HOST_API_LINUX
            fd = ee.data.fd;
            // Remove fd from the epoll. Ignore errors since we wouldn’t know what to do aobut them.
            ::epoll_ctl(m_fdEpoll.get(), EPOLL_CTL_DEL, fd, nullptr);
#endif
            // Remove from m_mapBlockedCoros and return the coroutine that was waiting for fd.
            return m_mapBlockedCoros.extract(fd);
         } else {
            return nullptr;
         }
      }
   }

private:
#if ABC_HOST_API_BSD
   //! File descriptor of the internal kqueue.
   io::filedesc m_fdKqueue;
#elif ABC_HOST_API_LINUX
   //! File descriptor of the internal epoll.
   io::filedesc m_fdEpoll;
#else
   #error "TODO: HOST_API"
#endif
   //! Coroutines that are blocked on a fd wait.
   collections::map<io::filedesc_t, std::shared_ptr<coroutine::context>> m_mapBlockedCoros;
   //! Coroutine context that every coroutine eventually returns to.
   ::ucontext_t m_uctxReturn;
};

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   #error "TODO: HOST_API"
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else


thread_local_value<std::shared_ptr<coroutine_scheduler>> coroutine_scheduler::sm_pcorosched;

coroutine_scheduler::coroutine_scheduler() :
   m_pcoroctxActive(nullptr) {
}

coroutine_scheduler::~coroutine_scheduler() {
}

void coroutine_scheduler::add_coroutine(coroutine const & coro) {
   ABC_TRACE_FUNC(this);

   // Add the coroutine to those ready to start.
   m_listStartingCoros.push_back(coro.m_pctx);
}

/*static*/ coroutine_scheduler & coroutine_scheduler::attach_to_current_thread(
   std::shared_ptr<coroutine_scheduler> pcorosched /*= nullptr*/
) {
   ABC_TRACE_FUNC(pcorosched);

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
