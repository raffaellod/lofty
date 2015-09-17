/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013, 2014, 2015
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

#ifndef _ABACLADE_STD_TYPEINFO_HXX
#define _ABACLADE_STD_TYPEINFO_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std {

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

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std {

//! Thrown in case of invalid dynamic_cast<>() (C++11 § 18.7.2 “Class bad_cast”).
class ABACLADE_SYM bad_cast : public exception {
public:
   //! Default constructor.
   bad_cast();

   //! Destructor.
   virtual ~bad_cast();

   //! See exception::what().
   virtual char const * what() const override;
};

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std {

//! Thrown in case of typeid(nullptr) (C++11 § 18.7.3 “Class bad_typeid”).
class ABACLADE_SYM bad_typeid : public exception {
public:
   //! Default constructor.
   bad_typeid();

   //! Destructor.
   virtual ~bad_typeid();

   //! See exception::what().
   virtual char const * what() const override;
};

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_STD_TYPEINFO_HXX
