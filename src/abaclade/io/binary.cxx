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
#include <abaclade/io/binary.hxx>
#include <abaclade/numeric.hxx>
#include <abaclade/os.hxx>
#include <abaclade/thread.hxx>
#include "binary/default_buffered.hxx"
#include "binary/detail/file_init_data.hxx"
#include "binary/file-subclasses.hxx"

#include <algorithm> // std::min()

#if ABC_HOST_API_POSIX
   #include <errno.h> // E* errno
   #include <fcntl.h> // F_* fcntl()
   #include <sys/stat.h> // S_* stat()
   #include <unistd.h> // *_FILENO isatty() open() pipe()
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

_std::shared_ptr<writer> stderr;
_std::shared_ptr<reader> stdin;
_std::shared_ptr<writer> stdout;

/*! Instantiates a binary::base specialization appropriate for the descriptor in *pfid, returning a
shared pointer to it.

@param pfid
   Data that will be passed to the constructor of the file object.
@return
   Shared pointer to the newly created object.
*/
static _std::shared_ptr<file_base> _construct(detail::file_init_data * pfid) {
   ABC_TRACE_FUNC(pfid);

#if ABC_HOST_API_POSIX
   if (::fstat(pfid->fd.get(), &pfid->statFile)) {
      exception::throw_os_error();
   }
   if (S_ISREG(pfid->statFile.st_mode)) {
      switch (pfid->am.base()) {
         case access_mode::read:
            return _std::make_shared<regular_file_reader>(pfid);
         case access_mode::write:
         case access_mode::write_append:
            return _std::make_shared<regular_file_writer>(pfid);
         case access_mode::read_write:
            return _std::make_shared<regular_file_readwriter>(pfid);
      }
   }
   if (S_ISCHR(pfid->statFile.st_mode) && ::isatty(pfid->fd.get())) {
      switch (pfid->am.base()) {
         case access_mode::read:
            return _std::make_shared<console_reader>(pfid);
         case access_mode::write:
            return _std::make_shared<console_writer>(pfid);
         case access_mode::read_write:
            return _std::make_shared<console_readwriter>(pfid);
         case access_mode::write_append:
            // TODO: use a better exception class.
            ABC_THROW(argument_error, ());
      }
   }
   if (S_ISFIFO(pfid->statFile.st_mode) || S_ISSOCK(pfid->statFile.st_mode)) {
      switch (pfid->am.base()) {
         case access_mode::read:
            return _std::make_shared<pipe_reader>(pfid);
         case access_mode::write:
            return _std::make_shared<pipe_writer>(pfid);
         case access_mode::read_write:
            return _std::make_shared<pipe_readwriter>(pfid);
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
                  return _std::make_shared<console_reader>(pfid);
               case access_mode::write:
                  return _std::make_shared<console_writer>(pfid);
               case access_mode::read_write:
                  return _std::make_shared<console_readwriter>(pfid);
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
               return _std::make_shared<regular_file_reader>(pfid);
            case access_mode::write:
            case access_mode::write_append:
               return _std::make_shared<regular_file_writer>(pfid);
            case access_mode::read_write:
               return _std::make_shared<regular_file_readwriter>(pfid);
         }
         break;

      case FILE_TYPE_PIPE:
         // Socket or pipe.
         switch (pfid->am.base()) {
            case access_mode::read:
               return _std::make_shared<pipe_reader>(pfid);
            case access_mode::write:
               return _std::make_shared<pipe_writer>(pfid);
            case access_mode::read_write:
               return _std::make_shared<pipe_readwriter>(pfid);
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
         return _std::make_shared<file_reader>(pfid);
      case access_mode::write:
         return _std::make_shared<file_writer>(pfid);
      case access_mode::read_write:
         return _std::make_shared<file_readwriter>(pfid);
      default:
         // TODO: use a better exception class.
         ABC_THROW(argument_error, ());
   }
}

/*! Returns a new binary I/O object controlling the specified file descriptor.

@param fd
   File descriptor to take ownership of.
@param am
   Desired access mode.
@return
   Pointer to a binary I/O object controlling fd.
*/
static _std::shared_ptr<file_base> _attach(filedesc && fd, access_mode am) {
   ABC_TRACE_FUNC(fd, am);

   detail::file_init_data fid;
   fid.fd = _std::move(fd);
   fid.am = am;
   /* Since this method is supposed to be used only for standard descriptors, assume that OS
   buffering is on. */
   fid.bBypassCache = false;
   return _construct(&fid);
}

ABACLADE_SYM _std::shared_ptr<buffered_reader> buffer_reader(_std::shared_ptr<reader> pbr) {
   // See if *pbr is also a binary::buffered_reader.
   if (auto pbbr = _std::dynamic_pointer_cast<buffered_reader>(pbr)) {
      return _std::move(pbbr);
   }
   // Add a buffering wrapper to *pbr.
   return _std::make_shared<default_buffered_reader>(_std::move(pbr));
}

ABACLADE_SYM _std::shared_ptr<buffered_writer> buffer_writer(_std::shared_ptr<writer> pbw) {
   // See if *pbw is also a binary::buffered_writer.
   if (auto pbbw = _std::dynamic_pointer_cast<buffered_writer>(pbw)) {
      return _std::move(pbbw);
   }
   // Add a buffering wrapper to *pbw.
   return _std::make_shared<default_buffered_writer>(_std::move(pbw));
}

_std::shared_ptr<file_reader> make_reader(io::filedesc && fd) {
   ABC_TRACE_FUNC(fd);

   detail::file_init_data fid;
   fid.fd = _std::move(fd);
   fid.am = access_mode::read;
   fid.bBypassCache = false;
   return _std::dynamic_pointer_cast<file_reader>(_construct(&fid));
}

_std::shared_ptr<file_writer> make_writer(io::filedesc && fd) {
   ABC_TRACE_FUNC(fd);

   detail::file_init_data fid;
   fid.fd = _std::move(fd);
   fid.am = access_mode::write;
   fid.bBypassCache = false;
   return _std::dynamic_pointer_cast<file_writer>(_construct(&fid));
}

_std::shared_ptr<file_readwriter> make_readwriter(io::filedesc && fd) {
   ABC_TRACE_FUNC(fd);

   detail::file_init_data fid;
   fid.fd = _std::move(fd);
   fid.am = access_mode::read_write;
   fid.bBypassCache = false;
   return _std::dynamic_pointer_cast<file_readwriter>(_construct(&fid));
}

_std::shared_ptr<file_base> open(
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
            this_coroutine::interruption_point();
            break;
         case ENAMETOOLONG: // File name too long (POSIX.1-2001)
         case ENOTDIR: // Not a directory (POSIX.1-2001)
            ABC_THROW(os::invalid_path, (op, iErr));
         case ENODEV: // No such device (POSIX.1-2001)
         case ENOENT: // No such file or directory (POSIX.1-2001)
            ABC_THROW(os::path_not_found, (op, iErr));
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
   ::HANDLE h = ::CreateFile(
      op.os_str().c_str(), iAccess, iShareMode, nullptr, iAction, iFlags, nullptr
   );
   if (h == INVALID_HANDLE_VALUE) {
      ::DWORD iErr = ::GetLastError();
      switch (iErr) {
         case ERROR_BAD_PATHNAME: // The specified path is invalid.
         case ERROR_DIRECTORY: // The directory name is invalid.
         case ERROR_INVALID_NAME: // The file name, directory name, or volume label syntax is
            // incorrect.
            ABC_THROW(os::invalid_path, (op, iErr));
         case ERROR_BAD_NETPATH: // The network path was not found.
         case ERROR_BAD_UNIT: // The system cannot find the specified device .
         case ERROR_NO_NET_OR_BAD_PATH: // No network provider accepted the given network path.
         case ERROR_INVALID_DRIVE: // The system cannot find the drive specified.
         case ERROR_PATH_NOT_FOUND: // The system cannot find the path specified.
         case ERROR_UNKNOWN_PORT: // The specified port is unknown.
            ABC_THROW(os::path_not_found, (op, iErr));
         default:
            exception::throw_os_error(iErr);
      }
   }
   fid.fd = filedesc(h);
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
   this_coroutine::interruption_point();
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
      this_coroutine::interruption_point();
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
      this_coroutine::interruption_point();
   }
   fidReader.fd = filedesc(fds[0]);
   fidWriter.fd = filedesc(fds[1]);
#elif ABC_HOST_API_WIN32
   if (bAsync) {
      // Win32 anonymous pipes don’t support asynchronous I/O, so create a named pipe instead.
      static long s_iSerial = 0;
      // Generate the pipe name.
      sstr<128> sPipeName;
      io::text::str_writer(external_buffer, sPipeName.str_ptr()).print(
         ABC_SL("\\\\.\\pipe\\abc::io::binary::pipe\\{}\\{}"),
         ::GetCurrentProcessId(), ::InterlockedIncrement(&s_iSerial)
      );
      /* Pipe buffers are allocated in the kernel’s non-paged memory pool, so this value should be
      small; the smallest it can get is a single memory page. */
      ::DWORD cbBuffer = static_cast< ::DWORD>(memory::page_size());
      // 0 means default connection timeout; irrelevant as we’ll connect the other end immediately.
      ::HANDLE hReader = ::CreateNamedPipe(
         sPipeName.c_str(),
         GENERIC_READ | PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE,
         1, cbBuffer, cbBuffer, 0, nullptr
      );
      if (hReader == INVALID_HANDLE_VALUE) {
         exception::throw_os_error();
      }
      fidReader.fd = filedesc(hReader);
      ::HANDLE hWriter = ::CreateFile(
         sPipeName.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr
      );
      if (hWriter == INVALID_HANDLE_VALUE) {
         // fidReader.fd is closed automatically.
         exception::throw_os_error();
      }
      fidWriter.fd = filedesc(hWriter);
   } else {
      ::HANDLE hReader, hWriter;
      if (!::CreatePipe(&hReader, &hWriter, nullptr, 0)) {
         exception::throw_os_error();
      }
      fidReader.fd = filedesc(hReader);
      fidWriter.fd = filedesc(hWriter);
   }
#else
   #error "TODO: HOST_API"
#endif
   this_coroutine::interruption_point();
   fidReader.am = access_mode::read;
   fidWriter.am = access_mode::write;
   fidReader.bBypassCache = fidWriter.bBypassCache = false;
   return pipe_ends(
      _std::make_shared<pipe_reader>(&fidReader), _std::make_shared<pipe_writer>(&fidWriter)
   );
}

}}} //namespace abc::io::binary

