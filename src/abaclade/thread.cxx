/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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
   #include <errno.h> // EINVAL
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

/*virtual*/ thread::main_args::~main_args() {
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
   int iErr = ::pthread_join(m_h, nullptr);
   if (iErr != 0) {
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

#if ABC_HOST_API_POSIX
/*static*/ void * thread::main(void * p) {
   std::unique_ptr<main_args> pma(static_cast<main_args *>(p));
   #if ABC_HOST_API_DARWIN
      // ID already retrieved by the creating thread.
      ::pthread_threadid_np(nullptr, &pma->pthr->m_id);
   #elif ABC_HOST_API_FREEBSD
      static_assert(
         sizeof pma->pthr->m_id == sizeof(decltype(::pthread_getthreadid_np())),
         "return value of pthread_getthreadid_np() must be the same size as thread::m_id"
      );
      pma->pthr->m_id = ::pthread_getthreadid_np();
   #elif ABC_HOST_API_LINUX
      static_assert(
         sizeof pma->pthr->m_id == sizeof(::pid_t),
         "pid_t must be the same size as native_handle_type"
      );
      // This is a call to ::gettid().
      pma->pthr->m_id = static_cast< ::pid_t>(::syscall(SYS_gettid));
   #else
      #error "TODO: HOST_API"
   #endif
   // Report that this thread is done with writing to *pma->pthr.
   ::sem_post(&pma->semReady);

   // TODO: exception handling similar to the process-wide main().
   pma->run_callback();
   return nullptr;
}
#elif ABC_HOST_API_WIN32
/*static*/ DWORD WINAPI thread::main(void * p) {
   std::unique_ptr<main_args> pma(static_cast<main_args *>(p));

   // TODO: exception handling similar to the process-wide main().
   pma->run_callback();
   return 0;
}
#else
   #error "TODO: HOST_API"
#endif

void thread::start(std::unique_ptr<main_args> pma) {
   ABC_TRACE_FUNC(this, pma);

#if ABC_HOST_API_POSIX
   pma->pthr = this;
   if (::sem_init(&pma->semReady, 0, 0)) {
      throw_os_error();
   }
   int iRet = ::pthread_create(&m_h, nullptr, &main, pma.get());
   if (iRet) {
      throw_os_error(iRet);
   }
   // Block until the new thread is finished updating *this.
   if (::sem_wait(&pma->semReady)) {
      // TODO: clarify who owns *pma at this point. Currently, this thread (pma non-nullptr).
      throw_os_error();
   }
#elif ABC_HOST_API_WIN32
   m_h = ::CreateThread(nullptr, 0, &main, pma.get(), 0, nullptr);
   if (!m_h) {
      throw_os_error();
   }
#else
   #error "TODO: HOST_API"
#endif
   // The new thread has now taken ownership of *pma.
   pma.release();
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
