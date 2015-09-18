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

#ifndef _ABACLADE_MATH_HXX
#define _ABACLADE_MATH_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! Mathematical functions and algorithms.
namespace math {}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace math {

//! Thrown in case of generic arithmetic errors.
class ABACLADE_SYM arithmetic_error : public generic_error {
public:
   //! Default constructor.
   arithmetic_error();

   /*! Copy constructor.

   @param x
      Source object.
   */
   arithmetic_error(arithmetic_error const & x);

   //! Destructor.
   virtual ~arithmetic_error();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   arithmetic_error & operator=(arithmetic_error const & x);

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

}} //namespace abc::math

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace math {

//! Thrown when the divisor of a division or modulo operation was zero.
class ABACLADE_SYM division_by_zero : public arithmetic_error {
public:
   //! Default constructor.
   division_by_zero();

   /*! Copy constructor.

   @param x
      Source object.
   */
   division_by_zero(division_by_zero const & x);

   //! Destructor.
   virtual ~division_by_zero();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   division_by_zero & operator=(division_by_zero const & x);

   //! See abc::arithmetic_error::init().
   void init(errint_t err = 0);
};

}} //namespace abc::math

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace math {

//! Thrown upon failure of a floating point operation.
class ABACLADE_SYM floating_point_error : public arithmetic_error {
public:
   //! Default constructor.
   floating_point_error();

   /*! Copy constructor.

   @param x
      Source object.
   */
   floating_point_error(floating_point_error const & x);

   //! Destructor.
   virtual ~floating_point_error();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   floating_point_error & operator=(floating_point_error const & x);

   //! See abc::arithmetic_error::init().
   void init(errint_t err = 0);
};

}} //namespace abc::matg

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace math {

/*! Thrown when the result of an arithmetic operation is too large to be represented in the target
data type. Because of the lack of standardization of floating point exception handling in C, most
floating point operations are also not checked. */
class ABACLADE_SYM overflow : public arithmetic_error {
public:
   //! Default constructor.
   overflow();

   /*! Copy constructor.

   @param x
      Source object.
   */
   overflow(overflow const & x);

   //! Destructor.
   virtual ~overflow();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   overflow & operator=(overflow const & x);

   //! See abc::arithmetic_error::init().
   void init(errint_t err = 0);
};

}} //namespace abc::math

////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace abc { namespace math { namespace detail {

/*! Helper for abc::math::abs(). Needed because function templates can’t be partially specialized,
but structs/classes can. */
template <typename T, bool t_bIsSigned = _std::is_signed<T>::value>
struct abs_helper;

// Partial specialization for signed types.
template <typename T>
struct abs_helper<T, true> {
   /*constexpr*/ T operator()(T t) const {
      return _std::move(t >= 0 ? t : -t);
   }
};

// Partial specialization for unsigned types.
template <typename T>
struct abs_helper<T, false> {
   /*constexpr*/ T operator()(T t) const {
      return _std::move(t);
   }
};

}}} //namespace abc::math::detail
//! @endcond

namespace abc { namespace math {

/*! Returns the absolute value of the argument. It avoids annoying compiler warnings if the argument
will never be negative (i.e. T is unsigned).

@param t
   Value.
@return
   Absolute value of t.
*/
template <typename T>
inline /*constexpr*/ T abs(T t) {
   return detail::abs_helper<T>()(_std::move(t));
}

}} //namespace abc::math

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_MATH_HXX
