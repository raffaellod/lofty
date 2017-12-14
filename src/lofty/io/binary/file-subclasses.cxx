/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/coroutine.hxx>
#include <lofty/io/binary.hxx>
#include <lofty/numeric.hxx>
#include <lofty/text.hxx>
#include <lofty/thread.hxx>
#include "_pvt/file_init_data.hxx"
#include "file-subclasses.hxx"

#if LOFTY_HOST_API_WIN32
   #include <algorithm> // std::min()
#endif
#include <climits> // CHAR_BIT

#if LOFTY_HOST_API_POSIX
   #include <sys/stat.h> // stat fstat()
   #include <unistd.h> // lseek()
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

tty_file_stream::tty_file_stream(_pvt::file_init_data * init_data) :
   file_stream(init_data) {
}

/*virtual*/ tty_file_stream::~tty_file_stream() {
}

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

tty_istream::tty_istream(_pvt::file_init_data * init_data) :
   file_stream(init_data),
   tty_file_stream(init_data),
   file_istream(init_data) {
}

/*virtual*/ tty_istream::~tty_istream() {
}

#if LOFTY_HOST_API_WIN32
/*virtual*/ std::size_t tty_istream::read_bytes(void * dst, std::size_t dst_max) /*override*/ {
   // Note: ::ReadConsole() expects and returns character counts in place of byte counts.

   ::DWORD chars_read, chars_to_read = static_cast< ::DWORD>(
      std::min<std::size_t>(dst_max, numeric::max< ::DWORD>::value)
   ) / sizeof(char_t);
   if (!::ReadConsole(fd.get(), dst, chars_to_read, &chars_read, nullptr)) {
      auto err = ::GetLastError();
      if (err != ERROR_HANDLE_EOF) {
         exception::throw_os_error(err);
      }
   }
   this_coroutine::interruption_point();
   return sizeof(char_t) * chars_read;
}
#endif //if LOFTY_HOST_API_WIN32

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

#if LOFTY_HOST_API_WIN32
//! Mapping table from ANSI terminal colors to Win32 console foreground colors.
static ::WORD const ansi_colors_to_foreground_colors[] = {
   /*black  */ 0,
   /*red    */ FOREGROUND_RED,
   /*green  */                  FOREGROUND_GREEN,
   /*yellow */ FOREGROUND_RED | FOREGROUND_GREEN,
   /*blue   */                                     FOREGROUND_BLUE,
   /*magenta*/ FOREGROUND_RED |                    FOREGROUND_BLUE,
   /*cyan   */                  FOREGROUND_GREEN | FOREGROUND_BLUE,
   /*white  */ FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
};

//! Mapping table from ANSI terminal colors to Win32 console background colors.
static ::WORD const ansi_colors_to_background_colors[] = {
   /*black  */ 0,
   /*red    */ BACKGROUND_RED,
   /*green  */                  BACKGROUND_GREEN,
   /*yellow */ BACKGROUND_RED | BACKGROUND_GREEN,
   /*blue   */                                     BACKGROUND_BLUE,
   /*magenta*/ BACKGROUND_RED |                    BACKGROUND_BLUE,
   /*cyan   */                  BACKGROUND_GREEN | BACKGROUND_BLUE,
   /*white  */ BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE
};
#endif

