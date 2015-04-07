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
#include <abaclade/thread.hxx>

#if ABC_HOST_API_POSIX
   #include <errno.h> // EINVAL errno
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

class thread::shared_data {
private:
   friend class thread;

public:
   /*! Constructor

   @param fnMain
      Initial value for m_fnInnerMain.
   */
   shared_data(std::function<void ()> fnMain) :
      m_fnInnerMain(std::move(fnMain)) {
   }

   //! Destructor.
   ~shared_data() {
   }

   //! Invokes the user-provided thread function.
   void inner_main() {
      m_fnInnerMain();
   }

private:
#if ABC_HOST_API_DARWIN
   //! Dispatch semaphore used by the new thread to report to its parent that it has started.
   ::dispatch_semaphore_t m_dsemReady;
#elif ABC_HOST_API_POSIX
   //! Semaphore used by the new thread to report to its parent that it has started.
   ::sem_t m_semReady;
#elif ABC_HOST_API_WIN32
   //! Event used by the new thread to report to its parent that it has started.
   HANDLE m_hReadyEvent;
#else
   #error "TODO: HOST_API"
#endif
   //! Function to be executed in the thread.
   std::function<void ()> m_fnInnerMain;
};


/*explicit*/ thread::thread(std::function<void ()> fnMain) :
#if ABC_HOST_API_POSIX
   m_id(0),
#elif ABC_HOST_API_WIN32
   m_h(nullptr),
#else
   #error "TODO: HOST_API"
#endif
   m_psd(std::make_shared<shared_data>(std::move(fnMain))) {
   ABC_TRACE_FUNC(this);

   start();
}
thread::thread(thread && thr) :
   m_h(thr.m_h),
#if ABC_HOST_API_POSIX
   m_id(thr.m_id),
#endif
   m_psd(std::move(thr.m_psd)) {
#if ABC_HOST_API_POSIX
   thr.m_id = 0;
   // pthreads does not provide a way to clear thr.m_h.
#else
   thr.m_h = nullptr;
#endif
}

thread::~thread() {
   ABC_TRACE_FUNC(this);

   if (joinable()) {
      // TODO: std::abort() or something similar.
   }
#if ABC_HOST_API_WIN32
   if (m_h) {
      ::CloseHandle(m_h);
   }
#endif
}

thread & thread::operator=(thread && thr) {
   ABC_TRACE_FUNC(this, thr);

   native_handle_type h(thr.m_h);
#if ABC_HOST_API_POSIX
   id_type tid(thr.m_id);
#endif
   detach();
   m_h = h;
#if ABC_HOST_API_POSIX
   // pthreads does not provide a way to clear thr.m_h.
   m_id = tid;
   thr.m_id = 0;
#else
   thr.m_h = nullptr;
#endif
   return *this;
}

bool thread::operator==(thread const & thr) const {
   ABC_TRACE_FUNC(this, thr);

#if ABC_HOST_API_POSIX
   return ::pthread_equal(m_h, thr.m_h);
#elif ABC_HOST_API_WIN32
   if (DWORD iThisTid = ::GetThreadId(m_h)) {
      if (DWORD iOtherTid = ::GetThreadId(thr.m_h)) {
         return iThisTid == iOtherTid;
      }
   }
   exception::throw_os_error();
#else
   #error "TODO: HOST_API"
#endif
}

void thread::detach() {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_API_POSIX
   m_id = 0;
   // pthreads does not provide a way to release m_h.
#elif ABC_HOST_API_WIN32
   if (m_h) {
      ::CloseHandle(m_h);
      m_h = nullptr;
   }
#else
   #error "TODO: HOST_API"
#endif
}

