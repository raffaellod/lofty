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
// abc::text::utf8_str_traits


namespace abc {
namespace text {

/*static*/ bool utf8_str_traits::validate(
   char8_t const * pchBegin, char8_t const * pchEnd, bool bThrowOnErrors /*= false*/
) {
   ABC_TRACE_FUNC(pchBegin, pchEnd, bThrowOnErrors);

   for (char8_t const * pch(pchBegin); pch < pchEnd; ) {
      uint8_t const * pbSrcCpBegin(reinterpret_cast<uint8_t const *>(pch));
      char8_t ch(*pch++);
      // This should be a lead byte, and not the start of an overlong or an invalid lead byte.
      if (!utf8_char_traits::is_valid_lead_char(ch)) {
         if (bThrowOnErrors) {
            ABC_THROW(decode_error, (
               SL("invalid UTF-8 lead byte"), pbSrcCpBegin, pbSrcCpBegin + 1
            ));
         } else {
            return false;
         }
      }

      // If the lead byte is 111?0000, activate the detection logic for overlong encodings in the
      // nested for loop; see below for more info.
      bool bValidateOnBitsInFirstTrailByte((ch & 0xef) == 0xe0);

      // Ensure that these bits are 0 to detect encoded code points above
      // (11110)100 (10)00xxxx (10)yyyyyy (10)zzzzzz, which is the highest valid code point
      // 10000 xxxxyyyy yyzzzzzz.
      char8_t iFirstTrailByteOffValidityMask(ch == '\xf4' ? 0x30 : 0x00);

      for (unsigned cbTrail(utf8_char_traits::lead_char_to_codepoint_size(ch)); --cbTrail; ) {
         if (pch == pchEnd || !utf8_char_traits::is_trail_char(ch = *pch++)) {
            // The string ended prematurely when we were expecting more trail characters, or this is
            // not a trail character.
            if (bThrowOnErrors) {
               ABC_THROW(decode_error, (
                  SL("unexpected end of UTF-8 sequence"),
                  pbSrcCpBegin, reinterpret_cast<uint8_t const *>(pch)
               ));
            } else {
               return false;
            }
         }
         if (bValidateOnBitsInFirstTrailByte) {
            // Detect overlong encodings by detecting zeros in the lead byte and masking the first
            // trail byte with an “on” mask.
            static char8_t const sc_aiOverlongDetectionMasks[] = {
               // 1-character sequences cannot be overlongs.
               /* 1 */ 0,
               // 2-character overlongs are filtered out by utf8_char_traits::is_valid_lead_char().
               /* 2 */ 0,
               // Detect 11100000 100xxxxx …, overlong for 110xxxxx ….
               /* 3 */ 0x20,
               // Detect 11110000 1000xxxx …, overlong for 1110xxxx ….
               /* 4 */ 0x30
               // Longer overlongs are possible, but they require a lead byte that is filtered out
               // by utf8_char_traits::is_valid_lead_char().
            };
            if (!(ch & sc_aiOverlongDetectionMasks[cbTrail])) {
               if (bThrowOnErrors) {
                  ABC_THROW(decode_error, (
                     SL("overlong UTF-8 sequence"),
                     pbSrcCpBegin, reinterpret_cast<uint8_t const *>(pch)
                  ));
               } else {
                  return false;
               }
            }
            bValidateOnBitsInFirstTrailByte = false;
         }
         if (iFirstTrailByteOffValidityMask) {
            // If the “off” mask reveals a “1” bit, this trail byte is invalid.
            if (ch & iFirstTrailByteOffValidityMask) {
               if (bThrowOnErrors) {
                  ABC_THROW(decode_error, (
                     SL("UTF-8 sequence decoded into invalid code point"),
                     pbSrcCpBegin, reinterpret_cast<uint8_t const *>(pch)
                  ));
               } else {
                  return false;
               }
            }
            iFirstTrailByteOffValidityMask = 0;
         }
      }
   }
   return true;
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::utf16_str_traits


namespace abc {
namespace text {

/*static*/ bool utf16_str_traits::validate(
   char16_t const * pchBegin, char16_t const * pchEnd, bool bThrowOnErrors /*= false*/
) {
   ABC_TRACE_FUNC(pchBegin, pchEnd, bThrowOnErrors);

   bool bExpectTrailSurrogate(false);
   for (char16_t const * pch(pchBegin); pch < pchEnd; ++pch) {
      uint8_t const * pbSrcCpBegin(reinterpret_cast<uint8_t const *>(pch));
      char16_t ch(*pch);
      bool bSurrogate(utf16_char_traits::is_surrogate(ch));
      if (bSurrogate) {
         bool bTrailSurrogate(utf16_char_traits::is_trail_char(ch));
         // If this is a lead surrogate and we were expecting a trail, or this is a trail surrogate
         // but we’re not in a surrogate, this character is invalid.
         if (bTrailSurrogate != bExpectTrailSurrogate) {
            if (bThrowOnErrors) {
               ABC_THROW(decode_error, (
                  SL("invalid lone surrogate"), pbSrcCpBegin, pbSrcCpBegin + sizeof(char16_t)
               ));
            } else {
               return false;
            }
         }
         bExpectTrailSurrogate = !bTrailSurrogate;
      } else if (bExpectTrailSurrogate) {
         // We were expecting a trail surrogate, but this is not a surrogate at all.
         if (bThrowOnErrors) {
            ABC_THROW(decode_error, (
               SL("invalid lone lead surrogate"), pbSrcCpBegin, pbSrcCpBegin + sizeof(char16_t)
            ));
         } else {
            return false;
         }
      }
   }
   // Cannot end in the middle of a surrogate.
   return !bExpectTrailSurrogate;
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::host_str_traits


namespace abc {
namespace text {

/*static*/ void host_str_traits::_build_find_failure_restart_table(
   char_t const * pchNeedleBegin, char_t const * pchNeedleEnd, mvector<size_t> * pvcchFailNext
) {
   ABC_TRACE_FUNC(pchNeedleBegin, pchNeedleEnd, pvcchFailNext);

   pvcchFailNext->set_size(static_cast<size_t>(pchNeedleEnd - pchNeedleBegin));
   auto itNextFailNext(pvcchFailNext->begin());

   // The earliest repetition of a non-first character can only occur on the fourth character, so
   // start by skipping two characters and storing two zeros for them, then the first iteration will
   // also always store an additional zero and consume one more character.
   char_t const * pchNeedle(pchNeedleBegin + 2);
   char_t const * pchRestart(pchNeedleBegin);
   *itNextFailNext++ = 0;
   *itNextFailNext++ = 0;
   size_t ichRestart(0);
   while (pchNeedle < pchNeedleEnd) {
      // Store the current failure restart index, or 0 if the previous character was the third or
      // was not a match.
      *itNextFailNext++ = ichRestart;
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


/*static*/ int host_str_traits::compare(
   char_t const * pch1Begin, char_t const * pch1End,
   char_t const * pch2Begin, char_t const * pch2End
) {
   ABC_TRACE_FUNC(pch1Begin, pch1End, pch2Begin, pch2End);

   char_t const * pch1(pch1Begin), * pch2(pch2Begin);
   while (pch1 < pch1End && pch2 < pch2End) {
      char_t ch1(*pch1++), ch2(*pch2++);
#if ABC_HOST_UTF == 8
      // Note: not only don’t sequences matter when scanning for the first differing bytes, but once
      // a pair of differing bytes is found, if they are part of a sequence, its start must have
      // been the same, so only their absolute value matters; if they started a sequence, the first
      // byte of a longer encoding (greater code point value) if greater than that of a shorter one.
#elif ABC_HOST_UTF == 16 //if ABC_HOST_UTF == 8
      // Surrogates mess with the ability to just compare the absolute char16_t value.
      bool bIsSurrogate1(host_char_traits::is_surrogate(ch1));
      bool bIsSurrogate2(host_char_traits::is_surrogate(ch2));
      if (bIsSurrogate1 != bIsSurrogate2) {
         if (bIsSurrogate1) {
            // If ch1 is a surrogate and ch2 is not, ch1 > ch2.
            return +1;
         } else /*if (bIsSurrogate2)*/ {
            // If ch2 is a surrogate and ch1 is not, ch1 < ch2.
            return -1;
         }
      }
      // The characters are both regular or surrogates. Since a difference in lead surrogate
      // generates bias, we only get to compare trails if the leads were equal.
#endif //if ABC_HOST_UTF == 8 … elif ABC_HOST_UTF == 16
      if (ch1 > ch2) {
         return +1;
      } else if (ch1 < ch2) {
         return -1;
      }
   }
   // If we’re still here, the string that didn’t run out of characters wins.
   if (pch1 < pch1End) {
      return +1;
   } else if (pch2 < pch2End) {
      return -1;
   } else {
      return 0;
   }
}


/*static*/ char_t const * host_str_traits::find_char(
   char_t const * pchHaystackBegin, char_t const * pchHaystackEnd, char32_t chNeedle
) {
   ABC_TRACE_FUNC(pchHaystackBegin, pchHaystackEnd, chNeedle);

   if (chNeedle <= host_char_traits::max_single_char_codepoint) {
      // The needle can be encoded as a single character, so this faster search can be used.
      return find_char(pchHaystackBegin, pchHaystackEnd, static_cast<char_t>(chNeedle));
   } else {
      // The needle is two or more characters, so take the slower approach.
      char_t achNeedle[host_char_traits::max_codepoint_length];
      host_char_traits::codepoint_to_chars(chNeedle, achNeedle);
      return find_char(pchHaystackBegin, pchHaystackEnd, achNeedle);
   }
}
/*static*/ char_t const * host_str_traits::find_char(
   char_t const * pchHaystackBegin, char_t const * pchHaystackEnd, char_t const* pchNeedle
) {
   ABC_TRACE_FUNC(pchHaystackBegin, pchHaystackEnd, pchNeedle);

#if ABC_HOST_UTF == 8
   char8_t chNeedleLead(*pchNeedle);
   for (char8_t const * pch(pchHaystackBegin), * pchNext; pch < pchHaystackEnd; pch = pchNext) {
      char8_t ch(*pch);
      unsigned cbCp(host_char_traits::lead_char_to_codepoint_size(ch));
      // Make the next iteration resume from the next code point.
      pchNext = pch + cbCp;
      if (ch == chNeedleLead) {
         if (--cbCp) {
            // The lead bytes match; check if the trailing ones do as well.
            char8_t const * pchCont(pch), * pchNeedleCont(pchNeedle);
            while (++pchCont < pchNext && *pchCont == *++pchNeedleCont) {
               ;
            }
            if (pchCont < pchNext) {
               continue;
            }
            // The lead and trailing bytes of pch and pchNeedle match: we found the needle.
         }
         return pch;
      }
   }
#elif ABC_HOST_UTF == 16 //if ABC_HOST_UTF == 8
   // In UTF-16, there’s always at most two characters per code point.
   char16_t chNeedle0(pchNeedle[0]);
   // We only have a second character if the first is a lead surrogate. Using NUL as a special value
   // is safe, because if this is a surrogate, the tail surrogate cannot be NUL.
   char16_t chNeedle1(host_char_traits::is_lead_surrogate(chNeedle0) ? pchNeedle[1] : host_char(0));
   // The bounds of this loop are safe: since we assume that both strings are valid UTF-16, if
   // pch[0] == chNeedle0 and chNeedle1 != NUL then pch[1] must be accessible.
   for (char16_t const * pch(pchHaystackBegin); pch < pchHaystackEnd; ++pch) {
      if (pch[0] == chNeedle0 && (!chNeedle1 || pch[1] == chNeedle1)) {
         return pch;
      }
   }
#endif //if ABC_HOST_UTF == 8 … elif ABC_HOST_UTF == 16
   return pchHaystackEnd;
}


/*static*/ char_t const * host_str_traits::find_char_last(
   char_t const * pchHaystackBegin, char_t const * pchHaystackEnd, char32_t chNeedle
) {
   ABC_TRACE_FUNC(pchHaystackBegin, pchHaystackEnd, chNeedle);

   if (chNeedle <= host_char_traits::max_single_char_codepoint) {
      // The needle can be encoded as a single character, so this faster search can be used.
      return find_char_last(pchHaystackBegin, pchHaystackEnd, static_cast<char_t>(chNeedle));
   } else {
      // The needle is two or more characters; this means that we can’t do the fast backwards scan
      // above, so just do a regular substring reverse search.
      char_t achNeedle[host_char_traits::max_codepoint_length];
      return find_substr_last(
         pchHaystackBegin, pchHaystackEnd,
         achNeedle, host_char_traits::codepoint_to_chars(chNeedle, achNeedle)
      );
   }
}


/*static*/ char_t const * host_str_traits::find_substr(
   char_t const * pchHaystackBegin, char_t const * pchHaystackEnd,
   char_t const * pchNeedleBegin, char_t const * pchNeedleEnd
) {
   ABC_TRACE_FUNC(pchHaystackBegin, pchHaystackEnd, pchNeedleBegin, pchNeedleEnd);

   if (pchNeedleBegin == pchNeedleEnd) {
      // No needle, so just return the beginning of the haystack.
      return pchHaystackBegin;
   }
   char_t const * pchHaystack(pchHaystackBegin);
   char_t const * pchNeedle(pchNeedleBegin);
   try {
      /** DOC:1502 KMP substring search

      This is an implementation of the Knuth-Morris-Pratt algorithm.

      Examples of the contents of vcchFailNext after the block below for different needles:

      ┌──────────────┬───┬─────┬─────┬───────┬───────┬───────────────┬─────────────┐
      │ Needle index │ 0 │ 0 1 │ 0 1 │ 0 1 2 │ 0 1 2 │ 0 1 2 3 4 5 6 │ 0 1 2 3 4 5 │
      ├──────────────┼───┼─────┼─────┼───────┼───────┼───────────────┼─────────────┤
      │ pchNeedle    │ A │ A A │ A B │ A A A │ A A B │ A B A A B A C │ A B A B C D │
      │ vcchFailNext │ 0 │ 0 0 │ 0 0 │ 0 0 0 │ 0 0 0 │ 0 0 0 0 1 2 3 │ 0 0 0 1 2 0 │
      └──────────────┴───┴─────┴─────┴───────┴───────┴───────────────┴─────────────┘
      */

      // Build the failure restart table.
      smvector<size_t, 64> vcchFailNext;
      _build_find_failure_restart_table(pchNeedleBegin, pchNeedleEnd, &vcchFailNext);

      size_t iFailNext(0);
      while (pchHaystack < pchHaystackEnd) {
         if (*pchHaystack == *pchNeedle) {
            ++pchNeedle;
            if (pchNeedle == pchNeedleEnd) {
               // The needle was exhausted, which means that all its characters were matched in the
               // haystack: we found the needle.
               return pchHaystack - iFailNext;
            }
            // Move to the next character and advance the index in vcchFailNext.
            ++pchHaystack;
            ++iFailNext;
         } else if (iFailNext > 0) {
            // The current character ends the match sequence; use vcchFailNext[iFailNext] to see how
            // much into the needle we can retry matching characters.
            iFailNext = vcchFailNext[static_cast<intptr_t>(iFailNext)];
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
      char_t chFirst(*pchNeedleBegin);
      for (; pchHaystack < pchHaystackEnd; ++pchHaystack) {
         if (*pchHaystack == chFirst) {
            char_t const * pchHaystackMatch(pchHaystack);
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


/*static*/ char_t const * host_str_traits::find_substr_last(
   char_t const * pchHaystackBegin, char_t const * pchHaystackEnd,
   char_t const * pchNeedleBegin, char_t const * pchNeedleEnd
) {
   ABC_TRACE_FUNC(pchHaystackBegin, pchHaystackEnd, pchNeedleBegin, pchNeedleEnd);

   // TODO: implement this!
   return pchHaystackEnd;
}


/*static*/ size_t host_str_traits::size_in_codepoints(
   char_t const * pchBegin, char_t const * pchEnd
) {
   ABC_TRACE_FUNC(pchBegin, pchEnd);

   size_t ccp(0);
   for (
      char_t const * pch(pchBegin);
      pch < pchEnd;
      pch += host_char_traits::lead_char_to_codepoint_size(*pch)
   ) {
      ++ccp;
   }
   return ccp;
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

