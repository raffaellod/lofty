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

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> instead of this file
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specialization for abc::text::_codepoint_proxy

namespace abc {

template <bool t_bConst>
class to_str_backend<text::_codepoint_proxy<t_bConst>> : public to_str_backend<char32_t> {
public:
   /*! Writes a code point proxy as a plain code point (char32_t), applying the formatting options.

   cpp
      Code point to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(text::_codepoint_proxy<t_bConst> const & cpp, io::text::writer * ptwOut) {
      to_str_backend<char32_t>::write(cpp.operator char32_t(), ptwOut);
   }
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specialization for abc::text::codepoint_iterator

namespace abc {

template <bool t_bConst>
class to_str_backend<text::codepoint_iterator<t_bConst>> :
   public to_str_backend<typename text::codepoint_iterator<t_bConst>::pointer> {
public:
   /*! Writes a code point iterator as a pointer, applying the formatting options.

   it
      Iterator to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(text::codepoint_iterator<t_bConst> const & it, io::text::writer * ptwOut) {
      to_str_backend<typename text::codepoint_iterator<t_bConst>::pointer>::write(
         it.base(), ptwOut
      );
   }
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

