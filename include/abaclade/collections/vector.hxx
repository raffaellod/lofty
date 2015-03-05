﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
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
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::detail::raw_vector

namespace abc {
namespace collections {
namespace detail {

/*! Thin templated wrapper for raw_*_vextr_impl to make the interface of those two classes
consistent, so vector doesn’t need specializations. */
template <typename T, bool t_bCopyConstructible, bool t_bTrivial = std::is_trivial<T>::value>
class raw_vector;

// Partial specialization for non-copyable, non-trivial types.
template <typename T>
class raw_vector<T, false, false> : public raw_complex_vextr_impl, public noncopyable {
public:
   //! Destructor.
   ~raw_vector() {
      type_void_adapter type;
      type.set_destr_fn<T>();
      destruct_items(type);
   }

   /*! Moves the contents of the two sources to *this.

   @param p1Begin
      Pointer to the start of the first source array.
   @param p1End
      Pointer to the end of the first source array.
   @param p2Begin
      Pointer to the start of the second source array.
   @param p2End
      Pointer to the end of the second source array.
   */
   void assign_concat_move(T * p1Begin, T * p1End, T * p2Begin, T * p2End) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      raw_complex_vextr_impl::assign_concat(type, p1Begin, p1End, p2Begin, p2End, 1 + 2);
   }

   //! See raw_complex_vextr_impl::assign_move().
   void assign_move(raw_complex_vextr_impl && rcvi) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      raw_complex_vextr_impl::assign_move(type, std::move(rcvi));
   }

   //! See raw_complex_vextr_impl::assign_move_dynamic_or_move_items().
   void assign_move_dynamic_or_move_items(raw_complex_vextr_impl && rcvi) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      raw_complex_vextr_impl::assign_move_dynamic_or_move_items(type, std::move(rcvi));
   }

   /*! Inserts elements at a specific position in the vector by moving them.

   @param ptOffset
      Pointer to where the elements should be inserted.
   @param ptInsert
      Pointer to the first element to insert.
   @param ciInsert
      Count of elements in the array pointed to by ptInsert.
   */
   void insert_move(T const * ptOffset, T * ptInsert, std::size_t ciInsert) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      raw_complex_vextr_impl::insert(
         type,
         static_cast<std::size_t>(
            reinterpret_cast<std::int8_t const *>(ptOffset) -
               raw_vextr_impl_base::begin<std::int8_t>()
         ),
         ptInsert, sizeof(T) * ciInsert, true
      );
   }

   /*! Removes a slice from the vector.

   @param ptRemoveBegin
      Pointer to the first element to remove.
   @param ptRemoveEnd
      Pointer to beyond the last element to remove.
   */
   void remove(T const * ptRemoveBegin, T const * ptRemoveEnd) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      raw_complex_vextr_impl::remove(
         type,
         static_cast<std::size_t>(
            reinterpret_cast<std::int8_t const *>(ptRemoveBegin) -
               raw_vextr_impl_base::begin<std::int8_t>()
         ),
         reinterpret_cast<std::size_t>(ptRemoveEnd) - reinterpret_cast<std::size_t>(ptRemoveBegin)
      );
   }

   /*! See raw_complex_vextr_impl::set_capacity().

   @param ciMin
      Minimum count of elements requested.
   @param bPreserve
      If true, the previous contents of the vector will be preserved even if the reallocation causes
      the vector to switch to a different item array.
   */
   void set_capacity(std::size_t ciMin, bool bPreserve) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      raw_complex_vextr_impl::set_capacity(type, sizeof(T) * ciMin, bPreserve);
   }

   /*! See raw_complex_vextr_impl::set_capacity().

   TODO: destruct in raw_complex_vextr_impl::set_size() any elements being taken out, and default-
   construct the newly-created elements here.

   @param ci
      New element count, in bytes.
   */
   void set_size(std::size_t ci) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      raw_complex_vextr_impl::set_size(type, sizeof(T) * ci);
   }

protected:
   //! See raw_complex_vextr_impl::raw_complex_vextr_impl().
   raw_vector(std::size_t cbEmbeddedCapacity) :
      raw_complex_vextr_impl(cbEmbeddedCapacity) {
   }
   raw_vector(T const * ptConstSrc, std::size_t ciSrc) :
      raw_complex_vextr_impl(ptConstSrc, ciSrc) {
   }

private:
   // Hide these raw_complex_vextr_impl methods to trigger errors as a debugging aid.

   void assign_copy(type_void_adapter const & type, T const * ptBegin, T const * ptEnd);
};

// Partial specialization for copyable, non-trivial types.
template <typename T>
class raw_vector<T, true, false> : public raw_vector<T, false, false> {
public:
   //! See raw_complex_vextr_impl::assign_copy().
   void assign_copy(T const * ptBegin, T const * ptEnd) {
      type_void_adapter type;
      type.set_copy_fn<T>();
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      raw_complex_vextr_impl::assign_copy(type, ptBegin, ptEnd);
   }

   //! See raw_complex_vextr_impl::assign_concat().
   void assign_concat(
      T const * p1Begin, T const * p1End, T const * p2Begin, T const * p2End, std::uint8_t iMove
   ) {
      type_void_adapter type;
      type.set_copy_fn<T>();
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      raw_complex_vextr_impl::assign_concat(type, p1Begin, p1End, p2Begin, p2End, iMove);
   }

   /*! Inserts elements at a specific position in the vector by copying them.

   @param ptOffset
      Pointer to where the elements should be inserted.
   @param ptInsert
      Pointer to the first element to insert.
   @param ciInsert
      Count of elements in the array pointed to by ptInsert.
   */
   void insert_copy(T const * ptOffset, T const * ptInsert, std::size_t ciInsert) {
      type_void_adapter type;
      type.set_copy_fn<T>();
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      raw_complex_vextr_impl::insert(
         type,
         static_cast<std::size_t>(
            reinterpret_cast<std::int8_t const *>(ptOffset) -
               raw_vextr_impl_base::begin<std::int8_t>()
         ),
         ptInsert, sizeof(T) * ciInsert, false
      );
   }

protected:
   //! See raw_vector<T, false, false>::raw_vector<T, false, false>().
   raw_vector(std::size_t cbEmbeddedCapacity) :
      raw_vector<T, false, false>(cbEmbeddedCapacity) {
   }
   raw_vector(T const * ptConstSrc, std::size_t ciSrc) :
      raw_vector<T, false, false>(ptConstSrc, ciSrc) {
   }
};

