/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015 Raffaello D. Di Napoli

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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

//! Implementation of text (character-based) I/O from/to a string.
class ABACLADE_SYM str_base : public virtual base, public noncopyable {
public:
   /*! Move constructor.

   @param sb
      Source object.
   */
   str_base(str_base && sb);

   //! Destructor.
   virtual ~str_base();

   //! See base::get_encoding().
   virtual abc::text::encoding get_encoding() const override;

protected:
   //! Default constructor.
   str_base();

protected:
   //! Current read/write offset into the string, in char_t units.
   std::size_t m_ichOffset;
};

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

//! Implementation of text (character-based) input from a string.
class ABACLADE_SYM str_reader : public virtual str_base, public virtual reader {
public:
   /*! Move constructor.

   @param sr
      Source object.
   */
   str_reader(str_reader && sr);

   /*! Constructor that assigns a string to read from.

   @param s
      Source string to be copied to the internal buffer.
   */
   explicit str_reader(str const & s);

   /*! Constructor that move-assigns a string to read from.

   @param s
      Source string to be moved to the internal buffer.
   */
   explicit str_reader(str && s);

   /*! Constructor that associates an external string to read from.

   @param ps
      Pointer to the source string to be used as external_buffer.
   */
   str_reader(external_buffer_t const &, str const * ps);

   //! Destructor.
   virtual ~str_reader();

   /*! Returns the count of characters (char_t units) still available for reading.

   @return
      Count of characters still available for reading.
   */
   std::size_t remaining_size_in_chars() const {
      return m_psReadBuf->size_in_chars() - m_ichOffset;
   }

protected:
   //! See reader::read_line_or_all().
   virtual bool read_line_or_all(str * psDst, bool bOneLine) override;

protected:
   //! Pointer to the source string, which is m_sReadBuf or an external string.
   str const * m_psReadBuf;
   //! Default target of m_psReadBuf, if none is supplied via the external_buffer constructor.
   str m_sReadBuf;
};

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

//! Implementation of text (character-based) output into a string.
class ABACLADE_SYM str_writer : public virtual str_base, public virtual writer {
public:
   //! Default constructor.
   str_writer();

   /*! Move constructor.

   @param sw
      Source object.
   */
   str_writer(str_writer && sw);

   /*! Constructor that associates an external string to write to.

   @param psBuf
      Pointer to a non-owned string to use as the destination for all writes.
   */
   str_writer(external_buffer_t const &, str * psBuf);

   //! Destructor.
   virtual ~str_writer();

   //! Truncates the internal buffer so that the next write will occur at offset 0.
   void clear();

   //! See writer::finalize().
   virtual void finalize() override;

   //! See writer::flush().
   virtual void flush() override;

   /*! Returns the internal string buffer as a read-only string.

   @return
      Content of the writer.
   */
   str const & get_str() const {
      return *m_psWriteBuf;
   }

   /*! Yields ownership of the internal string buffer. If the str_writer instance was constructed
   based on an external string, all internal variables will be successfully reset, but the result
   will be an empty string; the accumulated data will only be accessible through the external
   string.

   @return
      Former content of the writer.
   */
   str release_content();

   //! See writer::write_binary().
   virtual void write_binary(
      void const * pSrc, std::size_t cbSrc, abc::text::encoding enc
   ) override;

protected:
   //! Pointer to the destination string.
   str * m_psWriteBuf;
   //! Default target of m_psWriteBuf, if none is supplied via the external_buffer constructor.
   str m_sDefaultWriteBuf;
};

}}} //namespace abc::io::text
