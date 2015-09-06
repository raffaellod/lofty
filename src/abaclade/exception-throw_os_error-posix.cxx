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

// #include <abaclade.hxx> already done in throw_os_error.cxx.

#include <errno.h> // errno E*


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

void exception::throw_os_error() {
   throw_os_error(errno);
}
void exception::throw_os_error(errint_t err) {
   ABC_ASSERT(err != 0, ABC_SL("cannot throw an exception for a success"));
   switch (err) {
      case E2BIG: // Argument list too long (POSIX.1-2001)
      case EBADF: // Bad file number (POSIX.1-2001)
#ifdef EBADFD
      case EBADFD: // File descriptor in bad state (Linux)
#endif
      case EBADMSG: // Bad message (POSIX.1-2001)
#ifdef EBADR
      case EBADR: // Invalid request descriptor (Linux)
#endif
#ifdef EBADRQC
      case EBADRQC: // Invalid request code (Linux)
#endif
#ifdef EBADSLT
      case EBADSLT: // Invalid slot (Linux)
#endif
#ifdef ECHRNG
      case ECHRNG: // Channel number out of range (Linux)
#endif
      case EDESTADDRREQ: // Destination address required (POSIX.1-2001)
      case EINVAL: // Invalid argument (POSIX.1-2001)
      case EMSGSIZE: // Message too long (POSIX.1-2001)
#ifdef ENOTBLK
      case ENOTBLK: // Block device required (Linux)
#endif
      case ENOTSOCK: // Socket operation on non-socket (POSIX.1-2001)
         ABC_THROW(argument_error, (err));

      case ERANGE: // Math result not representable (POSIX.1-2001, C99)
         ABC_THROW(arithmetic_error, (err));

#ifdef ENOBUFS
      case ENOBUFS: // No buffer space available (Linux)
         ABC_THROW(buffer_error, (err));
#endif

      case EDOM: // Math argument out of domain of func (POSIX.1-2001, C99)
         ABC_THROW(domain_error, (err));

      case ECHILD: // No child processes (POSIX.1-2001)
      case EDEADLK: // Resource deadlock avoided (POSIX.1-2001)
      case EIDRM: // Identifier removed (POSIX.1-2001)
      case EILSEQ: // Illegal byte sequence (POSIX.1-2001, C99)
      case EINTR: // Interrupted function call (POSIX.1-2001)
      case EMULTIHOP: // Multihop attempted (POSIX.1-2001)
      case ENOEXEC: // Exec format error (POSIX.1-2001)
      case ENOLCK: // No locks available (POSIX.1-2001)
      case ENOPROTOOPT: // Protocol not available (POSIX.1-2001)
      case ESRCH: // No such process (POSIX.1-2001)
      default:
         ABC_THROW(generic_error, (err));

      case EAGAIN: // Try again (POSIX.1-2001)
      case EALREADY: // Operation already in progress (POSIX.1-2001)
      case EBUSY: // Device or resource busy (POSIX.1-2001)
      case ECANCELED: // Operation canceled (POSIX.1-2001)
#ifdef ECOMM
      case ECOMM: // Communication error on send (Linux)
#endif
      case ECONNABORTED: // Connection aborted (POSIX.1-2001)
      case ECONNREFUSED: // Connection refused (POSIX.1-2001)
      case ECONNRESET: // Connection reset by peer (POSIX.1-2001)
      case EDQUOT: // Quota exceeded (POSIX.1-2001)
      case EEXIST: // File exists (POSIX.1-2001)
      case EFBIG: // File too large (POSIX.1-2001)
#ifdef EHOSTDOWN
      case EHOSTDOWN: // Host is down (Linux)
#endif
      case EHOSTUNREACH: // No route to host (POSIX.1-2001)
      case EINPROGRESS: // Operation now in progress (POSIX.1-2001)
      case EIO: // I/O error (POSIX.1-2001)
      case EISDIR: // Is a directory (POSIX.1-2001)
#ifdef EISNAM
      case EISNAM: // Is a named type file (Linux)
#endif
      case ELOOP: // Too many symbolic links encountered (POSIX.1-2001)
#ifdef EMEDIUMTYPE
      case EMEDIUMTYPE: // Wrong medium type (Linux)
#endif
      case EMFILE: // Too many open files (POSIX.1-2001)
      case EMLINK: // Too many links (POSIX.1-2001)
      case ENETDOWN: // Network is down (POSIX.1-2001)
      case ENETRESET: // Connection aborted by network (POSIX.1-2001)
      case ENETUNREACH: // Network is unreachable (POSIX.1-2001)
      case ENFILE: // Too many open files in system (POSIX.1-2001)
#ifdef ENODATA
      case ENODATA: // No data available (POSIX.1-2001)
#endif
      case ENOLINK: // Link has been severed (POSIX.1-2001)
#ifdef ENOMEDIUM
      case ENOMEDIUM: // No medium found (Linux)
#endif
      case ENOSPC: // No space left on device (POSIX.1-2001)
      case ENOTCONN: // Transport endpoint is not connected (POSIX.1-2001)
      case ENOTEMPTY: // Directory not empty (POSIX.1-2001)
      case ENOTTY: // Not a typewriter (POSIX.1-2001)
      case ENXIO: // No such device or address (POSIX.1-2001)
      case ENOMSG: // No message of the desired type (POSIX.1-2001)
      case ENOTSUP: // Operation not supported (POSIX.1-2001)
      case EPIPE: // Broken pipe (POSIX.1-2001)
#ifdef EREMCHG
      case EREMCHG: // Remote address changed (Linux)
#endif
#ifdef EREMOTEIO
      case EREMOTEIO: // Remote I/O error (Linux)
#endif
      case EROFS: // Read-only file system (POSIX.1-2001)
#ifdef ESHUTDOWN
      case ESHUTDOWN: // Cannot send after socket shutdown (Linux)
#endif
      case ESPIPE: // Illegal seek (POSIX.1-2001)
      case ESTALE: // Stale NFS file handle (POSIX.1-2001)
#ifdef ESTRPIPE
      case ESTRPIPE: // Streams pipe error (Linux)
#endif
      case ETIMEDOUT: // Connection timed out (POSIX.1-2001)
      case ETXTBSY: // Text file busy (POSIX.1-2001)
// These two values may or may not be different.
#if EWOULDBLOCK != EAGAIN
      case EWOULDBLOCK: // Operation would block (POSIX.1-2001)
#endif
      case EXDEV: // Improper link (POSIX.1-2001)
         ABC_THROW(io::error, (err));

      case ENOMEM: // Out of memory (POSIX.1-2001)
         ABC_THROW(memory::allocation_error, (err));

      case EFAULT: // Bad address (POSIX.1-2001)
         ABC_THROW(memory::address_error, (err));

      case EADDRINUSE: // Address already in use (POSIX.1-2001).
      case EADDRNOTAVAIL: // Cannot assign requested address (POSIX.1-2001)
      case EAFNOSUPPORT: // Address family not supported (POSIX.1-2001)
      case EISCONN: // Transport endpoint is already connected (POSIX.1-2001)
#ifdef ENOTUNIQ
      case ENOTUNIQ: // Name not unique on network (Linux)
#endif
// These two values are supposed to differ, but on Linux they don’t.
#if EOPNOTSUPP != ENOTSUP
      case EOPNOTSUPP: // Operation not supported on socket (POSIX.1-2001)
#endif
#ifdef EPFNOSUPPORT
      case EPFNOSUPPORT: // Protocol family not supported (Linux)
#endif
      case EPROTO: // Protocol error (POSIX.1-2001)
      case EPROTONOSUPPORT: // Protocol not supported (POSIX.1-2001)
      case EPROTOTYPE: // Protocol wrong type for socket (POSIX.1-2001)
#ifdef ESOCKTNOSUPPORT
      case ESOCKTNOSUPPORT: // Socket type not supported (Linux)
#endif
         ABC_THROW(network_error, (err));

      case ENOSYS: // Function not implemented (POSIX.1-2001)
         ABC_THROW(not_implemented_error, (err));

      case ENAMETOOLONG: // File name too long (POSIX.1-2001)
      case ENOTDIR: // Not a directory (POSIX.1-2001)
         ABC_THROW(os::invalid_path, (os::path(ABC_SL("<not available>")), err));

      case ENODEV: // No such device (POSIX.1-2001)
      case ENOENT: // No such file or directory (POSIX.1-2001)
         ABC_THROW(os::path_not_found, (os::path(ABC_SL("<not available>")), err));

      case EOVERFLOW: // Value too large for defined data type (POSIX.1-2001)
         ABC_THROW(overflow_error, (err));

      case EACCES: // Permission denied (POSIX.1-2001)
      case EPERM: // Operation not permitted (POSIX.1-2001)
         ABC_THROW(security_error, (err));
   }
}

} //namespace abc
