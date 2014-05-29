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

#ifndef _ABC_HXX
   #error Please #include <abc.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_vector

namespace abc {

/** Thin templated wrapper for _raw_*_vextr_impl to make the interface of those two classes
consistent, so vector doesn’t need specializations.
*/
template <typename T, bool t_bCopyConstructible, bool t_bTrivial = std::is_trivial<T>::value>
class _raw_vector;

// Partial specialization for non-copyable, non-trivial types.
template <typename T>
class _raw_vector<T, false, false> :
   public _raw_complex_vextr_impl,
   public noncopyable {
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

   p1Begin
      Pointer to the start of the first source array.
   p1End
      Pointer to the end of the first source array.
   p2Begin
      Pointer to the start of the second source array.
   p2End
      Pointer to the end of the second source array.
   */
   void assign_concat_move(T * p1Begin, T * p1End, T * p2Begin, T * p2End) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      _raw_complex_vextr_impl::assign_concat(type, p1Begin, p1End, true, p2Begin, p2End, true);
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

   i
      Index at which the elements should be inserted. See abc::vector_base::translate_index() for
      allowed index values.
   p
      Pointer to the first element to add.
   ci
      Count of elements in the array pointed to by pAdd.
   */
   void insert_move(intptr_t i, T * p, size_t ci) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      _raw_complex_vextr_impl::insert(type, i, p, ci, true);
   }


   /** See _raw_complex_vextr_impl::remove_at().
   */
   void remove_at(intptr_t i) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      _raw_complex_vextr_impl::remove_at(type, i);
   }


   /** See _raw_complex_vextr_impl::remove_range().
   */
   void remove_range(intptr_t iBegin, intptr_t iEnd) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      _raw_complex_vextr_impl::remove_range(type, iBegin, iEnd);
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


   /** See _raw_complex_vextr_impl::set_size().
   */
   void set_size(size_t ci) {
      type_void_adapter type;
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      _raw_complex_vextr_impl::set_size(type, ci);
   }


protected:

   /** Constructor. See _raw_complex_vextr_impl::_raw_complex_vextr_impl().
   */
   _raw_vector(size_t ciStaticMax) :
      _raw_complex_vextr_impl(ciStaticMax) {
   }
   _raw_vector(T const * ptConstSrc, size_t ciSrc) :
      _raw_complex_vextr_impl(ptConstSrc, ciSrc) {
   }


private:

   // Hide these _raw_complex_vextr_impl methods to trigger errors as a debugging aid.

   void assign_copy(type_void_adapter const & type, T const * ptBegin, T const * ptEnd);
};

// Partial specialization for copyable, non-trivial types.
template <typename T>
class _raw_vector<T, true, false> :
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
   void assign_copy(T const * ptBegin, T const * ptEnd) {
      type_void_adapter type;
      type.set_copy_fn<T>();
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      _raw_complex_vextr_impl::assign_copy(type, ptBegin, ptEnd);
   }


   /** See _raw_complex_vextr_impl::assign_concat().
   */
   void assign_concat(
      T const * p1Begin, T const * p1End, bool bMove1,
      T const * p2Begin, T const * p2End, bool bMove2
   ) {
      type_void_adapter type;
      type.set_copy_fn<T>();
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      _raw_complex_vextr_impl::assign_concat(type, p1Begin, p1End, bMove1, p2Begin, p2End, bMove2);
   }


   /** Inserts elements at a specific position in the vector by copying them.

   i
      Index at which the elements should be inserted. See abc::vector_base::translate_index() for
      allowed index values.
   p
      Pointer to the first element to add.
   ci
      Count of elements in the array pointed to by pAdd.
   */
   void insert_copy(intptr_t i, T const * p, size_t ci) {
      type_void_adapter type;
      type.set_copy_fn<T>();
      type.set_destr_fn<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      _raw_complex_vextr_impl::insert(type, i, p, ci, false);
   }


