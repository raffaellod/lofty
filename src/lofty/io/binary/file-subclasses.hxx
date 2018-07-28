/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_IO_BINARY_FILE_SUBCLASSES_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_IO_BINARY_FILE_SUBCLASSES_HXX
#endif

#ifndef _LOFTY_IO_BINARY_FILE_SUBCLASSES_HXX_NOPUB
#define _LOFTY_IO_BINARY_FILE_SUBCLASSES_HXX_NOPUB

#include <lofty/io/binary.hxx>
#if LOFTY_HOST_API_WIN32
   #include <lofty/text/parsers/ansi_escape_sequences.hxx>
   #include <lofty/text/str.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary { namespace _pub {

//! Base for terminal/console binary streams.
class LOFTY_SYM tty_file_stream : public virtual file_stream {
public:
   //! Destructor.
   virtual ~tty_file_stream();

protected:
   //! See file_stream::file_stream().
   tty_file_stream(_pvt::file_init_data * init_data);
};

}}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary { namespace _pub {

//! Terminal/console input stream.
class LOFTY_SYM tty_istream : public virtual tty_file_stream, public virtual file_istream {
public:
   //! See file_istream::file_istream().
   tty_istream(_pvt::file_init_data * init_data);

   //! Destructor.
   virtual ~tty_istream();

#if LOFTY_HOST_API_WIN32
   // Under Win32, console files must use a dedicated API in order to support the native character type.

   //! See file_istream::read_bytes().
   virtual std::size_t read_bytes(void * dst, std::size_t dst_max) override;
#endif //if LOFTY_HOST_API_WIN32
};

}}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary { namespace _pub {

//! Terminal/console output stream.
class LOFTY_SYM tty_ostream :
   public virtual tty_file_stream,
   public virtual file_ostream
#if LOFTY_HOST_API_WIN32
   // Under Win32, ANSI escape sequences parsing is up to us.
   , private lofty::text::parsers::_LOFTY_PUBNS ansi_escape_sequences
#endif
   {
public:
   //! See file_ostream::file_ostream().
   tty_ostream(_pvt::file_init_data * init_data);

   //! Destructor.
   virtual ~tty_ostream();

#if LOFTY_HOST_API_WIN32
   /*! See file_ostream::flush(). Overridden because FlushFileBuffers() fails with console files, which are
   unbuffered. */
   virtual void flush() override;

   /*! See file_ostream::write_bytes(). Overridden because under Win32, console files must use a dedicated API
   in order to support the native character type. */
   virtual std::size_t write_bytes(void const * src, std::size_t src_size) override;

private:
   //! See lofty::text::parsers::ansi_escape_sequences::clear_display_area().
   virtual void clear_display_area(std::int16_t row, std::int16_t col, std::size_t char_size) override;

   //! See lofty::text::parsers::ansi_escape_sequences::get_cursor_pos_and_display_size().
   virtual void get_cursor_pos_and_display_size(
      std::int16_t * row, std::int16_t * col, std::int16_t * rows, std::int16_t * cols
   ) override;

   /* Determines whether output processing is enabled for the console pseudo-file.

   @return
      true if the bytes written are to be parsed for special characters, or false otherwise.
   */
   bool processing_enabled() const;

   //! See lofty::text::parsers::ansi_escape_sequences::scroll_text().
   virtual void scroll_text(std::int16_t rows, std::int16_t cols) override;

   //! See lofty::text::parsers::ansi_escape_sequences::set_char_attributes().
   virtual void set_char_attributes() override;

   //! See lofty::text::parsers::ansi_escape_sequences::set_cursor_pos().
   virtual void set_cursor_pos(std::int16_t row, std::int16_t col) override;

   //! See lofty::text::parsers::ansi_escape_sequences::set_cursor_visibility().
   virtual void set_cursor_visibility(bool visible) override;

   //! See lofty::text::parsers::ansi_escape_sequences::set_window_title().
   virtual void set_window_title(lofty::text::_LOFTY_PUBNS str const & title) override;

   /*! Writes a range of characters directly to the console, without any parsing.

   @param src_begin
      Start of the character array to write.
   @param src_end
      End of the character array to write.
   */
   void write_range(
      lofty::text::_LOFTY_PUBNS char_t const * src_begin, lofty::text::_LOFTY_PUBNS char_t const * src_end
   ) const;
#endif //if LOFTY_HOST_API_WIN32
};

}}}} //namespace lofty::io::binary::_pub

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary { namespace _pub {

//! Bidirectional terminal/console stream.
class LOFTY_SYM tty_iostream : public file_iostream, public tty_istream, public tty_ostream {
public:
   //! See tty_istream::tty_istream() and tty_ostream::tty_ostream().
   tty_iostream(_pvt::file_init_data * init_data);

   //! Destructor.
   virtual ~tty_iostream();
};

}}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary { namespace _pub {

//! Binary stream for the read end of a pipe.
class LOFTY_SYM pipe_istream : public virtual file_istream {
public:
   //! See file_istream::file_istream().
   pipe_istream(_pvt::file_init_data * init_data);

   //! Destructor.
   virtual ~pipe_istream();

protected:
#if LOFTY_HOST_API_WIN32
   /*! See file_istream::check_if_eof_or_throw_os_error(). Pipes report EOF in a completely different way than
   regular files. */
   virtual bool check_if_eof_or_throw_os_error(::DWORD bytes_read, ::DWORD err) const override;
#endif
};

}}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary { namespace _pub {

//! Binary output stream for the write end of a pipe.
class LOFTY_SYM pipe_ostream : public virtual file_ostream {
public:
   //! See file_ostream::file_ostream().
   pipe_ostream(_pvt::file_init_data * init_data);

   //! Destructor.
   virtual ~pipe_ostream();
};

}}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary { namespace _pub {

//! Bidirectional pipe end.
class LOFTY_SYM pipe_iostream : public file_iostream, public pipe_istream, public pipe_ostream {
public:
   //! See pipe_istream::pipe_istream() and pipe_ostream::pipe_ostream().
   pipe_iostream(_pvt::file_init_data * init_data);

   //! Destructor.
   virtual ~pipe_iostream();
};

}}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary { namespace _pub {

//! Base for binary streams for regular disk files.
class LOFTY_SYM regular_file_stream : public virtual file_stream, public seekable, public sized {
public:
   //! Destructor.
   virtual ~regular_file_stream();

   //! See seekable::seek().
   virtual io::_LOFTY_PUBNS offset_t seek(
      io::_LOFTY_PUBNS offset_t offset, io::_LOFTY_PUBNS seek_from whence
   ) override;

   //! See sized::size().
   virtual io::_LOFTY_PUBNS full_size_t size() const override;

   //! See seekable::tell().
   virtual io::_LOFTY_PUBNS offset_t tell() const override;

protected:
   //! See file_stream::file_stream().
   regular_file_stream(_pvt::file_init_data * init_data);

protected:
#if 0
   //! Physical alignment for unbuffered/direct disk access.
   unsigned physical_align;
#endif
};

}}}} //namespace lofty::io::binary::_pub

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary { namespace _pub {

//! Binary input stream for regular disk files.
class LOFTY_SYM regular_file_istream : public virtual regular_file_stream, public virtual file_istream {
public:
   //! See regular_file_stream().
   regular_file_istream(_pvt::file_init_data * init_data);

   //! Destructor.
   virtual ~regular_file_istream();
};

}}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary { namespace _pub {

//! Binary output stream for regular disk files.
class LOFTY_SYM regular_file_ostream : public virtual regular_file_stream, public virtual file_ostream {
public:
   //! See regular_file_stream().
   regular_file_ostream(_pvt::file_init_data * init_data);

   //! Destructor.
   virtual ~regular_file_ostream();

#if LOFTY_HOST_API_WIN32
   //! See file_ostream::write_bytes(). This override is necessary to emulate O_APPEND under Win32.
   virtual std::size_t write_bytes(void const * src, std::size_t src_size) override;

protected:
   //! If true, write_bytes() will emulate POSIX’s O_APPEND in platforms that don’t support it.
   bool append:1;
#endif
};

}}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary { namespace _pub {

//! Bidirectional file.
class LOFTY_SYM regular_file_iostream :
   public file_iostream,
   public regular_file_istream,
   public regular_file_ostream {
public:
   //! See regular_file_istream::regular_file_istream() and regular_file_ostream::regular_file_ostream().
   regular_file_iostream(_pvt::file_init_data * init_data);

   //! Destructor.
   virtual ~regular_file_iostream();
};

}}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_IO_BINARY_FILE_SUBCLASSES_HXX_NOPUB

#ifdef _LOFTY_IO_BINARY_FILE_SUBCLASSES_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace io { namespace binary {

   using _pub::tty_file_stream;
   using _pub::tty_istream;
   using _pub::tty_ostream;
   using _pub::tty_iostream;
   using _pub::pipe_istream;
   using _pub::pipe_ostream;
   using _pub::pipe_iostream;
   using _pub::regular_file_stream;
   using _pub::regular_file_istream;
   using _pub::regular_file_ostream;
   using _pub::regular_file_iostream;

   }}}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_IO_BINARY_FILE_SUBCLASSES_HXX
