/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#include <abaclade/thread.hxx>
#include "coroutine-scheduler.hxx"

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
   nullptr;
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
#if ABC_HOST_API_WIN32
      m_fdIocp = fd.m_fdIocp;
#endif
      fd.m_fd = smc_fdNull;
#if ABC_HOST_API_WIN32
      fd.m_fdIocp = smc_fdNull;
#endif
   }
   return *this;
}

#if ABC_HOST_API_WIN32
void filedesc::bind_to_this_coroutine_scheduler_iocp() {
   if (auto pcorosched = this_thread::coroutine_scheduler()) {
      ::HANDLE hIocp = pcorosched->iocp();
      if (m_fdIocp) {
         if (m_fdIocp != hIocp) {
            /* *this has been associated to (the IOCP of) a different coroutine scheduler in the
            past. */
            // TODO: use a better exception class.
            ABC_THROW(argument_error, ());
         }
      } else {
         /* First time *this is associated to (the IOCP of) a coroutine scheduler.
         This will fail with ERROR_INVALID_PARAMETER if m_fd has not been opened with OVERLAPPED
         support. */
         if (::CreateIoCompletionPort(m_fd, hIocp, reinterpret_cast< ::ULONG_PTR>(m_fd), 0)) {
            m_fdIocp = hIocp;
         } else {
            ::DWORD iErr = ::GetLastError();
            if (iErr != ERROR_INVALID_PARAMETER) {
               exception::throw_os_error(iErr);
            }
         }
      }
   }
}
#endif

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

////////////////////////////////////////////////////////////////////////////////////////////////////

#if ABC_HOST_API_WIN32
namespace abc { namespace io {

::DWORD overlapped::get_result() {
   // This will be thrown away; its value will be (and is already) available in InternalHigh.
   ::DWORD cbTranferred;
   ::GetOverlappedResult(nullptr, this, &cbTranferred, false);
   ::DWORD iErr = ::GetLastError();
   // Change Internal from an NTSTATUS to a Win32 error code.
   Internal = iErr;
   return iErr;
}

}} //namespace abc::io
#endif //if ABC_HOST_API_WIN32

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io {

/*explicit*/ error::error(errint_t err /*= 0*/) :
   generic_error(err ? err :
#if ABC_HOST_API_POSIX
      EIO
#else
      0
#endif
   ) {
}

error::error(error const & x) :
   generic_error(x) {
}

/*virtual*/ error::~error() ABC_STL_NOEXCEPT_TRUE() {
}

error & error::operator=(error const & x) {
   generic_error::operator=(x);
   return *this;
}

}} //namespace abc::io
