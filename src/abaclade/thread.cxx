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
// abc::detail::simple_event

/*! Event that can be waited for. Not compatible with coroutines, since it doesn’t yield to a
coroutine::scheduler. */
// TODO: make this a non-coroutine-friendly general-purpose event.
namespace abc {
namespace detail {

class simple_event : public noncopyable {
public:
   //! Constructor.
   simple_event() {
#if ABC_HOST_API_DARWIN
      m_dsem = ::dispatch_semaphore_create(0);
      if (!m_dsem) {
         exception::throw_os_error();
      }
#elif ABC_HOST_API_POSIX
      if (::sem_init(&m_sem, 0, 0)) {
         exception::throw_os_error();
      }
#elif ABC_HOST_API_WIN32
      m_hEvent = ::CreateEvent(nullptr, true, false, nullptr);
      if (!m_hEvent) {
         exception::throw_os_error();
      }
#else
   #error "TODO: HOST_API"
#endif
   }

   //! Destructor.
   ~simple_event() {
#if ABC_HOST_API_DARWIN
      ::dispatch_release(m_dsem);
#elif ABC_HOST_API_POSIX
      ::sem_destroy(&m_sem);
#elif ABC_HOST_API_WIN32
      ::CloseHandle(m_hEvent);
#else
   #error "TODO: HOST_API"
#endif
   }

   //! Raises the event.
   void raise() {
#if ABC_HOST_API_DARWIN
      ::dispatch_semaphore_signal(m_dsem);
#elif ABC_HOST_API_POSIX
      ::sem_post(&m_sem);
#elif ABC_HOST_API_WIN32
      ::SetEvent(m_hEvent);
#else
   #error "TODO: HOST_API"
#endif
   }

   //! Waits for the event to be raised by another thread.
   void wait() {
#if ABC_HOST_API_DARWIN
      ::dispatch_semaphore_wait(m_dsem, DISPATCH_TIME_FOREVER);
#elif ABC_HOST_API_POSIX
      /* Block until the new thread is finished updating *this. The only possible failure is EINTR,
      so we just keep on retrying. */
      while (::sem_wait(&m_sem)) {
         ;
      }
#elif ABC_HOST_API_WIN32
      ::WaitForSingleObject(m_hEvent, INFINITE);
#else
   #error "TODO: HOST_API"
#endif
   }

private:
#if ABC_HOST_API_DARWIN
   //! Underlying dispatch semaphore.
   ::dispatch_semaphore_t m_dsem;
#elif ABC_HOST_API_POSIX
   //! Underlying POSIX semaphore.
   ::sem_t m_sem;
#elif ABC_HOST_API_WIN32
   //! Underlying event.
   ::HANDLE m_hEvent;
#else
   #error "TODO: HOST_API"
#endif
};

} //namespace detail
} //namespace abc

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
      m_pseStarted(nullptr),
      m_fnInnerMain(std::move(fnMain)) {
   }

   //! Destructor.
   ~impl() {
#if ABC_HOST_API_WIN32
      if (m_h) {
         ::CloseHandle(m_h);
      }
#endif
   }

   //! Creates a thread to run outer_main().
   void start(std::shared_ptr<impl> * ppimplThis) {
      ABC_TRACE_FUNC(this);

      detail::simple_event seStarted;
      m_pseStarted = &seStarted;
      try {
#if ABC_HOST_API_POSIX
         if (int iErr = ::pthread_create(&m_h, nullptr, &outer_main, ppimplThis)) {
            exception::throw_os_error(iErr);
         }
#elif ABC_HOST_API_WIN32
         m_h = ::CreateThread(nullptr, 0, &outer_main, ppimplThis, 0, nullptr);
         if (!m_h) {
            exception::throw_os_error();
         }
#else
   #error "TODO: HOST_API"
#endif
         // Block until the new thread is finished updating *this.
         seStarted.wait();
      } catch (...) {
         m_pseStarted = nullptr;
         throw;
      }
      m_pseStarted = nullptr;
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
#endif
      // Report that this thread is done with writing to *pthr.
      pimplThis->m_pseStarted->raise();
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
#if ABC_HOST_API_POSIX
   //! OS-dependent ID for use with OS-specific API (pthread_*_np() functions and other native API).
   id_type m_id;
#endif
   /*! Pointer to an event used by the new thread to report to its parent that it has started. Only
   non-nullptr during the execution of start(). */
   detail::simple_event * m_pseStarted;
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
