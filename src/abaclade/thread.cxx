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

#include "coroutine-scheduler.hxx"

#include <abaclade.hxx>
#include <abaclade/coroutine.hxx>
#include <abaclade/thread.hxx>

#if ABC_HOST_API_POSIX
   #include <errno.h> // EINVAL errno
   #include <time.h> // nanosleep()
   #if ABC_HOST_API_DARWIN
      #include <dispatch/dispatch.h>
   #else
      #include <semaphore.h>
      #if ABC_HOST_API_FREEBSD
         #include <pthread_np.h> // pthread_getthreadid_np()
      #elif ABC_HOST_API_LINUX
         #include <sys/syscall.h> // SYS_*
         #include <unistd.h> // syscall()
      #endif
   #endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::thread

namespace abc {

class thread::impl {
private:
   friend class thread;

public:
   /*! Constructor

   @param fnMain
      Initial value for m_fnInnerMain.
   */
   impl(std::function<void ()> fnMain) :
#if ABC_HOST_API_POSIX
      m_id(0),
#elif ABC_HOST_API_WIN32
      m_h(nullptr),
#else
   #error "TODO: HOST_API"
#endif
      m_fnInnerMain(std::move(fnMain)) {
   }

   //! Destructor.
   ~impl() {
#if ABC_HOST_API_WIN32
      ::CloseHandle(m_h);
#endif
   }

