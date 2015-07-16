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
#include <abaclade/coroutine.hxx>
#include <abaclade/thread.hxx>
#include "binary/detail/file_init_data.hxx"

#if ABC_HOST_API_POSIX
   #include <errno.h> // E* errno
   #include <fcntl.h> // F_* fcntl()
   #include <sys/stat.h> // S_* stat()
   #include <unistd.h> // *_FILENO isatty() open() pipe()
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

std::shared_ptr<writer> stderr;
std::shared_ptr<reader> stdin;
std::shared_ptr<writer> stdout;

/*! Instantiates a binary::base specialization appropriate for the descriptor in *pfid, returning a
shared pointer to it.

pfid
   Data that will be passed to the constructor of the file object.
return
   Shared pointer to the newly created object.
*/
static std::shared_ptr<file_base> _construct(detail::file_init_data * pfid) {
   ABC_TRACE_FUNC(pfid);

#if ABC_HOST_API_POSIX
   if (::fstat(pfid->fd.get(), &pfid->statFile)) {
      exception::throw_os_error();
   }
   if (S_ISREG(pfid->statFile.st_mode)) {
      switch (pfid->am.base()) {
         case access_mode::read:
            return std::make_shared<regular_file_reader>(pfid);
         case access_mode::write:
         case access_mode::write_append:
            return std::make_shared<regular_file_writer>(pfid);
         case access_mode::read_write:
            return std::make_shared<regular_file_readwriter>(pfid);
      }
   }
   if (S_ISCHR(pfid->statFile.st_mode) && ::isatty(pfid->fd.get())) {
      switch (pfid->am.base()) {
         case access_mode::read:
            return std::make_shared<console_reader>(pfid);
         case access_mode::write:
            return std::make_shared<console_writer>(pfid);
         case access_mode::read_write:
            return std::make_shared<console_readwriter>(pfid);
         case access_mode::write_append:
            // TODO: use a better exception class.
            ABC_THROW(argument_error, ());
      }
   }
   if (S_ISFIFO(pfid->statFile.st_mode) || S_ISSOCK(pfid->statFile.st_mode)) {
      switch (pfid->am.base()) {
         case access_mode::read:
            return std::make_shared<pipe_reader>(pfid);
         case access_mode::write:
            return std::make_shared<pipe_writer>(pfid);
         case access_mode::read_write:
            return std::make_shared<pipe_readwriter>(pfid);
         case access_mode::write_append:
            // TODO: use a better exception class.
            ABC_THROW(argument_error, ());
      }
   }
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   switch (::GetFileType(pfid->fd.get())) {
      case FILE_TYPE_CHAR: {
         /* Serial line or console.

         Using ::GetConsoleMode() to detect a console handle requires GENERIC_READ access rights,
         which could be a problem with stdout/stderr because we don’t ask for that permission for
         these handles; however, for consoles, “The handles returned by CreateFile,
         CreateConsoleScreenBuffer, and GetStdHandle have the GENERIC_READ and GENERIC_WRITE access
         rights”, so we can trust this to succeed for console handles. */

         ::DWORD iConsoleMode;
         if (::GetConsoleMode(pfid->fd.get(), &iConsoleMode)) {
            switch (pfid->am.base()) {
               case access_mode::read:
                  return std::make_shared<console_reader>(pfid);
               case access_mode::write:
                  return std::make_shared<console_writer>(pfid);
               case access_mode::read_write:
                  return std::make_shared<console_readwriter>(pfid);
               case access_mode::write_append:
                  // TODO: use a better exception class.
                  ABC_THROW(argument_error, ());
            }
         }
         break;
      }
      case FILE_TYPE_DISK:
         // Regular file.
         switch (pfid->am.base()) {
            case access_mode::read:
               return std::make_shared<regular_file_reader>(pfid);
            case access_mode::write:
            case access_mode::write_append:
               return std::make_shared<regular_file_writer>(pfid);
            case access_mode::read_write:
               return std::make_shared<regular_file_readwriter>(pfid);
         }
         break;

      case FILE_TYPE_PIPE:
         // Socket or pipe.
         switch (pfid->am.base()) {
            case access_mode::read:
               return std::make_shared<pipe_reader>(pfid);
            case access_mode::write:
               return std::make_shared<pipe_writer>(pfid);
            case access_mode::read_write:
               return std::make_shared<pipe_readwriter>(pfid);
            case access_mode::write_append:
               // TODO: use a better exception class.
               ABC_THROW(argument_error, ());
         }
         break;

      case FILE_TYPE_UNKNOWN: {
         // Unknown or error.
         ::DWORD iErr = ::GetLastError();
         if (iErr != ERROR_SUCCESS) {
            exception::throw_os_error(iErr);
         }
         break;
      }
   }
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else

   // If a file object was not returned in the code above, return a generic file.
   switch (pfid->am.base()) {
      case access_mode::read:
         return std::make_shared<file_reader>(pfid);
      case access_mode::write:
         return std::make_shared<file_writer>(pfid);
      case access_mode::read_write:
         return std::make_shared<file_readwriter>(pfid);
      default:
         // TODO: use a better exception class.
         ABC_THROW(argument_error, ());
   }
}

