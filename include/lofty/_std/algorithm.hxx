/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2015, 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_STD_ALGORITHM_HXX
#define _LOFTY_STD_ALGORITHM_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {

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

}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_STD_ALGORITHM_HXX
