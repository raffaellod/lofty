/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013
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

#ifndef ABC_VECTOR_HXX
#define ABC_VECTOR_HXX

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abc/_vextr.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_vector

namespace abc {

/** Thin templated wrapper for _raw_*_vextr_impl to make the interface of those two classes
consistent, so vector doesn’t need specializations.
*/
template <
   typename T,
   bool t_bTrivial = std::is_trivial<T>::value,
   bool t_bCopyConstructible = std::is_copy_constructible<T>::value
>
class _raw_vector;

// Partial specialization for non-trivial, non-copyable types.
template <typename T>
class _raw_vector<T, false, false> :
   public _raw_complex_vextr_impl {
public:

   /** Destructor.
   */
   ~_raw_vector() {
      type_void_adapter type;
      type.set_destr_fn<T>();
      destruct_items(type);
   }


   /** Appends one or more elements by moving them to the end of the vector’s item array.

   p
      Pointer to the first element to add.
   ci
      Count of elements to add.
   */
   void append_move(T * p, size_t ci) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      _raw_complex_vextr_impl::append(type, p, ci, true);
   }


   /** Moves the contents of the two sources to *this.

   p1
      Pointer to the first source array.
   ci1
      Count of elements in the first source array.
   p2
      Pointer to the second source array.
   ci2
      Count of elements in the second source array.
   */
   void assign_concat_move(T const * p1, size_t ci1, T const * p2, size_t ci2) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      _raw_complex_vextr_impl::assign_concat(type, p1, ci1, true, p2, ci2, true);
   }


   /** See _raw_complex_vextr_impl::assign_move().
   */
   void assign_move(_raw_complex_vextr_impl && rcvi) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      _raw_complex_vextr_impl::assign_move(type, std::move(rcvi));
   }


   /** See _raw_complex_vextr_impl::assign_move_dynamic_or_move_items().
   */
   void assign_move_dynamic_or_move_items(_raw_complex_vextr_impl && rcvi) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      _raw_complex_vextr_impl::assign_move_dynamic_or_move_items(type, std::move(rcvi));
   }


   /** Removes all elements from the vector.
   */
   void clear() {
      this->~_raw_vector();
      assign_empty();
   }


   /** Inserts elements at a specific position in the vector by moving them.

   iOffset
      Index at which the elements should be inserted. If negative, it will be interpreted as an
      offset from the end of the vextr.
   pAdd
      Pointer to the first element to add.
   ciAdd
      Count of elements to add.
   */
   void insert_move(ptrdiff_t iOffset, T * p, size_t ci) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      _raw_complex_vextr_impl::insert(type, iOffset, p, ci, true);
   }


   /** See _raw_complex_vextr_impl::remove_at().
   */
   void remove_at(ptrdiff_t iOffset, ptrdiff_t ciRemove) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      _raw_complex_vextr_impl::remove_at(type, iOffset, ciRemove);
   }


   /** See _raw_complex_vextr_impl::set_capacity().
   */
   void set_capacity(size_t ciMin, bool bPreserve) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      _raw_complex_vextr_impl::set_capacity(type, ciMin, bPreserve);
   }


protected:

   /** Constructor. See _raw_complex_vextr_impl::_raw_complex_vextr_impl().
   */
   _raw_vector(size_t ciStaticMax) :
      _raw_complex_vextr_impl(ciStaticMax) {
   }
   _raw_vector(void const * pConstSrc, size_t ciSrc) :
      _raw_complex_vextr_impl(pConstSrc, ciSrc) {
   }
};

