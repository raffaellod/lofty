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
// abc::to_str_backend – specialization for abc::codepoint_iterator


namespace abc {

template <bool t_bConst>
class to_str_backend<codepoint_iterator<t_bConst>> :
   public to_str_backend<typename codepoint_iterator<t_bConst>::pointer> {
public:

   /** Writes an iterator as a pointer, applying the formatting options.

   it
      Iterator to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(codepoint_iterator<t_bConst> const & it, io::text::writer * ptwOut) {
      to_str_backend<typename codepoint_iterator<t_bConst>::pointer>::write(it.base(), ptwOut);
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