/*! Returns a new binary I/O object controlling the specified file descriptor.

fd
   File descriptor to take ownership of.
am
   Desired access mode.
return
   Pointer to a binary I/O object controlling fd.
*/
static std::shared_ptr<file_base> _attach(filedesc && fd, access_mode am) {
   ABC_TRACE_FUNC(fd, am);

   detail::file_init_data fid;
   fid.fd = std::move(fd);
   fid.am = am;
   /* Since this method is supposed to be used only for standard descriptors, assume that OS
   buffering is on. */
   fid.bBypassCache = false;
   return _construct(&fid);
}


std::shared_ptr<file_reader> make_reader(io::filedesc && fd) {
   ABC_TRACE_FUNC(fd);

   detail::file_init_data fid;
   fid.fd = std::move(fd);
   fid.am = access_mode::read;
   fid.bBypassCache = false;
   return std::dynamic_pointer_cast<file_reader>(_construct(&fid));
}

std::shared_ptr<file_writer> make_writer(io::filedesc && fd) {
   ABC_TRACE_FUNC(fd);

   detail::file_init_data fid;
   fid.fd = std::move(fd);
   fid.am = access_mode::write;
   fid.bBypassCache = false;
   return std::dynamic_pointer_cast<file_writer>(_construct(&fid));
}

std::shared_ptr<file_readwriter> make_readwriter(io::filedesc && fd) {
   ABC_TRACE_FUNC(fd);

   detail::file_init_data fid;
   fid.fd = std::move(fd);
   fid.am = access_mode::read_write;
   fid.bBypassCache = false;
   return std::dynamic_pointer_cast<file_readwriter>(_construct(&fid));
}

