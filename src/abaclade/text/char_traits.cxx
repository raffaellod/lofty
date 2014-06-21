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

// Optimization 1: odd indices would have the same values as the preceding even ones, so the number
// of elements can be cut in half.
// Optimization 2: the maximum length is less than 0xf, so each value is encoded in a nibble instead
// of a full byte.
//
// In the end, the lead byte is treated like this:
//
//    ┌─────────────┬──────────────┬────────┐
//    │ 7 6 5 4 3 2 │       1      │    0   │
//    ├─────────────┼──────────────┼────────┤
//    │ byte  index │ nibble index │ unused │
//    └─────────────┴──────────────┴────────┘
//
// See utf8_char_traits::lead_char_to_codepoint_size() for the actual code accessing this array.
uint8_t const utf8_char_traits::smc_acbCpSizesByLeadChar[] = {
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
   // These are either overlong (code points encoded using more bytes than necessary) or invalid
   // (the resulting symbol would be out of Unicode code point range).
   // 111110xx
   0x55,
   // 1111110x same as above, and 1111111x is invalid (not UTF-8), so just skip it.
   0x16
};

uint8_t const utf8_char_traits::smc_acbitShiftMask[] = {
// 0xxxxxxx 110xxxxx 1110xxxx 11110xxx 111110xx 1111110x
   0,       2,       3,       4,       5,       6
};



/*static*/ char8_t * utf8_char_traits::codepoint_to_chars(char32_t cp, char8_t * pchDstBegin) {
   ABC_TRACE_FUNC(cp, pchDstBegin);

   // Compute the length of the UTF-8 sequence for this code point.
   unsigned cbSeq;
   if (cp <= 0x00007f) {
      // Encode xxx xxxx as 0xxxxxxx.
      cbSeq = 1;
   } else if (cp <= 0x0007ff) {
      // Encode xxx xxyy yyyy as 110xxxxx 10yyyyyy.
      cbSeq = 2;
   } else if (cp <= 0x00ffff) {
      // Encode xxxx yyyy yyzz zzzz as 1110xxxx 10yyyyyy 10zzzzzz.
      cbSeq = 3;
   } else /*if (cp <= 0x10ffff)*/ {
      // Encode w wwxx xxxx yyyy yyzz zzzz as 11110www 10xxxxxx 10yyyyyy 10zzzzzz.
      cbSeq = 4;
   }
   // Calculate where the sequence will end, and write each byte backwards from there.
   char8_t * pchDstEnd(pchDstBegin + cbSeq);
   --cbSeq;
   char8_t iSeqIndicator(cont_length_to_seq_indicator(cbSeq));
   char8_t * pchDst(pchDstEnd);
   while (cbSeq--) {
      // Each trailing byte uses 6 bits.
      *--pchDst = char8_t(0x80 | (cp & 0x3f));
      cp >>= 6;
   }
   // The remaining code point bits (after >> 6 * (cbSeq - 1)) make up what goes in the lead byte.
   *--pchDst = iSeqIndicator | char8_t(cp);
   return pchDstEnd;
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::utf16_char_traits


namespace abc {
namespace text {

/*static*/ char16_t * utf16_char_traits::codepoint_to_chars(char32_t cp, char16_t * pchDstBegin) {
   ABC_TRACE_FUNC(cp, pchDstBegin);

   char16_t * pchDst(pchDstBegin);
   if (cp > 0x00ffff) {
      // The code point requires two UTF-16 characters: generate a surrogate pair.
      cp -= 0x10000;
      *pchDst++ = char16_t(0xd800 | ((cp & 0x0ffc00) >> 10));
      *pchDst++ = char16_t(0xdc00 |  (cp & 0x0003ff)       );
   } else {
      // The code point fits in a single UTF-16 character.
      *pchDst++ = char16_t(cp);
   }
   return pchDst;
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

