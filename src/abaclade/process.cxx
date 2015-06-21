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
#include <abaclade/process.hxx>

#if ABC_HOST_API_POSIX
   #include <errno.h> // EINVAL errno
   #include <sys/types.h> // id_t pid_t
   #include <sys/wait.h> // waitid() waitpid() W*
   #include <unistd.h> // getpid()
   #if ABC_HOST_API_BSD
      #include <sys/signal.h> // siginfo_t
   #endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

process::native_handle_type const process::smc_hNull =
#if ABC_HOST_API_POSIX
   0;
#elif ABC_HOST_API_WIN32
   nullptr;
#else
   #error "TODO: HOST_API"
#endif

/*explicit*/ process::process(id_type pid) :
#if ABC_HOST_API_POSIX
   // ID == native handle.
   m_h(pid) {
   static_assert(
      sizeof(id_type) == sizeof(::pid_t), "pid_t must be the same size as native_handle_type"
   );
#elif ABC_HOST_API_WIN32
   m_h(smc_hNull) {
   ABC_TRACE_FUNC(this);

   // For now, only get a minimum access level.
   m_h = ::OpenProcess(SYNCHRONIZE, false, pid);
   if (!m_h) {
      exception::throw_os_error();
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
      exception::throw_os_error();
   }
   return iPid;
#else
   #error "TODO: HOST_API"
#endif
}

int process::join() {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_API_POSIX
   int iStatus;
   while (::waitpid(static_cast< ::pid_t>(m_h), &iStatus, 0) != static_cast< ::pid_t>(m_h)) {
      int iErr = errno;
      if (iErr != EINTR) {
         exception::throw_os_error(iErr);
      }
   }
   if (WIFEXITED(iStatus)) {
      return WEXITSTATUS(iStatus);
   } else if (WIFSIGNALED(iStatus)) {
      return -WTERMSIG(iStatus);
   } else {
      // Should never happen.
      return -1;
   }
#elif ABC_HOST_API_WIN32
   if (::WaitForSingleObject(m_h, INFINITE) == WAIT_FAILED) {
      exception::throw_os_error();
   }
   DWORD iExitCode;
   if (!::GetExitCodeProcess(m_h, &iExitCode)) {
      exception::throw_os_error();
   }
   return static_cast<int>(iExitCode);
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
   if (::waitid(P_PID, static_cast< ::id_t>(m_h), &si, WEXITED | WNOHANG | WNOWAIT)) {
      exception::throw_os_error();
   }
   // waitid() sets this to m_h if the child is in the requested state (WEXITED).
   return si.si_pid != 0;
#elif ABC_HOST_API_WIN32
   DWORD iRet = ::WaitForSingleObject(m_h, 0);
   if (iRet == WAIT_FAILED) {
      exception::throw_os_error();
   }
   return iRet == WAIT_TIMEOUT;
#else
   #error "TODO: HOST_API"
#endif
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

to_str_backend<process>::to_str_backend() {
}

to_str_backend<process>::~to_str_backend() {
}

void to_str_backend<process>::set_format(istr const & sFormat) {
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

void to_str_backend<process>::write(process const & proc, io::text::writer * ptwOut) {
   ABC_TRACE_FUNC(this/*, proc*/, ptwOut);

   if (process::id_type id = proc.id()) {
      m_tsbStr.write(istr(ABC_SL("TID:")), ptwOut);
      m_tsbId.write(id, ptwOut);
   } else {
      m_tsbStr.write(istr(ABC_SL("TID:-")), ptwOut);
   }
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace this_process {

process::id_type id() {
#if ABC_HOST_API_POSIX
   return ::getpid();
#elif ABC_HOST_API_WIN32
   return ::GetCurrentProcessId();
#else
   #error "TODO: HOST_API"
#endif
}

}} //namespace abc::this_process
