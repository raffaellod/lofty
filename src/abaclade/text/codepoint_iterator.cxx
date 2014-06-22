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
// abc::text::_codepoint_iterator_impl


namespace abc {
namespace text {

char_t const * _codepoint_iterator_impl<true>::advance(ptrdiff_t i) const {
   ABC_TRACE_FUNC(this, i);

   char_t const * pch(m_pch);
   // If i is positive, move forward.
   for (; i > 0; --i) {
      // Find the next code point start, skipping any trail characters.
      pch += host_char_traits::lead_char_to_codepoint_size(*pch);
   }
   // If i is negative, move backwards.
   for (; i < 0; ++i) {
      // Moving to the previous code point requires finding the previous non-trail character.
      while (host_char_traits::is_trail_char(*--pch)) {
         ;
      }
   }
   // Return the resulting pointer.
   return pch;
}


ptrdiff_t _codepoint_iterator_impl<true>::distance(char_t const * pch) const {
   ABC_TRACE_FUNC(this, pch);

   if (m_pch > pch) {
      return ptrdiff_t(host_str_traits::size_in_codepoints(pch, m_pch));
   } else if (m_pch < pch) {
      return -ptrdiff_t(host_str_traits::size_in_codepoints(m_pch, pch));
   } else {
      return 0;
   }
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

