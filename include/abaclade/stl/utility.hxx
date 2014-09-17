/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013, 2014
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

#ifndef _ABACLADE_STL_UTILITY_HXX
#define _ABACLADE_STL_UTILITY_HXX

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> before this file
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/stl/type_traits.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// Declarations and implementations


namespace std {

//! Defines a member named type as T.
template <typename T>
struct identity {
   typedef T type;
};


/*! Allows a method to accept an argument of any reference type, while declaring only an r-value
reference (C++11 § 20.2.3 “forward/move helpers”). N2835-compliant.

t
   Source object.
return
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
inline typename enable_if<is_lvalue_reference<T>::value, T>::type forward(
   typename identity<T>::type t
) {
   return t;
}
// Forward l-values as r-values.
template <typename T>
inline typename enable_if<!is_lvalue_reference<T>::value, T &&>::type forward(
   typename identity<T>::type & t
) {
   return static_cast<T &&>(t);
}
#ifdef ABC_CXX_FUNC_DELETE
// Prevent forwarding r-values as l-values.
template <typename T>
inline typename enable_if<is_lvalue_reference<T>::value, T>::type forward(
   typename remove_reference<T>::type && t
) = delete;
#endif


/*! Converts a value into an r-value reference, enabling move semantics on the argument (C++11 §
20.2.3 “forward/move helpers”).

t
   Source object.
return
   R-value reference to t.
*/
template <typename T>
inline typename remove_reference<T>::type && move(T && t) {
   return static_cast<typename remove_reference<T>::type &&>(t);
}


/*! Swaps the value of two objects (C++11 § 20.2.2 “swap”).

t1
   First object.
t2
   Second object.
*/
template <typename T>
inline void swap(T & t1, T & t2) {
   T tt(move(t1));
   t1 = move(t2);
   t2 = move(tt);
}
template <typename T, std::size_t t_ci>
inline void swap(T (& t1)[t_ci], T (& t2)[t_ci]) {
   T const * pt1End = &t1 + t_ci;
   for (T * pt1 = t1, * pt2 = t2; pt1 < pt1End; ++pt1, ++pt2) {
      swap(*pt1, *pt2);
   }
}

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef _ABACLADE_STL_UTILITY_HXX

