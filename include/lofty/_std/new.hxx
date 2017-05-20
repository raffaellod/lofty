/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2015, 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_STD_NEW_HXX
#define _LOFTY_STD_NEW_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {

//! Type of nothrow constant (C++11 § 18.6 “Dynamic memory management”).
struct nothrow_t {};

//! Constant to request no exceptions to be thrown (C++11 § 18.6 “Dynamic memory management”).
extern LOFTY_SYM nothrow_t const nothrow;

}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {

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

}} //namespace lofty::_std

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

#endif //ifndef _LOFTY_STD_NEW_HXX
