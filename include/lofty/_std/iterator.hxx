/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_STD_ITERATOR_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_STD_ITERATOR_HXX
#endif

#ifndef _LOFTY_STD_ITERATOR_HXX_NOPUB
#define _LOFTY_STD_ITERATOR_HXX_NOPUB

#include <lofty/_pvt/lofty.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_STL_LOFTY

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

// TODO: STL iterator classes.

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#else //if LOFTY_HOST_STL_LOFTY
   #include <iterator>

   namespace lofty { namespace _std { namespace _pub {

   using ::std::bidirectional_iterator_tag;
   using ::std::forward_iterator_tag;
   using ::std::iterator;
   using ::std::random_access_iterator_tag;
   using ::std::reverse_iterator;

   }}}
#endif //if LOFTY_HOST_STL_LOFTY … else

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_STD_ITERATOR_HXX_NOPUB

#ifdef _LOFTY_STD_ITERATOR_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace _std {

   using _pub::bidirectional_iterator_tag;
   using _pub::forward_iterator_tag;
   using _pub::iterator;
   using _pub::random_access_iterator_tag;
   using _pub::reverse_iterator;

   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_STD_ITERATOR_HXX
