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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

int_from_text_istream_base::int_from_text_istream_base(bool is_signed_) :
   is_signed(is_signed_),
   prefix(false),
   unprefixed_base_or_shift(0) {
}

template <typename I>
inline void int_from_text_istream_base::convert_capture_impl(
   text::parsers::dynamic_match_capture const & capture0, I * dst
) const {
   LOFTY_TRACE_FUNC(this/*, capture0*/, dst);

   unsigned cap_group_index = 0;
   bool negative = false;
   if (is_signed) {
      if (auto sign_cap = capture0.capture_group(cap_group_index).str()) {
         negative = (sign_cap[0] == '-');
      }
      ++cap_group_index;
   }
   unsigned base_or_shift;
   if (prefix) {
      if (auto prefix_cap = capture0.capture_group(cap_group_index).str()) {
         switch (*prefix_cap.rbegin()) {
            case 'B':
            case 'b':
               base_or_shift = 1;
               break;
            case '0':
            case 'O':
            case 'o':
               base_or_shift = 3;
               break;
            case 'X':
            case 'x':
               base_or_shift = 4;
               break;
            LOFTY_SWITCH_WITHOUT_DEFAULT
         }
      } else {
         // We expected a prefix, but found none: it must be base 10.
         base_or_shift = 10;
      }
      ++cap_group_index;
   } else {
      base_or_shift = unprefixed_base_or_shift;
   }
   I ret = 0;
   if (base_or_shift == 10) {
      // TODO: use the UCD to determine the numeric value of each code point.
      LOFTY_FOR_EACH(char32_t cp, capture0.capture_group(cap_group_index).str()) {
         ret *= 10;
         ret += static_cast<I>(cp - '0');
      }
   } else {
      // Base 2 ^ n: can use | and <<.
      LOFTY_FOR_EACH(char32_t cp, capture0.capture_group(cap_group_index).str()) {
         ret <<= base_or_shift;
         if (cp >= '0' && cp <= '9') {
            ret |= static_cast<I>(cp - '0');
         } else if (cp >= 'a' && cp <= 'f') {
            ret |= static_cast<I>(cp - 'a' + 10);
         } else if (cp >= 'A' && cp <= 'F') {
            ret |= static_cast<I>(cp - 'A' + 10);
         }
      }
   }
   if (negative) {
      ret = -ret;
   }
   *dst = ret;
}

void int_from_text_istream_base::convert_capture_s64(
   text::parsers::dynamic_match_capture const & capture0, std::int64_t * dst
) const {
   convert_capture_impl(capture0, dst);
}

void int_from_text_istream_base::convert_capture_u64(
   text::parsers::dynamic_match_capture const & capture0, std::uint64_t * dst
) const {
   convert_capture_impl(capture0, dst);
}

#if LOFTY_HOST_WORD_SIZE < 64
void int_from_text_istream_base::convert_capture_s32(
   text::parsers::dynamic_match_capture const & capture0, std::int32_t * dst
) const {
   convert_capture_impl(capture0, dst);
}

void int_from_text_istream_base::convert_capture_u32(
   text::parsers::dynamic_match_capture const & capture0, std::uint32_t * dst
) const {
   convert_capture_impl(capture0, dst);
}

#if LOFTY_HOST_WORD_SIZE < 32
void int_from_text_istream_base::convert_capture_s16(
   text::parsers::dynamic_match_capture const & capture0, std::int16_t * dst
) const {
   convert_capture_impl(capture0, dst);
}

void int_from_text_istream_base::convert_capture_u16(
   text::parsers::dynamic_match_capture const & capture0, std::uint16_t * dst
) const {
   convert_capture_impl(capture0, dst);
}
#endif //if LOFTY_HOST_WORD_SIZE < 32
#endif //if LOFTY_HOST_WORD_SIZE < 64

