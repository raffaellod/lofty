/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014, 2015
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

namespace abc { namespace text {

std::ptrdiff_t const_codepoint_iterator::distance(std::size_t ich) const {
   ABC_TRACE_FUNC(this, ich);

   if (ich == m_ich) {
      return 0;
   } else {
      char_t const * pchBegin = m_ps->chars_begin();
      if (ich < m_ich) {
         return static_cast<std::ptrdiff_t>(
            str_traits::size_in_codepoints(pchBegin + ich, pchBegin + m_ich)
         );
      } else {
         return -static_cast<std::ptrdiff_t>(
            str_traits::size_in_codepoints(pchBegin + m_ich, pchBegin + ich)
         );
      }
   }
}

std::size_t const_codepoint_iterator::throw_if_end(std::size_t ich) const {
   ABC_TRACE_FUNC(this, ich);

   char_t const * pchBegin = m_ps->chars_begin(), * pch = pchBegin + ich;
   if (pch >= m_ps->chars_end()) {
      ABC_THROW(pointer_iterator_error, (pchBegin, m_ps->chars_end(), pch));
   }
   return ich;
}

}} //namespace abc::text