/* Partial specialization for trivial (and copyable) types. Methods here ignore the bMove argument
for the individual elements, because move semantics don’t apply (trivial values are always copied).
*/
template <typename T>
class raw_vector<T, true, true> : public raw_trivial_vextr_impl {
public:
   //! See raw_trivial_vextr_impl::assign_copy().
   void assign_copy(T const * ptBegin, T const * ptEnd) {
      raw_trivial_vextr_impl::assign_copy(ptBegin, ptEnd);
   }

   //! See raw_trivial_vextr_impl::assign_concat().
   void assign_concat(
      T const * p1Begin, T const * p1End, T const * p2Begin, T const * p2End, std::uint8_t iMove
   ) {
      ABC_UNUSED_ARG(iMove);
      raw_trivial_vextr_impl::assign_concat(p1Begin, p1End, p2Begin, p2End);
   }

   /*! Moves the contents of the two sources to *this.

   @param p1Begin
      Pointer to the start of the first source array.
   @param p1End
      Pointer to the end of the first source array.
   @param p2Begin
      Pointer to the start of the second source array.
   @param p2End
      Pointer to the end of the second source array.
   */
   void assign_concat_move(T * p1Begin, T * p1End, T * p2Begin, T * p2End) {
      raw_trivial_vextr_impl::assign_concat(p1Begin, p1End, p2Begin, p2End);
   }

   //! See raw_trivial_vextr_impl::assign_move().
   void assign_move(raw_trivial_vextr_impl && rtvi) {
      raw_trivial_vextr_impl::assign_move(std::move(rtvi));
   }

   //! See raw_trivial_vextr_impl::assign_move_dynamic_or_move_items().
   void assign_move_dynamic_or_move_items(raw_trivial_vextr_impl && rtvi) {
      raw_trivial_vextr_impl::assign_move_dynamic_or_move_items(std::move(rtvi));
   }

   /*! Inserts elements at a specific position in the vector.

   @param ptOffset
      Pointer to where the elements should be inserted.
   @param ptInsert
      Pointer to the first element to insert.
   @param ciInsert
      Count of elements in the array pointed to by ptInsert.
   */
   void insert_copy(T const * ptOffset, T const * ptInsert, std::size_t ciInsert) {
      raw_trivial_vextr_impl::insert_remove(
         static_cast<std::size_t>(
            reinterpret_cast<std::int8_t const *>(ptOffset) -
               raw_vextr_impl_base::begin<std::int8_t>()
         ),
         ptInsert, sizeof(T) * ciInsert, 0
      );
   }

   /*! Inserts one or more elements. Semantically this is supposed to move them, but for trivial
   types that’s the same as copying them.

   @param ptOffset
      Pointer to where the elements should be inserted.
   @param ptInsert
      Pointer to the first element to insert.
   @param ciInsert
      Count of elements in the array pointed to by ptInsert.
   */
   void insert_move(T const * ptOffset, T * ptInsert, std::size_t ciInsert) {
      raw_trivial_vextr_impl::insert_remove(
         static_cast<std::size_t>(
            reinterpret_cast<std::int8_t const *>(ptOffset) -
               raw_vextr_impl_base::begin<std::int8_t>()
         ),
         ptInsert, sizeof(T) * ciInsert, 0
      );
   }

   /*! Removes elements from the vector.

   @param ptRemoveBegin
      Pointer to the first element to remove.
   @param ptRemoveEnd
      Pointer to beyond the last element to remove.
   */
   void remove(T const * ptRemoveBegin, T const * ptRemoveEnd) {
      raw_trivial_vextr_impl::insert_remove(
         static_cast<std::size_t>(
            reinterpret_cast<std::int8_t const *>(ptRemoveBegin) -
               raw_vextr_impl_base::begin<std::int8_t>()
         ),
         nullptr, 0,
         reinterpret_cast<std::size_t>(ptRemoveEnd) - reinterpret_cast<std::size_t>(ptRemoveBegin)
      );
   }

   /*! See raw_trivial_vextr_impl::set_capacity().

   @param ciMin
      Minimum count of elements requested.
   @param bPreserve
      If true, the previous contents of the vector will be preserved even if the reallocation causes
      the vector to switch to a different item array.
   */
   void set_capacity(std::size_t ciMin, bool bPreserve) {
      raw_trivial_vextr_impl::set_capacity(sizeof(T) * ciMin, bPreserve);
   }

   /*! See raw_complex_vextr_impl::set_capacity().

   TODO: maybe default-construct the newly-created elements here for consistency with the non-
   trivial specialization?

   @param ci
      New element count, in bytes.
   */
   void set_size(std::size_t ci) {
      raw_trivial_vextr_impl::set_size(sizeof(T) * ci);
   }

protected:
   //! See raw_trivial_vextr_impl::raw_trivial_vextr_impl().
   raw_vector(std::size_t cbEmbeddedCapacity) :
      raw_trivial_vextr_impl(cbEmbeddedCapacity) {
   }
   raw_vector(T const * ptConstSrc, std::size_t ciSrc) :
      raw_trivial_vextr_impl(ptConstSrc, ciSrc) {
   }
};

} //namespace detail
} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::detail::vector_base

