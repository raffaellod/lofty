/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/bitmanip.hxx>
#include <lofty/coroutine.hxx>
#include <lofty/exception.hxx>
#include <lofty/io.hxx>
#include <lofty/io/binary.hxx>
#include <lofty/io/binary/buffer.hxx>
#include <lofty/io/binary/memory.hxx>
#include <lofty/logging.hxx>
#include <lofty/memory.hxx>
#include <lofty/numeric.hxx>
#include <lofty/os.hxx>
#include <lofty/os/path.hxx>
#include <lofty/_std/algorithm.hxx>
#include <lofty/_std/memory.hxx>
#include <lofty/_std/utility.hxx>
#include <lofty/thread.hxx>
#include "binary/default_buffered.hxx"
#include "binary/file-subclasses.hxx"
#include "binary/_pvt/file_init_data.hxx"
#if LOFTY_HOST_API_POSIX
   #include <errno.h> // E* errno
   #include <fcntl.h> // F_* fcntl()
   #include <sys/stat.h> // S_* stat()
   #include <unistd.h> // *_FILENO isatty() open() pipe()
#elif LOFTY_HOST_API_WIN32
   #include <lofty/text/str.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

_std::shared_ptr<ostream> stderr;
_std::shared_ptr<istream> stdin;
_std::shared_ptr<ostream> stdout;

_LOFTY_PUBNS_END

/*! Instantiates a file_stream subclass appropriate for the descriptor in *init_data, returning a shared
pointer to it.

@param init_data
   Data that will be passed to the constructor of the file stream.
@return
   Shared pointer to the newly created stream.
*/
static _std::shared_ptr<file_stream> _construct(_pvt::file_init_data * init_data) {
#if LOFTY_HOST_API_POSIX
   if (::fstat(init_data->fd.get(), &init_data->stat)) {
      exception::throw_os_error();
   }
   if (S_ISREG(init_data->stat.st_mode)) {
      switch (init_data->mode.base()) {
         case access_mode::read:
            return _std::make_shared<regular_file_istream>(init_data);
         case access_mode::write:
         case access_mode::write_append:
            return _std::make_shared<regular_file_ostream>(init_data);
         case access_mode::read_write:
            return _std::make_shared<regular_file_iostream>(init_data);
      }
   }
   if (S_ISCHR(init_data->stat.st_mode) && ::isatty(init_data->fd.get())) {
      switch (init_data->mode.base()) {
         case access_mode::read:
            return _std::make_shared<tty_istream>(init_data);
         case access_mode::write:
            return _std::make_shared<tty_ostream>(init_data);
         case access_mode::read_write:
            return _std::make_shared<tty_iostream>(init_data);
         case access_mode::write_append:
            // TODO: use a better exception class.
            LOFTY_THROW(argument_error, ());
      }
   }
   if (S_ISFIFO(init_data->stat.st_mode) || S_ISSOCK(init_data->stat.st_mode)) {
      switch (init_data->mode.base()) {
         case access_mode::read:
            return _std::make_shared<pipe_istream>(init_data);
         case access_mode::write:
            return _std::make_shared<pipe_ostream>(init_data);
         case access_mode::read_write:
            return _std::make_shared<pipe_iostream>(init_data);
         case access_mode::write_append:
            // TODO: use a better exception class.
            LOFTY_THROW(argument_error, ());
      }
   }
#elif LOFTY_HOST_API_WIN32 //if LOFTY_HOST_API_POSIX
   switch (::GetFileType(init_data->fd.get())) {
      case FILE_TYPE_CHAR: {
         /* Serial line or console.

         Using ::GetConsoleMode() to detect a console handle requires GENERIC_READ access rights, which could
         be a problem with stdout/stderr because we don’t ask for that permission for these handles; however,
         for consoles, “The handles returned by CreateFile, CreateConsoleScreenBuffer, and GetStdHandle have
         the GENERIC_READ and GENERIC_WRITE access rights”, so we can trust this to succeed for console
         handles. */

         ::DWORD console_mode;
         if (::GetConsoleMode(init_data->fd.get(), &console_mode)) {
            switch (init_data->mode.base()) {
               case access_mode::read:
                  return _std::make_shared<tty_istream>(init_data);
               case access_mode::write:
                  return _std::make_shared<tty_ostream>(init_data);
               case access_mode::read_write:
                  return _std::make_shared<tty_iostream>(init_data);
               case access_mode::write_append:
                  // TODO: use a better exception class.
                  LOFTY_THROW(argument_error, ());
            }
         }
         break;
      }
      case FILE_TYPE_DISK:
         // Regular file.
         switch (init_data->mode.base()) {
            case access_mode::read:
               return _std::make_shared<regular_file_istream>(init_data);
            case access_mode::write:
            case access_mode::write_append:
               return _std::make_shared<regular_file_ostream>(init_data);
            case access_mode::read_write:
               return _std::make_shared<regular_file_iostream>(init_data);
         }
         break;

      case FILE_TYPE_PIPE:
         // Socket or pipe.
         switch (init_data->mode.base()) {
            case access_mode::read:
               return _std::make_shared<pipe_istream>(init_data);
            case access_mode::write:
               return _std::make_shared<pipe_ostream>(init_data);
            case access_mode::read_write:
               return _std::make_shared<pipe_iostream>(init_data);
            case access_mode::write_append:
               // TODO: use a better exception class.
               LOFTY_THROW(argument_error, ());
         }
         break;

      case FILE_TYPE_UNKNOWN: {
         // Unknown or error.
         ::DWORD err = ::GetLastError();
         if (err != ERROR_SUCCESS) {
            exception::throw_os_error(err);
         }
         break;
      }
   }
#else //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32 … else

   // If a file object was not returned in the code above, return a generic file.
   switch (init_data->mode.base()) {
      case access_mode::read:
         return _std::make_shared<file_istream>(init_data);
      case access_mode::write:
         return _std::make_shared<file_ostream>(init_data);
      case access_mode::read_write:
         return _std::make_shared<file_iostream>(init_data);
      default:
         // TODO: use a better exception class.
         LOFTY_THROW(argument_error, ());
   }
}

