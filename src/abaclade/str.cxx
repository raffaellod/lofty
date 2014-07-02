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
         SL("unexpected character"), sFormat, static_cast<unsigned>(it - sFormat.cend())
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

char_t const str_base::smc_chNul('\0');


char_t const * str_base::_advance_char_ptr(char_t const * pch, ptrdiff_t i, bool bIndex) const {
   ABC_TRACE_FUNC(this, pch, i, bIndex);

   char_t const * pchBegin(chars_begin()), * pchEnd(chars_end());
   ptrdiff_t iOrig(i);

   // If i is positive, move forward.
   for (; i > 0 && pch < pchEnd; --i) {
      // Find the next code point start, skipping any trail characters.
      pch += text::host_char_traits::lead_char_to_codepoint_size(*pch);
   }
   // If i is negative, move backwards.
   for (; i < 0 && pch > pchBegin; ++i) {
      // Moving to the previous code point requires finding the previous non-trail character.
      while (text::host_char_traits::is_trail_char(*--pch)) {
         ;
      }
   }

   // Verify that the pointer is still within range: that’s not the case if we left either for loop
   // before i reached 0, or if the pointer was invalid on entry (e.g. accessing istr()[0]).
   if (i != 0 || pch < pchBegin || pch > pchEnd || (bIndex && pch == pchEnd)) {
      if (bIndex) {
         ABC_THROW(index_error, (iOrig));
      } else {
         ABC_THROW(pointer_iterator_error, (pchBegin, pchEnd, pch));
      }
   }

   // Return the resulting pointer.
   return pch;
}


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
      return c_str_pointer(&smc_chNul, c_str_pointer::deleter_type(false));
   }
}


dmvector<uint8_t> str_base::encode(text::encoding enc, bool bNulT) const {
   ABC_TRACE_FUNC(this, enc, bNulT);

   dmvector<uint8_t> vb;
   size_t cbChar, cbUsed, cbStr(size_in_bytes());
   if (enc == abc::text::encoding::host) {
      // Optimal case: no transcoding necessary.
      cbChar = sizeof(char_t);
      // Enlarge vb as necessary, then copy to it the contents of the string buffer.
      vb.set_capacity(cbStr + (bNulT ? sizeof(char_t) : 0), false);
      memory::copy(vb.begin().base(), _raw_trivial_vextr_impl::begin<uint8_t>(), cbStr);
      cbUsed = cbStr;
   } else {
      cbChar = text::get_encoding_size(enc);
      void const * pStr(chars_begin());
      // Calculate the size required, then resize vb accorgingly.
      cbUsed = abc::text::transcode(true, abc::text::encoding::host, &pStr, &cbStr, enc);
      vb.set_capacity(cbUsed + (bNulT ? cbChar : 0), false);
      // Transcode the string into vb.
      void * pBuf(vb.begin().base());
      // Re-assign to cbUsed because transcode() will set *(&cbUsed) to 0.
      cbUsed = abc::text::transcode(
         true, abc::text::encoding::host, &pStr, &cbStr, enc, &pBuf, &cbUsed
      );
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
   return pchStart >= chars_begin() && text::str_traits::compare(
      pchStart, chars_end(), s.chars_begin(), s.chars_end()
   ) == 0;
}


str_base::const_iterator str_base::find(char_t chNeedle, const_iterator itWhence) const {
   ABC_TRACE_FUNC(this, chNeedle, itWhence);

   validate_pointer(itWhence.base());
   return const_iterator(text::str_traits::find_char(itWhence.base(), chars_end(), chNeedle), this);
}
str_base::const_iterator str_base::find(char32_t chNeedle, const_iterator itWhence) const {
   ABC_TRACE_FUNC(this, chNeedle, itWhence);

   validate_pointer(itWhence.base());
   return const_iterator(text::str_traits::find_char(itWhence.base(), chars_end(), chNeedle), this);
}
str_base::const_iterator str_base::find(istr const & sNeedle, const_iterator itWhence) const {
   ABC_TRACE_FUNC(this, sNeedle, itWhence);

   validate_pointer(itWhence.base());
   return const_iterator(text::str_traits::find_substr(
      itWhence.base(), chars_end(), sNeedle.chars_begin(), sNeedle.chars_end()
   ), this);
}


str_base::const_iterator str_base::find_last(char_t chNeedle, const_iterator itWhence) const {
   ABC_TRACE_FUNC(this, chNeedle, itWhence);

   validate_pointer(itWhence.base());
   return const_iterator(text::str_traits::find_char_last(
      chars_begin(), itWhence.base(), chNeedle
   ), this);
}
str_base::const_iterator str_base::find_last(char32_t chNeedle, const_iterator itWhence) const {
   ABC_TRACE_FUNC(this, chNeedle, itWhence);

   validate_pointer(itWhence.base());
   return const_iterator(text::str_traits::find_char_last(
      chars_begin(), itWhence.base(), chNeedle
   ), this);
}
str_base::const_iterator str_base::find_last(istr const & sNeedle, const_iterator itWhence) const {
   ABC_TRACE_FUNC(this, sNeedle, itWhence);

   validate_pointer(itWhence.base());
   return const_iterator(text::str_traits::find_substr_last(
      chars_begin(), itWhence.base(), sNeedle.chars_begin(), sNeedle.chars_end()
   ), this);
}


bool str_base::starts_with(istr const & s) const {
   ABC_TRACE_FUNC(this, s);

   char_t const * pchEnd(chars_begin() + s.size_in_chars());
   return pchEnd <= chars_end() && text::str_traits::compare(
      chars_begin(), pchEnd, s.chars_begin(), s.chars_end()
   ) == 0;
}


str_base::const_iterator str_base::translate_index(intptr_t ich) const {
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
   return std::move(it);
}


std::pair<str_base::const_iterator, str_base::const_iterator> str_base::translate_range(
   intptr_t ichBegin, intptr_t ichEnd
) const {
   ABC_TRACE_FUNC(this, ichBegin, ichEnd);

   auto itBegin(translate_index(ichBegin));
   auto itEnd(translate_index(ichEnd));
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
      iHash ^= static_cast<size_t>(*it);
      iHash *= c_iFNVPrime;
   }
   return iHash;
}

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::mstr