// Partial specialization for non-trivial, copyable types.
template <typename T>
class _raw_vector<T, false, true> :
   public _raw_vector<T, false, false> {
public:

   /** Appends one or more elements by copying them to the end of the vector’s item array.

   p
      Pointer to the first element to add.
   ci
      Count of elements to add.
   */
   void append_copy(T const * p, size_t ci) {
      type_void_adapter type;
      type.set_copy_fn<T>();
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      _raw_complex_vextr_impl::append(type, p, ci, false);
   }


   /** See _raw_complex_vextr_impl::assign_copy().
   */
   void assign_copy(T const * p, size_t ci) {
      type_void_adapter type;
      type.set_copy_fn<T>();
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      _raw_complex_vextr_impl::assign_copy(type, p, ci);
   }


   /** See _raw_complex_vextr_impl::assign_concat().
   */
   void assign_concat(
      T const * p1, size_t ci1, bool bMove1, T const * p2, size_t ci2, bool bMove2
   ) {
      type_void_adapter type;
      type.set_copy_fn<T>();
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      _raw_complex_vextr_impl::assign_concat(type, p1, ci1, bMove1, p2, ci2, bMove2);
   }


   /** Inserts elements at a specific position in the vector by copying them.

   iOffset
      Index at which the elements should be inserted. If negative, it will be interpreted as an
      offset from the end of the vextr.
   pAdd
      Pointer to the first element to add.
   ciAdd
      Count of elements to add.
   */
   void insert_copy(ptrdiff_t iOffset, T const * p, size_t ci) {
      type_void_adapter type;
      type.set_copy_fn<T>();
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      _raw_complex_vextr_impl::insert(type, iOffset, p, ci, false);
   }


protected:

   /** Constructor. See _raw_vector<T, false, false>::_raw_vector<T, false, false>().
   */
   _raw_vector(size_t ciStaticMax) :
      _raw_vector<T, false, false>(ciStaticMax) {
   }
   _raw_vector(void const * pConstSrc, size_t ciSrc) :
      _raw_vector<T, false, false>(pConstSrc, ciSrc) {
   }
};

