/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

Copyright 2010, 2011, 2012, 2013
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef ABC_BITMANIP_HXX
#define ABC_BITMANIP_HXX

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::bitmanip globals


namespace abc {

namespace bitmanip {

/* Intrinsics in VS2010:
•	_BitScanForward
•	_BitScanReverse
•	_bittest
•	_bittestandcomplement
•	_bittestandreset
•	_bittestandset
•	_InterlockedCompareExchange
•	_InterlockedCompareExchange64
*/


/** Helper for ceiling_to_pow2, to unify specializations based on sizeof(I).

TODO: comment signature.
*/
template <typename I>
I _raw_ceiling_to_pow2(I i);


/** Returns the argument rounded up to the closest power of 2.

TODO: comment signature.
*/
template <typename I>
inline I ceiling_to_pow2(I i) {
	switch (sizeof(I)) {
		case sizeof(uint8_t):
			return _raw_ceiling_to_pow2(uint8_t(i));
		case sizeof(uint16_t):
			return _raw_ceiling_to_pow2(uint16_t(i));
		case sizeof(uint32_t):
			return _raw_ceiling_to_pow2(uint32_t(i));
		case sizeof(uint64_t):
			return _raw_ceiling_to_pow2(uint64_t(i));
	}
}


/** Returns the first argument rounded up to a multiple of the second, which has to be a power of 2.

TODO: comment signature.
*/
template <typename I>
inline /*constexpr*/ I ceiling_to_pow2_multiple(I i, I iStep) {
	--iStep;
	return (i + iStep) & ~iStep;
}


/** Rotates bits to the left (most significant bits shifted out, and back in to become least
significant).

TODO: comment signature.
*/
template <typename I>
inline /*constexpr*/ I rotate_l(I i, unsigned c) {
	return (i << c) | (i >> (sizeof(I) * CHAR_BIT - c));
}


/** Rotates bits to the right (least significant bits shifted out, and back in to become most
significant).

TODO: comment signature.
*/
template <typename I>
inline /*constexpr*/ I rotate_r(I i, unsigned c) {
	return (i >> c) | (i << (sizeof(I) * CHAR_BIT - c));
}

} //namespace bitmanip

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_BITMANIP_HXX