/*! Returns a new binary stream controlling the specified file descriptor.

@param fd
   File descriptor to take ownership of.
@param am
   Desired access mode.
@return
   Pointer to a binary stream controlling fd.
*/
static _std::shared_ptr<file_stream> _attach(filedesc && fd, access_mode mode) {
   _pvt::file_init_data init_data;
   init_data.fd = _std::move(fd);
   init_data.mode = mode;
   // Since this method is supposed to be used only for standard descriptors, assume that OS buffering is on.
   init_data.bypass_cache = false;
   return _construct(&init_data);
}

_LOFTY_PUBNS_BEGIN

LOFTY_SYM _std::shared_ptr<buffered_istream> buffer_istream(_std::shared_ptr<istream> bin_istream) {
   // See if *bin_istream is also a binary::buffered_istream.
   if (auto buf_bin_istream = _std::dynamic_pointer_cast<buffered_istream>(bin_istream)) {
      return _std::move(buf_bin_istream);
   } else {
      // Add a buffering wrapper to *bin_istream.
      return _std::make_shared<default_buffered_istream>(_std::move(bin_istream));
   }
}

LOFTY_SYM _std::shared_ptr<buffered_ostream> buffer_ostream(_std::shared_ptr<ostream> bin_ostream) {
   // See if *bin_ostream is also a binary::buffered_ostream.
   if (auto buf_bin_ostream = _std::dynamic_pointer_cast<buffered_ostream>(bin_ostream)) {
      return _std::move(buf_bin_ostream);
   } else {
      // Add a buffering wrapper to *bin_ostream.
      return _std::make_shared<default_buffered_ostream>(_std::move(bin_ostream));
   }
}

_std::shared_ptr<file_istream> make_istream(io::filedesc && fd) {
   _pvt::file_init_data init_data;
   init_data.fd = _std::move(fd);
   init_data.mode = access_mode::read;
   init_data.bypass_cache = false;
   return _std::dynamic_pointer_cast<file_istream>(_construct(&init_data));
}

_std::shared_ptr<file_ostream> make_ostream(io::filedesc && fd) {
   _pvt::file_init_data init_data;
   init_data.fd = _std::move(fd);
   init_data.mode = access_mode::write;
   init_data.bypass_cache = false;
   return _std::dynamic_pointer_cast<file_ostream>(_construct(&init_data));
}

_std::shared_ptr<file_iostream> make_iostream(io::filedesc && fd) {
   _pvt::file_init_data init_data;
   init_data.fd = _std::move(fd);
   init_data.mode = access_mode::read_write;
   init_data.bypass_cache = false;
   return _std::dynamic_pointer_cast<file_iostream>(_construct(&init_data));
}

