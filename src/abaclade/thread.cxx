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
   #if ABC_HOST_API_FREEBSD
      #include <pthread_np.h> // pthread_getthreadid_np()
   #elif ABC_HOST_API_LINUX
      #include <sys/syscall.h> // SYS_*
      #include <unistd.h> // syscall()
   #endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::thread

namespace abc {

/*virtual*/ thread::shared_data::~shared_data() {
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

bool thread::operator==(thread const & thr) const {
   ABC_TRACE_FUNC(this/*, thr*/);

#if ABC_HOST_API_POSIX
   return ::pthread_equal(m_h, thr.m_h);
#elif ABC_HOST_API_WIN32
   if (DWORD iThisTid = ::GetThreadId(m_h)) {
      if (DWORD iOtherTid = ::GetThreadId(thr.m_h)) {
         return iThisTid == iOtherTid;
      }
   }
   throw_os_error();
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
   }
   m_h = nullptr;
#else
   #error "TODO: HOST_API"
#endif
}

thread::id_type thread::id() const {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_API_POSIX
   return m_id;
#elif ABC_HOST_API_WIN32
   DWORD iTid = ::GetThreadId(m_h);
   if (iTid == 0) {
      throw_os_error();
   }
   return iTid;
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
      throw_os_error(iErr);
   }
   m_id = 0;
#elif ABC_HOST_API_WIN32
   DWORD iRet = ::WaitForSingleObject(m_h, INFINITE);
   if (iRet == WAIT_FAILED) {
      throw_os_error();
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
      throw_os_error();
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
#if ABC_HOST_API_DARWIN
      ::pthread_threadid_np(nullptr, &pthr->m_id);
      ::dispatch_semaphore_signal(psd->dsemReady);
#elif ABC_HOST_API_POSIX
   #if ABC_HOST_API_FREEBSD
      static_assert(
         sizeof pthr->m_id == sizeof(decltype(::pthread_getthreadid_np())),
         "return value of pthread_getthreadid_np() must be the same size as thread::m_id"
      );
      pthr->m_id = ::pthread_getthreadid_np();
   #elif ABC_HOST_API_LINUX
      static_assert(
         sizeof pthr->m_id == sizeof(::pid_t), "pid_t must be the same size as native_handle_type"
      );
      // This is a call to ::gettid().
      pthr->m_id = static_cast< ::pid_t>(::syscall(SYS_gettid));
   #else
      #error "TODO: HOST_API"
   #endif
      // Report that this thread is done with writing to *pthr.
      ::sem_post(&psd->semReady);
#elif ABC_HOST_API_WIN32
      // Report that this thread is done with writing to *pthr.
      ::SetEvent(psd->hReadyEvent);
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
   m_psd->dsemReady = ::dispatch_semaphore_create(0);
   if (!m_psd->dsemReady) {
      throw_os_error();
   }
   if (int iErr = ::pthread_create(&m_h, nullptr, &outer_main, this)) {
      ::dispatch_release(m_psd->dsemReady);
      throw_os_error(iErr);
   }
   // Block until the new thread is finished updating *this.
   ::dispatch_semaphore_wait(m_psd->dsemReady, DISPATCH_TIME_FOREVER);
   ::dispatch_release(m_psd->dsemReady);
#elif ABC_HOST_API_POSIX
   if (::sem_init(&m_psd->semReady, 0, 0)) {
      throw_os_error();
   }
   if (int iErr = ::pthread_create(&m_h, nullptr, &outer_main, this)) {
      ::sem_destroy(&m_psd->semReady);
      throw_os_error(iErr);
   }
   // Block until the new thread is finished updating *this. The only possible failure is EINTR.
   while (::sem_wait(&m_psd->semReady)) {
      ;
   }
   ::sem_destroy(&m_psd->semReady);
#elif ABC_HOST_API_WIN32
   m_psd->hReadyEvent = ::CreateEvent(nullptr, true, false, nullptr);
   m_h = ::CreateThread(nullptr, 0, &outer_main, this, 0, nullptr);
   if (!m_h) {
      DWORD iErr = ::GetLastError();
      ::CloseHandle(m_psd->hReadyEvent);
      throw_os_error(iErr);
   }
   // Block until the new thread is finished updating *this. Must not fail.
   ::WaitForSingleObject(m_psd->hReadyEvent, INFINITE);
   ::CloseHandle(m_psd->hReadyEvent);
#else
   #error "TODO: HOST_API"
#endif
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
