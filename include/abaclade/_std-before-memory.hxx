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

#ifndef _ABACLADE_STD_BEFORE_MEMORY_HXX
#define _ABACLADE_STD_BEFORE_MEMORY_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ABC_STLIMPL
   #include <abaclade/_std/utility.hxx>
#else
   #include <utility>

   namespace abc { namespace _std {

   using ::std::forward;
   using ::std::move;

   }} //namespace abc::_std
#endif

#if defined(ABC_STLIMPL) || !defined(ABC_CXX_VARIADIC_TEMPLATES)
   #include <abaclade/_std/tuple.hxx>
#else
   #include <tuple>

   namespace abc { namespace _std {

   using ::std::ignore;
   using ::std::tie;
   using ::std::tuple;
   using ::std::tuple_element;
   using ::std::tuple_get;
   using ::std::tuple_size;

   }} //namespace abc::_std
#endif

#if defined(ABC_STLIMPL) || ABC_HOST_CXX_MSC == 1600
   #include <abaclade/_std/atomic.hxx>
#else
   #include <atomic>

   namespace abc { namespace _std {

   using ::std::atomic;

   }} //namespace abc::_std
#endif

#ifdef ABC_STLIMPL
   #include <abaclade/_std/exception.hxx>
#else
   #if ABC_HOST_CXX_MSC
      // Silence warnings from system header files.
      #pragma warning(push)
      // “expression before comma has no effect; expected expression with side-effect”
      #pragma warning(disable: 4548)
      // “'function': exception specification does not match previous declaration”
      #pragma warning(disable: 4986)
   #endif //if ABC_HOST_CXX_MSC
   #include <exception>
   #if ABC_HOST_CXX_MSC
      #pragma warning(pop)
   #endif

   namespace abc { namespace _std {

   using ::std::exception;

   }} //namespace abc::_std
#endif

#ifdef ABC_STLIMPL
   #include <abaclade/_std/new.hxx>
#else
   #include <new>

   namespace abc { namespace _std {

   using ::std::nothrow;
   using ::std::nothrow_t;

   }} //namespace abc::_std
#endif

#ifdef ABC_STLIMPL
   #include <abaclade/_std/typeinfo.hxx>
#else
   #include <typeinfo>

   namespace abc { namespace _std {

   using ::std::type_info;

   }} //namespace abc::_std
#endif

#if defined(ABC_STLIMPL) || ABC_HOST_CXX_MSC == 1600
   // MSC16 has a half-assed std::shared_ptr that requires the type’s destructor to be defined.
   #include <abaclade/_std/memory.hxx>
#else
   #include <memory>

   #ifdef ABC_STLIMPL_IS_COPY_CONSTRUCTIBLE
      namespace std {

      // (Partially-) specialize is_copy_constructible for MSC-provided STL types.
      template <typename T, typename TDeleter>
      struct is_copy_constructible<unique_ptr<T, TDeleter>> : public false_type {};

      } //namespace std
   #endif

   namespace abc { namespace _std {

   using ::std::dynamic_pointer_cast;
   using ::std::make_shared;
   using ::std::shared_ptr;
   using ::std::static_pointer_cast;
   using ::std::unique_ptr;
   using ::std::weak_ptr;

   }} //namespace abc::_std
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_STD_BEFORE_MEMORY_HXX
