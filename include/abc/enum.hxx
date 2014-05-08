/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#ifndef _ABC_ENUM_HXX
#define _ABC_ENUM_HXX

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abc/char.hxx>
#include <abc/cppmacros.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc golbals


namespace abc {


/** DOC:3549 Enumeration classes

ABC features support for advanced enumeration classes. These are the features that set them apart
from C++11 “enum class” enumerations:

•  Strong typing: constants from one enumeration are not automatically converted to a different
   enumeration type;
•  Scoped constants: the enumeration members are members of the enumeration class, not of its
   containing namespace; different classes can therefore use constants of the same name (note that
   this will not confuse developers, since the constants will need to be qualified with the
   enumeration type name);
•  Conversion from/to string: instances of an ABC enumeration class can be serialized and
   deserialized as strings with no additional code.

The ABC_ENUM() macro declares an enumeration class containing the members provided as a list.

The name provided to ABC_ENUM() is associated to a C++ class, not the C++ enum; the latter is
available through the former, as in as my_enum::enum_type; there should little to no need to ever
directly refer to the C++ enum type.

This design is loosely based on <http://www.python.org/dev/peps/pep-0435/>.
*/

/** Defines an enumeration class derived from abc::enum_impl. See [DOC:3549 Enumeration classes] for
more information.

TODO: allow specifying a default value (instead of having __default = max + 1).

TODO: support for bit-field enumerations? Allow logical operation, smart conversion to/from string,
etc.

name
   Name of the enumeration type.
...
   Sequence of (name, value) pairs; these will be the members of the underlying C++ enum,
   name::enum_type.
*/
#define ABC_ENUM(name, ...) \
   struct ABC_CPP_CAT(_, name, _e) { \
   \
      /** Publicly-accessible enumerated constants. */ \
      enum enum_type { \
         ABC_CPP_TUPLELIST_WALK(_ABC_ENUM_MEMBER, __VA_ARGS__) \
         __default = 0 \
      }; \
   \
   \
      /** Returns a pointer to the name/value map to be used by abc::enum_impl. */ \
      static ::abc::enum_member const * _get_map() { \
         static ::abc::enum_member const sc_map[] = { \
            ABC_CPP_TUPLELIST_WALK(_ABC_ENUM_MEMBER_ARRAY_ITEM, __VA_ARGS__) \
            { nullptr, __default } \
         }; \
         return sc_map; \
      } \
   }; \
   typedef ::abc::enum_impl<ABC_CPP_CAT(_, name, _e)> name


/** Expands into an enum name/value assignment.

name
   Name of the enumeration constant.
value
   Value of the enumeration constant.
*/
#define _ABC_ENUM_MEMBER(name, value) \
         name = value,


/** Expands into an abc::enum_member initializer.

name
   Name of the enumeration constant.
value
   Value of the enumeration constant.
*/
#define _ABC_ENUM_MEMBER_ARRAY_ITEM(name, value) \
            { SL(ABC_CPP_TOSTRING(name)), value },

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::enum_member


namespace abc {

/** Enumeration member (name/value pair).
*/
struct ABCAPI enum_member {

   /** Name. */
   char_t const * pszName;
   /** Value. */
   int iValue;


   /** Finds and returns the member associated to the specified enumerated value or name. If no
   match is found, an exception will be throw.

   pem
      Pointer to the first item in the enumeration members array; the last item in the array has a
      nullptr name string pointer.
   i
      Value of the constant to search for.
   psz
      Name of the constant to search for.
   return
      Pointer to the matching name/value pair.
   */
   static enum_member const * find_in_map(enum_member const * pem, int i);
   static enum_member const * find_in_map(enum_member const * pem, char_t const * psz);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_enum_to_str_backend_impl


namespace abc {

namespace io {

// Forward declaration from abc/iostream.hxx.
class ostream;

} //namespace io

/** Implementation of the specializations of to_str_backend for enum_impl specializations.
*/
class ABCAPI _enum_to_str_backend_impl {
public:

