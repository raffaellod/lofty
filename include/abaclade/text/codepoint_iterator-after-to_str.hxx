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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text {

inline const_codepoint_proxy::operator char32_t() const {
   return host_char_traits::chars_to_codepoint(mc_ps->chars_begin() + mc_ich);
}

inline codepoint_proxy & codepoint_proxy::operator=(char_t ch) {
   const_cast<str *>(mc_ps)->_replace_codepoint(
      const_cast<str *>(mc_ps)->chars_begin() + mc_ich, ch
   );
   return *this;
}

inline codepoint_proxy & codepoint_proxy::operator=(char32_t cp) {
   const_cast<str *>(mc_ps)->_replace_codepoint(
      const_cast<str *>(mc_ps)->chars_begin() + mc_ich, cp
   );
   return *this;
}

}} //namespace abc::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text {

inline std::size_t const_codepoint_iterator::advance(std::ptrdiff_t iDelta, bool bIndex) const {
   return m_ps->_advance_char_index(m_ich, iDelta, bIndex);
}

inline char_t const * const_codepoint_iterator::base() const {
   return m_ps->chars_begin() + m_ich;
}

inline char_t * codepoint_iterator::base() const {
   return const_cast<str *>(m_ps)->chars_begin() + m_ich;
}

}} //namespace abc::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

template <>
class to_str_backend<text::const_codepoint_proxy> : public to_str_backend<char32_t> {
public:
   /*! Writes a code point proxy as a plain code point (char32_t), applying the formatting options.

   @param cpp
      Code point to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(text::const_codepoint_proxy const & cpp, io::text::writer * ptwOut) {
      to_str_backend<char32_t>::write(cpp.operator char32_t(), ptwOut);
   }
};

template <>
class to_str_backend<text::codepoint_proxy> : public to_str_backend<text::const_codepoint_proxy> {
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

template <>
class to_str_backend<text::const_codepoint_iterator> : public to_str_backend<text::char_t const *> {
public:
   /*! Writes a code point iterator as a character index, applying the formatting options.

   @param it
      Iterator to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(text::const_codepoint_iterator const & it, io::text::writer * ptwOut) {
      to_str_backend<text::char_t const *>::write(it.base(), ptwOut);
   }
};

template <>
class to_str_backend<text::codepoint_iterator> :
   public to_str_backend<text::const_codepoint_iterator> {
};

} //namespace abc
