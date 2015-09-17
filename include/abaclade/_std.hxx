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

#ifndef _ABACLADE_STD_HXX
#define _ABACLADE_STD_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*! @internal abc::_std contains STL implementation bits from Abaclade’s STL incomplete
implementation that we may want to use when _ABC_USE_STLIMPL is not defined, as Abaclade-only
alternatives to lacking/buggy host STL implementations. */
namespace _std {}

} //namespace abc

#if ABC_HOST_STL_ABACLADE || ABC_HOST_STL_MSVCRT == 1600
   #include <abaclade/_std/mutex.hxx>
#else
   #include <mutex>

   namespace abc { namespace _std {

   using ::std::lock_guard;
   using ::std::mutex;
   using ::std::unique_lock;

   }} //namespace abc::_std
#endif

#if ABC_HOST_STL_ABACLADE
   #include <abaclade/_std/stdexcept.hxx>
#else
   #include <stdexcept>
#endif

#if ABC_HOST_STL_ABACLADE
   #include <abaclade/_std/functional.hxx>
#else
   #include <functional>

   namespace abc { namespace _std {

   using ::std::function;

   }} //namespace abc::_std
#endif

#if ABC_HOST_STL_ABACLADE
   #include <abaclade/_std/iterator.hxx>
#else
   #include <iterator>

   namespace abc { namespace _std {

   using ::std::bidirectional_iterator_tag;
   using ::std::forward_iterator_tag;
   using ::std::iterator;
   using ::std::random_access_iterator_tag;
   using ::std::reverse_iterator;

   }} //namespace abc::_std
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_STD_HXX
