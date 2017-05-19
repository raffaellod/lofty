/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_COLLECTIONS_VECTOR_HXX
#define _LOFTY_COLLECTIONS_VECTOR_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/collections/_pvt/complex_vextr_impl.hxx>
#include <lofty/type_void_adapter.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

/*! Thin templated wrapper for *_vextr_impl to make the interface of those two classes consistent, so vector
doesn’t need specializations. */
template <
   typename T,
   bool copy_constructible = _std::is_copy_constructible<T>::value,
   bool trivial = _std::is_trivial<T>::value
>
class vector_impl;

// Partial specialization for non-copyable, non-trivial types.
template <typename T>
class vector_impl<T, false, false> : public complex_vextr_impl, public noncopyable {
public:
   //! Destructor.
   ~vector_impl() {
      type_void_adapter type;
      type.set_destruct<T>();
      destruct_items(type);
   }

   /*! Ensures that the vector has at least new_capacity_min of actual item space. If this causes *this to
   switch to using a different item array, any elements in the current one will be destructed unless preserve
   == true, which will cause them to be moved to the new item array.

   @param new_capacity_min
      Minimum count of elements requested.
   @param preserve
      If true, the previous contents of the vector will be preserved even if the reallocation causes the
      vector to switch to a different item array.
   */
   void set_capacity(std::size_t new_capacity_min, bool preserve) {
      type_void_adapter type;
      type.set_destruct<T>();
      type.set_move_construct<T>();
      complex_vextr_impl::set_capacity(type, sizeof(T) * new_capacity_min, preserve);
   }

   /*! Changes the count of items in the vector. If the new item count is greater than the current one, the
   added elements will be left uninitialized; it’s up to the caller to make sure that these elements are
   properly constructed, or problems will arise when the destructor will attempt to destruct these elements.

   TODO: destruct in complex_vextr_impl::set_size() any elements being taken out, and default-construct the
   newly-created elements here.

   @param new_size
      New vector size.
   */
   void set_size(std::size_t new_size) {
      type_void_adapter type;
      type.set_destruct<T>();
      type.set_move_construct<T>();
      complex_vextr_impl::set_size(type, sizeof(T) * new_size);
   }

protected:
   //! See complex_vextr_impl::assign_move_desc_or_move_items().
   void assign_move_desc_or_move_items(complex_vextr_impl && src) {
      type_void_adapter type;
      type.set_destruct<T>();
      type.set_move_construct<T>();
      complex_vextr_impl::assign_move_desc_or_move_items(type, _std::move(src));
   }

   /*! Inserts elements at a specific position in the vector by moving them.

   @param offset
      Pointer to where the elements should be inserted.
   @param src
      Pointer to the first element to insert.
   @param src_size
      Count of elements in the array pointed to by src.
   */
   void insert_move(T const * offset, T * src, std::size_t src_size) {
      type_void_adapter type;
      type.set_destruct<T>();
      type.set_move_construct<T>();
      type.set_size<T>();
      complex_vextr_impl::insert(
         type,
         static_cast<std::size_t>(
            reinterpret_cast<std::int8_t const *>(offset) - vextr_impl_base::begin<std::int8_t>()
         ),
         src, sizeof(T) * src_size, true
      );
   }

   /*! Removes a slice from the vector.

   @param remove_begin
      Pointer to the first element to remove.
   @param remove_end
      Pointer to beyond the last element to remove.
   */
   void remove(T const * remove_begin, T const * remove_end) {
      type_void_adapter type;
      type.set_destruct<T>();
      type.set_move_construct<T>();
      type.set_size<T>();
      complex_vextr_impl::remove(
         type,
         static_cast<std::size_t>(
            reinterpret_cast<std::int8_t const *>(remove_begin) - vextr_impl_base::begin<std::int8_t>()
         ),
         reinterpret_cast<std::size_t>(remove_end) - reinterpret_cast<std::size_t>(remove_begin)
      );
   }

protected:
   //! See complex_vextr_impl::complex_vextr_impl().
   vector_impl(std::size_t embedded_byte_capacity) :
      complex_vextr_impl(embedded_byte_capacity) {
   }

   //! See complex_vextr_impl::complex_vextr_impl().
   vector_impl(T const * const_src, std::size_t const_src_size) :
      complex_vextr_impl(const_src, const_src_size) {
   }
};

// Partial specialization for copyable, non-trivial types.
template <typename T>
class vector_impl<T, true, false> : public vector_impl<T, false, false> {
protected:
   //! See complex_vextr_impl::assign_copy().
   void assign_copy(T const * src_begin, T const * src_end) {
      type_void_adapter type;
      type.set_copy_construct<T>();
      type.set_destruct<T>();
      type.set_move_construct<T>();
      complex_vextr_impl::assign_copy(type, src_begin, src_end);
   }

   //! See complex_vextr_impl::assign_concat().
   void assign_concat(
      T const * src1_begin, T const * src1_end, T const * src2_begin, T const * src2_end, std::uint8_t move
   ) {
      type_void_adapter type;
      type.set_copy_construct<T>();
      type.set_destruct<T>();
      type.set_move_construct<T>();
      complex_vextr_impl::assign_concat(type, src1_begin, src1_end, src2_begin, src2_end, move);
   }

