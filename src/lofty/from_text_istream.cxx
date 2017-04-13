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
#include <lofty/text/parsers/dynamic.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

void throw_after_from_str_parse_failed(str const & src) {
   /* TODO: with a fair bit of work, parsers::dynamic could be modified to track the farthest character index
   it could parse successfully. */
   LOFTY_THROW(text::syntax_error, (LOFTY_SL("malformed input"), src, 0));
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

void from_text_istream<bool>::convert_capture(
   text::parsers::dynamic_match_capture const & capture0, bool * dst
) {
   *dst = (capture0.str() == true_str);
}

text::parsers::dynamic_state const * from_text_istream<bool>::format_to_parser_states(
   str const & format, text::parsers::dynamic * parser
) {
   LOFTY_TRACE_FUNC(this, format, parser);

   throw_on_unused_streaming_format_chars(format.cbegin(), format);

   auto true_state = parser->create_string_state(&true_str);
   auto false_state = parser->create_string_state(&false_str);
   true_state->set_alternative(false_state);
   return true_state;
}

} //namespace lofty
