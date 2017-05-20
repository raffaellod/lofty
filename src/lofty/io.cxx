/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/thread.hxx>
#include "coroutine-scheduler.hxx"

#if LOFTY_HOST_API_POSIX
   #include <fcntl.h> // F_* FD_* O_* fcntl()
   #include <unistd.h> // close()
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io {

filedesc_t const filedesc::null_fd =
#if LOFTY_HOST_API_POSIX
   -1;
#elif LOFTY_HOST_API_WIN32
   nullptr;
#else
   #error "TODO: HOST_API"
#endif

filedesc::~filedesc() {
   if (fd != null_fd) {
      // Ignore errors.
#if LOFTY_HOST_API_POSIX
      ::close(fd);
#elif LOFTY_HOST_API_WIN32
      ::CloseHandle(fd);
#else
   #error "TODO: HOST_API"
#endif
   }
}

filedesc & filedesc::operator=(filedesc && src) {
   if (src.fd != fd) {
      safe_close();
      fd = src.fd;
#if LOFTY_HOST_API_WIN32
      iocp_fd = src.iocp_fd;
#endif
      src.fd = null_fd;
#if LOFTY_HOST_API_WIN32
      src.iocp_fd = null_fd;
#endif
   }
   return *this;
}

#if LOFTY_HOST_API_WIN32
void filedesc::bind_to_this_coroutine_scheduler_iocp() {
   if (auto coro_sched = this_thread::coroutine_scheduler()) {
      ::HANDLE iocp = coro_sched->iocp();
      if (iocp_fd) {
         if (iocp_fd != iocp) {
            // *this has been associated to (the IOCP of) a different coroutine scheduler in the past.
            // TODO: use a better exception class.
            LOFTY_THROW(argument_error, ());
         }
      } else {
         /* First time *this is associated to (the IOCP of) a coroutine scheduler. This will fail with
         ERROR_INVALID_PARAMETER if fd has not been opened with OVERLAPPED support. */
         if (::CreateIoCompletionPort(fd, iocp, reinterpret_cast< ::ULONG_PTR>(fd), 0)) {
            iocp_fd = iocp;
         } else {
            auto err = ::GetLastError();
            if (err != ERROR_INVALID_PARAMETER) {
               exception::throw_os_error(err);
            }
         }
      }
   }
}
#endif

void filedesc::safe_close() {
   if (fd != null_fd) {
#if LOFTY_HOST_API_POSIX
      bool err = (::close(fd) < 0);
#elif LOFTY_HOST_API_WIN32
      bool err = !::CloseHandle(fd);
#else
   #error "TODO: HOST_API"
#endif
      // Yes, this will discard (leak) the file descriptor in case of errors.
      fd = null_fd;
      if (err) {
         exception::throw_os_error();
      }
   }
}

#if LOFTY_HOST_API_POSIX

void filedesc::set_close_on_exec(bool b) {
   int flags = ::fcntl(fd, F_GETFD, 0);
   if (flags < 0) {
      exception::throw_os_error();
   }
   if (b) {
      flags |= FD_CLOEXEC;
   } else {
      flags &= ~FD_CLOEXEC;
   }
   if (::fcntl(fd, F_SETFD, FD_CLOEXEC) < 0) {
      exception::throw_os_error();
   }
}

void filedesc::set_nonblocking(bool b) {
   int flags = ::fcntl(fd, F_GETFL, 0);
   if (flags < 0) {
      exception::throw_os_error();
   }
   if (b) {
      flags |= O_NONBLOCK;
   } else {
      flags &= ~O_NONBLOCK;
   }
   if (::fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
      exception::throw_os_error();
   }
}

#endif

}} //namespace lofty::io

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_API_WIN32
namespace lofty { namespace io {

::DWORD overlapped::get_result() {
   // This will be thrown away; its value will be (and is already) available in InternalHigh.
   ::DWORD tranferred_bytes;
   ::GetOverlappedResult(nullptr, this, &tranferred_bytes, false);
   ::DWORD err = ::GetLastError();
   // Change Internal from an NTSTATUS to a Win32 error code.
   Internal = err;
   return err;
}

}} //namespace lofty::io
#endif //if LOFTY_HOST_API_WIN32

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io {

/*explicit*/ error::error(errint_t err_ /*= 0*/) :
   generic_error(err_ ? err_ :
#if LOFTY_HOST_API_POSIX
      EIO
#else
      0
#endif
   ) {
}

error::error(error const & src) :
   generic_error(src) {
}

/*virtual*/ error::~error() LOFTY_STL_NOEXCEPT_TRUE() {
}

error & error::operator=(error const & src) {
   generic_error::operator=(src);
   return *this;
}

}} //namespace lofty::io