   /*! Inserts elements at a specific position in the vector by copying them.

   @param offset
      Pointer to where the elements should be inserted.
   @param src
      Pointer to the first element to insert.
   @param src_size
      Count of elements in the array pointed to by src.
   */
   void insert_copy(T const * offset, T const * src, std::size_t src_size) {
      type_void_adapter type;
      type.set_copy_construct<T>();
      type.set_destruct<T>();
      type.set_move_construct<T>();
      type.set_size<T>();
      complex_vextr_impl::insert(
         type,
         static_cast<std::size_t>(
            reinterpret_cast<std::int8_t const *>(offset) - vextr_impl_base::begin<std::int8_t>()
         ),
         src, sizeof(T) * src_size, false
      );
   }

protected:
   //! See vector_impl<T, false, false>::vector_impl<T, false, false>().
   vector_impl(std::size_t embedded_byte_capacity) :
      vector_impl<T, false, false>(embedded_byte_capacity) {
   }

   //! See vector_impl<T, false, false>::vector_impl<T, false, false>().
   vector_impl(std::size_t embedded_byte_capacity, T const * const_src, std::size_t const_src_size) :
      vector_impl<T, false, false>(embedded_byte_capacity, const_src, const_src_size) {
   }
};

/* Partial specialization for trivial (and copyable) types. Methods here ignore the move argument for the
individual elements, because move semantics don’t apply (trivial values are always copied). */
template <typename T>
class vector_impl<T, true, true> : public trivial_vextr_impl {
public:
   /*! Ensures that the vector has at least new_capacity_min of actual item space. If this causes *this to
   switch to using a different item array, any elements in the current one will be destructed unless preserve
   == true, which will cause them to be moved to the new item array.

   @param new_capacity_min
      Minimum count of elements requested.
   @param preserve
      If true, the previous contents of the vector will be preserved even if the reallocation causes the
      vector to switch to a different item array.
   */
   void set_capacity(std::size_t new_capacity_min, bool preserve) {
      trivial_vextr_impl::set_capacity(sizeof(T) * new_capacity_min, preserve);
   }

   /*! Changes the count of items in the vector. If the new item count is greater than the current one, the
   added elements will be left uninitialized; it’s up to the caller to make sure that these elements are
   properly constructed, or problems will arise when the destructor will attempt to destruct these elements.

   TODO: maybe default-construct the newly-created elements here for consistency with the non-trivial
   specialization?

   @param new_size
      New vector size.
   */
   void set_size(std::size_t new_size) {
      trivial_vextr_impl::set_size(sizeof(T) * new_size);
   }

protected:
   //! See trivial_vextr_impl::assign_copy().
   void assign_copy(T const * src_begin, T const * src_end) {
      trivial_vextr_impl::assign_copy(src_begin, src_end);
   }

   //! See trivial_vextr_impl::assign_concat().
   void assign_concat(
      T const * src1_begin, T const * src1_end, T const * src2_begin, T const * src2_end, std::uint8_t move
   ) {
      LOFTY_UNUSED_ARG(move);
      trivial_vextr_impl::assign_concat(src1_begin, src1_end, src2_begin, src2_end);
   }

   //! See trivial_vextr_impl::assign_move_desc_or_move_items().
   void assign_move_desc_or_move_items(trivial_vextr_impl && rtvi) {
      trivial_vextr_impl::assign_move_desc_or_move_items(_std::move(rtvi));
   }

   /*! Inserts elements at a specific position in the vector.

   @param offset
      Pointer to where the elements should be inserted.
   @param src
      Pointer to the first element to insert.
   @param src_size
      Count of elements in the array pointed to by src.
   */
   void insert_copy(T const * offset, T const * src, std::size_t src_size) {
      trivial_vextr_impl::insert_remove(
         static_cast<std::size_t>(
            reinterpret_cast<std::int8_t const *>(offset) - vextr_impl_base::begin<std::int8_t>()
         ),
         src, sizeof(T) * src_size, 0
      );
   }

   /*! Inserts one or more elements. Semantically this is supposed to move them, but for trivial types that’s
   the same as copying them.

   @param offset
      Pointer to where the elements should be inserted.
   @param src
      Pointer to the first element to insert.
   @param src_size
      Count of elements in the array pointed to by src.
   */
   void insert_move(T const * offset, T * src, std::size_t src_size) {
      trivial_vextr_impl::insert_remove(
         static_cast<std::size_t>(
            reinterpret_cast<std::int8_t const *>(offset) - vextr_impl_base::begin<std::int8_t>()
         ),
         src, sizeof(T) * src_size, 0
      );
   }

   /*! Removes elements from the vector.

   @param remove_begin
      Pointer to the first element to remove.
   @param remove_end
      Pointer to beyond the last element to remove.
   */
   void remove(T const * remove_begin, T const * remove_end) {
      trivial_vextr_impl::insert_remove(
         static_cast<std::size_t>(
            reinterpret_cast<std::int8_t const *>(remove_begin) - vextr_impl_base::begin<std::int8_t>()
         ),
         nullptr, 0,
         reinterpret_cast<std::size_t>(remove_end) - reinterpret_cast<std::size_t>(remove_begin)
      );
   }

protected:
   //! See trivial_vextr_impl::trivial_vextr_impl().
   vector_impl(std::size_t embedded_byte_capacity) :
      trivial_vextr_impl(embedded_byte_capacity) {
   }

   //! See trivial_vextr_impl::trivial_vextr_impl().
   vector_impl(std::size_t embedded_byte_capacity, T const * const_src, std::size_t const_src_size) :
      trivial_vextr_impl(embedded_byte_capacity, const_src, const_src_size) {
   }
};

}}} //namespace lofty::collections::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

//! Const iterator for vector elements.
template <typename T>
class vector_const_iterator {
private:
   template <typename T2, std::size_t embedded_capacity>
   friend class collections::vector;

protected:
   typedef vector<T, 0> vector_0;

public:
   typedef T const value_type;
   typedef T const * pointer;
   typedef T const & reference;
   typedef std::ptrdiff_t difference_type;
   typedef _std::random_access_iterator_tag iterator_category;

public:
   //! Default constructor.
   /*constexpr*/ vector_const_iterator() :
      owner_vec(nullptr),
      ptr(nullptr) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   vector_const_iterator(vector_const_iterator const & src) :
      owner_vec(src.owner_vec),
      ptr(src.ptr) {
   }

