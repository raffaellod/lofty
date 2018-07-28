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
Classes and macros useful to support explicit operator bool conversions even in absence of compiler support.
*/

#ifndef _LOFTY_EXPLICIT_OPERATOR_BOOL_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_EXPLICIT_OPERATOR_BOOL_HXX
#endif

#ifndef _LOFTY_EXPLICIT_OPERATOR_BOOL_HXX_NOPUB
#define _LOFTY_EXPLICIT_OPERATOR_BOOL_HXX_NOPUB

#include <lofty/_pvt/lofty.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Declares an explicit conversion operator to bool. This offers support for explicit operator bool() even
with compilers that don’t support C++11 explicit conversion operators. */
#ifdef LOFTY_CXX_EXPLICIT_CONVERSION_OPERATORS
   #define LOFTY_EXPLICIT_OPERATOR_BOOL \
      explicit operator bool
#else
   #define LOFTY_EXPLICIT_OPERATOR_BOOL \
      bool _explicit_operator_bool
#endif

#ifndef LOFTY_CXX_EXPLICIT_CONVERSION_OPERATORS
namespace lofty { namespace _pvt {

//! Non-template helper providing definitions for support_explicit_operator_bool.
struct explob_helper {
   //! Non-bool boolean type.
   typedef void (explob_helper::* bool_type)() const;

   //! A pointer to this method is used as a Boolean “true” by support_explicit_operator_bool.
   void bool_true() const {
   }
};

}} //namespace lofty::_pvt
#endif //ifndef LOFTY_CXX_EXPLICIT_CONVERSION_OPERATORS

namespace lofty {
_LOFTY_PUBNS_BEGIN

/*! Provides subclasses with support for C++11 explicit operator bool() even with non-C++11-compliant
compilers. */
template <typename T>
struct support_explicit_operator_bool {
#ifndef LOFTY_CXX_EXPLICIT_CONVERSION_OPERATORS
   /*! Non-bool boolean conversion operator, safer than operator bool(), and almost as good as explicit
   operator bool().

   @return
      A valid pointer if T::_explicit_operator_bool() returns true, or nullptr otherwise.
   */
   operator _pvt::explob_helper::bool_type() const {
      if (static_cast<T const *>(this)->_explicit_operator_bool()) {
         return &_pvt::explob_helper::bool_true;
      } else {
         return nullptr;
      }
   }
#endif //ifndef LOFTY_CXX_EXPLICIT_CONVERSION_OPERATORS
};

#ifndef LOFTY_CXX_EXPLICIT_CONVERSION_OPERATORS
   // Disable relational operators for support_explicit_operator_bool.
   #ifdef LOFTY_CXX_FUNC_DELETE
      #define LOFTY_RELOP_IMPL(op) \
         template <typename T, typename U> \
         bool operator op( \
            support_explicit_operator_bool<T> const &, support_explicit_operator_bool<U> const & \
         ) = delete;
   #else //ifdef LOFTY_CXX_FUNC_DELETE
      #define LOFTY_RELOP_IMPL(op) \
         template <typename T, typename U> \
         bool operator op( \
            support_explicit_operator_bool<T> const &, support_explicit_operator_bool<U> const & \
         );
   #endif //ifdef LOFTY_CXX_FUNC_DELETE … else

   LOFTY_RELOP_IMPL(==)
   LOFTY_RELOP_IMPL(!=)
   #undef LOFTY_RELOP_IMPL
#endif //ifndef LOFTY_CXX_EXPLICIT_CONVERSION_OPERATORS

_LOFTY_PUBNS_END
} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_EXPLICIT_OPERATOR_BOOL_HXX_NOPUB

#ifdef _LOFTY_EXPLICIT_OPERATOR_BOOL_HXX
   #undef _LOFTY_NOPUB

   namespace lofty {
      using _pub::support_explicit_operator_bool;
   }

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_EXPLICIT_OPERATOR_BOOL_HXX
