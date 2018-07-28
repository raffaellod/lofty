/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_IO_TEXT_HXX

#include <lofty/io/text-0.hxx>

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_IO_TEXT_HXX
#endif

#ifndef _LOFTY_IO_TEXT_HXX_NOPUB
#define _LOFTY_IO_TEXT_HXX_NOPUB

#include <lofty/io.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io {

//! Classes and functions to perform I/O in text mode (with encoding support).
namespace text {}

}} //namespace lofty::io

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declarations.
namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

class buffered_stream;
class buffered_istream;
class buffered_ostream;

_LOFTY_PUBNS_END
}}}

namespace lofty { namespace io { namespace text {
_LOFTY_PUBNS_BEGIN

//! Base for text streams built on top of binary::buffered_stream instances.
class LOFTY_SYM binbuf_stream : public virtual stream {
public:
   //! Destructor.
   virtual ~binbuf_stream();

   /*! Returns a pointer to the underlying buffered binary stream.

   @return
      Pointer to a buffered binary stream.
   */
   _std::_LOFTY_PUBNS shared_ptr<binary::_LOFTY_PUBNS buffered_stream> binary_buffered() const {
      return _binary_buffered_stream();
   }

   //! See base::get_encoding().
   virtual lofty::text::_LOFTY_PUBNS encoding get_encoding() const override;

protected:
   /*! Constructor.

   @param enc
      Initial value for get_encoding().
   */
   binbuf_stream(lofty::text::_LOFTY_PUBNS encoding enc);

   /*! Implementation of binary_buffered(). This enables binary_buffered() to be non-virtual, which in turn
   allows derived classes to override it changing its return type to be more specific.

   @return
      Pointer to a buffered binary stream.
   */
   virtual _std::_LOFTY_PUBNS shared_ptr<
      binary::_LOFTY_PUBNS buffered_stream
   > _binary_buffered_stream() const = 0;

protected:
   /*! Encoding used for I/O to/from the underlying buffered_stream. If not explicitly set, it will be
   automatically determined and assigned on the first read or write. */
   lofty::text::_LOFTY_PUBNS encoding default_enc;
};

_LOFTY_PUBNS_END
}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {
_LOFTY_PUBNS_BEGIN

//! Implementation of a text (character-based) input stream on top of a binary::buffered_istream instance.
class LOFTY_SYM binbuf_istream : public virtual binbuf_stream, public virtual istream {
public:
   /*! Constructor.

   @param buf_bin_istream
      Pointer to a binary buffered input stream to read from.
   @param enc
      Initial value for get_encoding(). If omitted, an encoding will be automatically detected (guessed) on
      the first read from the underlying binary istream.
   */
   explicit binbuf_istream(
      _std::_LOFTY_PUBNS shared_ptr<binary::_LOFTY_PUBNS buffered_istream> buf_bin_istream,
      lofty::text::_LOFTY_PUBNS encoding enc = lofty::text::_LOFTY_PUBNS encoding::unknown
   );

   //! Destructor.
   virtual ~binbuf_istream();

   //! See binbuf_stream::binary_buffered().
   _std::_LOFTY_PUBNS shared_ptr<binary::_LOFTY_PUBNS buffered_istream> binary_buffered() const {
      return buf_bin_istream;
   }

   //! See istream::consume_chars().
   virtual void consume_chars(std::size_t count) override;

   //! See istream::peek_chars().
   virtual lofty::text::_LOFTY_PUBNS str peek_chars(std::size_t count_min) override;

   // Pull in the other overload to avoid hiding it.
   using istream::read_all;

   //! See istream::read_line().
   virtual bool read_line(lofty::text::_LOFTY_PUBNS str * dst) override;

   //! See istream::unconsume_chars().
   virtual void unconsume_chars(lofty::text::_LOFTY_PUBNS str const & s) override;

protected:
   //! See binbuf_stream::_binary_buffered_stream().
   virtual _std::_LOFTY_PUBNS shared_ptr<
      binary::_LOFTY_PUBNS buffered_stream
   > _binary_buffered_stream() const override;

private:
   /*! Detects the encoding used in the provided buffer.

   @param buf
      Pointer to a buffer with the initial contents of the file.
   @param buf_byte_size
      Size of the buffer pointed to by buf.
   @return
      Size of the BOM, if found in the source. If non-zero, the caller should discard this many bytes from the
      provided buffer.
   */
   std::size_t detect_encoding(std::uint8_t const * buf, std::size_t buf_byte_size);

protected:
   //! Underlying binary buffered input stream.
   _std::_LOFTY_PUBNS shared_ptr<binary::_LOFTY_PUBNS buffered_istream> buf_bin_istream;

private:
   //! Buffer backing the string returned by peek_chars().
   lofty::text::_LOFTY_PUBNS str peek_buf;
   /*! First character index of the view into peek_buf returned by peek_chars(). Contents of peek_buf before
   this index have already been consumed, but are kept in it to avoid having to shift its contents on every
   call to consume_chars(). */
   std::size_t peek_buf_char_offset;
   //! true if a past call to peek_chars() got to EOF.
   bool eof:1;
};

_LOFTY_PUBNS_END
}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {
_LOFTY_PUBNS_BEGIN

//! Implementation of a text (character-based) output stream on top of a binary::buffered_ostream instance.
class LOFTY_SYM binbuf_ostream :
   public virtual binbuf_stream,
   public virtual ostream,
   public io::_LOFTY_PUBNS closeable {
public:
   /*! Constructor.

   @param buf_bin_ostream
      Pointer to a binary buffered output stream to write to.
   @param enc
      Initial value for get_encoding(). If omitted and never explicitly set, on the first write it will
      default to lofty::text::encoding::utf8.
   */
   explicit binbuf_ostream(
      _std::_LOFTY_PUBNS shared_ptr<binary::_LOFTY_PUBNS buffered_ostream> buf_bin_ostream,
      lofty::text::_LOFTY_PUBNS encoding enc = lofty::text::_LOFTY_PUBNS encoding::unknown
   );

   //! Destructor.
   virtual ~binbuf_ostream();

   //! See binbuf_stream::binary_buffered().
   _std::_LOFTY_PUBNS shared_ptr<binary::_LOFTY_PUBNS buffered_ostream> binary_buffered() const {
      return buf_bin_ostream;
   }

   //! See closeable::close().
   virtual void close() override;

   //! See ostream::flush().
   virtual void flush() override;

   //! See ostream::write_binary().
   virtual void write_binary(
      void const * src, std::size_t src_byte_size, lofty::text::_LOFTY_PUBNS encoding enc
   ) override;

protected:
   //! See binbuf_stream::_binary_buffered_stream().
   virtual _std::_LOFTY_PUBNS shared_ptr<
      binary::_LOFTY_PUBNS buffered_stream
   > _binary_buffered_stream() const override;

protected:
   //! Underlying binary buffered output stream.
   _std::_LOFTY_PUBNS shared_ptr<binary::_LOFTY_PUBNS buffered_ostream> buf_bin_ostream;
};

_LOFTY_PUBNS_END
}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declarations.
namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

class istream;
class ostream;

_LOFTY_PUBNS_END
}}}

