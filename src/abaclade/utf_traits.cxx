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
// abc::text::utf8_traits


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
// See utf8_traits::lead_char_to_codepoint_size() for the actual code accessing this array.
uint8_t const utf8_traits::smc_acbCpSizesByLeadChar[] = {
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

uint8_t const utf8_traits::smc_acbitShiftMask[] = {
// 0xxxxxxx 110xxxxx 1110xxxx 11110xxx 111110xx 1111110x
   0,       2,       3,       4,       5,       6
};

uint8_t const utf8_traits::smc_aiOverlongDetectionMasks[] = {
   // Leading byte = 110zzzzz, first continuation byte must have at least one K=1 in ccKyyyyy.
   0x20,
   // Leading byte = 1110zzzz, first continuation byte must have at least one K=1 in ccKKyyyy.
   0x30,
   // Leading byte = 11110zzz, first continuation byte must have at least one K=1 in ccKKKyyy.
   0x38,
   // Leading byte = 111110zz, first continuation byte must have at least one K=1 in ccKKKKyy.
   0x3c,
   // Leading byte = 1111110z, first continuation byte must have at least one K=1 in ccKKKKKy.
   0x3e
};


/*static*/ bool utf8_traits::is_valid(char8_t const * psz) {
   ABC_TRACE_FUNC(psz);

   unsigned cbCont(0);
   bool bCheckFirstContByteForOverlongs;
   while (char8_t ch = *psz++) {
      if (cbCont) {
         // Ensure that the lead byte is really followed by cbCont trailing bytes.
         if (!is_trail_char(ch)) {
            return false;
         }
         --cbCont;
         if (bCheckFirstContByteForOverlongs) {
            // Detect an overlong due to unused bits in the lead byte and first continuation bytes.
            // See smc_aiOverlongDetectionMasks for more information on how this check works.
            if (!(ch & smc_aiOverlongDetectionMasks[cbCont])) {
               return false;
            }
            bCheckFirstContByteForOverlongs = false;
         }
      } else {
         // This should be a lead byte, and not the invalid 1111111x.
         if (is_trail_char(ch) || uint8_t(ch) >= 0xfe) {
            return false;
         }
         // Detect an overlong that would fit in a single character: 11000001 10yyyyyy should have
         // been encoded as 01yyyyyy.
         if (ch == char8_t(0xc1)) {
            return false;
         }
         cbCont = lead_char_to_codepoint_size(ch) - 1;
         // If no code point bits are used (1) in the lead byte, enable overlong detection for
         // the first continuation byte.
         bCheckFirstContByteForOverlongs = get_lead_char_codepoint_bits(ch, cbCont) == 0;
      }
   }
   return cbCont == 0;
}
/*static*/ bool utf8_traits::is_valid(char8_t const * pchBegin, char8_t const * pchEnd) {
   ABC_TRACE_FUNC(pchBegin, pchEnd);

   unsigned cbCont(0);
   bool bCheckFirstContByteForOverlongs;
   for (char8_t const * pch(pchBegin); pch < pchEnd; ++pch) {
      char8_t ch(*pch);
      if (cbCont) {
         // Ensure that the lead byte is really followed by cbCont trailing bytes.
         if (!is_trail_char(ch)) {
            return false;
         }
         --cbCont;
         if (bCheckFirstContByteForOverlongs) {
            // Detect an overlong due to unused bits in the lead byte and first continuation bytes.
            // See smc_aiOverlongDetectionMasks for more information on how this check works.
            if (!(ch & smc_aiOverlongDetectionMasks[cbCont])) {
               return false;
            }
            bCheckFirstContByteForOverlongs = false;
         }
      } else {
         // This should be a lead byte, and not the invalid 1111111x.
         if (is_trail_char(ch) || uint8_t(ch) >= 0xfe) {
            return false;
         }
         // Detect an overlong that would fit in a single character: 11000001 10yyyyyy should have
         // been encoded as 01yyyyyy.
         if (ch == char8_t(0xc1)) {
            return false;
         }
         cbCont = lead_char_to_codepoint_size(ch) - 1;
         // If no code point bits are used (1) in the lead byte, enable overlong detection for
         // the first continuation byte.
         bCheckFirstContByteForOverlongs = get_lead_char_codepoint_bits(ch, cbCont) == 0;
      }
   }
   return cbCont == 0;
}


/*static*/ size_t utf8_traits::size_in_chars(char8_t const * psz) {
   ABC_TRACE_FUNC(psz);

   char8_t const * pch(psz);
   while (*pch) {
      ++pch;
   }
   return size_t(pch - psz);
}


/*static*/ size_t utf8_traits::size_in_codepoints(
   char8_t const * pchBegin, char8_t const * pchEnd
) {
   ABC_TRACE_FUNC(pchBegin, pchEnd);

   size_t ccp(0);
   // Count a single code point for each lead byte, skipping over trailing bytes.
   for (char8_t const * pch(pchBegin); pch < pchEnd; pch += lead_char_to_codepoint_size(*pch)) {
      ++ccp;
   }
   return ccp;
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::utf16_traits


namespace abc {
namespace text {

/*static*/ bool utf16_traits::is_valid(char16_t const * psz) {
   ABC_TRACE_FUNC(psz);

   bool bExpectTailSurrogate(false);
   while (char16_t ch = *psz++) {
      bool bSurrogate(is_surrogate(ch));
      if (bSurrogate) {
         bool bTrailSurrogate(is_trail_char(ch));
         // If this is a lead surrogate and we were expecting a trail, or this is a trail surrogate
         // but we’re not in a surrogate, this character is invalid.
         if (bTrailSurrogate != bExpectTailSurrogate) {
            return false;
         }
         bExpectTailSurrogate = !bTrailSurrogate;
      } else if (bExpectTailSurrogate) {
         // We were expecting a trail surrogate, but this is not a surrogate at all.
         return false;
      }
   }
   // Cannot end in the middle of a surrogate.
   return !bExpectTailSurrogate;
}
/*static*/ bool utf16_traits::is_valid(char16_t const * pchBegin, char16_t const * pchEnd) {
   ABC_TRACE_FUNC(pchBegin, pchEnd);

   bool bExpectTailSurrogate(false);
   for (char16_t const * pch(pchBegin); pch < pchEnd; ++pch) {
      char16_t ch(*pch);
      bool bSurrogate(is_surrogate(ch));
      if (bSurrogate) {
         bool bTrailSurrogate(is_trail_char(ch));
         // If this is a lead surrogate and we were expecting a trail, or this is a trail surrogate
         // but we’re not in a surrogate, this character is invalid.
         if (bTrailSurrogate != bExpectTailSurrogate) {
            return false;
         }
         bExpectTailSurrogate = !bTrailSurrogate;
      } else if (bExpectTailSurrogate) {
         // We were expecting a trail surrogate, but this is not a surrogate at all.
         return false;
      }
   }
   // Cannot end in the middle of a surrogate.
   return !bExpectTailSurrogate;
}


/*static*/ size_t utf16_traits::size_in_chars(char16_t const * psz) {
   ABC_TRACE_FUNC(psz);

   char16_t const * pch(psz);
   while (*pch) {
      ++pch;
   }
   return size_t(pch - psz);
}


/*static*/ size_t utf16_traits::size_in_codepoints(
   char16_t const * pchBegin, char16_t const * pchEnd
) {
   ABC_TRACE_FUNC(pchBegin, pchEnd);

   size_t ccp(0);
   for (char16_t const * pch(pchBegin); pch < pchEnd; pch += lead_char_to_codepoint_size(*pch)) {
      ++ccp;
   }
   return ccp;
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

