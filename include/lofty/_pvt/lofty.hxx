/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

/*! @file
File used to bootstrap Lofty; it’s always included by all Lofty header files. It’s in lofty/_pvt not because
it provides declarations in the lofty::_pvt namespace, but because it’s not meant to be included (directly) by
non-Lofty files. */

#ifndef _LOFTY__PVT_LOFTY_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY__PVT_LOFTY_HXX
#endif

#ifndef _LOFTY__PVT_LOFTY_HXX_NOPUB
#define _LOFTY__PVT_LOFTY_HXX_NOPUB

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! Lofty’s top-level namespace.
namespace lofty {
   /*! @internal lofty::_std contains STL implementation bits from Lofty’s STL incomplete implementation that
   we may want to use when _LOFTY_USE_STLIMPL is not defined, as Lofty-only alternatives to lacking/buggy host
   STL implementations. */
   namespace _std {}

   //! Support for performance tracking.
   namespace perf {}

   //! Top-level namespace for Lofty’s testing framework.
   namespace testing {}

   namespace text {

      //! Classes implementing semi-complete parsers, ready to be built upon.
      namespace parsers {}

   } //namespace text
} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond

// Only support Unicode Windows programs.
// TODO: support non-Unicode Windows programs (Win9x and Win16). In a very, very distant future – or past.
#if defined(_WIN32) && !defined(UNICODE)
   #define UNICODE
#endif

// Make sure DEBUG and _DEBUG are set if either is.
#if defined(DEBUG) && !defined(_DEBUG)
   #define _DEBUG
#elif defined(_DEBUG) && !defined(DEBUG)
   #define _DEBUG
#endif

//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <lofty/host.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {

/*! Type whose alignment requirement is at least as large as that of any scalar type (see C++11 § 18.2
“<cstddef>”). */
#if LOFTY_HOST_STL_LIBCXX || LOFTY_HOST_STL_LIBSTDCXX >= 40900
   using ::std::max_align_t;
#elif LOFTY_HOST_STL_LIBSTDCXX
   using ::max_align_t;
#else
   union max_align_t {
      //! @cond
      double d;
      long double ld;
      long long ll;
      //! @endcond
   };
#endif

}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Miscellaneous one-liners that don’t belong elsewhere or are used everywhere. Add sparingly.

/*! Avoids compiler warnings about purposely unused parameters. Win32 has UNREFERENCED_PARAMETER for this
purpose, but this is noticeably shorter :)

@param x
   Unused argument.
*/
#define LOFTY_UNUSED_ARG(x) \
   static_cast<void>(&x)

/*! Returns the number of items in a (static) array.

@param array
   Array for which to compute the count of items.
@return
   Count of items in array.
*/
#define LOFTY_COUNTOF(array) \
   (sizeof(array) / sizeof((array)[0]))

/*! Returns a size rounded (ceiling) to a count of _std::max_align_t units. This allows to declare storage
with alignment suitable for any type, just like std::malloc() does. Identical to
bitmanip::ceiling_to_pow2_multiple(size, sizeof(_std::max_align_t)).

@param size
   Size to be aligned to sizeof(_std::max_align_t).
@return
   Multiple of sizeof(_std::max_align_t) not smaller than size.
*/
#define LOFTY_ALIGNED_SIZE(size) \
   (\
      (static_cast<std::size_t>(size) + sizeof(::lofty::_std::max_align_t) - 1) / \
      sizeof(::lofty::_std::max_align_t) \
   )

/** Returns the offset of a member in a struct/class.

@param type
   Type containing the member.
@param member
   Identifier of the last member in type.
@return
   Offset of the specified member, in bytes.
*/
#define LOFTY_OFFSETOF(type, member) \
   (reinterpret_cast<std::size_t>(&reinterpret_cast<type *>(8192)->member) - 8192)

/** Returns the size of a struct/class, without padding added.

@param type
   Type of which to calculate the exact size.
@param last_member
   Identifier of the last member in type.
@return
   Size of type, in bytes. Guaranteed to be at most equal to sizeof(type).
*/
#if LOFTY_HOST_CXX_MSC
   /* MSC16/MSC18 BUG: sizeof last_member results in C2070 “illegal sizeof operand”, so we forcibly qualify
   last_member by pretending to retrieve it from an instance of the containing type. */
   #define LOFTY_UNPADDED_SIZEOF(type, last_member) \
      (LOFTY_OFFSETOF(type, last_member) + sizeof(reinterpret_cast<type *>(8192)->last_member))
#else
   #define LOFTY_UNPADDED_SIZEOF(type, last_member) \
      (LOFTY_OFFSETOF(type, last_member) + sizeof last_member)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Declares a symbol to be publicly visible (from the Lofty shared library) or imported from Lofty’s shared
library (into another library/executable). */
#ifdef COMPLEMAKE_BUILD_LOFTY
   #define LOFTY_SYM LOFTY_SYM_EXPORT
#else
   #define LOFTY_SYM LOFTY_SYM_IMPORT
#endif

/*! Declares a symbol to be publicly visible (from the Lofty testing shared library) or imported from Lofty’s
testing shared library (into another library/executable). */
#ifdef COMPLEMAKE_BUILD_LOFTY_TESTING
   #define LOFTY_TESTING_SYM LOFTY_SYM_EXPORT
#else
   #define LOFTY_TESTING_SYM LOFTY_SYM_IMPORT
#endif

#ifndef __DOXYGEN__
   #define _LOFTY_PUBNS       _pub::
   #define _LOFTY_PUBNS_BEGIN namespace _pub {
   #define _LOFTY_PUBNS_END   }
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY__PVT_LOFTY_HXX_NOPUB

#ifdef _LOFTY__PVT_LOFTY_HXX
   #undef _LOFTY_NOPUB

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY__PVT_LOFTY_HXX
