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

#ifndef _ABACLADE_IO_BINARY_FILE_SUBCLASSES_HXX
#define _ABACLADE_IO_BINARY_FILE_SUBCLASSES_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/io/binary.hxx>
#if ABC_HOST_API_WIN32
   #include <abaclade/text/parsers/ansi_escape_sequences.hxx>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Base for terminal/console binary streams.
class ABACLADE_SYM tty_file_stream : public virtual file_stream {
public:
   //! Destructor.
   virtual ~tty_file_stream();

protected:
   //! See file_stream::file_stream().
   tty_file_stream(detail::file_init_data * pfid);
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Terminal/console input stream.
class ABACLADE_SYM tty_istream :
   public virtual tty_file_stream,
   public virtual file_istream {
public:
   //! See file_istream::file_istream().
   tty_istream(detail::file_init_data * pfid);

   //! Destructor.
   virtual ~tty_istream();

#if ABC_HOST_API_WIN32
   /* Under Win32, console files must use a dedicated API in order to support the native character
   type. */

   //! See file_istream::read().
   virtual std::size_t read(void * p, std::size_t cbMax) override;
#endif //if ABC_HOST_API_WIN32
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Terminal/console output stream.
class ABACLADE_SYM tty_ostream :
   public virtual tty_file_stream,
   public virtual file_ostream
#if ABC_HOST_API_WIN32
   // Under Win32, ANSI escape sequences parsing is up to us.
   , private abc::text::parsers::ansi_escape_sequences
#endif
   {
public:
   //! See file_ostream::file_ostream().
   tty_ostream(detail::file_init_data * pfid);

   //! Destructor.
   virtual ~tty_ostream();

#if ABC_HOST_API_WIN32
   /* Under Win32, console files must use a dedicated API in order to support the native character
   type. */

   //! See file_ostream::write().
   virtual std::size_t write(void const * p, std::size_t cb) override;

private:
   //! See abc::text::parsers::ansi_escape_sequences::clear_display_area().
   virtual void clear_display_area(std::int16_t iRow, std::int16_t iCol, std::size_t cch) override;

   //! See abc::text::parsers::ansi_escape_sequences::get_cursor_pos_and_display_size().
   virtual void get_cursor_pos_and_display_size(
      std::int16_t * piRow, std::int16_t * piCol, std::int16_t * pcRows, std::int16_t * pcCols
   ) override;

   /* Determines whether output processing is enabled for the console pseudo-file.

   @return
      true if the bytes written are to be parsed for special characters, or false otherwise.
   */
   bool processing_enabled() const;

   //! See abc::text::parsers::ansi_escape_sequences::scroll_text().
   virtual void scroll_text(std::int16_t cRows, std::int16_t cCols) override;

   //! See abc::text::parsers::ansi_escape_sequences::set_char_attributes().
   virtual void set_char_attributes() override;

   //! See abc::text::parsers::ansi_escape_sequences::set_cursor_pos().
   virtual void set_cursor_pos(std::int16_t iRow, std::int16_t iCol) override;

   //! See abc::text::parsers::ansi_escape_sequences::set_cursor_visibility().
   virtual void set_cursor_visibility(bool bVisible) override;

   //! See abc::text::parsers::ansi_escape_sequences::set_window_title().
   virtual void set_window_title(str const & sTitle) override;

   /*! Writes a range of characters directly to the console, without any parsing.

   @param pchBegin
      Start of the character array to write.
   @param pchEnd
      End of the character array to write.
   */
   void write_range(char_t const * pchBegin, char_t const * pchEnd) const;

private:
   //! Mapping table from ANSI terminal colors to Win32 console background colors.
   static ::WORD const smc_aiAnsiColorToBackgroundColor[];
   //! Mapping table from ANSI terminal colors to Win32 console foreground colors.
   static ::WORD const smc_aiAnsiColorToForegroundColor[];
#endif //if ABC_HOST_API_WIN32
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Bidirectional terminal/console stream.
class ABACLADE_SYM tty_iostream :
   public file_iostream,
   public tty_istream,
   public tty_ostream {
public:
   //! See tty_istream::tty_istream() and tty_ostream::tty_ostream().
   tty_iostream(detail::file_init_data * pfid);

   //! Destructor.
   virtual ~tty_iostream();
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Binary stream for the read end of a pipe.
class ABACLADE_SYM pipe_istream : public virtual file_istream {
public:
   //! See file_istream::file_istream().
   pipe_istream(detail::file_init_data * pfid);

   //! Destructor.
   virtual ~pipe_istream();

protected:
#if ABC_HOST_API_WIN32
   /*! See file_istream::check_if_eof_or_throw_os_error(). Pipes report EOF in a completely different
   way than regular files. */
   virtual bool check_if_eof_or_throw_os_error(::DWORD cbRead, ::DWORD iErr) const override;
#endif
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Binary output stream for the write end of a pipe.
class ABACLADE_SYM pipe_ostream : public virtual file_ostream {
public:
   //! See file_ostream::file_ostream().
   pipe_ostream(detail::file_init_data * pfid);

   //! Destructor.
   virtual ~pipe_ostream();
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Bidirectional pipe end.
class ABACLADE_SYM pipe_iostream :
   public file_iostream,
   public pipe_istream,
   public pipe_ostream {
public:
   //! See pipe_istream::pipe_istream() and pipe_ostream::pipe_ostream().
   pipe_iostream(detail::file_init_data * pfid);

   //! Destructor.
   virtual ~pipe_iostream();
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Base for binary streams for regular disk files.
class ABACLADE_SYM regular_file_stream : public virtual file_stream, public seekable, public sized {
public:
   //! Destructor.
   virtual ~regular_file_stream();

   //! See seekable::seek().
   virtual offset_t seek(offset_t ibOffset, seek_from sfWhence) override;

   //! See sized::size().
   virtual full_size_t size() const override;

   //! See seekable::tell().
   virtual offset_t tell() const override;

protected:
   //! See file_stream::file_stream().
   regular_file_stream(detail::file_init_data * pfid);

protected:
   //! Size of the file.
   full_size_t m_cb;
#if 0
   //! Physical alignment for unbuffered/direct disk access.
   unsigned m_cbPhysAlign;
#endif
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Binary input stream for regular disk files.
class ABACLADE_SYM regular_file_istream :
   public virtual regular_file_stream,
   public virtual file_istream {
public:
   //! See regular_file_stream().
   regular_file_istream(detail::file_init_data * pfid);

   //! Destructor.
   virtual ~regular_file_istream();
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Binary output stream for regular disk files.
class ABACLADE_SYM regular_file_ostream :
   public virtual regular_file_stream,
   public virtual file_ostream {
public:
   //! See regular_file_stream().
   regular_file_ostream(detail::file_init_data * pfid);

   //! Destructor.
   virtual ~regular_file_ostream();

#if ABC_HOST_API_WIN32
   //! See file_ostream::write(). This override is necessary to emulate O_APPEND under Win32.
   virtual std::size_t write(void const * p, std::size_t cb) override;

protected:
   //! If true, write() will emulate POSIX’s O_APPEND in platforms that don’t support it.
   bool m_bAppend:1;
#endif
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Bidirectional file.
class ABACLADE_SYM regular_file_iostream :
   public file_iostream,
   public regular_file_istream,
   public regular_file_ostream {
public:
   /*! See regular_file_istream::regular_file_istream() and
   regular_file_ostream::regular_file_ostream(). */
   regular_file_iostream(detail::file_init_data * pfid);

   //! Destructor.
   virtual ~regular_file_iostream();
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_IO_BINARY_FILE_SUBCLASSES_HXX