namespace abc {
namespace collections {

// Forward declarations.
template <typename T, bool t_bCopyConstructible = std::is_copy_constructible<T>::value>
class mvector;
template <typename T, bool t_bCopyConstructible = std::is_copy_constructible<T>::value>
class dmvector;

namespace detail {

/*! Base class for vectors.

See [DOC:4019 abc::text::*str and abc::collections::*vector design] for implementation details for
this and all the *vector classes. */
template <typename T, bool t_bCopyConstructible = std::is_copy_constructible<T>::value>
class vector_base;

/* Partial specialization for non-copyable types. Note that it doesn’t force t_bCopyConstructible to
false on raw_vector, so that vector_base<T, true> can inherit from this and still get all the
copyable-only members of raw_vector<T, true>. */
template <typename T>
class vector_base<T, false> :
   protected raw_vector<T, std::is_copy_constructible<T>::value>,
   public support_explicit_operator_bool<vector_base<T, std::is_copy_constructible<T>::value>> {
private:
   //! true if T is copy constructible, or false otherwise.
   static bool const smc_bCopyConstructible = std::is_copy_constructible<T>::value;

public:
   typedef T value_type;
   typedef T * pointer;
   typedef T const * const_pointer;
   typedef T & reference;
   typedef T const & const_reference;
   typedef std::size_t size_type;
   typedef std::ptrdiff_t difference_type;
   typedef pointer_iterator<vector_base<T, std::is_copy_constructible<T>::value>, T> iterator;
   typedef pointer_iterator<
      vector_base<T, std::is_copy_constructible<T>::value
   >, T const> const_iterator;
   typedef std::reverse_iterator<iterator> reverse_iterator;
   typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

public:
   /*! Element access operator.

   @param i
      Element index. See abc::collections::vector_base::translate_index() for allowed index values.
   @return
      Element at index i.
   */
   T const & operator[](std::ptrdiff_t i) const {
      return *this->translate_index(i);
   }

   /*! Returns true if the vector size is greater than 0.

   @return
      true if the vector is not empty, or false otherwise.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      // Use std::int8_t to avoid multiplying by sizeof(T) when all we need is a greater-than check.
      return raw_vextr_impl_base::end<std::int8_t>() > raw_vextr_impl_base::begin<std::int8_t>();
   }

   /*! Equality relational operator.

   @param v
      Object to compare to *this.
   @return
      true if *this has the same count and value of elements as v, or false otherwise.
   */
   bool operator==(vector_base const & v) const {
      if (size() != v.size()) {
         return false;
      }
      for (auto it1(cbegin()), it2(v.cbegin()), it1End(cend()); it1 != it1End; ++it1, ++it2) {
         if (*it1 != *it2) {
            return false;
         }
      }
      return true;
   }
   template <std::size_t t_ci>
   bool operator==(T const (& at)[t_ci]) const {
      if (size() != t_ci) {
         return false;
      }
      T const * pt(at);
      for (auto it(cbegin()), itEnd(cend()); it != itEnd; ++it, ++pt) {
         if (*it != *pt) {
            return false;
         }
      }
      return true;
   }

   /*! Inequality relational operator.

   @param v
      Object to compare to *this.
   @return
      true if *this has different count or value of elements as v, or false otherwise.
   */
   bool operator!=(vector_base const & v) const {
      return !operator==(v);
   }

   /*! Returns a forward iterator set to the first element.

   @return
      Forward iterator to the first element.
   */
   const_iterator begin() const {
      return cbegin();
   }

   /*! Returns the maximum number of elements the array can currently hold.

   @return
      Current size of the item array storage, in elements.
   */
   std::size_t capacity() const {
      return raw_vextr_impl_base::capacity<T>();
   }

   /*! Returns a const forward iterator set to the first element.

   @return
      Forward iterator to the first element.
   */
   const_iterator cbegin() const {
      return const_iterator(raw_vextr_impl_base::begin<T>());
   }

   /*! Returns a const forward iterator set beyond the last element.

   @return
      Forward iterator to beyond the last element.
   */
   const_iterator cend() const {
      return const_iterator(raw_vextr_impl_base::end<T>());
   }

   /*! Returns a const reverse iterator set to the last element.

   @return
      Reverse iterator to the last element.
   */
   const_reverse_iterator crbegin() const {
      return const_reverse_iterator(cend());
   }

   /*! Returns a const reverse iterator set to before the first element.

   @return
      Reverse iterator to before the first element.
   */
   const_reverse_iterator crend() const {
      return const_reverse_iterator(cbegin());
   }

   /*! Returns a forward iterator set beyond the last element.

   @return
      Forward iterator to the first element.
   */
   const_iterator end() const {
      return cend();
   }

   /*! Returns the count of elements in the array.

   @return
      Count of elements.
   */
   std::size_t size() const {
      return raw_vextr_impl_base::size<T>();
   }

   /*! Returns a reverse iterator set to the last element.

   @return
      Reverse iterator to the last element.
   */
   const_reverse_iterator rbegin() const {
      return crbegin();
   }

   /*! Returns a reverse iterator set to before the first element.

   @return
      Reverse iterator to before the first element.
   */
   const_reverse_iterator rend() const {
      return crend();
   }

protected:
   /*! Constructor. The overload with ciEmbedded constructs the object as empty, setting m_p to
   nullptr or an empty string; the overload with pt constructs the object assigning an item array.

   @param ciEmbedded
      Count of slots in the embedded item array, or 0 if no embedded item array is present.
   @param pt
      Pointer to an array that will be adopted by the vector as read-only.
   @param ci
      Count of items in the array pointed to by pt.
   */
   vector_base(std::size_t ciEmbedded) :
      raw_vector<T, smc_bCopyConstructible>(ciEmbedded) {
   }
   vector_base(T const * pt, std::size_t ci) :
      raw_vector<T, smc_bCopyConstructible>(pt, ci) {
   }

   /*! See raw_vector<T>::assign_move().

   @param v
      Source vector.
   */
   void assign_move(vector_base && v) {
      raw_vector<T, smc_bCopyConstructible>::assign_move(
         static_cast<raw_vector<T, smc_bCopyConstructible> &&>(v)
      );
   }

   /*! See raw_vector<T>::assign_move_dynamic_or_move_items().

   @param v
      Source vector.
   */
   void assign_move_dynamic_or_move_items(vector_base && v) {
      raw_vector<T, smc_bCopyConstructible>::assign_move_dynamic_or_move_items(
         static_cast<raw_vector<T, smc_bCopyConstructible> &&>(v)
      );
   }

   /*! Converts a possibly negative item index into a pointer into the item array, throwing an
   exception if the result is out of bounds for the item array.

   @param i
      If positive, this is interpreted as a 0-based index; if negative, it’s interpreted as a
      1-based index from the end of the item array by adding this->size() to it.
   @return
      Pointer to the element.
   */
   T const * translate_index(std::ptrdiff_t i) const {
      return static_cast<T const *>(raw_vector<T, smc_bCopyConstructible>::translate_offset(
         static_cast<std::ptrdiff_t>(sizeof(T)) * i
      ));
   }

   /*! Converts a left-closed, right-open interval with possibly negative element indices into one
   consisting of two pointers into the item array.

   @param iBegin
      Left endpoint of the interval, inclusive. If positive, this is interpreted as a 0-based index;
      if negative, it’s interpreted as a 1-based index from the end of the item array by adding
      this->size() to it.
   @param iEnd
      Right endpoint of the interval, exclusive. If positive, this is interpreted as a 0-based
      index; if negative, it’s interpreted as a 1-based index from the end of the item array by
      adding this->size() to it.
   @return
      Left-closed, right-open interval such that return.first <= i < return.second, or the empty
      interval [nullptr, nullptr) if the indices represent an empty interval after being adjusted.
   */
   std::pair<T const *, T const *> translate_range(
      std::ptrdiff_t iBegin, std::ptrdiff_t iEnd
   ) const {
      auto range(raw_trivial_vextr_impl::translate_byte_range(
         static_cast<std::ptrdiff_t>(sizeof(T)) * iBegin,
         static_cast<std::ptrdiff_t>(sizeof(T)) * iEnd
      ));
      return std::make_pair(
         static_cast<T const *>(range.first), static_cast<T const *>(range.second)
      );
   }
};

// Partial specialization for copyable types.
template <typename T>
class vector_base<T, true> : public vector_base<T, false> {
public:
   /*! Returns a slice of the vector.

   @param iBegin
      Index of the first element. See abc::collections::vector_base::translate_range() for allowed
      begin index values.
   @param iEnd
      Index of the last element, exclusive. See abc::collections::vector_base::translate_range() for
      allowed end index values.
   */
   dmvector<T, true> slice(std::ptrdiff_t iBegin) const {
      return slice(iBegin, this->size());
   }
   dmvector<T, true> slice(std::ptrdiff_t iBegin, std::ptrdiff_t iEnd) const {
      auto range(this->translate_range(iBegin, iEnd));
      return dmvector<T, true>(range.first, range.second);
   }

protected:
   /*! Constructor. The overload with ciEmbedded constructs the object as empty, setting m_p to
   nullptr or an empty string; the overload with pt constructs the object assigning an item array.

   @param ciEmbedded
      Count of slots in the embedded item array, or 0 if no embedded item array is present.
   @param pt
      Pointer to an array that will be adopted by the vector as read-only.
   @param ci
      Count of items in the array pointed to by pt.
   */
   vector_base(std::size_t ciEmbedded) :
      vector_base<T, false>(ciEmbedded) {
   }
   vector_base(T const * pt, std::size_t ci) :
      vector_base<T, false>(pt, ci) {
   }
};

} //namespace detail
} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::mvector

namespace abc {
namespace collections {

/*! Class to be used as argument type for functions that want to modify a vector argument, since
this allows for in-place alterations to the vector. Both smvector and dmvector are automatically
converted to this. */
template <typename T, bool t_bCopyConstructible /*= std::is_copy_constructible<T>::value*/>
class mvector;

/* Partial specialization for non-copyable types. Note that it doesn’t force t_bCopyConstructible to
false on vector_base, so that mvector<T, true> can inherit from this and still get all the copyable-
only members of vector_base<T, true>. */
template <typename T>
class mvector<T, false> : public detail::vector_base<T, std::is_copy_constructible<T>::value> {
private:
   //! true if T is copy constructible, or false otherwise.
   static bool const smc_bCopyConstructible = std::is_copy_constructible<T>::value;
   //! Shortcut to access the base class.
   typedef detail::vector_base<T, smc_bCopyConstructible> vector_base_;

public:
   //! See detail::vector_base::iterator.
   typedef typename vector_base_::iterator iterator;
   //! See detail::vector_base::const_iterator.
   typedef typename vector_base_::const_iterator const_iterator;
   //! See detail::vector_base::reverse_iterator.
   typedef typename vector_base_::reverse_iterator reverse_iterator;
   //! See detail::vector_base::const_reverse_iterator.
   typedef typename vector_base_::const_reverse_iterator const_reverse_iterator;

public:
   /*! Assignment operator. R-value-reference arguments will have their contents transferred to
   *this.

   @param v
      Source vector.
   @return
      *this.
   */
   mvector & operator=(dmvector<T> && v) {
      this->assign_move(std::move(v));
      return *this;
   }

