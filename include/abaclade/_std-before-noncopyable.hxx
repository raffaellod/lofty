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

#if ABC_HOST_STL_ABACLADE
// In case we’re reimplementing all of STL, just merge ::abc::_std into ::std.
namespace std {

using namespace ::abc::_std;

} //namespace std
#endif


#if ABC_HOST_STL_ABACLADE
   #include <abaclade/_std/type_traits.hxx>
#else
   #if ABC_HOST_STL_MSVCRT == 1800
      // See abc::noncopyable to understand what’s going on here.
      #define is_copy_constructible _ABC_MSVCRT_is_copy_constructible
   #endif
   #include <type_traits>
   #if ABC_HOST_STL_MSVCRT == 1800
      #undef is_copy_constructible
   #endif

   namespace abc { namespace _std {

   using ::std::enable_if;
   using ::std::false_type;
   using ::std::integral_constant;
   using ::std::is_base_of;
   using ::std::true_type;

   }} //namespace abc::_std

   #if (ABC_HOST_STL_LIBSTDCXX && ABC_HOST_STL_LIBSTDCXX < 40900) || ABC_HOST_STL_MSVCRT
      // The STL implementations above need to be supplemented with Abaclade’s implementation.
      #define _ABC_STD_TYPE_TRAITS_SELECTIVE
      #include <abaclade/_std/type_traits.hxx>
      #undef _ABC_STD_TYPE_TRAITS_SELECTIVE
   #endif

   namespace abc { namespace _std {

   using ::std::add_lvalue_reference;
   using ::std::add_pointer;
   using ::std::add_rvalue_reference;
   using ::std::conditional;
   using ::std::decay;
   using ::std::is_arithmetic;
   using ::std::is_array;
   #ifndef _ABC_STD_TYPE_TRAITS_IS_COPY_CONSTRUCTIBLE
   using ::std::is_copy_constructible;
   #endif
   using ::std::is_empty;
   using ::std::is_enum;
   using ::std::is_function;
   using ::std::is_pointer;
   using ::std::is_reference;
   using ::std::is_scalar;
   using ::std::is_signed;
   using ::std::is_trivial;
   #ifndef _ABC_STD_TYPE_TRAITS_IS_TRIVIALLY_COPY_CONSTRUCTIBLE
   using ::std::is_trivially_copy_constructible;
   #endif
   #ifndef _ABC_STD_TYPE_TRAITS_IS_TRIVIALLY_DESTRUCTIBLE
   using ::std::is_trivially_destructible;
   #endif
   using ::std::remove_const;
   using ::std::remove_cv;
   using ::std::remove_extent;
   using ::std::remove_reference;
   using ::std::is_void;

   }} //namespace abc::_std
#endif

#if ABC_HOST_STL_ABACLADE
   #include <abaclade/_std/utility.hxx>
#else
   #include <utility>

   #if ABC_HOST_STL_MSVCRT && ABC_HOST_STL_MSVCRT < 1800
      // The STL implementations above need to be supplemented with Abaclade’s implementation.
      #define _ABC_STD_UTILITY_SELECTIVE
      #include <abaclade/_std/utility.hxx>
      #undef _ABC_STD_UTILITY_SELECTIVE
   #endif

   namespace abc { namespace _std {

   #ifndef _ABC_STD_UTILITY_DECLVAL
      using ::std::declval;
   #endif
   using ::std::forward;
   using ::std::move;
   using ::std::swap;

   }} //namespace abc::_std
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_STD_BEFORE_NONCOPYABLE_HXX
