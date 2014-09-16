/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013, 2014
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

#ifndef _ABACLADE_STL_NEW_HXX
#define _ABACLADE_STL_NEW_HXX

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> before this file
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/stl/exception.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// std::nothrow_t and std::nothrow


namespace std {

//! Type of nothrow constant (C++11 § 18.6 “Dynamic memory management”).
struct nothrow_t {};

//! Constant to request no exceptions to be thrown (C++11 § 18.6 “Dynamic memory management”).
extern nothrow_t const nothrow;

} //namespace std



////////////////////////////////////////////////////////////////////////////////////////////////////
// std::bad_alloc


namespace std {

//! Thrown when a memory allocation error occurs (C++11 § 18.6.2.1 “Class bad_alloc”).
class ABACLADE_SYM bad_alloc :
   public exception {
public:

   //! See exception::exception().
   bad_alloc();

   //! Destructor.
   virtual ~bad_alloc();

   //! See exception::what().
   virtual char const * what() const override;
};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// :: globals – storage allocation and deallocation


// Single-object new and delete (C++11 § 18.6.1.1 “Single-object forms”).
void * operator new(std::size_t cb) ABC_STL_NOEXCEPT_FALSE((std::bad_alloc));
void * operator new(std::size_t cb, std::nothrow_t const &) ABC_STL_NOEXCEPT_TRUE();
void operator delete(void * p) ABC_STL_NOEXCEPT_TRUE();
void operator delete(void * p, std::nothrow_t const &) ABC_STL_NOEXCEPT_TRUE();

// Array new[] and delete[] (C++11 § 18.6.1.2 “Array forms”).
void * operator new[](std::size_t cb) ABC_STL_NOEXCEPT_FALSE((std::bad_alloc));
void * operator new[](std::size_t cb, std::nothrow_t const &) ABC_STL_NOEXCEPT_TRUE();
void operator delete[](void * p) ABC_STL_NOEXCEPT_TRUE();
void operator delete[](void * p, std::nothrow_t const &) ABC_STL_NOEXCEPT_TRUE();

// Placement new and delete (C++11 § 18.6.1.3 “Placement forms”).
inline void * operator new(std::size_t cb, void * pMem) ABC_STL_NOEXCEPT_TRUE() {
   ABC_UNUSED_ARG(cb);
   return pMem;
}
inline void * operator new[](std::size_t cb, void * pMem) ABC_STL_NOEXCEPT_TRUE() {
   ABC_UNUSED_ARG(cb);
   return pMem;
}
inline void operator delete(void * p, void * pMem) ABC_STL_NOEXCEPT_TRUE() {
   ABC_UNUSED_ARG(p);
   ABC_UNUSED_ARG(pMem);
}
inline void operator delete[](void * p, void * pMem) ABC_STL_NOEXCEPT_TRUE() {
   ABC_UNUSED_ARG(p);
   ABC_UNUSED_ARG(pMem);
}


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef _ABACLADE_STL_NEW_HXX