   //! Creates a thread to run outer_main().
   void start(std::shared_ptr<impl> * ppimplThis) {
      ABC_TRACE_FUNC(this);

#if ABC_HOST_API_DARWIN
      m_dsemStarted = ::dispatch_semaphore_create(0);
      if (!m_dsemStarted) {
         exception::throw_os_error();
      }
      if (int iErr = ::pthread_create(&m_h, nullptr, &outer_main, ppimplThis)) {
         ::dispatch_release(m_dsemStarted);
         exception::throw_os_error(iErr);
      }
      // Block until the new thread is finished updating *this.
      ::dispatch_semaphore_wait(m_dsemStarted, DISPATCH_TIME_FOREVER);
      ::dispatch_release(m_dsemStarted);
#elif ABC_HOST_API_POSIX
      if (::sem_init(&m_semStarted, 0, 0)) {
         exception::throw_os_error();
      }
      if (int iErr = ::pthread_create(&m_h, nullptr, &outer_main, ppimplThis)) {
         ::sem_destroy(&m_semStarted);
         exception::throw_os_error(iErr);
      }
      /* Block until the new thread is finished updating *this. The only possible failure is EINTR,
      so we just keep on retrying. */
      while (::sem_wait(&m_semStarted)) {
         ;
      }
      ::sem_destroy(&m_semStarted);
#elif ABC_HOST_API_WIN32
      m_hStartedEvent = ::CreateEvent(nullptr, true, false, nullptr);
      m_h = ::CreateThread(nullptr, 0, &outer_main, ppimplThis, 0, nullptr);
      if (!m_h) {
         DWORD iErr = ::GetLastError();
         ::CloseHandle(m_hStartedEvent);
         exception::throw_os_error(iErr);
      }
      // Block until the new thread is finished updating *this. Must not fail.
      ::WaitForSingleObject(m_hStartedEvent, INFINITE);
      ::CloseHandle(m_hStartedEvent);
#else
   #error "TODO: HOST_API"
#endif
   }

private:
   /*! Lower-level wrapper for the thread function passed to the constructor. Under POSIX, this is
   also needed to assign the thread ID to the owning abc::thread instance.

   @param p
      *this.
   @return
      Unused.
   */
#if ABC_HOST_API_POSIX
   static void * outer_main(void * p) {
#elif ABC_HOST_API_WIN32
   static DWORD WINAPI outer_main(void * p) {
#else
   #error "TODO: HOST_API"
#endif
      /* Get a copy of the shared_ptr owning *this, so that members will be guaranteed to be
      accessible even after start() returns, in the creating thread.
      Dereferencing p is safe because the creating thread, which owns *p, is blocked, waiting for
      this thread to signal that it’s finished starting. */
      std::shared_ptr<impl> pimplThis(*static_cast<std::shared_ptr<impl> *>(p));
#if ABC_HOST_API_POSIX
      pimplThis->m_id = this_thread::id();
   #if ABC_HOST_API_DARWIN
      ::dispatch_semaphore_signal(pimplThis->m_dsemStarted);
   #else
      // Report that this thread is done with writing to *pthr.
      ::sem_post(&pimplThis->m_semStarted);
   #endif
#elif ABC_HOST_API_WIN32
      // Report that this thread is done with writing to *pthr.
      ::SetEvent(pimplThis->m_hStartedEvent);
#else
   #error "TODO: HOST_API"
#endif
      try {
         pimplThis->m_fnInnerMain();
      } catch (std::exception const & x) {
         exception::write_with_scope_trace(nullptr, &x);
         // TODO: kill every other thread.
      } catch (...) {
         exception::write_with_scope_trace();
         // TODO: kill every other thread.
      }
#if ABC_HOST_API_POSIX
      return nullptr;
#elif ABC_HOST_API_WIN32
      return 0;
#else
   #error "TODO: HOST_API"
#endif
   }

private:
   //! OS-dependent ID/handle.
   native_handle_type m_h;
#if ABC_HOST_API_DARWIN
   //! Dispatch semaphore used by the new thread to report to its parent that it has started.
   ::dispatch_semaphore_t m_dsemStarted;
#elif ABC_HOST_API_POSIX
   //! OS-dependent ID for use with OS-specific API (pthread_*_np() functions and other native API).
   id_type m_id;
   //! Semaphore used by the new thread to report to its parent that it has started.
   ::sem_t m_semStarted;
#elif ABC_HOST_API_WIN32
   //! Event used by the new thread to report to its parent that it has started.
   HANDLE m_hStartedEvent;
#else
   #error "TODO: HOST_API"
#endif
   //! Function to be executed in the thread.
   std::function<void ()> m_fnInnerMain;
};


/*explicit*/ thread::thread(std::function<void ()> fnMain) :
   m_pimpl(std::make_shared<impl>(std::move(fnMain))) {
   ABC_TRACE_FUNC(this);

   m_pimpl->start(&m_pimpl);
}

thread::~thread() {
   if (joinable()) {
      // TODO: std::terminate() or something similar.
   }
}

thread::id_type thread::id() const {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_API_POSIX
   return m_pimpl ? m_pimpl->m_id : 0;
#elif ABC_HOST_API_WIN32
   if (m_pimpl) {
      DWORD iTid = ::GetThreadId(m_pimpl->m_h);
      if (iTid == 0) {
         exception::throw_os_error();
      }
      return iTid;
   } else {
      return 0;
   }
#else
   #error "TODO: HOST_API"
#endif
}

void thread::join() {
   ABC_TRACE_FUNC(this);

   if (!m_pimpl) {
      // TODO: use a better exception class.
      ABC_THROW(argument_error, ());
   }
#if ABC_HOST_API_POSIX
   if (int iErr = ::pthread_join(m_pimpl->m_h, nullptr)) {
      exception::throw_os_error(iErr);
   }
#elif ABC_HOST_API_WIN32
   DWORD iRet = ::WaitForSingleObject(m_pimpl->m_h, INFINITE);
   if (iRet == WAIT_FAILED) {
      exception::throw_os_error();
   }
#else
   #error "TODO: HOST_API"
#endif
   // Release the impl instance; this will also make joinable() return false.
   m_pimpl.reset();
}

thread::native_handle_type thread::native_handle() const {
   return m_pimpl ? m_pimpl->m_h : native_handle_type();
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specialization for abc::thread

namespace abc {

to_str_backend<thread>::to_str_backend() {
}

to_str_backend<thread>::~to_str_backend() {
}

void to_str_backend<thread>::set_format(istr const & sFormat) {
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

void to_str_backend<thread>::write(thread const & thr, io::text::writer * ptwOut) {
   ABC_TRACE_FUNC(this/*, thr*/, ptwOut);

   if (thread::id_type id = thr.id()) {
      m_tsbStr.write(istr(ABC_SL("TID:")), ptwOut);
      m_tsbId.write(id, ptwOut);
   } else {
      m_tsbStr.write(istr(ABC_SL("TID:-")), ptwOut);
   }
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::this_thread

namespace abc {
namespace this_thread {

std::shared_ptr<coroutine::scheduler> const & attach_coroutine_scheduler(
   std::shared_ptr<coroutine::scheduler> pcorosched /*= nullptr*/
) {
   ABC_TRACE_FUNC(pcorosched);

   std::shared_ptr<coroutine::scheduler> & pcoroschedCurr = coroutine::scheduler::sm_pcorosched;
   if (pcorosched) {
      if (pcoroschedCurr) {
         // The current thread already has a coroutine scheduler.
         // TODO: use a better exception class.
         ABC_THROW(generic_error, ());
      }
      pcoroschedCurr = std::move(pcorosched);
   } else {
      // Create and set a new coroutine scheduler if the current thread didn’t already have one.
      if (!pcoroschedCurr) {
         pcoroschedCurr = std::make_shared<coroutine::scheduler>();
      }
   }
   return pcoroschedCurr;
}

std::shared_ptr<coroutine::scheduler> const & get_coroutine_scheduler() {
   return coroutine::scheduler::sm_pcorosched;
}

thread::id_type id() {
#if ABC_HOST_API_DARWIN
   thread::id_type id;
   ::pthread_threadid_np(nullptr, &id);
   return id;
#elif ABC_HOST_API_FREEBSD
   static_assert(
      sizeof(thread::id_type) == sizeof(decltype(::pthread_getthreadid_np())),
      "return type of ::pthread_getthreadid_np() must be the same size as thread::id_type"
   );
   return ::pthread_getthreadid_np();
#elif ABC_HOST_API_LINUX
   static_assert(
      sizeof(thread::id_type) == sizeof(::pid_t), "::pid_t must be the same size as thread::id_type"
   );
   // This is a call to ::gettid().
   return static_cast< ::pid_t>(::syscall(SYS_gettid));
#elif ABC_HOST_API_WIN32
   return ::GetCurrentThreadId();
#else
   #error "TODO: HOST_API"
#endif
}

void run_coroutines() {
   if (auto & pcorosched = get_coroutine_scheduler()) {
      pcorosched->run();
   }
}

void sleep_for_ms(unsigned iMillisecs) {
#if ABC_HOST_API_POSIX
   ::timespec tsRequested, tsRemaining;
   tsRequested.tv_sec = static_cast< ::time_t>(iMillisecs / 1000u);
   tsRequested.tv_nsec = static_cast<long>(
      static_cast<unsigned long>(iMillisecs % 1000u) * 1000000u
   );
   /* This loop will only repeat in case of EINTR. Technically ::nanosleep() may fail with EINVAL,
   but the calculation above makes that impossible. */
   while (::nanosleep(&tsRequested, &tsRemaining) < 0) {
      // Set the new requested time to whatever we didn’t get to sleep in the last nanosleep() call.
      tsRequested = tsRemaining;
   }
#elif ABC_HOST_API_WIN32
   ::Sleep(iMillisecs);
#else
   #error "TODO: HOST_API"
#endif
}

} //namespace this_thread
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
