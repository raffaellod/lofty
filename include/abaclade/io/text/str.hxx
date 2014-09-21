/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> instead of this file
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::str_base

namespace abc {
namespace io {
namespace text {

//! Implementation of text (character-based) I/O from/to a string.
class ABACLADE_SYM str_base : public virtual base, public noncopyable {
public:
   //! Destructor.
   virtual ~str_base();

   //! See base::get_encoding().
   virtual abc::text::encoding get_encoding() const override;

protected:
   //! See base::base().
   str_base();

protected:
   //! Current read/write offset into the string, in char_t units.
   std::size_t m_ichOffset;
};

} //namespace text
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::str_reader

namespace abc {
namespace io {
namespace text {

//! Implementation of text (character-based) input from a string.
class ABACLADE_SYM str_reader : public virtual str_base, public virtual reader {
public:
   /*! Constructor.

   s
      Source string to be copied or moved to the internal buffer.
   ps
      Pointer to the source string to be used as external_buffer.
   */
   explicit str_reader(istr const & s);
   explicit str_reader(istr && s);
   explicit str_reader(mstr && s);
   str_reader(external_buffer_t const &, istr const * ps);
   str_reader(external_buffer_t const &, mstr const * ps);

   //! See reader::read_while().
   virtual bool read_while(mstr * psDst, bool bOneLine) override;

   /*! Returns the count of characters (char_t units) still available for reading.

   return
      Count of characters still available for reading.
   */
   std::size_t remaining_size_in_chars() const {
      return m_psReadBuf->size_in_chars() - m_ichOffset;
   }

protected:
   //! Pointer to the source string, which is m_sReadBuf or an external string.
   istr const * m_psReadBuf;
   //! Default target of m_psReadBuf, if none is supplied via the external_buffer constructor.
   istr m_sReadBuf;
};

} //namespace text
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::str_writer

namespace abc {
namespace io {
namespace text {

//! Implementation of text (character-based) output into a string.
class ABACLADE_SYM str_writer : public virtual str_base, public virtual writer {
public:
   /*! Constructor.

   psBuf
      Pointer to a mutable string to use as the destination of all writes; otherwise an internal
      dynamically-allocated string will be used.
   */
   str_writer();
   str_writer(external_buffer_t const &, mstr * psBuf);

   //! Truncates the internal buffer so that the next write will occur at offset 0.
   void clear();

   /*! Returns the internal string buffer as a read-only string.

   return
      Content of the writer.
   */
   istr const & get_str() const {
      return *m_psWriteBuf;
   }

   /*! Yields ownership of the internal string buffer. If the str_writer instance was constructed
   based on an external string, all internal variables will be successfully reset, but the result
   will be an empty string; the accumulated data will only be accessible through the external
   string.

   return
      Former content of the writer.
   */
   dmstr release_content();

   //! See writer::write_binary().
   virtual void write_binary(
      void const * pSrc, std::size_t cbSrc, abc::text::encoding enc
   ) override;

protected:
   //! Pointer to the destination string.
   mstr * m_psWriteBuf;
   //! Default target of m_psWriteBuf, if none is supplied via the external_buffer constructor.
   dmstr m_sDefaultWriteBuf;
};

} //namespace text
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

