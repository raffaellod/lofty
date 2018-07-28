/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_STD_EXCEPTION_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_STD_EXCEPTION_HXX
#endif

#ifndef _LOFTY_STD_EXCEPTION_HXX_NOPUB
#define _LOFTY_STD_EXCEPTION_HXX_NOPUB

#include <lofty/_pvt/lofty.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_STL_LOFTY

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

//! Base class for standard exceptions (C++ § 18.8.1 “Class exception”).
class LOFTY_SYM exception {
public:
   //! Default constructor.
   exception();

   //! Destructor.
   virtual ~exception() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Returns information on the exception.

   @return
      Description of the exception.
   */
   virtual char const * what() const;
};

_LOFTY_PUBNS_END
}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#else //if LOFTY_HOST_STL_LOFTY
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

   namespace lofty { namespace _std { namespace _pub {

   using ::std::exception;
   using ::std::uncaught_exception;

   }}}
#endif //if LOFTY_HOST_STL_LOFTY … else

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_STD_EXCEPTION_HXX_NOPUB

#ifdef _LOFTY_STD_EXCEPTION_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace _std {

   using _pub::exception;
   using _pub::uncaught_exception;

   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_STD_EXCEPTION_HXX
