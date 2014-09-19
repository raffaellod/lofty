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

#include <abaclade.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::utf8_char_traits

namespace abc {
namespace text {

/* Optimization 1: odd indices would have the same values as the preceding even ones, so the number
of elements can be cut in half.

Optimization 2: the maximum length is less than 0xf, so each value is encoded in a nibble instead of
a full byte.

In the end, the lead byte is treated like this:

   ┌─────────────┬──────────────┬────────┐
   │ 7 6 5 4 3 2 │       1      │    0   │
   ├─────────────┼──────────────┼────────┤
   │ byte  index │ nibble index │ unused │
   └─────────────┴──────────────┴────────┘

See utf8_char_traits::lead_char_to_codepoint_size() for the actual code accessing this array. */
std::uint8_t const utf8_char_traits::smc_acbCpSizesByLeadChar[] = {
   // 0xxxxxxx
   0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
   0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
   // 10xxxxxx – invalid (cannot be start of a sequence), so just skip it.
   0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
   // 110xxxxx
   0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
   // 1110xxxx
   0x33, 0x33, 0x33, 0x33,
   // 11110xxx
   0x44, 0x44,
   /* These are either overlong (code points encoded using more bytes than necessary) or invalid
   (the resulting symbol would be out of Unicode code point range). */
   // 111110xx
   0x55,
   // 1111110x same as above, and 1111111x is invalid (not UTF-8), so just skip it.
   0x16
};

std::uint8_t const utf8_char_traits::smc_acbitShiftMask[] = {
// 0xxxxxxx 110xxxxx 1110xxxx 11110xxx 111110xx 1111110x
   0,       2,       3,       4,       5,       6
};

std::uint8_t const utf8_char_traits::smc_aiValidLeadCharsMask[] = {
   // 1-byte sequences (1:1 with ASCII).
   /* 0x0? */ 0xff, 0xff,
   /* 0x1? */ 0xff, 0xff,
   /* 0x2? */ 0xff, 0xff,
   /* 0x3? */ 0xff, 0xff,
   /* 0x4? */ 0xff, 0xff,
   /* 0x5? */ 0xff, 0xff,
   /* 0x6? */ 0xff, 0xff,
   /* 0x7? */ 0xff, 0xff,
   // Trail bytes (10xxxxxx).
   /* 0x8? */ 0x00, 0x00,
   /* 0x9? */ 0x00, 0x00,
   /* 0xa? */ 0x00, 0x00,
   /* 0xb? */ 0x00, 0x00,
   // 1100000x 10yyyyyy is always an overlong encoding of 0xyyyyyy.
   /* 0xc? */ 0x3f, 0xff,
   /* 0xd? */ 0xff, 0xff,
   /* 0xe? */ 0xff, 0xff,
   // Encodings 11110101 10xxxxxx 10yyyyyy 10zzzzzz and higher lead to code points greater than
   // 10000 11111111 11111111
   /* 0xf? */ 0xf8, 0x00
};

/*static*/ char32_t utf8_char_traits::chars_to_codepoint(char8_t const * pchSrcBegin) {
   ABC_TRACE_FUNC(pchSrcBegin);

   char8_t ch = *pchSrcBegin;
   unsigned cchTrail = lead_char_to_codepoint_size(ch) - 1;
   // Convert the first byte.
   char32_t cp = get_lead_char_codepoint_bits(ch, cchTrail);
   // Shift in any continuation bytes.
   for (; cchTrail; --cchTrail) {
      cp = (cp << 6) | (*++pchSrcBegin & 0x3f);
   }
   return cp;
}

/*static*/ unsigned utf8_char_traits::codepoint_size(char32_t cp) {
   ABC_TRACE_FUNC(cp);

   if (!text::is_codepoint_valid(cp)) {
      ABC_THROW(text::error, ());
   } else if (cp <= 0x00007f) {
      // Encode xxx xxxx as 0xxxxxxx.
      return 1;
   } else if (cp <= 0x0007ff) {
      // Encode xxx xxyy yyyy as 110xxxxx 10yyyyyy.
      return 2;
   } else if (cp <= 0x00ffff) {
      // Encode xxxx yyyy yyzz zzzz as 1110xxxx 10yyyyyy 10zzzzzz.
      return 3;
   } else /*if (cp <= 0x10ffff)*/ {
      // Encode w wwxx xxxx yyyy yyzz zzzz as 11110www 10xxxxxx 10yyyyyy 10zzzzzz.
      return 4;
   }
}

/*static*/ char8_t * utf8_char_traits::codepoint_to_chars(char32_t cp, char8_t * pchDstBegin) {
   ABC_TRACE_FUNC(cp, pchDstBegin);

   // Compute the length of the UTF-8 sequence for this code point.
   unsigned cbSeq = codepoint_size(cp);
   // Calculate where the sequence will end, and write each byte backwards from there.
   char8_t * pchDstEnd = pchDstBegin + cbSeq;
   --cbSeq;
   char8_t iSeqIndicator = cont_length_to_seq_indicator(cbSeq);
   char8_t * pchDst = pchDstEnd;
   while (cbSeq--) {
      // Each trailing byte uses 6 bits.
      *--pchDst = static_cast<char8_t>(0x80 | (cp & 0x3f));
      cp >>= 6;
   }
   // The remaining code point bits (after >> 6 * (cbSeq - 1)) make up what goes in the lead byte.
   *--pchDst = iSeqIndicator | static_cast<char8_t>(cp);
   return pchDstEnd;
}

} //namespace text
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::utf16_char_traits

namespace abc {
namespace text {

/*static*/ char32_t utf16_char_traits::chars_to_codepoint(char16_t const * pchSrcBegin) {
   ABC_TRACE_FUNC(pchSrcBegin);

   char16_t chSrc0 = *pchSrcBegin;
   if (!is_surrogate(chSrc0)) {
      return chSrc0;
   }
   char16_t chSrc1 = *++pchSrcBegin;
   char32_t cp = ((static_cast<char32_t>(chSrc0 & 0x03ff) << 10) | (chSrc1 & 0x03ff)) + 0x10000;
   return is_codepoint_valid(cp) ? cp : replacement_char;
}

/*static*/ unsigned utf16_char_traits::codepoint_size(char32_t cp) {
   ABC_TRACE_FUNC(cp);

   if (!text::is_codepoint_valid(cp)) {
      ABC_THROW(text::error, ());
   }
   return cp > 0x00ffff ? 2u : 1u;
}

/*static*/ char16_t * utf16_char_traits::codepoint_to_chars(char32_t cp, char16_t * pchDstBegin) {
   ABC_TRACE_FUNC(cp, pchDstBegin);

   char16_t * pchDst = pchDstBegin;
   if (codepoint_size(cp) > 1) {
      // The code point requires two UTF-16 characters: generate a surrogate pair.
      cp -= 0x10000;
      *pchDst++ = static_cast<char16_t>(0xd800 | ((cp & 0x0ffc00) >> 10));
      *pchDst++ = static_cast<char16_t>(0xdc00 |  (cp & 0x0003ff)       );
   } else {
      // The code point fits in a single UTF-16 character.
      *pchDst++ = static_cast<char16_t>(cp);
   }
   return pchDst;
}

} //namespace text
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

