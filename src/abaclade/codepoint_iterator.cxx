/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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
// abc::_codepoint_iterator_impl


namespace abc {

ptrdiff_t _codepoint_iterator_impl<true>::distance(character const * pch) const {
   if (m_pch >= pch) {
      return ptrdiff_t(istr::traits::size_in_codepoints(pch, m_pch));
   } else {
      return -ptrdiff_t(istr::traits::size_in_codepoints(m_pch, pch));
   }
}


void _codepoint_iterator_impl<true>::modify(ptrdiff_t i) {
   char_t const * pch(m_pch);
   while (i) {
      if (i >= 0) {
         // Move forward.
#if ABC_HOST_UTF == 8
         pch += 1 + istr::traits::leading_to_cont_length(*pch);
#elif ABC_HOST_UTF == 16 //if ABC_HOST_UTF == 8
         pch += 1 + ((*pch & 0xfc00) == 0xd800);
#endif //if ABC_HOST_UTF == 8 … elif ABC_HOST_UTF == 16
         --i;
      } else {
         // Move backwards.
#if ABC_HOST_UTF == 8
         while ((*--pch & 0xc0) == 0x80) {
            ;
         }
#elif ABC_HOST_UTF == 16 //if ABC_HOST_UTF == 8
         if ((*--pch & 0xfc00) == 0xdc00) {
            // Tail surrogate.
            --pch;
         }
#endif //if ABC_HOST_UTF == 8 … elif ABC_HOST_UTF == 16
         ++i;
      }
   }
   // Update the iterator position.
   m_pch = pch;
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

