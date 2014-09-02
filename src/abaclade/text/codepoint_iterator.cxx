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
// abc::text::_codepoint_proxy


namespace abc {
namespace text {

_codepoint_proxy<false> & _codepoint_proxy<false>::operator=(char32_t ch) {
   // Save the internal pointer of *this and this->mc_pcii so that if the string switches buffer we
   // can recalculate the pointers from these offsets.
   std::size_t ichThis(static_cast<std::size_t>(m_pch - mc_ps->chars_begin()));
   std::size_t ichIter;
   if (mc_pcii) {
      ichIter = static_cast<std::size_t>(mc_pcii->m_pch - mc_ps->chars_begin());
   }
   static_cast<mstr *>(const_cast<str_base *>(mc_ps))->_replace_codepoint(
      const_cast<char_t *>(m_pch), ch
   );
   // If _replace_codepoint() switched string buffer, recalculate the internal pointers from the
   // offsets saved above.
   m_pch = mc_ps->chars_begin() + ichThis;
   if (mc_pcii) {
      mc_pcii->m_pch = mc_ps->chars_begin() + ichIter;
   }
   return *this;
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::_codepoint_iterator_impl


namespace abc {
namespace text {

std::ptrdiff_t _codepoint_iterator_impl<true>::distance(char_t const * pch) const {
   ABC_TRACE_FUNC(this, pch);

   if (m_pch > pch) {
      return static_cast<std::ptrdiff_t>(str_traits::size_in_codepoints(pch, m_pch));
   } else if (m_pch < pch) {
      return -static_cast<std::ptrdiff_t>(str_traits::size_in_codepoints(m_pch, pch));
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

