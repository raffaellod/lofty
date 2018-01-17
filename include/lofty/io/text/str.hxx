/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_HXX_INTERNAL
   #error "Please #include <lofty.hxx> instead of this file"
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {

//! Implementation of text (character-based) stream from/to a string.
class LOFTY_SYM str_stream : public virtual stream, public noncopyable {
public:
   /*! Move constructor.

   @param src
      Source object.
   */
   str_stream(str_stream && src);

   //! Destructor.
   virtual ~str_stream();

   //! See stream::get_encoding().
   virtual lofty::text::encoding get_encoding() const override;

   /*! Returns the internal string buffer as a read-only string.

   @return
      Content of the stream.
   */
   str const & get_str() const {
      return *buf;
   }

protected:
   //! Default constructor.
   str_stream();

   /*! Constructor that initializes the stream with a copy of the contents of a string.

   @param src_buf
      String to be copied to the internal buffer.
   */
   explicit str_stream(str const & src_buf);

   /*! Constructor that initializes the stream by moving the contents of a string.

   @param src_buf
      String to be moved to the internal buffer.
   */
   explicit str_stream(str && src_buf);

   /*! Constructor that assigns an external string as the stream’s buffer.

   @param ext_buf
      Pointer to the string to be used as the stream’s buffer. The pointer is const so that str_istream can
      use it as well.
   */
   str_stream(external_buffer_t const &, str const * ext_buf);

protected:
   //! Pointer to the string buffer.
   str * buf;
   //! Default target of buf, if none is supplied via the external_buffer constructor.
   str default_buf;
   //! Current read/write offset into the string, in char_t units.
   std::size_t char_offset;
};

}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {

//! Implementation of text (character-based) input from a string.
class LOFTY_SYM str_istream : public virtual str_stream, public virtual istream {
public:
   /*! Constructor that assigns a string to read from.

   @param src_buf
      Source string to be copied to the internal buffer.
   */
   explicit str_istream(str const & src_buf);

   /*! Constructor that move-assigns a string to read from.

   @param src_buf
      Source string to be moved to the internal buffer.
   */
   explicit str_istream(str && src_buf);

   /*! Constructor that associates an external string to read from.

   @param ext_buf
      Pointer to the source string to be used as external_buffer.
   */
   str_istream(external_buffer_t const &, str const * ext_buf);

   //! Destructor.
   virtual ~str_istream();

   //! See istream::consume_chars().
   virtual void consume_chars(std::size_t count) override;

   //! See istream::peek_chars().
   virtual str peek_chars(std::size_t count_min) override;

   // Pull in the other overload to avoid hiding it.
   using istream::read_all;

   //! See istream::read_all().
   virtual void read_all(str * dst) override;

   /*! Returns the count of characters (char_t units) still available for reading.

   @return
      Count of characters still available for reading.
   */
   std::size_t remaining_size_in_chars() const {
      return buf->size_in_chars() - char_offset;
   }

   //! See istream::unconsume_chars().
   virtual void unconsume_chars(str const & s) override;
};

}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {

//! Implementation of text (character-based) output into a string.
class LOFTY_SYM str_ostream : public virtual str_stream, public virtual ostream {
public:
   //! Default constructor.
   str_ostream();

   /*! Move constructor.

   @param src
      Source object.
   */
   str_ostream(str_ostream && src);

   /*! Constructor that associates an external string to write to.

   @param ext_buf
      Pointer to a non-owned string to use as the destination for all writes.
   */
   str_ostream(external_buffer_t const &, str * ext_buf);

   //! Destructor.
   virtual ~str_ostream();

   //! Truncates the internal buffer so that the next write will occur at offset 0.
   void clear();

   //! See ostream::finalize().
   virtual void finalize() override;

   //! See ostream::flush().
   virtual void flush() override;

   /*! Yields ownership of the internal string buffer. If the str_ostream instance was constructed based on an
   external string, all internal variables will be successfully reset, but the result will be an empty string;
   the accumulated data will only be accessible through the external string.

   @return
      Former content of the stream.
   */
   str release_content();

   //! See ostream::write_binary().
   virtual void write_binary(void const * src, std::size_t src_byte_size, lofty::text::encoding enc) override;
};

}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {

//! Implementation of text (character-based) output into a fixed-size char array.
class LOFTY_SYM char_ptr_ostream : public ostream {
public:
   /*! Constructor.

   @param buf
      Pointer to a string buffer to use as the destination for all writes.
   @param buf_available
      Pointer to a variable that tracks the count of characters available in *buf excluding the trailing NUL
      terminator.
   */
   char_ptr_ostream(char * buf, std::size_t * buf_available);

   /*! Move constructor.

   @param src
      Source object.
   */
   char_ptr_ostream(char_ptr_ostream && src);

   //! Destructor.
   virtual ~char_ptr_ostream();

   //! See ostream::finalize().
   virtual void finalize() override;

   //! See ostream::flush().
   virtual void flush() override;

   //! See base::get_encoding().
   virtual lofty::text::encoding get_encoding() const override;

   //! See ostream::write_binary().
   virtual void write_binary(void const * src, std::size_t src_byte_size, lofty::text::encoding enc) override;

protected:
   //! Pointer to the destination string buffer.
   char * write_buf;
   //! Pointer to a variable that tracks the count of characters available *write_buf.
   std::size_t * write_buf_available_chars;
};

}}} //namespace lofty::io::text
