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
#include <abaclade/numeric.hxx>
#include <abaclade/text.hxx>
#include <abaclade/thread.hxx>
#include "detail/file_init_data.hxx"

#include <algorithm> // std::min()
#include <climits> // CHAR_BIT
#if ABC_HOST_API_POSIX
   #include <errno.h> // E* errno
   #include <sys/poll.h> // pollfd poll()
   #include <unistd.h> // ssize_t read() write()
#endif


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

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

console_file_base::console_file_base(detail::file_init_data * pfid) :
   file_base(pfid) {
}

/*virtual*/ console_file_base::~console_file_base() {
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

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

   // Note: ::ReadConsole() expects and returns character counts in place of byte counts.

   ::DWORD cchRead, cchToRead = static_cast< ::DWORD>(
      std::min<std::size_t>(cbMax, numeric::max< ::DWORD>::value)
   ) / sizeof(char_t);
   if (!::ReadConsole(m_fd.get(), p, cchToRead, &cchRead, nullptr)) {
      ::DWORD iErr = ::GetLastError();
      if (iErr != ERROR_HANDLE_EOF) {
         exception::throw_os_error(iErr);
      }
   }
   this_coroutine::interruption_point();
   return sizeof(char_t) * cchRead;
}
#endif //if ABC_HOST_API_WIN32

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

