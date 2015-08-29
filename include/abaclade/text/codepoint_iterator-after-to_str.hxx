﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text { namespace detail {

inline codepoint_proxy<false> & codepoint_proxy<false>::operator=(char_t ch) {
   mc_ps->_replace_codepoint(const_cast<char_t *>(mc_ps->chars_begin()) + m_ich, ch);
   return *this;
}

inline codepoint_proxy<false> & codepoint_proxy<false>::operator=(char32_t cp) {
   mc_ps->_replace_codepoint(const_cast<char_t *>(mc_ps->chars_begin()) + m_ich, cp);
   return *this;
}

inline codepoint_proxy<true>::operator char32_t() const {
   return host_char_traits::chars_to_codepoint(mc_ps->chars_begin() + m_ich);
}

}}} //namespace abc::text::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text { namespace detail {

inline std::size_t codepoint_iterator_impl<true>::advance(
   std::ptrdiff_t iDelta, bool bIndex
) const {
   return m_ps->_advance_char_index(m_ich, iDelta, bIndex);
}

inline char_t const * codepoint_iterator_impl<true>::base() const {
   return m_ps->chars_begin() + m_ich;
}

inline char_t * codepoint_iterator_impl<false>::base() const {
   return const_cast<char_t *>(m_ps->chars_begin()) + m_ich;
}

}}} //namespace abc::text::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

template <bool t_bConst>
class to_str_backend<text::detail::codepoint_proxy<t_bConst>> : public to_str_backend<char32_t> {
public:
   /*! Writes a code point proxy as a plain code point (char32_t), applying the formatting options.

   @param cpp
      Code point to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(text::detail::codepoint_proxy<t_bConst> const & cpp, io::text::writer * ptwOut) {
      to_str_backend<char32_t>::write(cpp.operator char32_t(), ptwOut);
   }
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

template <bool t_bConst>
class to_str_backend<text::codepoint_iterator<t_bConst>> :
   public to_str_backend<typename text::codepoint_iterator<t_bConst>::pointer> {
public:
   /*! Writes a code point iterator as a pointer, applying the formatting options.

   @param it
      Iterator to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(text::codepoint_iterator<t_bConst> const & it, io::text::writer * ptwOut) {
      to_str_backend<typename text::codepoint_iterator<t_bConst>::pointer>::write(
         it.base(), ptwOut
      );
   }
};

} //namespace abc
