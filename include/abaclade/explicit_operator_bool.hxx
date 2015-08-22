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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Declares an explicit conversion operator to bool. This offers support for explicit operator
bool() even with compilers that don’t support C++11 explicit conversion operators. */
#ifdef ABC_CXX_EXPLICIT_CONVERSION_OPERATORS
   #define ABC_EXPLICIT_OPERATOR_BOOL \
      explicit operator bool
#else
   #define ABC_EXPLICIT_OPERATOR_BOOL \
      bool _explicit_operator_bool
#endif

#ifndef ABC_CXX_EXPLICIT_CONVERSION_OPERATORS
namespace abc { namespace detail {

//! Non-template helper providing definitions for support_explicit_operator_bool.
struct explob_helper {
   //! Non-bool boolean type.
   typedef void (explob_helper::* bool_type)() const;

   //! A pointer to this method is used as a Boolean “true” by support_explicit_operator_bool.
   void bool_true() const {
   }
};

}} //namespace abc::detail
#endif //ifndef ABC_CXX_EXPLICIT_CONVERSION_OPERATORS

namespace abc {

/*! Provides subclasses with support for C++11 explicit operator bool() even with non-C++11-
compliant compilers. */
template <typename T>
struct support_explicit_operator_bool {
#ifndef ABC_CXX_EXPLICIT_CONVERSION_OPERATORS
   /*! Non-bool boolean conversion operator, safer than operator bool(), and almost as good as
   explicit operator bool().

   @return
      A valid pointer if T::_explicit_operator_bool() returns true, or nullptr otherwise.
   */
   operator detail::explob_helper::bool_type() const {
      if (static_cast<T const *>(this)->_explicit_operator_bool()) {
         return &detail::explob_helper::bool_true;
      } else {
         return nullptr;
      }
   }
#endif //ifndef ABC_CXX_EXPLICIT_CONVERSION_OPERATORS
};

#ifndef ABC_CXX_EXPLICIT_CONVERSION_OPERATORS
   // Disable relational operators for support_explicit_operator_bool.
   #ifdef ABC_CXX_FUNC_DELETE
      #define ABC_RELOP_IMPL(op) \
         template <typename T, typename U> \
         bool operator op( \
            support_explicit_operator_bool<T> const &, support_explicit_operator_bool<U> const & \
         ) = delete;
   #else //ifdef ABC_CXX_FUNC_DELETE
      #define ABC_RELOP_IMPL(op) \
         template <typename T, typename U> \
         bool operator op( \
            support_explicit_operator_bool<T> const &, support_explicit_operator_bool<U> const & \
         );
   #endif //ifdef ABC_CXX_FUNC_DELETE … else

   ABC_RELOP_IMPL(==)
   ABC_RELOP_IMPL(!=)
   #undef ABC_RELOP_IMPL
#endif //ifndef ABC_CXX_EXPLICIT_CONVERSION_OPERATORS

} //namespace abc
