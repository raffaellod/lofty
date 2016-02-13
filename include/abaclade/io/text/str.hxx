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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

//! Implementation of text (character-based) stream from/to a string.
class ABACLADE_SYM str_stream : public virtual stream, public noncopyable {
public:
   /*! Move constructor.

   @param ss
      Source object.
   */
   str_stream(str_stream && ss);

   //! Destructor.
   virtual ~str_stream();

   //! See stream::get_encoding().
   virtual abc::text::encoding get_encoding() const override;

   /*! Returns the internal string buffer as a read-only string.

   @return
      Content of the stream.
   */
   str const & get_str() const {
      return *m_psBuf;
   }

protected:
   //! Default constructor.
   str_stream();

   /*! Constructor that initializes the stream with a copy of the contents of a string.

   @param s
      String to be copied to the internal buffer.
   */
   explicit str_stream(str const & s);

   /*! Constructor that initializes the stream by moving the contents of a string.

   @param s
      String to be moved to the internal buffer.
   */
   explicit str_stream(str && s);

   /*! Constructor that assigns an external string as the stream’s buffer.

   @param ps
      Pointer to the string to be used as the stream’s buffer.
   */
   str_stream(external_buffer_t const &, str const * ps);

protected:
   //! Pointer to the string buffer.
   str * m_psBuf;
   //! Default target of m_psBuf, if none is supplied via the external_buffer constructor.
   str m_sDefaultBuf;
   //! Current read/write offset into the string, in char_t units.
   std::size_t m_ichOffset;
};

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

//! Implementation of text (character-based) input from a string.
class ABACLADE_SYM str_istream : public virtual str_stream, public virtual istream {
public:
   /*! Constructor that assigns a string to read from.

   @param s
      Source string to be copied to the internal buffer.
   */
   explicit str_istream(str const & s);

   /*! Constructor that move-assigns a string to read from.

   @param s
      Source string to be moved to the internal buffer.
   */
   explicit str_istream(str && s);

   /*! Constructor that associates an external string to read from.

   @param ps
      Pointer to the source string to be used as external_buffer.
   */
   str_istream(external_buffer_t const &, str const * ps);

   //! Destructor.
   virtual ~str_istream();

   //! See istream::consume_chars().
   virtual void consume_chars(std::size_t cch) override;

   //! See istream::peek_chars().
   virtual str peek_chars(std::size_t cchMin) override;

   // Pull in the other overload to avoid hiding it.
   using istream::read_all;

   //! See istream::read_all().
   virtual void read_all(str * psDst) override;

   /*! Returns the count of characters (char_t units) still available for reading.

   @return
      Count of characters still available for reading.
   */
   std::size_t remaining_size_in_chars() const {
      return m_psBuf->size_in_chars() - m_ichOffset;
   }

   //! See istream::unconsume_chars().
   virtual void unconsume_chars(str const & s) override;
};

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

//! Implementation of text (character-based) output into a string.
class ABACLADE_SYM str_ostream : public virtual str_stream, public virtual ostream {
public:
   //! Default constructor.
   str_ostream();

   /*! Move constructor.

   @param sos
      Source object.
   */
   str_ostream(str_ostream && sos);

   /*! Constructor that associates an external string to write to.

   @param psBuf
      Pointer to a non-owned string to use as the destination for all writes.
   */
   str_ostream(external_buffer_t const &, str * psBuf);

   //! Destructor.
   virtual ~str_ostream();

   //! Truncates the internal buffer so that the next write will occur at offset 0.
   void clear();

   //! See ostream::finalize().
   virtual void finalize() override;

   //! See ostream::flush().
   virtual void flush() override;

   /*! Yields ownership of the internal string buffer. If the str_ostream instance was constructed
   based on an external string, all internal variables will be successfully reset, but the result
   will be an empty string; the accumulated data will only be accessible through the external
   string.

   @return
      Former content of the stream.
   */
   str release_content();

   //! See ostream::write_binary().
   virtual void write_binary(
      void const * pSrc, std::size_t cbSrc, abc::text::encoding enc
   ) override;
};

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

//! Implementation of text (character-based) output into a fixed-size char array.
class ABACLADE_SYM char_ptr_ostream : public ostream {
public:
   /*! Constructor.

   @param pchBuf
      Pointer to a string buffer to use as the destination for all writes.
   @param pcchBufRemaining
      Pointer to a variable that tracks the count of characters available in *pchBuf excluding the
      trailing NUL terminator.
   */
   char_ptr_ostream(char * pchBuf, std::size_t * pcchBufRemaining);

   /*! Move constructor.

   @param cpos
      Source object.
   */
   char_ptr_ostream(char_ptr_ostream && cpos);

   //! Destructor.
   virtual ~char_ptr_ostream();

   //! See ostream::finalize().
   virtual void finalize() override;

   //! See ostream::flush().
   virtual void flush() override;

   //! See base::get_encoding().
   virtual abc::text::encoding get_encoding() const override;

   //! See ostream::write_binary().
   virtual void write_binary(
      void const * pSrc, std::size_t cbSrc, abc::text::encoding enc
   ) override;

protected:
   //! Pointer to the destination string buffer.
   char * m_pchWriteBuf;
   //! Pointer to a variable that tracks the count of characters available *m_pchWriteBuf.
   std::size_t * m_pcchWriteBufAvailable;
};

}}} //namespace abc::io::text
