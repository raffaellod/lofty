/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/logging.hxx>
#include <lofty/thread.hxx>
#include "coroutine-scheduler.hxx"

#if LOFTY_HOST_API_POSIX
   #include <fcntl.h> // F_* FD_* O_* fcntl()
   #include <unistd.h> // close()
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io {

filedesc_t const filedesc_t_null =
#if LOFTY_HOST_API_POSIX
   -1;
#elif LOFTY_HOST_API_WIN32
   nullptr;
#else
   #error "TODO: HOST_API"
#endif

filedesc::~filedesc() {
   if (fd != filedesc_t_null) {
      /* Unlike classes that implement io::closeable, we don’t warn about it not having being called, because
      there are many things (especially in Linux/BSD) using file descriptors that won’t generate errors when
      being closed, and it would be annoying to bug about those. */
      close();
   }
}

filedesc & filedesc::operator=(filedesc && src) {
   if (src.fd != fd) {
      if (fd != filedesc_t_null) {
         close();
      }
      fd = src.fd;
#if LOFTY_HOST_API_WIN32
      iocp_fd = src.iocp_fd;
#endif
      src.fd = filedesc_t_null;
#if LOFTY_HOST_API_WIN32
      src.iocp_fd = filedesc_t_null;
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

void filedesc::close() {
#if LOFTY_HOST_API_POSIX
   bool err = (::close(fd) < 0);
#elif LOFTY_HOST_API_WIN32
   bool err = !::CloseHandle(fd);
#else
   #error "TODO: HOST_API"
#endif
   // Yes, this will discard (leak) the file descriptor in case of errors.
   fd = filedesc_t_null;
   if (err) {
      exception::throw_os_error();
   }
}

#if LOFTY_HOST_API_POSIX

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

void filedesc::share_with_subprocesses(bool share) {
#if LOFTY_HOST_API_POSIX
   int flags = ::fcntl(fd, F_GETFD, 0);
   if (flags < 0) {
      exception::throw_os_error();
   }
   if (share) {
      flags &= ~FD_CLOEXEC;
   } else {
      flags |= FD_CLOEXEC;
   }
   if (::fcntl(fd, F_SETFD, FD_CLOEXEC) < 0) {
      exception::throw_os_error();
   }
#elif LOFTY_HOST_API_WIN32
   if (!::SetHandleInformation(fd, HANDLE_FLAG_INHERIT, share ? HANDLE_FLAG_INHERIT : 0u)) {
      exception::throw_os_error();
   }
#else
   #error "TODO: HOST_API"
#endif
}

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io {

/*explicit*/ timeout::timeout(errint_t err_ /*= 0*/) :
   error(err_ ? err_ :
#if LOFTY_HOST_API_POSIX
      ETIMEDOUT
#elif LOFTY_HOST_API_WIN32
      ERROR_TIMEOUT
#else
      0
#endif
   ) {
}

timeout::timeout(timeout const & src) :
   error(src) {
}

/*virtual*/ timeout::~timeout() LOFTY_STL_NOEXCEPT_TRUE() {
}

timeout & timeout::operator=(timeout const & src) {
   error::operator=(src);
   return *this;
}

}} //namespace lofty::io
