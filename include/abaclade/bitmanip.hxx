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

#ifndef _ABACLADE_BITMANIP_HXX
#define _ABACLADE_BITMANIP_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <climits> // CHAR_BIT


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::bitmanip globals

namespace abc {
namespace bitmanip {

/* Intrinsics in MSC16:
•  _BitScanForward
•  _BitScanReverse
•  _bittest
•  _bittestandcomplement
•  _bittestandreset
•  _bittestandset
•  _InterlockedCompareExchange
•  _InterlockedCompareExchange64
*/

namespace detail {

/*! Helper for ceiling_to_pow2(), to unify specializations based on sizeof(T). See
abc::bitmanip::ceiling_to_pow2(). */
ABACLADE_SYM std::uint8_t ceiling_to_pow2(std::uint8_t i);
ABACLADE_SYM std::uint16_t ceiling_to_pow2(std::uint16_t i);
ABACLADE_SYM std::uint32_t ceiling_to_pow2(std::uint32_t i);
ABACLADE_SYM std::uint64_t ceiling_to_pow2(std::uint64_t i);

} //namespace detail


/*! Returns the argument rounded up to the closest power of 2.

@param i
   Integer to round up.
@return
   Smallest power of 2 that’s not smaller than i.
*/
template <typename T>
inline T ceiling_to_pow2(T i) {
   static_assert(
      sizeof(T) == sizeof(std::uint8_t ) || sizeof(T) == sizeof(std::uint16_t) ||
      sizeof(T) == sizeof(std::uint32_t) || sizeof(T) == sizeof(std::uint64_t),
      "sizeof(T) is not a supported power of 2"
   );
   switch (sizeof(T)) {
      case sizeof(std::uint8_t):
         return detail::ceiling_to_pow2(static_cast<std::uint8_t>(i));
      case sizeof(std::uint16_t):
         return detail::ceiling_to_pow2(static_cast<std::uint16_t>(i));
      case sizeof(std::uint32_t):
         return detail::ceiling_to_pow2(static_cast<std::uint32_t>(i));
      case sizeof(std::uint64_t):
         return detail::ceiling_to_pow2(static_cast<std::uint64_t>(i));
   }
}

/*! Returns the first argument rounded up to a multiple of the second, which has to be a power of 2.

@param i
   Integer to round up.
@param iStep
   Power of 2 to use as step to increment i.
@return
   Smallest multiple of iStep that’s not smaller than i.
*/
template <typename T>
inline /*constexpr*/ T ceiling_to_pow2_multiple(T i, T iStep) {
   --iStep;
   return (i + iStep) & ~iStep;
}

/*! Rotates bits to the left (most significant bits shifted out, and back in to become least
significant).

@param i
   Integer the bits of which are to be rotated.
@param c
   Count of positions the bits in i will be shifted.
@return
   Rotated bits of i.
*/
template <typename T>
inline /*constexpr*/ T rotate_l(T i, unsigned c) {
   return (i << c) | (i >> (sizeof(T) * CHAR_BIT - c));
}

/*! Rotates bits to the right (least significant bits shifted out, and back in to become most
significant).

@param i
   Integer the bits of which are to be rotated.
@param c
   Count of positions the bits in i will be shifted.
@return
   Rotated bits of i.
*/
template <typename T>
inline /*constexpr*/ T rotate_r(T i, unsigned c) {
   return (i >> c) | (i << (sizeof(T) * CHAR_BIT - c));
}

} //namespace bitmanip
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_BITMANIP_HXX
