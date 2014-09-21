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
// abc::from_str

namespace abc {

template <typename T>
inline T from_str(istr const & s, istr const & sFormat /*= istr::empty*/) {
   io::text::str_reader tsr(external_buffer, &s);
   from_str_backend<T> fsb;
   fsb.set_format(sFormat);
   T t(fsb.read(t, &tsr));
   if (std::size_t cchRemaining = tsr.remaining_size_in_chars()) {
      // There are still unused characters in tsr, so the conversion failed.
      // TODO: throw an syntax_error-like exception.
   }
   return std::move(t);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

