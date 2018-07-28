/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_STD_ALGORITHM_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_STD_ALGORITHM_HXX
#endif

#ifndef _LOFTY_STD_ALGORITHM_HXX_NOPUB
#define _LOFTY_STD_ALGORITHM_HXX_NOPUB

#include <lofty/_pvt/lofty.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_STL_LOFTY || LOFTY_HOST_STL_MSVCRT == 1600

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

/*! Searches for a value in a range of iterators (C++11 § 25.2.5 “Find”).

@param begin
   Iterator to the first element in the range to search.
@param end
   Iterator to the end of the range to search.
@param needle
   Element to search for.
@return
   Iterator which dereferences to a value equal to needle, or the same iterator as the end argument.
*/
template <typename TItr, typename T>
inline typename TItr find(TItr begin, TItr end, T const & needle) {
   for (auto itr(begin); itr != end; ++itr) {
      if (*itr == needle) {
         return itr;
      }
   }
   return end;
}

/*! Returns the greatest of two objects (C++11 § 25.4.7 “Minimum and maximum”).

@param t1
   First value.
@param t2
   Second value.
@return
   Greatest of t1 and t2.
*/
template <typename T>
inline T const & max(T const & t1, T const & t2) {
   return t1 > t2 ? t1 : t2;
}
template <typename T, class TCompare>
inline T const & max(T const & t1, T const & t2, TCompare compare_fn) {
   return compare_fn(t1, t2) > 0 ? t1 : t2;
}

/*! Returns the least of two objects (C++11 § 25.4.7 “Minimum and maximum”).

@param t1
   First value.
@param t2
   Second value.
@return
   Least of t1 and t2.
*/
template <typename T>
inline T const & min(T const & t1, T const & t2) {
   return t1 < t2 ? t1 : t2;
}
template <typename T, class TCompare>
inline T const & min(T const & t1, T const & t2, TCompare compare_fn) {
   return compare_fn(t1, t2) < 0 ? t1 : t2;
}

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#else //if LOFTY_HOST_STL_LOFTY || LOFTY_HOST_STL_MSVCRT == 1600
   #include <algorithm>

   namespace lofty { namespace _std { namespace _pub {

   using ::std::find;
   using ::std::max;
   using ::std::min;

   }}}
#endif //if LOFTY_HOST_STL_LOFTY || LOFTY_HOST_STL_MSVCRT == 1600 … else

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_STD_ALGORITHM_HXX_NOPUB

#ifdef _LOFTY_STD_ALGORITHM_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace _std {

   using _pub::find;
   using _pub::max;
   using _pub::min;

   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_STD_ALGORITHM_HXX
