/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_BYTE_ORDER_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_BYTE_ORDER_HXX
#endif

#ifndef _LOFTY_BYTE_ORDER_HXX_NOPUB
#define _LOFTY_BYTE_ORDER_HXX_NOPUB

#include <lofty/_pvt/lofty.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Byte-ordering functions.
namespace byte_order {}

} //namespace lofty

// Define byte reordering functions.
#if defined(__GLIBC__)
   #include <byteswap.h> // bswap_*()
   #define LOFTY_HAVE_BSWAP
#endif
//! @cond
#ifndef LOFTY_HAVE_BSWAP
   namespace lofty { namespace byte_order { namespace _pvt {

   LOFTY_SYM std::uint16_t bswap_16(std::uint16_t i);
   LOFTY_SYM std::uint32_t bswap_32(std::uint32_t i);
   LOFTY_SYM std::uint64_t bswap_64(std::uint64_t i);

   }}} //namespace lofty::byte_order::_pvt
#endif

namespace lofty { namespace byte_order { namespace _pvt {

//! Implementation of swap(), specialized by size in bytes of the argument. See swap().
template <std::size_t size>
struct swap_impl;

// Specialization for 1-byte integers.
template <>
struct swap_impl<1> {
   typedef std::uint8_t type;

   type operator()(type i) {
      // No byte swapping on a single byte.
      return i;
   }
};

// Specialization for 2-byte integers.
template <>
struct swap_impl<2> {
   typedef std::uint16_t type;

   type operator()(type i) {
      return bswap_16(i);
   }
};

// Specialization for 4-byte integers.
template <>
struct swap_impl<4> {
   typedef std::uint32_t type;

   type operator()(type i) {
      return bswap_32(i);
   }
};

// Specialization for 8-byte integers.
template <>
struct swap_impl<8> {
   typedef std::uint64_t type;

   type operator()(type i) {
      return bswap_64(i);
   }
};

}}} //namespace lofty::byte_order::_pvt
//! @endcond

namespace lofty { namespace byte_order {
_LOFTY_PUBNS_BEGIN

/*! Unconditionally flips the byte order in a number. It’s only defined for types ranging in size from 2 to 8
bytes.

@param i
   Source integer.
@return
   Integer with the same byte values as i, but in reverse order.
*/
template <typename I>
inline I swap(I i) {
   typedef _pvt::swap_impl<sizeof(I)> swap_impl;
   return static_cast<I>(swap_impl()(static_cast<typename swap_impl::type>(i)));
}

/*! Converts a number from big endian to host endianness.

@param i
   Source integer.
@return
   Integer with the same byte values as i, but in reverse order if the host is little endian.
*/
template <typename I>
inline I be_to_host(I i) {
#if LOFTY_HOST_LITTLE_ENDIAN
   return swap(i);
#else
   return i;
#endif
}

/*! Converts a number from host endianness to big endian.

@param i
   Source integer.
@return
   Integer with the same byte values as i, but in reverse order if the host is little endian.
*/
template <typename I>
inline I host_to_be(I i) {
#if LOFTY_HOST_LITTLE_ENDIAN
   return swap(i);
#else
   return i;
#endif
}

/*! Converts a number from host endianness to little endian.

@param i
   Source integer.
@return
   Integer with the same byte values as i, but in reverse order if the host is big endian.
*/
template <typename I>
inline I host_to_le(I i) {
#if LOFTY_HOST_LITTLE_ENDIAN
   return i;
#else
   return swap(i);
#endif
}

/*! Converts a number from little endian to host endianness.

@param i
   Source integer.
@return
   Integer with the same byte values as i, but in reverse order if the host is big endian.
*/
template <typename I>
inline I le_to_host(I i) {
#if LOFTY_HOST_LITTLE_ENDIAN
   return i;
#else
   return swap(i);
#endif
}

_LOFTY_PUBNS_END
}} //namespace lofty::byte_order

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_BYTE_ORDER_HXX_NOPUB

#ifdef _LOFTY_BYTE_ORDER_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace byte_order {

   using _pub::swap;
   using _pub::be_to_host;
   using _pub::host_to_be;
   using _pub::host_to_le;
   using _pub::le_to_host;

   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_BYTE_ORDER_HXX