_std::shared_ptr<file_stream> open(os::path const & path, access_mode mode, bool bypass_cache /*= false*/) {
   bool async = (this_thread::coroutine_scheduler() != nullptr);
   _pvt::file_init_data init_data;
#if LOFTY_HOST_API_POSIX
   int flags;
   switch (mode.base()) {
      case access_mode::read:
         flags = O_RDONLY;
         break;
      case access_mode::read_write:
         flags = O_RDWR | O_CREAT;
         break;
      case access_mode::write:
         flags = O_WRONLY | O_CREAT | O_TRUNC;
         break;
      case access_mode::write_append:
         flags = O_APPEND;
         break;
   }
   flags |= O_CLOEXEC;
   if (async) {
      flags |= O_NONBLOCK;
   }
   #ifdef O_DIRECT
      if (bypass_cache) {
         flags |= O_DIRECT;
      }
   #endif
   // Note: this does not compare the new fd against 0; instead it calls init_data.fd.operator bool().
   while (!(init_data.fd = filedesc(::open(path.os_str().c_str(), flags, 0666)))) {
      int err = errno;
      switch (err) {
         case EINTR:
            this_coroutine::interruption_point();
            break;
         case ENAMETOOLONG: // File name too long (POSIX.1-2001)
         case ENOTDIR: // Not a directory (POSIX.1-2001)
            LOFTY_THROW(os::invalid_path, (path, err));
         case ENODEV: // No such device (POSIX.1-2001)
         case ENOENT: // No such file or directory (POSIX.1-2001)
            LOFTY_THROW(os::path_not_found, (path, err));
         default:
            exception::throw_os_error(err);
      }
   }
   #ifndef O_DIRECT
      #if LOFTY_HOST_API_DARWIN
         if (bypass_cache) {
            if (::fcntl(init_data.fd.get(), F_NOCACHE, 1) < 0) {
               exception::throw_os_error();
            }
         }
      #else
         #error "TODO: HOST_API"
      #endif
   #endif //ifndef O_DIRECT
#elif LOFTY_HOST_API_WIN32 //if LOFTY_HOST_API_POSIX
   ::DWORD access, sharing, action, flags = FILE_ATTRIBUTE_NORMAL;
   switch (mode.base()) {
      case access_mode::read:
         access = GENERIC_READ;
         sharing = FILE_SHARE_READ | FILE_SHARE_WRITE;
         action = OPEN_EXISTING;
         break;
      case access_mode::read_write:
         access = GENERIC_READ | GENERIC_WRITE;
         sharing = FILE_SHARE_READ;
         action = OPEN_ALWAYS;
         break;
      case access_mode::write:
         access = GENERIC_WRITE;
         sharing = FILE_SHARE_READ;
         action = CREATE_ALWAYS;
         break;
      case access_mode::write_append:
         /* This access combination is FILE_GENERIC_WRITE & ~FILE_WRITE_DATA; MSDN states that “for local
         files, write operations will not overwrite existing data”. Requiring fewer permissions, this also
         allows ::CreateFile() to succeed on files with stricter ACLs. */
         access = FILE_APPEND_DATA | FILE_WRITE_ATTRIBUTES | STANDARD_RIGHTS_WRITE | SYNCHRONIZE;
         sharing = FILE_SHARE_READ;
         action = OPEN_ALWAYS;
         break;
   }
   if (async) {
      flags |= FILE_FLAG_OVERLAPPED;
   }
   if (bypass_cache) {
      // Turn off all caching strategies and buffering.
      flags &= ~(FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_RANDOM_ACCESS);
      flags |= FILE_FLAG_NO_BUFFERING;
   }
   ::HANDLE h = ::CreateFile(path.os_str().c_str(), access, sharing, nullptr, action, flags, nullptr);
   if (h == INVALID_HANDLE_VALUE) {
      auto err = ::GetLastError();
      switch (err) {
         case ERROR_BAD_PATHNAME: // The specified path is invalid.
         case ERROR_DIRECTORY: // The directory name is invalid.
         case ERROR_INVALID_NAME: // The file name, directory name, or volume label syntax is incorrect.
            LOFTY_THROW(os::invalid_path, (path, err));
         case ERROR_BAD_NETPATH: // The network path was not found.
         case ERROR_BAD_UNIT: // The system cannot find the specified device .
         case ERROR_NO_NET_OR_BAD_PATH: // No network provider accepted the given network path.
         case ERROR_INVALID_DRIVE: // The system cannot find the drive specified.
         case ERROR_PATH_NOT_FOUND: // The system cannot find the path specified.
         case ERROR_UNKNOWN_PORT: // The specified port is unknown.
            LOFTY_THROW(os::path_not_found, (path, err));
         default:
            exception::throw_os_error(err);
      }
   }
   init_data.fd = filedesc(h);
#else //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32 … else
   this_coroutine::interruption_point();
   init_data.mode = mode;
   init_data.bypass_cache = bypass_cache;
   return _construct(&init_data);
}

