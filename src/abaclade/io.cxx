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
// abc::io::filedesc

namespace abc {
namespace io {

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
#if ABC_HOST_API_POSIX
      /* The man page for close(2) says:

         “Not checking the return value of close() is a common but nevertheless serious programming
         error. It is quite possible that errors on a previous write(2) operation are first reported
         at the final close(). Not checking the return value when closing the file may lead to
         silent loss of data. This can especially be observed with NFS and with disk quota. Note
         that the return value should only be used for diagnostics. In particular close() should not
         be retried after an EINTR since this may cause a reused descriptor from another thread to
         be closed.”

      For these reasons, this destructor does not check for errno because of the possible values
      that close(2) would set,
      •  EINTR is ignored here since there’s nothing this destructor can do;
      •  EIO is expected to never happen because this destructor relies on abc::io::binary::file* to
         have ensured that any outstanding I/O operations complete before this destructor is called;
      •  EBADF either can’t happen, or if it happens there’s nothing this destructor can do. */
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
      this->~filedesc();
      m_fd = fd.m_fd;
      fd.m_fd = smc_fdNull;
   }
   return *this;
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

} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
