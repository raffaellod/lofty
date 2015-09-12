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

// Forward declarations.

namespace abc { namespace io { namespace binary {

class reader;
class writer;
class buffered_base;
class buffered_reader;
class buffered_writer;

}}} //namespace abc::io::binary

namespace abc { namespace io { namespace text {

class reader;
class writer;
class binbuf_reader;
class binbuf_writer;

//! Text writer associated to the standard error output file.
extern ABACLADE_SYM _std::shared_ptr<writer> stderr;
//! Text reader associated to the standard input file.
extern ABACLADE_SYM _std::shared_ptr<reader> stdin;
//! Text writer associated to the standard output file.
extern ABACLADE_SYM _std::shared_ptr<writer> stdout;

/*! Creates and returns a text reader for the specified binary reader.

@param pbr
   Pointer to a binary reader.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text reader operating on top of the specified binary reader.
*/
ABACLADE_SYM _std::shared_ptr<binbuf_reader> make_reader(
   _std::shared_ptr<binary::reader> pbr, abc::text::encoding enc = abc::text::encoding::unknown
);

/*! Creates and returns a text writer for the specified binary writer.

@param pbw
   Pointer to a binary writer.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text writer operating on top of the specified binary writer.
*/
ABACLADE_SYM _std::shared_ptr<binbuf_writer> make_writer(
   _std::shared_ptr<binary::writer> pbw, abc::text::encoding enc = abc::text::encoding::unknown
);

/*! Opens a file for text-mode reading.

@param op
   Path to the file.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text reader for the file.
*/
ABACLADE_SYM _std::shared_ptr<binbuf_reader> open_reader(
   os::path const & op, abc::text::encoding enc = abc::text::encoding::unknown
);

/*! Opens a file for text-mode writing.

@param op
   Path to the file.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text writer for the file.
*/
ABACLADE_SYM _std::shared_ptr<binbuf_writer> open_writer(
   os::path const & op, abc::text::encoding enc = abc::text::encoding::utf8
);

}}} //namespace abc::io::text

namespace abc { namespace io { namespace text { namespace detail {

/*! Creates and returns a text writer associated to the standard error output file (stderr).

@return
   Standard error file.
*/
ABACLADE_SYM _std::shared_ptr<writer> make_stderr();

/*! Creates and returns a text reader associated to the standard input file (stdin).

@return
   Standard input file.
*/
ABACLADE_SYM _std::shared_ptr<reader> make_stdin();

/*! Creates and returns a text writer associated to the standard output file (stdout).

@return
   Standard output file.
*/
ABACLADE_SYM _std::shared_ptr<writer> make_stdout();

}}}} //namespace abc::io::text::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

//! Base for text I/O classes built on top of a binary::buffered_base instance.
class ABACLADE_SYM binbuf_base : public virtual base {
public:
   //! Destructor.
   virtual ~binbuf_base();

   /*! Returns a pointer to the underlying buffered binary I/O object.

   @return
      Pointer to a buffered binary I/O object.
   */
   _std::shared_ptr<binary::buffered_base> binary_buffered() const {
      return _binary_buffered_base();
   }

   //! See base::get_encoding().
   virtual abc::text::encoding get_encoding() const override;

protected:
   /*! Constructor.

   @param enc
      Initial value for get_encoding().
   */
   binbuf_base(abc::text::encoding enc);

   /*! Implementation of binary_buffered(). This enables binary_buffered() to be non-virtual, which
   in turn allows derived classes to override it changing its return type to be more specific.

   @return
      Pointer to a buffered binary I/O object.
   */
   virtual _std::shared_ptr<binary::buffered_base> _binary_buffered_base() const = 0;

protected:
   /*! Encoding used for I/O to/from the underlying buffered_base. If not explicitly set, it will be
   automatically determined and assigned on the first read or write. */
   abc::text::encoding m_enc;
};

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

//! Implementation of a text (character-based) reader on top of a binary::buffered_reader instance.
class ABACLADE_SYM binbuf_reader : public virtual binbuf_base, public virtual reader {
private:
   //! Finite-state automaton implementing much of read_line_or_all().
   class read_helper;

public:
   /*! Constructor.

   @param pbbr
      Pointer to a binary buffered reader to work with.
   @param enc
      Initial value for get_encoding(). If omitted, an encoding will be automatically detected
      (guessed) on the first read from the underlying binary reader.
   */
   explicit binbuf_reader(
      _std::shared_ptr<binary::buffered_reader> pbbr,
      abc::text::encoding enc = abc::text::encoding::unknown
   );

   //! Destructor.
   virtual ~binbuf_reader();

   //! See binbuf_base::binary_buffered().
   _std::shared_ptr<binary::buffered_reader> binary_buffered() const {
      return m_pbbr;
   }

protected:
   //! See binbuf_base::_binary_buffered_base().
   virtual _std::shared_ptr<binary::buffered_base> _binary_buffered_base() const override;

   //! See reader::read_line_or_all().
   virtual bool read_line_or_all(str * psDst, bool bOneLine) override;

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
   //! Underlying binary buffered reader.
   _std::shared_ptr<binary::buffered_reader> m_pbbr;

private:
   //! true if a previous call to read*() got to EOF.
   bool m_bEOF:1;
   /*! If true and m_lterm is line_terminator::any or line_terminator::convert_any_to_lf, and the
   next read operation encounters a leading ‘\n’, that character will not be considered as a line
   terminator; this way, even if a “\r\n” was broken into multiple reads, we’ll still present
   clients with a single ‘\n’ character instead of two, as it would happen without this tracker (one
   from the trailing ‘\r’ of the first read, one from the leading ‘\n’ of the second. */
   bool m_bDiscardNextLF:1;
};

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

//! Implementation of a text (character-based) writer on top of a binary::buffered_writer instance.
class ABACLADE_SYM binbuf_writer : public virtual binbuf_base, public virtual writer {
public:
   /*! Constructor.

   @param pbbw
      Pointer to a binary buffered writer to work with.
   @param enc
      Initial value for get_encoding(). If omitted and never explicitly set, on the first write it
      will default to abc::text::encoding::utf8.
   */
   explicit binbuf_writer(
      _std::shared_ptr<binary::buffered_writer> pbbw,
      abc::text::encoding enc = abc::text::encoding::unknown
   );

   //! Destructor.
   virtual ~binbuf_writer();

   //! See binbuf_base::binary_buffered().
   _std::shared_ptr<binary::buffered_writer> binary_buffered() const {
      return m_pbbw;
   }

   //! See writer::finalize().
   virtual void finalize() override;

   //! See writer::flush().
   virtual void flush() override;

   //! See writer::write_binary().
   virtual void write_binary(
      void const * pSrc, std::size_t cbSrc, abc::text::encoding enc
   ) override;

protected:
   //! See binbuf_base::_binary_buffered_base().
   virtual _std::shared_ptr<binary::buffered_base> _binary_buffered_base() const override;

protected:
   //! Underlying binary buffered writer.
   _std::shared_ptr<binary::buffered_writer> m_pbbw;
};

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_IO_TEXT_HXX
