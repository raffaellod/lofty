/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

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

#ifndef _ABACLADE_COLLECTIONS_VECTOR_HXX
#define _ABACLADE_COLLECTIONS_VECTOR_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/type_void_adapter.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

/*! Thin templated wrapper for *_vextr_impl to make the interface of those two classes consistent,
so vector doesn’t need specializations. */
template <
   typename T,
   bool t_bCopyConstructible = _std::is_copy_constructible<T>::value,
   bool t_bTrivial = _std::is_trivial<T>::value
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

   /*! Ensures that the vector has at least ciMin of actual item space. If this causes *this to
   switch to using a different item array, any elements in the current one will be destructed unless
   bPreserve == true, which will cause them to be moved to the new item array.

   @param ciMin
      Minimum count of elements requested.
   @param bPreserve
      If true, the previous contents of the vector will be preserved even if the reallocation causes
      the vector to switch to a different item array.
   */
   void set_capacity(std::size_t ciMin, bool bPreserve) {
      type_void_adapter type;
      type.set_destruct<T>();
      type.set_move_construct<T>();
      complex_vextr_impl::set_capacity(type, sizeof(T) * ciMin, bPreserve);
   }

   /*! Changes the count of items in the vector. If the new item count is greater than the current
   one, the added elements will be left uninitialized; it’s up to the caller to make sure that these
   elements are properly constructed, or problems will arise when the destructor will attempt to
   destruct these elements.

   TODO: destruct in complex_vextr_impl::set_size() any elements being taken out, and default-
   construct the newly-created elements here.

   @param ci
      New vector size.
   */
   void set_size(std::size_t ci) {
      type_void_adapter type;
      type.set_destruct<T>();
      type.set_move_construct<T>();
      complex_vextr_impl::set_size(type, sizeof(T) * ci);
   }

