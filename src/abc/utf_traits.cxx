/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abc/core.hxx>
#include <abc/utf_traits.hxx>
#include <abc/vector.hxx>
#include <abc/trace.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text globals

namespace abc {

namespace text {

/** Builds a failure restart table for searches using the Knuth-Morris-Pratt algorithm. See
[DOC:1502 KMP substring search] for how this is built and used.

TODO: comment signature.
*/
template <typename C>
static void _build_failure_restart_table(
   C const * pchNeedleBegin, C const * pchNeedleEnd, mvector<size_t> * pvcchFailNext
) {
   ABC_TRACE_FN((pchNeedleBegin, pchNeedleEnd, pvcchFailNext));

   // TODO: FIXME: this should be set_size(), not set_capacity().
   pvcchFailNext->set_capacity(size_t(pchNeedleEnd - pchNeedleBegin), false);
   size_t * pcchFailNext0(pvcchFailNext->data());

   // The earliest repetition of a non-first character can only occur on the fourth character, so
   // start by skipping two characters and storing two zeroes for them, then the first iteration
   // will also always store an additional zero and consume one more character.
   C const * pchNeedle(pchNeedleBegin + 2),
           * pchRestart(pchNeedleBegin);
   size_t * pcchFailNext(pcchFailNext0 + 2);
   pcchFailNext0[0] = 0;
   pcchFailNext0[1] = 0;
   size_t ichRestart(0);
   while (pchNeedle < pchNeedleEnd) {
      // Store the current failure restart index, or 0 if the previous character was the third or
      // was not a match.
      *pcchFailNext++ = ichRestart;
      if (*pchNeedle++ == *pchRestart) {
         // Another match: move the restart to the next character.
         ++ichRestart;
         ++pchRestart;
      } else if (ichRestart > 0) {
         // End of a match: restart self-matching from index 0.
         ichRestart = 0;
         pchRestart = pchNeedleBegin;
      }
   }
}

} //namespace text

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::utf8_traits


namespace abc {

namespace text {

char8_t const utf8_traits::bom[] = { '\xef', '\xbb', '\xbf' };

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
// See utf8_traits::leading_to_cont_length() for the actual code accessing this array.
uint8_t const utf8_traits::smc_acbConts[] = {
   // 0xxxxxxx
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   // 10xxxxxx - invalid (cannot be start of a sequence), so just skip it.
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



/*static*/ size_t utf8_traits::cp_len(char8_t const * pchBegin, char8_t const * pchEnd) {
   size_t ccp(0);
   // Count a single code point for each leading byte, skipping over trailing bytes.
   for (char8_t const * pch(pchBegin); pch < pchEnd; pch += 1 + leading_to_cont_length(*pch)) {
      ++ccp;
   }
   return ccp;
}


/*static*/ unsigned utf8_traits::from_utf32(char32_t ch32, char8_t * pchDst) {
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
   while (char8_t ch = *psz++) {
      // This should be a leading byte, and not the invalid 1111111x.
      if ((ch & 0xc0) == 0x80 || uint8_t(ch) >= 0xfe) {
         return false;
      }
      unsigned cbCont(leading_to_cont_length(ch));
      // Detect an overlong due to unused bits in the leading byte.
      if (!get_leading_cp_bits(ch, cbCont)) {
         return false;
      }
      // Ensure that the leading byte is really followed by cbCont trailing bytes.
      while (cbCont--) {
         // This condition includes == NUL: the unexpcted end of the string is a failure.
         if ((*psz++ & 0xc0) != 0x80) {
            return false;
         }
      }
   }
   return true;
}
/*static*/ bool utf8_traits::is_valid(char8_t const * pch, size_t cch) {
   while (cch--) {
      char8_t ch(*pch++);
      // This should be a leading byte, and not the invalid 1111111x.
      if ((ch & 0xc0) == 0x80 || uint8_t(ch) >= 0xfe) {
         return false;
      }
      unsigned cbCont(leading_to_cont_length(ch));
      // Ensure that the string has at least cbCont more bytes.
      if (cch < cbCont) {
         return false;
      }
      // Detect an overlong due to unused bits in the leading byte.
      if (!get_leading_cp_bits(ch, cbCont)) {
         return false;
      }
      // Ensure that the leading byte is really followed by cbCont trailing bytes.
      while (cbCont--) {
         if ((*pch++ & 0xc0) != 0x80) {
            return false;
         }
      }
   }
   return true;
}


/*static*/ char8_t const * utf8_traits::str_chr(
   char8_t const * pchHaystackBegin, char8_t const * pchHaystackEnd, char32_t chNeedle
) {
   if (chNeedle <= 0x00007f) {
      // The needle can be encoded as a single UTF-8 character, so this faster search can be used.
      char8_t ch8Needle(static_cast<char8_t>(chNeedle));
      for (char8_t const * pch(pchHaystackBegin); pch < pchHaystackEnd; ++pch) {
         if (*pch == ch8Needle) {
            return pch;
         }
      }
      return pchHaystackEnd;
   } else {
      // The needle is two or more UTF-8 characters, so take the slower approach.
      char8_t achNeedle[max_codepoint_length];
      from_utf32(chNeedle, achNeedle);
      return str_chr(pchHaystackBegin, pchHaystackEnd, achNeedle);
   }
}
/*static*/ char8_t const * utf8_traits::str_chr(
   char8_t const * pchHaystackBegin, char8_t const * pchHaystackEnd, char8_t const * pchNeedle
) {
   char8_t chNeedleLead(*pchNeedle);
   for (char8_t const * pch(pchHaystackBegin), * pchNext; pch < pchHaystackEnd; pch = pchNext) {
      char8_t ch(*pch);
      unsigned cbCont(leading_to_cont_length(ch));
      // Make the next iteration resume from the next code point.
      pchNext = pch + 1 /*ch*/ + cbCont;
      if (ch == chNeedleLead) {
         if (cbCont) {
            // The leading bytes match; check if the trailing ones do as well.
            char8_t const * pchCont(pch), * pchNeedleCont(pchNeedle);
            while (++pchCont < pchNext && *pchCont == *++pchNeedleCont) {
               ;
            }
            if (pchCont < pchNext) {
               continue;
            }
            // The leading and trailing bytes of pch and pchNeedle match: we found the needle.
         }
         return pch;
      }
   }
   return pchHaystackEnd;
}


/*static*/ char8_t const * utf8_traits::str_chr_r(
   char8_t const * pchHaystackBegin, char8_t const * pchHaystackEnd, char32_t chNeedle
) {
   if (chNeedle <= 0x00007f) {
      // The needle can be encoded as a single UTF-8 character, so this faster search can be used.
      char8_t ch8Needle(static_cast<char8_t>(chNeedle));
      for (char8_t const * pch(pchHaystackEnd); pch > pchHaystackBegin; ) {
         if (*--pch == ch8Needle) {
            return pch;
         }
      }
      return pchHaystackBegin;
   } else {
      // The needle is two or more UTF-8 characters; this means that we can’t do the fast backwards
      // scan above, so just do a regular substring reverse search.
      char8_t achNeedle[max_codepoint_length];
      unsigned cchSeq(from_utf32(chNeedle, achNeedle));
      return str_str_r(pchHaystackBegin, pchHaystackEnd, achNeedle, achNeedle + cchSeq);
   }
}


// Note for all overloads: not only sequences don’t matter when scanning for the first differing
// bytes, but once a pair of differing bytes is found, if they are part of a sequence, its start
// must have been the same, so only their absolute value matters; if they started a sequence, the
// first byte of a longer encoding (greater code point value) if greater than that of a shorter one.
/*static*/ int utf8_traits::str_cmp(char8_t const * psz1, char8_t const * psz2) {
   // This loop ends when there is bias (which includes psz2 being finished while there are still
   // characters in psz1) or psz1 is over.
   char8_t ch1;
   do {
      ch1 = *psz1++;
      char8_t ch2(*psz2++);
      if (ch1 > ch2) {
         return +1;
      } else if (ch1 < ch2) {
         return -1;
      }
   } while (ch1);
   return 0;
}
/*static*/ int utf8_traits::str_cmp(
   char8_t const * pch1, size_t cch1, char8_t const * pch2, size_t cch2
) {
   char8_t const * pch1End(pch1 + cch1), * pch2End(pch2 + cch2);
   while (pch1 < pch1End && pch2 < pch2End) {
      char8_t ch1(*pch1++), ch2(*pch2++);
      if (ch1 > ch2) {
         return +1;
      } else if (ch1 < ch2) {
         return -1;
      }
   }
   // If we’re still here, the longest string wins.
   if (cch1 > cch2) {
      return +1;
   } else if (cch1 < cch2) {
      return -1;
   } else {
      return 0;
   }
}


/*static*/ size_t utf8_traits::str_len(char8_t const * psz) {
   char8_t const * pch(psz);
   while (*pch) {
      ++pch;
   }
   return size_t(pch - psz);
}


/*static*/ char8_t const * utf8_traits::str_str(
   char8_t const * pchHaystackBegin, char8_t const * pchHaystackEnd,
   char8_t const * pchNeedleBegin, char8_t const * pchNeedleEnd
) {
   if (!(pchNeedleEnd - pchNeedleBegin)) {
      // No needle, so just return the beginning of the haystack.
      return pchHaystackBegin;
   }
   char8_t const * pchHaystack(pchHaystackBegin),
                 * pchNeedle(pchNeedleBegin);
   try {
      /** DOC:1502 KMP substring search

      This is an implementation of the Knuth-Morris-Pratt algorithm.

      Examples of the contents of pcchFailNext after the block below for different needles:

      ┌──────────────┬───┬─────┬─────┬───────┬───────┬───────────────┬─────────────┐
      │ Needle index │ 0 │ 0 1 │ 0 1 │ 0 1 2 │ 0 1 2 │ 0 1 2 3 4 5 6 │ 0 1 2 3 4 5 │
      ├──────────────┼───┼─────┼─────┼───────┼───────┼───────────────┼─────────────┤
      │ pchNeedle    │ A │ A A │ A B │ A A A │ A A B │ A B A A B A C │ A B A B C D │
      │ pcchFailNext │ 0 │ 0 0 │ 0 0 │ 0 0 0 │ 0 0 0 │ 0 0 0 0 1 2 3 │ 0 0 0 1 2 0 │
      └──────────────┴───┴─────┴─────┴───────┴───────┴───────────────┴─────────────┘
      */

      // Build the failure restart table.
      smvector<size_t, 64> vcchFailNext;
      _build_failure_restart_table(pchNeedleBegin, pchNeedleEnd, &vcchFailNext);
      size_t const * pcchFailNext(vcchFailNext.data());

      size_t iFailNext(0);
      while (pchHaystack < pchHaystackEnd) {
         if (*pchHaystack == *pchNeedle) {
            ++pchNeedle;
            if (pchNeedle == pchNeedleEnd) {
               // The needle was exhausted, which means that all its characters were matched in the
               // haystack: we found the needle.
               return pchHaystack - iFailNext;
            }
            // Move to the next character and advance the index in pcchFailNext.
            ++pchHaystack;
            ++iFailNext;
         } else if (iFailNext > 0) {
            // The current character ends the match sequence; use pcchFailNext[iFailNext] to see how
            // much into the needle we can retry matching characters.
            iFailNext = pcchFailNext[iFailNext];
            pchNeedle = pchNeedleBegin + iFailNext;
         } else {
            // Not a match, and no restart point: we’re out of options to match this character, so
            // consider it not-a-match and move past it.
            ++pchHaystack;
         }
      }
   } catch (std::bad_alloc const &) {
      // Could not allocate enough memory for the failure restart table: fall back to a trivial (and
      // potentially slower) substring search.
      char8_t chFirst(*pchNeedleBegin);
      for (; pchHaystack < pchHaystackEnd; ++pchHaystack) {
         if (*pchHaystack == chFirst) {
            char8_t const * pchHaystackMatch(pchHaystack);
            pchNeedle = pchNeedleBegin;
            while (++pchNeedle < pchNeedleEnd && *++pchHaystackMatch == *pchNeedle) {
               ;
            }
            if (pchNeedle >= pchNeedleEnd) {
               // The needle was exhausted, which means that all its characters were matched in the
               // haystack: we found the needle.
               return pchHaystack;
            }
         }
      }
   }
   return pchHaystackEnd;
}


/*static*/ char8_t const * utf8_traits::str_str_r(
   char8_t const * pchHaystackBegin, char8_t const * pchHaystackEnd,
   char8_t const * pchNeedleBegin, char8_t const * pchNeedleEnd
) {
   // TODO: implement this!
   return pchHaystackEnd;
}

} //namespace text

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::utf16_traits


namespace abc {

namespace text {

char16_t const utf16_traits::bom[] = {
   0xfeff
};



/*static*/ size_t utf16_traits::cp_len(char16_t const * pchBegin, char16_t const * pchEnd) {
   size_t ccp(0);
   // The & 0xfc00 will cause 0xdc00 characters to be treated like single invalid characters, since
   // they cannot occur before the 0xd800 that will cause them to be skipped.
   for (char16_t const * pch(pchBegin); pch < pchEnd; pch += 1 + ((*pch & 0xfc00) == 0xd800)) {
      ++ccp;
   }
   return ccp;
}


/*static*/ unsigned utf16_traits::from_utf32(char32_t ch32, char16_t * pchDst) {
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
   while (char16_t ch = *psz++) {
      switch (ch & 0xfc00) {
         case 0xd800: {
            char32_t ch32(char32_t(ch & 0x03ff) << 10);
            // Surrogate first half; expect at least one more characer.
            if (!(ch = *psz++)) {
               return false;
            }
            // The next character must be a surrogate second half.
            if ((ch & 0xfc00) != 0xdc00) {
               return false;
            }
            // The resulting character must be valid UTF-32.
            ch32 = (ch32 | (ch & 0x03ff)) + 0x10000;
            if (utf32_traits::is_valid(ch32)) {
               break;
            }
            // Fall through.
         }
         case 0xdc00:
            // Invalid second half of a surrogate pair.
            return false;
      }
   }
   return true;
}
/*static*/ bool utf16_traits::is_valid(char16_t const * pch, size_t cch) {
   while (cch--) {
      char16_t ch(*pch++);
      switch (ch & 0xfc00) {
         case 0xd800: {
            // Surrogate first half; expect at least one more characer.
            if (!cch--) {
               return false;
            }
            char32_t ch32(char32_t(ch & 0x03ff) << 10);
            ch = *pch++;
            // The next character must be a surrogate second half.
            if ((ch & 0xfc00) != 0xdc00) {
               return false;
            }
            // The resulting character must be valid UTF-32.
            ch32 = (ch32 | (ch & 0x03ff)) + 0x10000;
            if (utf32_traits::is_valid(ch32)) {
               break;
            }
            // Fall through.
         }
         case 0xdc00:
            // Invalid second half of a surrogate pair.
            return false;
      }
   }
   return true;
}


/*static*/ char16_t const * utf16_traits::str_chr(
   char16_t const * pchHaystackBegin, char16_t const * pchHaystackEnd, char32_t chNeedle
) {
   if (chNeedle <= 0x00ffff) {
      // The needle can be encoded as a single UTF-16 character, so this faster search can be used.
      char16_t ch16Needle(static_cast<char16_t>(chNeedle));
      for (char16_t const * pch(pchHaystackBegin); pch < pchHaystackEnd; ++pch) {
         if (*pch == ch16Needle) {
            return pch;
         }
      }
      return pchHaystackEnd;
   } else {
      // The needle is two UTF-16 characters, so take the slower approach.
      char16_t achNeedle[max_codepoint_length];
      from_utf32(chNeedle, achNeedle);
      return str_chr(pchHaystackBegin, pchHaystackEnd, achNeedle);
   }
}
/*static*/ char16_t const * utf16_traits::str_chr(
   char16_t const * pchHaystackBegin, char16_t const * pchHaystackEnd, char16_t const * pchNeedle
) {
   // In UTF-16, there’s always at most two characters per code point.
   char16_t chNeedle0(pchNeedle[0]);
   // We only have a second character if the first is a surrogate first half.
   char16_t chNeedle1((chNeedle0 & 0xfc00) == 0xd800 ? pchNeedle[1] : U16CL('\0'));
   // The bounds of this loop are safe: since we assume that both strings are valid UTF-16, if
   // pch[0] == chNeedle0 and chNeedle1 != NUL then pch[1] must be accessible.
   for (char16_t const * pch(pchHaystackBegin); pch < pchHaystackEnd; ++pch) {
      if (pch[0] == chNeedle0 && (!chNeedle1 || pch[1] == chNeedle1)) {
         return pch;
      }
   }
   return pchHaystackEnd;
}


/*static*/ char16_t const * utf16_traits::str_chr_r(
   char16_t const * pchHaystackBegin, char16_t const * pchHaystackEnd, char32_t chNeedle
) {
   if (chNeedle <= 0x00ffff) {
      // The needle can be encoded as a single UTF-16 character, so this faster search can be used.
      char16_t ch16Needle(static_cast<char16_t>(chNeedle));
      for (char16_t const * pch(pchHaystackEnd); pch > pchHaystackBegin; ) {
         if (*--pch == ch16Needle) {
            return pch;
         }
      }
      return pchHaystackBegin;
   } else {
      // The needle is two UTF-16 characters, so take the slower approach.
      char16_t achNeedle[max_codepoint_length];
      from_utf32(chNeedle, achNeedle);
      return str_chr_r(pchHaystackBegin, pchHaystackEnd, achNeedle);
   }
}
/*static*/ char16_t const * utf16_traits::str_chr_r(
   char16_t const * pchHaystackBegin, char16_t const * pchHaystackEnd, char16_t const * pchNeedle
) {
   // In UTF-16, there’s always at most two characters per code point.
   // Notice that this function is very much a mirrored version of str_chr(), so even the needle is
   // stored mirrored in chNeedle0/1: chNeedle1 is always used, and in case of a surrogate we use
   // chNeedle0 as well.
   char16_t chNeedle0((pchNeedle[0] & 0xfc00) == 0xd800 ? pchNeedle[0] : U16CL('\0'));
   char16_t chNeedle1(pchNeedle[chNeedle0 ? 1 : 0]);
   // The bounds of this loop are safe: since we assume that both strings are valid UTF-16, if
   // pch[0] == chNeedle1 and chNeedle0 != NUL then pch[-1] must be accessible.
   for (char16_t const * pch(pchHaystackEnd); pch > pchHaystackBegin; ) {
      if (*--pch == chNeedle1 && (!chNeedle0 || *(pch - 1) == chNeedle0)) {
         return pch;
      }
   }
   return pchHaystackBegin;
}


/*static*/ int utf16_traits::str_cmp(char16_t const * psz1, char16_t const * psz2) {
   char16_t ch1;
   do {
      ch1 = *psz1++;
      char16_t ch2(*psz2++);
      // Surrogates prevent us from just comparing the absolute char16_t values.
      bool bSurr1((ch1 & 0xf800) == 0xd800), bSurr2((ch2 & 0xf800) == 0xd800);
      if (bSurr1 == bSurr2) {
         // The characters are both regular or surrogates. Since a difference in surrogate firsts
         // generates bias, we only get to compare seconds if the firsts were equal.
         if (ch1 > ch2) {
            return +1;
         } else if (ch1 < ch2) {
            return -1;
         }
      } else if (bSurr1) {
         // If ch1 is a surrogate and ch2 is not, ch1 > ch2.
         return +1;
      } else /*if (bSurr2)*/ {
         // If ch2 is a surrogate and ch1 is not, ch1 < ch2.
         return -1;
      }
   } while (ch1);
   return 0;
}
/*static*/ int utf16_traits::str_cmp(
   char16_t const * pch1, size_t cch1, char16_t const * pch2, size_t cch2
) {
   char16_t const * pch1End(pch1 + cch1), * pch2End(pch2 + cch2);
   while (pch1 < pch1End && pch2 < pch2End) {
      char16_t ch1(*pch1++), ch2(*pch2++);
      // Surrogates mess with the ability to just compare the absolute char16_t value.
      bool bSurr1((ch1 & 0xf800) == 0xd800), bSurr2((ch2 & 0xf800) == 0xd800);
      if (bSurr1 == bSurr2) {
         // The characters are both regular or surrogates. Since a difference in surrogate firsts
         // generates bias, we only get to compare seconds if the firsts were equal.
         if (ch1 > ch2) {
            return +1;
         } else if (ch1 < ch2) {
            return -1;
         }
      } else if (bSurr1) {
         // If ch1 is a surrogate and ch2 is not, ch1 > ch2.
         return +1;
      } else /*if (bSurr2)*/ {
         // If ch2 is a surrogate and ch1 is not, ch1 < ch2.
         return -1;
      }
   }
   // If we’re still here, the longest string wins.
   if (cch1 > cch2) {
      return +1;
   } else if (cch1 < cch2) {
      return -1;
   } else {
      return 0;
   }
}


/*static*/ size_t utf16_traits::str_len(char16_t const * psz) {
   char16_t const * pch(psz);
   while (*pch) {
      ++pch;
   }
   return size_t(pch - psz);
}


/*static*/ char16_t const * utf16_traits::str_str(
   char16_t const * pchHaystackBegin, char16_t const * pchHaystackEnd,
   char16_t const * pchNeedleBegin, char16_t const * pchNeedleEnd
) {
   // TODO: redo using lookup table.
   size_t cchNeedle(size_t(pchNeedleEnd - pchNeedleBegin));
   if (!cchNeedle) {
      // No needle, so just return the whole haystack.
      return pchHaystackBegin;
   }
   // Back pchHaystackEnd up by cchNeedle - 1 characters, because when we have fewer than that many
   // characters we already know that the needle cannot be in the haystack.
   char16_t const * pchHaystackEndN(pchHaystackEnd - (cchNeedle - 1));
   char16_t chFirst(*pchNeedleBegin);
   for (
      char16_t const * pchHaystack(pchHaystackBegin); pchHaystack < pchHaystackEndN; ++pchHaystack
   ) {
      if (*pchHaystack == chFirst) {
         char16_t const * pchHaystack2(pchHaystack), * pchNeedle(pchNeedleBegin);
         while (++pchNeedle < pchNeedleEnd && *++pchHaystack2 == *pchNeedle) {
            ;
         }
         if (pchNeedle >= pchNeedleEnd) {
            // The needle was exhausted, which means that all its characters, were matched in the
            // haystack: we found the needle.
            return pchHaystack;
         }
      }
   }
   return pchHaystackEnd;
}


/*static*/ char16_t const * utf16_traits::str_str_r(
   char16_t const * pchHaystackBegin, char16_t const * pchHaystackEnd,
   char16_t const * pchNeedleBegin, char16_t const * pchNeedleEnd
) {
   // TODO: implement this!
   return pchHaystackEnd;
}

} //namespace text

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::utf32_traits


namespace abc {

namespace text {

char32_t const utf32_traits::bom[] = {
   0x00feff
};



/*static*/ bool utf32_traits::is_valid(char32_t const * psz) {
   while (char32_t ch = *psz++) {
      if (!is_valid(ch)) {
         return false;
      }
   }
   return true;
}
/*static*/ bool utf32_traits::is_valid(char32_t const * pch, size_t cch) {
   while (cch--) {
      if (!is_valid(*pch++)) {
         return false;
      }
   }
   return true;
}


/*static*/ char32_t const * utf32_traits::str_chr(
   char32_t const * pchHaystackBegin, char32_t const * pchHaystackEnd, char32_t chNeedle
) {
   for (char32_t const * pch(pchHaystackBegin); pch < pchHaystackEnd; ++pch) {
      if (*pch == chNeedle) {
         return pch;
      }
   }
   return pchHaystackEnd;
}


/*static*/ char32_t const * utf32_traits::str_chr_r(
   char32_t const * pchHaystackBegin, char32_t const * pchHaystackEnd, char32_t chNeedle
) {
   for (char32_t const * pch(pchHaystackEnd); pch > pchHaystackBegin; ) {
      if (*--pch == chNeedle) {
         return pch;
      }
   }
   return pchHaystackBegin;
}


/*static*/ int utf32_traits::str_cmp(char32_t const * psz1, char32_t const * psz2) {
   // This loop ends when there is bias (which includes psz2 being finished while there are still
   // characters in psz1) or psz1 is over.
   char32_t ch1;
   do {
      ch1 = *psz1++;
      char32_t ch2(*psz2++);
      if (ch1 > ch2) {
         return +1;
      } else if (ch1 < ch2) {
         return -1;
      }
   } while (ch1);
   return 0;
}
/*static*/ int utf32_traits::str_cmp(
   char32_t const * pch1, size_t cch1, char32_t const * pch2, size_t cch2
) {
   char32_t const * pch1End(pch1 + cch1), * pch2End(pch2 + cch2);
   while (pch1 < pch1End && pch2 < pch2End) {
      char32_t ch1(*pch1++), ch2(*pch2++);
      if (ch1 > ch2) {
         return +1;
      } else if (ch1 < ch2) {
         return -1;
      }
   }
   // If we’re still here, the longest string wins.
   if (cch1 > cch2) {
      return +1;
   } else if (cch1 < cch2) {
      return -1;
   } else {
      return 0;
   }
}


/*static*/ size_t utf32_traits::str_len(char32_t const * psz) {
   char32_t const * pch(psz);
   while (*pch) {
      ++pch;
   }
   return size_t(pch - psz);
}


/*static*/ char32_t const * utf32_traits::str_str(
   char32_t const * pchHaystackBegin, char32_t const * pchHaystackEnd,
   char32_t const * pchNeedleBegin, char32_t const * pchNeedleEnd
) {
   // TODO: redo using lookup table.
   size_t cchNeedle(size_t(pchNeedleEnd - pchNeedleBegin));
   if (!cchNeedle) {
      // No needle, so just return the whole haystack.
      return pchHaystackBegin;
   }
   // Back pchHaystackEnd up by cchNeedle - 1 characters, because when we have fewer than that many
   // characters we already know that the needle cannot be in the haystack.
   char32_t const * pchHaystackEndN(pchHaystackEnd - (cchNeedle - 1));
   char32_t chFirst(*pchNeedleBegin);
   for (
      char32_t const * pchHaystack(pchHaystackBegin); pchHaystack < pchHaystackEndN; ++pchHaystack
   ) {
      if (*pchHaystack == chFirst) {
         char32_t const * pchHaystack2(pchHaystack), * pchNeedle(pchNeedleBegin);
         while (++pchNeedle < pchNeedleEnd && *++pchHaystack2 == *pchNeedle) {
            ;
         }
         if (pchNeedle >= pchNeedleEnd) {
            // The needle was exhausted, which means that all its characters were matched in the
            // haystack: we found the needle.
            return pchHaystack;
         }
      }
   }
   return pchHaystackEnd;
}


/*static*/ char32_t const * utf32_traits::str_str_r(
   char32_t const * pchHaystackBegin, char32_t const * pchHaystackEnd,
   char32_t const * pchNeedleBegin, char32_t const * pchNeedleEnd
) {
   // TODO: implement this!
   return pchHaystackEnd;
}

} //namespace text

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

