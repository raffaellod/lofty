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
#include "detail/file_init_data.hxx"

#include <algorithm> // std::min()
#if ABC_HOST_API_POSIX
   #include <errno.h> // E* errno
   #include <sys/poll.h> // pollfd poll()
   #include <unistd.h> // ssize_t read() write()
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::file_base

namespace abc {
namespace io {
namespace binary {

file_base::file_base(detail::file_init_data * pfid) :
   m_fd(std::move(pfid->fd)),
#if ABC_HOST_API_POSIX
   m_pAsyncBuf(nullptr),
   m_cbAsyncBuf(0),
#endif
   m_bAllowAsync(pfid->bAllowAsync) {
#if ABC_HOST_API_WIN32
   if (m_bAllowAsync) {
      memory::clear(&m_ovl);
   }
#endif
}

/*virtual*/ file_base::~file_base() {
}

#if ABC_HOST_API_POSIX

bool file_base::async_poll(bool bWrite, bool bWait) const {
   ::pollfd pfd;
   pfd.fd = m_fd.get();
   pfd.events = (bWrite ? POLLOUT : POLLIN) | POLLPRI;
   // This may repeat in case of EINTR.
   for (;;) {
      int iRet = ::poll(&pfd, 1, bWait ? -1 : 0);
      if (iRet >= 0) {
         if (iRet > 0) {
            if (pfd.revents & (POLLERR | POLLNVAL)) {
               // TODO: how should async_join() return I/O errors?
            }
            /* Consider POLLHUP as input, so that ::read() can return 0 bytes. This helps mitigate
            the considerable differences among poll(2) implementations documented at
            <http://www.greenend.org.uk/rjk/tech/poll.html>, and Linux happens to be one of those
            setting *only* POLLHUP on a pipe with no open write fds. */
            if (pfd.revents & (bWrite ? POLLOUT : (POLLIN | POLLHUP))) {
               if (pfd.revents & POLLPRI) {
                  // TODO: anything special about “high priority data”?
               }
               return true;
            }
         }
         return false;
      }
      int iErr = errno;
      if (iErr != EINTR) {
         throw_os_error(iErr);
      }
   }
}

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX

/*virtual*/ std::size_t file_base::async_join() /*override*/ {
   ABC_TRACE_FUNC(this);

   if (!m_bAllowAsync) {
      // TODO: is async_join() on non-async file an error?
      return 0;
   }
   DWORD cbTransferred;
   if (!::GetOverlappedResult(m_fd.get(), &m_ovl, &cbTransferred, true)) {
      // TODO: how should async_join() return I/O errors?
      throw_os_error();
   }
   return cbTransferred;
}

/*virtual*/ bool file_base::async_pending() /*override*/ {
   ABC_TRACE_FUNC(this);

   if (!m_bAllowAsync) {
      return false;
   }
   DWORD cbTransferred;
   if (::GetOverlappedResult(m_fd.get(), &m_ovl, &cbTransferred, false)) {
      // Discard cbTransferred; clients will need to call async_join() to get the byte count.
      return false;
   }
   DWORD iErr = ::GetLastError();
   if (iErr == ERROR_IO_INCOMPLETE) {
      return true;
   }
   // TODO: how should async_pending() return I/O errors?
   throw_os_error();
}

#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32

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
#if ABC_HOST_API_WIN32
   if (m_bAllowAsync) {
      /* TODO: cancel the I/O and block to wait the end of cancellation. This gives a behavior
      similar to POSIX. */
      // TODO: warn that this is a bug because I/O errors are not being checked for.
   }
#endif
}

#if ABC_HOST_API_POSIX
/*virtual*/ std::size_t file_reader::async_join() /*override*/ {
   ABC_TRACE_FUNC(this);

   if (!m_pAsyncBuf) {
      // TODO: is async_join() on non-async file an error?
      return 0;
   }
   async_poll(false /*poll reading*/, true /*wait*/);
   std::ptrdiff_t cbRead = read_impl(m_pAsyncBuf, m_cbAsyncBuf);
   // If no exceptions were thrown, the buffer no longer needs to be tracked.
   m_pAsyncBuf = nullptr;
   m_cbAsyncBuf = 0;
   if (cbRead == -1) {
      /* This is possible in spite of the earlier poll(2) call if the data that was available turned
      out to be bad (e.g. CRC error) and had to be discarded. */
      // TODO: FIXME: this *will* confuse callers, being the same value returned for EOF.
      return 0;
   } else {
      return static_cast<std::size_t>(cbRead);
   }
}

/*virtual*/ bool file_reader::async_pending() /*override*/ {
   ABC_TRACE_FUNC(this);

   if (m_pAsyncBuf) {
      return async_poll(false /*poll reading*/, false /*don’t wait*/);
   } else {
      return false;
   }
}
#endif //if ABC_HOST_API_POSIX

/*virtual*/ std::size_t file_reader::read(void * p, std::size_t cbMax) /*override*/ {
   ABC_TRACE_FUNC(this, p, cbMax);

   async_join();
#if ABC_HOST_API_POSIX
   std::ptrdiff_t cbRead = read_impl(
      p, std::min<std::size_t>(cbMax, numeric::max< ::ssize_t>::value)
   );
   if (cbRead >= 0) {
      return static_cast<std::size_t>(cbRead);
   } else {
      // Remember the buffer for later attempts in async_join().
      m_pAsyncBuf = p;
      m_cbAsyncBuf = cbMax;
      // TODO: decide whether pending I/O should return 0 or tuple<size_t, bool>.
      return 0;
   }
#elif ABC_HOST_API_WIN32
   static_assert(sizeof(std::size_t) >= sizeof(DWORD), "fix read size calculation");
   ::OVERLAPPED * povl;
   if (m_bAllowAsync) {
      // Obtain the current file offset and set m_ovl to start there.
      long ibOffsetHigh = 0;
      m_ovl.Offset = ::SetFilePointer(m_fd.get(), 0, &ibOffsetHigh, FILE_CURRENT);
      m_ovl.OffsetHigh = static_cast<DWORD>(ibOffsetHigh);
      // Ignore errors; if m_fd is not a seekable file, ::ReadFile() will ignore Offset* anyway.
      povl = &m_ovl;
   } else {
      povl = nullptr;
   }
   DWORD cbRead;
   BOOL bRet = ::ReadFile(
      m_fd.get(), p, static_cast<DWORD>(std::min<std::size_t>(cbMax, numeric::max<DWORD>::value)),
      &cbRead, povl
   );
   DWORD iErr = bRet ? ERROR_SUCCESS : ::GetLastError();
   if (iErr == ERROR_IO_PENDING) {
      // TODO: decide whether pending I/O should return 0 or tuple<size_t, bool>.
      return 0;
   }
   if (check_if_eof_or_throw_os_error(cbRead, iErr)) {
      return 0;
   }
   return static_cast<std::size_t>(cbRead);
#else
   #error "TODO: HOST_API"
#endif
}

#if ABC_HOST_API_POSIX
std::ptrdiff_t file_reader::read_impl(void * p, std::size_t cbMax) {
   ABC_TRACE_FUNC(this, p, cbMax);

   static_assert(sizeof(std::ptrdiff_t) == sizeof(::ssize_t), "");
   // This may repeat in case of EINTR.
   for (;;) {
      ::ssize_t cbRead = ::read(m_fd.get(), p, cbMax);
      if (cbRead >= 0) {
         return cbRead;
      }
      int iErr = errno;
      switch (iErr) {
         case EINTR:
            break;
         case EAGAIN: // Try again (POSIX.1-2001)
   // These two values may or may not be different.
   #if EWOULDBLOCK != EAGAIN
         case EWOULDBLOCK: // Operation would block (POSIX.1-2001)
   #endif
            return -1;
         default:
            throw_os_error(iErr);
      }
   }
}
#endif

#if ABC_HOST_API_WIN32
/*virtual*/ bool file_reader::check_if_eof_or_throw_os_error(DWORD cbRead, DWORD iErr) const {
   if (iErr != ERROR_SUCCESS) {
      throw_os_error(iErr);
   }
   return cbRead == 0;
}
#endif //if ABC_HOST_API_WIN32

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
#if ABC_HOST_API_WIN32
   if (m_bAllowAsync) {
      // Block to avoid segfaults due to the buffer being deallocated while in use by the kernel.
      // TODO: NO, instead cancel the I/O and block to wait the end of cancellation.
      async_join();
      // TODO: warn that this is a bug because I/O errors are not being checked for.
   }
#endif
}

#if ABC_HOST_API_POSIX
/*virtual*/ std::size_t file_writer::async_join() /*override*/ {
   ABC_TRACE_FUNC(this);

   if (!m_pAsyncBuf) {
      // TODO: is async_join() on non-async file an error?
      return 0;
   }
   async_poll(true /*poll writing*/, true /*wait*/);
   std::ptrdiff_t cbWritten = write_impl(m_pAsyncBuf, m_cbAsyncBuf);
   // If no exceptions were thrown, the buffer no longer needs to be tracked.
   m_pAsyncBuf = nullptr;
   m_cbAsyncBuf = 0;
   if (cbWritten == -1) {
      // Should not be possible.
      return 0;
   } else {
      return static_cast<std::size_t>(cbWritten);
   }
}

/*virtual*/ bool file_writer::async_pending() /*override*/ {
   ABC_TRACE_FUNC(this);

   if (m_pAsyncBuf) {
      return async_poll(true /*poll writing*/, false /*don’t wait*/);
   } else {
      return false;
   }
}
#endif //if ABC_HOST_API_POSIX

/*virtual*/ void file_writer::flush() /*override*/ {
   ABC_TRACE_FUNC(this);

   async_join();
#if ABC_HOST_API_POSIX
   // TODO: investigate fdatasync().
   if (::fsync(m_fd.get())) {
      throw_os_error();
   }
#elif ABC_HOST_API_WIN32
   if (!::FlushFileBuffers(m_fd.get())) {
      throw_os_error();
   }
#else
   #error "TODO: HOST_API"
#endif
}

/*virtual*/ std::size_t file_writer::write(void const * p, std::size_t cb) /*override*/ {
   ABC_TRACE_FUNC(this, p, cb);

   async_join();
#if ABC_HOST_API_POSIX
   std::ptrdiff_t cbWritten = write_impl(
      p, std::min<std::size_t>(cb, numeric::max< ::ssize_t>::value)
   );
   if (cbWritten >= 0) {
      return static_cast<std::size_t>(cbWritten);
   } else {
      // Remember the buffer for later attempts in async_join().
      m_pAsyncBuf = const_cast<void *>(p);
      m_cbAsyncBuf = cb;
      // TODO: decide whether pending I/O should return 0 or tuple<size_t, bool>.
      return 0;
   }
#elif ABC_HOST_API_WIN32
   static_assert(sizeof(std::size_t) >= sizeof(DWORD), "fix write size calculation");
   ::OVERLAPPED * povl;
   if (m_bAllowAsync) {
      // Obtain the current file offset and set m_ovl to start there.
      long ibOffsetHigh = 0;
      m_ovl.Offset = ::SetFilePointer(m_fd.get(), 0, &ibOffsetHigh, FILE_CURRENT);
      m_ovl.OffsetHigh = static_cast<DWORD>(ibOffsetHigh);
      // Ignore errors; if m_fd is not a seekable file, ::WriteFile() will ignore Offset* anyway.
      povl = &m_ovl;
   } else {
      povl = nullptr;
   }
   DWORD cbWritten;
   if (!::WriteFile(
      m_fd.get(), p, static_cast<DWORD>(std::min<std::size_t>(cb, numeric::max<DWORD>::value)),
      &cbWritten, nullptr
   )) {
      DWORD iErr = ::GetLastError();
      if (iErr == ERROR_IO_PENDING) {
         // TODO: decide whether pending I/O should return 0 or tuple<size_t, bool>.
         return 0;
      }
      throw_os_error(iErr);
   }
   return static_cast<std::size_t>(cbWritten);
#else
   #error "TODO: HOST_API"
#endif
}

#if ABC_HOST_API_POSIX
std::ptrdiff_t file_writer::write_impl(void const * p, std::size_t cb) {
   ABC_TRACE_FUNC(this, p, cb);

   static_assert(sizeof(std::ptrdiff_t) == sizeof(::ssize_t), "");
   // This may repeat in case of EINTR.
   for (;;) {
      ::ssize_t cbWritten = ::write(m_fd.get(), p, cb);
      if (cbWritten >= 0) {
         return cbWritten;
      }
      int iErr = errno;
      switch (iErr) {
         case EINTR:
            break;
         case EAGAIN: // Try again (POSIX.1-2001)
   // These two values may or may not be different.
   #if EWOULDBLOCK != EAGAIN
         case EWOULDBLOCK: // Operation would block (POSIX.1-2001)
   #endif
            return -1;
         default:
            throw_os_error(iErr);
      }
   }
}
#endif

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

#if ABC_HOST_API_WIN32
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
#endif //if ABC_HOST_API_WIN32

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::console_writer

namespace abc {
namespace io {
namespace binary {

#if ABC_HOST_API_WIN32
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
#if ABC_HOST_API_WIN32
   ABC_TRACE_FUNC(this, pfid);