text::parsers::dynamic_state const * int_from_text_istream_base::format_to_parser_states(
   str const & format, text::parsers::dynamic * parser
) {
   LOFTY_TRACE_FUNC(this, format, parser);

   /* If > 0, support base prefixes 0b, 0B, 0, 0o, 0O, 0x, or 0X. That also implies that we can parse multiple
   bases; if omitted, format may only specify a single base because otherwise we wouldn’t be able to parse the
   digits. */
   auto itr(format.cbegin());
   if (itr != format.cend() && *itr == '#') {
      prefix = true;
      ++itr;
   }
   /* Groups for each base must be listed as alternatives in a specific order due to base 8 with its weird “0”
   prefix. */
   bool add_base2 = false, add_base8 = false, add_base10 = false, add_base16 = false;
   text::parsers::dynamic_state const * first_base_cap_group = nullptr;
   for (; itr != format.cend(); ++itr) {
      if (first_base_cap_group && !prefix) {
         LOFTY_THROW(text::syntax_error, (
            LOFTY_SL("prefix (#) required if multiple bases are specified"), format,
            static_cast<unsigned>(itr - format.cbegin())
         ));
      }
      char32_t cp = *itr;
      switch (cp) {
         case 'b':
            add_base2 = true;
            break;
         case 'd':
            add_base10 = true;
            break;
         case 'o':
            add_base8 = true;
            break;
         case 'x':
            add_base16 = true;
            break;
         default:
            LOFTY_THROW(text::syntax_error, (
               LOFTY_SL("unexpected character"), format, static_cast<unsigned>(itr - format.cbegin())
            ));
      }
   }
   if (prefix && !add_base2 && !add_base8 && !add_base10 && !add_base16) {
      // If prefixed and no base was explicitly selected, allow all of them.
      add_base2 = add_base8 = add_base10 = add_base16 = true;
   } else if (!add_base2 && !add_base8 && !add_base16) {
      // If not prefixed and no bases were selected, force base 10.
      add_base10 = true;
   }

   if (add_base2) {
      LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_RANGE_STATE(base2_digit_state, nullptr, nullptr, '0', '1');
      LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(base2_digits_rep_group, nullptr, nullptr, &base2_digit_state.base, 1);
      auto base2_digits_cap_group = parser->create_capture_group(&base2_digits_rep_group.base);
      text::parsers::dynamic_state * base_first_state;
      if (prefix) {
         LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(base2_prefix_upper_b_state, nullptr, nullptr, 'B');
         LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(base2_prefix_lower_b_state, nullptr, &base2_prefix_upper_b_state.base, 'b');
         LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(base2_prefix_0_state, &base2_prefix_lower_b_state.base, nullptr, '0');
         auto base2_prefix_cap_group = parser->create_capture_group(&base2_prefix_0_state.base);
         base2_prefix_cap_group->set_next(base2_digits_cap_group);
         base_first_state = base2_prefix_cap_group;
      } else {
         unprefixed_base_or_shift = 1;
         base_first_state = base2_digits_cap_group;
      }
      first_base_cap_group = base_first_state->set_alternative(first_base_cap_group);
   }
   if (add_base16) {
      LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_RANGE_STATE(base16_upper_alpha_digit_state, nullptr, nullptr, 'A', 'F');
      LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_RANGE_STATE(base16_lower_alpha_digit_state, nullptr, &base16_upper_alpha_digit_state.base, 'a', 'f');
      LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_RANGE_STATE(base16_num_digit_state, nullptr, &base16_lower_alpha_digit_state.base, '0', '9');
      LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(base16_digits_rep_group, nullptr, nullptr, &base16_num_digit_state.base, 1);
      auto base16_digits_cap_group = parser->create_capture_group(&base16_digits_rep_group.base);
      text::parsers::dynamic_state * base_first_state;
      if (prefix) {
         LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(base16_prefix_upper_x_state, nullptr, nullptr, 'X');
         LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(base16_prefix_lower_x_state, nullptr, &base16_prefix_upper_x_state.base, 'x');
         LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(base16_prefix_0_state, &base16_prefix_lower_x_state.base, nullptr, '0');
         auto base16_prefix_cap_group = parser->create_capture_group(&base16_prefix_0_state.base);
         base16_prefix_cap_group->set_next(base16_digits_cap_group);
         base_first_state = base16_prefix_cap_group;
      } else {
         unprefixed_base_or_shift = 4;
         base_first_state = base16_digits_cap_group;
      }
      first_base_cap_group = base_first_state->set_alternative(first_base_cap_group);
   }
   if (add_base10) {
      LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_RANGE_STATE(base10_digit_state, nullptr, nullptr, '0', '9');
      LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(base10_digits_rep_group, nullptr, nullptr, &base10_digit_state.base, 1);
      auto base10_digits_cap_group = parser->create_capture_group(&base10_digits_rep_group.base);
      text::parsers::dynamic_state * base_first_state;
      if (prefix) {
         /* Must add a capture group even if base 10 has no prefix, otherwise the index of the last capture
         group will be off by 1. */
         auto base10_prefix_cap_group = parser->create_capture_group(nullptr);
         base10_prefix_cap_group->set_next(base10_digits_cap_group);
         base_first_state = base10_prefix_cap_group;
      } else {
         unprefixed_base_or_shift = 10;
         base_first_state = base10_digits_cap_group;
      }
      first_base_cap_group = base_first_state->set_alternative(first_base_cap_group);
   }
   if (add_base8) {
      LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_RANGE_STATE(base8_digit_state, nullptr, nullptr, '0', '7');
      LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(base8_digits_rep_group, nullptr, nullptr, &base8_digit_state.base, 1);
      auto base8_digits_cap_group = parser->create_capture_group(&base8_digits_rep_group.base);
      text::parsers::dynamic_state * base_first_state;
      if (prefix) {
         LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(base8_prefix_upper_o_state, nullptr, nullptr, 'O');
         LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(base8_prefix_lower_o_state, nullptr, &base8_prefix_upper_o_state.base, 'o');
         // For octal it’s “0[Oo]?”, unlike hexadecimal’s “0[Xx]” (“o” is optional).
         LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_GROUP(base8_prefix_o_rep_group, nullptr, nullptr, &base8_prefix_lower_o_state.base, 0, 1);
         LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(base8_prefix_0_state, &base8_prefix_o_rep_group.base, nullptr, '0');
         auto base8_prefix_cap_group = parser->create_capture_group(&base8_prefix_0_state.base);
         base8_prefix_cap_group->set_next(base8_digits_cap_group);
         base_first_state = base8_prefix_cap_group;
      } else {
         unprefixed_base_or_shift = 3;
         base_first_state = base8_digits_cap_group;
      }
      first_base_cap_group = base_first_state->set_alternative(first_base_cap_group);
   }

   if (is_signed) {
      LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(plus_state, nullptr, nullptr, '+');
      LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(minus_state, nullptr, &plus_state.base, '-');
      LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_GROUP(plus_minus_rep_group, nullptr, nullptr, &minus_state.base, 0, 1);
      auto plus_minus_cap_group = parser->create_capture_group(&plus_minus_rep_group.base);
      plus_minus_cap_group->set_next(first_base_cap_group);
      return plus_minus_cap_group;
   } else {
      // The integer type is unsigned, so we won’t accept a sign at all.
      return first_base_cap_group;
   }
}

}} //namespace lofty::_pvt
