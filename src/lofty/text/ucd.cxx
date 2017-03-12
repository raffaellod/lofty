/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016-2017 Raffaello D. Di Napoli

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
#include <lofty/text/ucd.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text { namespace ucd {

#define _LOFTY_TEXT_PROPERTY_BEGIN(name) \
   static _pvt::property_data::member_range_t const LOFTY_CPP_CAT(_, name, _ranges)[] = {

#define _LOFTY_TEXT_PROPERTY_END(name) \
   }; \
   static _pvt::property_data const LOFTY_CPP_CAT(_, name, _data) = { \
      LOFTY_SL(#name), \
      static_cast<std::uint8_t>(LOFTY_COUNTOF(#name) - 1 /*NUL*/), \
      LOFTY_CPP_CAT(_, name, _ranges), \
      &LOFTY_CPP_CAT(_, name, _ranges)[LOFTY_COUNTOF(LOFTY_CPP_CAT(_, name, _ranges))] \
   }; \
   property const & property::name = static_cast<property const &>(LOFTY_CPP_CAT(_, name, _data));

// TODO: generate these lists programmatically from PropList.txt.

_LOFTY_TEXT_PROPERTY_BEGIN(white_space)
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
_LOFTY_TEXT_PROPERTY_END(white_space)

#undef _LOFTY_TEXT_PROPERTY_BEGIN
#undef _LOFTY_TEXT_PROPERTY_END

str property::members() const {
   LOFTY_TRACE_FUNC(this);

   str s;
   for (auto member_range = member_ranges_begin; member_range < member_ranges_end; ++member_range) {
      for (auto cp = member_range->first_cp; cp <= member_range->last_cp; ++cp) {
         s += cp;
      }
   }
   return _std::move(s);
}

bool property::test(char32_t cp) const {
   for (auto member_range = member_ranges_begin; member_range < member_ranges_end; ++member_range) {
      if (cp >= member_range->first_cp && cp <= member_range->last_cp) {
         return true;
      }
   }
   return false;
}

}}} //namespace lofty::text::ucd
