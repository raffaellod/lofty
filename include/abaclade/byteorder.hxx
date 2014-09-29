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

#ifndef _ABACLADE_HXX_INTERNAL
   #error Please #include <abaclade.hxx> instead of this file
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::byteorder globals

#define ABC_HOST_LITTLE_ENDIAN 0
#define ABC_HOST_BIG_ENDIAN 0

#if ABC_HOST_GCC
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
   ABACLADE_SYM std::uint16_t bswap_16(std::uint16_t i);
   ABACLADE_SYM std::uint32_t bswap_32(std::uint32_t i);
   ABACLADE_SYM std::uint64_t bswap_64(std::uint64_t i);
#endif


namespace abc {
namespace byteorder {

//! Implementation of swap(), specialized by size in bytes of the argument. See swap().
template <std::size_t cb>
struct _swap_impl;

// Specialization for 1-byte integers.
template <>
struct _swap_impl<1> {
   typedef std::uint8_t type;

   type operator()(type i) {
      // No byte swapping on a single byte.
      return i;
   }
};

// Specialization for 2-byte integers.
template <>
struct _swap_impl<2> {
   typedef std::uint16_t type;

   type operator()(type i) {
      return bswap_16(i);
   }
};

// Specialization for 4-byte integers.
template <>
struct _swap_impl<4> {
   typedef std::uint32_t type;

   type operator()(type i) {
      return bswap_32(i);
   }
};

// Specialization for 8-byte integers.
template <>
struct _swap_impl<8> {
   typedef std::uint64_t type;

