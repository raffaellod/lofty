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


////////////////////////////////////////////////////////////////////////////////////////////////////
// :: globals – helpers for abc::byteorder globals

#ifndef ABC_HAVE_BSWAP

std::uint16_t bswap_16(std::uint16_t i) {
   return
      ((i & 0xff00u) >> 8) |
      ((i & 0x00ffu) << 8);
}

std::uint32_t bswap_32(std::uint32_t i) {
   return
      ((i & 0xff000000u) >> 24) |
      ((i & 0x00ff0000u) >>  8) |
      ((i & 0x0000ff00u) <<  8) |
      ((i & 0x000000ffu) << 24);
}

std::uint64_t bswap_64(std::uint64_t i) {
   return
      ((i & 0xff00000000000000ull) >> 56) |
      ((i & 0x00ff000000000000ull) >> 40) |
      ((i & 0x0000ff0000000000ull) >> 24) |
      ((i & 0x000000ff00000000ull) >>  8) |
      ((i & 0x00000000ff000000ull) <<  8) |
      ((i & 0x0000000000ff0000ull) << 24) |
      ((i & 0x000000000000ff00ull) << 40) |
      ((i & 0x00000000000000ffull) << 56);
}

#endif //ifndef ABC_HAVE_BSWAP

////////////////////////////////////////////////////////////////////////////////////////////////////

