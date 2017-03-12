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
#include <lofty/bitmanip.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace bitmanip { namespace _pvt {

std::uint8_t ceiling_to_pow2(std::uint8_t i) {
   unsigned ret = static_cast<unsigned>(i - 1);
   ret |= ret >> 1;
   ret |= ret >> 2;
   ret |= ret >> 4;
   return static_cast<std::uint8_t>(ret + 1);
}
std::uint16_t ceiling_to_pow2(std::uint16_t i) {
   unsigned ret = static_cast<unsigned>(i - 1);
   ret |= ret >> 1;
   ret |= ret >> 2;
   ret |= ret >> 4;
   ret |= ret >> 8;
   return static_cast<std::uint16_t>(ret + 1);
}
std::uint32_t ceiling_to_pow2(std::uint32_t i) {
   --i;
   i |= i >> 1;
   i |= i >> 2;
   i |= i >> 4;
   i |= i >> 8;
   i |= i >> 16;
   return i + 1;
}
std::uint64_t ceiling_to_pow2(std::uint64_t i) {
   --i;
   i |= i >> 1;
   i |= i >> 2;
   i |= i >> 4;
   i |= i >> 8;
   i |= i >> 16;
   i |= i >> 32;
   return i + 1;
}

}}} //namespace lofty::bitmanip::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
