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
#include <lofty/byte_order.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef LOFTY_HAVE_BSWAP

namespace lofty { namespace byte_order { namespace _pvt {

std::uint16_t bswap_16(std::uint16_t i) {
   return std::uint16_t(
      ((i & std::uint16_t(0xff00u)) >> 8) |
      ((i & std::uint16_t(0x00ffu)) << 8)
   );
}

std::uint32_t bswap_32(std::uint32_t i) {
   return std::uint32_t(
      ((i & std::uint32_t(0xff000000u)) >> 24) |
      ((i & std::uint32_t(0x00ff0000u)) >>  8) |
      ((i & std::uint32_t(0x0000ff00u)) <<  8) |
      ((i & std::uint32_t(0x000000ffu)) << 24)
   );
}

std::uint64_t bswap_64(std::uint64_t i) {
   return std::uint64_t(
      ((i & std::uint64_t(0xff00000000000000u)) >> 56) |
      ((i & std::uint64_t(0x00ff000000000000u)) >> 40) |
      ((i & std::uint64_t(0x0000ff0000000000u)) >> 24) |
      ((i & std::uint64_t(0x000000ff00000000u)) >>  8) |
      ((i & std::uint64_t(0x00000000ff000000u)) <<  8) |
      ((i & std::uint64_t(0x0000000000ff0000u)) << 24) |
      ((i & std::uint64_t(0x000000000000ff00u)) << 40) |
      ((i & std::uint64_t(0x00000000000000ffu)) << 56)
   );
}

}}} //namespace lofty::byte_order::_pvt

#endif //ifndef LOFTY_HAVE_BSWAP
