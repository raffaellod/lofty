/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_STD_TYPEINFO_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_STD_TYPEINFO_HXX
#endif

#ifndef _LOFTY_STD_TYPEINFO_HXX_NOPUB
#define _LOFTY_STD_TYPEINFO_HXX_NOPUB

#include <lofty/_pvt/lofty.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_STL_LOFTY

#include <lofty/noncopyable.hxx>
#include <lofty/_std/exception.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

//! Runtime type information (C++11 § 18.7.1 “Class type_info”).
class type_info : public noncopyable {
public:
   //! Destructor.
   virtual ~type_info();

   /*! Equality relational operator.

   @param ti
      Object to compare to *this.
   @return
      true if *this refers to the same type as ti, or false otherwise.
   */
   bool operator==(type_info const & ti) const;

   /*! Inequality relational operator.

   @param ti
      Object to compare to *this.
   @return
      true if *this refers to a different type than ti, or false otherwise.
   */
   bool operator!=(type_info const & ti) const {
      return !operator==(ti);
   }

   /*! Returns an hash code for *this.

   @return
      Hash code for the type.
   */
   std::size_t hash_code() const;

   /*! Returns the name of the type.

   @return
      Name of the type.
   */
   char const * name() const;
};

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

//! Thrown in case of invalid dynamic_cast<>() (C++11 § 18.7.2 “Class bad_cast”).
class LOFTY_SYM bad_cast : public exception {
public:
   //! Default constructor.
   bad_cast();

   //! Destructor.
   virtual ~bad_cast();

   //! See exception::what().
   virtual char const * what() const override;
};

_LOFTY_PUBNS_END
}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

//! Thrown in case of typeid(nullptr) (C++11 § 18.7.3 “Class bad_typeid”).
class LOFTY_SYM bad_typeid : public exception {
public:
   //! Default constructor.
   bad_typeid();

   //! Destructor.
   virtual ~bad_typeid();

   //! See exception::what().
   virtual char const * what() const override;
};

_LOFTY_PUBNS_END
}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#else //if LOFTY_HOST_STL_LOFTY
   #include <typeinfo>

   namespace lofty { namespace _std { namespace _pub {

   using ::std::bad_cast;
   using ::std::bad_typeid;
   using ::std::type_info;

   }}}
#endif //if LOFTY_HOST_STL_LOFTY … else

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_STD_TYPEINFO_HXX_NOPUB

#ifdef _LOFTY_STD_TYPEINFO_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace _std {

   using _pub::bad_cast;
   using _pub::bad_typeid;
   using _pub::type_info;

   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_STD_TYPEINFO_HXX
