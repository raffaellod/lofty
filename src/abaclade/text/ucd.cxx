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
#include <abaclade/text/ucd.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text { namespace ucd {

#define _ABC_TEXT_PROPERTY_BEGIN(name) \
   static _pvt::property_data::member_range const ABC_CPP_CAT(gc_pdmr_, name)[] = {

#define _ABC_TEXT_PROPERTY_END(name) \
   }; \
   static _pvt::property_data const ABC_CPP_CAT(gc_pd_, name) = { \
      ABC_SL(#name), \
      static_cast<std::uint8_t>(ABC_COUNTOF(#name) - 1 /*NUL*/), \
      ABC_CPP_CAT(gc_pdmr_, name), \
      &ABC_CPP_CAT(gc_pdmr_, name)[ABC_COUNTOF(ABC_CPP_CAT(gc_pdmr_, name))] \
   }; \
   property const & property::name = static_cast<property const &>(ABC_CPP_CAT(gc_pd_, name));

// TODO: generate these lists programmatically from PropList.txt.

_ABC_TEXT_PROPERTY_BEGIN(white_space)
   { 0x0009, 0x000d },
   { 0x0020, 0x0020 },
   { 0x0085, 0x0085 },
   { 0x00a0, 0x00a0 },
   { 0x1680, 0x1680 },
   { 0x2000, 0x200a },
   { 0x2028, 0x2028 },
   { 0x2029, 0x2029 },
   { 0x202f, 0x202f },
   { 0x205f, 0x205f },
   { 0x3000, 0x3000 }
_ABC_TEXT_PROPERTY_END(white_space)

#undef _ABC_TEXT_PROPERTY_BEGIN
#undef _ABC_TEXT_PROPERTY_END

str property::members() const {
   ABC_TRACE_FUNC(this);

   str s;
   for (member_range const * pmr = m_pmrBegin; pmr < m_pmrEnd; ++pmr) {
      for (char32_t cp = pmr->cpFirst; cp <= pmr->cpLast; ++cp) {
         s += cp;
      }
   }
   return _std::move(s);
}

bool property::test(char32_t cp) const {
   for (member_range const * pmr = m_pmrBegin; pmr < m_pmrEnd; ++pmr) {
      if (cp >= pmr->cpFirst && cp <= pmr->cpLast) {
         return true;
      }
   }
   return false;
}

}}} //namespace abc::text::ucd