_LOFTY_PUBNS_END
}}} //namespace lofty::io::binary

namespace lofty { namespace io { namespace binary { namespace _pvt {

_std::shared_ptr<ostream> make_stderr() {
   /* TODO: under Win32, GUI subsystem programs will get nullptr when calling ::GetStdHandle(). To avoid
   exceptions later when performing I/O on it, we need to ::SetStdHandle() with a file opened on “NUL”. This
   mimics the behavior of Linux GUI programs, where all their standard I/O handles are open on /dev/null. */

   return _std::dynamic_pointer_cast<ostream>(_attach(filedesc(
#if LOFTY_HOST_API_POSIX
      STDERR_FILENO
#elif LOFTY_HOST_API_WIN32
      ::GetStdHandle(STD_ERROR_HANDLE)
#else
   #error "TODO: HOST_API"
#endif
   ), access_mode::write));
}

_std::shared_ptr<istream> make_stdin() {
   /* TODO: under Win32, GUI subsystem programs will get nullptr when calling ::GetStdHandle(). To avoid
   exceptions later when performing I/O on it, we need to ::SetStdHandle() with a file opened on “NUL”. This
   mimics the behavior of Linux GUI programs, where all their standard I/O handles are open on /dev/null. */

   return _std::dynamic_pointer_cast<istream>(_attach(filedesc(
#if LOFTY_HOST_API_POSIX
      STDIN_FILENO
#elif LOFTY_HOST_API_WIN32
      ::GetStdHandle(STD_INPUT_HANDLE)
#else
   #error "TODO: HOST_API"
#endif
   ), access_mode::read));
}

_std::shared_ptr<ostream> make_stdout() {
   /* TODO: under Win32, GUI subsystem programs will get nullptr when calling ::GetStdHandle(). To avoid
   exceptions later when performing I/O on it, we need to ::SetStdHandle() with a file opened on “NUL”. This
   mimics the behavior of Linux GUI programs, where all their standard I/O handles are open on /dev/null. */

   return _std::dynamic_pointer_cast<ostream>(_attach(filedesc(
#if LOFTY_HOST_API_POSIX
      STDOUT_FILENO
#elif LOFTY_HOST_API_WIN32
      ::GetStdHandle(STD_OUTPUT_HANDLE)
#else
   #error "TODO: HOST_API"
#endif
   ), access_mode::write));
}

}}}} //namespace lofty::io::binary::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

stream::stream() {
}

/*virtual*/ stream::~stream() {
}

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

istream::istream() {
}

/*virtual*/ istream::~istream() {
}

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

ostream::ostream() {
}

/*virtual*/ ostream::~ostream() {
}

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

buffered_stream::buffered_stream() {
}

/*virtual*/ buffered_stream::~buffered_stream() {
}

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

buffered_istream::buffered_istream() {
}

/*virtual*/ buffered_istream::~buffered_istream() {
}

/*virtual*/ std::size_t buffered_istream::read_bytes(void * dst, std::size_t dst_max) /*override*/ {
   if (dst_max == 0) {
      // No need to read anything.
      return 0;
   }
   // Attempt to read at least the count of bytes requested by the caller.
   auto buf(peek<std::int8_t>(dst_max));
   if (buf.size == 0) {
      // No more data available (EOF).
      return 0;
   }
   if (buf.size > dst_max) {
      // The caller can’t/won’t get more than dst_max bytes.
      buf.size = dst_max;
   }
   /* Copy whatever was read into the caller-supplied buffer. This extra buffer-to-buffer copy is why using
   peek() directly is preferred. */
   memory::copy(static_cast<std::int8_t *>(dst), buf.ptr, buf.size);
   consume<std::int8_t>(buf.size);
   return buf.size;
}

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

buffered_ostream::buffered_ostream() {
}

/*virtual*/ buffered_ostream::~buffered_ostream() {
}

