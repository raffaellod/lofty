/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013, 2014
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABC_STL_TYPEINFO_HXX
#define _ABC_STL_TYPEINFO_HXX

#ifndef _ABC_HXX
   #error Please #include <abc.hxx> before this file
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// std::type_info


namespace std {

/** Runtime type information (C++11 § 18.7.1 “Class type_info”).
*/
class type_info :
   public ::abc::noncopyable {
public:

   /** Destructor.
   */
   virtual ~type_info();


   /** Equality relational operator.

   TODO: comment signature.
   */
   bool operator==(type_info const & ti) const;


   /** Inequality relational operator.

   TODO: comment signature.
   */
   bool operator!=(type_info const & ti) const {
      return !operator==(ti);
   }


   /** Returns true if *this collates before ti.

   TODO: comment signature.
   */
   bool before(type_info const & ti) const;


   /** Returns an hash code for *this.

   TODO: comment signature.
   */
   size_t hash_code() const;


   /** Returns the name of the type.

   TODO: comment signature.
   */
   char const * name() const;
};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// std::bad_alloc


namespace std {

/** Thrown in case of invalid dynamic_cast<>() (C++11 § 18.7.2 “Class bad_cast”).
*/
class ABCAPI bad_cast :
   public exception {
public:

   /** See exception::exception().
   */
   bad_cast();


   /** Destructor.
   */
   virtual ~bad_cast();


   /** See exception::what().
   */
   virtual char const * what() const;
};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// std::bad_typeid


namespace std {

/** Thrown in case of typeid(nullptr) (C++11 § 18.7.3 “Class bad_typeid”).
*/
class ABCAPI bad_typeid :
   public exception {
public:

   /** See exception::exception().
   */
   bad_typeid();


   /** Destructor.
   */
   virtual ~bad_typeid();


   /** See exception::what().
   */
   virtual char const * what() const;
};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef _ABC_STL_TYPEINFO_HXX