   type operator()(type i) {
      return bswap_64(i);
   }
};

/*! Unconditionally flips the byte order in a number. It’s only defined for types ranging in size
from 2 to 8 bytes.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order.
*/
template <typename I>
inline I swap(I i) {
   typedef _swap_impl<sizeof(I)> swap_impl;
   return I(swap_impl()(typename swap_impl::type(i)));
}

/*! Converts a number from host endianness to big endian.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order if the host is little endian.
*/
template <typename I>
inline I host_to_be(I i) {
#if ABC_HOST_LITTLE_ENDIAN
   return swap(i);
#else
   return i;
#endif
}

/*! Converts a number from host endianness to little endian.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order if the host is big endian.
*/
template <typename I>
inline I host_to_le(I i) {
#if ABC_HOST_LITTLE_ENDIAN
   return i;
#else
   return swap(i);
#endif
}

/*! Converts a number from big endian to host endianness.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order if the host is little endian.
*/
template <typename I>
inline I be_to_host(I i) {
#if ABC_HOST_LITTLE_ENDIAN
   return swap(i);
#else
   return i;
#endif
}

/*! Converts a number from little endian to host endianness.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order if the host is big endian.
*/
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


// Define compile-time counterparts to the above functions.
// These can all go away as soon as constexpr is available on all supported compilers.

/*! Unconditionally flips the byte order in a 16-bit number.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order.
*/
#define ABC_BYTEORDER_SWAP16(i) \
   ( \
      ((static_cast<std::uint16_t>(i) & 0xff00) >> 8) | \
      ((static_cast<std::uint16_t>(i) & 0x00ff) << 8) \
   )

/*! Unconditionally flips the byte order in a 32-bit number.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order.
*/
#define ABC_BYTEORDER_SWAP32(i) \
   ( \
      ((static_cast<std::uint32_t>(i) & 0xff000000) >> 24) | \
      ((static_cast<std::uint32_t>(i) & 0x00ff0000) >>  8) | \
      ((static_cast<std::uint32_t>(i) & 0x0000ff00) <<  8) | \
      ((static_cast<std::uint32_t>(i) & 0x000000ff) << 24) \
   )

/*! Unconditionally flips the byte order in a 64-bit number.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order.
*/
#define ABC_BYTEORDER_SWAP64(i) \
   ( \
      ((static_cast<std::uint64_t>(i) & 0xff00000000000000ull) >> 56) | \
      ((static_cast<std::uint64_t>(i) & 0x00ff000000000000ull) >> 40) | \
      ((static_cast<std::uint64_t>(i) & 0x0000ff0000000000ull) >> 24) | \
      ((static_cast<std::uint64_t>(i) & 0x000000ff00000000ull) >>  8) | \
      ((static_cast<std::uint64_t>(i) & 0x00000000ff000000ull) <<  8) | \
      ((static_cast<std::uint64_t>(i) & 0x0000000000ff0000ull) << 24) | \
      ((static_cast<std::uint64_t>(i) & 0x000000000000ff00ull) << 40) | \
      ((static_cast<std::uint64_t>(i) & 0x00000000000000ffull) << 56) \
   )

/*! Converts a 16-bit number from host endianness to big endian.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order if the host is little endian.
*/
#if ABC_HOST_LITTLE_ENDIAN
   #define ABC_BYTEORDER_HOSTTOBE16(i) ABC_BYTEORDER_SWAP16(i)
#else
   #define ABC_BYTEORDER_HOSTTOBE16(i) static_cast<std::uint16_t>(i)
#endif

/*! Converts a 32-bit number from host endianness to big endian.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order if the host is little endian.
*/
#if ABC_HOST_LITTLE_ENDIAN
   #define ABC_BYTEORDER_HOSTTOBE32(i) ABC_BYTEORDER_SWAP32(i)
#else
   #define ABC_BYTEORDER_HOSTTOBE32(i) static_cast<std::uint32_t>(i)
#endif

/*! Converts a 64-bit number from host endianness to big endian.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order if the host is little endian.
*/
#if ABC_HOST_LITTLE_ENDIAN
   #define ABC_BYTEORDER_HOSTTOBE64(i) ABC_BYTEORDER_SWAP64(i)
#else
   #define ABC_BYTEORDER_HOSTTOBE64(i) static_cast<std::uint64_t>(i)
#endif

/*! Converts a 16-bit number from host endianness to little endian.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order if the host is big endian.
*/
#if ABC_HOST_LITTLE_ENDIAN
   #define ABC_BYTEORDER_HOSTTOLE16(i) static_cast<std::uint16_t>(i)
#else
   #define ABC_BYTEORDER_HOSTTOLE16(i) ABC_BYTEORDER_SWAP16(i)
#endif

/*! Converts a 32-bit number from host endianness to little endian.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order if the host is big endian.
*/
#if ABC_HOST_LITTLE_ENDIAN
   #define ABC_BYTEORDER_HOSTTOLE32(i) static_cast<std::uint32_t>(i)
#else
   #define ABC_BYTEORDER_HOSTTOLE32(i) ABC_BYTEORDER_SWAP32(i)
#endif

/*! Converts a 64-bit number from host endianness to little endian.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order if the host is big endian.
*/
#if ABC_HOST_LITTLE_ENDIAN
   #define ABC_BYTEORDER_HOSTTOLE64(i) static_cast<std::uint64_t>(i)
#else
   #define ABC_BYTEORDER_HOSTTOLE64(i) ABC_BYTEORDER_SWAP64(i)
#endif

/*! Converts a 16-bit number from big endian to host endianness.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order if the host is little endian.
*/
#if ABC_HOST_LITTLE_ENDIAN
   #define ABC_BYTEORDER_BETOHOST16(i) ABC_BYTEORDER_SWAP16(i)
#else
   #define ABC_BYTEORDER_BETOHOST16(i) static_cast<std::uint16_t>(i)
#endif

/*! Converts a 32-bit number from big endian to host endianness.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order if the host is little endian.
*/
#if ABC_HOST_LITTLE_ENDIAN
   #define ABC_BYTEORDER_BETOHOST32(i) ABC_BYTEORDER_SWAP32(i)
#else
   #define ABC_BYTEORDER_BETOHOST32(i) static_cast<std::uint32_t>(i)
#endif

/*! Converts a 64-bit number from big endian to host endianness.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order if the host is little endian.
*/
#if ABC_HOST_LITTLE_ENDIAN
   #define ABC_BYTEORDER_BETOHOST64(i) ABC_BYTEORDER_SWAP64(i)
#else
   #define ABC_BYTEORDER_BETOHOST64(i) static_cast<std::uint64_t>(i)
#endif

/*! Converts a 16-bit number from little endian to host endianness.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order if the host is big endian.
*/
#if ABC_HOST_LITTLE_ENDIAN
   #define ABC_BYTEORDER_LETOHOST16(i) static_cast<std::uint16_t>(i)
#else
   #define ABC_BYTEORDER_LETOHOST16(i) ABC_BYTEORDER_SWAP16(i)
#endif

/*! Converts a 32-bit number from little endian to host endianness.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order if the host is big endian.
*/
#if ABC_HOST_LITTLE_ENDIAN
   #define ABC_BYTEORDER_LETOHOST32(i) static_cast<std::uint32_t>(i)
#else
   #define ABC_BYTEORDER_LETOHOST32(i) ABC_BYTEORDER_SWAP32(i)
#endif

/*! Converts a 64-bit number from little endian to host endianness.

i
   Source integer.
return
   Integer with the same byte values as i, but in reverse order if the host is big endian.
*/
#if ABC_HOST_LITTLE_ENDIAN
   #define ABC_BYTEORDER_LETOHOST64(i) static_cast<std::uint64_t>(i)
#else
   #define ABC_BYTEORDER_LETOHOST64(i) ABC_BYTEORDER_SWAP64(i)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