#if ABC_HOST_API_WIN32
::WORD const console_writer::smc_aiAnsiColorToForegroundColor[] = {
   /* black   */ 0,
   /* red     */ FOREGROUND_RED,
   /* green   */                  FOREGROUND_GREEN,
   /* yellow  */ FOREGROUND_RED | FOREGROUND_GREEN,
   /* blue    */                                     FOREGROUND_BLUE,
   /* magenta */ FOREGROUND_RED |                    FOREGROUND_BLUE,
   /* cyan    */                  FOREGROUND_GREEN | FOREGROUND_BLUE,
   /* white   */ FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
};
::WORD const console_writer::smc_aiAnsiColorToBackgroundColor[] = {
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
         csbi.wAttributes & (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)
      ) == smc_aiAnsiColorToBackgroundColor[i]) {
         m_chattrDefault.clrBackground = static_cast<ansi_terminal_color::enum_type>(i);
      }
      if ((
         csbi.wAttributes & (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
      ) == smc_aiAnsiColorToForegroundColor[i]) {
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

   ::DWORD iConsoleMode;
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

   ::WORD iAttr;
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

/*virtual*/ void console_writer::set_window_title(str const & sTitle) /*override*/ {
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
   this_coroutine::interruption_point();
   return cb;
}

void console_writer::write_range(char_t const * pchBegin, char_t const * pchEnd) const {
   ABC_TRACE_FUNC(this, pchBegin, pchEnd);

   // This loop may repeat more than once in the unlikely case cch exceeds what can fit in a DWORD.
   while (std::size_t cch = static_cast<std::size_t>(pchEnd - pchBegin)) {
      ::DWORD cchLastWritten;
      if (!::WriteConsole(
         m_fd.get(), pchBegin, static_cast< ::DWORD>(
            std::min<std::size_t>(cch, numeric::max< ::DWORD>::value)
         ), &cchLastWritten, nullptr
      )) {
         exception::throw_os_error();
      }
      // Some bytes were written; prepare for the next attempt.
      pchBegin += cchLastWritten;
   }
}
#endif //if ABC_HOST_API_WIN32

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

console_readwriter::console_readwriter(detail::file_init_data * pfid) :
   file_base(pfid),
   file_reader(pfid),
   file_writer(pfid),
   console_file_base(pfid),
   file_readwriter(pfid),
   console_reader(pfid),
   console_writer(pfid) {
}

/*virtual*/ console_readwriter::~console_readwriter() {
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

pipe_reader::pipe_reader(detail::file_init_data * pfid) :
   file_base(pfid),
   file_reader(pfid) {
}

/*virtual*/ pipe_reader::~pipe_reader() {
}

#if ABC_HOST_API_WIN32
/*virtual*/ bool pipe_reader::check_if_eof_or_throw_os_error(
   ::DWORD cbRead, ::DWORD iErr
) const /*override*/ {
   ABC_UNUSED_ARG(cbRead);
   switch (iErr) {
      case ERROR_SUCCESS:
         return false;
      case ERROR_BROKEN_PIPE:
         return true;
      default:
         exception::throw_os_error(iErr);
   }
}
#endif //if ABC_HOST_API_WIN32

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

pipe_writer::pipe_writer(detail::file_init_data * pfid) :
   file_base(pfid),
   file_writer(pfid) {
}

/*virtual*/ pipe_writer::~pipe_writer() {
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

pipe_readwriter::pipe_readwriter(detail::file_init_data * pfid) :
   file_base(pfid),
   file_reader(pfid),
   file_writer(pfid),
   file_readwriter(pfid),
   pipe_reader(pfid),
   pipe_writer(pfid) {
}

/*virtual*/ pipe_readwriter::~pipe_readwriter() {
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

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
   if (!::GetFileSizeEx(m_fd.get(), reinterpret_cast< ::LARGE_INTEGER *>(&m_cb))) {
      exception::throw_os_error();
   }
#else //if _WIN32_WINNT >= 0x0500
   ::DWORD cbHigh, cbLow = ::GetFileSize(fd.get(), &cbHigh);
   if (cbLow == INVALID_FILE_SIZE) {
      ::DWORD iErr = ::GetLastError();
      if (iErr != ERROR_SUCCESS) {
         exception::throw_os_error(iErr);
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
   if (ibNewOffset < 0) {
      exception::throw_os_error();
   }
   return ibNewOffset;

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX

   static_assert(
      sizeof(ibOffset) == sizeof(::LARGE_INTEGER),
      "abc::io::offset_t must be the same size as LARGE_INTEGER"
   );
   ::DWORD iWhence;
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
   ::LARGE_INTEGER ibNewOffset;
   ibNewOffset.QuadPart = ibOffset;
#if _WIN32_WINNT >= 0x0500
   if (!::SetFilePointerEx(m_fd.get(), ibNewOffset, &ibNewOffset, iWhence)) {
      exception::throw_os_error();
   }
#else //if _WIN32_WINNT >= 0x0500
   ibNewOffset.LowPart = ::SetFilePointer(
      m_fd.get(), ibNewOffset.LowPart, &ibNewOffset.HighPart, iWhence
   );
   if (ibNewOffset.LowPart == INVALID_SET_FILE_POINTER) {
      ::DWORD iErr = ::GetLastError();
      if (iErr != ERROR_SUCCESS) {
         exception::throw_os_error(iErr);
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
   /* Seeking 0 bytes from the current position won’t change the internal status of the file
   descriptor, so casting the const-ness away is not semantically wrong. */
   return const_cast<regular_file_base *>(this)->seek(0, seek_from::current);
#else
   #error "TODO: HOST_API"
#endif
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

regular_file_reader::regular_file_reader(detail::file_init_data * pfid) :
   file_base(pfid),
   regular_file_base(pfid),
   file_reader(pfid) {
}

/*virtual*/ regular_file_reader::~regular_file_reader() {
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

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

   /* Emulating O_APPEND in Win32 requires a little more code: we have to manually seek to EOF, then
   write-protect the bytes we’re going to add, and then release the write protection. */

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
         ABC_TRACE_FUNC(this, fd, ibOffset, cb);

         if (m_fd != INVALID_HANDLE_VALUE) {
            unlock();
         }
         m_fd = fd;
         m_ibOffset.QuadPart = static_cast< ::LONGLONG>(ibOffset);
         m_cb.QuadPart = static_cast< ::LONGLONG>(cb);
         if (!::LockFile(
            m_fd, m_ibOffset.LowPart, static_cast< ::DWORD>(m_ibOffset.HighPart), m_cb.LowPart,
            static_cast< ::DWORD>(m_cb.HighPart)
         )) {
            ::DWORD iErr = ::GetLastError();
            if (iErr == ERROR_LOCK_VIOLATION) {
               return false;
            }
            exception::throw_os_error(iErr);
         }
         return true;
      }

      //! Releases the lock acquired by lock().
      void unlock() {
         ABC_TRACE_FUNC(this);

         if (!::UnlockFile(
            m_fd, m_ibOffset.LowPart, static_cast< ::DWORD>(m_ibOffset.HighPart), m_cb.LowPart,
            static_cast< ::DWORD>(m_cb.HighPart)
         )) {
            exception::throw_os_error();
         }
      }

   private:
      //! Locked file.
      filedesc_t m_fd;
      //! Start of the locked byte range.
      ::LARGE_INTEGER m_ibOffset;
      //! Length of the locked byte range.
      ::LARGE_INTEGER m_cb;
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

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

regular_file_readwriter::regular_file_readwriter(detail::file_init_data * pfid) :
   file_base(pfid),
   file_reader(pfid),
   file_writer(pfid),
   regular_file_base(pfid),
   file_readwriter(pfid),
   regular_file_reader(pfid),
   regular_file_writer(pfid) {
}

/*virtual*/ regular_file_readwriter::~regular_file_readwriter() {
}

}}} //namespace abc::io::binary