namespace lofty { namespace os {
_LOFTY_PUBNS_BEGIN

class path;

_LOFTY_PUBNS_END
}}

namespace lofty { namespace io { namespace text {
_LOFTY_PUBNS_BEGIN

//! Text stream associated to the standard error output file.
extern LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<ostream> stderr;
//! Text stream associated to the standard input file.
extern LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<istream> stdin;
//! Text stream associated to the standard output file.
extern LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<ostream> stdout;

/*! Creates and returns a text input stream for the specified binary input stream.

@param bin_istream
   Pointer to a binary input stream.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text input stream operating on top of the specified binary input stream.
*/
LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<binbuf_istream> make_istream(
   _std::_LOFTY_PUBNS shared_ptr<binary::_LOFTY_PUBNS istream> bin_istream,
   lofty::text::_LOFTY_PUBNS encoding enc = lofty::text::_LOFTY_PUBNS encoding::unknown
);

/*! Creates and returns a text output stream for the specified binary output stream.

@param bin_ostream
   Pointer to a binary output stream.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text output stream operating on top of the specified binary output stream.
*/
LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<binbuf_ostream> make_ostream(
   _std::_LOFTY_PUBNS shared_ptr<binary::_LOFTY_PUBNS ostream> bin_ostream,
   lofty::text::_LOFTY_PUBNS encoding enc = lofty::text::_LOFTY_PUBNS encoding::unknown
);

/*! Opens a file for text-mode reading.

@param path
   Path to the file.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text input stream for the file.
*/
LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<binbuf_istream> open_istream(
   os::_LOFTY_PUBNS path const & path,
   lofty::text::_LOFTY_PUBNS encoding enc = lofty::text::_LOFTY_PUBNS encoding::unknown
);

/*! Opens a file for text-mode writing.

@param path
   Path to the file.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text output stream for the file.
*/
LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<binbuf_ostream> open_ostream(
   os::_LOFTY_PUBNS path const & path,
   lofty::text::_LOFTY_PUBNS encoding enc = lofty::text::_LOFTY_PUBNS encoding::utf8
);

_LOFTY_PUBNS_END
}}} //namespace lofty::io::text

namespace lofty { namespace io { namespace text { namespace _pvt {

/*! Creates and returns a text stream associated to the standard error output file (stderr).

@return
   Standard error file.
*/
LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<_LOFTY_PUBNS ostream> make_stderr();

/*! Creates and returns a text stream associated to the standard input file (stdin).

@return
   Standard input file.
*/
LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<_LOFTY_PUBNS istream> make_stdin();

/*! Creates and returns a text stream associated to the standard output file (stdout).

@return
   Standard output file.
*/
LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<_LOFTY_PUBNS ostream> make_stdout();

}}}} //namespace lofty::io::text::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_IO_TEXT_HXX_NOPUB

#ifdef _LOFTY_IO_TEXT_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace io { namespace text {

   using _pub::binbuf_istream;
   using _pub::binbuf_ostream;
   using _pub::binbuf_stream;
   using _pub::make_istream;
   using _pub::make_ostream;
   using _pub::open_istream;
   using _pub::open_ostream;
   using _pub::stderr;
   using _pub::stdin;
   using _pub::stdout;

   }}}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_IO_TEXT_HXX