   /*! Dereferencing operator.

   @return
      Reference to the current item.
   */
   T const & operator*() const {
      return *vector_0::validate_pointer(owner_vec, ptr, false);
   }

   /*! Dereferencing member access operator.

   @return
      Pointer to the current item.
   */
   T const * operator->() const {
      return vector_0::validate_pointer(owner_vec, ptr, false);
   }

   /*! Element access operator.

   @param i
      Element index.
   @return
      Reference to the specified element.
   */
   T const & operator[](std::ptrdiff_t i) const {
      return *vector_0::validate_pointer(owner_vec, ptr + i, false);
   }

   /*! Addition-assignment operator.

   @param i
      Count of positions by which to advance the iterator.
   @return
      *this after it’s moved forward by i positions.
   */
   vector_const_iterator & operator+=(std::ptrdiff_t i) {
      ptr = *vector_0::validate_pointer(owner_vec, ptr + i, true);
      return *this;
   }

   /*! Subtraction-assignment operator.

   @param i
      Count of positions by which to rewind the iterator.
   @return
      *this after it’s moved backwards by i positions.
   */
   vector_const_iterator & operator-=(std::ptrdiff_t i) {
      ptr = *vector_0::validate_pointer(owner_vec, ptr - i, true);
      return *this;
   }

   /*! Addition operator.

   @param i
      Count of positions by which to advance the iterator.
   @return
      Iterator that’s i items ahead of *this.
   */
   vector_const_iterator operator+(std::ptrdiff_t i) const {
      return vector_const_iterator(owner_vec, vector_0::validate_pointer(owner_vec, ptr + i, true));
   }

   /*! Subtraction operator.

   @param i
      Count of positions by which to rewind the iterator.
   @return
      Iterator that’s i items behind *this.
   */
   vector_const_iterator operator-(std::ptrdiff_t i) const {
      return vector_const_iterator(owner_vec, vector_0::validate_pointer(owner_vec, ptr - i, true));
   }

   /*! Difference operator.

   @param right
      Iterator from which to calculate the distance.
   @return
      Distance between *this and right.
   */
   std::ptrdiff_t operator-(vector_const_iterator right) const {
      return ptr - right.ptr;
   }

   /*! Preincrement operator.

   @return
      *this after it’s moved to the value following the one currently pointed to.
   */
   vector_const_iterator & operator++() {
      ptr = vector_0::validate_pointer(owner_vec, ptr + 1, true);
      return *this;
   }

   /*! Postincrement operator.

   @return
      Iterator pointing to the value following the one pointed to by this iterator.
   */
   vector_const_iterator operator++(int) {
      T const * old_ptr = ptr;
      ptr = vector_0::validate_pointer(owner_vec, ptr + 1, true);
      return vector_const_iterator(owner_vec, old_ptr);
   }

   /*! Predecrement operator.

   @return
      *this after it’s moved to the value preceding the one currently pointed to.
   */
   vector_const_iterator & operator--() {
      ptr = vector_0::validate_pointer(owner_vec, ptr - 1, true);
      return *this;
   }

   /*! Postdecrement operator.

   @return
      Iterator pointing to the value preceding the one pointed to by this iterator.
   */
   vector_const_iterator operator--(int) {
      T const * old_ptr = ptr;
      ptr = vector_0::validate_pointer(owner_vec, ptr - 1, true);
      return vector_const_iterator(owner_vec, old_ptr);
   }

// Relational operators.
#define LOFTY_RELOP_IMPL(op) \
   bool operator op(vector_const_iterator const & right) const { \
      return ptr op right.ptr; \
   }
LOFTY_RELOP_IMPL(==)
LOFTY_RELOP_IMPL(!=)
LOFTY_RELOP_IMPL(>)
LOFTY_RELOP_IMPL(>=)
LOFTY_RELOP_IMPL(<)
LOFTY_RELOP_IMPL(<=)
#undef LOFTY_RELOP_IMPL

protected:
   /*! Constructor.

   @param owner_vec_
      Pointer to the vector containing *ptr_.
   @param ptr_
      Pointer to set the iterator to.
   */
   explicit vector_const_iterator(vector_0 const * owner_vec_, T const * ptr_) :
      owner_vec(owner_vec_),
      ptr(ptr_) {
   }

protected:
   //! Pointer to the vector containing the current element.
   vector<T, 0> const * owner_vec;
   //! Pointer to the current element.
   T const * ptr;
};

//! Non-const iterator for vector elements.
template <typename T>
class vector_iterator : public vector_const_iterator<T> {
private:
   template <typename T2, std::size_t embedded_capacity>
   friend class collections::vector;
   typedef vector_const_iterator<T> vci;

public:
   typedef T value_type;
   typedef T * pointer;
   typedef T & reference;

public:
   //! Default constructor.
   /*constexpr*/ vector_iterator() {
   }

   //! See vector_const_iterator::operator*().
   T & operator*() const {
      return const_cast<T &>(vci::operator*());
   }

   //! See vector_const_iterator::operator->().
   T * operator->() const {
      return const_cast<T *>(vci::operator->());
   }

   //! See vector_const_iterator::operator[]().
   T & operator[](std::ptrdiff_t i) const {
      return const_cast<T &>(vci::operator[](i));
   }

   //! See vector_const_iterator::operator+=().
   vector_iterator & operator+=(std::ptrdiff_t i) {
      vci::operator+=(i);
      return *this;
   }