/*virtual*/ std::size_t buffered_ostream::write_bytes(void const * src, std::size_t src_size) /*override*/ {
   // Obtain a buffer large enough.
   auto buf(get_buffer<std::int8_t>(src_size));
   // Copy the source data into the buffer.
   memory::copy(buf.ptr, static_cast<std::int8_t const *>(src), src_size);
   commit<std::int8_t>(src_size);
   return src_size;
}

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

file_stream::file_stream(_pvt::file_init_data * init_data) :
   fd(_std::move(init_data->fd)) {
}

/*virtual*/ file_stream::~file_stream() {
}

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

file_istream::file_istream(_pvt::file_init_data * init_data) :
   file_stream(init_data) {
}

/*virtual*/ file_istream::~file_istream() {
   if (fd) {
      /* If *this was a file_iostream, the file_ostream destructor has already been called, and this will be a
      no-op; otherwise it’s safe to do it here, since there’s nothing that could fail when closing a file only
      open for reading. */
      fd.close();
   }
}

/*virtual*/ std::size_t file_istream::read_bytes(void * dst, std::size_t dst_max) /*override*/ {
#if LOFTY_HOST_API_POSIX
   // This may repeat in case of EINTR.
   for (;;) {
      ::ssize_t bytes_read = ::read(
         fd.get(), dst, _std::min<std::size_t>(dst_max, numeric::max< ::ssize_t>::value)
      );
      if (bytes_read >= 0) {
         this_coroutine::interruption_point();
         return static_cast<std::size_t>(bytes_read);
      }
      int err = errno;
      switch (err) {
         case EINTR:
            this_coroutine::interruption_point();
            break;
         case EAGAIN:
   #if EWOULDBLOCK != EAGAIN
         case EWOULDBLOCK:
   #endif
            this_coroutine::sleep_until_fd_ready(fd.get(), false /*read*/, 0 /*TODO: timeout*/);
            break;
         default:
            exception::throw_os_error(err);
      }
   }
#elif LOFTY_HOST_API_WIN32 //if LOFTY_HOST_API_POSIX
   ::DWORD bytes_read, bytes_to_read = static_cast< ::DWORD>(
      _std::min<std::size_t>(dst_max, numeric::max< ::DWORD>::value)
   );
   overlapped ovl;
   {
      // Obtain the current file offset and set ovl to start there.
      long offset_high = 0;
      ovl.Offset = ::SetFilePointer(fd.get(), 0, &offset_high, FILE_CURRENT);
      if (ovl.Offset != INVALID_SET_FILE_POINTER || ::GetLastError() == ERROR_SUCCESS) {
         ovl.OffsetHigh = static_cast< ::DWORD>(offset_high);
      } else {
         ovl.Offset = 0;
         ovl.OffsetHigh = 0;
      }
   }
   fd.bind_to_this_coroutine_scheduler_iocp();
   ::BOOL ret = ::ReadFile(fd.get(), dst, bytes_to_read, &bytes_read, &ovl);
   ::DWORD err = ret ? ERROR_SUCCESS : ::GetLastError();
   if (err == ERROR_IO_PENDING) {
      this_coroutine::sleep_until_fd_ready(fd.get(), false /*read*/, 0 /*TODO: timeout*/, &ovl);
      err = ovl.status();
      bytes_read = ovl.transferred_size();
   }
   this_coroutine::interruption_point();
   return check_if_eof_or_throw_os_error(bytes_read, err) ? 0 : bytes_read;
#else //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32 … else
}

#if LOFTY_HOST_API_WIN32
/*virtual*/ bool file_istream::check_if_eof_or_throw_os_error(::DWORD bytes_read, ::DWORD err) const {
   switch (err) {
      case ERROR_SUCCESS:
         return bytes_read == 0;
      case ERROR_HANDLE_EOF:
         return true;
      default:
         exception::throw_os_error(err);
   }
}
#endif

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

file_ostream::file_ostream(_pvt::file_init_data * init_data) :
   file_stream(init_data) {
}

/*virtual*/ file_ostream::~file_ostream() {
   /* Verify that fd is no longer open. If that’s not the case, the caller neglected to verify that the OS
   write buffer was flushed successfully. */
   if (fd) {
      LOFTY_LOG(
         err, LOFTY_SL("instance of {} @ {} being destructed before close() was invoked on it\n"),
         typeid(*this), this
      );
      fd.close();
   }
}

/*virtual*/ void file_ostream::close() /*override*/ {
   flush();
   fd.close();
}