protected:
   //! See complex_vextr_impl::assign_move_desc_or_move_items().
   void assign_move_desc_or_move_items(complex_vextr_impl && rcvi) {
      type_void_adapter type;
      type.set_destruct<T>();
      type.set_move_construct<T>();
      complex_vextr_impl::assign_move_desc_or_move_items(type, _std::move(rcvi));
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
      type.set_destruct<T>();
      type.set_move_construct<T>();
      type.set_size<T>();
      complex_vextr_impl::insert(
         type,
         static_cast<std::size_t>(
            reinterpret_cast<std::int8_t const *>(ptOffset) - vextr_impl_base::begin<std::int8_t>()
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
      type.set_destruct<T>();
      type.set_move_construct<T>();
      type.set_size<T>();
      complex_vextr_impl::remove(
         type,
         static_cast<std::size_t>(
            reinterpret_cast<std::int8_t const *>(ptRemoveBegin) -
               vextr_impl_base::begin<std::int8_t>()
         ),
         reinterpret_cast<std::size_t>(ptRemoveEnd) - reinterpret_cast<std::size_t>(ptRemoveBegin)
      );
   }

protected:
   //! See complex_vextr_impl::complex_vextr_impl().
   vector_impl(std::size_t cbEmbeddedCapacity) :
      complex_vextr_impl(cbEmbeddedCapacity) {
   }

   //! See complex_vextr_impl::complex_vextr_impl().
   vector_impl(T const * ptConstSrc, std::size_t ciSrc) :
      complex_vextr_impl(ptConstSrc, ciSrc) {
   }

private:
   // Hide these complex_vextr_impl methods to trigger errors as a debugging aid.
   void assign_copy(type_void_adapter const & type, T const * ptBegin, T const * ptEnd);
};

// Partial specialization for copyable, non-trivial types.
template <typename T>
class vector_impl<T, true, false> : public vector_impl<T, false, false> {
protected:
   //! See complex_vextr_impl::assign_copy().
   void assign_copy(T const * ptBegin, T const * ptEnd) {
      type_void_adapter type;
      type.set_copy_construct<T>();
      type.set_destruct<T>();
      type.set_move_construct<T>();
      complex_vextr_impl::assign_copy(type, ptBegin, ptEnd);
   }

   //! See complex_vextr_impl::assign_concat().
   void assign_concat(
      T const * p1Begin, T const * p1End, T const * p2Begin, T const * p2End, std::uint8_t iMove
   ) {
      type_void_adapter type;
      type.set_copy_construct<T>();
      type.set_destruct<T>();
      type.set_move_construct<T>();
      complex_vextr_impl::assign_concat(type, p1Begin, p1End, p2Begin, p2End, iMove);
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
      type.set_copy_construct<T>();
      type.set_destruct<T>();
      type.set_move_construct<T>();
      type.set_size<T>();
      complex_vextr_impl::insert(
         type,
         static_cast<std::size_t>(
            reinterpret_cast<std::int8_t const *>(ptOffset) - vextr_impl_base::begin<std::int8_t>()
         ),
         ptInsert, sizeof(T) * ciInsert, false
      );
   }

protected:
   //! See vector_impl<T, false, false>::vector_impl<T, false, false>().
   vector_impl(std::size_t cbEmbeddedCapacity) :
      vector_impl<T, false, false>(cbEmbeddedCapacity) {
   }

   //! See vector_impl<T, false, false>::vector_impl<T, false, false>().
   vector_impl(std::size_t cbEmbeddedCapacity, T const * ptConstSrc, std::size_t ciSrc) :
      vector_impl<T, false, false>(cbEmbeddedCapacity, ptConstSrc, ciSrc) {
   }
};

/* Partial specialization for trivial (and copyable) types. Methods here ignore the bMove argument
for the individual elements, because move semantics don’t apply (trivial values are always
copied). */
template <typename T>
class vector_impl<T, true, true> : public trivial_vextr_impl {
public:
   /*! Ensures that the vector has at least ciMin of actual item space. If this causes *this to
   switch to using a different item array, any elements in the current one will be destructed unless
   bPreserve == true, which will cause them to be moved to the new item array.

   @param ciMin
      Minimum count of elements requested.
   @param bPreserve
      If true, the previous contents of the vector will be preserved even if the reallocation causes
      the vector to switch to a different item array.
   */
   void set_capacity(std::size_t ciMin, bool bPreserve) {
      trivial_vextr_impl::set_capacity(sizeof(T) * ciMin, bPreserve);
   }

   /*! Changes the count of items in the vector. If the new item count is greater than the current
   one, the added elements will be left uninitialized; it’s up to the caller to make sure that these
   elements are properly constructed, or problems will arise when the destructor will attempt to
   destruct these elements.

   TODO: maybe default-construct the newly-created elements here for consistency with the non-
   trivial specialization?

   @param ci
      New vector size.
   */
   void set_size(std::size_t ci) {
      trivial_vextr_impl::set_size(sizeof(T) * ci);
   }

protected:
   //! See trivial_vextr_impl::assign_copy().
   void assign_copy(T const * ptBegin, T const * ptEnd) {
      trivial_vextr_impl::assign_copy(ptBegin, ptEnd);
   }

   //! See trivial_vextr_impl::assign_concat().
   void assign_concat(
      T const * p1Begin, T const * p1End, T const * p2Begin, T const * p2End, std::uint8_t iMove
   ) {
      ABC_UNUSED_ARG(iMove);
      trivial_vextr_impl::assign_concat(p1Begin, p1End, p2Begin, p2End);
   }

   //! See trivial_vextr_impl::assign_move_desc_or_move_items().
   void assign_move_desc_or_move_items(trivial_vextr_impl && rtvi) {
      trivial_vextr_impl::assign_move_desc_or_move_items(_std::move(rtvi));
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
      trivial_vextr_impl::insert_remove(
         static_cast<std::size_t>(
            reinterpret_cast<std::int8_t const *>(ptOffset) - vextr_impl_base::begin<std::int8_t>()
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
      trivial_vextr_impl::insert_remove(
         static_cast<std::size_t>(
            reinterpret_cast<std::int8_t const *>(ptOffset) - vextr_impl_base::begin<std::int8_t>()
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
      trivial_vextr_impl::insert_remove(
         static_cast<std::size_t>(
            reinterpret_cast<std::int8_t const *>(ptRemoveBegin) -
               vextr_impl_base::begin<std::int8_t>()
         ),
         nullptr, 0,
         reinterpret_cast<std::size_t>(ptRemoveEnd) - reinterpret_cast<std::size_t>(ptRemoveBegin)
      );
   }

protected:
   //! See trivial_vextr_impl::trivial_vextr_impl().
   vector_impl(std::size_t cbEmbeddedCapacity) :
      trivial_vextr_impl(cbEmbeddedCapacity) {
   }

   //! See trivial_vextr_impl::trivial_vextr_impl().
   vector_impl(std::size_t cbEmbeddedCapacity, T const * ptConstSrc, std::size_t ciSrc) :
      trivial_vextr_impl(cbEmbeddedCapacity, ptConstSrc, ciSrc) {
   }
};

}}} //namespace abc::collections::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

//! Const iterator for vector elements.
template <typename T>
class vector_const_iterator {
private:
   template <typename T2, std::size_t t_ciEmbeddedCapacity>
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
      m_pv(nullptr),
      m_pt(nullptr) {
   }

   /*! Copy constructor.

   @param it
      Source object.
   */
   vector_const_iterator(vector_const_iterator const & it) :
      m_pv(it.m_pv),
      m_pt(it.m_pt) {
   }

   /*! Dereferencing operator.

   @return
      Reference to the current item.
   */
   T const & operator*() const {
      return *vector_0::validate_pointer(m_pv, m_pt, false);
   }

   /*! Dereferencing member access operator.

   @return
      Pointer to the current item.
   */
   T const * operator->() const {
      return vector_0::validate_pointer(m_pv, m_pt, false);
   }

   /*! Element access operator.

   @param i
      Element index.
   @return
      Reference to the specified element.
   */
   T const & operator[](std::ptrdiff_t i) const {
      return *vector_0::validate_pointer(m_pv, m_pt + i, false);
   }

   /*! Addition-assignment operator.

   @param i
      Count of positions by which to advance the iterator.
   @return
      *this after it’s moved forward by i positions.
   */
   vector_const_iterator & operator+=(std::ptrdiff_t i) {
      m_pt = *vector_0::validate_pointer(m_pv, m_pt + i, true);
      return *this;
   }

   /*! Subtraction-assignment operator.

   @param i
      Count of positions by which to rewind the iterator.
   @return
      *this after it’s moved backwards by i positions.
   */
   vector_const_iterator & operator-=(std::ptrdiff_t i) {
      m_pt = *vector_0::validate_pointer(m_pv, m_pt - i, true);
      return *this;
   }

   /*! Addition operator.

   @param i
      Count of positions by which to advance the iterator.
   @return
      Iterator that’s i items ahead of *this.
   */
   vector_const_iterator operator+(std::ptrdiff_t i) const {
      return vector_const_iterator(m_pv, vector_0::validate_pointer(m_pv, m_pt + i, true));
   }

   /*! Subtraction/difference operator.

   @param i
      Count of positions by which to rewind the iterator.
   @param it
      Iterator from which to calculate the distance.
   @return
      Iterator that’s i items behind *this (subtraction) or distance between *this and it
      (difference).
   */
   vector_const_iterator operator-(std::ptrdiff_t i) const {
      return vector_const_iterator(m_pv, vector_0::validate_pointer(m_pv, m_pt - i, true));
   }
   std::ptrdiff_t operator-(vector_const_iterator it) const {
      return m_pt - it.m_pt;
   }

   /*! Preincrement operator.

   @return
      *this after it’s moved to the value following the one currently pointed to.
   */
   vector_const_iterator & operator++() {
      m_pt = vector_0::validate_pointer(m_pv, m_pt + 1, true);
      return *this;
   }

   /*! Postincrement operator.

   @return
      Iterator pointing to the value following the one pointed to by this iterator.
   */
   vector_const_iterator operator++(int) {
      T const * pt = m_pt;
      m_pt = vector_0::validate_pointer(m_pv, m_pt + 1, true);
      return vector_const_iterator(m_pv, pt);
   }

   /*! Predecrement operator.

   @return
      *this after it’s moved to the value preceding the one currently pointed to.
   */
   vector_const_iterator & operator--() {
      m_pt = vector_0::validate_pointer(m_pv, m_pt - 1, true);
      return *this;
   }

   /*! Postdecrement operator.

   @return
      Iterator pointing to the value preceding the one pointed to by this iterator.
   */
   vector_const_iterator operator--(int) {
      T const * pt = m_pt;
      m_pt = vector_0::validate_pointer(m_pv, m_pt - 1, true);
      return vector_const_iterator(m_pv, pt);
   }

// Relational operators.
#define ABC_RELOP_IMPL(op) \
   bool operator op(vector_const_iterator const & it) const { \
      return m_pt op it.m_pt; \
   }
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
ABC_RELOP_IMPL(>)
ABC_RELOP_IMPL(>=)
ABC_RELOP_IMPL(<)
ABC_RELOP_IMPL(<=)
#undef ABC_RELOP_IMPL

protected:
   /*! Constructor.

   @param pv
      Pointer to the vector containing *pt.
   @param pt
      Pointer to set the iterator to.
   */
   explicit vector_const_iterator(vector_0 const * pv, T const * pt) :
      m_pv(pv),
      m_pt(pt) {
   }

protected:
   //! Pointer to the vector containing the current element.
   vector<T, 0> const * m_pv;
   //! Pointer to the current element.
   T const * m_pt;
};

//! Non-const iterator for vector elements.
template <typename T>
class vector_iterator : public vector_const_iterator<T> {
private:
   template <typename T2, std::size_t t_ciEmbeddedCapacity>
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

   @param pv
      Pointer to the vector containing *pt.
   @param pt
      Pointer to set the iterator to.
   */
   explicit vector_iterator(typename vci::vector_0 const * pv, T const * pt) :
      vci(pv, pt) {
   }

   /*! Copy constructor.

   @param it
      Source object.
   */
   vector_iterator(vci const & it) :
      vci(it) {
   }
};

}}} //namespace abc::collections::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections {

// Partial specialization for non-copyable types.
template <typename T>
class vector<T, 0> :
   public detail::vector_impl<T>,
   public support_explicit_operator_bool<vector<T, 0>> {
private:
   template <typename T2>
   friend class detail::vector_iterator;
   template <typename T2>
   friend class detail::vector_const_iterator;

   //! true if T is copy constructible, or false otherwise.
   static bool const smc_bCopyConstructible = _std::is_copy_constructible<T>::value;
   typedef detail::vector_impl<T> vector_impl;

public:
   typedef T value_type;
   typedef T * pointer;
   typedef T const * const_pointer;
   typedef T & reference;
   typedef T const & const_reference;
   typedef std::size_t size_type;
   typedef std::ptrdiff_t difference_type;
   typedef detail::vector_iterator<T> iterator;
   typedef detail::vector_const_iterator<T> const_iterator;
   typedef _std::reverse_iterator<iterator> reverse_iterator;
   typedef _std::reverse_iterator<const_iterator> const_reverse_iterator;

public:
   //! Default constructor.
   vector() :
      vector_impl(0) {
   }

   /*! Move constructor.

   @param v
      Source object.
   */
   vector(vector && v) :
      vector_impl(0) {
      vector_impl::assign_move_desc_or_move_items(_std::move(v));
   }

   /*! Copy constructor.

   @param v
      Source object.
   */
   vector(vector const & v) :
      vector_impl(0) {
      vector_impl::assign_copy(v.data(), v.data_end());
   }

   /*! Constructor that concatenates two vectors, copying elements from the first and moving
   elements from the second.

   @param v1
      First source vector.
   @param v2
      Second source vector.
   */
   template <std::size_t t_ciEmbeddedCapacity1, std::size_t t_ciEmbeddedCapacity2>
   vector(vector<T, t_ciEmbeddedCapacity1> const & v1, vector<T, t_ciEmbeddedCapacity2> && v2) :
      vector_impl(0) {
      vector_impl::assign_concat(v1.data(), v1.data_end(), v2.data(), v2.data_end(), 2);
   }

   /*! Constructor that concatenates two vectors, copying elements from both.

   @param v1
      First source vector.
   @param v2
      Second source vector.
   */
   template <std::size_t t_ciEmbeddedCapacity1, std::size_t t_ciEmbeddedCapacity2>
   vector(
      vector<T, t_ciEmbeddedCapacity1> const & v1, vector<T, t_ciEmbeddedCapacity2> const & v2
   ) :
      vector_impl(0) {
      vector_impl::assign_concat(v1.data(), v1.data_end(), v2.data(), v2.data_end(), 0);
   }

   /*! Constructor that copies elements from a C array.

   @param at
      Source array whose elements should be copied.
   */
   template <std::size_t t_ci>
   explicit vector(T const (& at)[t_ci]) :
      vector_impl(0) {
      vector_impl::assign_copy(at, at + t_ci);
   }

   /*! Constructor that copies elements from an array.

   @param ptBegin
      Pointer to the beginning of the source array.
   @param ptEnd
      Pointer to the end of the source array.
   */
   vector(T const * ptBegin, T const * ptEnd) :
      vector_impl(0) {
      vector_impl::assign_copy(ptBegin, ptEnd);
   }

   /*! Constructor that concatenates two arrays, copying elements from both.

   @param p1Begin
      Pointer to the start of the first source array.
   @param p1End
      Pointer to the end of the first source array.
   @param p2Begin
      Pointer to the start of the second source array.
   @param p2End
      Pointer to the end of the second source array.
   */
   vector(T const * pt1Begin, T const * pt1End, T const * pt2Begin, T const * pt2End) :
      vector_impl(0) {
      vector_impl::assign_concat(pt1Begin, pt1End, pt2Begin, pt2End, 0);
   }

   /*! Move-assignment operator.

   @param v
      Source object.
   @return
      *this.
   */
   vector & operator=(vector && v) {
      vector_impl::assign_move_desc_or_move_items(_std::move(v));
      return *this;
   }

   /*! Copy-assignment operator.

   @param v
      Source object.
   @return
      *this.
   */
   vector & operator=(vector const & v) {
      vector_impl::assign_copy(v.data(), v.data_end());
      return *this;
   }

   /*! Concatenation-assignment operator that moves elements from the source.

   @param v
      Vector to concatenate.
   @return
      *this.
   */
   template <std::size_t t_ciEmbeddedCapacity2>
   vector & operator+=(vector<T, t_ciEmbeddedCapacity2> && v) {
      vector_impl::insert_move(data_end(), v.data(), v.size());
      return *this;
   }

   /*! Concatenation-assignment operator that copies elements from the source.

   @param v
      Vector to concatenate.
   @return
      *this.
   */
   template <std::size_t t_ciEmbeddedCapacity2>
   vector & operator+=(vector<T, t_ciEmbeddedCapacity2> const & v) {
      vector_impl::insert_copy(data_end(), v.data(), v.size());
      return *this;
   }

   /*! Element access operator.

   @param i
      Element index.
   @return
      Element at index i.
   */
   T & operator[](std::ptrdiff_t i) {
      return *validate_pointer(data() + i, false);
   }
   T const & operator[](std::ptrdiff_t i) const {
      return const_cast<vector *>(this)->operator[](i);
   }

   /*! Boolean evaluation operator.

   @return
      true if the vector is not empty, or false otherwise.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
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

   @param itOffset
      Iterator at which the element should be inserted.
   @param t
      Element to insert.
   */
   void insert(const_iterator itOffset, typename _std::remove_const<T>::type && t) {
      validate_pointer(itOffset.m_pt, true);
      vector_impl::insert_move(itOffset.m_pt, &t, 1);
   }

   /*! Inserts an element at a specific position in the vector.

   @param itOffset
      Iterator at which the element should be inserted.
   @param t
      Element to insert.
   */
   void insert(const_iterator itOffset, T const & t) {
      validate_pointer(itOffset.m_pt, true);
      vector_impl::insert_copy(itOffset.m_pt, &t, 1);
   }

   /*! Inserts elements at a specific position in the vector.

   @param itOffset
      Iterator at which the element should be inserted.
   @param pt
      Pointer to the first element to insert.
   @param ci
      Count of elements in the array pointed to by pt.
   */
   void insert(const_iterator itOffset, T const * pt, std::size_t ci) {
      validate_pointer(itOffset.m_pt, true);
      vector_impl::insert_copy(itOffset.m_pt, pt, ci);
   }

   /*! Removes and returns the last element in the vector.

   @return
      Former last element of the vector.
   */
   T pop_back() {
      T * ptBack = &back();
      T tBack(std::move(*ptBack));
      vector_impl::remove(ptBack, ptBack + 1);
      return std::move(tBack);
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

   @param pt
      Pointer to an array of elements to copy to the end of the vector.
   @param ci
      Count of elements in the array pointed to by pt.
   */
   void push_back(T const * pt, std::size_t ci) {
      vector_impl::insert_copy(data_end(), pt, ci);
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

   @param it
      Iterator to the element to remove.
   */
   void remove_at(const_iterator it) {
      validate_pointer(it.m_pt, false);
      vector_impl::remove(it.m_pt, it.m_pt + 1);
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

   @param itBegin
      Iterator to the first element to return.
   */
   vector slice(const_iterator itBegin) const {
      return vector(itBegin.m_pt, data_end());
   }

   /*! Returns a slice of the vector.

   @param itBegin
      Iterator to the first element to return.
   @param itEnd
      Iterator to the element after the last one to return.
   */
   vector slice(const_iterator itBegin, const_iterator itEnd) const {
      return vector(itBegin.m_pt, itEnd.m_pt);
   }

protected:
   /*! Constructor for subclasses with an embedded item array.

   @param cbEmbeddedCapacity
      Size of the embedded character array, in bytes.
   */
   vector(std::size_t cbEmbeddedCapacity) :
      vector_impl(cbEmbeddedCapacity) {
   }

   /*! Move constructor.

   @param cbEmbeddedCapacity
      Size of the embedded character array, in bytes.
   @param v
      Source object.
   */
   vector(std::size_t cbEmbeddedCapacity, vector && v) :
      vector_impl(cbEmbeddedCapacity) {
      vector_impl::assign_move_desc_or_move_items(_std::move(v));
   }

   /*! Copy constructor for subclasses with an embedded item array.

   @param cbEmbeddedCapacity
      Size of the embedded character array, in bytes.
   @param v
      Source object.
   */
   vector(std::size_t cbEmbeddedCapacity, vector const & v) :
      vector_impl(cbEmbeddedCapacity) {
      vector_impl::assign_copy(v.data(), v.data_end());
   }

   /*! Copy constructor from C arrays for subclasses with an embedded item array.

   @param pt
      Pointer to a C array that will be copied to the vector.
   @param ci
      Count of items in the array pointed to by pt.
   */
   vector(std::size_t cbEmbeddedCapacity, T const * pt, std::size_t ci) :
      vector_impl(cbEmbeddedCapacity) {
      vector_impl::assign_copy(pt, pt + ci);
   }

   /*! Throws a collections::out_of_range if a pointer is not within bounds.

   @param pt
      Pointer to validate.
   @param bAllowEnd
      If true, pt == data_end() is allowed; if false, it’s not.
   */
   template <typename TPtr>
   TPtr * validate_pointer(TPtr * pt, bool bAllowEnd) const {
      vector_impl::validate_pointer(pt, bAllowEnd);
      return pt;
   }

   /*! Throws a collections::out_of_range if a pointer is not within bounds of *pv.

   This overload is static so that it will validate that this (pv) is not nullptr before
   dereferencing it.

   @param pv
      this.
   @param pt
      Pointer to validate.
   @param bAllowEnd
      If true, pt == pv->data_end() is allowed; if false, it’s not.
   */
   template <typename TPtr>
   static TPtr * validate_pointer(vector const * pv, TPtr * pt, bool bAllowEnd) {
      vector_impl::validate_pointer(pv, pt, bAllowEnd);
      return pt;
   }
};

/*! Concatenation operator.

@param vL
   Left operand.
@param vR
   Right operand.
@return
   Vector resulting from the concatenation of vL and vR.
*/
template <typename T, std::size_t t_cchEmbeddedCapacityL, std::size_t t_cchEmbeddedCapacityR>
inline vector<T, t_cchEmbeddedCapacityL> operator+(
   vector<T, t_cchEmbeddedCapacityL> && vL, vector<T, t_cchEmbeddedCapacityR> && vR
) {
   vL += vR;
   return _std::move(vL);
}

/*! Concatenation operator.

@param vL
   Left operand.
@param vR
   Right operand.
@return
   Vector resulting from the concatenation of vL and vR.
*/
template <typename T, std::size_t t_cchEmbeddedCapacityL, std::size_t t_cchEmbeddedCapacityR>
inline typename _std::enable_if<
   _std::is_copy_constructible<T>::value, vector<T, t_cchEmbeddedCapacityL>
>::type operator+(
   vector<T, t_cchEmbeddedCapacityL> && vL, vector<T, t_cchEmbeddedCapacityR> const & vR
) {
   vL += vR;
   return _std::move(vL);
}

/*! Concatenation operator that copies elements from the first and moving elements from the second.

@param vL
   Left operand.
@param vR
   Right operand.
@return
   Vector resulting from the concatenation of vL and vR.
*/
template <typename T, std::size_t t_cchEmbeddedCapacityL, std::size_t t_cchEmbeddedCapacityR>
inline typename _std::enable_if<_std::is_copy_constructible<T>::value, vector<T>>::type operator+(
   vector<T, t_cchEmbeddedCapacityL> const & vL, vector<T, t_cchEmbeddedCapacityR> && vR
) {
   return vector<T>(vL, _std::move(vR));
}

/*! Concatenation operator.

@param vL
   Left operand.
@param vR
   Right operand.
@return
   Vector resulting from the concatenation of vL and vR.
*/
template <typename T, std::size_t t_cchEmbeddedCapacityL, std::size_t t_cchEmbeddedCapacityR>
inline typename _std::enable_if<_std::is_copy_constructible<T>::value, vector<T>>::type operator+(
   vector<T, t_cchEmbeddedCapacityL> const & vL, vector<T, t_cchEmbeddedCapacityR> const & vR
) {
   return vector<T>(vL, vR);
}

// General definition, with embedded item array.
template <typename T, std::size_t t_ciEmbeddedCapacity>
class vector :
   private vector<T, 0>,
   private detail::vextr_prefixed_item_array<T, t_ciEmbeddedCapacity> {
private:
   using detail::vextr_prefixed_item_array<T, t_ciEmbeddedCapacity>::smc_cbEmbeddedCapacity;
   //! true if T is copy constructible, or false otherwise.
   static bool const smc_bCopyConstructible = _std::is_copy_constructible<T>::value;
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
      vector_0(smc_cbEmbeddedCapacity) {
   }

   /*! Move constructor.

   @param v
      Source object.
   */
   vector(vector && v) :
      vector_0(smc_cbEmbeddedCapacity, _std::move(v)) {
   }

   /*! Copy constructor.

   @param v
      Source object.
   */
   vector(vector const & v) :
      vector_0(smc_cbEmbeddedCapacity, v) {
   }

   /*! Move constructor.

   @param v
      Source object.
   */
   vector(vector_0 && v) :
      vector_0(smc_cbEmbeddedCapacity, _std::move(v)) {
   }

   /*! Copy constructor.

   @param v
      Source object.
   */
   template <std::size_t t_ciEmbeddedCapacity2>
   vector(vector<T, t_ciEmbeddedCapacity2> const & v) :
      vector_0(smc_cbEmbeddedCapacity, v) {
   }

   /*! Constructor that copies elements from a C array.

   @param at
      Source array.
   */
   template <std::size_t t_ci>
   explicit vector(T const (& at)[t_ci]) :
      vector_0(smc_cbEmbeddedCapacity, at, at + t_ci) {
   }

   /*! Constructor that copies elements from an array.

   @param pt
      Pointer to an array.
   @param ci
      Count of items in *pt.
   */
   vector(T const * ptBegin, T const * ptEnd) :
      vector_0(smc_cbEmbeddedCapacity, ptBegin, ptEnd) {
   }

   /*! Move-assignment operator.

   @param v
      Source object.
   @return
      *this.
   */
   vector & operator=(vector && v) {
      vector_0::operator=(_std::move(v));
      return *this;
   }

   /*! Copy-assignment operator.

   @param v
      Source object.
   @return
      *this.
   */
   vector & operator=(vector const & v) {
      vector_0::operator=(v);
      return *this;
   }

   /*! Move-assignment operator.

   @param v
      Source object.
   @return
      *this.
   */
   vector & operator=(vector_0 && v) {
      vector_0::operator=(_std::move(v));
      return *this;
   }

   /*! Copy-assignment operator.

   @param v
      Source object.
   @return
      *this.
   */
   template <std::size_t t_ciEmbeddedCapacity2>
   vector & operator=(vector<T, t_ciEmbeddedCapacity2> const & v) {
      vector_0::operator=(v);
      return *this;
   }

   /*! Concatenation-assignment operator that moves elements from the source.

   @param v
      Vector to concatenate.
   @return
      *this.
   */
   template <std::size_t t_ciEmbeddedCapacity2>
   vector & operator+=(vector<T, t_ciEmbeddedCapacity2> && v) {
      vector_0::operator+=(_std::move(v));
      return *this;
   }

   /*! Concatenation-assignment operator that copies elements from the source.

   @param v
      Vector to concatenate.
   @return
      *this.
   */
   template <std::size_t t_ciEmbeddedCapacity2>
   vector & operator+=(vector<T, t_ciEmbeddedCapacity2> const & v) {
      vector_0::operator+=(_std::move(v));
      return *this;
   }

   using vector_0::operator[];
#ifdef ABC_CXX_EXPLICIT_CONVERSION_OPERATORS
   using vector_0::operator bool;
#else
   using vector_0::operator abc::detail::explob_helper::bool_type;
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

@param vL
   Left comparand.
@param vR
   Right comparand.
@return
   true if vL and vR have the same element count and values, or false otherwise.
*/
template <typename T, std::size_t t_ciEmbeddedCapacityL, std::size_t t_ciEmbeddedCapacityR>
bool operator==(
   vector<T, t_ciEmbeddedCapacityL> const & vL, vector<T, t_ciEmbeddedCapacityR> const & vR
) {
   if (vL.size() != vR.size()) {
      return false;
   }
   for (auto itL(vL.cbegin()), itR(vR.cbegin()), itLEnd(vL.cend()); itL != itLEnd; ++itL, ++itR) {
      if (*itL != *itR) {
         return false;
      }
   }
   return true;
}

/*! Inequality relational operator.

@param vL
   Left comparand.
@param vR
   Right comparand.
@return
   true if vL and vR have a different element count or values, or false otherwise.
*/
template <typename T, std::size_t t_ciEmbeddedCapacityL, std::size_t t_ciEmbeddedCapacityR>
bool operator!=(
   vector<T, t_ciEmbeddedCapacityL> const & vL, vector<T, t_ciEmbeddedCapacityR> const & vR
) {
   return !operator==(vL, vR);
}

}} //namespace abc::collections

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

/*! Base class for the specializations of to_str_backend for vector types. Not using templates, so
the implementation can be in a cxx file. */
class ABACLADE_SYM vector_to_str_backend : public abc::detail::sequence_to_str_backend {
public:
   //! Default constructor.
   vector_to_str_backend();

   //! Destructor.
   ~vector_to_str_backend();

protected:
   /*! Formatting options to be applied to the individual elements, obtained from the constructor
   argument sFormat. */
   str m_sEltFormat;
};

}}} //namespace abc::collections::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

