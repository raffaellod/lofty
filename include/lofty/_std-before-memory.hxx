/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015, 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_STD_BEFORE_MEMORY_HXX
#define _LOFTY_STD_BEFORE_MEMORY_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {

/*! Type whose alignment requirement is at least as large as that of any scalar type (see C++11 § 18.2
“<cstddef>”). */
#if LOFTY_HOST_STL_LIBCXX || LOFTY_HOST_STL_LIBSTDCXX >= 40900
   typedef std::max_align_t max_align_t;
#elif LOFTY_HOST_STL_LIBSTDCXX
   typedef ::max_align_t max_align_t;
#else
   union max_align_t {
      //! @cond
      double d;
      long double ld;
      long long ll;
      //! @endcond
   };
#endif

}} //namespace lofty::_std

#if LOFTY_HOST_STL_LOFTY || !defined(LOFTY_CXX_VARIADIC_TEMPLATES)
   #include <lofty/_std/tuple.hxx>
#else
   #include <tuple>

   namespace lofty { namespace _std {

   using ::std::get;
   using ::std::ignore;
   using ::std::make_tuple;
   using ::std::tie;
   using ::std::tuple;
   using ::std::tuple_element;
   using ::std::tuple_size;

   }} //namespace lofty::_std
#endif

#if LOFTY_HOST_STL_LOFTY || LOFTY_HOST_STL_MSVCRT == 1600
   #include <lofty/_std/atomic.hxx>
#else
   #include <atomic>

   namespace lofty { namespace _std {

   using ::std::atomic;

   }} //namespace lofty::_std
#endif

#if LOFTY_HOST_STL_LOFTY
   #include <lofty/_std/exception.hxx>
#else
   #if LOFTY_HOST_STL_MSVCRT && LOFTY_HOST_CXX_MSC
      // Silence warnings from system header files.
      #pragma warning(push)
      // “expression before comma has no effect; expected expression with side-effect”
      #pragma warning(disable: 4548)
      // “'function': exception specification does not match previous declaration”
      #pragma warning(disable: 4986)
   #endif
   #include <exception>
   #if LOFTY_HOST_STL_MSVCRT && LOFTY_HOST_CXX_MSC
      #pragma warning(pop)
   #endif

   namespace lofty { namespace _std {

   using ::std::exception;
   using ::std::uncaught_exception;

   }} //namespace lofty::_std
#endif

#if LOFTY_HOST_STL_LOFTY
   #include <lofty/_std/new.hxx>
#else
   #include <new>

   namespace lofty { namespace _std {

   using ::std::bad_alloc;
   using ::std::nothrow;
   using ::std::nothrow_t;

   }} //namespace lofty::_std
#endif

#if LOFTY_HOST_STL_LOFTY
   #include <lofty/_std/typeinfo.hxx>
#else
   #include <typeinfo>

   namespace lofty { namespace _std {

   using ::std::type_info;

   }} //namespace lofty::_std
#endif

#if LOFTY_HOST_STL_LOFTY || LOFTY_HOST_STL_MSVCRT == 1600
   /* MSC16 BUG: has a half-assed std::shared_ptr that requires the type’s destructor to be defined at the
   point of declaration of the pointer. */
   #include <lofty/_std/memory.hxx>
#else
   #include <memory>

   namespace lofty { namespace _std {

   using ::std::default_delete;
   using ::std::dynamic_pointer_cast;
   using ::std::make_shared;
   using ::std::shared_ptr;
   using ::std::static_pointer_cast;
   using ::std::unique_ptr;
   using ::std::weak_ptr;

   #ifdef _LOFTY_STLIMPL_IS_COPY_CONSTRUCTIBLE
      // (Partially-) specialize is_copy_constructible for stock STL types.
      template <typename T, typename TDeleter>
      struct is_copy_constructible<unique_ptr<T, TDeleter>> : public false_type {};
   #endif

   }} //namespace lofty::_std
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_STD_BEFORE_MEMORY_HXX
