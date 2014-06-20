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

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

