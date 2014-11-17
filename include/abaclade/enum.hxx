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

#ifndef _ABACLADE_HXX_INTERNAL
   #error Please #include <abaclade.hxx> instead of this file
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals

namespace abc {

/*! DOC:3549 Enumeration classes

Abaclade features support for advanced enumeration classes. These are the features that set them
apart from C++11 “enum class” enumerations:

•  Strong typing: constants from one enumeration are not implicitly converted to a different
   enumeration type;
•  Scoped constants: the enumeration members are members of the enumeration class, not of its
   containing scope; different classes can therefore use constants of the same name because their
   usage will need to be qualified with the enumeration type name;
•  Conversion from/to string: instances of an Abaclade enumeration class can be serialized and
   de-serialized as strings with no additional code.

The ABC_ENUM() macro declares an enumeration class containing the members provided as a list.

The name provided to ABC_ENUM() is associated to a C++ class, not the C++ enum; the latter is
available through the former, as in as my_enum::enum_type; there should little to no need to ever
directly refer to the C++ enum type.

This design is loosely based on <http://www.python.org/dev/peps/pep-0435/>.
*/

/*! Implementation of the various ABC_ENUM() flavors.

@param name
   Name of the enumeration type.
@param iBaseValue
   Value of __COUNTER__ prior to the expansion of the member values; used to dynamically generate
   values in sequence for the enum members.
@param cMembers
   Count of members of the enumeration.
@param members
   C++ enum members.
@param arrayitems
   Internal name/value array items.
@param ...
   Sequence of (name, value) pairs; these will be the members of the underlying C++ enum,
   name::enum_type.
*/
#define _ABC_ENUM_IMPL(name, iBaseValue, cMembers, members, arrayitems) \
   class ABC_CPP_CAT(_, name, _e) { \
   private: \
      static int const smc_iBase = iBaseValue + 1; \
   \
   public: \
      /*! Publicly-accessible enumerated constants. */ \
      enum enum_type { \
         members \
      }; \
   \
      /*! Returns a pointer to the name/value map to be used by abc::enum_impl. */ \
      static ::abc::detail::enum_member const * _get_map() { \
         static ::abc::detail::enum_member const sc_map[] = { \
            arrayitems \
            { nullptr, 0, 0 } \
         }; \
         return sc_map; \
      } \
   \
   protected: \
      /*! Number of members specified for the enum. */ \
      static std::size_t const smc_cMembers = cMembers; \
   }; \
   typedef ::abc::enum_impl<ABC_CPP_CAT(_, name, _e)> name

/*! Expands into an enum name/value assignment.

@param name
   Name of the enumeration constant. The associated value is incremental in the ABC_ENUM, just like
   in C++ enums.
*/
#define _ABC_ENUM_MEMBER(name) \
         name = __COUNTER__ - smc_iBase,

/*! Expands into an enum name/value assignment.

@param name
   Name of the enumeration constant.
@param value
   Value of the enumeration constant.
*/
#define _ABC_ENUM_MEMBER_PAIR(name, value) \
         name = value,

/*! Expands into an abc::detail::enum_member initializer.

@param name
   Name of the enumeration constant.
*/
#define _ABC_ENUM_MEMBER_ARRAY_ITEM(name) \
            { \
               ABC_SL(ABC_CPP_TOSTRING(name)), \
               static_cast<unsigned short>(ABC_COUNTOF(ABC_CPP_TOSTRING(name)) - 1 /*NUL*/), \
               name \
            },

/*! Expands into _ABC_ENUM_MEMBER_ARRAY_ITEM().

@param name
   Name of the enumeration constant.
@param value
   Value of the enumeration constant. Not used.
*/
#define _ABC_ENUM_MEMBER_PAIR_ARRAY_ITEM(name, value) \
   _ABC_ENUM_MEMBER_ARRAY_ITEM(name)

/*! Defines an enumeration class as a specialization of abc::enum_impl. See [DOC:3549 Enumeration
classes] for more information.

TODO: support for bit-field enumerations? Allow logical operation, smart conversion to/from string,
etc.

@param name
   Name of the enumeration type.
@param ...
   Sequence of (name, value) pairs; these will be the members of the underlying C++ enum,
   name::enum_type.
*/
#define ABC_ENUM(name, ...) \
   _ABC_ENUM_IMPL( \
      name, \
      __COUNTER__, \
      ABC_CPP_LIST_COUNT(__VA_ARGS__), \
      ABC_CPP_TUPLELIST_WALK(_ABC_ENUM_MEMBER_PAIR, __VA_ARGS__), \
      ABC_CPP_TUPLELIST_WALK(_ABC_ENUM_MEMBER_PAIR_ARRAY_ITEM, __VA_ARGS__) \
   )

/*! Defines an enumeration class as a specialization of abc::enum_impl. See [DOC:3549 Enumeration
classes] for more information. Similar to ABC_ENUM(), except the members are listed individually and
their values cannot be explicitly specified; for example:

   ABC_ENUM_AUTO_VALUES(myenum, item1, item2, item3);

@param name
   Name of the enumeration type.
@param ...
   Sequence of member names; these will be the members of the underlying C++ enum, name::enum_type.
   Their values will start from 0 and increase by 1 for each member.
*/
#define ABC_ENUM_AUTO_VALUES(name, ...) \
   _ABC_ENUM_IMPL( \
      name, \
      __COUNTER__, \
      ABC_CPP_LIST_COUNT(__VA_ARGS__), \
      ABC_CPP_LIST_WALK(_ABC_ENUM_MEMBER, __VA_ARGS__), \
      ABC_CPP_LIST_WALK(_ABC_ENUM_MEMBER_ARRAY_ITEM, __VA_ARGS__) \
   )

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::enum_member

namespace abc {
namespace detail {

//! Enumeration member (name/value pair).
struct ABACLADE_SYM enum_member {
   //! Name.
   char_t const * pszName;
   //! Size of *pszName, in characters.
   unsigned short cchName;
   //! Value.
   int iValue;

