/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014, 2015
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
#include <abaclade/event_loop.hxx>
#include <abaclade/collections/map.hxx>

#if ABC_HOST_API_POSIX
   #include <errno.h> // EINTR errno
   #include <unistd.h> // close()
   #if ABC_HOST_API_BSD
      #include <sys/event.h>
      #include <sys/time.h>
      #include <sys/types.h>
   #elif ABC_HOST_API_LINUX
      #include <sys/epoll.h>
   #endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::event_loop

namespace abc {

namespace {

//! Contains OS-specific data members of event_loop.
struct event_loop_impl_t {
   //! If true, one or more event sources have been added/changed/removed.
   bool bChanged;
#if ABC_HOST_API_BSD
   //! File descriptor of the internal kqueue.
   int fdKqueue;

   //! Constructor.
   event_loop_impl_t() :
      fdKqueue(::kqueue()) {
      if (fdKqueue == -1) {
         throw_os_error();
      }
   }

   //! Destructor.
   ~event_loop_impl_t() {
      ::close(fdKqueue);
   }

#elif ABC_HOST_API_LINUX //if ABC_HOST_API_BSD
   //! File descriptor of the internal epoll.
   int fdEpoll;
   /*! Array of eventfd file descriptors, used to receive exit notifications from threads and
   processes. */
   collections::dmvector<int> vfdEvents;

   //! Constructor.
   event_loop_impl_t() :
      fdEpoll(::epoll_create1(EPOLL_CLOEXEC)) {
      if (fdEpoll == -1) {
         throw_os_error();
      }
   }

   //! Destructor.
   ~event_loop_impl_t() {
      ABC_FOR_EACH(int fdEvent, vfdEvents) {
         ::close(fdEvent);
      }
      ::close(fdEpoll);
   }

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_BSD … elif ABC_HOST_API_LINUX
   //! Constructor.
   event_loop_impl_t() {
   }

