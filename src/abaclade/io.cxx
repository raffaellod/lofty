/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
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

#if ABC_HOST_API_POSIX
   #include <fcntl.h> // F_* FD_* O_* fcntl()
   #include <unistd.h> // close()
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io {

filedesc_t const filedesc::smc_fdNull =
#if ABC_HOST_API_POSIX
   -1;
#elif ABC_HOST_API_WIN32
   INVALID_HANDLE_VALUE;
#else
   #error "TODO: HOST_API"
#endif

filedesc::~filedesc() {
   if (m_fd != smc_fdNull) {
      // Ignore errors.
#if ABC_HOST_API_POSIX
      ::close(m_fd);
#elif ABC_HOST_API_WIN32
      ::CloseHandle(m_fd);
#else
   #error "TODO: HOST_API"
#endif
   }
}

filedesc & filedesc::operator=(filedesc && fd) {
   if (fd.m_fd != m_fd) {
      safe_close();
      m_fd = fd.m_fd;
      fd.m_fd = smc_fdNull;
   }
   return *this;
}

void filedesc::safe_close() {
   if (m_fd != smc_fdNull) {
#if ABC_HOST_API_POSIX
      bool bErr = (::close(m_fd) < 0);
#elif ABC_HOST_API_WIN32
      bool bErr = !::CloseHandle(m_fd);
#else
   #error "TODO: HOST_API"
#endif
      // Yes, this will discard (leak) the file descriptor in case of errors.
      m_fd = smc_fdNull;
      if (bErr) {
         exception::throw_os_error();
      }
   }
}

#if ABC_HOST_API_POSIX

void filedesc::set_close_on_exec(bool b) {
   int iFlags = ::fcntl(m_fd, F_GETFD, 0);
   if (iFlags < 0) {
      exception::throw_os_error();
   }
   if (b) {
      iFlags |= FD_CLOEXEC;
   } else {
      iFlags &= ~FD_CLOEXEC;
   }
   if (::fcntl(m_fd, F_SETFD, FD_CLOEXEC) < 0) {
      exception::throw_os_error();
   }
}

void filedesc::set_nonblocking(bool b) {
   int iFlags = ::fcntl(m_fd, F_GETFL, 0);
   if (iFlags < 0) {
      exception::throw_os_error();
   }
   if (b) {
      iFlags |= O_NONBLOCK;
   } else {
      iFlags &= ~O_NONBLOCK;
   }
   if (::fcntl(m_fd, F_SETFL, O_NONBLOCK) < 0) {
      exception::throw_os_error();
   }
}

#endif

}} //namespace abc::io