/*virtual*/ void file_ostream::flush() /*override*/ {
#if LOFTY_HOST_API_POSIX
   // TODO: investigate fdatasync().
   // This may repeat in case of EINTR.
   while (::fsync(fd.get()) < 0) {
      int err = errno;
      if (err == EINTR) {
         this_coroutine::interruption_point();
      } else if (
   #if LOFTY_HOST_API_DARWIN
         err == ENOTSUP
   #else
         err == EINVAL
   #endif
      ) {
         // fd.get() does not support fsync(3); ignore the error.
         break;
      } else {
         exception::throw_os_error();
      }
   }
#elif LOFTY_HOST_API_WIN32
   if (!::FlushFileBuffers(fd.get())) {
      ::DWORD err = ::GetLastError();
      if (err != ERROR_INVALID_FUNCTION) {
         exception::throw_os_error(err);
      }
      // fd.get() does not support FlushFileBuffers(); ignore the error.
   }
#else
   #error "TODO: HOST_API"
#endif
   this_coroutine::interruption_point();
}

/*virtual*/ std::size_t file_ostream::write_bytes(void const * src, std::size_t src_size) /*override*/ {
   std::int8_t const * src_bytes = static_cast<std::int8_t const *>(src);
#if LOFTY_HOST_API_POSIX
   // This may repeat in case of EINTR or in case ::write() couldn’t write all the bytes.
   for (;;) {
      std::size_t bytes_to_write = _std::min<std::size_t>(src_size, numeric::max< ::ssize_t>::value);
      ::ssize_t bytes_written = ::write(fd.get(), src_bytes, bytes_to_write);
      if (bytes_written >= 0) {
         src_bytes += bytes_written;
         src_size -= static_cast<std::size_t>(bytes_written);
         if (src_size == 0) {
            break;
         }
      } else {
         int err = errno;
         switch (err) {
            case EINTR:
               this_coroutine::interruption_point();
               break;
            case EAGAIN:
   #if EWOULDBLOCK != EAGAIN
            case EWOULDBLOCK:
   #endif
               this_coroutine::sleep_until_fd_ready(fd.get(), true /*write*/, 0 /*TODO: timeout*/);
               break;
            default:
               exception::throw_os_error(err);
         }
      }
   }
   this_coroutine::interruption_point();
#elif LOFTY_HOST_API_WIN32 //if LOFTY_HOST_API_POSIX
   do {
      ::DWORD bytes_written, bytes_to_write = static_cast< ::DWORD>(
         _std::min<std::size_t>(src_size, numeric::max< ::DWORD>::value)
      );
      overlapped ovl;
      {
         // Obtain the current file offset and set ovl to start there.
         long offset_high = 0;
         ovl.Offset = ::SetFilePointer(fd.get(), 0, &offset_high, FILE_CURRENT);
         if (ovl.Offset != INVALID_SET_FILE_POINTER || ::GetLastError() == ERROR_SUCCESS) {
            ovl.OffsetHigh = static_cast< ::DWORD>(offset_high);
         } else {
            ovl.Offset = 0;
            ovl.OffsetHigh = 0;
         }
      }
      fd.bind_to_this_coroutine_scheduler_iocp();
      if (!::WriteFile(fd.get(), src_bytes, bytes_to_write, &bytes_written, &ovl)) {
         auto err = ::GetLastError();
         if (err == ERROR_IO_PENDING) {
            this_coroutine::sleep_until_fd_ready(fd.get(), true /*write*/, 0 /*TODO: timeout*/, &ovl);
         }
         err = ovl.status();
         if (err != ERROR_SUCCESS) {
            exception::throw_os_error(err);
         }
         bytes_written = ovl.transferred_size();
      }
      this_coroutine::interruption_point();
      src_bytes += bytes_written;
      src_size -= bytes_written;
   } while (src_size);
#else //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32 … else
   return static_cast<std::size_t>(src_bytes - static_cast<std::int8_t const *>(src));
}

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

file_iostream::file_iostream(_pvt::file_init_data * init_data) :
   file_stream(init_data),
   file_istream(init_data),
   file_ostream(init_data) {
}

/*virtual*/ file_iostream::~file_iostream() {
}

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

