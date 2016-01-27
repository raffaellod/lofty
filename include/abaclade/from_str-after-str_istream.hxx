/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2016 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

template <typename T>
inline T from_str(str const & s, str const & sFormat /*= str::empty*/) {
   io::text::str_istream sis(external_buffer, &s);
   from_str_backend<T> fsb;
   fsb.set_format(sFormat);
   T t;
   fsb.read(&t, &sis));
   if (std::size_t cchRemaining = sis.remaining_size_in_chars()) {
      // There are still unused characters in sis, so the conversion failed.
      ABC_THROW(syntax_error, (
         ABC_SL("unexpected character"), sFormat,
         static_cast<unsigned>(s.index_from_char_index(s.size_in_chars() - cchRemaining))
      ));
   }
   return _std::move(t);
}

} //namespace abc
