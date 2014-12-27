/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

// #include <abaclade.hxx> already done in exception-os.cxx.
#include <abaclade/io/text/file.hxx>

#include <cstdlib> // std::abort()

#include <errno.h> // errno E*
#include <signal.h> // sigaction sig*()


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals

namespace abc {

void throw_os_error() {
   throw_os_error(errno);
}
void throw_os_error(errint_t err) {
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
      case ENAMETOOLONG: // File name too long (POSIX.1-2001)
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
      case EINTR: // Interrupted function call (POSIX.1-2001)
      case ENOEXEC: // Exec format error (POSIX.1-2001)
      case ENOLCK: // No locks available (POSIX.1-2001)
      case ESRCH: // No such process (POSIX.1-2001)
         ABC_THROW(environment_error, (err));

      case ENODEV: // No such device (POSIX.1-2001)
      case ENOENT: // No such file or directory (POSIX.1-2001)
         ABC_THROW(file_not_found_error, (os::path(), err));

      case EIDRM: // Identifier removed (POSIX.1-2001)
      case EILSEQ: // Illegal byte sequence (POSIX.1-2001, C99)
      case EMULTIHOP: // Multihop attempted (POSIX.1-2001)
      case ENOPROTOOPT: // Protocol not available (POSIX.1-2001)
      default:
         ABC_THROW(generic_error, (err));

      case EAGAIN: // Try again (POSIX.1-2001)
      case EALREADY: // Operation already in progress (POSIX.1-2001)
      case EBUSY: // Device or resource busy (POSIX.1-2001)
      case ECANCELED: // Operation canceled (POSIX.1-2001)
      case EDQUOT: // Quota exceeded (POSIX.1-2001)
      case EEXIST: // File exists (POSIX.1-2001)
      case EFBIG: // File too large (POSIX.1-2001)
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
      case ENFILE: // Too many open files in system (POSIX.1-2001)
#ifdef ENODATA
      case ENODATA: // No data available (POSIX.1-2001)
#endif
#ifdef ENOMEDIUM
      case ENOMEDIUM: // No medium found (Linux)
#endif
      case ENOSPC: // No space left on device (POSIX.1-2001)
      case ENOTDIR: // Not a directory (POSIX.1-2001)
      case ENOTEMPTY: // Directory not empty (POSIX.1-2001)
      case ENOTTY: // Not a typewriter (POSIX.1-2001)
      case ENXIO: // No such device or address (POSIX.1-2001)
      case ENOMSG: // No message of the desired type (POSIX.1-2001)
      case ENOTSUP: // Operation not supported (POSIX.1-2001)
      case EPIPE: // Broken pipe (POSIX.1-2001)
      case EROFS: // Read-only file system (POSIX.1-2001)
      case ESPIPE: // Illegal seek (POSIX.1-2001)
      case ESTALE: // Stale NFS file handle (POSIX.1-2001)
#ifdef ESTRPIPE
      case ESTRPIPE: // Streams pipe error (Linux)
#endif
      case ETXTBSY: // Text file busy (POSIX.1-2001)
// These two values may or may not be different.
#if EWOULDBLOCK != EAGAIN
      case EWOULDBLOCK: // Operation would block (POSIX.1-2001)
#endif
      case EXDEV: // Improper link (POSIX.1-2001)
         ABC_THROW(io_error, (err));

      case ENOMEM: // Out of memory (POSIX.1-2001)
         ABC_THROW(memory_allocation_error, (err));

      case EFAULT: // Bad address (POSIX.1-2001)
         ABC_THROW(memory_address_error, (err));

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

#ifdef ECOMM
      case ECOMM: // Communication error on send (Linux)
#endif
      case ECONNABORTED: // Connection aborted (POSIX.1-2001)
      case ECONNREFUSED: // Connection refused (POSIX.1-2001)
      case ECONNRESET: // Connection reset by peer (POSIX.1-2001)
#ifdef EHOSTDOWN
      case EHOSTDOWN: // Host is down (Linux)
#endif
      case EHOSTUNREACH: // No route to host (POSIX.1-2001)
      case ENETDOWN: // Network is down (POSIX.1-2001)
      case ENETRESET: // Connection aborted by network (POSIX.1-2001)
      case ENETUNREACH: // Network is unreachable (POSIX.1-2001)
      case ENOLINK: // Link has been severed (POSIX.1-2001)
      case ENOTCONN: // Transport endpoint is not connected (POSIX.1-2001)
#ifdef EREMCHG
      case EREMCHG: // Remote address changed (Linux)
#endif
#ifdef EREMOTEIO
      case EREMOTEIO: // Remote I/O error (Linux)
#endif
#ifdef ESHUTDOWN
      case ESHUTDOWN: // Cannot send after socket shutdown (Linux)
#endif
      case ETIMEDOUT: // Connection timed out (POSIX.1-2001)
         ABC_THROW(network_io_error, (err));

      case ENOSYS: // Function not implemented (POSIX.1-2001)
         ABC_THROW(not_implemented_error, (err));

      case EOVERFLOW: // Value too large for defined data type (POSIX.1-2001)
         ABC_THROW(overflow_error, (err));

      case EACCES: // Permission denied (POSIX.1-2001)
      case EPERM: // Operation not permitted (POSIX.1-2001)
         ABC_THROW(security_error, (err));
   }
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::exception

namespace abc {

// These should be member variables of exception::async_handler_manager, but they’re not due to
// their header file requirements.

namespace {

//! Signals that we can translate into C++ exceptions.
int const g_aiHandledSignals[] = {
// Signal (Default action) Description (standard).
// SIGABRT, // (Core) Abort signal from abort(3) (POSIX.1-1990).
// SIGALRM, // (Term) Timer signal from alarm(2) (POSIX.1-1990).
   SIGBUS,  // (Core) Bus error (bad memory access) (POSIX.1-2001).
// SIGCHLD, // (Ign ) Child stopped or terminated (POSIX.1-1990).
// SIGCONT, // (Cont) Continue if stopped (POSIX.1-1990).
   SIGFPE,  // (Core) Floating point exception (POSIX.1-1990).
// SIGHUP,  // (Term) Hangup on controlling terminal or death of controlling process (POSIX.1-1990).
// SIGILL,  // (Core) Illegal Instruction (POSIX.1-1990).
// SIGINT,  // (Term) Interrupt from keyboard (POSIX.1-1990).
// SIGPIPE, // (Term) Broken pipe: write to pipe with no readers (POSIX.1-1990).
// SIGPROF, // (Term) Profiling timer expired (POSIX.1-2001).
// SIGQUIT, // (Core) Quit from keyboard (POSIX.1-1990).
   SIGSEGV  // (Core) Invalid memory reference (POSIX.1-1990).
// SIGTERM  // (Term) Termination signal (POSIX.1-1990).
// SIGTRAP  // (Core) Trace/breakpoint trap (POSIX.1-2001).
// SIGTSTP  // (Stop) Stop typed at terminal (POSIX.1-1990).
// SIGTTIN  // (Stop) Terminal input for background process (POSIX.1-1990).
// SIGTTOU  // (Stop) Terminal output for background process (POSIX.1-1990).
// SIGUSR1  // (Term) User-defined signal 1 (POSIX.1-1990).
// SIGUSR2  // (Term) User-defined signal 2 (POSIX.1-1990).
};
//! Default handler for each of the signals above.
struct ::sigaction g_asaDefault[ABC_COUNTOF(g_aiHandledSignals)];

//! Translates POSIX signals into C++ exceptions, whenever possible.
void eahm_sigaction(int iSignal, ::siginfo_t * psi, void * pctx) {
   ABC_TRACE_FUNC(iSignal, psi, pctx);

   /* Don’t let external programs mess with us: if the source is not the kernel, ignore the error.
   POSIX.1-2008 states that:
      “Historically, an si_code value of less than or equal to zero indicated that the signal was
      generated by a process via the kill() function, and values of si_code that provided additional
      information for implementation-generated signals, such as SIGFPE or SIGSEGV, were all
      positive. […] if si_code is less than or equal to zero, the signal was generated by a process.
      However, since POSIX.1b did not specify that SI_USER (or SI_QUEUE) had a value less than or
      equal to zero, it is not true that when the signal is generated by a process, the value of
      si_code will always be less than or equal to zero. XSI applications should check whether
      si_code is SI_USER or SI_QUEUE in addition to checking whether it is less than or equal to
      zero.”
   So we do exactly that – except we skip checking for SI_USER and SI_QUEUE at this point because
   they don’t apply to many signals this handler takes care of. */
   if (psi->si_code <= 0) {
      return;
   }

   switch (iSignal) {
      case SIGBUS:
         /* TODO: this is the only way we can test SIGBUS on x86, otherwise the program will get
         stuck in an endless memory-allocating loop. How can this be made to only execute when
         running that one test? */

         // Disable alignment checking if the architecture supports it.
#ifdef __GNUC__
   #if defined(__i386__)
         __asm__(
            "pushf\n"
            "andl $0xfffbffff,(%esp)\n"
            "popf"
         );
   #elif defined(__x86_64__)
         __asm__(
            "pushf\n"
            "andl $0xfffffffffffbffff,(%rsp)\n"
            "popf"
         );
   #endif
#endif //ifdef __GNUC__

         /* There aren’t many codes here that are safe to handle; most of them indicate that there
         is some major memory corruption going on, and in that case we really don’t want to keep on
         going – even the code to throw an exception could be compromised. */
         switch (psi->si_code) {
            case BUS_ADRALN: // Invalid address alignment.
               ABC_THROW(abc::memory_access_error, (psi->si_addr));
         }
         break;

      case SIGFPE:
         switch (psi->si_code) {
            case FPE_INTDIV: // Integer divide by zero.
               ABC_THROW(abc::division_by_zero_error, ());

            case FPE_INTOVF: // Integer overflow.
               ABC_THROW(abc::overflow_error, ());

            case FPE_FLTDIV: // Floating-point divide by zero.
            case FPE_FLTOVF: // Floating-point overflow.
            case FPE_FLTUND: // Floating-point underflow.
            case FPE_FLTRES: // Floating-point inexact result.
            case FPE_FLTINV: // Floating-point invalid operation.
            case FPE_FLTSUB: // Subscript out of range.
               ABC_THROW(abc::floating_point_error, ());
         }
         /* At the time of writing, the above case labels don’t leave out any values, but that’s not
         necessarily going to be true in 5 years, so… */
         ABC_THROW(abc::arithmetic_error, ());

      case SIGSEGV:
         if (psi->si_addr == nullptr) {
            ABC_THROW(abc::null_pointer_error, ());
         } else {
            ABC_THROW(abc::memory_address_error, (psi->si_addr));
         }
   }
   /* Handle all unrecognized cases here. Since here we only handle signals for which the default
   actions is a core dump, calling abort (which sends SIGABRT, also causing a core dump) is the same
   as invoking the default action. */
   std::abort();
}

} //namespace


exception::async_handler_manager::async_handler_manager() {
   struct ::sigaction saNew;
   saNew.sa_sigaction = eahm_sigaction;
   sigemptyset(&saNew.sa_mask);
   /* Without SA_NODEFER (POSIX.1-2001), the handler would be disabled during its own execution,
   only to be restored when the handler returns. Since we’ll throw a C++ exception from within the
   handler, the restoration would be skipped, and if the signal were raised again, we’d just crash.
   SA_SIGINFO (POSIX.1-2001) provides the handler with more information about the signal, which we
   use to generate more precise exceptions. */
   saNew.sa_flags = SA_NODEFER | SA_SIGINFO;

   // Setup handlers for the signals in g_aiHandledSignals.
   for (std::size_t i = ABC_COUNTOF(g_aiHandledSignals); i-- > 0; ) {
      ::sigaction(g_aiHandledSignals[i], &saNew, &g_asaDefault[i]);
   }
}

exception::async_handler_manager::~async_handler_manager() {
   // Restore the saved signal handlers.
   for (std::size_t i = ABC_COUNTOF(g_aiHandledSignals); i-- > 0; ) {
      ::sigaction(g_aiHandledSignals[i], &g_asaDefault[i], nullptr);
   }
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