pipe::pipe() {
   bool async = (this_thread::coroutine_scheduler() != nullptr);
   _pvt::file_init_data read_end_init_data, write_end_init_data;
#if LOFTY_HOST_API_DARWIN
   int fds[2];
   // pipe2() is not available, so emulate it with pipe() + fcntl().
   while (::pipe(fds)) {
      int err = errno;
      if (err != EINTR) {
         exception::throw_os_error(err);
      }
      this_coroutine::interruption_point();
   }
   // Set the .fd members immediately, so they’ll get closed automatically in case of exceptions.
   read_end_init_data.fd = filedesc(fds[0]);
   write_end_init_data.fd = filedesc(fds[1]);
   /* Note that at this point there’s no hack that will ensure a fork()/exec() from another thread won’t leak
   the two file descriptors. That’s the whole point of pipe2(). */
   read_end_init_data.fd.share_with_subprocesses(false);
   write_end_init_data.fd.share_with_subprocesses(false);
   if (async) {
      read_end_init_data.fd.set_nonblocking(true);
      write_end_init_data.fd.set_nonblocking(true);
   }
#elif LOFTY_HOST_API_LINUX || LOFTY_HOST_API_FREEBSD
   int fds[2], flags = O_CLOEXEC;
   if (async) {
      flags |= O_NONBLOCK;
   }
   while (::pipe2(fds, flags)) {
      int err = errno;
      if (err != EINTR) {
         exception::throw_os_error(err);
      }
      this_coroutine::interruption_point();
   }
   read_end_init_data.fd = filedesc(fds[0]);
   write_end_init_data.fd = filedesc(fds[1]);
#elif LOFTY_HOST_API_WIN32
   if (async) {
      // Win32 anonymous pipes don’t support asynchronous I/O, so create a named pipe instead.
      static long serial = 0;
      lofty::text::sstr<64> pipe_name;
      pipe_name.format(
         LOFTY_SL("\\\\.\\pipe\\lofty::io::binary::pipe\\{}\\{}"),
         ::GetCurrentProcessId(), ::InterlockedIncrement(&serial)
      );
      /* Pipe buffers are allocated in the kernel’s non-paged memory pool, so this value should be small; the
      smallest it can get is a single memory page. */
      ::DWORD buffer_size = static_cast< ::DWORD>(memory::page_size());
      // 0 means default connection timeout; irrelevant as we’ll connect the other end immediately.
      ::HANDLE read_end_handle = ::CreateNamedPipe(
         pipe_name.c_str(), GENERIC_READ | PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE,
         1 /*instances*/, buffer_size, buffer_size, 0 /*default timeout*/, nullptr
      );
      if (read_end_handle == INVALID_HANDLE_VALUE) {
         exception::throw_os_error();
      }
      read_end_init_data.fd = filedesc(read_end_handle);
      ::HANDLE write_end_handle = ::CreateFile(
         pipe_name.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, nullptr
      );
      if (write_end_handle == INVALID_HANDLE_VALUE) {
         // read_end_init_data.fd is closed automatically.
         exception::throw_os_error();
      }
      write_end_init_data.fd = filedesc(write_end_handle);
   } else {
      ::HANDLE read_end_handle, write_end_handle;
      if (!::CreatePipe(&read_end_handle, &write_end_handle, nullptr, 0)) {
         exception::throw_os_error();
      }
      read_end_init_data.fd = filedesc(read_end_handle);
      write_end_init_data.fd = filedesc(write_end_handle);
   }
#else
   #error "TODO: HOST_API"
#endif
   this_coroutine::interruption_point();
   read_end_init_data.mode = access_mode::read;
   write_end_init_data.mode = access_mode::write;
   read_end_init_data.bypass_cache = false;
   write_end_init_data.bypass_cache = false;
   read_end  = _std::make_shared<pipe_istream>(&read_end_init_data);
   write_end = _std::make_shared<pipe_ostream>(&write_end_init_data);
}

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

buffer::buffer(buffer && src) :
   ptr(_std::move(src.ptr)),
   size_(src.size_),
   used_offset_(src.used_offset_),
   available_offset_(src.available_offset_) {
   src.size_ = 0;
   src.used_offset_ = 0;
   src.available_offset_ = 0;
}

/*explicit*/ buffer::buffer(std::size_t size__) :
   ptr(memory::alloc_bytes_unique(size__)),
   size_(size__),
   used_offset_(0),
   available_offset_(0) {
}

buffer::~buffer() {
}

buffer & buffer::operator=(buffer && src) {
   ptr = _std::move(src.ptr);
   size_ = src.size_;
   src.size_ = 0;
   used_offset_ = src.used_offset_;
   src.used_offset_ = 0;
   available_offset_ = src.available_offset_;
   src.available_offset_ = 0;
   return *this;
}