std::shared_ptr<file_base> open(
   os::path const & op, access_mode am, bool bBypassCache /*= false*/
) {
   ABC_TRACE_FUNC(op, am, bBypassCache);

   bool bAsync = (this_thread::coroutine_scheduler() != nullptr);
   detail::file_init_data fid;
#if ABC_HOST_API_POSIX
   int iFlags;
   switch (am.base()) {
      case access_mode::read:
         iFlags = O_RDONLY;
         break;
      case access_mode::read_write:
         iFlags = O_RDWR | O_CREAT;
         break;
      case access_mode::write:
         iFlags = O_WRONLY | O_CREAT | O_TRUNC;
         break;
      case access_mode::write_append:
         iFlags = O_APPEND;
         break;
   }
   iFlags |= O_CLOEXEC;
   if (bAsync) {
      iFlags |= O_NONBLOCK;
   }
   #ifdef O_DIRECT
      if (bBypassCache) {
         iFlags |= O_DIRECT;
      }
   #endif
   // Note: this does not compare the new fd against 0; instead it calls fid.fd.operator bool().
   while (!(fid.fd = filedesc(::open(op.os_str().c_str(), iFlags, 0666)))) {
      int iErr = errno;
      switch (iErr) {
         case EINTR:
            break;
         case ENODEV: // No such device (POSIX.1-2001)
         case ENOENT: // No such file or directory (POSIX.1-2001)
            ABC_THROW(file_not_found_error, (op, iErr));
         default:
            exception::throw_os_error(iErr);
      }
   }
   #ifndef O_DIRECT
      #if ABC_HOST_API_DARWIN
         if (bBypassCache) {
            if (::fcntl(fid.fd.get(), F_NOCACHE, 1) < 0) {
               exception::throw_os_error();
            }
         }
      #else
         #error "TODO: HOST_API"
      #endif
   #endif //ifndef O_DIRECT
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   ::DWORD iAccess, iShareMode, iAction, iFlags = FILE_ATTRIBUTE_NORMAL;
   switch (am.base()) {
      case access_mode::read:
         iAccess = GENERIC_READ;
         iShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
         iAction = OPEN_EXISTING;
         break;
      case access_mode::read_write:
         iAccess = GENERIC_READ | GENERIC_WRITE;
         iShareMode = FILE_SHARE_READ;
         iAction = OPEN_ALWAYS;
         break;
      case access_mode::write:
         iAccess = GENERIC_WRITE;
         iShareMode = FILE_SHARE_READ;
         iAction = CREATE_ALWAYS;
         break;
      case access_mode::write_append:
         /* This iAccess combination is FILE_GENERIC_WRITE & ~FILE_WRITE_DATA; MSDN states that “for
         local files, write operations will not overwrite existing data”. Requiring fewer
         permissions, this also allows ::CreateFile() to succeed on files with stricter ACLs. */
         iAccess = FILE_APPEND_DATA | FILE_WRITE_ATTRIBUTES | STANDARD_RIGHTS_WRITE | SYNCHRONIZE;
         iShareMode = FILE_SHARE_READ;
         iAction = OPEN_ALWAYS;
         break;
   }
   if (bAsync) {
      iFlags |= FILE_FLAG_OVERLAPPED;
   }
   if (bBypassCache) {
      // Turn off all caching strategies and buffering.
      iFlags &= ~(FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_RANDOM_ACCESS);
      iFlags |= FILE_FLAG_NO_BUFFERING;
   }
   if (!(fid.fd = filedesc(::CreateFile(
      op.os_str().c_str(), iAccess, iShareMode, nullptr, iAction, iFlags, nullptr
   )))) {
      ::DWORD iErr = ::GetLastError();
      switch (iErr) {
         case ERROR_PATH_NOT_FOUND: // The system cannot find the path specified.
         case ERROR_UNKNOWN_PORT: // The specified port is unknown.
            ABC_THROW(file_not_found_error, (op, iErr));
         default:
            exception::throw_os_error(iErr);
      }
   }
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
   fid.am = am;
   fid.bBypassCache = bBypassCache;
   return _construct(&fid);
}

pipe_ends pipe() {
   ABC_TRACE_FUNC();

   bool bAsync = (this_thread::coroutine_scheduler() != nullptr);
   detail::file_init_data fidReader, fidWriter;
#if ABC_HOST_API_DARWIN
   int fds[2];
   // pipe2() is not available, so emulate it with pipe() + fcntl().
   while (::pipe(fds)) {
      int iErr = errno;
      if (iErr != EINTR) {
         exception::throw_os_error(iErr);
      }
   }
   // Set the .fd members immediately, so they’ll get closed automatically in case of exceptions.
   fidReader.fd = filedesc(fds[0]);
   fidWriter.fd = filedesc(fds[1]);
   /* Note that at this point there’s no hack that will ensure a fork()/exec() from another thread
   won’t leak the two file descriptors. That’s the whole point of pipe2(). */
   fidReader.fd.set_close_on_exec(true);
   fidWriter.fd.set_close_on_exec(true);
   if (bAsync) {
      fidReader.fd.set_nonblocking(true);
      fidWriter.fd.set_nonblocking(true);
   }
#elif ABC_HOST_API_LINUX || ABC_HOST_API_FREEBSD
   int fds[2], iFlags = O_CLOEXEC;
   if (bAsync) {
      iFlags |= O_NONBLOCK;
   }
   while (::pipe2(fds, iFlags)) {
      int iErr = errno;
      if (iErr != EINTR) {
         exception::throw_os_error(iErr);
      }
   }
   fidReader.fd = filedesc(fds[0]);
   fidWriter.fd = filedesc(fds[1]);
#elif ABC_HOST_API_WIN32
   if (bAsync) {
      // Win32 anonymous pipes don’t support asynchronous I/O, so create a named pipe instead.
      static long s_iSerial = 0;
      // Generate the pipe name.
      smstr<128> sPipeName;
      io::text::str_writer(external_buffer, &sPipeName).print(
         ABC_SL("\\\\.\\pipe\\abc::io::binary::pipe\\{}\\{}"),
         ::GetCurrentProcessId(), ::InterlockedIncrement(&s_iSerial)
      );
      /* Pipe buffers are allocated in the kernel’s non-paged memory pool, so this value should be
      small; the smallest it can get is a single memory page. */
      ::DWORD cbBuffer = static_cast< ::DWORD>(memory::page_size());
      // 0 means default connection timeout; irrelevant as we’ll connect the other end immediately.
      fidReader.fd = filedesc(::CreateNamedPipe(
         sPipeName.c_str(),
         GENERIC_READ | PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE,
         1, cbBuffer, cbBuffer, 0, nullptr
      ));
      if (!fidReader.fd) {
         exception::throw_os_error();
      }
      fidWriter.fd = filedesc(::CreateFile(
         sPipeName.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr
      ));
      if (!fidWriter.fd) {
         // fidReader.fd is closed automatically.
         exception::throw_os_error();
      }
   } else {
      ::HANDLE hRead, hWrite;
      if (!::CreatePipe(&hRead, &hWrite, nullptr, 0)) {
         exception::throw_os_error();
      }
      fidReader.fd = filedesc(hRead);
      fidWriter.fd = filedesc(hWrite);
   }
#else
   #error "TODO: HOST_API"
#endif
   fidReader.am = access_mode::read;
   fidWriter.am = access_mode::write;
   fidReader.bBypassCache = fidWriter.bBypassCache = false;
   return pipe_ends(
      std::make_shared<pipe_reader>(&fidReader), std::make_shared<pipe_writer>(&fidWriter)
   );
}

}}} //namespace abc::io::binary