namespace abc { namespace io { namespace binary { namespace detail {

_std::shared_ptr<writer> make_stderr() {
   ABC_TRACE_FUNC();

   /* TODO: under Win32, GUI subsystem programs will get nullptr when calling ::GetStdHandle(). To
   avoid exceptions later when performing I/O on it, we need to ::SetStdHandle() with a file opened
   on “NUL”. This mimics the behavior of Linux GUI programs, where all their standard I/O handles
   are open on /dev/null. */

   return _std::dynamic_pointer_cast<writer>(_attach(filedesc(
#if ABC_HOST_API_POSIX
      STDERR_FILENO
#elif ABC_HOST_API_WIN32
      ::GetStdHandle(STD_ERROR_HANDLE)
#else
   #error "TODO: HOST_API"
#endif
   ), access_mode::write));
}

_std::shared_ptr<reader> make_stdin() {
   ABC_TRACE_FUNC();

   /* TODO: under Win32, GUI subsystem programs will get nullptr when calling ::GetStdHandle(). To
   avoid exceptions later when performing I/O on it, we need to ::SetStdHandle() with a file opened
   on “NUL”. This mimics the behavior of Linux GUI programs, where all their standard I/O handles
   are open on /dev/null. */

   return _std::dynamic_pointer_cast<reader>(_attach(filedesc(
#if ABC_HOST_API_POSIX
      STDIN_FILENO
#elif ABC_HOST_API_WIN32
      ::GetStdHandle(STD_INPUT_HANDLE)
#else
   #error "TODO: HOST_API"
#endif
   ), access_mode::read));
}

_std::shared_ptr<writer> make_stdout() {
   ABC_TRACE_FUNC();

   /* TODO: under Win32, GUI subsystem programs will get nullptr when calling ::GetStdHandle(). To
   avoid exceptions later when performing I/O on it, we need to ::SetStdHandle() with a file opened
   on “NUL”. This mimics the behavior of Linux GUI programs, where all their standard I/O handles
   are open on /dev/null. */

   return _std::dynamic_pointer_cast<writer>(_attach(filedesc(
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

base::base() {
}

/*virtual*/ base::~base() {
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

reader::reader() {
}

/*virtual*/ reader::~reader() {
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

writer::writer() {
}

/*virtual*/ writer::~writer() {
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

buffered_base::buffered_base() {
}

/*virtual*/ buffered_base::~buffered_base() {
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

buffered_reader::buffered_reader() {
}

/*virtual*/ buffered_reader::~buffered_reader() {
}

/*virtual*/ std::size_t buffered_reader::read(void * p, std::size_t cbMax) /*override*/ {
   ABC_TRACE_FUNC(this, p, cbMax);

   std::size_t cbReadTotal(0);
   while (cbMax) {
      // Attempt to read at least the count of bytes requested by the caller.
      std::int8_t const * pbBuf;
      std::size_t cbBuf;
      _std::tie(pbBuf, cbBuf) = peek<std::int8_t>(cbMax);
      if (cbBuf == 0) {
         // No more data available.
         break;
      }
      // Copy whatever was read into the caller-supplied buffer.
      memory::copy(static_cast<std::int8_t *>(p), pbBuf, cbBuf);
      cbReadTotal += cbBuf;
      /* Advance the pointer and decrease the count of bytes to read, so that the next call will
      attempt to fill in the remaining buffer space. */
      p = static_cast<std::int8_t *>(p) + cbBuf;
      cbMax -= cbBuf;
   }
   return cbReadTotal;
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

buffered_writer::buffered_writer() {
}

/*virtual*/ buffered_writer::~buffered_writer() {
}

/*virtual*/ std::size_t buffered_writer::write(void const * p, std::size_t cb) /*override*/ {
   ABC_TRACE_FUNC(this, p, cb);

   // Obtain a buffer large enough.
   std::int8_t * pbBuf;
   std::size_t cbBuf;
   _std::tie(pbBuf, cbBuf) = get_buffer<std::int8_t>(cb);
   // Copy the source data into the buffer.
   memory::copy(pbBuf, static_cast<std::int8_t const *>(p), cb);
   return cb;
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

file_base::file_base(detail::file_init_data * pfid) :
   m_fd(_std::move(pfid->fd)) {
}

/*virtual*/ file_base::~file_base() {
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

file_reader::file_reader(detail::file_init_data * pfid) :
   file_base(pfid) {
}

/*virtual*/ file_reader::~file_reader() {
   /* If *this was a file_readwriter, the file_writer destructor has already been called, and this
   will be a no-op; otherwise it’s safe to do it here, since there’s nothing that could fail when
   closing a file only open for reading. */
   m_fd.safe_close();
}

/*virtual*/ std::size_t file_reader::read(void * p, std::size_t cbMax) /*override*/ {
   ABC_TRACE_FUNC(this, p, cbMax);

#if ABC_HOST_API_POSIX
   // This may repeat in case of EINTR.
   for (;;) {
      ::ssize_t cbRead = ::read(
         m_fd.get(), p, std::min<std::size_t>(cbMax, numeric::max< ::ssize_t>::value)
      );
      if (cbRead >= 0) {
         this_coroutine::interruption_point();
         return static_cast<std::size_t>(cbRead);
      }
      int iErr = errno;
      switch (iErr) {
         case EINTR:
            this_coroutine::interruption_point();
            break;
         case EAGAIN:
   #if EWOULDBLOCK != EAGAIN
         case EWOULDBLOCK:
   #endif
            this_coroutine::sleep_until_fd_ready(m_fd.get(), false);
            break;
         default:
            exception::throw_os_error(iErr);
      }
   }
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   ::DWORD cbRead, cbToRead = static_cast< ::DWORD>(
      std::min<std::size_t>(cbMax, numeric::max< ::DWORD>::value)
   );
   overlapped ovl;
   {
      // Obtain the current file offset and set ovl to start there.
      long ibOffsetHigh = 0;
      ovl.Offset = ::SetFilePointer(m_fd.get(), 0, &ibOffsetHigh, FILE_CURRENT);
      if (ovl.Offset != INVALID_SET_FILE_POINTER || ::GetLastError() == ERROR_SUCCESS) {
         ovl.OffsetHigh = static_cast< ::DWORD>(ibOffsetHigh);
      } else {
         ovl.Offset = 0;
         ovl.OffsetHigh = 0;
      }
   }
   m_fd.bind_to_this_coroutine_scheduler_iocp();
   ::BOOL bRet = ::ReadFile(m_fd.get(), p, cbToRead, &cbRead, &ovl);
   ::DWORD iErr = bRet ? ERROR_SUCCESS : ::GetLastError();
   if (iErr == ERROR_IO_PENDING) {
      this_coroutine::sleep_until_fd_ready(m_fd.get(), false, &ovl);
      iErr = ovl.status();
      cbRead = ovl.transferred_size();
   }
   this_coroutine::interruption_point();
   return check_if_eof_or_throw_os_error(cbRead, iErr) ? 0 : cbRead;
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
}

#if ABC_HOST_API_WIN32
/*virtual*/ bool file_reader::check_if_eof_or_throw_os_error(::DWORD cbRead, ::DWORD iErr) const {
   switch (iErr) {
      case ERROR_SUCCESS:
         return cbRead == 0;
      case ERROR_HANDLE_EOF:
         return true;
      default:
         exception::throw_os_error(iErr);
   }
}
#endif

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

file_writer::file_writer(detail::file_init_data * pfid) :
   file_base(pfid) {
}

/*virtual*/ file_writer::~file_writer() {
   /* Verify that m_fd is no longer open. If that’s not the case, the caller neglected to verify
   that the OS write buffer was flushed successfully. */
   if (m_fd) {
      // This will cause a call to std::terminate().
      //ABC_THROW(destructing_unfinalized_object, (this));
   }
}

/*virtual*/ void file_writer::finalize() /*override*/ {
   ABC_TRACE_FUNC(this);

   m_fd.safe_close();
}

/*virtual*/ void file_writer::flush() /*override*/ {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_API_POSIX
   // TODO: investigate fdatasync().
   // This may repeat in case of EINTR.
   while (::fsync(m_fd.get()) < 0) {
      int iErr = errno;
      if (iErr == EINTR) {
         this_coroutine::interruption_point();
      } else if (
   #if ABC_HOST_API_DARWIN
         iErr == ENOTSUP
   #else
         iErr == EINVAL
   #endif
      ) {
         // m_fd.get() does not support fsync(3); ignore the error.
         break;
      } else {
         exception::throw_os_error();
      }
   }
#elif ABC_HOST_API_WIN32
   if (!::FlushFileBuffers(m_fd.get())) {
      ::DWORD iErr = ::GetLastError();
      if (iErr != ERROR_INVALID_FUNCTION) {
         exception::throw_os_error(iErr);
      }
      // m_fd.get() does not support FlushFileBuffers(); ignore the error.
   }
#else
   #error "TODO: HOST_API"
#endif
   this_coroutine::interruption_point();
}

/*virtual*/ std::size_t file_writer::write(void const * p, std::size_t cb) /*override*/ {
   ABC_TRACE_FUNC(this, p, cb);

   std::int8_t const * pb = static_cast<std::int8_t const *>(p);
#if ABC_HOST_API_POSIX
   // This may repeat in case of EINTR or in case ::write() couldn’t write all the bytes.
   for (;;) {
      std::size_t cbToWrite = std::min<std::size_t>(cb, numeric::max< ::ssize_t>::value);
      ::ssize_t cbWritten = ::write(m_fd.get(), pb, cbToWrite);
      if (cbWritten >= 0) {
         pb += cbWritten;
         cb -= static_cast<std::size_t>(cbWritten);
         if (cb == 0) {
            break;
         }
      } else {
         int iErr = errno;
         switch (iErr) {
            case EINTR:
               this_coroutine::interruption_point();
               break;
            case EAGAIN:
   #if EWOULDBLOCK != EAGAIN
            case EWOULDBLOCK:
   #endif
               this_coroutine::sleep_until_fd_ready(m_fd.get(), true);
               break;
            default:
               exception::throw_os_error(iErr);
         }
      }
   }
   this_coroutine::interruption_point();
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   do {
      ::DWORD cbWritten, cbToWrite = static_cast< ::DWORD>(
         std::min<std::size_t>(cb, numeric::max< ::DWORD>::value)
      );
      overlapped ovl;
      {
         // Obtain the current file offset and set ovl to start there.
         long ibOffsetHigh = 0;
         ovl.Offset = ::SetFilePointer(m_fd.get(), 0, &ibOffsetHigh, FILE_CURRENT);
         if (ovl.Offset != INVALID_SET_FILE_POINTER || ::GetLastError() == ERROR_SUCCESS) {
            ovl.OffsetHigh = static_cast< ::DWORD>(ibOffsetHigh);
         } else {
            ovl.Offset = 0;
            ovl.OffsetHigh = 0;
         }
      }
      m_fd.bind_to_this_coroutine_scheduler_iocp();
      if (!::WriteFile(m_fd.get(), pb, cbToWrite, &cbWritten, &ovl)) {
         ::DWORD iErr = ::GetLastError();
         if (iErr == ERROR_IO_PENDING) {
            this_coroutine::sleep_until_fd_ready(m_fd.get(), true, &ovl);
         }
         iErr = ovl.status();
         if (iErr != ERROR_SUCCESS) {
            exception::throw_os_error(iErr);
         }
         cbWritten = ovl.transferred_size();
      }
      this_coroutine::interruption_point();
      pb += cbWritten;
      cb -= cbWritten;
   } while (cb);
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
   return static_cast<std::size_t>(pb - static_cast<std::int8_t const *>(p));
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

file_readwriter::file_readwriter(detail::file_init_data * pfid) :
   file_base(pfid),
   file_reader(pfid),
   file_writer(pfid) {
}

/*virtual*/ file_readwriter::~file_readwriter() {
}

}}} //namespace abc::io::binary