   /*! Concatenation-assignment operator.

   @param v
      Vector to concatenate.
   @return
      *this.
   */
   mvector & operator+=(mvector && v) {
      this->insert_move(this->cend(), v.begin().base(), v.size());
      return *this;
   }

   //! See detail::vector_base::operator[]().
   T & operator[](std::ptrdiff_t i) {
      return const_cast<T &>(vector_base_::operator[](i));
   }
   T const & operator[](std::ptrdiff_t i) const {
      return vector_base_::operator[](i);
   }

   /*! Adds elements at the end of the vector.

   @param t
      Element to add.
   */
   void push_back(typename std::remove_const<T>::type && t) {
      insert(this->cend(), std::move(t));
   }

   //! See detail::vector_base::begin(). Here also available in non-const overload.
   iterator begin() {
      return iterator(detail::raw_vextr_impl_base::begin<T>());
   }
   using vector_base_::begin;

   //! Removes all elements from the vector.
   void clear() {
      static_cast<vector_base_ *>(this)->~vector_base_();
      this->assign_empty();
   }

   //! See detail::vector_base::end(). Here also available in non-const overload.
   iterator end() {
      return iterator(detail::raw_vextr_impl_base::end<T>());
   }
   using vector_base_::end;

   /*! Inserts elements at a specific position in the vector.

   @param iOffset
      Index at which the element should be inserted. See detail::vector_base::translate_index() for
      allowed index values.
   @param itOffset
      Iterator at which the element should be inserted.
   @param t
      Element to insert.
   */
   void insert(std::ptrdiff_t iOffset, typename std::remove_const<T>::type && t) {
      this->insert_move(this->translate_index(iOffset), &t, 1);
   }
   void insert(const_iterator itOffset, typename std::remove_const<T>::type && t) {
      this->validate_pointer(itOffset.base());
      this->insert_move(itOffset.base(), &t, 1);
   }