// Partial specialization for trivial (and copyable) types. Methods here ignore the bMove argument
// for the individual elements, because move semantics don’t apply (trivial values are always
// copied).
template <typename T>
class _raw_vector<T, true, true> :
   public _raw_trivial_vextr_impl {
public:

   /** Appends one or more elements.

   p
      Pointer to the first element to add.
   ci
      Count of elements to add.
   */
   void append_copy(void const * p, size_t ci) {
      _raw_trivial_vextr_impl::append(sizeof(T), p, ci);
   }


   /** Appends one or more elements. Semantically this is supposed to move them, but for trivial
   types that’s the same as copying them.

   p
      Pointer to the first element to add.
   ci
      Count of elements to add.
   */
   void append_move(void * p, size_t ci) {
      _raw_trivial_vextr_impl::append(sizeof(T), p, ci);
   }


   /** See _raw_trivial_vextr_impl::assign_copy().
   */
   void assign_copy(void const * p, size_t ci) {
      _raw_trivial_vextr_impl::assign_copy(sizeof(T), p, ci);
   }


   /** See _raw_trivial_vextr_impl::assign_concat().
   */
   void assign_concat(
      void const * p1, size_t ci1, bool bMove1, void const * p2, size_t ci2, bool bMove2
   ) {
      ABC_UNUSED_ARG(bMove1);
      ABC_UNUSED_ARG(bMove2);
      _raw_trivial_vextr_impl::assign_concat(sizeof(T), p1, ci1, p2, ci2);
   }


   /** Moves the contents of the two sources to *this.

   p1
      Pointer to the first source array.
   ci1
      Count of elements in the first source array.
   p2
      Pointer to the second source array.
   ci2
      Count of elements in the second source array.
   */
   void assign_concat_move(void const * p1, size_t ci1, void const * p2, size_t ci2) {
      _raw_trivial_vextr_impl::assign_concat(sizeof(T), p1, ci1, p2, ci2);
   }


   /** See _raw_trivial_vextr_impl::assign_move().
   */
   void assign_move(_raw_trivial_vextr_impl && rtvi) {
      _raw_trivial_vextr_impl::assign_move(std::move(rtvi));
   }


   /** See _raw_trivial_vextr_impl::assign_move_dynamic_or_move_items().
   */
   void assign_move_dynamic_or_move_items(_raw_trivial_vextr_impl && rtvi) {
      _raw_trivial_vextr_impl::assign_move_dynamic_or_move_items(std::move(rtvi));
   }


   /** Removes all elements from the vector.
   */
   void clear() {
      this->~_raw_vector();
      assign_empty();
   }


   /** Inserts one or more elements.

   iOffset
      Index at which the items should be inserted. If negative, it’s going to be interpreted as an
      index from the end of the vextr.
   p
      Pointer to the first element to add.
   ci
      Count of elements to add.
   */
   void insert_copy(ptrdiff_t iOffset, void const * p, size_t ci) {
      _raw_trivial_vextr_impl::insert(sizeof(T), iOffset, p, ci);
   }


   /** Inserts one or more elements. Semantically this is supposed to move them, but for trivial types
   that’s the same as copying them.

   iOffset
      Index at which the items should be inserted. If negative, it’s going to be interpreted as an
      index from the end of the vextr.
   p
      Pointer to the first element to add.
   ci
      Count of elements to add.
   */
   void insert_move(ptrdiff_t iOffset, void const * p, size_t ci) {
      _raw_trivial_vextr_impl::insert(sizeof(T), iOffset, p, ci);
   }


   /** See _raw_trivial_vextr_impl::remove_at().
   */
   void remove_at(ptrdiff_t iOffset, ptrdiff_t ciRemove) {
      _raw_trivial_vextr_impl::remove_at(sizeof(T), iOffset, ciRemove);
   }


   /** See _raw_trivial_vextr_impl::set_capacity().
   */
   void set_capacity(size_t ciMin, bool bPreserve) {
      _raw_trivial_vextr_impl::set_capacity(sizeof(T), ciMin, bPreserve);
   }


protected:

   /** Constructor. See _raw_trivial_vextr_impl::_raw_trivial_vextr_impl().
   */
   _raw_vector(size_t ciStaticMax) :
      _raw_trivial_vextr_impl(ciStaticMax) {
   }
   _raw_vector(void const * pConstSrc, size_t ciSrc) :
      _raw_trivial_vextr_impl(pConstSrc, ciSrc) {
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::vector_base


namespace abc {

// Forward declarations.
template <typename T>
class mvector;
template <typename T>
class dmvector;


/** Base class for vectors.

See [DOC:4019 abc::*str_ and abc::*vector design] for implementation details for this and all the
*vector classes.
*/
template <typename T>
class vector_base :
   protected _raw_vector<T>,
   public _iterable_vector<vector_base<T>, T>,
   public support_explicit_operator_bool<vector_base<T>> {

   /** Shortcut for the base class providing iterator-based types and methods. */
   typedef _iterable_vector<vector_base<T>, T> itvec;


public:

   /** Element type. */
   typedef T item_type;
   /** See _iterable_vector::const_iterator. */
   typedef typename itvec::const_iterator const_iterator;


public:

   /** Element access operator.

   i
      Element index.
   return
      Element at index i.
   */
   T const & operator[](intptr_t i) const {
      this->validate_index(i);
      return data()[i];
   }


   /** Returns true if the length is greater than 0.

   return
      true if the vector is not empty, or false otherwise.
   */
   explicit_operator_bool() const {
      return size() > 0;
   }


   /** Returns the maximum number of elements the array can currently hold.

   return
      Current size of the item array storage, in elements.
   */
   size_t capacity() const {
      return _raw_vector<T>::capacity();
   }


   /** Returns a pointer to the item array.

   return
      Pointer to the item array.
   */
   T * data() {
      // For some reason, GCC doesn’t like this:
      //    return _raw_vector<T>::data<T>();
      return _raw_vextr_impl_base::data<T>();
   }
   T const * data() const {
      // For some reason, GCC doesn’t like this:
      //    return _raw_vector<T>::data<T>();
      return _raw_vextr_impl_base::data<T>();
   }


   /** Returns the count of elements in the array.

   return
      Count of elements.
   */
   size_t size() const {
      return _raw_vector<T>::size();
   }


   /** Looks for the specified value; returns the index of the first matching element, or -1 for no
   matches.

   TODO: comment signature.
   */
   ptrdiff_t index_of(T const & t, ptrdiff_t iFirst = 0) const {
      T const * pt0(data()), * ptEnd(pt0 + size());
      for (T const * pt(pt0 + this->adjust_index(iFirst)); pt < ptEnd; ++pt) {
         if (*pt == t) {
            return pt - pt0;
         }
      }
      return -1;
   }


   /** Looks for the specified value; returns the index of the first matching element, or -1 for no
   matches.

   TODO: comment signature.
   */
   ptrdiff_t last_index_of(T const & t) const {
      return last_index_of(t, ptrdiff_t(size()));
   }
   ptrdiff_t last_index_of(T const & t, ptrdiff_t iFirst) const {
      T const * pt0(data());
      for (T const * pt(pt0 + this->adjust_index(iFirst)); pt >= pt0; --pt) {
         if (*pt == t) {
            return pt - pt0;
         }
      }
      return -1;
   }


   /** Returns a segment of the vector.

   iFirst
      0-based index of the first element. If negative, it’s 1-based index from the end of the
      vector.
   [ci]
      Count of elements to return. If negative, it’s the count of elements to skip, from the end of
      the vector.
   */
   dmvector<T> slice(ptrdiff_t iFirst) const {
      return slice(iFirst, size());
   }
   dmvector<T> slice(ptrdiff_t iFirst, ptrdiff_t ci) const {
      this->adjust_range(&iFirst, &ci);
      return dmvector<T>(data() + iFirst, size_t(ci));
   }


protected:

   /** Constructor. The overload with ciStatic constructs the object as empty, setting m_p to
   nullptr or an empty string; the overload with pt constructs the object assigning an item array.

   ciStatic
      Count of slots in the static item array, or 0 if no static item array is present.
   pt
      Pointer to an array that will be adopted by the vector as read-only.
   ci
      Count of items in the array pointed to by pt.
   */
   vector_base(size_t ciStatic) :
      _raw_vector<T>(ciStatic) {
   }
   vector_base(T const * pt, size_t ci) :
      _raw_vector<T>(pt, ci) {
   }


   /** See _raw_vector<T>::assign_move().

   v
      Source vector.
   */
   void assign_move(vector_base && v) {
      _raw_vector<T>::assign_move(static_cast<_raw_vector<T> &&>(v));
   }


   /** See _raw_vector<T>::assign_move_dynamic_or_move_items().

   v
      Source vector.
   */
   void assign_move_dynamic_or_move_items(vector_base && v) {
      _raw_vector<T>::assign_move_dynamic_or_move_items(static_cast<_raw_vector<T> &&>(v));
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::mvector


namespace abc {

/** vector_base-derived class, to be used as argument type for functions that want to modify a
vector argument, since this allows for in-place alterations to the vector. Both smvector and
dmvector are automatically converted to this.
*/
template <typename T>
class mvector :
   public vector_base<T> {

   /** Shortcut for the base class providing iterator-based types and methods. */
   typedef _iterable_vector<vector_base<T>, T> itvec;


public:

   /** See vector_base<T>::const_iterator. */
   typedef typename itvec::const_iterator const_iterator;


public:

   /** Assignment operator. R-value-reference arguments will have their contents transferred to
   *this.

   v
      Source vector.
   return
      *this.
   */
   mvector & operator=(mvector const & v) {
      this->assign_copy(v.data(), v.size());
      return *this;
   }
   mvector & operator=(dmvector<T> && v) {
      this->assign_move(std::move(v));
      return *this;
   }


   /** Concatenation-assignment operator.

   v
      Vector to concatenate.
   return
      *this.
   */
   mvector & operator+=(mvector const & v) {
      append_copy(v.data(), v.size());
      return *this;
   }
   mvector & operator+=(mvector && v) {
      append_move(v.data(), v.size());
      return *this;
   }


   /** See vector_base::operator[]().
   */
   T & operator[](intptr_t i) {
      return const_cast<T &>(vector_base<T>::operator[](i));
   }
   T const & operator[](intptr_t i) const {
      return vector_base<T>::operator[](i);
   }


   /** Adds elements at the end of the vector.

   t
      Element to copy (const &) or move (&&) to the end of the vector.
   pt
      Pointer to an array of elements to copy to the end of the vector.
   ci
      Count of elements in the array pointed to by pt.
   */
   void append(T const & t) {
      this->append_copy(&t, 1);
   }
   void append(typename std::remove_const<T>::type && t) {
      this->append_move(&t, 1);
   }
   void append(T const * pt, size_t ci) {
      this->append_copy(pt, ci);
   }


   /** Removes all elements from the vector.
   */
   void clear() {
      vector_base<T>::clear();
   }


   /** Inserts elements at a specific position in the vector.

   i
      0-based index of the element. If negative, it’s 1-based index from the end of the vector.
   TODO: comment signature.
   */
   void insert(ptrdiff_t i, T const & t) {
      this->insert_copy(i, &t, 1);
   }
   void insert(ptrdiff_t i, typename std::remove_const<T>::type && t) {
      this->insert_move(i, &t, 1);
   }
   void insert(ptrdiff_t i, T const * pt, size_t ci) {
      this->insert_copy(i, pt, ci);
   }
   void insert(const_iterator it, T const & t) {
      this->insert_copy(it - itvec::cbegin(), &t, 1);
   }
   void insert(const_iterator it, typename std::remove_const<T>::type && t) {
      this->insert_move(it - itvec::cbegin(), std::move(t), &t, 1);
   }
   void insert(const_iterator it, T const * pt, size_t ci) {
      this->insert_copy(it - itvec::cbegin(), pt, ci);
   }


   /** Removes elements from the vector.

   i
      0-based index of the element to be removed. If negative, it’s 1-based index from the end of
      the vector.
   it
      Iterator positioned on the first element to remove.
   ciRemove
      Count of elements to remove.
   */
   void remove_at(ptrdiff_t i, ptrdiff_t ciRemove = 1) {
      vector_base<T>::remove_at(i, ciRemove);
   }
   void remove_at(const_iterator it, ptrdiff_t ciRemove = 1) {
      vector_base<T>::remove_at(it - itvec::cbegin(), ciRemove);
   }


   /** Ensures that the item array has at least ciMin of actual item space. If this causes *this to
   switch to using a different item array, any elements in the current one will be destructed unless
   bPreserve == true, which will cause them to be moved to the new item array.

   ciMin
      Minimum count of elements requested.
   bPreserve
      If true, the previous contents of the item array will be preserved even if the reallocation
      causes the vector to switch to a different item array.
   */
   void set_capacity(size_t ciMin, bool bPreserve) {
      vector_base<T>::set_capacity(ciMin, bPreserve);
   }


   /** Resizes the vector so that it only takes up as much memory as strictly necessary.
   */
   void shrink_to_fit() {
      // TODO: implement this.
   }


protected:

   /** Constructor. Constructs the object as empty, setting m_p to nullptr.

   ciStatic
      Count of slots in the static item array, or 0 if no static item array is present.
   */
   mvector(size_t ciStaticMax) :
      vector_base<T>(ciStaticMax) {
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::dmvector


namespace abc {

/** Dynamically-allocated mutable vector.
*/
template <typename T>
class dmvector :
   public mvector<T> {
public:

   /** Constructor. R-value-reference arguments (v, v1, v2) will have their contents transferred to
   *this.

   v
      Source vector.
   v1
      First source vector.
   v2
      Second source vector.
   at
      Source array whose elements should be copied.
   pt
      Pointer to an array whose elements should be copied.
   ci
      Count of items in the array pointed to by pt.
   pt1
      Pointer to an array whose elements should be copied.
   ci1
      Count of items in the array pointed to by pt1.
   pt2
      Pointer to an array whose elements should be copied.
   ci2
      Count of items in the array pointed to by pt2.
   */
   dmvector() :
      mvector<T>(0) {
   }
   dmvector(dmvector const & v) :
      mvector<T>(0) {
      this->assign_copy(v.data(), v.size());
   }
   dmvector(dmvector && v) :
      mvector<T>(0) {
      this->assign_move(std::move(v));
   }
   dmvector(mvector<T> const & v) :
      mvector<T>(0) {
      this->assign_copy(v.data(), v.size());
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmvector && overload.
   dmvector(mvector<T> && v) :
      mvector<T>(0) {
      this->assign_move_dynamic_or_move_items(std::move(v));
   }
   dmvector(mvector<T> const & v1, mvector<T> const & v2) :
      mvector<T>(0) {
      this->assign_concat(v1.data(), v1.size(), false, v2.data(), v2.size(), false);
   }
   dmvector(mvector<T> && v1, mvector<T> const & v2) :
      mvector<T>(0) {
      this->assign_concat(v1.data(), v1.size(), true, v2.data(), v2.size(), false);
   }
   dmvector(mvector<T> const & v1, mvector<T> && v2) :
      mvector<T>(0) {
      this->assign_concat(v1.data(), v1.size(), false, v2.data(), v2.size(), true);
   }
   dmvector(mvector<T> && v1, mvector<T> && v2) :
      mvector<T>(0) {
      this->assign_concat_move(v1.data(), v1.size(), v2.data(), v2.size());
   }
   template <size_t t_ci>
   explicit dmvector(T const (& at)[t_ci]) :
      mvector<T>(0) {
      this->assign_copy(at, t_ci);
   }
   dmvector(T const * pt, size_t ci) :
      mvector<T>(0) {
      this->assign_copy(pt, ci);
   }
   dmvector(T const * pt1, size_t ci1, T const * pt2, size_t ci2) :
      mvector<T>(0) {
      this->assign_concat(pt1, ci1, false, pt2, ci2, false);
   }


   /** Assignment operator. R-value-reference arguments will have their contents transferred to
   *this.

   v
      Source vector.
   return
      *this;
   */
   dmvector & operator=(dmvector const & v) {
      this->assign_copy(v.data(), v.size());
      return *this;
   }
   dmvector & operator=(dmvector && v) {
      this->assign_move(std::move(v));
      return *this;
   }
   dmvector & operator=(mvector<T> const & v) {
      this->assign_copy(v.data(), v.size());
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmvector && overload.
   dmvector & operator=(mvector<T> && v) {
      this->assign_move_dynamic_or_move_items(std::move(v));
      return *this;
   }
};

} //namespace abc


/** Concatenation operator.

v1
   Left operand.
v2
   Right operand.
return
   Vector resulting from the concatenation of v1 and v2.
*/
template <typename T>
inline abc::dmvector<T> operator+(abc::vector_base<T> const & v1, abc::vector_base<T> const & v2) {
   return abc::dmvector<T>(
      static_cast<abc::mvector<T> const &>(v1), static_cast<abc::mvector<T> const &>(v2)
   );
}
// Overloads taking an mvector r-value-reference as either or both operands; they can avoid creating
// intermediate copies of the elements from one or both source vectors.
// TODO: verify that compilers actually select these overloads whenever possible.
template <typename T>
inline abc::dmvector<T> operator+(abc::mvector<T> && v1, abc::mvector<T> const & v2) {
   return abc::dmvector<T>(std::move(v1), v2);
}
template <typename T>
inline abc::dmvector<T> operator+(abc::mvector<T> const & v1, abc::mvector<T> && v2) {
   return abc::dmvector<T>(v1, std::move(v2));
}
template <typename T>
inline abc::dmvector<T> operator+(abc::mvector<T> && v1, abc::mvector<T> && v2) {
   return abc::dmvector<T>(std::move(v1), std::move(v2));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::smvector


namespace abc {

template <typename T, size_t t_ciStatic>
class smvector :
   public mvector<T> {
protected:

   /** Actual static item array size. */
   static size_t const smc_ciFixed = _ABC__RAW_VEXTR_IMPL_BASE__ADJUST_ITEM_COUNT(t_ciStatic);


public:

   /** Constructor. R-value-reference arguments will have their contents transferred to *this.

   v
      Source vector.
   at
      Source array whose elements should be copied.
   pt
      Pointer to an array whose elements should be copied.
   ci
      Count of items in the array pointed to by pt.
   */
   smvector() :
      mvector<T>(smc_ciFixed) {
   }
   smvector(smvector const & v) :
      mvector<T>(smc_ciFixed) {
      this->assign_copy(v.data(), v.size());
   }
   // If the source is using its static item array, it will be copied without allocating a dynamic
   // one; if the source is dynamic, it will be moved. Either way, this won’t throw.
   smvector(smvector && v) :
      mvector<T>(smc_ciFixed) {
      this->assign_move_dynamic_or_move_items(std::move(v));
   }
   // If the source is using its static item array, it will be copied without allocating a dynamic
   // one since it’s smaller than this object’s; if the source is dynamic, it will be moved. Either
   // way, this won’t throw.
   template <size_t t_ciStatic2>
   smvector(
      typename std::enable_if<(t_ciStatic > t_ciStatic2), smvector<T, t_ciStatic2> &&>::type v
   ) :
      mvector<T>(smc_ciFixed) {
      this->assign_move_dynamic_or_move_items(std::move(v));
   }
   smvector(mvector<T> const & v) :
      mvector<T>(smc_ciFixed) {
      this->assign_copy(v.data(), v.size());
   }
   // This can throw exceptions, but it’s allowed to since it’s not the smvector && overload.
   // This also covers smvector of different static size > t_ciStatic.
   smvector(mvector<T> && v) :
      mvector<T>(smc_ciFixed) {
      this->assign_move_dynamic_or_move_items(std::move(v));
   }
   smvector(dmvector<T> && v) :
      mvector<T>(smc_ciFixed) {
      this->assign_move(std::move(v));
   }
   template <size_t t_ci>
   explicit smvector(T const (& at)[t_ci]) :
      mvector<T>(smc_ciFixed) {
      this->assign_copy(at, t_ci);
   }
   smvector(T const * pt, size_t ci) :
      mvector<T>(smc_ciFixed) {
      this->assign_copy(pt, ci);
   }


   /** Assignment operator. R-value-reference arguments will have their contents transferred to
   *this.

   v
      Source vector.
   return
      *this;
   */
   smvector & operator=(smvector const & v) {
      this->assign_copy(v.data(), v.size());
      return *this;
   }
   // If the source is using its static item array, it will be copied without allocating a dynamic
   // one; if the source is dynamic, it will be moved. Either way, this won’t throw.
   smvector & operator=(smvector && v) {
      this->assign_move_dynamic_or_move_items(std::move(v));
      return *this;
   }
   // If the source is using its static item array, it will be copied without allocating a dynamic
   // one since it’s smaller than this object’s; if the source is dynamic, it will be moved. Either
   // way, this won’t throw.
   template <size_t t_ciStatic2>
   smvector & operator=(
      typename std::enable_if<(t_ciStatic > t_ciStatic2), smvector<T, t_ciStatic2> &&>::type v
   ) {
      this->assign_move_dynamic_or_move_items(std::move(v));
      return *this;
   }
   smvector & operator=(mvector<T> const & v) {
      this->assign_copy(v.data(), v.size());
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the smvector && overload.
   // This also covers smvector of different static size > t_ciStatic.
   smvector & operator=(mvector<T> && v) {
      this->assign_move_dynamic_or_move_items(std::move(v));
      return *this;
   }
   smvector & operator=(dmvector<T> && v) {
      this->assign_move(std::move(v));
      return *this;
   }


private:

   // This section must match exactly _raw_vextr_impl_base_with_static_item_array.

   /** See _raw_vextr_impl_base_with_static_item_array::m_ciStaticMax. */
   size_t m_ciStaticMax;
   /** See _raw_vextr_impl_base_with_static_item_array::m_at. */
   std::max_align_t m_at[ABC_ALIGNED_SIZE(sizeof(T) * smc_ciFixed)];
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_VECTOR_HXX

