/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#include <abaclade.hxx>
#include <abaclade/bitmanip.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::bitmanip globals

namespace abc {
namespace bitmanip {
namespace detail {

std::uint8_t ceiling_to_pow2(std::uint8_t i) {
   unsigned iPow2 = static_cast<unsigned>(i - 1);
   iPow2 |= iPow2 >> 1;
   iPow2 |= iPow2 >> 2;
   iPow2 |= iPow2 >> 4;
   return static_cast<std::uint8_t>(iPow2 + 1);
}
std::uint16_t ceiling_to_pow2(std::uint16_t i) {
   unsigned iPow2 = static_cast<unsigned>(i - 1);
   iPow2 |= iPow2 >> 1;
   iPow2 |= iPow2 >> 2;
   iPow2 |= iPow2 >> 4;
   iPow2 |= iPow2 >> 8;
   return static_cast<std::uint16_t>(iPow2 + 1);
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

} //namespace detail
} //namespace bitmanip
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
