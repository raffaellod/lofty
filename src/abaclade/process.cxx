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

process::process() :
#if ABC_HOST_API_POSIX
   m_h(0) {
#elif ABC_HOST_API_WIN32
   m_h(nullptr) {
#else
   #error "TODO: HOST_API"
#endif
}

process::~process() {
   if (joinable()) {
      // TODO: std::abort() or something similar.
   }
#if ABC_HOST_API_WIN32
   if (m_h) {
      ::CloseHandle(m_h);
   }
#endif
}

#if ABC_HOST_API_WIN32
id_type process::id() const {
   DWORD iPid = ::GetProcessId(m_h);
   if (iPid == 0) {
      throw_os_error();
   }
   return iPid;
}
#endif

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

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
