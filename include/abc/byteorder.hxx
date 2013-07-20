// -*- coding: utf-8; mode: c++; tab-width: 3 -*-
//--------------------------------------------------------------------------------------------------
// Application-Building Components
// Copyright 2010-2013 Raffaello D. Di Napoli
//--------------------------------------------------------------------------------------------------
// This file is part of Application-Building Components (henceforth referred to as ABC).
//
// ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ABC. If not, see
// <http://www.gnu.org/licenses/>.
//--------------------------------------------------------------------------------------------------

#ifndef ABC_BYTEORDER_HXX
#define ABC_BYTEORDER_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/core.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// Declarations

#define ABC_HOST_LITTLE_ENDIAN 0
#define ABC_HOST_BIG_ENDIAN 0

#if _GCC_VER
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		#undef ABC_HOST_LITTLE_ENDIAN
		#define ABC_HOST_LITTLE_ENDIAN 1
	#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		#undef ABC_HOST_BIG_ENDIAN
		#define ABC_HOST_BIG_ENDIAN 1
	#endif
#elif defined(__GLIBC__)
	#include <endian.h> // BYTE_ORDER *_ENDIAN

	#if __BYTE_ORDER == __LITTLE_ENDIAN
		#undef ABC_HOST_LITTLE_ENDIAN
		#define ABC_HOST_LITTLE_ENDIAN 1
	#elif __BYTE_ORDER == __BIG_ENDIAN
		#undef ABC_HOST_BIG_ENDIAN
		#define ABC_HOST_BIG_ENDIAN 1
	#endif
#elif defined(__i386__)   || defined(_M_IX86)  || \
		defined(__x86_64__) || defined(_M_X64)   || defined(_M_AMD64) || \
		defined(__ia64__)   || defined(_M_IA64)  || \
		defined(__alpha__)  || defined(_M_ALPHA)
	#undef ABC_HOST_LITTLE_ENDIAN
	#define ABC_HOST_LITTLE_ENDIAN 1
#elif defined(__powerpc__) || defined(_M_PPC) || defined(_M_MPPC)
	#undef ABC_HOST_BIG_ENDIAN
	#define ABC_HOST_BIG_ENDIAN 1
#endif

#if !ABC_HOST_LITTLE_ENDIAN && !ABC_HOST_BIG_ENDIAN
	#error Unable to detect byte order (endianness)
#endif


// Define byte reordering functions.

#if defined(__GLIBC__)
	#include <byteswap.h> // bswap_*()
	#define ABC_HAVE_BSWAP
#else
	uint16_t bswap_16(uint16_t i);
	uint32_t bswap_32(uint32_t i);
	uint64_t bswap_64(uint64_t i);
#endif


namespace abc {

namespace byteorder {

/// Unconditionally flips the byte order in a number. It’s only defined for types ranging in size
// from 2 to 8 bytes.
template <typename I>
I swap(I i);

/// Converts a number from host endianness to big endian.
template <typename I>
I host_to_be(I i);

/// Converts a number from host endianness to little endian.
template <typename I>
I host_to_le(I i);

/// Converts a number from big endian to host endianness.
template <typename I>
I be_to_host(I i);

/// Converts a number from little endian to host endianness.
template <typename I>
I le_to_host(I i);

} //namespace byteorder

} //namespace abc


// Define compile-time counterparts to the above functions.
// These can all go away as soon as constexpr is available on all supported compilers.

/// Unconditionally flips the byte order in a 16-bit number.
//
#define STATIC_BYTEORDER_SWAP16(i) \
	( \
		((uint16_t(i) & 0xff00) >> 8) | \
		((uint16_t(i) & 0x00ff) << 8) \
	)

/// Unconditionally flips the byte order in a 32-bit number.
//
#define STATIC_BYTEORDER_SWAP32(i) \
	( \
		((uint32_t(i) & 0xff000000) >> 24) | \
		((uint32_t(i) & 0x00ff0000) >>  8) | \
		((uint32_t(i) & 0x0000ff00) <<  8) | \
		((uint32_t(i) & 0x000000ff) << 24) \
	)

/// Unconditionally flips the byte order in a 64-bit number.
//
#define STATIC_BYTEORDER_SWAP64(i) \
	( \
		((uint64_t(i) & 0xff00000000000000ull) >> 56) | \
		((uint64_t(i) & 0x00ff000000000000ull) >> 40) | \
		((uint64_t(i) & 0x0000ff0000000000ull) >> 24) | \
		((uint64_t(i) & 0x000000ff00000000ull) >>  8) | \
		((uint64_t(i) & 0x00000000ff000000ull) <<  8) | \
		((uint64_t(i) & 0x0000000000ff0000ull) << 24) | \
		((uint64_t(i) & 0x000000000000ff00ull) << 40) | \
		((uint64_t(i) & 0x00000000000000ffull) << 56) \
	)


