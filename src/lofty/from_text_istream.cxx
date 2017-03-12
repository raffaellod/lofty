/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/from_str.hxx>
#include <lofty/text.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

void throw_on_unused_from_str_chars(io::text::str_istream const & src) {
   if (std::size_t remaining_chars = src.remaining_size_in_chars()) {
      // There are still unused characters in src, so the conversion failed.
      str const & src_str = src.get_str();
      LOFTY_THROW(text::syntax_error, (
         LOFTY_SL("unexpected character"), src_str,
         static_cast<unsigned>(src_str.index_from_char_index(src_str.size_in_chars() - remaining_chars))
      ));
   }
}

}} //namespace lofty::_pvt

namespace lofty {

void throw_on_unused_streaming_format_chars(
   str::const_iterator const & format_consumed_end, str const & format
) {
   if (format_consumed_end != format.cend()) {
      LOFTY_THROW(text::syntax_error, (
         LOFTY_SL("unexpected character in format string"), format,
         static_cast<unsigned>(format_consumed_end - format.cbegin())
      ));
   }
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

from_text_istream<bool>::from_text_istream() :
   true_str(LOFTY_SL("true")),
   false_str(LOFTY_SL("false")) {
}

void from_text_istream<bool>::set_format(str const & format) {
   LOFTY_TRACE_FUNC(this, format);

   throw_on_unused_streaming_format_chars(format.cbegin(), format);
}

void from_text_istream<bool>::read(bool * dst, io::text::istream * src) {
   LOFTY_TRACE_FUNC(this, dst, src);

   sstr<16> read_word;
   for (str peek_buf; (peek_buf = src->peek_chars(1)); src->consume_chars(peek_buf.size_in_chars())) {
      for (auto itr(peek_buf.cbegin()), end(peek_buf.cend()); itr != end; ++itr) {
         char32_t cp = *itr;
         // TODO: replace this with the UCD equivalent of \w from lofty::text.
         if ((cp < '0' || cp > '9') && (cp < 'A' || cp > 'Z') && (cp < 'a' || cp > 'z')) {
            // Consume all preceding characters and stop.
            src->consume_chars(itr.char_index());
            goto break_outer_for;
         }
         read_word += cp;
      }
   }
break_outer_for:
   if (read_word == true_str) {
      *dst = true;
   } else if (read_word == false_str) {
      *dst = false;
   } else {
      src->unconsume_chars(read_word.str());
      // TODO: provide more information in the exception and/or use a better exception class.
      LOFTY_THROW(text::syntax_error, (LOFTY_SL("unrecognized input"), read_word.str()));
   }
}

} //namespace lofty
