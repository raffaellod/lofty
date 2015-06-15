/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
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
// abc::to_str_backend – specialization for abc::text::char_ptr_to_str_adapter

namespace abc {

void to_str_backend<text::char_ptr_to_str_adapter>::write(
   text::char_ptr_to_str_adapter const & cs, io::text::writer * ptwOut
) {
   std::size_t cch = text::size_in_chars(cs.m_psz);
   text::encoding enc(text::guess_encoding(cs.m_psz, cs.m_psz + cch));
   text::detail::str_to_str_backend::write(cs.m_psz, sizeof(char) * cch, enc, ptwOut);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
