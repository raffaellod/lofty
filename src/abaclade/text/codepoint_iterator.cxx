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

ptrdiff_t _codepoint_iterator_impl<true>::distance(char_t const * pch) const {
   ABC_TRACE_FUNC(this, pch);

   if (m_pch > pch) {
      return static_cast<ptrdiff_t>(host_str_traits::size_in_codepoints(pch, m_pch));
   } else if (m_pch < pch) {
      return -static_cast<ptrdiff_t>(host_str_traits::size_in_codepoints(m_pch, pch));
   } else {
      return 0;
   }
}


char_t const * _codepoint_iterator_impl<true>::throw_if_end(char_t const * pch) const {
   ABC_TRACE_FUNC(this, pch);

   if (pch >= m_ps->chars_end()) {
      ABC_THROW(pointer_iterator_error, (m_ps->chars_begin(), m_ps->chars_end(), pch));
   }
   return pch;
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

