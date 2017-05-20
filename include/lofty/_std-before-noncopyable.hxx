/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015, 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_STD_BEFORE_NONCOPYABLE_HXX
#define _LOFTY_STD_BEFORE_NONCOPYABLE_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_STL_LOFTY
// In case we’re reimplementing all of STL, just merge ::lofty::_std into ::std.
namespace std {

using namespace ::lofty::_std;

} //namespace std
#endif


#if LOFTY_HOST_STL_LOFTY
   #include <lofty/_std/type_traits.hxx>
#else
   #if LOFTY_HOST_STL_MSVCRT == 1800
      // See lofty::noncopyable to understand what’s going on here.
      #define is_copy_constructible _LOFTY_MSVCRT_is_copy_constructible
   #endif
   #include <type_traits>
   #if LOFTY_HOST_STL_MSVCRT == 1800
      #undef is_copy_constructible
   #endif

   namespace lofty { namespace _std {

   using ::std::enable_if;
   using ::std::false_type;
   using ::std::integral_constant;
   using ::std::is_base_of;
   using ::std::is_trivial;
   using ::std::true_type;

   }} //namespace lofty::_std

   #if (LOFTY_HOST_STL_LIBSTDCXX && LOFTY_HOST_STL_LIBSTDCXX < 50000) || LOFTY_HOST_STL_MSVCRT
      // The STL implementations above need to be supplemented with Lofty’s implementation.
      #define _LOFTY_STD_TYPE_TRAITS_SELECTIVE
      #include <lofty/_std/type_traits.hxx>
      #undef _LOFTY_STD_TYPE_TRAITS_SELECTIVE
   #endif

   namespace lofty { namespace _std {

   using ::std::add_lvalue_reference;
   using ::std::add_pointer;
   using ::std::add_rvalue_reference;
   using ::std::conditional;
   using ::std::decay;
   using ::std::is_arithmetic;
   using ::std::is_array;
   #ifndef _LOFTY_STD_TYPE_TRAITS_IS_COPY_CONSTRUCTIBLE
   using ::std::is_copy_constructible;
   #endif
   using ::std::is_empty;
   using ::std::is_enum;
   using ::std::is_function;
   using ::std::is_pointer;
   using ::std::is_reference;
   using ::std::is_scalar;
   using ::std::is_signed;
   #ifndef _LOFTY_STD_TYPE_TRAITS_IS_TRIVIALLY_COPY_CONSTRUCTIBLE
   using ::std::is_trivially_copy_constructible;
   #endif
   #ifndef _LOFTY_STD_TYPE_TRAITS_IS_TRIVIALLY_DESTRUCTIBLE
   using ::std::is_trivially_destructible;
   #endif
   #ifndef _LOFTY_STD_TYPE_TRAITS_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE
   using ::std::is_trivially_move_constructible;
   #endif
   using ::std::is_void;
   using ::std::remove_const;
   using ::std::remove_cv;
   using ::std::remove_extent;
   using ::std::remove_reference;

   }} //namespace lofty::_std
#endif

#if LOFTY_HOST_STL_LOFTY
   #include <lofty/_std/utility.hxx>
#else
   #include <utility>

   #if LOFTY_HOST_STL_MSVCRT && LOFTY_HOST_STL_MSVCRT < 1800
      // The STL implementations above need to be supplemented with Lofty’s implementation.
      #define _LOFTY_STD_UTILITY_SELECTIVE
      #include <lofty/_std/utility.hxx>
      #undef _LOFTY_STD_UTILITY_SELECTIVE
   #endif

   namespace lofty { namespace _std {

   #ifndef _LOFTY_STD_UTILITY_DECLVAL
   using ::std::declval;
   #endif
   using ::std::forward;
   using ::std::move;
   using ::std::swap;

   }} //namespace lofty::_std
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_STD_BEFORE_NONCOPYABLE_HXX
