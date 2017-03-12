/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/text.hxx>
#include <lofty/text/char_ptr_to_str_adapter.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

void to_text_ostream<text::char_ptr_to_str_adapter>::write(
   text::char_ptr_to_str_adapter const & src, io::text::ostream * dst
) {
   void const * src_bytes;
   std::size_t src_byte_size;
   text::encoding enc;
   if (src.s) {
      src_bytes = src.s;
      auto char_size = text::size_in_chars(src.s);
      enc = text::guess_encoding(src.s, src.s + char_size);
      src_byte_size = sizeof src.s[0] * char_size;
   } else {
      static char_t const nullptr_str[] = LOFTY_SL("<nullptr>");
      src_bytes = nullptr_str;
      enc = text::encoding::host;
      src_byte_size = sizeof nullptr_str - sizeof nullptr_str[0] /*NUL*/;
   }
   text::_pvt::str_to_text_ostream::write(src_bytes, src_byte_size, enc, dst);
}

} //namespace lofty