   //! See vector_const_iterator::operator-=().
   vector_iterator & operator-=(std::ptrdiff_t i) {
      vci::operator-=(i);
      return *this;
   }

   //! See vector_const_iterator::operator+().
   vector_iterator operator+(std::ptrdiff_t i) const {
      return vci::operator+(i);
   }

   using vci::operator-;

   //! See vector_const_iterator::operator-().
   vector_iterator operator-(std::ptrdiff_t i) const {
      return vci::operator-(i);
   }

   //! See vector_const_iterator::operator++().
   vector_iterator & operator++() {
      vci::operator++();
      return *this;
   }

   //! See vector_const_iterator::operator++().
   vector_iterator operator++(int) {
      return vci::operator++(0);
   }

   //! See vector_const_iterator::operator--().
   vector_iterator & operator--() {
      vci::operator--();
      return *this;
   }

   //! See vector_const_iterator::operator--().
   vector_iterator operator--(int) {
      return vci::operator--(0);
   }

protected:
   /*! Constructor.

   @param owner_vec_
      Pointer to the vector containing *pt.
   @param ptr_
      Pointer to set the iterator to.
   */
   explicit vector_iterator(typename vci::vector_0 const * owner_vec_, T const * ptr_) :
      vci(owner_vec_, ptr_) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   vector_iterator(vci const & src) :
      vci(src) {
   }
};

}}} //namespace lofty::collections::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections {

// Partial specialization with no embedded array (default).
template <typename T>
class vector<T, 0> :
   public _pvt::vector_impl<T>,
   public support_explicit_operator_bool<vector<T, 0>> {
private:
   template <typename T2>
   friend class _pvt::vector_iterator;
   template <typename T2>
   friend class _pvt::vector_const_iterator;

   //! true if T is copy constructible, or false otherwise.
   static bool const copy_constructible = _std::is_copy_constructible<T>::value;
   typedef _pvt::vector_impl<T> vector_impl;

public:
   typedef T value_type;
   typedef T * pointer;
   typedef T const * const_pointer;
   typedef T & reference;
   typedef T const & const_reference;
   typedef std::size_t size_type;
   typedef std::ptrdiff_t difference_type;
   typedef _pvt::vector_iterator<T> iterator;
   typedef _pvt::vector_const_iterator<T> const_iterator;
   typedef _std::reverse_iterator<iterator> reverse_iterator;
   typedef _std::reverse_iterator<const_iterator> const_reverse_iterator;

public:
   //! Default constructor.
   vector() :
      vector_impl(0) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   vector(vector && src) :
      vector_impl(0) {
      vector_impl::assign_move_desc_or_move_items(_std::move(src));
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   vector(vector const & src) :
      vector_impl(0) {
      vector_impl::assign_copy(src.data(), src.data_end());
   }

   /*! Constructor that concatenates two vectors, copying elements from the first and moving elements from the
   second.

   @param src1
      First source vector.
   @param src2
      Second source vector.
   */
   template <std::size_t src1_embedded_capacity, std::size_t src2_embedded_capacity>
   vector(vector<T, src1_embedded_capacity> const & src1, vector<T, src2_embedded_capacity> && src2) :
      vector_impl(0) {
      vector_impl::assign_concat(src1.data(), src1.data_end(), src2.data(), src2.data_end(), 2);
   }

   /*! Constructor that concatenates two vectors, copying elements from both.

   @param src1
      First source vector.
   @param src2
      Second source vector.
   */
   template <std::size_t src1_embedded_capacity, std::size_t src2_embedded_capacity>
   vector(vector<T, src1_embedded_capacity> const & src1, vector<T, src2_embedded_capacity> const & src2) :
      vector_impl(0) {
      vector_impl::assign_concat(src1.data(), src1.data_end(), src2.data(), src2.data_end(), 0);
   }

   /*! Constructor that copies elements from a C array.

   @param src
      Source array whose elements should be copied.
   */
   template <std::size_t src_size>
   explicit vector(T const (& src)[src_size]) :
      vector_impl(0) {
      vector_impl::assign_copy(src, src + src_size);
   }

   /*! Constructor that copies elements from an array.

   @param src_begin
      Pointer to the beginning of the source array.
   @param src_end
      Pointer to the end of the source array.
   */
   vector(T const * src_begin, T const * src_end) :
      vector_impl(0) {
      vector_impl::assign_copy(src_begin, src_end);
   }

   /*! Constructor that concatenates two arrays, copying elements from both.

   @param src1_begin
      Pointer to the start of the first source array.
   @param src1_end
      Pointer to the end of the first source array.
   @param src2_begin
      Pointer to the start of the second source array.
   @param src2_end
      Pointer to the end of the second source array.
   */
   vector(T const * src1_begin, T const * src1_end, T const * src2_begin, T const * src2_end) :
      vector_impl(0) {
      vector_impl::assign_concat(src1_begin, src1_end, src2_begin, src2_end, 0);
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   vector & operator=(vector && src) {
      vector_impl::assign_move_desc_or_move_items(_std::move(src));
      return *this;
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   vector & operator=(vector const & src) {
      vector_impl::assign_copy(src.data(), src.data_end());
      return *this;
   }

   /*! Concatenation-assignment operator that moves elements from the source.

   @param src
      Vector to concatenate.
   @return
      *this.
   */
   template <std::size_t src_embedded_capacity>
   vector & operator+=(vector<T, src_embedded_capacity> && src) {
      vector_impl::insert_move(data_end(), src.data(), src.size());
      return *this;
   }

   /*! Concatenation-assignment operator that copies elements from the source.

   @param src
      Vector to concatenate.
   @return
      *this.
   */
   template <std::size_t src_embedded_capacity>
   vector & operator+=(vector<T, src_embedded_capacity> const & src) {
      vector_impl::insert_copy(data_end(), src.data(), src.size());
      return *this;
   }

   /*! Element access operator.

   @param i
      Element index.
   @return
      Reference to the element at index i.
   */
   T & operator[](std::ptrdiff_t i) {
      return *validate_pointer(data() + i, false);
   }

   /*! Const element access operator.

   @param i
      Element index.
   @return
      Const reference to the element at index i.
   */
   T const & operator[](std::ptrdiff_t i) const {
      return const_cast<vector *>(this)->operator[](i);
   }

   /*! Boolean evaluation operator.

   @return
      true if the vector is not empty, or false otherwise.
   */
   LOFTY_EXPLICIT_OPERATOR_BOOL() const {
      // Use std::int8_t to avoid multiplying by sizeof(T) when all we need is a greater-than check.
      return vector_impl::template end<std::int8_t>() > vector_impl::template begin<std::int8_t>();
   }

   /*! Returns a reference to the last element.

   @return
      Reference to the last element.
   */
   T & back() {
      return *validate_pointer(data_end() - 1, false);
   }

   /*! Returns a const reference to the last element.

   @return
      Const reference to the last element.
   */
   T const & back() const {
      return const_cast<vector *>(this)->back();
   }

   /*! Returns an iterator set to the first element.

   @return
      Iterator to the first element.
   */
   iterator begin() {
      return iterator(this, data());
   }

   /*! Returns a const iterator set to the first element.

   @return
      Const iterator to the first element.
   */
   const_iterator begin() const {
      return const_cast<vector *>(this)->begin();
   }

   /*! Returns the maximum number of elements the array can currently hold.

   @return
      Current size of the item array storage, in elements.
   */
   std::size_t capacity() const {
      return vector_impl::template capacity<T>();
   }

   /*! Returns a const forward iterator set to the first element.

   @return
      Forward iterator to the first element.
   */
   const_iterator cbegin() const {
      return const_cast<vector *>(this)->begin();
   }

   /*! Returns a const forward iterator set beyond the last element.

   @return
      Forward iterator to beyond the last element.
   */
   const_iterator cend() const {
      return const_cast<vector *>(this)->end();
   }

   //! Removes all elements from the vector.
   void clear() {
      static_cast<vector_impl *>(this)->~vector_impl();
      vector_impl::assign_empty();
   }

   /*! Returns a const reverse iterator set to the last element.

   @return
      Reverse iterator to the last element.
   */
   const_reverse_iterator crbegin() const {
      return const_cast<vector *>(this)->rbegin();
   }

   /*! Returns a const reverse iterator set to before the first element.

   @return
      Reverse iterator to before the first element.
   */
   const_reverse_iterator crend() const {
      return const_cast<vector *>(this)->rend();
   }

   /*! Returns a pointer to the element array.

   @return
      Pointer to the element array.
   */
   T * data() {
      return vector_impl::template begin<T>();
   }

   /*! Returns a const pointer to the element array.

   @return
      Const pointer to the element array.
   */
   T const * data() const {
      return const_cast<vector *>(this)->data();
   }

   /*! Returns a pointer to the end of the element array.

   @return
      Pointer to the end of the element array.
   */
   T * data_end() {
      return vector_impl::template end<T>();
   }

   /*! Returns a const pointer to the end of the element array.

   @return
      Const pointer to the end of the element array.
   */
   T const * data_end() const {
      return const_cast<vector *>(this)->data_end();
   }

   /*! Returns an iterator set beyond the last element.

   @return
      Iterator to the first element.
   */
   iterator end() {
      return iterator(this, data_end());
   }

   /*! Returns a const iterator set beyond the last element.

   @return
      Const iterator to the first element.
   */
   const_iterator end() const {
      return const_cast<vector *>(this)->end();
   }

   /*! Returns a reference to the first element.

   @return
      Reference to the first element.
   */
   T & front() {
      return *validate_pointer(data(), false);
   }

   /*! Returns a const reference to the first element.

   @return
      Const reference to the first element.
   */
   T const & front() const {
      return const_cast<vector *>(this)->front();
   }

   /*! Inserts elements at a specific position in the vector.

   @param offset
      Iterator at which the element should be inserted.
   @param t
      Element to insert.
   */
   void insert(const_iterator offset, typename _std::remove_const<T>::type && t) {
      validate_pointer(offset.ptr, true);
      vector_impl::insert_move(offset.ptr, &t, 1);
   }

   /*! Inserts an element at a specific position in the vector.

   @param offset
      Iterator at which the element should be inserted.
   @param t
      Element to insert.
   */
   void insert(const_iterator offset, T const & t) {
      validate_pointer(offset.ptr, true);
      vector_impl::insert_copy(offset.ptr, &t, 1);
   }

   /*! Inserts elements at a specific position in the vector.

   @param offset
      Iterator at which the element should be inserted.
   @param src
      Pointer to the first element to insert.
   @param src_size
      Count of elements in the array pointed to by src.
   */
   void insert(const_iterator offset, T const * src, std::size_t src_size) {
      validate_pointer(offset.ptr, true);
      vector_impl::insert_copy(offset.ptr, src, src_size);
   }

   /*! Removes and returns the last element in the vector.

   @return
      Former last element of the vector.
   */
   T pop_back() {
      T * back_ptr = &back();
      T back_t(_std::move(*back_ptr));
      vector_impl::remove(back_ptr, back_ptr + 1);
      return _std::move(back_t);
   }

   /*! Adds an element at the end of the vector.

   @param t
      Element to add.
   */
   void push_back(typename _std::remove_const<T>::type && t) {
      vector_impl::insert_move(data_end(), &t, 1);
   }

   /*! Adds an element at the end of the vector.

   @param t
      Element to copy to the end of the vector.
   */
   void push_back(T const & t) {
      vector_impl::insert_copy(data_end(), &t, 1);
   }

   /*! Adds elements at the end of the vector.

   @param src
      Pointer to an array of elements to copy to the end of the vector.
   @param src_size
      Count of elements in the array pointed to by src.
   */
   void push_back(T const * src, std::size_t src_size) {
      vector_impl::insert_copy(data_end(), src, src_size);
   }

   /*! Returns a reverse iterator set to the last element.

   @return
      Reverse iterator to the last element.
   */
   reverse_iterator rbegin() {
      return reverse_iterator(iterator(this, data_end()));
   }

   /*! Returns a const reverse iterator set to the last element.

   @return
      Const reverse iterator to the last element.
   */
   const_reverse_iterator rbegin() const {
      return const_cast<vector *>(this)->rbegin();
   }

   /*! Removes a single element from the vector.

   @param itr
      Iterator to the element to remove.
   */
   void remove_at(const_iterator itr) {
      validate_pointer(itr.ptr, false);
      vector_impl::remove(itr.ptr, itr.ptr + 1);
   }

   /*! Returns a reverse iterator set to before the first element.

   @return
      Reverse iterator to before the first element.
   */
   reverse_iterator rend() {
      return reverse_iterator(iterator(this, data()));
   }

   /*! Returns a const reverse iterator set to before the first element.

   @return
      Const reverse iterator to before the first element.
   */
   const_reverse_iterator rend() const {
      return const_cast<vector *>(this)->rend();
   }

   //! Resizes the vector so that it only takes up as much memory as strictly necessary.
   void shrink_to_fit() {
      // TODO: implement this.
   }

   /*! Returns the count of elements in the array.

   @return
      Count of elements.
   */
   std::size_t size() const {
      return vector_impl::template size<T>();
   }

   /*! Returns a slice of the vector up to its end.

   @param slice_begin
      Iterator to the first element to return.
   @return
      Vector slice.
   */
   vector slice(const_iterator slice_begin) const {
      return vector(slice_begin.ptr, data_end());
   }

   /*! Returns a slice of the vector.

   @param slice_begin
      Iterator to the first element to return.
   @param slice_end
      Iterator to the element after the last one to return.
   @return
      Vector slice.
   */
   vector slice(const_iterator slice_begin, const_iterator slice_end) const {
      return vector(slice_begin.ptr, slice_end.ptr);
   }

protected:
   /*! Constructor for subclasses with an embedded item array.

   @param embedded_byte_capacity
      Size of the embedded character array, in bytes.
   */
   vector(std::size_t embedded_byte_capacity) :
      vector_impl(embedded_byte_capacity) {
   }

   /*! Move constructor.

   @param embedded_byte_capacity
      Size of the embedded character array, in bytes.
   @param src
      Source object.
   */
   vector(std::size_t embedded_byte_capacity, vector && src) :
      vector_impl(embedded_byte_capacity) {
      vector_impl::assign_move_desc_or_move_items(_std::move(src));
   }

   /*! Copy constructor for subclasses with an embedded item array.

   @param embedded_byte_capacity
      Size of the embedded character array, in bytes.
   @param src
      Source object.
   */
   vector(std::size_t embedded_byte_capacity, vector const & src) :
      vector_impl(embedded_byte_capacity) {
      vector_impl::assign_copy(src.data(), src.data_end());
   }

   /*! Copy constructor from C arrays for subclasses with an embedded item array.

   @param embedded_byte_capacity
      Size of the embedded character array, in bytes.
   @param src
      Pointer to a C array that will be copied to the vector.
   @param src_size
      Count of items in the array pointed to by src.
   */
   vector(std::size_t embedded_byte_capacity, T const * src, std::size_t src_size) :
      vector_impl(embedded_byte_capacity) {
      vector_impl::assign_copy(src, src + src_size);
   }

   /*! Throws a collections::out_of_range if a pointer is not within bounds.

   @param p
      Pointer to validate.
   @param allow_end
      If true, p == data_end() is allowed; if false, it’s not.
   @return
      p.
   */
   template <typename TPtr>
   TPtr * validate_pointer(TPtr * p, bool allow_end) const {
      vector_impl::validate_pointer(p, allow_end);
      return p;
   }

   /*! Throws a collections::out_of_range if a pointer is not within bounds of *this_ptr.

   This overload is static so that it will validate that this_ptr is not nullptr before dereferencing it.

   @param this_ptr
      this.
   @param p
      Pointer to validate.
   @param allow_end
      If true, p == this_ptr->data_end() is allowed; if false, it’s not.
   @return
      p.
   */
   template <typename TPtr>
   static TPtr * validate_pointer(vector const * this_ptr, TPtr * p, bool allow_end) {
      vector_impl::validate_pointer(this_ptr, p, allow_end);
      return p;
   }
};

/*! Concatenation operator.

@param left
   Left operand.
@param right
   Right operand.
@return
   Vector resulting from the concatenation of left and right.
*/
template <typename T, std::size_t left_embedded_capacity, std::size_t right_embedded_capacity>
inline vector<T, left_embedded_capacity> operator+(
   vector<T, left_embedded_capacity> && left, vector<T, right_embedded_capacity> && right
) {
   left += right;
   return _std::move(left);
}

/*! Concatenation operator.

@param left
   Left operand.
@param right
   Right operand.
@return
   Vector resulting from the concatenation of left and right.
*/
template <typename T, std::size_t left_embedded_capacity, std::size_t right_embedded_capacity>
inline typename _std::enable_if<
   _std::is_copy_constructible<T>::value, vector<T, left_embedded_capacity>
>::type operator+(
   vector<T, left_embedded_capacity> && left, vector<T, right_embedded_capacity> const & right
) {
   left += right;
   return _std::move(left);
}

/*! Concatenation operator that copies elements from the first and moving elements from the second.

@param left
   Left operand.
@param right
   Right operand.
@return
   Vector resulting from the concatenation of left and right.
*/
template <typename T, std::size_t left_embedded_capacity, std::size_t right_embedded_capacity>
inline typename _std::enable_if<_std::is_copy_constructible<T>::value, vector<T>>::type operator+(
   vector<T, left_embedded_capacity> const & left, vector<T, right_embedded_capacity> && right
) {
   return vector<T>(left, _std::move(right));
}

/*! Concatenation operator.

@param left
   Left operand.
@param right
   Right operand.
@return
   Vector resulting from the concatenation of left and right.
*/
template <typename T, std::size_t left_embedded_capacity, std::size_t right_embedded_capacity>
inline typename _std::enable_if<_std::is_copy_constructible<T>::value, vector<T>>::type operator+(
   vector<T, left_embedded_capacity> const & left, vector<T, right_embedded_capacity> const & right
) {
   return vector<T>(left, right);
}

// General definition, with embedded item array.
template <typename T, std::size_t embedded_capacity>
class vector :
   private vector<T, 0>,
   private _pvt::vextr_prefixed_array<T, embedded_capacity> {
private:
   using _pvt::vextr_prefixed_array<T, embedded_capacity>::embedded_byte_capacity;
   //! true if T is copy constructible, or false otherwise.
   static bool const copy_constructible = _std::is_copy_constructible<T>::value;
   typedef vector<T, 0> vector_0;

public:
   typedef typename vector_0::value_type             value_type;
   typedef typename vector_0::pointer                pointer;
   typedef typename vector_0::const_pointer          const_pointer;
   typedef typename vector_0::reference              reference;
   typedef typename vector_0::const_reference        const_reference;
   typedef typename vector_0::size_type              size_type;
   typedef typename vector_0::difference_type        difference_type;
   typedef typename vector_0::iterator               iterator;
   typedef typename vector_0::const_iterator         const_iterator;
   typedef typename vector_0::reverse_iterator       reverse_iterator;
   typedef typename vector_0::const_reverse_iterator const_reverse_iterator;

public:
   //! Default constructor.
   vector() :
      vector_0(embedded_byte_capacity) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   vector(vector && src) :
      vector_0(embedded_byte_capacity, _std::move(src)) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   vector(vector const & src) :
      vector_0(embedded_byte_capacity, src) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   vector(vector_0 && src) :
      vector_0(embedded_byte_capacity, _std::move(src)) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   template <std::size_t src_embedded_capacity>
   vector(vector<T, src_embedded_capacity> const & src) :
      vector_0(embedded_byte_capacity, src) {
   }

   /*! Constructor that copies elements from a C array.

   @param src
      Source array.
   */
   template <std::size_t src_size>
   explicit vector(T const (& src)[src_size]) :
      vector_0(embedded_byte_capacity, src, src + src_size) {
   }

   /*! Constructor that copies elements from an array.

   @param src_begin
      Pointer to the start of the array to copy.
   @param src_end
      Pointer to the end of the array to copy.
   */
   vector(T const * src_begin, T const * src_end) :
      vector_0(embedded_byte_capacity, src_begin, src_end) {
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   vector & operator=(vector && src) {
      vector_0::operator=(_std::move(src));
      return *this;
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   vector & operator=(vector const & src) {
      vector_0::operator=(src);
      return *this;
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   vector & operator=(vector_0 && src) {
      vector_0::operator=(_std::move(src));
      return *this;
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   template <std::size_t src_embedded_capacity>
   vector & operator=(vector<T, src_embedded_capacity> const & src) {
      vector_0::operator=(src);
      return *this;
   }

   /*! Concatenation-assignment operator that moves elements from the source.

   @param src
      Vector to concatenate.
   @return
      *this.
   */
   template <std::size_t src_embedded_capacity>
   vector & operator+=(vector<T, src_embedded_capacity> && src) {
      vector_0::operator+=(_std::move(src));
      return *this;
   }

   /*! Concatenation-assignment operator that copies elements from the source.

   @param src
      Vector to concatenate.
   @return
      *this.
   */
   template <std::size_t src_embedded_capacity>
   vector & operator+=(vector<T, src_embedded_capacity> const & src) {
      vector_0::operator+=(_std::move(src));
      return *this;
   }

   using vector_0::operator[];
#ifdef LOFTY_CXX_EXPLICIT_CONVERSION_OPERATORS
   using vector_0::operator bool;
#else
   using vector_0::operator lofty::_pvt::explob_helper::bool_type;
#endif
   using vector_0::back;
   using vector_0::begin;
   using vector_0::capacity;
   using vector_0::cbegin;
   using vector_0::cend;
   using vector_0::clear;
   using vector_0::crbegin;
   using vector_0::crend;
   using vector_0::data;
   using vector_0::data_end;
   using vector_0::end;
   using vector_0::front;
   using vector_0::insert;
   using vector_0::pop_back;
   using vector_0::push_back;
   using vector_0::rbegin;
   using vector_0::remove_at;
   using vector_0::rend;
   using vector_0::set_capacity;
   using vector_0::set_size;
   using vector_0::shrink_to_fit;
   using vector_0::size;
   using vector_0::slice;

   /*! Allows using the object as a vector<T, 0> const instance.

   @return
      *this.
   */
   vector_0 const & vector0() const {
      return *this;
   }

   /*! Returns a pointer to the object as a vector<T, 0> instance.

   @return
      this.
   */
   vector_0 * vector0_ptr() {
      return this;
   }
};

/*! Equality relational operator.

@param left
   Left comparand.
@param right
   Right comparand.
@return
   true if left and right have the same element count and values, or false otherwise.
*/
template <typename T, std::size_t left_embedded_capacity, std::size_t right_embedded_capacity>
bool operator==(
   vector<T, left_embedded_capacity> const & left, vector<T, right_embedded_capacity> const & right
) {
   if (left.size() != right.size()) {
      return false;
   }
   for (
      auto left_itr(left.cbegin()), right_itr(right.cbegin()), left_end(left.cend());
      left_itr != left_end;
      ++left_itr, ++right_itr
   ) {
      if (*left_itr != *right_itr) {
         return false;
      }
   }
   return true;
}

/*! Inequality relational operator.

@param left
   Left comparand.
@param right
   Right comparand.
@return
   true if left and right have a different element count or values, or false otherwise.
*/
template <typename T, std::size_t left_embedded_capacity, std::size_t right_embedded_capacity>
bool operator!=(
   vector<T, left_embedded_capacity> const & left, vector<T, right_embedded_capacity> const & right
) {
   return !operator==(left, right);
}

}} //namespace lofty::collections

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

/*! Base class for the specializations of from_text_istream for vector types. Not using templates, so the
implementation can be in a cxx file. */
class LOFTY_SYM vector_from_text_istream : public lofty::_pvt::sequence_from_text_istream {
public:
   //! Default constructor.
   vector_from_text_istream();

   //! Destructor.
   ~vector_from_text_istream();
};

}}} //namespace lofty::collections::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <typename T, std::size_t embedded_capacity>
class from_text_istream<collections::vector<T, embedded_capacity>> :
   public collections::_pvt::vector_from_text_istream {
private:
   typedef collections::_pvt::vector_from_text_istream vector_from_text_istream;

public:
   /*! Converts a capture into a value of the appropriate type.

   @param capture0
      Pointer to the top-level capture.
   @param dst
      Pointer to the destination object.
   */
   void convert_capture(
      text::parsers::dynamic_match_capture const & capture0, collections::vector<T, embedded_capacity> * dst
   ) {
      std::size_t count = vector_from_text_istream::captures_count(capture0);
      dst->set_size(count);
      for (std::size_t i = 0; i < count; ++i) {
         auto const & elt_capture = vector_from_text_istream::capture_at(capture0, i);
         elt_ftis.convert_capture(elt_capture, &(*dst)[static_cast<std::ptrdiff_t>(i)]);
      }
   }

   /*! Creates parser states for the specified input format.

   @param format
      Formatting options.
   @param parser
      Pointer to the parser instance to use to create non-static states.
   @return
      First parser state.
   */
   text::parsers::dynamic_state const * format_to_parser_states(
      text::parsers::ere_capture_format const & format, text::parsers::dynamic * parser
   ) {
      auto const & elt_format = vector_from_text_istream::extract_elt_format(format);
      auto elt_first_state = elt_ftis.format_to_parser_states(elt_format, parser);
      return vector_from_text_istream::format_to_parser_states(format, parser, elt_first_state);
   }

protected:
   //! Backend for the individual elements.
   from_text_istream<T> elt_ftis;
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

/*! Base class for the specializations of to_text_ostream for vector types. Not using templates, so the
implementation can be in a cxx file. */
class LOFTY_SYM vector_to_text_ostream : public lofty::_pvt::sequence_to_text_ostream {
public:
   //! Default constructor.
   vector_to_text_ostream();

   //! Destructor.
   ~vector_to_text_ostream();

   /*! Changes the output format.

   @param format
      Formatting options.
   @return
      Formatting options to be applied to each individual element.
   */
   str set_format(str const & format);
};

}}} //namespace lofty::collections::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <typename T, std::size_t embedded_capacity>
class to_text_ostream<collections::vector<T, embedded_capacity>> :
   public collections::_pvt::vector_to_text_ostream {
public:
   /*! Changes the output format.

   @param format
      Formatting options.
   */
   void set_format(str const & format) {
//    LOFTY_TRACE_FUNC(this, format);

      str elt_format(collections::_pvt::vector_to_text_ostream::set_format(format));
      elt_ttos.set_format(elt_format);
   }

   /*! Writes a vector, applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(collections::vector<T, embedded_capacity> const & src, io::text::ostream * dst) {
//    LOFTY_TRACE_FUNC(this/*, src*/, dst);

      _write_start(dst);
      auto itr(src.cbegin()), end(src.cend());
      if (itr != end) {
         elt_ttos.write(*itr, dst);
         while (++itr != end) {
            _write_separator(dst);
            elt_ttos.write(*itr, dst);
         }
      }
      _write_end(dst);
   }

protected:
   //! Backend for the individual elements.
   to_text_ostream<T> elt_ttos;
};

template <typename T>
class to_text_ostream<collections::_pvt::vector_const_iterator<T>> :
   public to_text_ostream<typename collections::_pvt::vector_const_iterator<T>::pointer> {
public:
   /*! Writes an iterator as a pointer, applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(collections::_pvt::vector_const_iterator<T> const & src, io::text::ostream * dst) {
      to_text_ostream<typename collections::_pvt::vector_const_iterator<T>::pointer>::write(&*src, dst);
   }
};

template <typename T>
class to_text_ostream<collections::_pvt::vector_iterator<T>> :
   public to_text_ostream<collections::_pvt::vector_const_iterator<T>> {
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_COLLECTIONS_VECTOR_HXX