#if ABC_HOST_LITTLE_ENDIAN
	/// Converts a 16-bit number from host endianness to big endian.
	#define STATIC_BYTEORDER_HOSTTOBE16(i) STATIC_BYTEORDER_SWAP16(i)
	/// Converts a 32-bit number from host endianness to big endian.
	#define STATIC_BYTEORDER_HOSTTOBE32(i) STATIC_BYTEORDER_SWAP32(i)
	/// Converts a 64-bit number from host endianness to big endian.
	#define STATIC_BYTEORDER_HOSTTOBE64(i) STATIC_BYTEORDER_SWAP64(i)

	/// Converts a 16-bit number from host endianness to little endian.
	#define STATIC_BYTEORDER_HOSTTOLE16(i) uint16_t(i)
	/// Converts a 32-bit number from host endianness to little endian.
	#define STATIC_BYTEORDER_HOSTTOLE32(i) uint32_t(i)
	/// Converts a 64-bit number from host endianness to little endian.
	#define STATIC_BYTEORDER_HOSTTOLE64(i) uint64_t(i)

	/// Converts a 16-bit number from big endian to host endianness.
	#define STATIC_BYTEORDER_BETOHOST16(i) STATIC_BYTEORDER_SWAP16(i)
	/// Converts a 32-bit number from big endian to host endianness.
	#define STATIC_BYTEORDER_BETOHOST32(i) STATIC_BYTEORDER_SWAP32(i)
	/// Converts a 64-bit number from big endian to host endianness.
	#define STATIC_BYTEORDER_BETOHOST64(i) STATIC_BYTEORDER_SWAP64(i)

	/// Converts a 16-bit number from little endian to host endianness.
	#define STATIC_BYTEORDER_LETOHOST16(i) uint16_t(i)
	/// Converts a 32-bit number from little endian to host endianness.
	#define STATIC_BYTEORDER_LETOHOST32(i) uint32_t(i)
	/// Converts a 64-bit number from little endian to host endianness.
	#define STATIC_BYTEORDER_LETOHOST64(i) uint64_t(i)
#else //if ABC_HOST_LITTLE_ENDIAN
	/// Converts a 16-bit number from host endianness to big endian.
	#define STATIC_BYTEORDER_HOSTTOBE16(i) uint16_t(i)
	/// Converts a 32-bit number from host endianness to big endian.
	#define STATIC_BYTEORDER_HOSTTOBE32(i) uint32_t(i)
	/// Converts a 64-bit number from host endianness to big endian.
	#define STATIC_BYTEORDER_HOSTTOBE64(i) uint64_t(i)

	/// Converts a 16-bit number from host endianness to little endian.
	#define STATIC_BYTEORDER_HOSTTOLE16(i) STATIC_BYTEORDER_SWAP16(i)
	/// Converts a 32-bit number from host endianness to little endian.
	#define STATIC_BYTEORDER_HOSTTOLE32(i) STATIC_BYTEORDER_SWAP32(i)
	/// Converts a 64-bit number from host endianness to little endian.
	#define STATIC_BYTEORDER_HOSTTOLE64(i) STATIC_BYTEORDER_SWAP64(i)

	/// Converts a 16-bit number from big endian to host endianness.
	#define STATIC_BYTEORDER_BETOHOST16(i) uint16_t(i)
	/// Converts a 32-bit number from big endian to host endianness.
	#define STATIC_BYTEORDER_BETOHOST32(i) uint32_t(i)
	/// Converts a 64-bit number from big endian to host endianness.
	#define STATIC_BYTEORDER_BETOHOST64(i) uint64_t(i)

	/// Converts a 16-bit number from little endian to host endianness.
	#define STATIC_BYTEORDER_LETOHOST16(i) STATIC_BYTEORDER_SWAP16(i)
	/// Converts a 32-bit number from little endian to host endianness.
	#define STATIC_BYTEORDER_LETOHOST32(i) STATIC_BYTEORDER_SWAP32(i)
	/// Converts a 64-bit number from little endian to host endianness.
	#define STATIC_BYTEORDER_LETOHOST64(i) STATIC_BYTEORDER_SWAP64(i)
#endif //if ABC_HOST_LITTLE_ENDIAN … else



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::byteorder globals


namespace abc {

namespace byteorder {

template <typename I>
inline I swap(I i) {
	switch (sizeof(I)) {
		case sizeof(uint16_t):
			return I(bswap_16(uint16_t(i)));
		case sizeof(uint32_t):
			return I(bswap_32(uint32_t(i)));
		case sizeof(uint64_t):
			return I(bswap_64(uint64_t(i)));
	}
}


template <typename I>
inline I host_to_be(I i) {
#if ABC_HOST_LITTLE_ENDIAN
	return swap(i);
#else
	return i;
#endif
}


template <typename I>
inline I host_to_le(I i) {
#if ABC_HOST_LITTLE_ENDIAN
	return i;
#else
	return swap(i);
#endif
}


template <typename I>
inline I be_to_host(I i) {
#if ABC_HOST_LITTLE_ENDIAN
	return swap(i);
#else
	return i;
#endif
}


template <typename I>
inline I le_to_host(I i) {
#if ABC_HOST_LITTLE_ENDIAN
	return i;
#else
	return swap(i);
#endif
}

} //namespace byteorder

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_BYTEORDER_HXX