protected:

   /** Constructor. See _raw_vector<T, false, false>::_raw_vector<T, false, false>().
   */
   _raw_vector(size_t ciStaticMax) :
      _raw_vector<T, false, false>(ciStaticMax) {
   }
   _raw_vector(T const * ptConstSrc, size_t ciSrc) :
      _raw_vector<T, false, false>(ptConstSrc, ciSrc) {
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
   void append_copy(T const * p, size_t ci) {
      _raw_trivial_vextr_impl::append(sizeof(T), p, ci);
   }


   /** Appends one or more elements. Semantically this is supposed to move them, but for trivial
   types that’s the same as copying them.

   p
      Pointer to the first element to add.
   ci
      Count of elements to add.
   */
   void append_move(T * p, size_t ci) {
      _raw_trivial_vextr_impl::append(sizeof(T), p, ci);
   }


   /** See _raw_trivial_vextr_impl::assign_copy().
   */
   void assign_copy(T const * ptBegin, T const * ptEnd) {
      _raw_trivial_vextr_impl::assign_copy(sizeof(T), ptBegin, ptEnd);
   }


   /** See _raw_trivial_vextr_impl::assign_concat().
   */
   void assign_concat(
      T const * p1Begin, T const * p1End, bool bMove1,
      T const * p2Begin, T const * p2End, bool bMove2
   ) {
      ABC_UNUSED_ARG(bMove1);
      ABC_UNUSED_ARG(bMove2);
      _raw_trivial_vextr_impl::assign_concat(sizeof(T), p1Begin, p1End, p2Begin, p2End);
   }


   /** Moves the contents of the two sources to *this.

   p1Begin
      Pointer to the start of the first source array.
   p1End
      Pointer to the end of the first source array.
   p2Begin
      Pointer to the start of the second source array.
   p2End
      Pointer to the end of the second source array.
   */
   void assign_concat_move(T * p1Begin, T * p1End, T * p2Begin, T * p2End) {
      _raw_trivial_vextr_impl::assign_concat(sizeof(T), p1Begin, p1End, p2Begin, p2End);
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

   i
      Index at which the elements should be inserted. See abc::vector_base::translate_index() for
      allowed index values.
   pt
      Pointer to the first element to add.
   ci
      Count of elements in the array pointed to by p.
   */
   void insert_copy(intptr_t i, T const * pt, size_t ci) {
      _raw_trivial_vextr_impl::insert(sizeof(T), i, pt, ci);
   }


   /** Inserts one or more elements. Semantically this is supposed to move them, but for trivial
   types that’s the same as copying them.

   i
      Index at which the elements should be inserted. See abc::vector_base::translate_index() for
      allowed index values.
   pt
      Pointer to the first element to add.
   ci
      Count of elements in the array pointed to by p.
   */
   void insert_move(intptr_t i, T const * pt, size_t ci) {
      _raw_trivial_vextr_impl::insert(sizeof(T), i, pt, ci);
   }


   /** See _raw_trivial_vextr_impl::remove_at().
   */
   void remove_at(intptr_t i) {
      _raw_trivial_vextr_impl::remove_at(sizeof(T), i);
   }


   /** See _raw_trivial_vextr_impl::remove_range().
   */
   void remove_range(intptr_t iBegin, intptr_t iEnd) {
      _raw_trivial_vextr_impl::remove_range(sizeof(T), iBegin, iEnd);
   }


   /** See _raw_trivial_vextr_impl::set_capacity().
   */
   void set_capacity(size_t ciMin, bool bPreserve) {
      _raw_trivial_vextr_impl::set_capacity(sizeof(T), ciMin, bPreserve);
   }


   /** See _raw_trivial_vextr_impl::set_size().
   */
   void set_size(size_t ci) {
      _raw_trivial_vextr_impl::set_size(sizeof(T), ci);
   }


protected:

   /** Constructor. See _raw_trivial_vextr_impl::_raw_trivial_vextr_impl().
   */
   _raw_vector(size_t ciStaticMax) :
      _raw_trivial_vextr_impl(ciStaticMax) {
   }
   _raw_vector(T const * ptConstSrc, size_t ciSrc) :
      _raw_trivial_vextr_impl(ptConstSrc, ciSrc) {
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::vector_base


namespace abc {

// Forward declarations.
template <typename T, bool t_bCopyConstructible = std::is_copy_constructible<T>::value>
class mvector;
template <typename T, bool t_bCopyConstructible = std::is_copy_constructible<T>::value>
class dmvector;

/** Base class for vectors.

See [DOC:4019 abc::*str and abc::*vector design] for implementation details for this and all the
*vector classes.
*/
template <typename T, bool t_bCopyConstructible = std::is_copy_constructible<T>::value>
class vector_base;

// Partial specialization for non-copyable types. Note that it doesn’t force t_bCopyConstructible to
// false on _raw_vector, so that vector_base<T, true> can inherit from this and still get all the
// copyable-only members of _raw_vector<T, true>.
template <typename T>
class vector_base<T, false> :
   protected _raw_vector<T, std::is_copy_constructible<T>::value>,
   public support_explicit_operator_bool<vector_base<T, std::is_copy_constructible<T>::value>> {

   /** true if T is copy constructible, or false otherwise. */
   static bool const smc_bCopyConstructible = std::is_copy_constructible<T>::value;


public:

   typedef T value_type;
   typedef T * pointer;
   typedef T const * const_pointer;
   typedef T & reference;
   typedef T const & const_reference;
   typedef size_t size_type;
   typedef ptrdiff_t difference_type;
   typedef pointer_iterator<vector_base<T, std::is_copy_constructible<T>::value>, T> iterator;
   typedef pointer_iterator<
      vector_base<T, std::is_copy_constructible<T>::value
   >, T const> const_iterator;
   typedef std::reverse_iterator<iterator> reverse_iterator;
   typedef std::reverse_iterator<const_iterator> const_reverse_iterator;


public:

   /** Element access operator.

   i
      Element index. See abc::vector_base::translate_index() for allowed index values.
   return
      Element at index i.
   */
   T const & operator[](intptr_t i) const {
      return *this->translate_index(i);
   }


   /** Returns true if the length is greater than 0.

   return
      true if the vector is not empty, or false otherwise.
   */
   explicit_operator_bool() const {
      // Use int8_t to avoid multiplying by sizeof(T) when all we need is a greater-than check.
      return _raw_vextr_impl_base::end<int8_t>() > _raw_vextr_impl_base::begin<int8_t>();
   }


   /** Equality comparison operator.

   v
      Object to compare to *this.
   return
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
   template <size_t t_ci>
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


   /** Inequality comparison operator.

   v
      Object to compare to *this.
   return
      true if *this has different count or value of elements as v, or false otherwise.
   */
   bool operator!=(vector_base const & v) const {
      return !operator==(v);
   }


   /** Returns a forward iterator set to the first element.

   return
      Forward iterator to the first element.
   */
   const_iterator begin() const {
      return const_iterator(_raw_vextr_impl_base::begin<T>());
   }


   /** Returns the maximum number of elements the array can currently hold.

   return
      Current size of the item array storage, in elements.
   */
   size_t capacity() const {
      return _raw_vector<T, smc_bCopyConstructible>::capacity();
   }


   /** Returns a const forward iterator set to the first element.

   return
      Forward iterator to the first element.
   */
   const_iterator cbegin() const {
      return const_iterator(_raw_vextr_impl_base::begin<T>());
   }


   /** Returns a const forward iterator set beyond the last element.

   return
      Forward iterator to beyond the last element.
   */
   const_iterator cend() const {
      return const_iterator(_raw_vextr_impl_base::end<T>());
   }


   /** Returns a const reverse iterator set to the last element.

   return
      Reverse iterator to the last element.
   */
   const_reverse_iterator crbegin() const {
      return const_reverse_iterator(cend());
   }


   /** Returns a const reverse iterator set to before the first element.

   return
      Reverse iterator to before the first element.
   */
   const_reverse_iterator crend() const {
      return const_reverse_iterator(cbegin());
   }


   /** Returns a forward iterator set beyond the last element.

   return
      Forward iterator to the first element.
   */
   const_iterator end() const {
      return const_iterator(_raw_vextr_impl_base::end<T>());
   }


   /** Returns the count of elements in the array.

   return
      Count of elements.
   */
   size_t size() const {
      return _raw_vextr_impl_base::size<T>();
   }


   /** Returns a reverse iterator set to the last element.

   return
      Reverse iterator to the last element.
   */
   const_reverse_iterator rbegin() const {
      return const_reverse_iterator(end());
   }


   /** Returns a reverse iterator set to before the first element.

   return
      Reverse iterator to before the first element.
   */
   const_reverse_iterator rend() const {
      return const_reverse_iterator(begin());
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
      _raw_vector<T, smc_bCopyConstructible>(ciStatic) {
   }
   vector_base(T const * pt, size_t ci) :
      _raw_vector<T, smc_bCopyConstructible>(pt, ci) {
   }


   /** See _raw_vector<T>::assign_move().

   v
      Source vector.
   */
   void assign_move(vector_base && v) {
      _raw_vector<T, smc_bCopyConstructible>::assign_move(
         static_cast<_raw_vector<T, smc_bCopyConstructible> &&>(v)
      );
   }


   /** See _raw_vector<T>::assign_move_dynamic_or_move_items().

   v
      Source vector.
   */
   void assign_move_dynamic_or_move_items(vector_base && v) {
      _raw_vector<T, smc_bCopyConstructible>::assign_move_dynamic_or_move_items(
         static_cast<_raw_vector<T, smc_bCopyConstructible> &&>(v)
      );
   }


   /** Converts a possibly negative item index into a pointer into the item array, throwing an
   exception if the result is out of bounds for the item array.

   i
      If positive, this is interpreted as a 0-based index; if negative, it’s interpreted as a
      1-based index from the end of the item array by adding this->size() to it.
   return
      Pointer to the element.
   */
   T const * translate_index(intptr_t i) const {
      return static_cast<T const *>(
         _raw_vector<T, smc_bCopyConstructible>::translate_offset(ptrdiff_t(sizeof(T)) * i)
      );
   }


   /** Converts a left-closed, right-open interval with possibly negative element indices into one
   consisting of two pointers into the item array.

   iBegin
      Left endpoint of the interval, inclusive. If positive, this is interpreted as a 0-based index;
      if negative, it’s interpreted as a 1-based index from the end of the item array by adding
      this->size() to it.
   iEnd
      Right endpoint of the interval, exclusive. If positive, this is interpreted as a 0-based
      index; if negative, it’s interpreted as a 1-based index from the end of the item array by
      adding this->size() to it.
   return
      Left-closed, right-open interval such that return.first <= i < return.second, or the empty
      interval [nullptr, nullptr) if the indices represent an empty interval after being adjusted.
   */
   std::pair<T const *, T const *> translate_range(intptr_t iBegin, intptr_t iEnd) const {
      auto range(_raw_trivial_vextr_impl::translate_byte_range(
         ptrdiff_t(sizeof(T)) * iBegin, ptrdiff_t(sizeof(T)) * iEnd
      ));
      return std::make_pair(
         static_cast<T const *>(range.first), static_cast<T const *>(range.second)
      );
   }
};

// Partial specialization for copyable types.
template <typename T>
class vector_base<T, true> :
   public vector_base<T, false> {
public:

   /** Returns a slice of the vector.

   iBegin
      Index of the first element. See abc::vector_base::translate_range() for allowed begin index
      values.
   iEnd
      Index of the last element, exclusive. See abc::vector_base::translate_range() for allowed end
      index values.
   */
   dmvector<T, true> slice(intptr_t iBegin) const {
      return slice(iBegin, this->size());
   }
   dmvector<T, true> slice(intptr_t iBegin, intptr_t iEnd) const {
      auto range(this->translate_range(iBegin, iEnd));
      return dmvector<T, true>(range.first, range.second);
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
      vector_base<T, false>(ciStatic) {
   }
   vector_base(T const * pt, size_t ci) :
      vector_base<T, false>(pt, ci) {
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
template <typename T, bool t_bCopyConstructible /*= std::is_copy_constructible<T>::value*/>
class mvector;

// Partial specialization for non-copyable types. Note that it doesn’t force t_bCopyConstructible to
// false on vector_base, so that mvector<T, true> can inherit from this and still get all the
// copyable-only members of vector_base<T, true>.
template <typename T>
class mvector<T, false> :
   public vector_base<T, std::is_copy_constructible<T>::value> {

   /** true if T is copy constructible, or false otherwise. */
   static bool const smc_bCopyConstructible = std::is_copy_constructible<T>::value;
   /** Shortcut to access the base class. */
   typedef vector_base<T, smc_bCopyConstructible> vector_base_;


public:

   /** See vector_base::iterator. */
   typedef typename vector_base_::iterator iterator;
   /** See vector_base::const_iterator. */
   typedef typename vector_base_::const_iterator const_iterator;
   /** See vector_base::reverse_iterator. */
   typedef typename vector_base_::reverse_iterator reverse_iterator;
   /** See vector_base::const_reverse_iterator. */
   typedef typename vector_base_::const_reverse_iterator const_reverse_iterator;


public:

   /** Assignment operator. R-value-reference arguments will have their contents transferred to
   *this.

   v
      Source vector.
   return
      *this.
   */
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
   mvector & operator+=(mvector && v) {
      this->append_move(v.begin().base(), v.size());
      return *this;
   }


   /** See vector_base::operator[]().
   */
   T & operator[](intptr_t i) {
      return const_cast<T &>(vector_base_::operator[](i));
   }
   T const & operator[](intptr_t i) const {
      return vector_base_::operator[](i);
   }


   /** Adds elements at the end of the vector.

   t
      Element to copy (const &) or move (&&) to the end of the vector.
   pt
      Pointer to an array of elements to copy to the end of the vector.
   ci
      Count of elements in the array pointed to by pt.
   */
   void append(typename std::remove_const<T>::type && t) {
      this->append_move(&t, 1);
   }


   /** See vector_base::begin(). Here also available in non-const overload.
   */
   iterator begin() {
      return iterator(_raw_vextr_impl_base::begin<T>());
   }
   const_iterator begin() const {
      return vector_base_::begin();
   }


   /** Removes all elements from the vector.
   */
   void clear() {
      vector_base_::clear();
   }


   /** See vector_base::end(). Here also available in non-const overload.
   */
   iterator end() {
      return iterator(_raw_vextr_impl_base::end<T>());
   }
   const_iterator end() const {
      return vector_base_::end();
   }


   /** Inserts elements at a specific position in the vector.

   i
      Index at which the element should be inserted. See abc::vector_base::translate_index() for
      allowed index values.
   it
      Iterator at which the element should be inserted.
   t
      Element to add.
   */
   void insert(intptr_t i, typename std::remove_const<T>::type && t) {
      this->insert_move(i, &t, 1);
   }
   void insert(const_iterator it, typename std::remove_const<T>::type && t) {
      this->insert_move(it - this->cbegin(), std::move(t), &t, 1);
   }


   /** See vector_base::rbegin(). Here also available in non-const overload.
   */
   reverse_iterator rbegin() {
      return reverse_iterator(iterator(_raw_vextr_impl_base::end<T>()));
   }
   const_reverse_iterator rbegin() const {
      return vector_base_::rbegin();
   }


   /** Removes a single element from the vector.

   i
      Index of the element to remove. See abc::vector_base::translate_index() for allowed index
      values.
   it
      Iterator to the element to remove.
   */
   void remove_at(intptr_t i) {
      vector_base_::remove_at(i);
   }
   void remove_at(const_iterator it) {
      vector_base_::remove_at(it - this->cbegin());
   }


   /** See vector_base::rend(). Here also available in non-const overload.
   */
   reverse_iterator rend() {
      return reverse_iterator(iterator(_raw_vextr_impl_base::begin<T>()));
   }
   const_reverse_iterator rend() const {
      return vector_base_::rend();
   }


   /** Removes a range of elements from the vector.

   iBegin
      Index of the first element. See abc::vector_base::translate_range() for allowed begin index
      values.
   itBegin
      Iterator to the first element to remove.
   iEnd
      Index of the last element, exclusive. See abc::vector_base::translate_range() for allowed end
      index values.
   itEnd
      Iterator to past the last element to remove.
   */
   void remove_range(intptr_t iBegin, intptr_t iEnd) {
      vector_base_::remove_range(iBegin, iEnd);
   }
   void remove_range(intptr_t iBegin, const_iterator itEnd) {
      vector_base_::remove_range(iBegin, itEnd - this->cbegin());
   }
   void remove_range(const_iterator itBegin, intptr_t iEnd) {
      vector_base_::remove_range(itBegin - this->cbegin(), iEnd);
   }
   void remove_range(const_iterator itBegin, const_iterator itEnd) {
      vector_base_::remove_range(itBegin - this->cbegin(), itEnd - this->cbegin());
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
      vector_base_::set_capacity(ciMin, bPreserve);
   }


   /** Changes the count of items in the vector. If the new item count is greater than the current
   one, the added elements will be left uninitialized; it’s up to the caller to make sure that these
   elements are properly constructed, or problems will arise when the destructor will attempt to
   destruct these elements.

   ci
      New vector size.
   */
   void set_size(size_t ci) {
      vector_base_::set_size(ci);
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
      vector_base_(ciStaticMax) {
   }
};

// Partial specialization for copyable types.
template <typename T>
class mvector<T, true> :
   public mvector<T, false> {
public:

   /** See vector_base::iterator. */
   typedef typename mvector<T, false>::iterator iterator;
   /** See vector_base::const_iterator. */
   typedef typename mvector<T, false>::const_iterator const_iterator;
   /** See vector_base::reverse_iterator. */
   typedef typename mvector<T, false>::reverse_iterator reverse_iterator;
   /** See vector_base::const_reverse_iterator. */
   typedef typename mvector<T, false>::const_reverse_iterator const_reverse_iterator;


public:

   /** Assignment operator. R-value-reference arguments will have their contents transferred to
   *this.

   v
      Source vector.
   return
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


   /** Concatenation-assignment operator.

   v
      Vector to concatenate.
   return
      *this.
   */
   mvector & operator+=(mvector const & v) {
      this->append_copy(v.cbegin().base(), v.size());
      return *this;
   }
   mvector & operator+=(mvector && v) {
      this->append_move(v.begin().base(), v.size());
      return *this;
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


   /** Inserts elements at a specific position in the vector.

   i
      Index at which the element should be inserted. See abc::vector_base::translate_index() for
      allowed index values.
   it
      Iterator at which the element should be inserted.
   t
      Element to add.
   pt
      Pointer to the first element to add.
   ci
      Count of elements in the array pointed to by pt.
   */
   void insert(intptr_t i, T const & t) {
      this->insert_copy(i, &t, 1);
   }
   void insert(intptr_t i, typename std::remove_const<T>::type && t) {
      this->insert_move(i, &t, 1);
   }
   void insert(intptr_t i, T const * pt, size_t ci) {
      this->insert_copy(i, pt, ci);
   }
   void insert(const_iterator it, T const & t) {
      this->insert_copy(it - this->cbegin(), &t, 1);
   }
   void insert(const_iterator it, typename std::remove_const<T>::type && t) {
      this->insert_move(it - this->cbegin(), std::move(t), &t, 1);
   }
   void insert(const_iterator it, T const * pt, size_t ci) {
      this->insert_copy(it - this->cbegin(), pt, ci);
   }


protected:

   /** See mvector<T, false>::mvector().
   */
   mvector(size_t ciStaticMax) :
      mvector<T, false>(ciStaticMax) {
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::dmvector


namespace abc {

/** Dynamically-allocated mutable vector.
*/
template <typename T, bool t_bCopyConstructible /*= std::is_copy_constructible<T>::value*/>
class dmvector;

// Partial specialization for non-copyable types.
template <typename T>
class dmvector<T, false> :
   public mvector<T, false> {
public:

   /** Constructor. The individual items or the entire source item array will be moved to *this.

   v
      Source vector.
   v1
      First source vector.
   v2
      Second source vector.
   ci
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


   /** Assignment operator. The individual items or the entire source item array will be moved to
   *this.

   v
      Source vector.
   return
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
class dmvector<T, true> :
   public mvector<T, true> {
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
   p1Begin
      Pointer to the start of the first source array, whose elements should be copied.
   p1End
      Pointer to the end of the first source array.
   p2Begin
      Pointer to the start of the second source array, whose elements should be copied.
   p2End
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
         v1.cbegin().base(), v1.cend().base(), false, v2.cbegin().base(), v2.cend().base(), false
      );
   }
   dmvector(mvector<T, true> && v1, mvector<T, true> const & v2) :
      mvector<T, true>(0) {
      this->assign_concat(
         v1.begin().base(), v1.end().base(), true, v2.cbegin().base(), v2.cend().base(), false
      );
   }
   dmvector(mvector<T, true> const & v1, mvector<T, true> && v2) :
      mvector<T, true>(0) {
      this->assign_concat(
         v1.cbegin().base(), v1.cend().base(), false, v2.begin().base(), v2.end().base(), true
      );
   }
   dmvector(mvector<T, true> && v1, mvector<T, true> && v2) :
      mvector<T, true>(0) {
      this->assign_concat_move(
         v1.begin().base(), v1.end().base(), v2.begin().base(), v2.end().base()
      );
   }
   template <size_t t_ci>
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
      this->assign_concat(pt1Begin, pt1End, false, pt2Begin, pt2End, false);
   }


   /** Assignment operator. R-value-reference arguments will have their contents transferred to
   *this.

   v
      Source vector.
   return
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
inline typename std::enable_if<std::is_copy_constructible<T>::value, abc::dmvector<T, true>>::type
operator+(abc::vector_base<T, true> const & v1, abc::vector_base<T, true> const & v2) {
   return abc::dmvector<T, true>(
      static_cast<abc::mvector<T, true> const &>(v1), static_cast<abc::mvector<T, true> const &>(v2)
   );
}
// Overloads taking an mvector r-value-reference as either or both operands; they can avoid creating
// intermediate copies of the elements from one or both source vectors.
// TODO: verify that compilers actually select these overloads whenever possible.
template <typename T>
inline typename std::enable_if<std::is_copy_constructible<T>::value, abc::dmvector<T, true>>::type
operator+(
   abc::mvector<T, true> && v1, abc::mvector<T, true> const & v2
) {
   return abc::dmvector<T, true>(std::move(v1), v2);
}
template <typename T>
inline typename std::enable_if<std::is_copy_constructible<T>::value, abc::dmvector<T, true>>::type
operator+(
   abc::mvector<T, true> const & v1, abc::mvector<T, true> && v2
) {
   return abc::dmvector<T, true>(v1, std::move(v2));
}
template <typename T, bool t_bCopyConstructible>
inline abc::dmvector<T, t_bCopyConstructible> operator+(
   abc::mvector<T, t_bCopyConstructible> && v1, abc::mvector<T, t_bCopyConstructible> && v2
) {
   return abc::dmvector<T, t_bCopyConstructible>(std::move(v1), std::move(v2));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::smvector


namespace abc {

/** mvector_-derived class, good for clients that need in-place manipulation of vectors that are
most likely to be shorter than a known small size.
*/
template <
   typename T, size_t t_ciStatic, bool t_bCopyConstructible = std::is_copy_constructible<T>::value
>
class smvector;

// Partial specialization for non-copyable types.
template <typename T, size_t t_ciStatic>
class smvector<T, t_ciStatic, false> :
   public mvector<T, false> {
protected:

   /** Actual static item array size. */
   static size_t const smc_ciFixed = _ABC__RAW_VEXTR_IMPL_BASE__ADJUST_ITEM_COUNT(t_ciStatic);


public:

   /** Constructor. The individual items or the entire source item array will be moved to *this.

   v
      Source vector.
   */
   smvector() :
      mvector<T, false>(smc_ciFixed) {
   }
   // If the source is using its static item array, it will be copied without allocating a dynamic
   // one; if the source is dynamic, it will be moved. Either way, this won’t throw.
   smvector(smvector && v) :
      mvector<T, false>(smc_ciFixed) {
      this->assign_move_dynamic_or_move_items(std::move(v));
   }
   // If the source is using its static item array, it will be copied without allocating a dynamic
   // one since it’s smaller than this object’s; if the source is dynamic, it will be moved. Either
   // way, this won’t throw.
   template <size_t t_ciStatic2>
   smvector(typename std::enable_if<
      (t_ciStatic > t_ciStatic2), smvector<T, t_ciStatic2, false> &&
   >::type v) :
      mvector<T, false>(smc_ciFixed) {
      this->assign_move_dynamic_or_move_items(std::move(v));
   }
   // This can throw exceptions, but it’s allowed to since it’s not the smvector && overload.
   // This also covers smvector of different static size > t_ciStatic.
   smvector(mvector<T, false> && v) :
      mvector<T, false>(smc_ciFixed) {
      this->assign_move_dynamic_or_move_items(std::move(v));
   }
   smvector(dmvector<T, false> && v) :
      mvector<T, false>(smc_ciFixed) {
      this->assign_move(std::move(v));
   }


   /** Assignment operator. The individual items or the entire source item array will be moved to
   *this.

   v
      Source vector.
   return
      *this.
   */
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
   smvector & operator=(typename std::enable_if<
      (t_ciStatic > t_ciStatic2), smvector<T, t_ciStatic2, false> &&
   >::type v) {
      this->assign_move_dynamic_or_move_items(std::move(v));
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the smvector && overload.
   // This also covers smvector of different static size > t_ciStatic.
   smvector & operator=(mvector<T, false> && v) {
      this->assign_move_dynamic_or_move_items(std::move(v));
      return *this;
   }
   smvector & operator=(dmvector<T, false> && v) {
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

// Partial specialization for copyable types.
template <
   typename T, size_t t_ciStatic
>
class smvector<T, t_ciStatic, true> :
   public mvector<T, true> {
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
      mvector<T, true>(smc_ciFixed) {
   }
   smvector(smvector const & v) :
      mvector<T, true>(smc_ciFixed) {
      this->assign_copy(v.cbegin().base(), v.cend().base());
   }
   // If the source is using its static item array, it will be copied without allocating a dynamic
   // one; if the source is dynamic, it will be moved. Either way, this won’t throw.
   smvector(smvector && v) :
      mvector<T, true>(smc_ciFixed) {
      this->assign_move_dynamic_or_move_items(std::move(v));
   }
   // If the source is using its static item array, it will be copied without allocating a dynamic
   // one since it’s smaller than this object’s; if the source is dynamic, it will be moved. Either
   // way, this won’t throw.
   template <size_t t_ciStatic2>
   smvector(typename std::enable_if<
      (t_ciStatic > t_ciStatic2), smvector<T, t_ciStatic2, true> &&
   >::type v) :
      mvector<T, true>(smc_ciFixed) {
      this->assign_move_dynamic_or_move_items(std::move(v));
   }
   smvector(mvector<T, true> const & v) :
      mvector<T, true>(smc_ciFixed) {
      this->assign_copy(v.cbegin().base(), v.cend().base());
   }
   // This can throw exceptions, but it’s allowed to since it’s not the smvector && overload.
   // This also covers smvector of different static size > t_ciStatic.
   smvector(mvector<T, true> && v) :
      mvector<T, true>(smc_ciFixed) {
      this->assign_move_dynamic_or_move_items(std::move(v));
   }
   smvector(dmvector<T, true> && v) :
      mvector<T, true>(smc_ciFixed) {
      this->assign_move(std::move(v));
   }
   template <size_t t_ci>
   explicit smvector(T const (& at)[t_ci]) :
      mvector<T, true>(smc_ciFixed) {
      this->assign_copy(at, at + t_ci);
   }
   smvector(T const * ptBegin, T const * ptEnd) :
      mvector<T, true>(smc_ciFixed) {
      this->assign_copy(ptBegin, ptEnd);
   }


   /** Assignment operator. R-value-reference arguments will have their contents transferred to
   *this.

   v
      Source vector.
   return
      *this.
   */
   smvector & operator=(smvector const & v) {
      this->assign_copy(v.cbegin().base(), v.cend().base());
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
   smvector & operator=(typename std::enable_if<
      (t_ciStatic > t_ciStatic2), smvector<T, t_ciStatic2, true> &&
   >::type v) {
      this->assign_move_dynamic_or_move_items(std::move(v));
      return *this;
   }
   smvector & operator=(mvector<T, true> const & v) {
      this->assign_copy(v.cbegin().base(), v.cend().base());
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the smvector && overload.
   // This also covers smvector of different static size > t_ciStatic.
   smvector & operator=(mvector<T, true> && v) {
      this->assign_move_dynamic_or_move_items(std::move(v));
      return *this;
   }
   smvector & operator=(dmvector<T, true> && v) {
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

