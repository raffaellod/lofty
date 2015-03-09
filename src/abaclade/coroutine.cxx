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
   #if ABC_HOST_API_BSD
      #include <sys/event.h>
      #include <sys/time.h>
      #include <sys/types.h>
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

   void reset(::ucontext_t * puctxReturn) {
      if (::getcontext(&m_uctx) < 0) {
         exception::throw_os_error();
      }
      m_uctx.uc_stack.ss_sp = &m_aiStack;
      m_uctx.uc_stack.ss_size = sizeof m_aiStack;
      m_uctx.uc_link = puctxReturn;
      ::makecontext(&m_uctx, reinterpret_cast<void (*)()>(&outer_main), 1, this);
   }

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

public:
   ::ucontext_t m_uctx;
private:
   std::function<void ()> m_fnInnerMain;
   // TODO: use MINSIGSTKSZ.
   abc::max_align_t m_aiStack[1024];
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

thread_local_value<std::shared_ptr<coroutine_scheduler>> coroutine_scheduler::sm_pcorosched;

coroutine_scheduler::coroutine_scheduler() :
   m_pcoroctxActive(nullptr),
   m_fdEpoll(::epoll_create1(EPOLL_CLOEXEC)) {
   if (!m_fdEpoll) {
      exception::throw_os_error();
   }
}

coroutine_scheduler::~coroutine_scheduler() {
   // TODO: verify that m_listStartingCoros and m_mapBlockedCoros are empty.
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
      pcorosched = std::make_shared<coroutine_scheduler>();
   }
   return *(
      sm_pcorosched = std::move(pcorosched)
   ).operator std::shared_ptr<coroutine_scheduler> const &();
}

std::shared_ptr<coroutine::context> coroutine_scheduler::find_coroutine_to_activate() {
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
         /* Remove this event source from the epoll. Ignore errors, since we wouldn’t know what to
         do aobut them. */
         ::epoll_ctl(m_fdEpoll.get(), EPOLL_CTL_DEL, eeReady.data.fd, nullptr);
         // Find which coroutine was waiting for eeReady and remove it from m_mapBlockedCoros.
         // TODO: need map::pop().
         auto itBlockedCoro(m_mapBlockedCoros.find(eeReady.data.fd));
         std::shared_ptr<coroutine::context> pcoroctxToActivate(std::move(itBlockedCoro->value));
         m_mapBlockedCoros.remove(itBlockedCoro);
         return std::move(pcoroctxToActivate);
      } else {
         return nullptr;
      }
   }
}

void coroutine_scheduler::run() {
   while ((m_pcoroctxActive = find_coroutine_to_activate())) {
      if (::swapcontext(&m_uctxReturn, &m_pcoroctxActive->m_uctx) < 0) {
         /* TODO: in case of errors, which should never happen here since all coroutines have the
         same stack size, inject a stack overflow exception in *m_pcoroctxActive. */
      }
   }
   // Release the last coroutine.
   m_pcoroctxActive.reset();
}

void coroutine_scheduler::yield_while_async_pending(io::filedesc const & fd, bool bWrite) {
   // Add fd to the epoll as a new event source.
   ::epoll_event ee;
   ee.data.fd = fd.get();
   /* Use EPOLLONESHOT to avoid waking up multiple threads for the same fd becoming ready. This
   means we’d need to then rearm it in find_coroutine_to_activate() when it becomes ready, but we’ll
   remove it instead. */
   ee.events = EPOLLONESHOT | (bWrite ? EPOLLOUT : EPOLLIN) | EPOLLPRI;
   if (::epoll_ctl(m_fdEpoll.get(), EPOLL_CTL_ADD, ee.data.fd, &ee) < 0) {
      exception::throw_os_error();
   }
   // Deactivate the current coroutine and find one to activate instead.
   ::ucontext_t * puctxActive = &m_pcoroctxActive->m_uctx;
   auto itBlockedCoro(
      m_mapBlockedCoros.add_or_assign(ee.data.fd, std::move(m_pcoroctxActive)).first
   );
   try {
      m_pcoroctxActive = find_coroutine_to_activate();
   } catch (...) {
      // If anything went wrong, restore the coroutine that was active.
      // TODO: need map::pop().
      m_pcoroctxActive = std::move(itBlockedCoro->value);
      m_mapBlockedCoros.remove(itBlockedCoro);
      throw;
   }
   /* If the context changed, i.e. the coroutine that’s ready to run is not the one that was active,
   switch to it. */
   if (&m_pcoroctxActive->m_uctx != puctxActive) {
      if (::swapcontext(puctxActive, &m_pcoroctxActive->m_uctx) < 0) {
         /* TODO: in case of errors, which should never happen here since all coroutines have the
         same stack size, inject a stack overflow exception in *m_pcoroctxActive. */
      }
   }
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
