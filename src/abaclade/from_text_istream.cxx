/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016 Raffaello D. Di Napoli

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
#include <abaclade/from_str.hxx>
#include <abaclade/text.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _pvt {

ABACLADE_SYM void throw_on_unused_from_str_chars(
   io::text::str_istream const & sis, str const & sSrc, str const & sFormat
) {
   if (std::size_t cchRemaining = sis.remaining_size_in_chars()) {
      // There are still unused characters in sis, so the conversion failed.
      ABC_THROW(text::syntax_error, (
         ABC_SL("unexpected character"), sFormat,
         static_cast<unsigned>(sSrc.index_from_char_index(sSrc.size_in_chars() - cchRemaining))
      ));
   }
}

}} //namespace abc::_pvt

namespace abc {

void throw_on_unused_streaming_format_chars(
   str::const_iterator const & itConsumedEnd, str const & sFormat
) {
   if (itConsumedEnd != sFormat.cend()) {
      ABC_THROW(text::syntax_error, (
         ABC_SL("unexpected character in format string"), sFormat,
         static_cast<unsigned>(itConsumedEnd - sFormat.cbegin())
      ));
   }
}

} //namespace abc
