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

#ifndef _ABACLADE_IO_TEXT_HXX
#define _ABACLADE_IO_TEXT_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/io.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io {

//! Classes and functions to perform I/O in text mode (with encoding support).
namespace text {}

}} //namespace abc::io

////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declarations.
namespace abc { namespace io { namespace binary {

class buffered_stream;
class buffered_istream;
class buffered_ostream;

}}} //namespace abc::io::binary

namespace abc { namespace io { namespace text {

//! Base for text streams built on top of binary::buffered_stream instances.
class ABACLADE_SYM binbuf_stream : public virtual stream {
public:
   //! Destructor.
   virtual ~binbuf_stream();

   /*! Returns a pointer to the underlying buffered binary stream.

   @return
      Pointer to a buffered binary stream.
   */
   _std::shared_ptr<binary::buffered_stream> binary_buffered() const {
      return _binary_buffered_stream();
   }

   //! See base::get_encoding().
   virtual abc::text::encoding get_encoding() const override;

protected:
   /*! Constructor.

   @param enc
      Initial value for get_encoding().
   */
   binbuf_stream(abc::text::encoding enc);

   /*! Implementation of binary_buffered(). This enables binary_buffered() to be non-virtual, which
   in turn allows derived classes to override it changing its return type to be more specific.

   @return
      Pointer to a buffered binary stream.
   */
   virtual _std::shared_ptr<binary::buffered_stream> _binary_buffered_stream() const = 0;

protected:
   /*! Encoding used for I/O to/from the underlying buffered_stream. If not explicitly set, it will
   be automatically determined and assigned on the first read or write. */
   abc::text::encoding m_enc;
};

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

/*! Implementation of a text (character-based) input stream on top of a binary::buffered_istream
instance. */
class ABACLADE_SYM binbuf_istream : public virtual binbuf_stream, public virtual istream {
public:
   /*! Constructor.

   @param pbbis
      Pointer to a binary buffered input stream to read from.
   @param enc
      Initial value for get_encoding(). If omitted, an encoding will be automatically detected
      (guessed) on the first read from the underlying binary istream.
   */
   explicit binbuf_istream(
      _std::shared_ptr<binary::buffered_istream> pbbis,
      abc::text::encoding enc = abc::text::encoding::unknown
   );

   //! Destructor.
   virtual ~binbuf_istream();

   //! See binbuf_stream::binary_buffered().
   _std::shared_ptr<binary::buffered_istream> binary_buffered() const {
      return m_pbbis;
   }

   //! See istream::consume_chars().
   virtual void consume_chars(std::size_t cch) override;

   //! See istream::peek_chars().
   virtual str peek_chars(std::size_t cchMin) override;

   // Pull in the other overload to avoid hiding it.
   using istream::read_all;

   //! See istream::read_line().
   virtual bool read_line(str * psDst) override;

protected:
   //! See binbuf_stream::_binary_buffered_stream().
   virtual _std::shared_ptr<binary::buffered_stream> _binary_buffered_stream() const override;

private:
   /*! Detects the encoding used in the provided buffer.

   @param pb
      Pointer to a buffer with the initial contents of the file.
   @param cb
      Size of the buffer pointed to by pb.
   @return
      Size of the BOM, if found in the source. If non-zero, the caller should discard this many
      bytes from the provided buffer.
   */
   std::size_t detect_encoding(std::uint8_t const * pb, std::size_t cb);

protected:
   //! Underlying binary buffered input stream.
   _std::shared_ptr<binary::buffered_istream> m_pbbis;

private:
   //! Buffer backing the string returned by peek_chars().
   str m_sPeekBuf;
   /*! First character index of the view into m_sPeekBuf returned by peek_chars(). Contents of
   m_sPeekBuf before this index have already been consumed, but are kept in it to avoid having to
   shift its contents on every call to consume_chars(). */
   std::size_t m_ichPeekBufOffset;
   //! true if a past call to peek_chars() got to EOF.
   bool m_bEOF:1;
};

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

/*! Implementation of a text (character-based) output stream on top of a binary::buffered_ostream
instance. */
class ABACLADE_SYM binbuf_ostream : public virtual binbuf_stream, public virtual ostream {
public:
   /*! Constructor.

   @param pbbos
      Pointer to a binary buffered output stream to write to.
   @param enc
      Initial value for get_encoding(). If omitted and never explicitly set, on the first write it
      will default to abc::text::encoding::utf8.
   */
   explicit binbuf_ostream(
      _std::shared_ptr<binary::buffered_ostream> pbbos,
      abc::text::encoding enc = abc::text::encoding::unknown
   );

