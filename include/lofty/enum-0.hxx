/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

/*! @file
Facilities to create smart enumerations, as described in @ref enumeration-classes */

#ifndef _LOFTY_ENUM_0_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_ENUM_0_HXX
#endif

#ifndef _LOFTY_ENUM_0_HXX_NOPUB
#define _LOFTY_ENUM_0_HXX_NOPUB

#include <lofty/cpp.hxx>
#include <lofty/text-0.hxx>

/*! @page enumeration-classes Enumeration classes
Support for advanced enumeration classes. These are the features that set them apart from pre-C++11 “enum”
types:

•  Strong typing: constants from one enumeration are not implicitly converted to a different enumeration type;
•  Scoped constants: the enumeration members are members of the enumeration class, not of its containing
   scope, like in a C++11 enum class; different classes can therefore use constants of the same name because
   their usage will need to be qualified with the enumeration type name.

In addition, these features are not found even in the newer C++11 “enum class” types:

•  Conversion from/to string: instances of a Lofty enumeration class can be serialized and de-serialized as
   strings with no additional code.

The LOFTY_ENUM() macro declares an enumeration class containing the members provided as a list;
LOFTY_ENUM_AUTO_VALUES() behaves like a C++ enum lacking explicit enumerated values.

The name provided to LOFTY_ENUM() is associated to a C++ class, not the C++ enum; the latter is available
through the former, as in as my_enum::enum_type; there should little to no need to ever directly refer to the
C++ enum type.

This design is loosely based on <http://www.python.org/dev/peps/pep-0435/>. */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond

/*! Implementation of the various LOFTY_ENUM*() flavors.

@param other_members
   Additional members to be injected as-is at the beginning of the class.
@param name
   Name of the enumeration type.
@param members_size_
   Count of members of the enumeration.
@param members
   C++ enum members.
@param arrayitems
   Internal name/value array items.
*/
#define _LOFTY_ENUM_IMPL(other_members, name, members_size_, members, arrayitems) \
   class LOFTY_CPP_CAT(_, name, _e) { \
   other_members \
   \
   public: \
      /*! Publicly-accessible enumerated constants. */ \
      enum enum_type { \
         members \
      }; \
   \
      /*! Returns a pointer to the name/value map to be used by lofty::enum_impl. */ \
      static ::lofty::_pvt::enum_member const * _get_map() { \
         static ::lofty::_pvt::enum_member const sc_map[] = { \
            arrayitems \
            { nullptr, 0, 0 } \
         }; \
         return sc_map; \
      } \
   \
   protected: \
      /*! Number of members specified for the enum. */ \
      static std::size_t const members_size = members_size_; \
   }; \
   typedef ::lofty::_pub::enum_impl<LOFTY_CPP_CAT(_, name, _e)> name

/*! Expands into an enum name/value assignment.

@param name
   Name of the enumeration constant. The associated value is incremental in the LOFTY_ENUM, just like in C++
   enums.
*/
#define _LOFTY_ENUM_MEMBER(name) \
         name = __COUNTER__ - value_base,

/*! Expands into an enum name/value assignment.

@param name
   Name of the enumeration constant.
@param value
   Value of the enumeration constant.
*/
#define _LOFTY_ENUM_MEMBER_PAIR(name, value) \
         name = value,

/*! Expands into a lofty::_pvt::enum_member initializer.

@param name
   Name of the enumeration constant.
*/
#define _LOFTY_ENUM_MEMBER_ARRAY_ITEM(name) \
            { \
               LOFTY_SL(LOFTY_CPP_TOSTRING(name)), \
               static_cast<unsigned short>(LOFTY_COUNTOF(LOFTY_CPP_TOSTRING(name)) - 1 /*NUL*/), \
               name \
            },

/*! Expands into _LOFTY_ENUM_MEMBER_ARRAY_ITEM().

@param name
   Name of the enumeration constant.
@param value
   Value of the enumeration constant. Not used.
*/
#define _LOFTY_ENUM_MEMBER_PAIR_ARRAY_ITEM(name, value) \
   _LOFTY_ENUM_MEMBER_ARRAY_ITEM(name)

//! @endcond

/*! Defines an enumeration class as a specialization of lofty::enum_impl. See @ref enumeration-classes for
more information.

TODO: support for bit-field enumerations? Allow logical operation, smart conversion to/from string, etc.

@param name
   Name of the enumeration type.
@param ...
   Sequence of (name, value) pairs; these will be the members of the underlying C++ enum, name::enum_type.
*/
#define LOFTY_ENUM(name, ...) \
   _LOFTY_ENUM_IMPL( \
      public: \
      , \
      name, \
      LOFTY_CPP_LIST_COUNT(__VA_ARGS__), \
      LOFTY_CPP_TUPLELIST_WALK(_LOFTY_ENUM_MEMBER_PAIR, __VA_ARGS__), \
      LOFTY_CPP_TUPLELIST_WALK(_LOFTY_ENUM_MEMBER_PAIR_ARRAY_ITEM, __VA_ARGS__) \
   )

