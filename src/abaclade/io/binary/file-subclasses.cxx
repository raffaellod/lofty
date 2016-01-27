/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2016 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#include <abaclade/coroutine.hxx>
#include <abaclade/io/binary.hxx>
#include <abaclade/numeric.hxx>
#include <abaclade/text.hxx>
#include <abaclade/thread.hxx>
#include "detail/file_init_data.hxx"
#include "file-subclasses.hxx"

#if ABC_HOST_API_WIN32
   #include <algorithm> // std::min()
#endif
#include <climits> // CHAR_BIT

#if ABC_HOST_API_POSIX
   #include <sys/stat.h> // stat fstat()
   #include <unistd.h> // lseek()
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

tty_file_stream::tty_file_stream(detail::file_init_data * pfid) :
   file_stream(pfid) {
}

/*virtual*/ tty_file_stream::~tty_file_stream() {
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

tty_istream::tty_istream(detail::file_init_data * pfid) :
   file_stream(pfid),
   tty_file_stream(pfid),
   file_istream(pfid) {
}

/*virtual*/ tty_istream::~tty_istream() {
}

#if ABC_HOST_API_WIN32
/*virtual*/ std::size_t tty_istream::read(void * p, std::size_t cbMax) /*override*/ {
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
::WORD const tty_ostream::smc_aiAnsiColorToForegroundColor[] = {
   /* black   */ 0,
   /* red     */ FOREGROUND_RED,
   /* green   */                  FOREGROUND_GREEN,
   /* yellow  */ FOREGROUND_RED | FOREGROUND_GREEN,
   /* blue    */                                     FOREGROUND_BLUE,
   /* magenta */ FOREGROUND_RED |                    FOREGROUND_BLUE,
   /* cyan    */                  FOREGROUND_GREEN | FOREGROUND_BLUE,
   /* white   */ FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
};
::WORD const tty_ostream::smc_aiAnsiColorToBackgroundColor[] = {
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

tty_ostream::tty_ostream(detail::file_init_data * pfid) :
   file_stream(pfid),
   tty_file_stream(pfid),
   file_ostream(pfid) {
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

/*virtual*/ tty_ostream::~tty_ostream() {
}

#if ABC_HOST_API_WIN32
/*virtual*/ void tty_ostream::clear_display_area(
   std::int16_t iRow, std::int16_t iCol, std::size_t cch
) /*override*/ {
   ABC_TRACE_FUNC(this, iRow, iCol, cch);

   // TODO: implementation.
}

/*virtual*/ void tty_ostream::get_cursor_pos_and_display_size(
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

bool tty_ostream::processing_enabled() const {
   ABC_TRACE_FUNC(this);

   ::DWORD iConsoleMode;
   if (!::GetConsoleMode(m_fd.get(), &iConsoleMode)) {
      // TODO: is this worth throwing an exception for?
      return false;
   }
   return (iConsoleMode & ENABLE_PROCESSED_OUTPUT) != 0;
}

/*virtual*/ void tty_ostream::scroll_text(std::int16_t cRows, std::int16_t cCols) /*override*/ {
   ABC_TRACE_FUNC(this, cRows, cCols);

   // TODO: implementation.
}

/*virtual*/ void tty_ostream::set_char_attributes() /*override*/ {
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

/*virtual*/ void tty_ostream::set_cursor_pos(
   std::int16_t iRow, std::int16_t iCol
) /*override*/ {
   ABC_TRACE_FUNC(this, iRow, iCol);

   ::COORD coord;
   coord.X = iCol;
   coord.Y = iRow;
   ::SetConsoleCursorPosition(m_fd.get(), coord);
}

/*virtual*/ void tty_ostream::set_cursor_visibility(bool bVisible) /*override*/ {
   ABC_TRACE_FUNC(this, bVisible);

   ::CONSOLE_CURSOR_INFO cci;
   ::GetConsoleCursorInfo(m_fd.get(), &cci);
   cci.bVisible = bVisible;
   ::SetConsoleCursorInfo(m_fd.get(), &cci);
}

/*virtual*/ void tty_ostream::set_window_title(str const & sTitle) /*override*/ {
   ABC_TRACE_FUNC(this, sTitle);

   ::SetConsoleTitle(sTitle.c_str());
}

/*virtual*/ std::size_t tty_ostream::write(void const * p, std::size_t cb) /*override*/ {
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

void tty_ostream::write_range(char_t const * pchBegin, char_t const * pchEnd) const {
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

tty_iostream::tty_iostream(detail::file_init_data * pfid) :
   file_stream(pfid),
   file_istream(pfid),
   file_ostream(pfid),
   tty_file_stream(pfid),
   file_iostream(pfid),
   tty_istream(pfid),
   tty_ostream(pfid) {
}

/*virtual*/ tty_iostream::~tty_iostream() {
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

pipe_istream::pipe_istream(detail::file_init_data * pfid) :
   file_stream(pfid),
   file_istream(pfid) {
}

/*virtual*/ pipe_istream::~pipe_istream() {
}

#if ABC_HOST_API_WIN32
/*virtual*/ bool pipe_istream::check_if_eof_or_throw_os_error(
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

pipe_ostream::pipe_ostream(detail::file_init_data * pfid) :
   file_stream(pfid),
   file_ostream(pfid) {
}

/*virtual*/ pipe_ostream::~pipe_ostream() {
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

pipe_iostream::pipe_iostream(detail::file_init_data * pfid) :
   file_stream(pfid),
   file_istream(pfid),
   file_ostream(pfid),
   file_iostream(pfid),
   pipe_istream(pfid),
   pipe_ostream(pfid) {
}

/*virtual*/ pipe_iostream::~pipe_iostream() {
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

regular_file_stream::regular_file_stream(detail::file_init_data * pfid) :
   file_stream(pfid) {
#if 0
   ABC_TRACE_FUNC(this, pfid);

#if ABC_HOST_API_POSIX
   if (pfid->bBypassCache) {
      // For unbuffered access, use the filesystem-suggested I/O size increment.
      m_cbPhysAlign = static_cast<unsigned>(pfid->statFile.st_blksize);
   }
#elif ABC_HOST_API_WIN32
   if (pfid->bBypassCache) {
      /* Should really use ::DeviceIoCtl(IOCTL_STORAGE_QUERY_PROPERTY) on the disk containing this
      file. For now, use 4 KiB alignment, since that’s the most recent commonly used physical sector
      size. */
      m_cbPhysAlign = 4096;
   }
#else
   #error "TODO: HOST_API"
#endif
#endif
}

/*virtual*/ regular_file_stream::~regular_file_stream() {
}

/*virtual*/ offset_t regular_file_stream::seek(offset_t ibOffset, seek_from sfWhence) /*override*/ {
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

/*virtual*/ full_size_t regular_file_stream::size() const /*override*/ {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_API_POSIX
   struct ::stat statFile;
   if (::fstat(m_fd.get(), &statFile)) {
      exception::throw_os_error();
   }
   return static_cast<full_size_t>(statFile.st_size);
#elif ABC_HOST_API_WIN32
   #if _WIN32_WINNT >= 0x0500
      ::LARGE_INTEGER cb;
      if (!::GetFileSizeEx(m_fd.get(), &cb)) {
         exception::throw_os_error();
      }
      return static_cast<full_size_t>(cb.QuadPart);
   #else
      ::DWORD cbHigh, cbLow = ::GetFileSize(fd.get(), &cbHigh);
      if (cbLow == INVALID_FILE_SIZE) {
         ::DWORD iErr = ::GetLastError();
         if (iErr != ERROR_SUCCESS) {
            exception::throw_os_error(iErr);
         }
      }
      return (static_cast<full_size_t>(cbHigh) << sizeof(cbLow) * CHAR_BIT) | cbLow;
   #endif
#else
   #error "TODO: HOST_API"
#endif
}

/*virtual*/ offset_t regular_file_stream::tell() const /*override*/ {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_API_POSIX || ABC_HOST_API_WIN32
   /* Seeking 0 bytes from the current position won’t change the internal status of the file
   descriptor, so casting the const-ness away is not semantically wrong. */
   return const_cast<regular_file_stream *>(this)->seek(0, seek_from::current);
#else
   #error "TODO: HOST_API"
#endif
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

regular_file_istream::regular_file_istream(detail::file_init_data * pfid) :
   file_stream(pfid),
   regular_file_stream(pfid),
   file_istream(pfid) {
}

/*virtual*/ regular_file_istream::~regular_file_istream() {
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

regular_file_ostream::regular_file_ostream(detail::file_init_data * pfid) :
   file_stream(pfid),
   regular_file_stream(pfid),
   file_ostream(pfid) {
   ABC_TRACE_FUNC(this, pfid);

#if ABC_HOST_API_WIN32
   m_bAppend = (pfid->am == access_mode::write_append);
#endif
}

/*virtual*/ regular_file_ostream::~regular_file_ostream() {
}

#if ABC_HOST_API_WIN32
/*virtual*/ std::size_t regular_file_ostream::write(void const * p, std::size_t cb) /*override*/ {
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

   return file_ostream::write(p, cb);
}
#endif //if ABC_HOST_API_WIN32

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

regular_file_iostream::regular_file_iostream(detail::file_init_data * pfid) :
   file_stream(pfid),
   file_istream(pfid),
   file_ostream(pfid),
   regular_file_stream(pfid),
   file_iostream(pfid),
   regular_file_istream(pfid),
   regular_file_ostream(pfid) {
}

/*virtual*/ regular_file_iostream::~regular_file_iostream() {
}

}}} //namespace abc::io::binary