   //! Destructor.
   virtual ~binbuf_ostream();

   //! See binbuf_stream::binary_buffered().
   _std::shared_ptr<binary::buffered_ostream> binary_buffered() const {
      return m_pbbos;
   }

   //! See ostream::finalize().
   virtual void finalize() override;

   //! See ostream::flush().
   virtual void flush() override;

   //! See ostream::write_binary().
   virtual void write_binary(
      void const * pSrc, std::size_t cbSrc, abc::text::encoding enc
   ) override;

protected:
   //! See binbuf_stream::_binary_buffered_stream().
   virtual _std::shared_ptr<binary::buffered_stream> _binary_buffered_stream() const override;

protected:
   //! Underlying binary buffered output stream.
   _std::shared_ptr<binary::buffered_ostream> m_pbbos;
};

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declarations.
namespace abc { namespace io { namespace binary {

class istream;
class ostream;

}}} //namespace abc::io::binary

namespace abc { namespace os {

class path;

}} //namespace abc::os

namespace abc { namespace io { namespace text {

//! Text stream associated to the standard error output file.
extern ABACLADE_SYM _std::shared_ptr<ostream> stderr;
//! Text stream associated to the standard input file.
extern ABACLADE_SYM _std::shared_ptr<istream> stdin;
//! Text stream associated to the standard output file.
extern ABACLADE_SYM _std::shared_ptr<ostream> stdout;

/*! Creates and returns a text input stream for the specified binary input stream.

@param pbis
   Pointer to a binary input stream.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text input stream operating on top of the specified binary input stream.
*/
ABACLADE_SYM _std::shared_ptr<binbuf_istream> make_istream(
   _std::shared_ptr<binary::istream> pbis, abc::text::encoding enc = abc::text::encoding::unknown
);

/*! Creates and returns a text output stream for the specified binary output stream.

@param pbos
   Pointer to a binary output stream.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text output stream operating on top of the specified binary output stream.
*/
ABACLADE_SYM _std::shared_ptr<binbuf_ostream> make_ostream(
   _std::shared_ptr<binary::ostream> pbos, abc::text::encoding enc = abc::text::encoding::unknown
);

/*! Opens a file for text-mode reading.

@param op
   Path to the file.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text input stream for the file.
*/
ABACLADE_SYM _std::shared_ptr<binbuf_istream> open_istream(
   os::path const & op, abc::text::encoding enc = abc::text::encoding::unknown
);

/*! Opens a file for text-mode writing.

@param op
   Path to the file.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text output stream for the file.
*/
ABACLADE_SYM _std::shared_ptr<binbuf_ostream> open_ostream(
   os::path const & op, abc::text::encoding enc = abc::text::encoding::utf8
);

}}} //namespace abc::io::text

namespace abc { namespace io { namespace text { namespace _pvt {

/*! Creates and returns a text stream associated to the standard error output file (stderr).

@return
   Standard error file.
*/
ABACLADE_SYM _std::shared_ptr<ostream> make_stderr();

/*! Creates and returns a text stream associated to the standard input file (stdin).

@return
   Standard input file.
*/
ABACLADE_SYM _std::shared_ptr<istream> make_stdin();

/*! Creates and returns a text stream associated to the standard output file (stdout).

@return
   Standard output file.
*/
ABACLADE_SYM _std::shared_ptr<ostream> make_stdout();

}}}} //namespace abc::io::text::_pvt

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_IO_TEXT_HXX
