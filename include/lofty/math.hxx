/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_MATH_HXX
#define _LOFTY_MATH_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Mathematical functions and algorithms.
namespace math {}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace math {

//! Thrown in case of generic arithmetic errors.
class LOFTY_SYM arithmetic_error : public generic_error {
public:
   /*! Constructor.

   @param err
      OS-defined error number associated to the exception.
   */
   explicit arithmetic_error(errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   arithmetic_error(arithmetic_error const & src);

   //! Destructor.
   virtual ~arithmetic_error() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   arithmetic_error & operator=(arithmetic_error const & src);
};

}} //namespace lofty::math

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace math {

//! Thrown when the divisor of a division or modulo operation was zero.
class LOFTY_SYM division_by_zero : public arithmetic_error {
public:
   /*! Constructor.

   @param err
      OS-defined error number associated to the exception.
   */
   explicit division_by_zero(errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   division_by_zero(division_by_zero const & src);

   //! Destructor.
   virtual ~division_by_zero() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   division_by_zero & operator=(division_by_zero const & src);
};

}} //namespace lofty::math

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace math {

//! Thrown upon failure of a floating point operation.
class LOFTY_SYM floating_point_error : public arithmetic_error {
public:
   /*! Constructor.

   @param err
      OS-defined error number associated to the exception.
   */
   explicit floating_point_error(errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   floating_point_error(floating_point_error const & src);

   //! Destructor.
   virtual ~floating_point_error() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   floating_point_error & operator=(floating_point_error const & src);
};

}} //namespace lofty::matg

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace math {

/*! Thrown when the result of an arithmetic operation is too large to be represented in the target data type.
Because of the lack of standardization of floating point exception handling in C, most floating point
operations are also not checked. */
class LOFTY_SYM overflow : public arithmetic_error {
public:
   /*! Constructor.

   @param err
      OS-defined error number associated to the exception.
   */
   explicit overflow(errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   overflow(overflow const & src);

   //! Destructor.
   virtual ~overflow() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   overflow & operator=(overflow const & src);
};

}} //namespace lofty::math

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty { namespace math { namespace _pvt {

/*! Helper for lofty::math::abs(). Needed because function templates can’t be partially specialized, but
structs/classes can. */
template <typename T, bool t_is_signed = _std::is_signed<T>::value>
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

}}} //namespace lofty::math::_pvt
//! @endcond

namespace lofty { namespace math {

/*! Returns the absolute value of the argument. It avoids annoying compiler warnings if the argument will
never be negative (i.e. T is unsigned).

@param t
   Value.
@return
   Absolute value of t.
*/
template <typename T>
inline /*constexpr*/ T abs(T t) {
   return _pvt::abs_helper<T>()(_std::move(t));
}

}} //namespace lofty::math

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_MATH_HXX
