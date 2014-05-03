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

#include <abc/core.hxx>
#include <abc/str.hxx>
#include <abc/trace.hxx>



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


void _str_to_str_backend::write(void const * p, size_t cb, text::encoding enc, ostream * posOut) {
   ABC_TRACE_FN((this, p, cb, enc, posOut));

   // TODO: apply format options.
   posOut->write_raw(p, cb, enc);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_str


namespace abc {

_raw_str::c_str_pointer _raw_str::c_str(size_t cbItem) const {
   void const * pData(data<void>());
   if (m_rvpd.get_bNulT()) {
      // The string already includes a NUL terminator, so we can simply return the same array.
      return c_str_pointer(pData, memory::conditional_deleter<void const>(false));
   }
   if (size_t cb = cbItem * size()) {
      // The string is not empty but lacks a NUL terminator: create a temporary copy that
      // includes a NUL, and return it.
      c_str_pointer psz(c_str_pointer(pData, memory::conditional_deleter<void const>(true)));
      memory::copy(const_cast<void *>(psz.get()), pData, cb);
      terminate(cbItem, static_cast<int8_t *>(const_cast<void *>(psz.get())) + cb);
      return std::move(psz);
   }
   // The string is empty, so a static NUL character will suffice.
   return c_str_pointer(&smc_chNUL, memory::conditional_deleter<void const>(false));
}


// Implementation based on the Fowler/Noll/Vo variant 1a (FNV-1a) algorithm. See
// <http://www.isthe.com/chongo/tech/comp/fnv/> for details.
//
// The primes are calculated by src/fnv_hash_basis.py.
size_t _raw_str::hash(size_t cbItem) const {
   ABC_TRACE_FN((this, cbItem));

   // The checks below are based on the assumption that sizeof(size_t) == ABC_HOST_WORD_SIZE.
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
   int8_t const * pbBegin(static_cast<int8_t const *>(m_p)),
                * pbEnd(pbBegin + cbItem * size());
   for (int8_t const * pb(pbBegin); pb < pbEnd; ++pb) {
      iHash ^= size_t(*pb);
      iHash *= c_iFNVPrime;
   }
   return iHash;
}


void _raw_str::set_size(size_t cbItem, size_t cch) {
   ABC_TRACE_FN((this, cbItem, cch));

   if (cch != size()) {
      if (cch > capacity()) {
         // Enlarge the item array.
         set_capacity(cbItem, cch, true);
      }
      m_ci = cch;
   }
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