   /*! Finds and returns the member associated to the specified enumerated value or name. If no
   match is found, an exception will be throw.

   @param pem
      Pointer to the first item in the enumeration members array; the last item in the array has a
      nullptr name string pointer.
   @param iValue
      Value of the constant to search for.
   @param sName
      Name of the constant to search for.
   @return
      Pointer to the matching name/value pair.
   */
   static enum_member const * find_in_map(enum_member const * pem, int iValue);
   static enum_member const * find_in_map(enum_member const * pem, istr const & sName);
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::enum_impl

namespace abc {

//! Implementation of enumeration classes.
template <class T>
class enum_impl : public T {
public:
   //! Underlying C++ eunm type.
   typedef typename T::enum_type enum_type;

   /*! Constructor.

   @param e
      Source value.
   @param iValue
      Integer value to be converted to enum_type. If i has a value not in enum_type, an exception
      will be thrown.
   @param sName
      String to be converted to enum_type. If this does not match exactly the name of one of the
      members of enum_type, an exception of type abc::domain_error will be thrown.
   */
   enum_impl() {
   }
   enum_impl(enum_type e) :
      m_e(e) {
   }
   enum_impl(enum_impl const & e) :
      m_e(e.m_e) {
   }
   // Conversion from integer.
   explicit enum_impl(int iValue) :
      m_e(static_cast<enum_type>(detail::enum_member::find_in_map(T::_get_map(), iValue)->iValue)) {
   }
   // Conversion from string.
   explicit enum_impl(istr const & sName) :
      m_e(static_cast<enum_type>(detail::enum_member::find_in_map(T::_get_map(), sName)->iValue)) {
   }

   /*! Assignment operator.

   @param e
      Source value.
   @return
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

   /*! Returns the current base enumerated value.

   @return
      Current value.
   */
   enum_type base() const {
      return m_e;
   }

   /*! Returns the name of the current enumerated value.

   @return
      Name of the current value.
   */
   istr name() const;

   /*! Returns the count of members in the enumeration.

   @return
      Member count.
   */
   static std::size_t size() {
      return T::smc_cMembers;
   }

protected:
   /*! Returns a pointer to the name/value pair for the current value.

   @return
      Pointer to the name/value pair for the current value.
   */
   detail::enum_member const * _member() const {
      return detail::enum_member::find_in_map(T::_get_map(), m_e);
   }

public:
   /*! Count of the members of the enumeration. Same as the value returned by size(), but this can
   be used in constant contexts, such as the size of an array.
   */
   static std::size_t const size_const = T::smc_cMembers;

private:
   //! Enumerated value.
   enum_type m_e;
};

// Relational operators.
#define ABC_RELOP_IMPL(op) \
   template <class T> \
   inline bool operator op(enum_impl<T> const & e1, enum_impl<T> const & e2) { \
      return e1.base() op e2.base(); \
   } \
   template <class T> \
   inline bool operator op(enum_impl<T> const & e1, typename enum_impl<T>::enum_type e2) { \
      return e1.base() op e2; \
   } \
   template <class T> \
   inline bool operator op(typename enum_impl<T>::enum_type e1, enum_impl<T> const & e2) { \
      return e1 op e2.base(); \
   }
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
ABC_RELOP_IMPL(>)
ABC_RELOP_IMPL(>=)
ABC_RELOP_IMPL(<)
ABC_RELOP_IMPL(<=)
#undef ABC_RELOP_IMPL

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
