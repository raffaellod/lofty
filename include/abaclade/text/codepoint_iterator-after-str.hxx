﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

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

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::_codepoint_iterator_impl


namespace abc {
namespace text {

inline _codepoint_proxy<false> & _codepoint_proxy<false>::operator=(char_t ch) {
   static_cast<mstr *>(const_cast<str_base *>(m_ps))->_replace_codepoint(
      const_cast<char_t *>(m_pch), ch
   );
   return *this;
}
inline _codepoint_proxy<false> & _codepoint_proxy<false>::operator=(char32_t ch) {
   static_cast<mstr *>(const_cast<str_base *>(m_ps))->_replace_codepoint(
      const_cast<char_t *>(m_pch), ch
   );
   return *this;
}


inline char_t const * _codepoint_iterator_impl<true>::advance(ptrdiff_t i, bool bIndex) const {
   return m_ps->_advance_char_ptr(m_pch, i, bIndex);
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