template <typename T, std::size_t t_ciEmbeddedCapacity>
class to_str_backend<collections::vector<T, t_ciEmbeddedCapacity>> :
   public collections::detail::vector_to_str_backend {
public:
   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(str const & sFormat) {
//    ABC_TRACE_FUNC(this, sFormat);

      collections::detail::vector_to_str_backend::set_format(sFormat);
      m_tsbElt.set_format(m_sEltFormat);
   }

   /*! Writes a vector, applying the formatting options.

   @param v
      Vector to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(collections::vector<T, t_ciEmbeddedCapacity> const & v, io::text::writer * ptwOut) {
//    ABC_TRACE_FUNC(this/*, v*/, ptwOut);

      _write_start(ptwOut);
      auto it(v.cbegin()), itEnd(v.cend());
      if (it != itEnd) {
         m_tsbElt.write(*it, ptwOut);
         while (++it != itEnd) {
            _write_separator(ptwOut);
            m_tsbElt.write(*it, ptwOut);
         }
      }
      _write_end(ptwOut);
   }

protected:
   //! Backend for the individual elements.
   to_str_backend<T> m_tsbElt;
};

template <typename T>
class to_str_backend<collections::detail::vector_const_iterator<T>> :
   public to_str_backend<typename collections::detail::vector_const_iterator<T>::pointer> {
public:
   /*! Writes an iterator as a pointer, applying the formatting options.

   @param it
      Iterator to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(collections::detail::vector_const_iterator<T> const & it, io::text::writer * ptwOut) {
      to_str_backend<
         typename collections::detail::vector_const_iterator<T>::pointer
      >::write(&*it, ptwOut);
   }
};

template <typename T>
class to_str_backend<collections::detail::vector_iterator<T>> :
   public to_str_backend<collections::detail::vector_const_iterator<T>> {
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COLLECTIONS_VECTOR_HXX
