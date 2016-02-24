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

void throw_on_unused_from_str_chars(io::text::str_istream const & sis) {
   if (std::size_t cchRemaining = sis.remaining_size_in_chars()) {
      // There are still unused characters in sis, so the conversion failed.
      str const & sSrc = sis.get_str();
      ABC_THROW(text::syntax_error, (
         ABC_SL("unexpected character"), sSrc,
         static_cast<unsigned>(sSrc.index_from_char_index(sSrc.size_in_chars() - cchRemaining))
      ));
   }
}

}} //namespace abc::_pvt

namespace abc {

void throw_on_unused_streaming_format_chars(
   str::const_iterator const & itFormatConsumedEnd, str const & sFormat
) {
   if (itFormatConsumedEnd != sFormat.cend()) {
      ABC_THROW(text::syntax_error, (
         ABC_SL("unexpected character in format string"), sFormat,
         static_cast<unsigned>(itFormatConsumedEnd - sFormat.cbegin())
      ));
   }
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

from_text_istream<bool>::from_text_istream() :
   m_sTrue(ABC_SL("true")),
   m_sFalse(ABC_SL("false")) {
}

void from_text_istream<bool>::set_format(str const & sFormat) {
   ABC_TRACE_FUNC(this, sFormat);

   throw_on_unused_streaming_format_chars(sFormat.cbegin(), sFormat);
}

void from_text_istream<bool>::read(bool * pb, io::text::istream * ptis) {
   ABC_TRACE_FUNC(this, pb, ptis);

   sstr<16> sRead;
   for (str sPeek; (sPeek = ptis->peek_chars(1)); ptis->consume_chars(sPeek.size_in_chars())) {
      for (auto itPeek(sPeek.cbegin()), itPeekEnd(sPeek.cend()); itPeek != itPeekEnd; ++itPeek) {
         char32_t cp = *itPeek;
         // TODO: replace this with the UCD equivalent of \w from abc::text.
         if ((cp < '0' || cp > '9') && (cp < 'A' || cp > 'Z') && (cp < 'a' || cp > 'z')) {
            // Consume all preceding characters and stop.
            ptis->consume_chars(itPeek.char_index());
            goto break_outer_for;
         }
         sRead += cp;
      }
   }
break_outer_for:
   if (sRead == m_sTrue) {
      *pb = true;
   } else if (sRead == m_sFalse) {
      *pb = false;
   } else {
      ptis->unconsume_chars(sRead.str());
      // TODO: provide more information in the exception and/or use a better exception class.
      ABC_THROW(text::syntax_error, (ABC_SL("unrecognized input"), sRead));
   }
}

} //namespace abc