tty_ostream::tty_ostream(_pvt::file_init_data * init_data) :
   file_stream(init_data),
   tty_file_stream(init_data),
   file_ostream(init_data) {
#if LOFTY_HOST_API_WIN32
   ::CONSOLE_SCREEN_BUFFER_INFO con_screen;
   ::GetConsoleScreenBufferInfo(fd.get(), &con_screen);
   for (unsigned i = 0; i < LOFTY_COUNTOF(ansi_colors_to_foreground_colors); ++i) {
      using lofty::text::parsers::ansi_terminal_color;
      if ((
         con_screen.wAttributes & (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE)
      ) == ansi_colors_to_background_colors[i]) {
         default_char_attr.background_color = static_cast<ansi_terminal_color::enum_type>(i);
      }
      if ((
         con_screen.wAttributes & (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
      ) == ansi_colors_to_foreground_colors[i]) {
         default_char_attr.foreground_color = static_cast<ansi_terminal_color::enum_type>(i);
      }
   }
   default_char_attr.blink_speed   = 0;
   default_char_attr.concealed     = false;
   default_char_attr.crossed_out   = false;
   default_char_attr.intensity     = (con_screen.wAttributes & FOREGROUND_INTENSITY) ? 2u : 1u;
   default_char_attr.italic        = false;
   default_char_attr.reverse_video = false;
   default_char_attr.underline     = false;

   curr_char_attr = default_char_attr;
#endif
}

/*virtual*/ tty_ostream::~tty_ostream() {
}

#if LOFTY_HOST_API_WIN32
/*virtual*/ void tty_ostream::clear_display_area(
   std::int16_t row, std::int16_t col, std::size_t char_size
) /*override*/ {
   // TODO: implementation.
}

/*virtual*/ void tty_ostream::get_cursor_pos_and_display_size(
   std::int16_t * row, std::int16_t * col, std::int16_t * rows, std::int16_t * cols
) /*override*/ {
   ::CONSOLE_SCREEN_BUFFER_INFO con_screen;
   ::GetConsoleScreenBufferInfo(fd.get(), &con_screen);
   *row = con_screen.dwCursorPosition.Y;
   *col = con_screen.dwCursorPosition.X;
   *rows = con_screen.dwSize.Y;
   *cols = con_screen.dwSize.X;
}

bool tty_ostream::processing_enabled() const {
   ::DWORD console_mode;
   if (!::GetConsoleMode(fd.get(), &console_mode)) {
      // TODO: is this worth throwing an exception for?
      return false;
   }
   return (console_mode & ENABLE_PROCESSED_OUTPUT) != 0;
}

/*virtual*/ void tty_ostream::scroll_text(std::int16_t rows, std::int16_t cols) /*override*/ {
   // TODO: implementation.
}

/*virtual*/ void tty_ostream::set_char_attributes() /*override*/ {
   ::WORD con_text_attr;
   if (curr_char_attr.concealed) {
      if (curr_char_attr.reverse_video) {
         con_text_attr  = ansi_colors_to_background_colors[curr_char_attr.foreground_color];
         con_text_attr |= ansi_colors_to_foreground_colors[curr_char_attr.foreground_color];
         if (curr_char_attr.intensity == 2) {
            // Turn on background intensity as well, to match foreground intensity.
            con_text_attr |= FOREGROUND_INTENSITY | BACKGROUND_INTENSITY;
         }
      } else {
         con_text_attr  = ansi_colors_to_background_colors[curr_char_attr.background_color];
         con_text_attr |= ansi_colors_to_foreground_colors[curr_char_attr.background_color];
      }
   } else {
      if (curr_char_attr.reverse_video) {
         con_text_attr  = ansi_colors_to_background_colors[curr_char_attr.foreground_color];
         con_text_attr |= ansi_colors_to_foreground_colors[curr_char_attr.background_color];
      } else {
         con_text_attr  = ansi_colors_to_background_colors[curr_char_attr.background_color];
         con_text_attr |= ansi_colors_to_foreground_colors[curr_char_attr.foreground_color];
      }
      if (curr_char_attr.intensity == 2) {
         con_text_attr |= FOREGROUND_INTENSITY;
      }
   }
   ::SetConsoleTextAttribute(fd.get(), con_text_attr);
}

/*virtual*/ void tty_ostream::set_cursor_pos(std::int16_t row, std::int16_t col) /*override*/ {
   ::COORD con_cur_pos;
   con_cur_pos.X = col;
   con_cur_pos.Y = row;
   ::SetConsoleCursorPosition(fd.get(), con_cur_pos);
}

/*virtual*/ void tty_ostream::set_cursor_visibility(bool visible) /*override*/ {
   ::CONSOLE_CURSOR_INFO con_cur;
   ::GetConsoleCursorInfo(fd.get(), &con_cur);
   con_cur.bVisible = visible;
   ::SetConsoleCursorInfo(fd.get(), &con_cur);
}

/*virtual*/ void tty_ostream::set_window_title(str const & title) /*override*/ {
   ::SetConsoleTitle(title.c_str());
}

/*virtual*/ std::size_t tty_ostream::write_bytes(void const * src, std::size_t src_size) /*override*/ {
   auto src_chars_begin = static_cast<char_t const *>(src);
   auto src_chars_end = reinterpret_cast<char_t const *>(static_cast<std::int8_t const *>(src) + src_size);
   auto written_src_chars_end = src_chars_begin;
   if (processing_enabled()) {
      for (auto char_ptr = src_chars_begin; char_ptr < src_chars_end; ) {
         char_t ch = *char_ptr;
         if (lofty::text::host_char_traits::is_lead_surrogate(ch)) {
            /* ::WriteConsole() is unable to handle UTF-16 surrogates, so write a replacement character in
            place of the surrogate pair. */
            if (written_src_chars_end < char_ptr) {
               write_range(written_src_chars_end, char_ptr);
            }
            ++char_ptr;
            // If a trail surrogate follows, consume it immediately.
            if (char_ptr < src_chars_end && lofty::text::host_char_traits::is_trail_char(*char_ptr)) {
               ++char_ptr;
            }
            written_src_chars_end = char_ptr;
            // Write the replacement character.
            ch = lofty::text::replacement_char;
            write_range(&ch, &ch + 1);
         } else if (consume_char(ch)) {
            // ch is part of an ANSI escape sequence.
            if (written_src_chars_end < char_ptr) {
               write_range(written_src_chars_end, char_ptr);
            }
            written_src_chars_end = ++char_ptr;
         } else {
            ++char_ptr;
         }
      }
   }
   if (written_src_chars_end < src_chars_end) {
      write_range(written_src_chars_end, src_chars_end);
   }
   this_coroutine::interruption_point();
   return src_size;
}

void tty_ostream::write_range(char_t const * src_begin, char_t const * src_end) const {
   // This loop may repeat more than once in the unlikely case src_size exceeds what can fit in a DWORD.
   while (auto src_size = static_cast<std::size_t>(src_end - src_begin)) {
      ::DWORD written_size;
      if (!::WriteConsole(
         fd.get(), src_begin, static_cast< ::DWORD>(
            std::min<std::size_t>(src_size, numeric::max< ::DWORD>::value)
         ), &written_size, nullptr
      )) {
         exception::throw_os_error();
      }
      // Some characters were written; prepare for the next attempt.
      src_begin += written_size;
   }
}
#endif //if LOFTY_HOST_API_WIN32

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

tty_iostream::tty_iostream(_pvt::file_init_data * init_data) :
   file_stream(init_data),
   file_istream(init_data),
   file_ostream(init_data),
   tty_file_stream(init_data),
   file_iostream(init_data),
   tty_istream(init_data),
   tty_ostream(init_data) {
}

/*virtual*/ tty_iostream::~tty_iostream() {
}

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

pipe_istream::pipe_istream(_pvt::file_init_data * init_data) :
   file_stream(init_data),
   file_istream(init_data) {
}

/*virtual*/ pipe_istream::~pipe_istream() {
}

#if LOFTY_HOST_API_WIN32
/*virtual*/ bool pipe_istream::check_if_eof_or_throw_os_error(
   ::DWORD read_bytes, ::DWORD err
) const /*override*/ {
   LOFTY_UNUSED_ARG(read_bytes);
   switch (err) {
      case ERROR_SUCCESS:
         return false;
      case ERROR_BROKEN_PIPE:
         return true;
      default:
         exception::throw_os_error(err);
   }
}
#endif //if LOFTY_HOST_API_WIN32

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

pipe_ostream::pipe_ostream(_pvt::file_init_data * init_data) :
   file_stream(init_data),
   file_ostream(init_data) {
}

/*virtual*/ pipe_ostream::~pipe_ostream() {
}

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

pipe_iostream::pipe_iostream(_pvt::file_init_data * init_data) :
   file_stream(init_data),
   file_istream(init_data),
   file_ostream(init_data),
   file_iostream(init_data),
   pipe_istream(init_data),
   pipe_ostream(init_data) {
}

/*virtual*/ pipe_iostream::~pipe_iostream() {
}

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

regular_file_stream::regular_file_stream(_pvt::file_init_data * init_data) :
   file_stream(init_data) {
#if 0
#if LOFTY_HOST_API_POSIX
   if (init_data->bypass_cache) {
      // For unbuffered access, use the filesystem-suggested I/O size increment.
      physical_align = static_cast<unsigned>(init_data->stat.st_blksize);
   }
#elif LOFTY_HOST_API_WIN32
   if (init_data->bypass_cache) {
      /* Should really use ::DeviceIoCtl(IOCTL_STORAGE_QUERY_PROPERTY) on the disk containing this file. For
      now, use 4 KiB alignment, since that’s the most recent commonly used physical sector size. */
      physical_align = 4096;
   }
#else
   #error "TODO: HOST_API"
#endif
#endif
}

/*virtual*/ regular_file_stream::~regular_file_stream() {
}

/*virtual*/ offset_t regular_file_stream::seek(offset_t offset, seek_from whence) /*override*/ {
#if LOFTY_HOST_API_POSIX

   int whence_i;
   switch (whence.base()) {
      case seek_from::start:
         whence_i = SEEK_SET;
         break;
      case seek_from::current:
         whence_i = SEEK_CUR;
         break;
      case seek_from::end:
         whence_i = SEEK_END;
         break;
   }
   offset_t new_offset = ::lseek(fd.get(), offset, whence_i);
   if (new_offset < 0) {
      exception::throw_os_error();
   }
   return new_offset;

#elif LOFTY_HOST_API_WIN32 //if LOFTY_HOST_API_POSIX

   static_assert(
      sizeof(offset) == sizeof(::LARGE_INTEGER), "lofty::io::offset_t must be the same size as LARGE_INTEGER"
   );
   ::DWORD whence_i;
   switch (whence.base()) {
      case seek_from::start:
         whence_i = FILE_BEGIN;
         break;
      case seek_from::current:
         whence_i = FILE_CURRENT;
         break;
      case seek_from::end:
         whence_i = FILE_END;
         break;
   }
   ::LARGE_INTEGER new_offset;
   new_offset.QuadPart = offset;
#if _WIN32_WINNT >= 0x0500
   if (!::SetFilePointerEx(fd.get(), new_offset, &new_offset, whence_i)) {
      exception::throw_os_error();
   }
#else //if _WIN32_WINNT >= 0x0500
   new_offset.LowPart = ::SetFilePointer(fd.get(), new_offset.LowPart, &new_offset.HighPart, whence_i);
   if (new_offset.LowPart == INVALID_SET_FILE_POINTER) {
      auto err = ::GetLastError();
      if (err != ERROR_SUCCESS) {
         exception::throw_os_error(err);
      }
   }
#endif //if _WIN32_WINNT >= 0x0500 … else
   return new_offset.QuadPart;

#else //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32 … else
}

/*virtual*/ full_size_t regular_file_stream::size() const /*override*/ {
#if LOFTY_HOST_API_POSIX
   struct ::stat stat;
   if (::fstat(fd.get(), &stat)) {
      exception::throw_os_error();
   }
   return static_cast<full_size_t>(stat.st_size);
#elif LOFTY_HOST_API_WIN32
   #if _WIN32_WINNT >= 0x0500
      ::LARGE_INTEGER file_size;
      if (!::GetFileSizeEx(fd.get(), &file_size)) {
         exception::throw_os_error();
      }
      return static_cast<full_size_t>(file_size.QuadPart);
   #else
      ::DWORD file_size_high, file_size_low = ::GetFileSize(fd.get(), &file_size_high);
      if (file_size_low == INVALID_FILE_SIZE) {
         auto err = ::GetLastError();
         if (err != ERROR_SUCCESS) {
            exception::throw_os_error(err);
         }
      }
      return (static_cast<full_size_t>(file_size_high) << sizeof(file_size_low) * CHAR_BIT) | file_size_low;
   #endif
#else
   #error "TODO: HOST_API"
#endif
}

/*virtual*/ offset_t regular_file_stream::tell() const /*override*/ {
#if LOFTY_HOST_API_POSIX || LOFTY_HOST_API_WIN32
   /* Seeking 0 bytes from the current position won’t change the internal status of the file descriptor, so
   casting the const-ness away is not semantically wrong. */
   return const_cast<regular_file_stream *>(this)->seek(0, seek_from::current);
#else
   #error "TODO: HOST_API"
#endif
}

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

regular_file_istream::regular_file_istream(_pvt::file_init_data * init_data) :
   file_stream(init_data),
   regular_file_stream(init_data),
   file_istream(init_data) {
}

/*virtual*/ regular_file_istream::~regular_file_istream() {
}

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

regular_file_ostream::regular_file_ostream(_pvt::file_init_data * init_data) :
   file_stream(init_data),
   regular_file_stream(init_data),
   file_ostream(init_data) {
#if LOFTY_HOST_API_WIN32
   append = (init_data->mode == access_mode::write_append);
#endif
}

/*virtual*/ regular_file_ostream::~regular_file_ostream() {
}

#if LOFTY_HOST_API_WIN32
/*virtual*/ std::size_t regular_file_ostream::write_bytes(
   void const * src, std::size_t src_size
) /*override*/ {
   /* Emulating O_APPEND in Win32 requires a little more code: we have to manually seek to EOF, then write-
   protect the bytes we’re going to add, and then release the write protection. */

   /*! Win32 ::LockFile() / ::UnlockFile() helper.

   TODO: this will probably find use somewhere else as well, so move it to file.hxx. */
   class file_lock {
   public:
      //! Constructor.
      file_lock() :
         locked_fd(INVALID_HANDLE_VALUE) {
      }

      //! Destructor.
      ~file_lock() {
         if (locked_fd != INVALID_HANDLE_VALUE) {
            unlock();
         }
      }

      /*! Attempts to lock a range of bytes for the specified file. Returns true if a lock was acquired, false
      if it was not because of any or all of the requested bytes being locked by another process, or throws an
      exception for any other error.

      @param fd
         Open file to lock.
      @param offset
         Offset of the first byte to lock.
      @param size
         Count of bytes to lock, starting from offset.
      @return
         true if the specified range could be locked, or false if the range has already been locked.
      */
      bool lock(filedesc_t fd, offset_t offset, full_size_t size) {
         if (locked_fd != INVALID_HANDLE_VALUE) {
            unlock();
         }
         locked_fd = fd;
         range_offset.QuadPart = static_cast< ::LONGLONG>(offset);
         range_size.QuadPart = static_cast< ::LONGLONG>(size);
         if (!::LockFile(
            locked_fd, range_offset.LowPart, static_cast< ::DWORD>(range_offset.HighPart), range_size.LowPart,
            static_cast< ::DWORD>(range_size.HighPart)
         )) {
            ::DWORD err = ::GetLastError();
            if (err == ERROR_LOCK_VIOLATION) {
               return false;
            }
            exception::throw_os_error(err);
         }
         return true;
      }

      //! Releases the lock acquired by lock().
      void unlock() {
         if (!::UnlockFile(
            locked_fd, range_offset.LowPart, static_cast< ::DWORD>(range_offset.HighPart), range_size.LowPart,
            static_cast< ::DWORD>(range_size.HighPart)
         )) {
            exception::throw_os_error();
         }
      }

   private:
      //! Locked file.
      filedesc_t locked_fd;
      //! Start of the locked byte range.
      ::LARGE_INTEGER range_offset;
      //! Length of the locked byte range.
      ::LARGE_INTEGER range_size;
   };

   // The file_lock has to be in this scope, so it will unlock after the write is performed.
   file_lock write_lock;
   if (append) {
      /* In this loop, we’ll seek to EOF and try to lock the not-yet-existing bytes that we want to write to;
      if the latter fails, we’ll assume that somebody else is doing the same, so we’ll retry from the seek.

      TODO: guarantee of termination? Maybe the foreign locker won’t release the lock, ever. This is too easy
      to fool. */

      offset_t eof_offset;
      do {
         eof_offset = seek(0, seek_from::end);
      } while (!write_lock.lock(fd.get(), eof_offset, src_size));
      // Now the write can occur; the lock will be released automatically at the end.
   }

   return file_ostream::write_bytes(src, src_size);
}
#endif //if LOFTY_HOST_API_WIN32

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

regular_file_iostream::regular_file_iostream(_pvt::file_init_data * init_data) :
   file_stream(init_data),
   file_istream(init_data),
   file_ostream(init_data),
   regular_file_stream(init_data),
   file_iostream(init_data),
   regular_file_istream(init_data),
   regular_file_ostream(init_data) {
}

/*virtual*/ regular_file_iostream::~regular_file_iostream() {
}

}}} //namespace lofty::io::binary