   //! See detail::vector_base::rbegin(). Here also available in non-const overload.
   reverse_iterator rbegin() {
      return reverse_iterator(iterator(detail::raw_vextr_impl_base::end<T>()));
   }
   using vector_base_::rbegin;

   /*! Removes a single element from the vector.

   @param i
      Index of the element to remove. See detail::vector_base::translate_index() for allowed index
      values.
   @param it
      Iterator to the element to remove.
   */
   void remove_at(std::ptrdiff_t i) {
      T const * pt = this->translate_index(i);
      this->remove(pt, pt + 1);
   }
   void remove_at(const_iterator it) {
      this->validate_pointer_noend(it.base());
      this->remove(it.base(), (it + 1).base());
   }

   //! See detail::vector_base::rend(). Here also available in non-const overload.
   reverse_iterator rend() {
      return reverse_iterator(iterator(detail::raw_vextr_impl_base::begin<T>()));
   }
   using vector_base_::rend;

   /*! Removes a range of elements from the vector.

   @param iBegin
      Index of the first element. See detail::vector_base::translate_range() for allowed begin index
      values.
   @param itBegin
      Iterator to the first element to remove.
   @param iEnd
      Index of the last element, exclusive. See detail::vector_base::translate_range() for allowed
      end index values.
   @param itEnd
      Iterator to beyond the last element to remove.
   */
   void remove_range(std::ptrdiff_t iBegin, std::ptrdiff_t iEnd) {
      auto range(this->translate_range(iBegin, iEnd));
      this->remove(range.first, range.second);
   }
   void remove_range(std::ptrdiff_t iBegin, const_iterator itEnd) {
      auto range(this->translate_range(iBegin, itEnd - this->cbegin()));
      this->remove(range.first, range.second);
   }
   void remove_range(const_iterator itBegin, std::ptrdiff_t iEnd) {
      auto range(this->translate_range(itBegin - this->cbegin(), iEnd));
      this->remove(range.first, range.second);
   }
   void remove_range(const_iterator itBegin, const_iterator itEnd) {
      this->validate_pointer(itBegin.base());
      this->validate_pointer(itEnd.base());
      this->remove(itBegin.base(), itEnd.base());
   }

   /*! Ensures that the item array has at least ciMin of actual item space. If this causes *this to
   switch to using a different item array, any elements in the current one will be destructed unless
   bPreserve == true, which will cause them to be moved to the new item array.

   @param ciMin
      Minimum count of elements requested.
   @param bPreserve
      If true, the previous contents of the item array will be preserved even if the reallocation
      causes the vector to switch to a different item array.
   */
   void set_capacity(std::size_t ciMin, bool bPreserve) {
      vector_base_::set_capacity(ciMin, bPreserve);
   }

   /*! Changes the count of items in the vector. If the new item count is greater than the current
   one, the added elements will be left uninitialized; it’s up to the caller to make sure that these
   elements are properly constructed, or problems will arise when the destructor will attempt to
   destruct these elements.

   @param ci
      New vector size.
   */
   void set_size(std::size_t ci) {
      vector_base_::set_size(ci);
   }

   //! Resizes the vector so that it only takes up as much memory as strictly necessary.
   void shrink_to_fit() {
      // TODO: implement this.
   }

protected:
   /*! Constructor. Constructs the object as empty, setting m_p to nullptr.

   @param cbEmbeddedCapacity
      Size of the embedded item array, in bytes, or 0 if no embedded item array is present.
   */
   mvector(std::size_t cbEmbeddedCapacity) :
      vector_base_(cbEmbeddedCapacity) {
   }
};

// Partial specialization for copyable types.
template <typename T>
class mvector<T, true> : public mvector<T, false> {
public:
   //! See detail::vector_base::iterator.
   typedef typename mvector<T, false>::iterator iterator;
   //! See detail::vector_base::const_iterator.
   typedef typename mvector<T, false>::const_iterator const_iterator;
   //! See detail::vector_base::reverse_iterator.
   typedef typename mvector<T, false>::reverse_iterator reverse_iterator;
   //! See detail::vector_base::const_reverse_iterator.
   typedef typename mvector<T, false>::const_reverse_iterator const_reverse_iterator;

public:
   /*! Assignment operator. R-value-reference arguments will have their contents transferred to
   *this.

   @param v
      Source vector.
   @return
      *this.
   */
   mvector & operator=(mvector const & v) {
      this->assign_copy(v.cbegin().base(), v.cend().base());
      return *this;
   }
   mvector & operator=(dmvector<T> && v) {
      this->assign_move(std::move(v));
      return *this;
   }

   /*! Concatenation-assignment operator.

   @param v
      Vector to concatenate.
   @return
      *this.
   */
   mvector & operator+=(mvector const & v) {
      this->insert_copy(this->cend().base(), v.begin().base(), v.size());
      return *this;
   }
   mvector & operator+=(mvector && v) {
      this->insert_move(this->cend().base(), v.begin().base(), v.size());
      return *this;
   }

