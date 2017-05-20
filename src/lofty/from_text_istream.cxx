/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/from_str.hxx>
#include <lofty/text.hxx>
#include <lofty/text/parsers/dynamic.hxx>
#include <lofty/text/parsers/regex.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
   text::parsers::regex_capture_format const & format, text::parsers::dynamic * parser
) {
   LOFTY_TRACE_FUNC(this/*, format*/, parser);

   // TODO: more format validation.
   throw_on_unused_streaming_format_chars(format.expr.cbegin(), format.expr);

   auto true_state = parser->create_string_state(&true_str);
   auto false_state = parser->create_string_state(&false_str);
   true_state->set_alternative(false_state);
   return true_state;
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

/*explicit*/ int_from_text_istream_base::int_from_text_istream_base(bool is_signed_) :
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
   text::parsers::regex_capture_format const & format, text::parsers::dynamic * parser
) {
   LOFTY_TRACE_FUNC(this/*, format*/, parser);

   // TODO: more format validation.

   /* If > 0, support base prefixes 0b, 0B, 0, 0o, 0O, 0x, or 0X. That also implies that we can parse multiple
   bases; if omitted, format may only specify a single base because otherwise we wouldn’t be able to parse the
   digits. */
   auto itr(format.expr.cbegin());
   if (itr != format.expr.cend() && *itr == '#') {
      prefix = true;
      ++itr;
   }
   /* Groups for each base must be listed as alternatives in a specific order due to base 8 with its weird “0”
   prefix. */
   bool add_base2 = false, add_base8 = false, add_base10 = false, add_base16 = false;
   text::parsers::dynamic_state const * first_base_cap_group = nullptr;
   for (; itr != format.expr.cend(); ++itr) {
      if (first_base_cap_group && !prefix) {
         LOFTY_THROW(text::syntax_error, (
            LOFTY_SL("prefix (#) required if multiple bases are specified"), format.expr,
            static_cast<unsigned>(itr - format.expr.cbegin())
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
               LOFTY_SL("unexpected character"),
               format.expr, static_cast<unsigned>(itr - format.expr.cbegin())
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

   if (add_base10) {
      first_base_cap_group = create_base10_parser_states(parser)->set_alternative(first_base_cap_group);
   }
   if (add_base8) {
      first_base_cap_group = create_base8_parser_states(parser)->set_alternative(first_base_cap_group);
   }
   if (add_base16) {
      first_base_cap_group = create_base16_parser_states(parser)->set_alternative(first_base_cap_group);
   }
   if (add_base2) {
      first_base_cap_group = create_base2_parser_states(parser)->set_alternative(first_base_cap_group);
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

text::parsers::dynamic_state * int_from_text_istream_base::create_base2_parser_states(
   text::parsers::dynamic * parser
) {
   LOFTY_TRACE_FUNC(this, parser);

   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_RANGE_STATE(digit_state, nullptr, nullptr, '0', '1');
   LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(digits_rep_group, nullptr, nullptr, &digit_state.base, 1);
   auto digits_cap_group = parser->create_capture_group(&digits_rep_group.base);
   if (prefix) {
      LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(prefix_upper_b_state, nullptr, nullptr, 'B');
      LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(prefix_lower_b_state, nullptr, &prefix_upper_b_state.base, 'b');
      LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(prefix_0_state, &prefix_lower_b_state.base, nullptr, '0');
      return parser->create_capture_group(&prefix_0_state.base)->set_next(digits_cap_group);
   } else {
      unprefixed_base_or_shift = 1;
      return digits_cap_group;
   }
}

text::parsers::dynamic_state * int_from_text_istream_base::create_base8_parser_states(
   text::parsers::dynamic * parser
) {
   LOFTY_TRACE_FUNC(this, parser);

   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_RANGE_STATE(digit_state, nullptr, nullptr, '0', '7');
   LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(digits_rep_group, nullptr, nullptr, &digit_state.base, 1);
   auto digits_cap_group = parser->create_capture_group(&digits_rep_group.base);
   if (prefix) {
      LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(prefix_upper_o_state, nullptr, nullptr, 'O');
      LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(prefix_lower_o_state, nullptr, &prefix_upper_o_state.base, 'o');
      // For octal it’s “0[Oo]?”, unlike hexadecimal’s “0[Xx]” (“o” is optional).
      LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_GROUP(prefix_o_rep_group, nullptr, nullptr, &prefix_lower_o_state.base, 0, 1);
      LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(prefix_0_state, &prefix_o_rep_group.base, nullptr, '0');
      return parser->create_capture_group(&prefix_0_state.base)->set_next(digits_cap_group);
   } else {
      unprefixed_base_or_shift = 3;
      return digits_cap_group;
   }
}

text::parsers::dynamic_state * int_from_text_istream_base::create_base10_parser_states(
   text::parsers::dynamic * parser
) {
   LOFTY_TRACE_FUNC(this, parser);

   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_RANGE_STATE(digit_state, nullptr, nullptr, '0', '9');
   LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(digits_rep_group, nullptr, nullptr, &digit_state.base, 1);
   auto digits_cap_group = parser->create_capture_group(&digits_rep_group.base);
   if (prefix) {
      /* Must add a capture group even if base 10 has no prefix, otherwise the index of the last capture
      group will be off by 1. */
      return parser->create_capture_group(nullptr)->set_next(digits_cap_group);
   } else {
      unprefixed_base_or_shift = 10;
      return digits_cap_group;
   }
}

text::parsers::dynamic_state * int_from_text_istream_base::create_base16_parser_states(
   text::parsers::dynamic * parser
) {
   LOFTY_TRACE_FUNC(this, parser);

   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_RANGE_STATE(upper_alpha_digit_state, nullptr, nullptr, 'A', 'F');
   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_RANGE_STATE(lower_alpha_digit_state, nullptr, &upper_alpha_digit_state.base, 'a', 'f');
   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_RANGE_STATE(num_digit_state, nullptr, &lower_alpha_digit_state.base, '0', '9');
   LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(digits_rep_group, nullptr, nullptr, &num_digit_state.base, 1);
   auto digits_cap_group = parser->create_capture_group(&digits_rep_group.base);
   if (prefix) {
      LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(prefix_upper_x_state, nullptr, nullptr, 'X');
      LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(prefix_lower_x_state, nullptr, &prefix_upper_x_state.base, 'x');
      LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(prefix_0_state, &prefix_lower_x_state.base, nullptr, '0');
      return parser->create_capture_group(&prefix_0_state.base)->set_next(digits_cap_group);
   } else {
      unprefixed_base_or_shift = 4;
      return digits_cap_group;
   }
}

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

struct sequence_from_text_istream::impl {
   text::parsers::dynamic_match_capture curr_capture;
   text::parsers::regex_capture_format elt_format;
};


sequence_from_text_istream::sequence_from_text_istream(str const & start_delim_, str const & end_delim_) :
   separator(LOFTY_SL(", ")),
   start_delim(start_delim_),
   end_delim(end_delim_),
   pimpl(new impl()) {
}

sequence_from_text_istream::~sequence_from_text_istream() {
}

std::size_t sequence_from_text_istream::captures_count(
   text::parsers::dynamic_match_capture const & capture0
) const {
   LOFTY_TRACE_FUNC(this/*, capture0*/);

   std::size_t captures = capture0.repetition_group(1).size();
   if (captures > 0) {
      captures += capture0.repetition_group(1)[0].repetition_group(0).size();
   }
   return captures;
}

text::parsers::dynamic_match_capture const & sequence_from_text_istream::capture_at(
   text::parsers::dynamic_match_capture const & capture0, std::size_t i
) {
   LOFTY_TRACE_FUNC(this/*, capture0*/, i);

   auto all_elts_group(capture0.repetition_group(1)[0]);
   if (i == 0) {
      pimpl->curr_capture = all_elts_group.capture_group(0);
   } else {
      pimpl->curr_capture = all_elts_group.repetition_group(0)[i - 1].capture_group(0);
   }
   return pimpl->curr_capture;
}

text::parsers::regex_capture_format const & sequence_from_text_istream::extract_elt_format(
   text::parsers::regex_capture_format const & format
) {
   LOFTY_TRACE_FUNC(this/*, format*/);

   // TODO: more format validation.

   // TODO: parse format.expr with regex::parse_capture_format() (itself a TODO).
   pimpl->elt_format.expr = str(external_buffer, format.expr.data(), format.expr.size());
   return pimpl->elt_format;
}

static text::parsers::dynamic_state * expr_to_group(text::parsers::dynamic * parser, str const & expr) {
   LOFTY_TRACE_FUNC(parser, expr);

   text::parsers::dynamic_state * first_state;
   if (expr) {
      text::parsers::regex regex(parser, expr);
      first_state = regex.parse_with_no_captures();
   } else {
      first_state = nullptr;
   }
   return parser->create_repetition_group(first_state, 1, 1);
}

text::parsers::dynamic_state const * sequence_from_text_istream::format_to_parser_states(
   text::parsers::regex_capture_format const & format, text::parsers::dynamic * parser,
   text::parsers::dynamic_state const * elt_first_state
) {
   LOFTY_TRACE_FUNC(this/*, format*/, parser, elt_first_state);

   // TODO: more format validation.
   LOFTY_UNUSED_ARG(format);

   auto more_elt_cap_group = parser->create_capture_group(elt_first_state);
   auto separator_first_state = expr_to_group(parser, separator);
   separator_first_state->set_next(more_elt_cap_group);
   auto more_elt_cap_rep_group = parser->create_repetition_group(separator_first_state, 0);
   auto first_elt_cap_group = parser->create_capture_group(elt_first_state);
   first_elt_cap_group->set_next(more_elt_cap_rep_group);
   auto all_elt_rep_group = parser->create_repetition_group(first_elt_cap_group, 0, 1);
   auto end_first_state = expr_to_group(parser, end_delim);
   all_elt_rep_group->set_next(end_first_state);
   auto start_first_state = expr_to_group(parser, start_delim);
   start_first_state->set_next(all_elt_rep_group);
   return start_first_state;
}

}} //namespace lofty::_pvt
