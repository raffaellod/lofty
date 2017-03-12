/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2017 Raffaello D. Di Napoli

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
#include <lofty/collections/vector.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

vector_to_text_ostream::vector_to_text_ostream() :
   lofty::_pvt::sequence_to_text_ostream(LOFTY_SL("{"), LOFTY_SL("}")) {
}

vector_to_text_ostream::~vector_to_text_ostream() {
}

str vector_to_text_ostream::set_format(str const & format) {
   LOFTY_TRACE_FUNC(this, format);

   auto itr(format.cbegin());

   // Add parsing of the format string here.
   // TODO: parse format and store the appropriate element format in elt_format.
   str elt_format;

   throw_on_unused_streaming_format_chars(itr, format);

   return _std::move(elt_format);
}

}}} //namespace lofty::collections::_pvt
