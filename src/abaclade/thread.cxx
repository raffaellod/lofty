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
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::thread

namespace abc {

thread::thread() :
#if ABC_HOST_API_POSIX
   m_id(0) {
#elif ABC_HOST_API_WIN32
   m_h(nullptr) {
#else
   #error "TODO: HOST_API"
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

bool thread::operator==(thread const & thr) const {
   ABC_TRACE_FUNC(this/*, thr*/);

#if ABC_HOST_API_POSIX
   return ::pthread_equal(m_h, thr.m_h);
#elif ABC_HOST_API_WIN32
   if (DWORD iThisTid = ::GetThreadId(m_thr.m_h)) {
      if (DWORD iOtherTid = ::GetThreadId(p.m_thr.m_h)) {
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

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
