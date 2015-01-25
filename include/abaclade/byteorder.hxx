/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
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

#ifndef _ABACLADE_BYTEORDER_HXX
#define _ABACLADE_BYTEORDER_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::byteorder globals

// Define byte reordering functions.

#if defined(__GLIBC__)
   #include <byteswap.h> // bswap_*()
   #define ABC_HAVE_BSWAP
#endif


namespace abc {

#ifndef ABC_HAVE_BSWAP
   namespace detail {

   ABACLADE_SYM std::uint16_t bswap_16(std::uint16_t i);
   ABACLADE_SYM std::uint32_t bswap_32(std::uint32_t i);
   ABACLADE_SYM std::uint64_t bswap_64(std::uint64_t i);

   } //namespace detail
#endif

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
      using namespace detail;
      return bswap_16(i);
   }
};

// Specialization for 4-byte integers.
template <>
struct _swap_impl<4> {
   typedef std::uint32_t type;

   type operator()(type i) {
      using namespace detail;
      return bswap_32(i);
   }
};

// Specialization for 8-byte integers.
template <>
struct _swap_impl<8> {
   typedef std::uint64_t type;

   type operator()(type i) {
      using namespace detail;
      return bswap_64(i);
   }
};

/*! Unconditionally flips the byte order in a number. It’s only defined for types ranging in size
from 2 to 8 bytes.

@param i
   Source integer.
@return
   Integer with the same byte values as i, but in reverse order.
*/
template <typename I>
inline I swap(I i) {
   typedef _swap_impl<sizeof(I)> swap_impl;
   return I(swap_impl()(typename swap_impl::type(i)));
}

/*! Converts a number from host endianness to big endian.

@param i
   Source integer.
@return
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

@param i
   Source integer.
@return
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

@param i
   Source integer.
@return
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

@param i
   Source integer.
@return
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

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_BYTEORDER_HXX
