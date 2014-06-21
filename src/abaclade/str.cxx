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
// abc::_str_to_str_backend


namespace abc {

void _str_to_str_backend::set_format(istr const & sFormat) {
   ABC_TRACE_FUNC(this, sFormat);

   auto it(sFormat.cbegin());

   // Add parsing of the format string here.

   // If we still have any characters, they are garbage.
   if (it != sFormat.cend()) {
      ABC_THROW(syntax_error, (
         SL("unexpected character"), sFormat, unsigned(it - sFormat.cend())
      ));
   }
}


void _str_to_str_backend::write(
   void const * p, size_t cb, text::encoding enc, io::text::writer * ptwOut
) {
   ABC_TRACE_FUNC(this, p, cb, enc, ptwOut);

   ptwOut->write_binary(p, cb, enc);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::str_base


namespace abc {

char_t const str_base::smc_chNUL(CL('\0'));


str_base::c_str_pointer str_base::c_str() const {
   ABC_TRACE_FUNC(this);

   if (m_bNulT) {
      // The string already includes a NUL terminator, so we can simply return the same array.
      return c_str_pointer(chars_begin(), c_str_pointer::deleter_type(false));
   } else if (size_t cch = size_in_chars()) {
      // The string is not empty but lacks a NUL terminator: create a temporary copy that includes a
      // NUL, and return it.
      c_str_pointer psz(
         memory::alloc<char_t const []>(cch + 1 /*NUL*/).release(),
         c_str_pointer::deleter_type(true)
      );
      char_t * pch(const_cast<char_t *>(psz.get()));
      memory::copy(pch, chars_begin(), cch);
      memory::clear(pch + cch);
      return std::move(psz);
   } else {
      // The string is empty, so a static NUL character will suffice.
      return c_str_pointer(&smc_chNUL, c_str_pointer::deleter_type(false));
   }
}


dmvector<uint8_t> str_base::encode(text::encoding enc, bool bNulT) const {
   ABC_TRACE_FUNC(this, enc, bNulT);

   dmvector<uint8_t> vb;
   size_t cbChar, cbUsed, cbStr(size_in_bytes());
   if (enc == abc::text::encoding::host) {
      // Optimal case: no transcoding necessary.
      cbChar = sizeof(char_t);
      // Enlarge the string as necessary, then overwrite any character in the affected range.
      vb.set_capacity(cbStr + (bNulT ? sizeof(char_t) : 0), false);
      memory::copy(vb.begin().base(), _raw_trivial_vextr_impl::begin<uint8_t>(), cbStr);
      cbUsed = cbStr;
   } else {
      cbChar = text::get_encoding_size(enc);
      cbUsed = 0;
      void const * pStr(chars_begin());
      // Calculate the additional size required.
      size_t cbDstEst(abc::text::estimate_transcoded_size(
         abc::text::encoding::host, pStr, cbStr, enc
      ));
      for (;;) {
         vb.set_capacity(cbDstEst, true);
         // Get the resulting buffer and its actual size.
         void * pBuf(vb.begin().base() + cbUsed);
         size_t cbBuf(vb.capacity() - cbUsed);
         // Fill as much of the buffer as possible, and increment cbUsed accordingly.
         cbUsed += abc::text::transcode(
            std::nothrow, abc::text::encoding::host, &pStr, &cbStr, enc, &pBuf, &cbBuf
         );
         if (!cbStr) {
            break;
         }
         // The buffer needs to be larger than estimated; let’s try in increments. Since we don’t
         // want to repeat this many times, increment the previous estimate by 50%.
         size_t cbDstNewEst(cbDstEst + (cbDstEst << 1));
         // Detect overflow.
         if (cbDstNewEst < cbDstEst) {
            cbDstNewEst = numeric::max<size_t>::value;
         }
         cbDstEst = cbDstNewEst;
      }
   }
   if (bNulT) {
      memory::clear(vb.begin().base() + cbUsed, cbChar);
      cbUsed += cbChar;
   }
   // Assign the vector its size, and return it.
   vb.set_size(cbUsed);
   return std::move(vb);
}


bool str_base::ends_with(istr const & s) const {
   ABC_TRACE_FUNC(this, s);

   char_t const * pchStart(chars_end() - s.size_in_chars());
   return pchStart >= chars_begin() && str_cmp(
      pchStart, chars_end(), s.chars_begin(), s.chars_end()
   ) == 0;
}


str_base::const_iterator str_base::find(char32_t chNeedle, const_iterator itWhence) const {
   ABC_TRACE_FUNC(this, chNeedle, itWhence);

   validate_pointer(itWhence.base());
   char_t const * pchEnd(chars_end());
   char_t const * pch(str_chr(itWhence.base(), pchEnd, chNeedle));
   return const_iterator(pch ? pch : pchEnd);
}
str_base::const_iterator str_base::find(istr const & sNeedle, const_iterator itWhence) const {
   ABC_TRACE_FUNC(this, sNeedle, itWhence);

   validate_pointer(itWhence.base());
   char_t const * pchEnd(chars_end());
   char_t const * pch(str_str(itWhence.base(), pchEnd, sNeedle.chars_begin(), sNeedle.chars_end()));
   return const_iterator(pch ? pch : pchEnd);
}


str_base::const_iterator str_base::find_last(char32_t chNeedle, const_iterator itWhence) const {
   ABC_TRACE_FUNC(this, chNeedle, itWhence);

   validate_pointer(itWhence.base());
   char_t const * pch(str_chr_r(chars_begin(), itWhence.base(), chNeedle));
   return const_iterator(pch ? pch : chars_end());
}
str_base::const_iterator str_base::find_last(istr const & sNeedle, const_iterator itWhence) const {
   ABC_TRACE_FUNC(this, sNeedle, itWhence);

   validate_pointer(itWhence.base());
   char_t const * pch(str_str_r(
      chars_begin(), itWhence.base(), sNeedle.chars_begin(), sNeedle.chars_end()
   ));
   return const_iterator(pch ? pch : chars_end());
}


bool str_base::starts_with(istr const & s) const {
   ABC_TRACE_FUNC(this, s);

   char_t const * pchEnd(chars_begin() + s.size_in_chars());
   return pchEnd <= chars_end() && str_cmp(
      chars_begin(), pchEnd, s.chars_begin(), s.chars_end()
   ) == 0;
}


/*static*/ char_t const * str_base::str_chr(
   char_t const * pchHaystackBegin, char_t const * pchHaystackEnd, char32_t chNeedle
) {
   ABC_TRACE_FUNC(pchHaystackBegin, pchHaystackEnd, chNeedle);

   if (chNeedle <= char32_t(numeric::max<char_t>::value)) {
      // The needle can be encoded as a single character, so this faster search can be used.
      for (char_t const * pch(pchHaystackBegin); pch < pchHaystackEnd; ++pch) {
         if (*pch == static_cast<char_t>(chNeedle)) {
            return pch;
         }
      }
      return pchHaystackEnd;
   } else {
      // The needle is two or more characters, so take the slower approach.
      char_t achNeedle[text::host_char_traits::max_codepoint_length];
      text::host_char_traits::codepoint_to_chars(chNeedle, achNeedle);
      return str_chr(pchHaystackBegin, pchHaystackEnd, achNeedle);
   }
}
/*static*/ char_t const * str_base::str_chr(
   char_t const * pchHaystackBegin, char_t const * pchHaystackEnd, char_t const* pchNeedle
) {
   ABC_TRACE_FUNC(pchHaystackBegin, pchHaystackEnd, pchNeedle);

#if ABC_HOST_UTF == 8
   char8_t chNeedleLead(*pchNeedle);
   for (char8_t const * pch(pchHaystackBegin), * pchNext; pch < pchHaystackEnd; pch = pchNext) {
      char8_t ch(*pch);
      unsigned cbCp(text::host_char_traits::lead_char_to_codepoint_size(ch));
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
   char16_t chNeedle1(
      text::host_char_traits::is_lead_surrogate(chNeedle0) ? pchNeedle[1] : U16CL('\0')
   );
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


/*static*/ char_t const * str_base::str_chr_r(
   char_t const * pchHaystackBegin, char_t const * pchHaystackEnd, char32_t chNeedle
) {
   ABC_TRACE_FUNC(pchHaystackBegin, pchHaystackEnd, chNeedle);

   if (chNeedle <= char32_t(numeric::max<char_t>::value)) {
      // The needle can be encoded as a single character, so this faster search can be used.
      for (char_t const * pch(pchHaystackEnd); pch > pchHaystackBegin; ) {
         if (*--pch == static_cast<char_t>(chNeedle)) {
            return pch;
         }
      }
      return pchHaystackBegin;
   } else {
      // The needle is two or more characters; this means that we can’t do the fast backwards scan
      // above, so just do a regular substring reverse search.
      char_t achNeedle[text::host_char_traits::max_codepoint_length];
      return str_str_r(
         pchHaystackBegin, pchHaystackEnd,
         achNeedle, text::host_char_traits::codepoint_to_chars(chNeedle, achNeedle)
      );
   }
}


/*static*/ int str_base::str_cmp(
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
      bool bIsSurrogate1(text::host_char_traits::is_surrogate(ch1));
      bool bIsSurrogate2(text::host_char_traits::is_surrogate(ch2));
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


/*static*/ char_t const * str_base::str_str(
   char_t const * pchHaystackBegin, char_t const * pchHaystackEnd,
   char_t const * pchNeedleBegin, char_t const * pchNeedleEnd
) {
   ABC_TRACE_FUNC(pchHaystackBegin, pchHaystackEnd, pchNeedleBegin, pchNeedleEnd);

   if (!(pchNeedleEnd - pchNeedleBegin)) {
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
      str_str_build_failure_restart_table(pchNeedleBegin, pchNeedleEnd, &vcchFailNext);

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
            iFailNext = vcchFailNext[intptr_t(iFailNext)];
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


/*static*/ void str_base::str_str_build_failure_restart_table(
   char_t const * pchNeedleBegin, char_t const * pchNeedleEnd, mvector<size_t> * pvcchFailNext
) {
   ABC_TRACE_FUNC(pchNeedleBegin, pchNeedleEnd, pvcchFailNext);

   pvcchFailNext->set_size(size_t(pchNeedleEnd - pchNeedleBegin));
   auto itNextFailNext(pvcchFailNext->begin());

   // The earliest repetition of a non-first character can only occur on the fourth character, so
   // start by skipping two characters and storing two zeroes for them, then the first iteration
   // will also always store an additional zero and consume one more character.
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


/*static*/ char_t const * str_base::str_str_r(
   char_t const * pchHaystackBegin, char_t const * pchHaystackEnd,
   char_t const * pchNeedleBegin, char_t const * pchNeedleEnd
) {
   ABC_TRACE_FUNC(pchHaystackBegin, pchHaystackEnd, pchNeedleBegin, pchNeedleEnd);

   // TODO: implement this!
   return pchHaystackEnd;
}


str_base::const_iterator str_base::translate_index(intptr_t ich) const {
   auto ret(translate_index_nothrow(ich));
   if (!ret.second) {
      ABC_THROW(index_error, (ich));
   }
   return ret.first;
}


std::pair<str_base::const_iterator, bool> str_base::translate_index_nothrow(intptr_t ich) const {
   ABC_TRACE_FUNC(this, ich);

   const_iterator it, itLoopEnd;
   int iDelta;
   if (ich >= 0) {
      // The character index is non-negative: assume it’s faster to reach the corresponding code
      // point index by starting from the beginning.
      it = begin();
      itLoopEnd = end();
      iDelta = 1;
   } else {
      // The character index is negative: assume it’s faster to reach the corresponding code point
      // index by starting from the end.
      it = end();
      itLoopEnd = begin();
      iDelta = -1;
   }
   while (ich && it != itLoopEnd) {
      ich -= iDelta;
      it += iDelta;
   }
   if (it == itLoopEnd) {
      // The above loop did not exhaust ich, so ceil the returned iterator to itLoopEnd.
      return std::pair<const_iterator, bool>(itLoopEnd, false);
   } else {
      // The above loop exhausted ich, so *it is the correct character.
      return std::pair<const_iterator, bool>(it, true);
   }
}


std::pair<str_base::const_iterator, str_base::const_iterator> str_base::translate_range(
   intptr_t ichBegin, intptr_t ichEnd
) const {
   ABC_TRACE_FUNC(this, ichBegin, ichEnd);

   auto itBegin(translate_index_nothrow(ichBegin).first);
   auto itEnd(translate_index_nothrow(ichEnd).first);
   // If the interval is empty, return [end(), end()) .
   if (itBegin >= itEnd) {
      return std::pair<const_iterator, const_iterator>(end(), end());
   }
   // Return the constructed interval.
   return std::pair<const_iterator, const_iterator>(itBegin, itEnd);
}

} //namespace abc


namespace std {

// Implementation based on the Fowler/Noll/Vo variant 1a (FNV-1a) algorithm. See
// <http://www.isthe.com/chongo/tech/comp/fnv/> for details.
//
// The bases are calculated by src/fnv_hash_basis.py.
size_t hash<abc::str_base>::operator()(abc::str_base const & s) const {
   ABC_TRACE_FUNC(this, s);

   static_assert(
      sizeof(size_t) * 8 == ABC_HOST_WORD_SIZE,
      "unexpected sizeof(size_t) will break FNV prime/basis selection"
   );
#if ABC_HOST_WORD_SIZE == 16
   size_t const c_iFNVPrime(0x1135);
   size_t const c_iFNVBasis(16635u);
#elif ABC_HOST_WORD_SIZE == 32
   size_t const c_iFNVPrime(0x01000193);
   size_t const c_iFNVBasis(2166136261u);
#elif ABC_HOST_WORD_SIZE == 64
   size_t const c_iFNVPrime(0x00000100000001b3);
   size_t const c_iFNVBasis(14695981039346656037u);
#endif
   size_t iHash(c_iFNVBasis);
   for (auto it(s.cbegin()), itEnd(s.cend()); it != itEnd; ++it) {
      iHash ^= size_t(*it);
      iHash *= c_iFNVPrime;
   }
   return iHash;
}

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////

