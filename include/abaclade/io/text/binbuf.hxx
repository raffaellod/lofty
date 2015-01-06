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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::binbuf_base

namespace abc {
namespace io {
namespace text {

//! Base for text I/O classes built on top of a binary::buffered_base instance.
class ABACLADE_SYM binbuf_base : public virtual base {
public:
   //! Destructor.
   virtual ~binbuf_base();

   /*! Returns a pointer to the underlying buffered binary I/O object.

   @return
      Pointer to a buffered binary I/O object.
   */
   std::shared_ptr<binary::buffered_base> binary_buffered() const {
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
   virtual std::shared_ptr<binary::buffered_base> _binary_buffered_base() const = 0;

protected:
   /*! Encoding used for I/O to/from the underlying buffered_base. If not explicitly set, it will be
   automatically determined and assigned on the first read or write. */
   abc::text::encoding m_enc;
};

} //namespace text
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::binbuf_reader

namespace abc {
namespace io {
namespace text {

//! Implementation of a text (character-based) reader on top of a binary::buffered_reader instance.
class ABACLADE_SYM binbuf_reader : public virtual binbuf_base, public virtual reader {
public:
   /*! Constructor.

   @param pbbr
      Pointer to a binary buffered reader to work with.
   @param enc
      Initial value for get_encoding(). If omitted, an encoding will be automatically detected
      (guessed) on the first read from the underlying binary reader.
   */
   explicit binbuf_reader(
      std::shared_ptr<binary::buffered_reader> pbbr,
      abc::text::encoding enc = abc::text::encoding::unknown
   );

   //! Destructor.
   virtual ~binbuf_reader();

   //! See binbuf_base::binary_buffered().
   std::shared_ptr<binary::buffered_reader> binary_buffered() const {
      return m_pbbr;
   }

protected:
   //! See binbuf_base::_binary_buffered_base().
   virtual std::shared_ptr<binary::buffered_base> _binary_buffered_base() const override;

   //! See reader::read_while().
   virtual bool read_while(mstr * psDst, bool bOneLine) override;

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
   std::size_t detect_encoding(std::int8_t const * pb, std::size_t cb);

   /*! Implementation of read_while() for the source encoding == host encoding.

   @param pb
      Pointer to a buffer with the initial contents of the file.
   @param pcb
      Pointer to the size of the buffer pointed to by pb; on return it will contain the count of
      bytes remaining in the last peek buffer.
   @param psDst
      Pointer to the string that will receive the read data.
   @param bOneLine
      If true, reading will stop at the first line terminator character.
   @return
      Count of characters read into *psDst.
   */
   std::size_t read_while_with_host_encoding(
      std::int8_t const * pbSrc, std::size_t * pcbSrc, mstr * psDst, bool bOneLine
   );

   /*! Implementation of read_while() for the source encoding != host encoding.

   @param pb
      Pointer to a buffer with the initial contents of the file.
   @param pcb
      Pointer to the size of the buffer pointed to by pb; on return it will contain the count of
      bytes remaining in the last peek buffer.
   @param psDst
      Pointer to the string that will receive the read data.
   @param bOneLine
      If true, reading will stop at the first line terminator character.
   @return
      Count of characters read into *psDst.
   */
   std::size_t read_while_with_transcode(
      std::int8_t const * pbSrc, std::size_t * pcbSrc, mstr * psDst, bool bOneLine
   );

protected:
   //! Underlying binary buffered reader.
   std::shared_ptr<binary::buffered_reader> m_pbbr;

private:
   /*! If true and m_lterm is line_terminator::any or line_terminator::convert_any_to_lf, and the
   next read operation encounters a leading ‘\n’, that character will not be considered as a line
   terminator; this way, even if a “\r\n” was broken into multiple reads, we’ll still present
   clients with a single ‘\n’ character instead of two, as it would happen without this tracker (one
   from the trailing ‘\r’ of the first read, one from the leading ‘\n’ of the second. */
   bool m_bDiscardNextLF:1;
};

} //namespace text
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::binbuf_writer

namespace abc {
namespace io {
namespace text {

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
      std::shared_ptr<binary::buffered_writer> pbbw,
      abc::text::encoding enc = abc::text::encoding::unknown
   );

   //! Destructor.
   virtual ~binbuf_writer();

   //! See binbuf_base::binary_buffered().
   std::shared_ptr<binary::buffered_writer> binary_buffered() const {
      return m_pbbw;
   }

   //! See writer::write_binary().
   virtual void write_binary(
      void const * pSrc, std::size_t cbSrc, abc::text::encoding enc
   ) override;

protected:
   //! See binbuf_base::_binary_buffered_base().
   virtual std::shared_ptr<binary::buffered_base> _binary_buffered_base() const override;

protected:
   //! Underlying binary buffered writer.
   std::shared_ptr<binary::buffered_writer> m_pbbw;
};

} //namespace text
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