   /*! Adds elements at the end of the vector.

   @param t
      Element to copy (const &) or move (&&) to the end of the vector.
   @param pt
      Pointer to an array of elements to copy to the end of the vector.
   @param ci
      Count of elements in the array pointed to by pt.
   */
   void push_back(T const & t) {
      this->insert_copy(this->cend().base(), &t, 1);
   }
   void push_back(typename std::remove_const<T>::type && t) {
      this->insert_move(this->cend().base(), &t, 1);
   }
   void push_back(T const * pt, std::size_t ci) {
      this->insert_copy(this->cend().base(), pt, ci);
   }

   /*! Inserts elements at a specific position in the vector.

   @param iOffset
      Index at which the element should be inserted. See detail::vector_base::translate_index() for
      allowed index values.
   @param itOffset
      Iterator at which the element should be inserted.
   @param t
      Element to insert.
   @param pt
      Pointer to the first element to insert.
   @param ci
      Count of elements in the array pointed to by pt.
   */
   void insert(std::ptrdiff_t iOffset, T const & t) {
      this->insert_copy(this->translate_index(iOffset), &t, 1);
   }
   void insert(std::ptrdiff_t iOffset, typename std::remove_const<T>::type && t) {
      this->insert_move(this->translate_index(iOffset), &t, 1);
   }
   void insert(std::ptrdiff_t iOffset, T const * pt, std::size_t ci) {
      this->insert_copy(this->translate_index(iOffset), pt, ci);
   }
   void insert(const_iterator itOffset, T const & t) {
      this->validate_pointer(itOffset.base());
      this->insert_copy(itOffset.base(), &t, 1);
   }
   void insert(const_iterator itOffset, typename std::remove_const<T>::type && t) {
      this->validate_pointer(itOffset.base());
      this->insert_move(itOffset.base(), &t, 1);
   }
   void insert(const_iterator itOffset, T const * pt, std::size_t ci) {
      this->validate_pointer(itOffset.base());
      this->insert_copy(itOffset.base(), pt, ci);
   }

protected:
   //! See mvector<T, false>::mvector().
   mvector(std::size_t cbEmbeddedCapacity) :
      mvector<T, false>(cbEmbeddedCapacity) {
   }
};

} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::dmvector

namespace abc {
namespace collections {

//! Dynamically-allocated mutable vector.
template <typename T, bool t_bCopyConstructible /*= std::is_copy_constructible<T>::value*/>
class dmvector;

// Partial specialization for non-copyable types.
template <typename T>
class dmvector<T, false> : public mvector<T, false> {
public:
   /*! Constructor. The individual items or the entire source item array will be moved to *this.

   @param v
      Source vector.
   @param v1
      First source vector.
   @param v2
      Second source vector.
   @param ci
      Count of items in the array pointed to by pt.
   */
   dmvector() :
      mvector<T, false>(0) {
   }
   dmvector(dmvector && v) :
      mvector<T, false>(0) {
      this->assign_move(std::move(v));
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmvector && overload.
   dmvector(mvector<T, false> && v) :
      mvector<T, false>(0) {
      this->assign_move_dynamic_or_move_items(std::move(v));
   }
   dmvector(mvector<T, false> && v1, mvector<T, false> && v2) :
      mvector<T, false>(0) {
      this->assign_concat_move(v1.begin().base(), v1.size(), v2.begin().base(), v2.size());
   }

   /*! Assignment operator. The individual items or the entire source item array will be moved to
   *this.

   @param v
      Source vector.
   @return
      *this.
   */
   dmvector & operator=(dmvector && v) {
      this->assign_move(std::move(v));
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmvector && overload.
   dmvector & operator=(mvector<T, false> && v) {
      this->assign_move_dynamic_or_move_items(std::move(v));
      return *this;
   }
};

// Partial specialization for copyable types.
template <typename T>
class dmvector<T, true> : public mvector<T, true> {
public:
   /*! Constructor. R-value-reference arguments (v, v1, v2) will have their contents transferred to
   *this.

   @param v
      Source vector.
   @param v1
      First source vector.
   @param v2
      Second source vector.
   @param at
      Source array whose elements should be copied.
   @param pt
      Pointer to an array whose elements should be copied.
   @param ci
      Count of items in the array pointed to by pt.
   @param p1Begin
      Pointer to the start of the first source array, whose elements should be copied.
   @param p1End
      Pointer to the end of the first source array.
   @param p2Begin
      Pointer to the start of the second source array, whose elements should be copied.
   @param p2End
      Pointer to the end of the second source array.
   */
   dmvector() :
      mvector<T, true>(0) {
   }
   dmvector(dmvector const & v) :
      mvector<T, true>(0) {
      this->assign_copy(v.cbegin().base(), v.cend().base());
   }
   dmvector(dmvector && v) :
      mvector<T, true>(0) {
      this->assign_move(std::move(v));
   }
   dmvector(mvector<T, true> const & v) :
      mvector<T, true>(0) {
      this->assign_copy(v.cbegin().base(), v.cend().base());
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmvector && overload.
   dmvector(mvector<T, true> && v) :
      mvector<T, true>(0) {
      this->assign_move_dynamic_or_move_items(std::move(v));
   }
   dmvector(mvector<T, true> const & v1, mvector<T, true> const & v2) :
      mvector<T, true>(0) {
      this->assign_concat(
         v1.cbegin().base(), v1.cend().base(), v2.cbegin().base(), v2.cend().base(), 0
      );
   }
   dmvector(mvector<T, true> && v1, mvector<T, true> const & v2) :
      mvector<T, true>(0) {
      this->assign_concat(
         v1.begin().base(), v1.end().base(), v2.cbegin().base(), v2.cend().base(), 1
      );
   }
   dmvector(mvector<T, true> const & v1, mvector<T, true> && v2) :
      mvector<T, true>(0) {
      this->assign_concat(
         v1.cbegin().base(), v1.cend().base(), v2.begin().base(), v2.end().base(), 2
      );
   }
   dmvector(mvector<T, true> && v1, mvector<T, true> && v2) :
      mvector<T, true>(0) {
      this->assign_concat_move(
         v1.begin().base(), v1.end().base(), v2.begin().base(), v2.end().base()
      );
   }
   template <std::size_t t_ci>
   explicit dmvector(T const (& at)[t_ci]) :
      mvector<T, true>(0) {
      this->assign_copy(at, at + t_ci);
   }
   dmvector(T const * ptBegin, T const * ptEnd) :
      mvector<T, true>(0) {
      this->assign_copy(ptBegin, ptEnd);
   }
   dmvector(T const * pt1Begin, T const * pt1End, T const * pt2Begin, T const * pt2End) :
      mvector<T, true>(0) {
      this->assign_concat(pt1Begin, pt1End, pt2Begin, pt2End, 0);
   }

   /*! Assignment operator. R-value-reference arguments will have their contents transferred to
   *this.

   @param v
      Source vector.
   @return
      *this.
   */
   dmvector & operator=(dmvector const & v) {
      this->assign_copy(v.cbegin().base(), v.cend().base());
      return *this;
   }
   dmvector & operator=(dmvector && v) {
      this->assign_move(std::move(v));
      return *this;
   }
   dmvector & operator=(mvector<T, true> const & v) {
      this->assign_copy(v.cbegin().base(), v.cend().base());
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmvector && overload.
   dmvector & operator=(mvector<T, true> && v) {
      this->assign_move_dynamic_or_move_items(std::move(v));
      return *this;
   }
};


/*! Concatenation operator.

@param v1
   Left operand.
@param v2
   Right operand.
@return
   Vector resulting from the concatenation of v1 and v2.
*/
template <typename T>
inline typename std::enable_if<std::is_copy_constructible<T>::value, dmvector<T, true>>::type
operator+(detail::vector_base<T, true> const & v1, detail::vector_base<T, true> const & v2) {
   return dmvector<T, true>(
      static_cast<mvector<T, true> const &>(v1), static_cast<mvector<T, true> const &>(v2)
   );
}
/* Overloads taking an mvector r-value-reference as either or both operands; they can avoid creating
intermediate copies of the elements from one or both source vectors. */
template <typename T>
inline typename std::enable_if<std::is_copy_constructible<T>::value, dmvector<T, true>>::type
operator+(mvector<T, true> && v1, mvector<T, true> const & v2) {
   return dmvector<T, true>(std::move(v1), v2);
}
template <typename T>
inline typename std::enable_if<std::is_copy_constructible<T>::value, dmvector<T, true>>::type
operator+(mvector<T, true> const & v1, mvector<T, true> && v2) {
   return dmvector<T, true>(v1, std::move(v2));
}
template <typename T, bool t_bCopyConstructible>
inline dmvector<T, t_bCopyConstructible> operator+(
   mvector<T, t_bCopyConstructible> && v1, mvector<T, t_bCopyConstructible> && v2
) {
   return dmvector<T, t_bCopyConstructible>(std::move(v1), std::move(v2));
}

} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::smvector

namespace abc {
namespace collections {

/*! mvector_-derived class, good for clients that need in-place manipulation of vectors that are
most likely to be shorter than a known small size. */
template <
   typename T, std::size_t t_ciEmbeddedCapacity,
   bool t_bCopyConstructible = std::is_copy_constructible<T>::value
>
class smvector;

// Partial specialization for non-copyable types.
template <typename T, std::size_t t_ciEmbeddedCapacity>
class smvector<T, t_ciEmbeddedCapacity, false> :
   public mvector<T, false>,
   private detail::raw_vextr_prefixed_item_array<T, t_ciEmbeddedCapacity> {
private:
   using detail::raw_vextr_prefixed_item_array<T, t_ciEmbeddedCapacity>::smc_cbEmbeddedCapacity;

public:
   /*! Constructor. The individual items or the entire source item array will be moved to *this.

   @param v
      Source vector.
   */
   smvector() :
      mvector<T, false>(smc_cbEmbeddedCapacity) {
   }
   /* If the source is using its embedded item array, it will be copied without allocating a dynamic
   one; if the source is dynamic, it will be moved. Either way, this won’t throw. */
   smvector(smvector && v) :
      mvector<T, false>(smc_cbEmbeddedCapacity) {
      this->assign_move_dynamic_or_move_items(std::move(v));
   }
   /* If the source is using its embedded item array, it will be copied without allocating a dynamic
   one since it’s smaller than this object’s; if the source is dynamic, it will be moved. Either
   way, this won’t throw. */
   template <std::size_t t_ciEmbeddedCapacity2>
   smvector(typename std::enable_if<
      (t_ciEmbeddedCapacity > t_ciEmbeddedCapacity2), smvector<T, t_ciEmbeddedCapacity2, false> &&
   >::type v) :
      mvector<T, false>(smc_cbEmbeddedCapacity) {
      this->assign_move_dynamic_or_move_items(std::move(v));
   }
   /* This can throw exceptions, but it’s allowed to since it’s not the smvector && overload. This
   also covers smvector of different embedded fixed size > t_ciEmbeddedCapacity. */
   smvector(mvector<T, false> && v) :
      mvector<T, false>(smc_cbEmbeddedCapacity) {
      this->assign_move_dynamic_or_move_items(std::move(v));
   }
   smvector(dmvector<T, false> && v) :
      mvector<T, false>(smc_cbEmbeddedCapacity) {
      this->assign_move(std::move(v));
   }

   /*! Assignment operator. The individual items or the entire source item array will be moved to
   *this.

   @param v
      Source vector.
   @return
      *this.
   */
   /* If the source is using its embedded item array, it will be copied without allocating a dynamic
   one; if the source is dynamic, it will be moved. Either way, this won’t throw. */
   smvector & operator=(smvector && v) {
      this->assign_move_dynamic_or_move_items(std::move(v));
      return *this;
   }
   /* If the source is using its embedded item array, it will be copied without allocating a dynamic
   one since it’s smaller than this object’s; if the source is dynamic, it will be moved. Either
   way, this won’t throw. */
   template <std::size_t t_ciEmbeddedCapacity2>
   smvector & operator=(typename std::enable_if<
      (t_ciEmbeddedCapacity > t_ciEmbeddedCapacity2), smvector<T, t_ciEmbeddedCapacity2, false> &&
   >::type v) {
      this->assign_move_dynamic_or_move_items(std::move(v));
      return *this;
   }
   /* This can throw exceptions, but it’s allowed to since it’s not the smvector && overload. This
   also covers smvector of different embedded fixed size > t_ciEmbeddedCapacity. */
   smvector & operator=(mvector<T, false> && v) {
      this->assign_move_dynamic_or_move_items(std::move(v));
      return *this;
   }
   smvector & operator=(dmvector<T, false> && v) {
      this->assign_move(std::move(v));
      return *this;
   }
};

// Partial specialization for copyable types.
template <typename T, std::size_t t_ciEmbeddedCapacity>
class smvector<T, t_ciEmbeddedCapacity, true> :
   public mvector<T, true>,
   private detail::raw_vextr_prefixed_item_array<T, t_ciEmbeddedCapacity> {
private:
   using detail::raw_vextr_prefixed_item_array<T, t_ciEmbeddedCapacity>::smc_cbEmbeddedCapacity;

public:
   /*! Constructor. R-value-reference arguments will have their contents transferred to *this.

   @param v
      Source vector.
   @param at
      Source array whose elements should be copied.
   @param pt
      Pointer to an array whose elements should be copied.
   @param ci
      Count of items in the array pointed to by pt.
   */
   smvector() :
      mvector<T, true>(smc_cbEmbeddedCapacity) {
   }
   smvector(smvector const & v) :
      mvector<T, true>(smc_cbEmbeddedCapacity) {
      this->assign_copy(v.cbegin().base(), v.cend().base());
   }
   /* If the source is using its embedded item array, it will be copied without allocating a dynamic
   one; if the source is dynamic, it will be moved. Either way, this won’t throw. */
   smvector(smvector && v) :
      mvector<T, true>(smc_cbEmbeddedCapacity) {
      this->assign_move_dynamic_or_move_items(std::move(v));
   }
   /* If the source is using its embedded item array, it will be copied without allocating a dynamic
   one since it’s smaller than this object’s; if the source is dynamic, it will be moved. Either
   way, this won’t throw. */
   template <std::size_t t_ciEmbeddedCapacity2>
   smvector(typename std::enable_if<
      (t_ciEmbeddedCapacity > t_ciEmbeddedCapacity2), smvector<T, t_ciEmbeddedCapacity2, true> &&
   >::type v) :
      mvector<T, true>(smc_cbEmbeddedCapacity) {
      this->assign_move_dynamic_or_move_items(std::move(v));
   }
   smvector(mvector<T, true> const & v) :
      mvector<T, true>(smc_cbEmbeddedCapacity) {
      this->assign_copy(v.cbegin().base(), v.cend().base());
   }
   /* This can throw exceptions, but it’s allowed to since it’s not the smvector && overload. This
   also covers smvector of different embedded fixed size > t_ciEmbeddedCapacity. */
   smvector(mvector<T, true> && v) :
      mvector<T, true>(smc_cbEmbeddedCapacity) {
      this->assign_move_dynamic_or_move_items(std::move(v));
   }
   smvector(dmvector<T, true> && v) :
      mvector<T, true>(smc_cbEmbeddedCapacity) {
      this->assign_move(std::move(v));
   }
   template <std::size_t t_ci>
   explicit smvector(T const (& at)[t_ci]) :
      mvector<T, true>(smc_cbEmbeddedCapacity) {
      this->assign_copy(at, at + t_ci);
   }
   smvector(T const * ptBegin, T const * ptEnd) :
      mvector<T, true>(smc_cbEmbeddedCapacity) {
      this->assign_copy(ptBegin, ptEnd);
   }

   /*! Assignment operator. R-value-reference arguments will have their contents transferred to
   *this.

   @param v
      Source vector.
   @return
      *this.
   */
   smvector & operator=(smvector const & v) {
      this->assign_copy(v.cbegin().base(), v.cend().base());
      return *this;
   }
   /* If the source is using its embedded item array, it will be copied without allocating a dynamic
   one; if the source is dynamic, it will be moved. Either way, this won’t throw. */
   smvector & operator=(smvector && v) {
      this->assign_move_dynamic_or_move_items(std::move(v));
      return *this;
   }
   /* If the source is using its embedded item array, it will be copied without allocating a dynamic
   one since it’s smaller than this object’s; if the source is dynamic, it will be moved. Either
   way, this won’t throw. */
   template <std::size_t t_ciEmbeddedCapacity2>
   smvector & operator=(typename std::enable_if<
      (t_ciEmbeddedCapacity > t_ciEmbeddedCapacity2), smvector<T, t_ciEmbeddedCapacity2, true> &&
   >::type v) {
      this->assign_move_dynamic_or_move_items(std::move(v));
      return *this;
   }
   smvector & operator=(mvector<T, true> const & v) {
      this->assign_copy(v.cbegin().base(), v.cend().base());
      return *this;
   }
   /* This can throw exceptions, but it’s allowed to since it’s not the smvector && overload. This
   also covers smvector of different embedded fixed size > t_ciEmbeddedCapacity. */
   smvector & operator=(mvector<T, true> && v) {
      this->assign_move_dynamic_or_move_items(std::move(v));
      return *this;
   }
   smvector & operator=(dmvector<T, true> && v) {
      this->assign_move(std::move(v));
      return *this;
   }
};

} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
