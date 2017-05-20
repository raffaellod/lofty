/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015, 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_STD_HXX
#define _LOFTY_STD_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! @internal lofty::_std contains STL implementation bits from Lofty’s STL incomplete implementation that we
may want to use when _LOFTY_USE_STLIMPL is not defined, as Lofty-only alternatives to lacking/buggy host STL
implementations. */
namespace _std {}

} //namespace lofty

#if LOFTY_HOST_STL_LOFTY || LOFTY_HOST_STL_MSVCRT == 1600
   #include <lofty/_std/mutex.hxx>
#else
   #include <mutex>

   namespace lofty { namespace _std {

   using ::std::lock_guard;
   using ::std::mutex;
   using ::std::unique_lock;

   }} //namespace lofty::_std
#endif

#if LOFTY_HOST_STL_LOFTY
   #include <lofty/_std/stdexcept.hxx>
#else
   #include <stdexcept>
#endif

#if LOFTY_HOST_STL_LOFTY
   #include <lofty/_std/functional.hxx>
#else
   #include <functional>

   namespace lofty { namespace _std {

   using ::std::function;

   }} //namespace lofty::_std
#endif

#if LOFTY_HOST_STL_LOFTY
   #include <lofty/_std/iterator.hxx>
#else
   #include <iterator>

   namespace lofty { namespace _std {

   using ::std::bidirectional_iterator_tag;
   using ::std::forward_iterator_tag;
   using ::std::iterator;
   using ::std::random_access_iterator_tag;
   using ::std::reverse_iterator;

   }} //namespace lofty::_std
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_STD_HXX