   ::CONSOLE_SCREEN_BUFFER_INFO csbi;
   ::GetConsoleScreenBufferInfo(m_fd.get(), &csbi);
   for (unsigned i = 0; i < ABC_COUNTOF(smc_aiAnsiColorToForegroundColor); ++i) {
      using abc::text::parsers::ansi_terminal_color;
      if ((
         csbi.wAttributes & (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE
      )) == smc_aiAnsiColorToBackgroundColor[i]) {
         m_chattrDefault.clrBackground = static_cast<ansi_terminal_color::enum_type>(i);
      }
      if ((
         csbi.wAttributes & (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
      )) == smc_aiAnsiColorToForegroundColor[i]) {
         m_chattrDefault.clrForeground = static_cast<ansi_terminal_color::enum_type>(i);
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

#if ABC_HOST_API_WIN32
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
#endif //if ABC_HOST_API_WIN32

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

#if ABC_HOST_API_WIN32
/*virtual*/ bool pipe_reader::check_if_eof_or_throw_os_error(
   DWORD cbRead, DWORD iErr
) const /*override*/ {
   ABC_UNUSED_ARG(cbRead);
   switch (iErr) {
      case ERROR_SUCCESS:
         return false;
      case ERROR_BROKEN_PIPE:
         return true;
      default:
         throw_os_error(iErr);
   }
}
#endif //if ABC_HOST_API_WIN32

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

#if ABC_HOST_API_POSIX

   m_cb = static_cast<std::size_t>(pfid->statFile.st_size);
#if 0
   if (pfid->bBypassCache) {
      // For unbuffered access, use the filesystem-suggested I/O size increment.
      m_cbPhysAlign = static_cast<unsigned>(pfid->statFile.st_blksize);
   }
#endif

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX

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
   if (pfid->bBypassCache) {
      /* Should really use ::DeviceIoCtl(IOCTL_STORAGE_QUERY_PROPERTY) on the disk containing this
      file. For now, use 4 KiB alignment, since that’s the most recent commonly used physical sector
      size. */
      m_cbPhysAlign = 4096;
   }
#endif

#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
}

/*virtual*/ regular_file_base::~regular_file_base() {
}

/*virtual*/ offset_t regular_file_base::seek(offset_t ibOffset, seek_from sfWhence) /*override*/ {
   ABC_TRACE_FUNC(this, ibOffset, sfWhence);

#if ABC_HOST_API_POSIX

   int iWhence;
   switch (sfWhence.base()) {
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

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX

   static_assert(
      sizeof(ibOffset) == sizeof(LARGE_INTEGER),
      "abc::io::offset_t must be the same size as LARGE_INTEGER"
   );
   DWORD iWhence;
   switch (sfWhence.base()) {
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

#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
}

/*virtual*/ full_size_t regular_file_base::size() const /*override*/ {
   ABC_TRACE_FUNC(this);

   return m_cb;
}

/*virtual*/ offset_t regular_file_base::tell() const /*override*/ {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_API_POSIX || ABC_HOST_API_WIN32
   // Seeking 0 bytes from the current position won’t change the internal status of the file
   // descriptor, so casting the const-ness away is not semantically wrong.
   return const_cast<regular_file_base *>(this)->seek(0, seek_from::current);
#else
   #error "TODO: HOST_API"
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

#if ABC_HOST_API_WIN32
   m_bAppend = (pfid->am == access_mode::write_append);
#endif
}

/*virtual*/ regular_file_writer::~regular_file_writer() {
}

#if ABC_HOST_API_WIN32
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
#endif //if ABC_HOST_API_WIN32

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