namespace abc { namespace io { namespace binary { namespace detail {

std::shared_ptr<writer> make_stderr() {
   ABC_TRACE_FUNC();

   /* TODO: under Win32, GUI subsystem programs will get nullptr when calling ::GetStdHandle(). To
   avoid exceptions later when performing I/O on it, we need to ::SetStdHandle() with a file opened
   on “NUL”. This mimics the behavior of Linux GUI programs, where all their standard I/O handles
   are open on /dev/null. */

   return std::dynamic_pointer_cast<writer>(_attach(filedesc(
#if ABC_HOST_API_POSIX
      STDERR_FILENO
#elif ABC_HOST_API_WIN32
      ::GetStdHandle(STD_ERROR_HANDLE)
#else
   #error "TODO: HOST_API"
#endif
   ), access_mode::write));
}

std::shared_ptr<reader> make_stdin() {
   ABC_TRACE_FUNC();

   /* TODO: under Win32, GUI subsystem programs will get nullptr when calling ::GetStdHandle(). To
   avoid exceptions later when performing I/O on it, we need to ::SetStdHandle() with a file opened
   on “NUL”. This mimics the behavior of Linux GUI programs, where all their standard I/O handles
   are open on /dev/null. */

   return std::dynamic_pointer_cast<reader>(_attach(filedesc(
#if ABC_HOST_API_POSIX
      STDIN_FILENO
#elif ABC_HOST_API_WIN32
      ::GetStdHandle(STD_INPUT_HANDLE)
#else
   #error "TODO: HOST_API"
#endif
   ), access_mode::read));
}

std::shared_ptr<writer> make_stdout() {
   ABC_TRACE_FUNC();

   /* TODO: under Win32, GUI subsystem programs will get nullptr when calling ::GetStdHandle(). To
   avoid exceptions later when performing I/O on it, we need to ::SetStdHandle() with a file opened
   on “NUL”. This mimics the behavior of Linux GUI programs, where all their standard I/O handles
   are open on /dev/null. */

   return std::dynamic_pointer_cast<writer>(_attach(filedesc(
#if ABC_HOST_API_POSIX
      STDOUT_FILENO
#elif ABC_HOST_API_WIN32
      ::GetStdHandle(STD_OUTPUT_HANDLE)
#else
   #error "TODO: HOST_API"
#endif
   ), access_mode::write));
}

}}}} //namespace abc::io::binary::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

/*virtual*/ base::~base() {
}

}}} //namespace abc::io::binary