/*! Defines an enumeration class as a specialization of lofty::enum_impl. See @ref enumeration-classes for
more information. Similar to LOFTY_ENUM(), except the members are listed individually and their values cannot
be explicitly specified; for example:

   LOFTY_ENUM_AUTO_VALUES(myenum, item1, item2, item3);

@param name
   Name of the enumeration type.
@param ...
   Sequence of member names; these will be the members of the underlying C++ enum, name::enum_type. Their
   values will start from 0 and increase by 1 for each member, just like in a C++ enum.
*/
#define LOFTY_ENUM_AUTO_VALUES(name, ...) \
   _LOFTY_ENUM_IMPL( \
      private: \
         static int const value_base = __COUNTER__ + 1; \
      , \
      name, \
      LOFTY_CPP_LIST_COUNT(__VA_ARGS__), \
      LOFTY_CPP_LIST_WALK(_LOFTY_ENUM_MEMBER, __VA_ARGS__), \
      LOFTY_CPP_LIST_WALK(_LOFTY_ENUM_MEMBER_ARRAY_ITEM, __VA_ARGS__) \
   )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

//! Enumeration member (name/value pair).
struct LOFTY_SYM enum_member {
   //! Name.
   text::_LOFTY_PUBNS char_t const * name;
   //! Size of *name, in characters.
   unsigned short name_size;
   //! Value.
   int value;

   /*! Finds and returns the member associated to the specified enumerated value. If no match is found, an
   exception will be throw.

   @param members
      Pointer to the first item in the enumeration members array; the last item in the array has a nullptr
      name string pointer.
   @param value
      Value of the constant to search for.
   @return
      Pointer to the matching name/value pair.
   */
   static enum_member const * find_in_map(enum_member const * members, int value);

   /*! Finds and returns the member associated to the specified value name. If no match is found, an exception
   will be throw.

   @param members
      Pointer to the first item in the enumeration members array; the last item in the array has a nullptr
      name string pointer.
   @param name
      Name of the constant to search for.
   @return
      Pointer to the matching name/value pair.
   */
   static enum_member const * find_in_map(enum_member const * members, text::_LOFTY_PUBNS str const & name);
};

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {
_LOFTY_PUBNS_BEGIN

/*! Implementation of enumeration classes. Not to be used directly; to implement an enumeration, use
LOFTY_ENUM() or LOFTY_ENUM_AUTO_VALUES() instead. See @ref enumeration-classes. */
template <class T>
class enum_impl : public T {
public:
   //! Underlying C++ enum type.
   typedef typename T::enum_type enum_type;

public:
   //! Default constructor.
   enum_impl() {
   }

   /*! Constructor.

   @param e_
      Initial value.
   */
   enum_impl(enum_type e_) :
      e(e_) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   enum_impl(enum_impl const & src) :
      e(src.e) {
   }

   /*! Constructor that converts from an integer.

   @param value
      Integer value to be converted to enum_type. If this doesn’t match the value of any enum_type member, an
      exception of type lofty::domain_error will be thrown.
   */
   explicit enum_impl(int value) :
      e(static_cast<enum_type>(_pvt::enum_member::find_in_map(T::_get_map(), value)->value)) {
   }

   /*! Constructor that converts from a string.

   @param name
      String to be converted to enum_type. If this doesn’t match exactly the name of any enum_type member, an
      exception of type lofty::domain_error will be thrown.
   */
   explicit enum_impl(text::_LOFTY_PUBNS str const & name_) :
      e(static_cast<enum_type>(_pvt::enum_member::find_in_map(T::_get_map(), name_)->value)) {
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   enum_impl & operator=(enum_impl const & src) {
      e = src.e;
      return *this;
   }

   /*! Assignment operator.

   @param e_
      Source value.
   @return
      *this.
   */
   enum_impl & operator=(enum_type e_) {
      e = e_;
      return *this;
   }

   /*! Returns the current base enumerated value.

   @return
      Current value.
   */
   enum_type base() const {
      return e;
   }

   /*! Returns the name of the current enumerated value.

   @return
      Name of the current value.
   */
   text::_LOFTY_PUBNS str name() const;

   /*! Returns the count of members in the enumeration.

   @return
      Member count.
   */
   static std::size_t size() {
      return T::members_size;
   }

protected:
   /*! Returns a pointer to the name/value pair for the current value.

   @return
      Pointer to the name/value pair for the current value.
   */
   _pvt::enum_member const * _member() const {
      return _pvt::enum_member::find_in_map(T::_get_map(), e);
   }

public:
   /*! Count of the members of the enumeration. Same as the value returned by size(), but this can be used in
   constant contexts, such as the size of an array. */
   static std::size_t const size_const = T::members_size;

private:
   //! Enumerated value.
   enum_type e;
};

// Relational operators.
#define LOFTY_RELOP_IMPL(op) \
   template <class T> \
   inline bool operator op(enum_impl<T> const & left, enum_impl<T> const & right) { \
      return left.base() op right.base(); \
   } \
   template <class T> \
   inline bool operator op(enum_impl<T> const & left, typename enum_impl<T>::enum_type right) { \
      return left.base() op right; \
   } \
   template <class T> \
   inline bool operator op(typename enum_impl<T>::enum_type left, enum_impl<T> const & right) { \
      return left op right.base(); \
   }
LOFTY_RELOP_IMPL(==)
LOFTY_RELOP_IMPL(!=)
LOFTY_RELOP_IMPL(>)
LOFTY_RELOP_IMPL(>=)
LOFTY_RELOP_IMPL(<)
LOFTY_RELOP_IMPL(<=)
#undef LOFTY_RELOP_IMPL

_LOFTY_PUBNS_END
} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_ENUM_0_HXX_NOPUB

#ifdef _LOFTY_ENUM_0_HXX
   #undef _LOFTY_NOPUB

   namespace lofty {

   using _pub::enum_impl;

   }

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_ENUM_0_HXX