void buffer::expand_to(std::size_t new_size) {
   memory::realloc_unique(&ptr, new_size);
   size_ = new_size;
}

void buffer::make_unused_available() {
   memory::move(static_cast<std::int8_t *>(ptr.get()), get_used(), used_size());
   available_offset_ -= used_offset_;
   used_offset_ = 0;
}

void buffer::shrink_to_fit() {
   if (used_offset_ > 0) {
      make_unused_available();
   }
   memory::realloc_unique(&ptr, available_offset_);
   size_ = available_offset_;
}

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

memory_stream::memory_stream() {
}

memory_stream::memory_stream(memory_stream && src) :
   buf(_std::move(src.buf)) {
}

/*explicit*/ memory_stream::memory_stream(buffer && buf_) :
   buf(_std::move(buf_)) {
}

/*virtual*/ memory_stream::~memory_stream() {
}

/*virtual*/ void memory_stream::commit_bytes(std::size_t count) /*override*/ {
   if (count > buf.available_size()) {
      // Can’t commit more bytes than are available in the write buffer.
      // TODO: use a better exception class.
      LOFTY_THROW(argument_error, ());
   }
   // Increase the count of used bytes in the buffer.
   buf.mark_as_used(count);
}

/*virtual*/ void memory_stream::consume_bytes(std::size_t count) /*override*/ {
   if (count > buf.used_size()) {
      // Can’t consume more bytes than are used in the read buffer.
      // TODO: use a better exception class.
      LOFTY_THROW(argument_error, ());
   }
   // Shift the “used window” of the read buffer by count bytes.
   buf.mark_as_unused(count);
}

/*virtual*/ void memory_stream::close() /*override*/ {
   // Nothing to do.
}

/*virtual*/ void memory_stream::flush() /*override*/ {
   // An in-memory stream doesn’t need flushing.
}

/*virtual*/ buffer_range<void> memory_stream::get_buffer_bytes(std::size_t count) /*override*/ {
   // If the requested size is more than what can fit in the buffer, compact it, or enlarge it.
   if (count > buf.available_size()) {
      // See if compacting the buffer creates enough room. If that’s not enough, enlarge the buffer.
      buf.make_unused_available();
      if (count > buf.available_size()) {
         std::size_t buf_size = bitmanip::ceiling_to_pow2_multiple(count, buf_default_size);
         buf.expand_to(buf_size);
      }
   }
   // Return the available portion of the buffer.
   return buffer_range<void>(buf.get_available(), buf.available_size());
}

/*virtual*/ buffer_range<void const> memory_stream::peek_bytes(std::size_t count) /*override*/ {
   // Ignore count; we’ll always return the entire used portion of the buffer.
   LOFTY_UNUSED_ARG(count);
   // Return the “used window” of the buffer.
   return buffer_range<void const>(buf.get_used(), buf.used_size());
}

void memory_stream::rewind() {
   buf.mark_unused_as_used();
}

/*virtual*/ offset_t memory_stream::seek(offset_t offset, seek_from whence) /*override*/ {
   // This implementation moves buf.used_offset() within 0 to buf.available_offset().
   switch (whence.base()) {
      case seek_from::start:
         break;
      case seek_from::current:
         offset += static_cast<offset_t>(buf.used_offset());
         break;
      case seek_from::end:
         offset += static_cast<offset_t>(buf.available_offset());
         break;
   }
   if (offset < 0 || offset > static_cast<offset_t>(buf.available_offset())) {
      LOFTY_THROW(io::error, ());
   }
   buf.mark_as_unused(static_cast<std::size_t>(
      -static_cast<std::ptrdiff_t>(buf.unused_size()) + static_cast<std::ptrdiff_t>(offset)
   ));
   return static_cast<offset_t>(buf.used_offset());
}

/*virtual*/ full_size_t memory_stream::size() const /*override*/ {
   return buf.available_offset();
}

/*virtual*/ offset_t memory_stream::tell() const /*override*/ {
   return static_cast<offset_t>(buf.used_offset());
}

/*virtual*/ _std::shared_ptr<stream> memory_stream::_unbuffered_stream() const /*override*/ {
   // Removing constness is bad, but no alternatives ara viable (return const, or make method non-const).
   return _std::dynamic_pointer_cast<stream>(_std::const_pointer_cast<memory_stream>(shared_from_this()));
}

}}} //namespace lofty::io::binary
