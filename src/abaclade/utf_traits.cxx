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
// In the end, the leading byte is treated like this:
//
//    ┌─────────────┬──────────────┬────────┐
//    │ 7 6 5 4 3 2 │       1      │    0   │
//    ├─────────────┼──────────────┼────────┤
//    │ byte  index │ nibble index │ unused │
//    └─────────────┴──────────────┴────────┘
//
// See utf8_traits::leading_to_cont_length() for the actual code accessing this array.
uint8_t const utf8_traits::smc_acbConts[] = {
   // 0xxxxxxx
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   // 10xxxxxx – invalid (cannot be start of a sequence), so just skip it.
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   // 110xxxxx
   0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
   // 1110xxxx
   0x22, 0x22, 0x22, 0x22,
   // 11110xxx
   0x33, 0x33,
   // These are either overlong (code points encoded using more bytes than necessary) or invalid
   // (the resulting symbol would be out of Unicode code point range).
   // 111110xx
   0x44,
   // 1111110x same as above, and 1111111x is invalid (not UTF-8), so just skip it.
   0x05
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


/*static*/ size_t utf8_traits::cp_len(char8_t const * pchBegin, char8_t const * pchEnd) {
   ABC_TRACE_FUNC(pchBegin, pchEnd);

   size_t ccp(0);
   // Count a single code point for each leading byte, skipping over trailing bytes.
   for (char8_t const * pch(pchBegin); pch < pchEnd; pch += 1 + leading_to_cont_length(*pch)) {
      ++ccp;
   }
   return ccp;
}


/*static*/ unsigned utf8_traits::from_utf32(char32_t ch32, char8_t * pchDst) {
   ABC_TRACE_FUNC(ch32, pchDst);

   char8_t const * pchDst0(pchDst);
   // Compute the length of this sequence.
   unsigned cbCont;
   if (ch32 <= 0x00007f) {
      cbCont = 0;
   } else if (ch32 <= 0x0007ff) {
      cbCont = 1;
   } else if (ch32 <= 0x00ffff) {
      cbCont = 2;
   } else {
      cbCont = 3;
   }
   // Since each trailing byte can take 6 bits, the remaining ones (after >> 6 * cbCont) make up
   // what goes in the leading byte.
   *pchDst++ = cont_length_to_seq_indicator(cbCont) | char8_t(ch32 >> 6 * cbCont);
   while (cbCont--) {
      *pchDst++ = char8_t(0x80 | ((ch32 >> 6 * cbCont) & 0x3f));
   }
   return unsigned(pchDst - pchDst0);
}


/*static*/ bool utf8_traits::is_valid(char8_t const * psz) {
   ABC_TRACE_FUNC(psz);

   unsigned cbCont(0);
   bool bCheckFirstContByteForOverlongs;
   while (char8_t ch = *psz++) {
      if (cbCont) {
         // Ensure that the leading byte is really followed by cbCont trailing bytes.
         if ((ch & 0xc0) != 0x80) {
            return false;
         }
         --cbCont;
         if (bCheckFirstContByteForOverlongs) {
            // Detect an overlong due to unused bits in the leading and first continuation bytes.
            // See smc_aiOverlongDetectionMasks for more information on how this check works.
            if (!(ch & smc_aiOverlongDetectionMasks[cbCont])) {
               return false;
            }
            bCheckFirstContByteForOverlongs = false;
         }
      } else {
         // This should be a leading byte, and not the invalid 1111111x.
         if ((ch & 0xc0) == 0x80 || uint8_t(ch) >= 0xfe) {
            return false;
         }
         // Detect an overlong that would fit in a single character: 11000001 10yyyyyy should have
         // been encoded as 01yyyyyy.
         if (ch == char8_t(0xc1)) {
            return false;
         }
         cbCont = leading_to_cont_length(ch);
         // If no code point bits are used (1) in the leading byte, enable overlong detection for
         // the first continuation byte.
         bCheckFirstContByteForOverlongs = get_leading_cp_bits(ch, cbCont) == 0;
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
         // Ensure that the leading byte is really followed by cbCont trailing bytes.
         if ((ch & 0xc0) != 0x80) {
            return false;
         }
         --cbCont;
         if (bCheckFirstContByteForOverlongs) {
            // Detect an overlong due to unused bits in the leading and first continuation bytes.
            // See smc_aiOverlongDetectionMasks for more information on how this check works.
            if (!(ch & smc_aiOverlongDetectionMasks[cbCont])) {
               return false;
            }
            bCheckFirstContByteForOverlongs = false;
         }
      } else {
         // This should be a leading byte, and not the invalid 1111111x.
         if ((ch & 0xc0) == 0x80 || uint8_t(ch) >= 0xfe) {
            return false;
         }
         // Detect an overlong that would fit in a single character: 11000001 10yyyyyy should have
         // been encoded as 01yyyyyy.
         if (ch == char8_t(0xc1)) {
            return false;
         }
         cbCont = leading_to_cont_length(ch);
         // If no code point bits are used (1) in the leading byte, enable overlong detection for
         // the first continuation byte.
         bCheckFirstContByteForOverlongs = get_leading_cp_bits(ch, cbCont) == 0;
      }
   }
   return cbCont == 0;
}


/*static*/ size_t utf8_traits::str_len(char8_t const * psz) {
   ABC_TRACE_FUNC(psz);

   char8_t const * pch(psz);
   while (*pch) {
      ++pch;
   }
   return size_t(pch - psz);
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::utf16_traits


namespace abc {
namespace text {

/*static*/ size_t utf16_traits::cp_len(char16_t const * pchBegin, char16_t const * pchEnd) {
   ABC_TRACE_FUNC(pchBegin, pchEnd);

   size_t ccp(0);
   // The & 0xfc00 will cause 0xdc00 characters to be treated like single invalid characters, since
   // they cannot occur before the 0xd800 that will cause them to be skipped.
   for (char16_t const * pch(pchBegin); pch < pchEnd; pch += 1 + ((*pch & 0xfc00) == 0xd800)) {
      ++ccp;
   }
   return ccp;
}


/*static*/ unsigned utf16_traits::from_utf32(char32_t ch32, char16_t * pchDst) {
   ABC_TRACE_FUNC(ch32, pchDst);

   if (ch32 <= 0x00ffff) {
      // The code point fits in a single UTF-16 character.
      pchDst[0] = char16_t(ch32);
      return 1;
   } else {
      // The code point requires two UTF-16 characters: generate a surrogate pair.
      ch32 -= 0x10000;
      pchDst[0] = char16_t(0xd800 | ((ch32 & 0x0ffc00) >> 10));
      pchDst[1] = char16_t(0xdc00 |  (ch32 & 0x0003ff)       );
      return 2;
   }
}


/*static*/ bool utf16_traits::is_valid(char16_t const * psz) {
   ABC_TRACE_FUNC(psz);

   bool bExpectTailSurrogate(false);
   while (char16_t ch = *psz++) {
      // Select lead and trail surrogates (11011xyy yyyyyyyy).
      bool bSurrogate((ch & 0xf800) == 0xd800);
      if (bSurrogate) {
         // Extract the x (see above) using 00000100 00000000 as mask.
         bool bTailSurrogate((ch & 0x0400) != 0);
         // If this is a lead surrogate and we were expecting a trail, or this is a trail surrogate
         // but we’re not in a surrogate, this character is invalid.
         if (bTailSurrogate != bExpectTailSurrogate) {
            return false;
         }
         bExpectTailSurrogate = !bTailSurrogate;
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
      // Select lead and trail surrogates (11011xyy yyyyyyyy).
      bool bSurrogate((ch & 0xf800) == 0xd800);
      if (bSurrogate) {
         // Extract the x (see above) using 00000100 00000000 as mask.
         bool bTailSurrogate((ch & 0x0400) != 0);
         // If this is a lead surrogate and we were expecting a trail, or this is a trail surrogate
         // but we’re not in a surrogate, this character is invalid.
         if (bTailSurrogate != bExpectTailSurrogate) {
            return false;
         }
         bExpectTailSurrogate = !bTailSurrogate;
      } else if (bExpectTailSurrogate) {
         // We were expecting a trail surrogate, but this is not a surrogate at all.
         return false;
      }
   }
   // Cannot end in the middle of a surrogate.
   return !bExpectTailSurrogate;
}


/*static*/ size_t utf16_traits::str_len(char16_t const * psz) {
   ABC_TRACE_FUNC(psz);

   char16_t const * pch(psz);
   while (*pch) {
      ++pch;
   }
   return size_t(pch - psz);
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

