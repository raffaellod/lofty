/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#include <abc.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_str_to_str_backend


namespace abc {

_str_to_str_backend::_str_to_str_backend(istr const & sFormat) {
   ABC_TRACE_FN((this, sFormat));

   auto it(sFormat.cbegin());

   // TODO: parse the format string.

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
   ABC_TRACE_FN((this, p, cb, enc, ptwOut));

   // TODO: apply format options.
   ptwOut->write_binary(p, cb, enc);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::str_base


namespace abc {

str_base::c_str_pointer str_base::c_str() const {
   ABC_TRACE_FN((this));

   if (m_rvpd.get_bNulT()) {
      // The string already includes a NUL terminator, so we can simply return the same array.
      return c_str_pointer(cbegin().base(), c_str_pointer::deleter_type(false));
   } else if (size_t cch = size()) {
      // The string is not empty but lacks a NUL terminator: create a temporary copy that includes a
      // NUL, and return it.
      c_str_pointer psz(
         memory::alloc<char_t const []>(cch + 1 /*NUL*/).release(),
         c_str_pointer::deleter_type(true)
      );
      char_t * pch(const_cast<char_t *>(psz.get()));
      memory::copy(pch, cbegin().base(), cch);
      memory::clear(pch + cch);
      return std::move(psz);
   } else {
      // The string is empty, so a static NUL character will suffice.
      return c_str_pointer(&smc_chNUL, c_str_pointer::deleter_type(false));
   }
}


dmvector<uint8_t> str_base::encode(text::encoding enc, bool bNulT) const {
   ABC_TRACE_FN((this, enc, bNulT));

   dmvector<uint8_t> vb;
   size_t cbChar, cbUsed, cbStr(size() * sizeof(char_t));
   if (enc == abc::text::encoding::host) {
      // Optimal case: no transcoding necessary.
      cbChar = sizeof(char_t);
      // Enlarge the string as necessary, then overwrite any character in the affected range.
      vb.set_capacity(cbStr + (bNulT ? sizeof(char_t) : 0), false);
      memory::copy(vb.begin().base(), reinterpret_cast<uint8_t const *>(cbegin().base()), cbStr);
      cbUsed = cbStr;
   } else {
      cbChar = text::get_encoding_size(enc);
      cbUsed = 0;
      void const * pStr(cbegin().base());
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
   ABC_TRACE_FN((this, s));

   auto itStart(cend() - intptr_t(s.size()));
   return itStart >= cbegin() && traits::str_cmp(
      itStart.base(), cend().base(), s.cbegin().base(), s.cend().base()
   ) == 0;
}


str_base::const_iterator str_base::find(char32_t chNeedle, const_iterator itWhence) const {
   ABC_TRACE_FN((this, chNeedle, itWhence));

   auto itEnd(cend());
   char_t const * pch(traits::str_chr(itWhence.base(), itEnd.base(), chNeedle));
   return pch ? const_iterator(pch) : itEnd;
}
str_base::const_iterator str_base::find(istr const & sNeedle, const_iterator itWhence) const {
   ABC_TRACE_FN((this, sNeedle, itWhence));

   auto itEnd(cend());
   char_t const * pch(traits::str_str(
      itWhence.base(), itEnd.base(), sNeedle.cbegin().base(), sNeedle.cend().base()
   ));
   return pch ? const_iterator(pch) : itEnd;
}


str_base::const_iterator str_base::find_last(char32_t chNeedle, const_iterator itWhence) const {
   ABC_TRACE_FN((this, chNeedle, itWhence));

   char_t const * pch(traits::str_chr_r(cbegin().base(), itWhence.base(), chNeedle));
   return pch ? const_iterator(pch) : cend();
}
str_base::const_iterator str_base::find_last(istr const & sNeedle, const_iterator itWhence) const {
   ABC_TRACE_FN((this, sNeedle, itWhence));

   char_t const * pch(traits::str_str_r(
      cbegin().base(), itWhence.base(), sNeedle.cbegin().base(), sNeedle.cend().base()
   ));
   return pch ? const_iterator(pch) : cend();
}


bool str_base::starts_with(istr const & s) const {
   ABC_TRACE_FN((this, s));

   auto itEnd(cbegin() + intptr_t(s.size()));
   return itEnd <= cend() && traits::str_cmp(
      cbegin().base(), itEnd.base(), s.cbegin().base(), s.cend().base()
   ) == 0;
}

} //namespace abc


namespace std {

// Implementation based on the Fowler/Noll/Vo variant 1a (FNV-1a) algorithm. See
// <http://www.isthe.com/chongo/tech/comp/fnv/> for details.
//
// The bases are calculated by src/fnv_hash_basis.py.
size_t hash<abc::str_base>::operator()(abc::str_base const & s) const {
   ABC_TRACE_FN((this, s));

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