   /** Constructor.

   crFormat
      Formatting options.
   */
   _enum_to_str_backend_impl(char_range const & crFormat);


protected:

   /** Writes an enumeration value, applying the formatting options.

   e
      Enumeration value to write.
   posOut
      Pointer to the output stream to write to.
   */
   void write_impl(int i, enum_member const * pem, io::ostream * posOut);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::enum_impl


namespace abc {

/** Implementation of enumeration classes.
*/
template <class T>
class enum_impl :
   public T {
public:

   typedef typename T::enum_type enum_type;


   /** Constructor.

   e
      Source value.
   i
      Integer value to be converted to enum_type. If i has a value not in enum_type, an exception
      will be thrown.
   psz
      String to be converted to enum_type. If *psz does not match exactly the name of one of the
      members of enum_type, an exception will be thrown.
   */
   enum_impl() :
      m_e(enum_type::__default) {
   }
   enum_impl(enum_type e) :
      m_e(e) {
   }
   enum_impl(enum_impl const & e) :
      m_e(e.m_e) {
   }
   // Conversion from integer.
   explicit enum_impl(int i) :
      m_e(static_cast<enum_type>(enum_member::find_in_map(T::_get_map(), i)->iValue)) {
   }
   // Conversion from string.
   // TODO: change to accept abc::char_range, so we get automatic conversion from abc::*str.
   explicit enum_impl(char_t const * psz) :
      m_e(static_cast<enum_type>(enum_member::find_in_map(T::_get_map(), psz)->iValue)) {
   }


   /** Assignment operator.

   e
      Source value.
   return
      *this.
   */
   enum_impl & operator=(enum_impl const & e) {
      m_e = e.m_e;
      return *this;
   }
   enum_impl & operator=(enum_type e) {
      m_e = e;
      return *this;
   }


   /** Returns the current base enumerated value.

   return
      Current value.
   */
   enum_type base() const {
      return m_e;
   }


   /** Returns the name of the current enumerated value.

   TODO: change to return abc::char_range, so we get automatic conversion to abc::istr const.

   return
      Name of the current value.
   */
   char_t const * name() const {
      return _member()->pszName;
   }


protected:

   /** Returns a pointer to the name/value pair for the current value.

   return
      Pointer to the name/value pair for the current value.
   */
   enum_member const * _member() const {
      return enum_member::find_in_map(T::_get_map(), m_e);
   }


private:

   /** Enumerated value. */
   enum_type m_e;
};

} //namespace abc


// Relational operators.
#define ABC_RELOP_IMPL(op) \
   template <class T> \
   inline bool operator op(abc::enum_impl<T> const & e1, abc::enum_impl<T> const & e2) { \
      return e1.base() op e2.base(); \
   } \
   template <class T> \
   inline bool operator op(abc::enum_impl<T> const & e1, typename T::enum_type e2) { \
      return e1.base() op e2; \
   } \
   template <class T> \
   inline bool operator op(typename T::enum_type e1, abc::enum_impl<T> const & e2) { \
      return e1 op e2.base(); \
   }
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
ABC_RELOP_IMPL(>)
ABC_RELOP_IMPL(>=)
ABC_RELOP_IMPL(<)
ABC_RELOP_IMPL(<=)
#undef ABC_RELOP_IMPL


namespace abc {

// Forward declaration from abc/to_str_backend.hxx.
template <typename T>
class to_str_backend;


// Specialization of to_str_backend.
template <class T>
class to_str_backend<enum_impl<T>> :
   public _enum_to_str_backend_impl {
public:

   /** Constructor. See abc::_enum_to_str_backend_impl::_enum_to_str_backend_impl().
   */
   to_str_backend(char_range const & crFormat = char_range()) :
      _enum_to_str_backend_impl(crFormat) {
   }


   /** See abc::_enum_to_str_backend_impl::write().
   */
   void write(enum_impl<T> e, io::ostream * posOut) {
      _enum_to_str_backend_impl::write_impl(e.base(), e._get_map(), posOut);
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef _ABC_ENUM_HXX