thread::id_type thread::id() const {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_API_POSIX
   return m_id;
#elif ABC_HOST_API_WIN32
   if (m_h) {
      DWORD iTid = ::GetThreadId(m_h);
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

#if ABC_HOST_API_POSIX
   /* pthread_join() would return EINVAL if passed a non-joinable thread ID. Since there’s no such
   thing as an “invalid thread ID” in pthreads that can be trusted to always be non-joinable, we
   duplicate that sanity check here. */
   if (!m_id) {
      ABC_THROW(argument_error, (EINVAL));
   }
   if (int iErr = ::pthread_join(m_h, nullptr)) {
      exception::throw_os_error(iErr);
   }
   m_id = 0;
#elif ABC_HOST_API_WIN32
   DWORD iRet = ::WaitForSingleObject(m_h, INFINITE);
   if (iRet == WAIT_FAILED) {
      exception::throw_os_error();
   }
#else
   #error "TODO: HOST_API"
#endif
}

bool thread::joinable() const {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_API_POSIX
   return m_id != 0;
#elif ABC_HOST_API_WIN32
   if (!m_h) {
      return false;
   }
   DWORD iRet = ::WaitForSingleObject(m_h, 0);
   if (iRet == WAIT_FAILED) {
      exception::throw_os_error();
   }
   return iRet == WAIT_TIMEOUT;
#else
   #error "TODO: HOST_API"
#endif
}

/*static*/
#if ABC_HOST_API_POSIX
   void *
#elif ABC_HOST_API_WIN32
   DWORD WINAPI
#else
   #error "TODO: HOST_API"
#endif
thread::outer_main(void * p) {
   std::shared_ptr<shared_data> psd;
   {
      thread * pthr = static_cast<thread *>(p);
      psd = pthr->m_psd;
#if ABC_HOST_API_POSIX
      pthr->m_id = this_thread::id();
#endif
#if ABC_HOST_API_DARWIN
      ::dispatch_semaphore_signal(psd->m_dsemReady);
#elif ABC_HOST_API_POSIX
      // Report that this thread is done with writing to *pthr.
      ::sem_post(&psd->m_semReady);
#elif ABC_HOST_API_WIN32
      // Report that this thread is done with writing to *pthr.
      ::SetEvent(psd->m_hReadyEvent);
#else
   #error "TODO: HOST_API"
#endif
   }

   try {
      psd->inner_main();
   } catch (std::exception const & x) {
      exception::write_with_scope_trace(nullptr, &x);
      // TODO: support “moving” the exception to a different thread (e.g. join_with_exceptions()).
   } catch (...) {
      exception::write_with_scope_trace();
      // TODO: support “moving” the exception to a different thread (e.g. join_with_exceptions()).
   }
   return 0;
}

void thread::start() {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_API_DARWIN
   m_psd->m_dsemReady = ::dispatch_semaphore_create(0);
   if (!m_psd->m_dsemReady) {
      exception::throw_os_error();
   }
   if (int iErr = ::pthread_create(&m_h, nullptr, &outer_main, this)) {
      ::dispatch_release(m_psd->m_dsemReady);
      exception::throw_os_error(iErr);
   }
   // Block until the new thread is finished updating *this.
   ::dispatch_semaphore_wait(m_psd->m_dsemReady, DISPATCH_TIME_FOREVER);
   ::dispatch_release(m_psd->m_dsemReady);
#elif ABC_HOST_API_POSIX
   if (::sem_init(&m_psd->m_semReady, 0, 0)) {
      exception::throw_os_error();
   }
   if (int iErr = ::pthread_create(&m_h, nullptr, &outer_main, this)) {
      ::sem_destroy(&m_psd->m_semReady);
      exception::throw_os_error(iErr);
   }
   /* Block until the new thread is finished updating *this. The only possible failure is EINTR, so
   we just keep on retrying. */
   while (::sem_wait(&m_psd->m_semReady)) {
      ;
   }
   ::sem_destroy(&m_psd->m_semReady);
#elif ABC_HOST_API_WIN32
   m_psd->m_hReadyEvent = ::CreateEvent(nullptr, true, false, nullptr);
   m_h = ::CreateThread(nullptr, 0, &outer_main, this, 0, nullptr);
   if (!m_h) {
      DWORD iErr = ::GetLastError();
      ::CloseHandle(m_psd->m_hReadyEvent);
      exception::throw_os_error(iErr);
   }
   // Block until the new thread is finished updating *this. Must not fail.
   ::WaitForSingleObject(m_psd->m_hReadyEvent, INFINITE);
   ::CloseHandle(m_psd->m_hReadyEvent);
#else
   #error "TODO: HOST_API"
#endif
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend ‒ specialization for abc::thread

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
// abc::thread

namespace abc {
namespace this_thread {

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

} //namespace this_thread
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
