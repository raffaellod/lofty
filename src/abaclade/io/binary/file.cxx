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

#include <abaclade.hxx>
#include <abaclade/io/binary/file.hxx>

#include <algorithm>
#if ABC_TARGET_API_POSIX
   #include <errno.h> // errno
   #include <fcntl.h> // O_* fcntl()
   #include <sys/stat.h> // S_* stat()
   #include <unistd.h> // *_FILENO ssize_t close() isatty() open() read() write()
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary globals

namespace abc {
namespace io {
namespace binary {

namespace detail {

struct file_init_data {
#if ABC_TARGET_API_POSIX
   //! Set by _construct().
   struct ::stat statFile;
#endif
   //! See file_base::m_fd. To be set before calling _construct().
   filedesc fd;
   /*! Determines what type of I/O object will be instantiated. To be set before calling
   _construct(). */
   access_mode am;
   //! See file_base::m_bBuffered. To be set before calling _construct().
   bool bBuffered:1;
};

} //namespace detail

namespace {

std::shared_ptr<file_writer> g_pbfwStdErr;
std::shared_ptr<file_reader> g_pbfrStdIn;
std::shared_ptr<file_writer> g_pbfwStdOut;

/*! Instantiates a binary::base specialization appropriate for the descriptor in *pfid, returning a
shared pointer to it.

pfid
   Data that will be passed to the constructor of the file object.
return
   Shared pointer to the newly created object.
*/
std::shared_ptr<file_base> _construct(detail::file_init_data * pfid) {
   ABC_TRACE_FUNC(pfid);

#if ABC_TARGET_API_POSIX
   if (::fstat(pfid->fd.get(), &pfid->statFile)) {
      throw_os_error();
   }
   if (S_ISREG(pfid->statFile.st_mode)) {
      switch (pfid->am.base()) {
         case access_mode::read:
            return std::make_shared<regular_file_reader>(pfid);
         case access_mode::write:
         case access_mode::append:
            return std::make_shared<regular_file_writer>(pfid);
         case access_mode::read_write:
         // default is here just to silence compiler warnings.
         default:
            // TODO: regular_file_random
            break;
      }
   }
   if (S_ISCHR(pfid->statFile.st_mode) && ::isatty(pfid->fd.get())) {
      switch (pfid->am.base()) {
         case access_mode::read:
            return std::make_shared<console_reader>(pfid);
         case access_mode::write:
            return std::make_shared<console_writer>(pfid);
         case access_mode::append:
         case access_mode::read_write:
         // default is here just to silence compiler warnings.
         default:
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
         case access_mode::append:
         case access_mode::read_write:
         // default is here just to silence compiler warnings.
         default:
            // TODO: use a better exception class.
            ABC_THROW(argument_error, ());
      }
   }
#elif ABC_TARGET_API_WIN32 //if ABC_TARGET_API_POSIX
   switch (::GetFileType(pfid->fd.get())) {
      case FILE_TYPE_CHAR: {
         /* Serial line or console.

         Using ::GetConsoleMode() to detect a console handle requires GENERIC_READ access rights,
         which could be a problem with stdout/stderr because we don’t ask for that permission for
         these handles; however, for consoles, “The handles returned by CreateFile,
         CreateConsoleScreenBuffer, and GetStdHandle have the GENERIC_READ and GENERIC_WRITE access
         rights”, so we can trust this to succeed for console handles. */

         DWORD iConsoleMode;
         if (::GetConsoleMode(pfid->fd.get(), &iConsoleMode)) {
            switch (pfid->am.base()) {
               case access_mode::read:
                  return std::make_shared<console_reader>(pfid);
               case access_mode::write:
                  return std::make_shared<console_writer>(pfid);
               case access_mode::append:
               case access_mode::read_write:
               // default is here just to silence compiler warnings.
               default:
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
            case access_mode::append:
               return std::make_shared<regular_file_writer>(pfid);
            case access_mode::read_write:
            // default is here just to silence compiler warnings.
            default:
               // TODO: regular_file_random
               break;
         }
         break;

      case FILE_TYPE_PIPE:
         // Socket or pipe.
         switch (pfid->am.base()) {
            case access_mode::read:
               return std::make_shared<pipe_reader>(pfid);
            case access_mode::write:
               return std::make_shared<pipe_writer>(pfid);
            case access_mode::append:
            case access_mode::read_write:
            // default is here just to silence compiler warnings.
            default:
               // TODO: use a better exception class.
               ABC_THROW(argument_error, ());
         }
         break;

      case FILE_TYPE_UNKNOWN: {
         // Unknown or error.
         DWORD iErr = ::GetLastError();
         if (iErr != ERROR_SUCCESS) {
            throw_os_error(iErr);
         }
         break;
      }
   }
#else //if ABC_TARGET_API_POSIX … elif ABC_TARGET_API_WIN32
   #error "TODO: TARGET_API"
#endif //if ABC_TARGET_API_POSIX … elif ABC_TARGET_API_WIN32 … else

   // If a file object was not returned in the code above, return a generic file.
   switch (pfid->am.base()) {
      case access_mode::read:
         return std::make_shared<file_reader>(pfid);
      case access_mode::write:
         return std::make_shared<file_writer>(pfid);
      case access_mode::append:
      case access_mode::read_write:
      // default is here just to silence compiler warnings.
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
std::shared_ptr<file_base> _attach(filedesc && fd, access_mode am) {
   ABC_TRACE_FUNC(/*fd*/);

   detail::file_init_data fid;
   fid.fd = std::move(fd);
   fid.am = am;
   // Since this method is supposed to be used only for standard descriptors, assume that OS
   // buffering is on.
   fid.bBuffered = true;
   return _construct(&fid);
}

} //namespace


std::shared_ptr<file_writer> stderr() {
   ABC_TRACE_FUNC();

   /* TODO: under Win32, GUI subsystem programs will get nullptr when calling ::GetStdHandle(). To
   avoid exceptions later when performing I/O on it, we need to ::SetStdHandle() with a file opened
   on “NUL”. This mimics the behavior of Linux GUI programs, where all their standard I/O handles
   are open on /dev/null. */

   // TODO: mutex!
   if (!g_pbfwStdErr) {
      g_pbfwStdErr = std::dynamic_pointer_cast<file_writer>(_attach(filedesc(
#if ABC_TARGET_API_POSIX
         STDERR_FILENO,
#elif ABC_TARGET_API_WIN32
         ::GetStdHandle(STD_ERROR_HANDLE),
#else
   #error "TODO: TARGET_API"
#endif
         false
      ), access_mode::write));
   }
   return g_pbfwStdErr;
}

std::shared_ptr<file_reader> stdin() {
   ABC_TRACE_FUNC();

   /* TODO: under Win32, GUI subsystem programs will get nullptr when calling ::GetStdHandle(). To
   avoid exceptions later when performing I/O on it, we need to ::SetStdHandle() with a file opened
   on “NUL”. This mimics the behavior of Linux GUI programs, where all their standard I/O handles
   are open on /dev/null. */

   // TODO: mutex!
   if (!g_pbfrStdIn) {
      g_pbfrStdIn = std::dynamic_pointer_cast<file_reader>(_attach(filedesc(
#if ABC_TARGET_API_POSIX
         STDIN_FILENO,
#elif ABC_TARGET_API_WIN32
         ::GetStdHandle(STD_INPUT_HANDLE),
#else
   #error "TODO: TARGET_API"
#endif
         false
      ), access_mode::read));
   }
   return g_pbfrStdIn;
}

std::shared_ptr<file_writer> stdout() {
   ABC_TRACE_FUNC();

   /* TODO: under Win32, GUI subsystem programs will get nullptr when calling ::GetStdHandle(). To
   avoid exceptions later when performing I/O on it, we need to ::SetStdHandle() with a file opened
   on “NUL”. This mimics the behavior of Linux GUI programs, where all their standard I/O handles
   are open on /dev/null. */

   // TODO: mutex!
   if (!g_pbfwStdOut) {
      g_pbfwStdOut = std::dynamic_pointer_cast<file_writer>(_attach(filedesc(
#if ABC_TARGET_API_POSIX
         STDOUT_FILENO,
#elif ABC_TARGET_API_WIN32
         ::GetStdHandle(STD_OUTPUT_HANDLE),
#else
   #error "TODO: TARGET_API"
#endif
         false
      ), access_mode::write));
   }
   return g_pbfwStdOut;
}

std::shared_ptr<file_base> open(os::path const & op, access_mode am, bool bBuffered /*= true*/) {
   ABC_TRACE_FUNC(op, am, bBuffered);

   detail::file_init_data fid;
#if ABC_TARGET_API_POSIX
   int fi;
   switch (am.base()) {
      default:
      case access_mode::read:
         fi = O_RDONLY;
         break;
      case access_mode::write:
         fi = O_WRONLY | O_CREAT | O_TRUNC;
         break;
      case access_mode::read_write:
         fi = O_RDWR | O_CREAT;
         break;
      case access_mode::append:
         fi = O_APPEND;
         break;
   }
#ifdef O_DIRECT
   if (!bBuffered) {
      fi |= O_DIRECT;
   }
#endif
   fid.fd = ::open(op.os_str().c_str(), fi, 0666);
   if (!fid.fd) {
      switch (errno) {
         case ENODEV: // No such device (POSIX.1-2001)
         case ENOENT: // No such file or directory (POSIX.1-2001)
            ABC_THROW(file_not_found_error, (op, errno));
         default:
            throw_os_error(errno);
      }
   }
#ifndef O_DIRECT
#if ABC_TARGET_API_DARWIN
   if (!bBuffered) {
      if (::fcntl(fid.fd.get(), F_NOCACHE, 1) == -1) {
         throw_os_error();
      }
   }
#else
   #error "TODO: TARGET_API"
#endif
#endif //ifndef O_DIRECT

#elif ABC_TARGET_API_WIN32 //if ABC_TARGET_API_POSIX
   DWORD fiAccess, fiShareMode, iAction, fi = FILE_ATTRIBUTE_NORMAL;
   switch (am.base()) {
      default:
      case access_mode::read:
         fiAccess = GENERIC_READ;
         fiShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
         iAction = OPEN_EXISTING;
         break;
      case access_mode::write:
         fiAccess = GENERIC_WRITE;
         fiShareMode = FILE_SHARE_READ;
         iAction = CREATE_ALWAYS;
         break;
      case access_mode::read_write:
         fiAccess = GENERIC_READ | GENERIC_WRITE;
         fiShareMode = FILE_SHARE_READ;
         iAction = OPEN_ALWAYS;
         break;
      case access_mode::append:
         /* This combination is FILE_GENERIC_WRITE & ~FILE_WRITE_DATA; MSDN states that “for local
         files, write operations will not overwrite existing data”. Requiring fewer permissions,
         this also allows ::CreateFile() to succeed on files with stricter ACLs. */

         fiAccess = FILE_APPEND_DATA | FILE_WRITE_ATTRIBUTES | STANDARD_RIGHTS_WRITE | SYNCHRONIZE;
         fiShareMode = FILE_SHARE_READ;
         iAction = OPEN_ALWAYS;
         break;
   }
   if (!bBuffered) {
      fi |= FILE_FLAG_NO_BUFFERING;
   } else if (fiAccess & GENERIC_READ) {
      fi |= FILE_FLAG_SEQUENTIAL_SCAN;
   }
   fid.fd = ::CreateFile(op.os_str().c_str(), fiAccess, fiShareMode, nullptr, iAction, fi, nullptr);
   if (!fid.fd) {
      DWORD iErr = ::GetLastError();
      switch (iErr) {
         case ERROR_PATH_NOT_FOUND: // The system cannot find the path specified.
         case ERROR_UNKNOWN_PORT: // The specified port is unknown.
            ABC_THROW(file_not_found_error, (op, iErr));
         default:
            throw_os_error(iErr);
      }
   }
#else //if ABC_TARGET_API_POSIX … elif ABC_TARGET_API_WIN32
   #error "TODO: TARGET_API"
#endif //if ABC_TARGET_API_POSIX … elif ABC_TARGET_API_WIN32 … else
   fid.am = am;
   fid.bBuffered = bBuffered;
   return _construct(&fid);
}

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::filedesc

namespace abc {
namespace io {

filedesc_t const filedesc::smc_fdNull =
#if ABC_TARGET_API_POSIX
   -1;
#elif ABC_TARGET_API_WIN32
   INVALID_HANDLE_VALUE;
#else
   #error "TODO: TARGET_API"
#endif

filedesc::filedesc(filedesc && fd) :
   m_fd(fd.m_fd),
   m_bOwn(fd.m_bOwn) {
   ABC_TRACE_FUNC(this);

   fd.m_fd = smc_fdNull;
   fd.m_bOwn = false;
}

filedesc::~filedesc() {
   ABC_TRACE_FUNC(this);

   if (m_bOwn && m_fd != smc_fdNull) {
#if ABC_TARGET_API_POSIX
      ::close(m_fd);
#elif ABC_TARGET_API_WIN32
      ::CloseHandle(m_fd);
#else
   #error "TODO: TARGET_API"
#endif
   }
}

filedesc & filedesc::operator=(filedesc_t fd) {
   ABC_TRACE_FUNC(this, fd);

   if (fd != m_fd) {
      this->~filedesc();
   }
   m_fd = fd;
   m_bOwn = true;
   return *this;
}
filedesc & filedesc::operator=(filedesc && fd) {
   ABC_TRACE_FUNC(this);

   if (fd.m_fd != m_fd) {
      this->~filedesc();
      m_fd = fd.m_fd;
      m_bOwn = fd.m_bOwn;
      fd.m_fd = smc_fdNull;
   }
   return *this;
}

} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::file_base

namespace abc {
namespace io {
namespace binary {

file_base::file_base(detail::file_init_data * pfid) :
   m_fd(std::move(pfid->fd)) {
}

/*virtual*/ file_base::~file_base() {
}

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::file_reader

namespace abc {
namespace io {
namespace binary {

file_reader::file_reader(detail::file_init_data * pfid) :
   file_base(pfid) {
}

/*virtual*/ file_reader::~file_reader() {
}

/*virtual*/ std::size_t file_reader::read(void * p, std::size_t cbMax) /*override*/ {
   ABC_TRACE_FUNC(this, p, cbMax);

   std::int8_t * pb = static_cast<std::int8_t *>(p);
   /* The top half of this loop is OS-specific; the rest is generalized. As a guideline, the OS
   read()-equivalent function is invoked at least once, so we give it a chance to report any errors,
   instead of masking them by skipping the call (e.g. due to cbMax == 0 on input). */
   do {
#if ABC_TARGET_API_POSIX
      // This will be repeated at most three times, just to break a size_t-sized block down into
      // ssize_t-sized blocks.
      ::ssize_t cbLastRead = ::read(
         m_fd.get(), pb, std::min<std::size_t>(cbMax, numeric::max< ::ssize_t>::value)
      );
      if (cbLastRead == 0) {
         // EOF.
         break;
      }
      if (cbLastRead < 0) {
         throw_os_error();
      }
#elif ABC_TARGET_API_WIN32 //if ABC_TARGET_API_POSIX
      // This will be repeated at least once, and as long as we still have some bytes to read, and
      // reading them does not fail.
      DWORD cbLastRead;
      BOOL bRet = ::ReadFile(
         m_fd.get(), pb,
         static_cast<DWORD>(std::min<std::size_t>(cbMax, numeric::max<DWORD>::value)),
         &cbLastRead, nullptr
      );
      if (readfile_returned_eof(cbLastRead, bRet ? ERROR_SUCCESS : ::GetLastError())) {
         break;
      }
#else //if ABC_TARGET_API_POSIX … elif ABC_TARGET_API_WIN32
   #error "TODO: TARGET_API"
#endif //if ABC_TARGET_API_POSIX … elif ABC_TARGET_API_WIN32 … else
      // Some bytes were read; prepare for the next attempt.
      pb += cbLastRead;
      cbMax -= static_cast<std::size_t>(cbLastRead);
   } while (cbMax);

   return static_cast<std::size_t>(pb - static_cast<std::int8_t *>(p));
}

#if ABC_TARGET_API_WIN32
/*virtual*/ bool file_reader::readfile_returned_eof(DWORD cchRead, DWORD iErr) const {
   if (iErr != ERROR_SUCCESS) {
      throw_os_error(iErr);
   }
   return cchRead == 0;
}
#endif //if ABC_TARGET_API_WIN32

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::file_writer

namespace abc {
namespace io {
namespace binary {

file_writer::file_writer(detail::file_init_data * pfid) :
   file_base(pfid) {
}

/*virtual*/ file_writer::~file_writer() {
}

/*virtual*/ void file_writer::flush() /*override*/ {
   ABC_TRACE_FUNC(this);

#if ABC_TARGET_API_POSIX
   // TODO: investigate fdatasync().
   if (::fsync(m_fd.get())) {
      throw_os_error();
   }
#elif ABC_TARGET_API_WIN32
   if (!::FlushFileBuffers(m_fd.get())) {
      throw_os_error();
   }
#else
   #error "TODO: TARGET_API"
#endif
}

/*virtual*/ std::size_t file_writer::write(void const * p, std::size_t cb) /*override*/ {
   ABC_TRACE_FUNC(this, p, cb);

   std::int8_t const * pb = static_cast<std::int8_t const *>(p);

   /* The top half of this loop is OS-specific; the rest is generalized. As a guideline, the OS
   write()-equivalent function is invoked at least once, so we give it a chance to report any
   errors, instead of masking them by skipping the call (e.g. due to cb == 0 on input). */
   do {
#if ABC_TARGET_API_POSIX
      // This will be repeated at most three times, just to break a size_t-sized block down into
      // ssize_t-sized blocks.
      ::ssize_t cbLastWritten = ::write(
         m_fd.get(), pb, std::min<std::size_t>(cb, numeric::max< ::ssize_t>::value)
      );
      if (cbLastWritten < 0) {
         throw_os_error();
      }
#elif ABC_TARGET_API_WIN32 //if ABC_TARGET_API_POSIX
      // This will be repeated at least once, and as long as we still have some bytes to write, and
      // writing them does not fail.
      DWORD cbLastWritten;
      if (!::WriteFile(
         m_fd.get(), pb, static_cast<DWORD>(std::min<std::size_t>(cb, numeric::max<DWORD>::value)),
         &cbLastWritten, nullptr
      )) {
         throw_os_error();
      }
#else //if ABC_TARGET_API_POSIX … elif ABC_TARGET_API_WIN32
   #error "TODO: TARGET_API"
#endif //if ABC_TARGET_API_POSIX … elif ABC_TARGET_API_WIN32 … else
      // Some bytes were written; prepare for the next attempt.
      pb += cbLastWritten;
      cb -= static_cast<std::size_t>(cbLastWritten);
   } while (cb);

   return static_cast<std::size_t>(pb - static_cast<std::int8_t const *>(p));
}

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::console_file_base

namespace abc {
namespace io {
namespace binary {

console_file_base::console_file_base(detail::file_init_data * pfid) :
   file_base(pfid) {
}

/*virtual*/ console_file_base::~console_file_base() {
}

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::console_reader

namespace abc {
namespace io {
namespace binary {

console_reader::console_reader(detail::file_init_data * pfid) :
   file_base(pfid),
   console_file_base(pfid),
   file_reader(pfid) {
}

/*virtual*/ console_reader::~console_reader() {
}

#if ABC_TARGET_API_WIN32
/*virtual*/ std::size_t console_reader::read(void * p, std::size_t cbMax) /*override*/ {
   ABC_TRACE_FUNC(this, p, cbMax);

   // Note: ::WriteConsole() expects character counts in place of byte counts, so everything must be
   // divided by sizeof(char_t).
   std::size_t cchMax = cbMax / sizeof(char_t);

   std::int8_t * pb = static_cast<std::int8_t *>(p);
   // ::ReadConsole() is invoked at least once, so we give it a chance to report any errors, instead
   // of masking them by skipping the call (e.g. due to cbMax == 0 on input).
   do {
      // This will be repeated at least once, and as long as we still have some bytes to read, and
      // reading them does not fail.
      DWORD cchLastRead;
      if (!::ReadConsole(
         m_fd.get(), pb,
         static_cast<DWORD>(std::min<std::size_t>(cchMax, numeric::max<DWORD>::value)),
         &cchLastRead, nullptr
      )) {
         DWORD iErr = ::GetLastError();
         if (iErr == ERROR_HANDLE_EOF) {
            break;
         }
         throw_os_error(iErr);
      }
      if (!cchLastRead) {
         break;
      }
      // Some bytes were read; prepare for the next attempt.
      pb += cchLastRead * sizeof(char_t);
      cchMax -= static_cast<std::size_t>(cchLastRead);
   } while (cchMax);

   return static_cast<std::size_t>(pb - static_cast<std::int8_t *>(p));
}
#endif //if ABC_TARGET_API_WIN32

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::console_writer

namespace abc {
namespace io {
namespace binary {

#if ABC_TARGET_API_WIN32
WORD const console_writer::smc_aiAnsiColorToForegroundColor[] = {
   /* black   */ 0,
   /* red     */ FOREGROUND_RED,
   /* green   */                  FOREGROUND_GREEN,
   /* yellow  */ FOREGROUND_RED | FOREGROUND_GREEN,
   /* blue    */                                     FOREGROUND_BLUE,
   /* magenta */ FOREGROUND_RED |                    FOREGROUND_BLUE,
   /* cyan    */                  FOREGROUND_GREEN | FOREGROUND_BLUE,
   /* white   */ FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
};
WORD const console_writer::smc_aiAnsiColorToBackgroundColor[] = {
   /* black   */ 0,
   /* red     */ BACKGROUND_RED,
   /* green   */                  BACKGROUND_GREEN,
   /* yellow  */ BACKGROUND_RED | BACKGROUND_GREEN,
   /* blue    */                                     BACKGROUND_BLUE,
   /* magenta */ BACKGROUND_RED |                    BACKGROUND_BLUE,
   /* cyan    */                  BACKGROUND_GREEN | BACKGROUND_BLUE,
   /* white   */ BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE
};
#endif

console_writer::console_writer(detail::file_init_data * pfid) :
   file_base(pfid),
   console_file_base(pfid),
   file_writer(pfid) {
#if ABC_TARGET_API_WIN32
   ABC_TRACE_FUNC(this, pfid);

   ::CONSOLE_SCREEN_BUFFER_INFO csbi;
   ::GetConsoleScreenBufferInfo(m_fd.get(), &csbi);
   for (unsigned i = 0; i < ABC_COUNTOF(smc_aiAnsiColorToForegroundColor); ++i) {
      if ((
         csbi.wAttributes & (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE
      )) == smc_aiAnsiColorToBackgroundColor[i]) {
         m_chattrDefault.clrBackground = static_cast<abc::text::ansi_terminal_color::enum_type>(i);
      }
      if ((
         csbi.wAttributes & (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
      )) == smc_aiAnsiColorToForegroundColor[i]) {
         m_chattrDefault.clrForeground = static_cast<abc::text::ansi_terminal_color::enum_type>(i);
      }
   }
   m_chattrDefault.iBlinkSpeed   = 0;
   m_chattrDefault.bConcealed    = false;
   m_chattrDefault.bCrossedOut   = false;
   m_chattrDefault.iIntensity    = (csbi.wAttributes & FOREGROUND_INTENSITY) ? 2u : 1u;
   m_chattrDefault.bItalic       = false;
   m_chattrDefault.bReverseVideo = false;
   m_chattrDefault.iUnderline    = false;

   m_chattrCurr = m_chattrDefault;
#endif
}

/*virtual*/ console_writer::~console_writer() {
}

#if ABC_TARGET_API_WIN32
/*virtual*/ void console_writer::clear_display_area(
   std::int16_t iRow, std::int16_t iCol, std::size_t cch
) /*override*/ {
   ABC_TRACE_FUNC(this, iRow, iCol, cch);

   // TODO: implementation.
}

/*virtual*/ void console_writer::get_cursor_pos_and_display_size(
   std::int16_t * piRow, std::int16_t * piCol, std::int16_t * pcRows, std::int16_t * pcCols
) /*override*/ {
   ABC_TRACE_FUNC(this, piRow, piCol, pcRows, pcCols);

   ::CONSOLE_SCREEN_BUFFER_INFO csbi;
   ::GetConsoleScreenBufferInfo(m_fd.get(), &csbi);
   *piRow = csbi.dwCursorPosition.Y;
   *piCol = csbi.dwCursorPosition.X;
   *pcRows = csbi.dwSize.Y;
   *pcCols = csbi.dwSize.X;
}

bool console_writer::processing_enabled() const {
   ABC_TRACE_FUNC(this);

   DWORD iConsoleMode;
   if (!::GetConsoleMode(m_fd.get(), &iConsoleMode)) {
      // TODO: is this worth throwing an exception for?
      return false;
   }
   return (iConsoleMode & ENABLE_PROCESSED_OUTPUT) != 0;
}

/*virtual*/ void console_writer::scroll_text(std::int16_t cRows, std::int16_t cCols) /*override*/ {
   ABC_TRACE_FUNC(this, cRows, cCols);

   // TODO: implementation.
}

/*virtual*/ void console_writer::set_char_attributes() /*override*/ {
   ABC_TRACE_FUNC(this);

   WORD iAttr;
   if (m_chattrCurr.bConcealed) {
      if (m_chattrCurr.bReverseVideo) {
         iAttr  = smc_aiAnsiColorToBackgroundColor[m_chattrCurr.clrForeground];
         iAttr |= smc_aiAnsiColorToForegroundColor[m_chattrCurr.clrForeground];
         if (m_chattrCurr.iIntensity == 2) {
            // Turn on background intensity as well, to match foreground intensity.
            iAttr |= FOREGROUND_INTENSITY | BACKGROUND_INTENSITY;
         }
      } else {
         iAttr  = smc_aiAnsiColorToBackgroundColor[m_chattrCurr.clrBackground];
         iAttr |= smc_aiAnsiColorToForegroundColor[m_chattrCurr.clrBackground];
      }
   } else {
      if (m_chattrCurr.bReverseVideo) {
         iAttr  = smc_aiAnsiColorToBackgroundColor[m_chattrCurr.clrForeground];
         iAttr |= smc_aiAnsiColorToForegroundColor[m_chattrCurr.clrBackground];
      } else {
         iAttr  = smc_aiAnsiColorToBackgroundColor[m_chattrCurr.clrBackground];
         iAttr |= smc_aiAnsiColorToForegroundColor[m_chattrCurr.clrForeground];
      }
      if (m_chattrCurr.iIntensity == 2) {
         iAttr |= FOREGROUND_INTENSITY;
      }
   }
   ::SetConsoleTextAttribute(m_fd.get(), iAttr);
}

/*virtual*/ void console_writer::set_cursor_pos(std::int16_t iRow, std::int16_t iCol) /*override*/ {
   ABC_TRACE_FUNC(this, iRow, iCol);

   ::COORD coord;
   coord.X = iCol;
   coord.Y = iRow;
   ::SetConsoleCursorPosition(m_fd.get(), coord);
}

/*virtual*/ void console_writer::set_cursor_visibility(bool bVisible) /*override*/ {
   ABC_TRACE_FUNC(this, bVisible);

   ::CONSOLE_CURSOR_INFO cci;
   ::GetConsoleCursorInfo(m_fd.get(), &cci);
   cci.bVisible = bVisible;
   ::SetConsoleCursorInfo(m_fd.get(), &cci);
}

/*virtual*/ void console_writer::set_window_title(istr const & sTitle) /*override*/ {
   ABC_TRACE_FUNC(this, sTitle);

   ::SetConsoleTitle(sTitle.c_str());
}

/*virtual*/ std::size_t console_writer::write(void const * p, std::size_t cb) /*override*/ {
   ABC_TRACE_FUNC(this, p, cb);

   char_t const * pchBegin = static_cast<char_t const *>(p);
   char_t const * pchEnd = reinterpret_cast<char_t const *>(
      static_cast<std::int8_t const *>(p) + cb
   );
   char_t const * pchLastWritten = pchBegin;
   if (processing_enabled()) {
      for (char_t const * pch = pchBegin; pch < pchEnd; ) {
         char_t ch = *pch;
         if (abc::text::host_char_traits::is_lead_surrogate(ch)) {
            /* ::WriteConsole() is unable to handle UTF-16 surrogates, so write a replacement
            character in place of the surrogate pair. */
            if (pchLastWritten < pch) {
               write_range(pchLastWritten, pch);
            }
            ++pch;
            // If a trail surrogate follows, consume it immediately.
            if (pch < pchEnd && abc::text::host_char_traits::is_trail_char(*pch)) {
               ++pch;
            }
            pchLastWritten = pch;
            // Write the replacement character.
            ch = abc::text::replacement_char;
            write_range(&ch, &ch + 1);
         } else if (consume_char(ch)) {
            // ch is part of an ANSI escape sequence.
            if (pchLastWritten < pch) {
               write_range(pchLastWritten, pch);
            }
            pchLastWritten = ++pch;
         } else {
            ++pch;
         }
      }
   }
   if (pchLastWritten < pchEnd) {
      write_range(pchLastWritten, pchEnd);
   }
   return cb;
}

void console_writer::write_range(char_t const * pchBegin, char_t const * pchEnd) const {
   ABC_TRACE_FUNC(this, pchBegin, pchEnd);

   // This loop may repeat more than once in the unlikely case cch exceeds what can fit in a DWORD.
   while (std::size_t cch = static_cast<std::size_t>(pchEnd - pchBegin)) {
      DWORD cchLastWritten;
      if (!::WriteConsole(
         m_fd.get(), pchBegin, static_cast<DWORD>(
            std::min<std::size_t>(cch, numeric::max<DWORD>::value)
         ), &cchLastWritten, nullptr
      )) {
         throw_os_error();
      }
      // Some bytes were written; prepare for the next attempt.
      pchBegin += cchLastWritten;
   }
}
#endif //if ABC_TARGET_API_WIN32

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::pipe_reader

namespace abc {
namespace io {
namespace binary {

pipe_reader::pipe_reader(detail::file_init_data * pfid) :
   file_base(pfid),
   file_reader(pfid) {
}

/*virtual*/ pipe_reader::~pipe_reader() {
}

#if ABC_TARGET_API_WIN32

/*virtual*/ bool pipe_reader::readfile_returned_eof(DWORD cchRead, DWORD iErr) const /*override*/ {
   ABC_UNUSED_ARG(cchRead);
   switch (iErr) {
      case ERROR_SUCCESS:
         return false;
      case ERROR_BROKEN_PIPE:
         return true;
      default:
         throw_os_error(iErr);
   }
}

#endif //if ABC_TARGET_API_WIN32

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::pipe_writer

namespace abc {
namespace io {
namespace binary {

pipe_writer::pipe_writer(detail::file_init_data * pfid) :
   file_base(pfid),
   file_writer(pfid) {
}

/*virtual*/ pipe_writer::~pipe_writer() {
}

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::regular_file_base

namespace abc {
namespace io {
namespace binary {

regular_file_base::regular_file_base(detail::file_init_data * pfid) :
   file_base(pfid) {
   ABC_TRACE_FUNC(this, pfid);

#if ABC_TARGET_API_POSIX

   m_cb = static_cast<std::size_t>(pfid->statFile.st_size);
#if 0
   if (!m_bBuffered) {
      // For unbuffered access, use the filesystem-suggested I/O size increment.
      m_cbPhysAlign = static_cast<unsigned>(pfid->statFile.st_blksize);
   }
#endif

#elif ABC_TARGET_API_WIN32 //if ABC_TARGET_API_POSIX

#if _WIN32_WINNT >= 0x0500
   static_assert(
      sizeof(m_cb) == sizeof(LARGE_INTEGER),
      "abc::io::full_size_t must be the same size as LARGE_INTEGER"
   );
   if (!::GetFileSizeEx(m_fd.get(), reinterpret_cast<LARGE_INTEGER *>(&m_cb))) {
      throw_os_error();
   }
#else //if _WIN32_WINNT >= 0x0500
   DWORD cbHigh, cbLow = ::GetFileSize(fd.get(), &cbHigh);
   if (cbLow == INVALID_FILE_SIZE) {
      DWORD iErr = ::GetLastError();
      if (iErr != ERROR_SUCCESS) {
         throw_os_error(iErr);
      }
   }
   m_cb = (static_cast<full_size_t>(cbHigh) << sizeof(cbLow) * CHAR_BIT) | cbLow;
#endif //if _WIN32_WINNT >= 0x0500 … else
#if 0
   if (!m_bBuffered) {
      /* Should really use ::DeviceIoCtl(IOCTL_STORAGE_QUERY_PROPERTY) on the disk containing this
      file. For now, use 4 KiB alignment, since that’s the most recent commonly used physical sector
      size. */
      m_cbPhysAlign = 4096;
   }
#endif

#else //if ABC_TARGET_API_POSIX … elif ABC_TARGET_API_WIN32
   #error "TODO: TARGET_API"
#endif //if ABC_TARGET_API_POSIX … elif ABC_TARGET_API_WIN32 … else
}

/*virtual*/ regular_file_base::~regular_file_base() {
}

/*virtual*/ offset_t regular_file_base::seek(offset_t ibOffset, seek_from sfWhence) /*override*/ {
   ABC_TRACE_FUNC(this, ibOffset, sfWhence);

#if ABC_TARGET_API_POSIX

   int iWhence;
   switch (sfWhence.base()) {
      // default is here just to silence compiler warnings.
      default:
      case seek_from::start:
         iWhence = SEEK_SET;
         break;
      case seek_from::current:
         iWhence = SEEK_CUR;
         break;
      case seek_from::end:
         iWhence = SEEK_END;
         break;
   }
   offset_t ibNewOffset = ::lseek(m_fd.get(), ibOffset, iWhence);
   if (ibNewOffset == -1) {
      throw_os_error();
   }
   return ibNewOffset;

#elif ABC_TARGET_API_WIN32 //if ABC_TARGET_API_POSIX

   static_assert(
      sizeof(ibOffset) == sizeof(LARGE_INTEGER),
      "abc::io::offset_t must be the same size as LARGE_INTEGER"
   );
   DWORD iWhence;
   switch (sfWhence.base()) {
      // default is here just to silence compiler warnings.
      default:
      case seek_from::start:
         iWhence = FILE_BEGIN;
         break;
      case seek_from::current:
         iWhence = FILE_CURRENT;
         break;
      case seek_from::end:
         iWhence = FILE_END;
         break;
   }
   LARGE_INTEGER ibNewOffset;
   ibNewOffset.QuadPart = ibOffset;
#if _WIN32_WINNT >= 0x0500
   if (!::SetFilePointerEx(m_fd.get(), ibNewOffset, &ibNewOffset, iWhence)) {
      throw_os_error();
   }
#else //if _WIN32_WINNT >= 0x0500
   ibNewOffset.LowPart = ::SetFilePointer(
      m_fd.get(), ibNewOffset.LowPart, &ibNewOffset.HighPart, iWhence
   );
   if (ibNewOffset.LowPart == INVALID_SET_FILE_POINTER) {
      DWORD iErr = ::GetLastError();
      if (iErr != ERROR_SUCCESS) {
         throw_os_error(iErr);
      }
   }
#endif //if _WIN32_WINNT >= 0x0500 … else
   return ibNewOffset.QuadPart;

#else //if ABC_TARGET_API_POSIX … elif ABC_TARGET_API_WIN32
   #error "TODO: TARGET_API"
#endif //if ABC_TARGET_API_POSIX … elif ABC_TARGET_API_WIN32 … else
}

/*virtual*/ full_size_t regular_file_base::size() const /*override*/ {
   ABC_TRACE_FUNC(this);

   return m_cb;
}

/*virtual*/ offset_t regular_file_base::tell() const /*override*/ {
   ABC_TRACE_FUNC(this);

#if ABC_TARGET_API_POSIX || ABC_TARGET_API_WIN32
   // Seeking 0 bytes from the current position won’t change the internal status of the file
   // descriptor, so casting the const-ness away is not semantically wrong.
   return const_cast<regular_file_base *>(this)->seek(0, seek_from::current);
#else
   #error "TODO: TARGET_API"
#endif
}

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::regular_file_reader

namespace abc {
namespace io {
namespace binary {

regular_file_reader::regular_file_reader(detail::file_init_data * pfid) :
   file_base(pfid),
   regular_file_base(pfid),
   file_reader(pfid) {
}

/*virtual*/ regular_file_reader::~regular_file_reader() {
}

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::regular_file_writer

namespace abc {
namespace io {
namespace binary {

regular_file_writer::regular_file_writer(detail::file_init_data * pfid) :
   file_base(pfid),
   regular_file_base(pfid),
   file_writer(pfid) {
   ABC_TRACE_FUNC(this, pfid);

#if ABC_TARGET_API_WIN32
   m_bAppend = (pfid->am == access_mode::append);
#endif
}

/*virtual*/ regular_file_writer::~regular_file_writer() {
}

#if ABC_TARGET_API_WIN32
/*virtual*/ std::size_t regular_file_writer::write(void const * p, std::size_t cb) /*override*/ {
   ABC_TRACE_FUNC(this, p, cb);

   // Emulating O_APPEND in Win32 requires a little more code: we have to manually seek to EOF, then
   // write-protect the bytes we’re going to add, and then release the write protection.

   /*! Win32 ::LockFile() / ::UnlockFile() helper.

   TODO: this will probably find use somewhere else as well, so move it to file.hxx.
   */
   class file_lock {
   public:
      //! Constructor.
      file_lock() :
         m_fd(INVALID_HANDLE_VALUE) {
      }

      //! Destructor.
      ~file_lock() {
         if (m_fd != INVALID_HANDLE_VALUE) {
            unlock();
         }
      }

      /*! Attempts to lock a range of bytes for the specified file. Returns true if a lock was
      acquired, false if it was not because of any or all of the requested bytes being locked by
      another process, or throws an exception for any other error.

      fd
         Open file to lock.
      ibOffset
         Offset of the first byte to lock.
      cb
         Count of bytes to lock, starting from ibOffset.
      return
         true if the specified range could be locked, or false if the range has already been locked.
      */
      bool lock(filedesc_t fd, offset_t ibOffset, full_size_t cb) {
         if (m_fd != INVALID_HANDLE_VALUE) {
            unlock();
         }
         m_fd = fd;
         m_ibOffset.QuadPart = static_cast<LONGLONG>(ibOffset);
         m_cb.QuadPart = static_cast<LONGLONG>(cb);
         if (!::LockFile(
            m_fd, m_ibOffset.LowPart, static_cast<DWORD>(m_ibOffset.HighPart), m_cb.LowPart,
            static_cast<DWORD>(m_cb.HighPart)
         )) {
            DWORD iErr = ::GetLastError();
            if (iErr == ERROR_LOCK_VIOLATION) {
               return false;
            }
            throw_os_error(iErr);
         }
         return true;
      }

      //! Releases the lock acquired by lock().
      void unlock() {
         if (!::UnlockFile(
            m_fd, m_ibOffset.LowPart, static_cast<DWORD>(m_ibOffset.HighPart), m_cb.LowPart,
            static_cast<DWORD>(m_cb.HighPart)
         )) {
            throw_os_error();
         }
      }

   private:
      //! Locked file.
      filedesc_t m_fd;
      //! Start of the locked byte range.
      LARGE_INTEGER m_ibOffset;
      //! Length of the locked byte range.
      LARGE_INTEGER m_cb;
   };

   // The file_lock has to be in this scope, so it will unlock after the write is performed.
   file_lock flAppend;
   if (m_bAppend) {
      /* In this loop, we’ll seek to EOF and try to lock the not-yet-existing bytes that we want to
      write to; if the latter fails, we’ll assume that somebody else is doing the same, so we’ll
      retry from the seek.

      TODO: guarantee of termination? Maybe the foreign locker won’t release the lock, ever. This is
      too easy to fool. */

      offset_t ibEOF;
      do {
         ibEOF = seek(0, seek_from::end);
      } while (!flAppend.lock(m_fd.get(), ibEOF, cb));
      // Now the write can occur; the lock will be released automatically at the end.
   }

   return file_writer::write(p, cb);
}
#endif //if ABC_TARGET_API_WIN32

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
