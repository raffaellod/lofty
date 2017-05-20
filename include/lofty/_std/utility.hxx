/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2015, 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_STD_UTILITY_HXX
#define _LOFTY_STD_UTILITY_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {

#if !defined(_LOFTY_STD_UTILITY_SELECTIVE) || (LOFTY_HOST_STL_MSVCRT && LOFTY_HOST_STL_MSVCRT < 1800)
   // MSC16 lacks a definition of std::declval().
   template <typename T>
   typename add_rvalue_reference<T>::type declval();

   #define _LOFTY_STD_UTILITY_DECLVAL
#endif //if !defined(_LOFTY_STD_UTILITY_SELECTIVE) || (LOFTY_HOST_STL_MSVCRT && LOFTY_HOST_STL_MSVCRT < 1800)

#ifndef _LOFTY_STD_UTILITY_SELECTIVE

//! Defines a member named type as T.
template <typename T>
struct identity {
   typedef T type;
};

/*! Allows a method to accept an argument of any reference type, while declaring only an r-value reference
(C++11 § 20.2.3 “forward/move helpers”). N2835-compliant.

@param t
   Source object.
@return
   R-value reference to t.
*/
// Forward r-values as r-values.
template <typename T>
inline typename enable_if<!is_lvalue_reference<T>::value, T &&>::type forward(
   typename identity<T>::type && t
) {
   return static_cast<T &&>(t);
}
// Forward l-values as l-values.
template <typename T>
inline typename enable_if<is_lvalue_reference<T>::value, T>::type forward(typename identity<T>::type t) {
   return t;
}
// Forward l-values as r-values.
template <typename T>
inline typename enable_if<!is_lvalue_reference<T>::value, T &&>::type forward(
   typename identity<T>::type & t
) {
   return static_cast<T &&>(t);
}
#ifdef LOFTY_CXX_FUNC_DELETE
// Prevent forwarding r-values as l-values.
template <typename T>
inline typename enable_if<is_lvalue_reference<T>::value, T>::type forward(
   typename remove_reference<T>::type && t
) = delete;
#endif

/*! Converts a value into an r-value reference, enabling move semantics on the argument (C++11 § 20.2.3
“forward/move helpers”).

@param t
   Source object.
@return
   R-value reference to t.
*/
template <typename T>
inline typename remove_reference<T>::type && move(T && t) {
   return static_cast<typename remove_reference<T>::type &&>(t);
}

/*! Swaps the value of two objects (C++11 § 20.2.2 “swap”).

@param t1
   First object.
@param t2
   Second object.
*/
template <typename T>
inline void swap(T & t1, T & t2) {
   T tt(move(t1));
   t1 = move(t2);
   t2 = move(tt);
}
template <typename T, std::size_t size>
inline void swap(T (& t1)[size], T (& t2)[size]) {
   T const * t1_end = &t1 + size;
   for (T * t1_ptr = t1, * t2_ptr = t2; t1_ptr < t1_end; ++t1_ptr, ++t2_ptr) {
      swap(*t1_ptr, *t2_ptr);
   }
}

#endif //ifndef _LOFTY_STD_UTILITY_SELECTIVE

}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_STD_UTILITY_HXX
