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
#include <abaclade/process.hxx>

#if ABC_HOST_API_POSIX
   #include <sys/types.h>
   #include <sys/wait.h>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::process

namespace abc {

process::native_handle_type const process::smc_hNull =
#if ABC_HOST_API_POSIX
   0;
#elif ABC_HOST_API_WIN32
   nullptr;
#else
   #error "TODO: HOST_API"
#endif

/*explicit*/ process::process(id_type id) :
#if ABC_HOST_API_POSIX
   // ID == native handle.
   m_h(id) {
   static_assert(
      sizeof(native_handle_type) == sizeof(::pid_t),
      "pid_t must be the same size as native_handle_type"
   );
#elif ABC_HOST_API_WIN32
   m_h(smc_hNull) {
   ABC_TRACE_FUNC(this);

   // For now, only get a minimum access level.
   m_h = ::OpenProcess(SYNCHRONIZE, false, id);
   if (!m_h) {
      throw_os_error();
   }
#else
   #error "TODO: HOST_API"
#endif
}

process::~process() {
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

void process::detach() {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_API_WIN32
   if (m_h) {
      ::CloseHandle(m_h);
   }
#endif
   m_h = smc_hNull;
}

process::id_type process::id() const {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_API_POSIX
   // ID == native handle.
   return m_h;
#elif ABC_HOST_API_WIN32
   DWORD iPid = ::GetProcessId(m_h);
   if (iPid == 0) {
      throw_os_error();
   }
   return iPid;
#else
   #error "TODO: HOST_API"
#endif
}

void process::join() {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_API_POSIX
   ::siginfo_t si;
   if (::waitid(P_PID, static_cast< ::id_t>(m_h), &si, WEXITED) == -1) {
      throw_os_error();
   }
#elif ABC_HOST_API_WIN32
   DWORD iRet = ::WaitForSingleObject(m_h, INFINITE);
   if (iRet == WAIT_FAILED) {
      throw_os_error();
   }
#else
   #error "TODO: HOST_API"
#endif
}

bool process::joinable() const {
   ABC_TRACE_FUNC(this);

   if (m_h == smc_hNull) {
      return false;
   }
#if ABC_HOST_API_POSIX
   ::siginfo_t si;
   /* waitid() will not touch this field if m_h is not in a waitable (“joinable”) state, so we have
   to in order to check it after the call. */
   si.si_pid = 0;
   if (::waitid(P_PID, static_cast< ::id_t>(m_h), &si, WEXITED | WNOHANG | WNOWAIT) == -1) {
      throw_os_error();
   }
   // waitid() sets this to m_h if the child is in the requested state (WEXITED).
   return si.si_pid != 0;
#elif ABC_HOST_API_WIN32
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