   //! Destructor.
   ~event_loop_impl_t() {
   }

#else //if ABC_HOST_API_BSD … elif ABC_HOST_API_LINUX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_BSD … elif ABC_HOST_API_LINUX … elif ABC_HOST_API_WIN32 … else
};

} //namespace

event_loop::event_loop() :
   m_pImpl(new event_loop_impl_t) {
}

event_loop::~event_loop() {
   delete static_cast<event_loop_impl_t *>(m_pImpl);
}

void event_loop::add_file_source(
   std::shared_ptr<io::binary::file_base> pfile, file_event_handler_t fnHandler
) {
   ABC_TRACE_FUNC(this, m_pImpl, pfile/*, fnHandler*/);

   event_loop_impl_t * pimpl = static_cast<event_loop_impl_t *>(m_pImpl);
   ABC_UNUSED_ARG(pimpl);
   ABC_UNUSED_ARG(pfile);
   ABC_UNUSED_ARG(fnHandler);
}

void event_loop::add_process_source(
   std::shared_ptr<process> pproc, process_event_handler_t fnHandler
) {
   ABC_TRACE_FUNC(this, m_pImpl, pproc/*, fnHandler*/);

   event_loop_impl_t * pimpl = static_cast<event_loop_impl_t *>(m_pImpl);
   ABC_UNUSED_ARG(pimpl);
   ABC_UNUSED_ARG(pproc);
   ABC_UNUSED_ARG(fnHandler);
}

void event_loop::add_thread_source(std::shared_ptr<thread> pthr, thread_event_handler_t fnHandler) {
   ABC_TRACE_FUNC(this, m_pImpl, pthr/*, fnHandler*/);

   event_loop_impl_t * pimpl = static_cast<event_loop_impl_t *>(m_pImpl);
   ABC_UNUSED_ARG(pimpl);
   ABC_UNUSED_ARG(pthr);
   ABC_UNUSED_ARG(fnHandler);
}

timer event_loop::add_timer_source(std::uint32_t iMilliseconds, timer_event_handler_t fnHandler) {
   ABC_TRACE_FUNC(this, m_pImpl, iMilliseconds/*, fnHandler*/);

   event_loop_impl_t * pimpl = static_cast<event_loop_impl_t *>(m_pImpl);
   ABC_UNUSED_ARG(pimpl);
   ABC_UNUSED_ARG(iMilliseconds);
   ABC_UNUSED_ARG(fnHandler);
   return timer();
}

void event_loop::run() {
   ABC_TRACE_FUNC(this, m_pImpl);

   event_loop_impl_t * pimpl = static_cast<event_loop_impl_t *>(m_pImpl);

#if ABC_HOST_API_BSD

   collections::smvector<struct ::kevent, 16> vkeReady;
   pimpl->bChanged = true;
   for (;;) {
      // TODO: compare & swap bChanged.
      if (pimpl->bChanged) {
         pimpl->bChanged = false;
         std::size_t cSources = 0;
         // TODO: calculate new capacity.
         if (!cSources) {
            return;
         }
         vkeReady.set_capacity(cSources, false);
      }

      int cReadyEvents = ::kevent(
         pimpl->fdKqueue, nullptr, 0, vkeReady.begin().base(),
         static_cast<int>(vkeReady.capacity()), nullptr
      );
      if (cReadyEvents == -1) {
         int iErr = errno;
         if (iErr == EINTR) {
            continue;
         }
         throw_os_error(iErr);
      }
      // Resize the vector to include only elements written by epoll_wait().
      vkeReady.set_size(static_cast<std::size_t>(cReadyEvents));
      ABC_FOR_EACH(struct ::kevent const & ke, vkeReady) {
         // TODO: consume the event.
         ABC_UNUSED_ARG(ke);
      }
   }

#elif ABC_HOST_API_LINUX //if ABC_HOST_API_BSD

   collections::smvector< ::epoll_event, 16> veeReady;
   pimpl->bChanged = true;
   for (;;) {
      // TODO: compare & swap bChanged.
      if (pimpl->bChanged) {
         pimpl->bChanged = false;
         std::size_t cSources = 0;
         // TODO: calculate new capacity.
         if (!cSources) {
            return;
         }
         veeReady.set_capacity(cSources, false);
      }

      int cReadyFds = ::epoll_wait(
         pimpl->fdEpoll, veeReady.begin().base(), static_cast<int>(veeReady.capacity()), -1
      );
      if (cReadyFds == -1) {
         int iErr = errno;
         if (iErr == EINTR) {
            continue;
         }
         throw_os_error(iErr);
      }
      // Resize the vector to include only elements written by epoll_wait().
      veeReady.set_size(static_cast<std::size_t>(cReadyFds));
      ABC_FOR_EACH(::epoll_event const & ee, veeReady) {
         // TODO: consume the event.
         ABC_UNUSED_ARG(ee);
      }
   }

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_BSD … elif ABC_HOST_API_LINUX

   collections::smvector<HANDLE, 64> vhSources;
   DWORD cSources;
   pimpl->bChanged = true;
   for (;;) {
      // TODO: compare & swap bChanged.
      if (pimpl->bChanged) {
         pimpl->bChanged = false;
         // TODO: rebuild vhSources.
         cSources = static_cast<DWORD>(vhSources.size());
         if (!cSources) {
            return;
         }
         if (cSources > MAXIMUM_WAIT_OBJECTS) {
            // TODO: spawn new thread(s) just to wait on the excess handles. Complicated.
         }
      } else {
         // TODO: shuffle vhSources to avoid neglecting higher-indexed events.
      }

      DWORD iRet = ::WaitForMultipleObjects(cSources, vhSources.begin().base(), false, INFINITE);
      if (iRet == WAIT_FAILED) {
         throw_os_error();
      } else if (iRet >= WAIT_OBJECT_0 && iRet < WAIT_OBJECT_0 + cSources) {
         HANDLE hReady = vhSources[iRet - WAIT_OBJECT_0];
         // TODO: consume the event.
      } else if (iRet >= WAIT_ABANDONED_0 && iRet < WAIT_ABANDONED_0 + cSources) {
         HANDLE hReady = vhSources[iRet - WAIT_ABANDONED_0];
         // TODO: do something about the abandoned event ‒ but what?!
      }
   }

#else //if ABC_HOST_API_BSD … elif ABC_HOST_API_LINUX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_BSD … elif ABC_HOST_API_LINUX … elif ABC_HOST_API_WIN32 … else
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::timer

namespace abc {

timer::timer() {
}

timer::~timer() {
}

void timer::start() {
}

void timer::stop() {
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
