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

#include <abaclade.hxx>
#include <abaclade/collections/vector.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace _pvt {

vector_to_text_ostream::vector_to_text_ostream() :
   abc::_pvt::sequence_to_text_ostream(ABC_SL("{"), ABC_SL("}")) {
}

vector_to_text_ostream::~vector_to_text_ostream() {
}

str vector_to_text_ostream::set_format(str const & sFormat) {
   ABC_TRACE_FUNC(this, sFormat);

   auto it(sFormat.cbegin());

   // Add parsing of the format string here.
   // TODO: parse sFormat and store the appropriate element format in sEltFormat.
   str sEltFormat;

   throw_on_unused_streaming_format_chars(it, sFormat);

   return _std::move(sEltFormat);
}

}}} //namespace abc::collections::_pvt
