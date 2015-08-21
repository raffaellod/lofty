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

#ifndef _ABACLADE_STD_BEFORE_NONCOPYABLE_HXX
#define _ABACLADE_STD_BEFORE_NONCOPYABLE_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ABC_STLIMPL
// In case we’re reimplementing all of STL, just merge ::abc::_std into ::std.
namespace std {
using namespace ::abc::_std;
} //namespace std
#endif


#ifdef ABC_STLIMPL
   #include <abaclade/_std/type_traits.hxx>
#else
   #if ABC_HOST_CXX_MSC == 1800
      // See abc::noncopyable to understand what’s going on here.
      #define is_copy_constructible _ABC_MSC18_is_copy_constructible
   #endif
   #include <type_traits>
   #if ABC_HOST_CXX_MSC == 1800
      #undef is_copy_constructible
   #endif

   namespace abc { namespace _std {

   using ::std::add_lvalue_reference;
   using ::std::add_reference;
   using ::std::conditional;
   using ::std::enable_if;
   using ::std::false_type;
   using ::std::is_base_of;
#ifdef ABC_CXX_STL_CXX11_TYPE_TRAITS
   using ::std::is_copy_constructible;
#endif
   using ::std::is_reference;
   using ::std::is_scalar;
   using ::std::is_signed;
   using ::std::is_trivial;
   using ::std::is_trivially_destructible;
   using ::std::remove_const;
   using ::std::remove_cv;
   using ::std::remove_reference;

   }} //namespace abc::_std
#endif

#ifdef ABC_STLIMPL
   #include <abaclade/_std/utility.hxx>
#else
   #include <utility>

   namespace abc { namespace _std {

   using ::std::declval;
   using ::std::forward;
   using ::std::move;
   using ::std::swap;

   }} //namespace abc::_std
#endif

// Provide a definition of std::is_copy_constructible for STL implementations lacking it.
#if ( \
   (ABC_HOST_CXX_GCC && ABC_HOST_CXX_GCC < 40700) || (ABC_HOST_CXX_MSC && ABC_HOST_CXX_MSC < 1900) \
) && !defined(ABC_STLIMPL)

namespace abc { namespace _std {

#if ABC_HOST_CXX_GCC
   // GCC lacks a definition of std::add_reference.
   template <typename T>
   struct add_reference {
      typedef T & type;
   };
   template <typename T>
   struct add_reference<T &> {
      typedef T & type;
   };
#elif ABC_HOST_CXX_MSC && ABC_HOST_CXX_MSC < 1800
   // MSC16 lacks a definition of std::declval.
   template <typename T>
   typename add_rvalue_reference<T>::type declval();
#endif

template <typename T, typename = void>
struct is_copy_constructible {
private:
   static int test(T &);
   static char test(...);

   typedef typename add_reference<T>::type TRef;

public:
   static bool const value = (sizeof(test(declval<TRef>())) == sizeof(int))
#if ABC_HOST_CXX_MSC == 1800
      /* MSC18 does provide an implementation which, while severely flawed (see abc::noncopyable),
      may be stricter than this, so && its return value. */
      && std::_ABC_MSC18_is_copy_constructible<T>::value
#endif
   ;
};

#define ABC_STLIMPL_IS_COPY_CONSTRUCTIBLE

}} //namespace abc::_std

#endif /*if ((ABC_HOST_CXX_GCC && ABC_HOST_CXX_GCC < 40700) ||
             (ABC_HOST_CXX_MSC && ABC_HOST_CXX_MSC < 1900) && !defined(ABC_STLIMPL) */

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_STD_BEFORE_NONCOPYABLE_HXX
