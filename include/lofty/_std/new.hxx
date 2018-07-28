/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_STD_NEW_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_STD_NEW_HXX
#endif

#ifndef _LOFTY_STD_NEW_HXX_NOPUB
#define _LOFTY_STD_NEW_HXX_NOPUB

#include <lofty/_pvt/lofty.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_STL_LOFTY

#include <lofty/_std/exception.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

//! Type of nothrow constant (C++11 § 18.6 “Dynamic memory management”).
struct nothrow_t {};

//! Constant to request no exceptions to be thrown (C++11 § 18.6 “Dynamic memory management”).
extern LOFTY_SYM nothrow_t const nothrow;

_LOFTY_PUBNS_END
}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

//! Thrown when a memory allocation error occurs (C++11 § 18.6.2.1 “Class bad_alloc”).
class LOFTY_SYM bad_alloc : public exception {
public:
   //! See exception::exception().
   bad_alloc();

   //! Destructor.
   virtual ~bad_alloc();

   //! See exception::what().
   virtual char const * what() const override;
};

_LOFTY_PUBNS_END
}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Single-object new and delete (C++11 § 18.6.1.1 “Single-object forms”).
void * operator new(std::size_t alloc_size) LOFTY_STL_NOEXCEPT_FALSE((lofty::_std::bad_alloc));
void * operator new(std::size_t alloc_size, lofty::_std::nothrow_t const &) LOFTY_STL_NOEXCEPT_TRUE();
void operator delete(void * p) LOFTY_STL_NOEXCEPT_TRUE();
void operator delete(void * p, lofty::_std::nothrow_t const &) LOFTY_STL_NOEXCEPT_TRUE();

// Array new[] and delete[] (C++11 § 18.6.1.2 “Array forms”).
void * operator new[](std::size_t alloc_size) LOFTY_STL_NOEXCEPT_FALSE((lofty::_std::bad_alloc));
void * operator new[](std::size_t alloc_size, lofty::_std::nothrow_t const &) LOFTY_STL_NOEXCEPT_TRUE();
void operator delete[](void * p) LOFTY_STL_NOEXCEPT_TRUE();
void operator delete[](void * p, lofty::_std::nothrow_t const &) LOFTY_STL_NOEXCEPT_TRUE();

// Placement new and delete (C++11 § 18.6.1.3 “Placement forms”).
inline void * operator new(std::size_t, void * p) LOFTY_STL_NOEXCEPT_TRUE() {
   return p;
}
inline void * operator new[](std::size_t, void * p) LOFTY_STL_NOEXCEPT_TRUE() {
   return p;
}
inline void operator delete(void *, void *) LOFTY_STL_NOEXCEPT_TRUE() {
}
inline void operator delete[](void *, void *) LOFTY_STL_NOEXCEPT_TRUE() {
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#else //if LOFTY_HOST_STL_LOFTY
   #include <new>

   namespace lofty { namespace _std { namespace _pub {

   using ::std::bad_alloc;
   using ::std::nothrow;
   using ::std::nothrow_t;

   }}}
#endif //if LOFTY_HOST_STL_LOFTY … else

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_STD_NEW_HXX_NOPUB

#ifdef _LOFTY_STD_NEW_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace _std {

   using _pub::bad_alloc;
   using _pub::nothrow;
   using _pub::nothrow_t;

   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_STD_NEW_HXX
