/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/io/binary.hxx>
#include <lofty/io/text.hxx>
#include <lofty/numeric.hxx>
#include <lofty/text.hxx>

#include <algorithm> // std::min()


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {

binbuf_stream::binbuf_stream(lofty::text::encoding enc) :
   stream(),
   default_enc(enc) {
}

/*virtual*/ binbuf_stream::~binbuf_stream() {
}

/*virtual*/ lofty::text::encoding binbuf_stream::get_encoding() const /*override*/ {
   return default_enc;
}

}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {

binbuf_istream::binbuf_istream(
   _std::shared_ptr<binary::buffered_istream> buf_bin_istream_,
   lofty::text::encoding enc /*= lofty::text::encoding::unknown*/
) :
   stream(),
   binbuf_stream(enc),
   istream(),
   buf_bin_istream(_std::move(buf_bin_istream_)),
   peek_buf_char_offset(0),
   eof(false) {
}

/*virtual*/ binbuf_istream::~binbuf_istream() {
}

/*virtual*/ _std::shared_ptr<binary::buffered_stream> binbuf_istream::_binary_buffered_stream(
) const /*override*/ {
   return buf_bin_istream;
}

/*virtual*/ void binbuf_istream::consume_chars(std::size_t count) /*override*/ {
   if (count > peek_buf.size_in_chars() - peek_buf_char_offset) {
      // TODO: use a better exception class.
      LOFTY_THROW(argument_error, ());
   }
   peek_buf_char_offset += count;
}

std::size_t binbuf_istream::detect_encoding(std::uint8_t const * buf, std::size_t buf_byte_size) {
   std::size_t total_byte_size, bom_byte_size;
   if (auto sized = _std::dynamic_pointer_cast<binary::sized>(buf_bin_istream->unbuffered())) {
      /* This special value prevents guess_encoding() from dismissing UTF-16/32 as impossible just because the
      need to clip total_byte_size to a std::size_t resulted in an odd count of bytes. */
      static std::size_t const aligned_max = numeric::max<std::size_t>::value & ~sizeof(char32_t);
      total_byte_size = static_cast<std::size_t>(
         std::min(sized->size(), static_cast<full_size_t>(aligned_max))
      );
   } else {
      total_byte_size = 0;
   }
   default_enc = lofty::text::guess_encoding(buf, buf + buf_byte_size, total_byte_size, &bom_byte_size);
   // Cannot continue if the encoding is still unknown.
   if (default_enc == lofty::text::encoding::unknown) {
      // TODO: provide more information in the exception.
      LOFTY_THROW(lofty::text::error, ());
   }
   return bom_byte_size;
}

/*virtual*/ str binbuf_istream::peek_chars(std::size_t count_min) /*override*/ {
   // The peek buffer might already contain enough characters.
   std::size_t peek_buf_char_size = peek_buf.size_in_chars() - peek_buf_char_offset;
   if (peek_buf_char_size < count_min && !eof) {
      /* Ensure the peek buffer is large enough to hold the requested count of characters or an arbitrary
      minimum (chosen for efficiency). */
      if (count_min > peek_buf.capacity() - peek_buf_char_offset) {
         // If there’s any unused space in peek_buf, recover it now.
         /* TODO: might use a different strategy to decide if it’s more convenient to just allocate a bigger
         buffer based on peek_buf.capacity() vs. peek_buf_char_size, i.e. the cost of memory::realloc() vs.
         the cost of memory::move(). */
         if (peek_buf_char_offset > 0) {
            if (peek_buf_char_size > 0) {
               memory::move(peek_buf.data(), peek_buf.data() + peek_buf_char_offset, peek_buf_char_size);
            }
            peek_buf_char_offset = 0;
            peek_buf.set_size_in_chars(peek_buf_char_size, false /*don’t clear*/);
         }
         static std::size_t const peek_buf_char_size_min = 128;
         std::size_t needed_capacity = std::max(count_min, peek_buf_char_size_min);
         if (needed_capacity > peek_buf.capacity()) {
            peek_buf.set_capacity(needed_capacity, true /*preserve*/);
         }
      }
      char_t * peek_buf_chars_begin = peek_buf.data() + peek_buf_char_offset;
      std::size_t peek_buf_bytes_capacity = (peek_buf.capacity() - peek_buf_char_offset) * sizeof(char_t);
      void * peek_buf_end = peek_buf_chars_begin;

      std::size_t min_peek_bytes = 1;
      do {
         auto src(buf_bin_istream->peek<std::uint8_t>(min_peek_bytes));
         if (src.size == 0) {
            eof = true;
            break;
         }

         // If the encoding is still undefined, try to guess it now.
         if (default_enc == lofty::text::encoding::unknown) {
            if (std::size_t bom_byte_size = detect_encoding(src.ptr, src.size)) {
               // Consume the BOM that was read.
               buf_bin_istream->consume<std::uint8_t>(bom_byte_size);
               src.ptr += bom_byte_size;
               src.size -= bom_byte_size;
            }
         }

         // Transcode the binary peek buffer into peek_buf at peek_buf_char_offset + peek_buf_char_size.
         std::size_t src_remaining_bytes = src.size;
         std::size_t peek_buf_transcoded_byte_size = lofty::text::transcode(
            true, default_enc, reinterpret_cast<void const **>(&src.ptr), &src_remaining_bytes,
            lofty::text::encoding::host, &peek_buf_end, &peek_buf_bytes_capacity
         );
         if (peek_buf_transcoded_byte_size == 0) {
            // Couldn’t transcode even a single code point; get more bytes and try again.
            ++min_peek_bytes;
            continue;
         }
         // If this was changed, ensure it’s reset now that we successfully transcoded something.
         min_peek_bytes = 1;

         // Permanently remove the transcoded bytes from the binary buffer.
         buf_bin_istream->consume<std::uint8_t>(src.size - src_remaining_bytes);
         // Account for the characters just transcoded.
         peek_buf_char_size += peek_buf_transcoded_byte_size / sizeof(char_t);
         peek_buf.set_size_in_chars(peek_buf_char_offset + peek_buf_char_size, false /*don’t clear*/);
      } while (peek_buf_char_size < count_min);
   }
   // Return a view of peek_buf to avoid copying it.
   return str(
      external_buffer, peek_buf.data() + peek_buf_char_offset, peek_buf.size_in_chars() - peek_buf_char_offset
   );
}

/*virtual*/ bool binbuf_istream::read_line(str * dst) /*override*/ {
   if (eof) {
      dst->clear();
      return false;
   } else {
      // This will result in calls to peek_chars(), which will set eof as appropriate.
      istream::read_line(dst);
      return true;
   }
}

/*virtual*/ void binbuf_istream::unconsume_chars(str const & s) /*override*/ {
   if (std::size_t count = s.size_in_chars()) {
      peek_buf.set_size_in_chars(count + peek_buf.size_in_chars(), false /*don’t clear*/);
      memory::copy(peek_buf.data(), s.data(), count);
   }
}

}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {

binbuf_ostream::binbuf_ostream(
   _std::shared_ptr<binary::buffered_ostream> buf_bin_ostream_,
   lofty::text::encoding enc /*= lofty::text::encoding::unknown*/
) :
   stream(),
   binbuf_stream(enc),
   ostream(),
   buf_bin_ostream(_std::move(buf_bin_ostream_)) {
}

/*virtual*/ binbuf_ostream::~binbuf_ostream() {
   // Let buf_bin_ostream detect whether close() was not called.
}

/*virtual*/ _std::shared_ptr<binary::buffered_stream> binbuf_ostream::_binary_buffered_stream(
) const /*override*/ {
   return buf_bin_ostream;
}

/*virtual*/ void binbuf_ostream::close() /*override*/ {
   if (buf_bin_ostream.use_count() == 1) {
      // This is the last owner of buf_bin_ostream, unless another thread is running weak_ptr::lock() on it.
      buf_bin_ostream->close();
   }
}

/*virtual*/ void binbuf_ostream::flush() /*override*/ {
   buf_bin_ostream->flush();
}

/*virtual*/ void binbuf_ostream::write_binary(
   void const * src, std::size_t src_byte_size, lofty::text::encoding enc
) /*override*/ {
   LOFTY_ASSERT(enc != lofty::text::encoding::unknown, LOFTY_SL("cannot write data with unknown encoding"));

   // If no encoding has been set yet, default to UTF-8.
   if (default_enc == lofty::text::encoding::unknown) {
      default_enc = lofty::text::encoding::utf8;
   }

   // Easy case.
   if (src_byte_size == 0) {
      return;
   }
   auto dst(buf_bin_ostream->get_buffer<std::int8_t>(src_byte_size));
   if (enc == default_enc) {
      // Optimal case: no transcoding necessary.
      memory::copy(dst.ptr, static_cast<std::int8_t const *>(src), src_byte_size);
      dst.size = src_byte_size;
   } else {
      // Sub-optimal case: transcoding is needed.
      dst.size = lofty::text::transcode(
         true, enc, &src, &src_byte_size, default_enc, reinterpret_cast<void **>(&dst.ptr), &dst.size
      );
   }
   buf_bin_ostream->commit_bytes(dst.size);
}

}}} //namespace lofty::io::text
