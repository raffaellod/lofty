/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/text.hxx>

#include <algorithm> // std::min()


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {

str_stream::str_stream() :
   stream(),
   buf(&default_buf),
   char_offset(0) {
}

str_stream::str_stream(str_stream && src) :
   stream(_std::move(src)),
   buf(src.buf != &src.default_buf ? src.buf : &default_buf),
   default_buf(_std::move(src.default_buf)),
   char_offset(src.char_offset) {
   // Make src use its own internal buffer.
   src.buf = &src.default_buf;
   src.char_offset = 0;
}

/*explicit*/ str_stream::str_stream(str const & src_buf) :
   stream(),
   buf(&default_buf),
   default_buf(src_buf),
   char_offset(0) {
}

/*explicit*/ str_stream::str_stream(str && src_buf) :
   stream(),
   buf(&default_buf),
   default_buf(_std::move(src_buf)),
   char_offset(0) {
}

/*explicit*/ str_stream::str_stream(external_buffer_t const &, str const * ext_buf) :
   stream(),
   buf(const_cast<str *>(ext_buf)),
   char_offset(0) {
}

/*virtual*/ str_stream::~str_stream() {
}

/*virtual*/ lofty::text::encoding str_stream::get_encoding() const /*override*/ {
   return lofty::text::encoding::host;
}

}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {

str_istream::str_istream(str const & src_buf) :
   stream(),
   str_stream(src_buf),
   istream() {
}

str_istream::str_istream(str && src_buf) :
   stream(),
   str_stream(_std::move(src_buf)),
   istream() {
}

str_istream::str_istream(external_buffer_t const & eb, str const * ext_buf) :
   stream(),
   str_stream(eb, ext_buf),
   istream() {
}

/*virtual*/ str_istream::~str_istream() {
}

/*virtual*/ void str_istream::consume_chars(std::size_t count) /*override*/ {
   if (count > remaining_size_in_chars()) {
      // TODO: use a better exception class.
      LOFTY_THROW(argument_error, ());
   }
   char_offset += count;
}

/*virtual*/ str str_istream::peek_chars(std::size_t count_min) /*override*/ {
   // Always return the whole string buffer after char_offset, ignoring count_min.
   LOFTY_UNUSED_ARG(count_min);
   return str(external_buffer, buf->data() + char_offset, remaining_size_in_chars());
}

/*virtual*/ void str_istream::read_all(str * dst) /*override*/ {
   *dst = _std::move(*buf);
   buf = &default_buf;
   char_offset = 0;
}

/*virtual*/ void str_istream::unconsume_chars(str const & s) /*override*/ {
   std::size_t count = s.size_in_chars();
   if (count > char_offset) {
      // TODO: use a better exception class.
      LOFTY_THROW(argument_error, ());
   }
   char_offset -= count;
}

}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {

str_ostream::str_ostream() :
   stream(),
   str_stream(),
   ostream() {
}

str_ostream::str_ostream(str_ostream && src) :
   stream(_std::move(src)),
   str_stream(_std::move(src)),
   ostream(_std::move(src)) {
}

str_ostream::str_ostream(external_buffer_t const & eb, str * ext_buf) :
   stream(),
   str_stream(eb, ext_buf),
   ostream() {
}

/*virtual*/ str_ostream::~str_ostream() {
}

void str_ostream::clear() {
   buf->set_size_in_chars(0);
   char_offset = 0;
}

/*virtual*/ void str_ostream::finalize() /*override*/ {
   // Nothing to do.
}

/*virtual*/ void str_ostream::flush() /*override*/ {
   // Nothing to do.
}

str str_ostream::release_content() {
   char_offset = 0;
   return _std::move(*buf);
}

/*virtual*/ void str_ostream::write_binary(
   void const * src, std::size_t src_byte_size, lofty::text::encoding enc
) /*override*/ {
   if (src_byte_size == 0) {
      // Nothing to do.
      return;
   }
   LOFTY_ASSERT(enc != lofty::text::encoding::unknown, LOFTY_SL("cannot write data with unknown encoding"));
   if (enc == lofty::text::encoding::host) {
      // Optimal case: no transcoding necessary.
      std::size_t src_char_size = src_byte_size / sizeof(char_t);
      // Enlarge the string as necessary, then overwrite any character in the affected range.
      buf->set_capacity(char_offset + src_char_size, true);
      memory::copy(buf->data() + char_offset, static_cast<char_t const *>(src), src_char_size);
      char_offset += src_char_size;
   } else {
      // Calculate the additional buffer size required.
      std::size_t buf_byte_size = lofty::text::transcode(
         true, enc, &src, &src_byte_size, lofty::text::encoding::host
      );
      buf->set_capacity(char_offset + buf_byte_size / sizeof(char_t), true);
      // Transcode the source into the string buffer and advance char_offset accordingly.
      void * buf_dst = buf->data() + char_offset;
      char_offset += lofty::text::transcode(
         true, enc, &src, &src_byte_size, lofty::text::encoding::host, &buf_dst, &buf_byte_size
      ) / sizeof(char_t);
   }
   // Truncate the string.
   buf->set_size_in_chars(char_offset);
}

}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {

char_ptr_ostream::char_ptr_ostream(char * buf, std::size_t * buf_available) :
   write_buf(buf),
   write_buf_available_chars(buf_available) {
}

char_ptr_ostream::char_ptr_ostream(char_ptr_ostream && src) :
   ostream(_std::move(src)),
   write_buf(src.write_buf),
   write_buf_available_chars(src.write_buf_available_chars) {
   src.write_buf = nullptr;
}

/*virtual*/ char_ptr_ostream::~char_ptr_ostream() {
   if (write_buf) {
      /* NUL-terminate the string. Always safe, since *write_buf_available_chars doesn’t include space for the
      NUL terminator. */
      *write_buf = '\0';
   }
}

/*virtual*/ void char_ptr_ostream::finalize() /*override*/ {
   // Nothing to do.
}

/*virtual*/ void char_ptr_ostream::flush() /*override*/ {
   // Nothing to do.
}

/*virtual*/ lofty::text::encoding char_ptr_ostream::get_encoding() const /*override*/ {
   // Assume char is always UTF-8.
   return lofty::text::encoding::utf8;
}

/*virtual*/ void char_ptr_ostream::write_binary(
   void const * src, std::size_t src_byte_size, lofty::text::encoding enc
) /*override*/ {
   if (src_byte_size == 0) {
      // Nothing to do.
      return;
   }
   LOFTY_ASSERT(enc != lofty::text::encoding::unknown, LOFTY_SL("cannot write data with unknown encoding"));
   if (enc == lofty::text::encoding::utf8) {
      // Optimal case: no transcoding necessary.
      std::size_t src_chars = std::min(*write_buf_available_chars, src_byte_size / sizeof(char));
      memory::copy(write_buf, static_cast<char const *>(src), src_chars);
      write_buf += src_chars;
      *write_buf_available_chars -= src_chars;
   } else {
      // Transcode the source into the string buffer.
      lofty::text::transcode(
         true, enc, &src, &src_byte_size, lofty::text::encoding::utf8,
         reinterpret_cast<void **>(&write_buf), write_buf_available_chars
      );
   }
}

}}} //namespace lofty::io::text
