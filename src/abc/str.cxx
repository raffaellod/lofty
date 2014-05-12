﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

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

#include <abc/core.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_str_to_str_backend


namespace abc {

_str_to_str_backend::_str_to_str_backend(char_range const & crFormat) {
   ABC_TRACE_FN((this, crFormat));

   auto it(crFormat.cbegin());

   // TODO: parse the format string.

   // If we still have any characters, they are garbage.
   if (it != crFormat.cend()) {
      ABC_THROW(syntax_error, (
         SL("unexpected character"), crFormat, unsigned(it - crFormat.cend())
      ));
   }
}


void _str_to_str_backend::write(
   void const * p, size_t cb, text::encoding enc, io::ostream * posOut
) {
   ABC_TRACE_FN((this, p, cb, enc, posOut));

   // TODO: apply format options.
   posOut->write_raw(p, cb, enc);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::str_base


namespace abc {

str_base::c_str_pointer str_base::c_str() const {
   ABC_TRACE_FN((this));

   char_t const * pchData(data());
   if (m_rvpd.get_bNulT()) {
      // The string already includes a NUL terminator, so we can simply return the same array.
      return c_str_pointer(pchData, c_str_pointer::deleter_type(false));
   }
   if (size_t cch = size()) {
      // The string is not empty but lacks a NUL terminator: create a temporary copy that includes a
      // NUL, and return it.
      c_str_pointer psz(
         memory::alloc<char_t const []>(cch + 1 /*NUL*/).release(),
         c_str_pointer::deleter_type(true)
      );
      memory::copy(const_cast<char_t *>(psz.get()), pchData, cch);
      terminate(sizeof(char_t), const_cast<char_t *>(psz.get()) + cch);
      return std::move(psz);
   }
   // The string is empty, so a static NUL character will suffice.
   return c_str_pointer(
      reinterpret_cast<char_t const *>(&smc_chNUL), c_str_pointer::deleter_type(false)
   );
}

} //namespace abc


namespace std {

// Implementation based on the Fowler/Noll/Vo variant 1a (FNV-1a) algorithm. See
// <http://www.isthe.com/chongo/tech/comp/fnv/> for details.
//
// The primes are calculated by src/fnv_hash_basis.py.
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
// abc::mstr


namespace abc {

void mstr::set_size(size_t cch) {
   ABC_TRACE_FN((this, cch));

   if (cch != size()) {
      if (cch > capacity()) {
         // Enlarge the item array.
         set_capacity(cch, true);
      }
      m_ci = cch;
   }
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

