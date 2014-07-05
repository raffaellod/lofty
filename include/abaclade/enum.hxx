/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals


namespace abc {


/** DOC:3549 Enumeration classes

Abaclade features support for advanced enumeration classes. These are the features that set them
apart from C++11 “enum class” enumerations:

•  Strong typing: constants from one enumeration are not automatically converted to a different
   enumeration type;
•  Scoped constants: the enumeration members are members of the enumeration class, not of its
   containing namespace; different classes can therefore use constants of the same name (note that
   this will not confuse developers, since the constants will need to be qualified with the
   enumeration type name);
•  Conversion from/to string: instances of an Abaclade enumeration class can be serialized and
   de-serialized as strings with no additional code.

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
            { nullptr, 0, 0 } \
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
            { \
               ABC_SL(ABC_CPP_TOSTRING(name)), \
               ABC_COUNTOF(ABC_CPP_TOSTRING(name)) - 1 /*NUL*/, \
               name \
            },

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::enum_member


namespace abc {

/** Enumeration member (name/value pair).
*/
struct ABACLADE_SYM enum_member {

   /** Name. */
   char_t const * pszName;
   /** Length of *pszName, in characters. */
   unsigned short cchName;
   /** Value. */
   int iValue;


   /** Finds and returns the member associated to the specified enumerated value or name. If no
   match is found, an exception will be throw.

   pem
      Pointer to the first item in the enumeration members array; the last item in the array has a
      nullptr name string pointer.
   iValue
      Value of the constant to search for.
   sName
      Name of the constant to search for.
   return
      Pointer to the matching name/value pair.
   */
   static enum_member const * find_in_map(enum_member const * pem, int iValue);
   static enum_member const * find_in_map(enum_member const * pem, istr const & sName);
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
   iValue
      Integer value to be converted to enum_type. If i has a value not in enum_type, an exception
      will be thrown.
   sName
      String to be converted to enum_type. If this does not match exactly the name of one of the
      members of enum_type, an exception of type abc::domain_error will be thrown.
   */
   enum_impl() :
      m_e(T::__default) {
   }
   enum_impl(enum_type e) :
      m_e(e) {
   }
   enum_impl(enum_impl const & e) :
      m_e(e.m_e) {
   }
   // Conversion from integer.
   explicit enum_impl(int iValue) :
      m_e(static_cast<enum_type>(enum_member::find_in_map(T::_get_map(), iValue)->iValue)) {
   }
   // Conversion from string.
   explicit enum_impl(istr const & sName) :
      m_e(static_cast<enum_type>(enum_member::find_in_map(T::_get_map(), sName)->iValue)) {
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

   return
      Name of the current value.
   */
   istr name() const;


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
   inline bool operator op( \
      abc::enum_impl<T> const & e1, typename abc::enum_impl<T>::enum_type e2 \
   ) { \
      return e1.base() op e2; \
   } \
   template <class T> \
   inline bool operator op( \
      typename abc::enum_impl<T>::enum_type e1, abc::enum_impl<T> const & e2 \
   ) { \
      return e1 op e2.base(); \
   }
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
ABC_RELOP_IMPL(>)
ABC_RELOP_IMPL(>=)
ABC_RELOP_IMPL(<)
ABC_RELOP_IMPL(<=)
#undef ABC_RELOP_IMPL


////////////////////////////////////////////////////////////////////////////////////////////////////

