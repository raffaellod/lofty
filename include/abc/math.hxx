/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#ifndef _ABC_MATH_HXX
#define _ABC_MATH_HXX

#ifndef _ABC_CORE_HXX
   #error Please #include <abc/core.hxx> instead of this file
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::math::abs()

namespace abc {
namespace math {

/** Helper for abc::math::abs(). Needed because function templates can’t be partially specialized,
but structs/classes can.
*/
template <typename T, bool t_bIsSigned = std::is_signed<T>::value>
struct _abs_helper;

// Partial specialization for signed types.
template <typename T>
struct _abs_helper<T, true> {

   /*constexpr*/ T operator()(T t) const {
      return std::move(t >= 0 ? t : -t);
   }
};

// Partial specialization for unsigned types.
template <typename T>
struct _abs_helper<T, false> {

   /*constexpr*/ T operator()(T t) const {
      return std::move(t);
   }
};


/** Returns the absolute value of the argument. It avoids annoying compiler warnings if the argument
will never be negative (i.e. T is unsigned).

t
   Value.
return
   Absolute value of t.
*/
template <typename T>
inline /*constexpr*/ T abs(T t) {
   return _abs_helper<T>()(std::move(t));
}


} //namespace math
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef _ABC_MATH_HXX