namespace abc {

void mstr::_replace_codepoint(char_t * pch, char_t chNew) {
   ABC_TRACE_FUNC(pch, chNew);

   size_t cbRemove(sizeof(char_t) * text::host_char_traits::lead_char_to_codepoint_size(*pch));
   uintptr_t ich(static_cast<uintptr_t>(pch - chars_begin()));
   _raw_trivial_vextr_impl::insert_remove(ich, nullptr, sizeof(char_t), cbRemove);
   // insert_remove() may have switched string buffer, so recalculate pch now.
   pch = chars_begin() + ich;
   // At this point, insert_remove() validated pch.
   *pch = chNew;
}
void mstr::_replace_codepoint(char_t * pch, char32_t chNew) {
   ABC_TRACE_FUNC(pch, chNew);

   size_t cbInsert(sizeof(char_t) * text::host_char_traits::codepoint_size(chNew));
   size_t cbRemove(sizeof(char_t) * text::host_char_traits::lead_char_to_codepoint_size(*pch));
   uintptr_t ich(static_cast<uintptr_t>(pch - chars_begin()));
   _raw_trivial_vextr_impl::insert_remove(ich, nullptr, cbInsert, cbRemove);
   // insert_remove() may have switched string buffer, so recalculate pch now.
   pch = chars_begin() + ich;
   // At this point, insert_remove() validated pch and codepoint_size() validated chNew; this means
   // that there’s nothing that could go wrong here leaving us in an inconsistent state.
   text::host_char_traits::traits_base::codepoint_to_chars(chNew, pch);
}


void mstr::set_from(std::function<size_t (char_t * pch, size_t cchMax)> fnRead) {
   typedef _raw_vextr_impl_base rvib;
   // The initial size avoids a few reallocations (* smc_iGrowthRate ** 2).
   // Multiplying by smc_iGrowthRate should guarantee that set_capacity() will allocate exactly the
   // requested number of characters, eliminating the need to query back with capacity().
   size_t cchRet, cchMax(rvib::smc_cbCapacityMin * rvib::smc_iGrowthRate);
   do {
      cchMax *= rvib::smc_iGrowthRate;
      set_capacity(cchMax, false);
      cchRet = fnRead(chars_begin(), cchMax);
   } while (cchRet >= cchMax);
   // Finalize the length.
   set_size_in_chars(cchRet);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

