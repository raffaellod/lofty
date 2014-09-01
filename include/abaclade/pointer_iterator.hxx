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
// abc::pointer_iterator


namespace abc {

//! Iterator based on a plain pointer.
template <typename TCont, typename TVal>
class pointer_iterator :
   public std::iterator<std::random_access_iterator_tag, TVal> {
public:

   /*! Constructor.

   pt
      Pointer to set the iterator to.
   it
      Source iterator.
   */
   /*constexpr*/ pointer_iterator() :
      m_ptval(nullptr) {
   }
   explicit pointer_iterator(TVal * pt) :
      m_ptval(pt) {
   }
   // Allows to convert between non-const to const TVals.
   template <typename TVal2>
   pointer_iterator(pointer_iterator<TCont, TVal2> const & it) :
      m_ptval(it.base()) {
   }


   /*! Dereferencing operator.

   return
      Reference to the current item.
   */
   TVal & operator*() const {
      return *m_ptval;
   }


   /*! Dereferencing member access operator.

   return
      Pointer to the current item.
   */
   TVal * operator->() const {
      return m_ptval;
   }


   /*! Element access operator.

   i
      Index relative to *this.
   return
      Reference to the specified item.
   */
   TVal & operator[](std::ptrdiff_t i) const {
      return m_ptval[i];
   }


   /*! Addition-assignment operator.

   i
      Count of positions by which to advance the iterator.
   return
      *this after it’s moved forward by i positions.
   */
   pointer_iterator & operator+=(std::ptrdiff_t i) {
      m_ptval += i;
      return *this;
   }


   /*! Subtraction-assignment operator.

   i
      Count of positions by which to rewind the iterator.
   return
      *this after it’s moved backwards by i positions.
   */
   pointer_iterator & operator-=(std::ptrdiff_t i) {
      m_ptval -= i;
      return *this;
   }


   /*! Addition operator.

   i
      Count of positions by which to advance the iterator.
   return
      Iterator that’s i items ahead of *this.
   */
   pointer_iterator operator+(std::ptrdiff_t i) const {
      return pointer_iterator(m_ptval + i);
   }


   /*! Subtraction/difference operator.

   i
      Count of positions by which to rewind the iterator.
   it
      Iterator from which to calculate the distance.
   return
      Iterator that’s i items behind *this (subtraction) or distance between *this and it
      (difference).
   */
   pointer_iterator operator-(std::ptrdiff_t i) const {
      return pointer_iterator(m_ptval - i);
   }
   template <typename TVal2>
   std::ptrdiff_t operator-(pointer_iterator<TCont, TVal2> it) const {
      return m_ptval - it.base();
   }


   /*! Preincrement operator.

   return
      *this after it’s moved to the value following the one currently pointed to by.
   */
   pointer_iterator & operator++() {
      ++m_ptval;
      return *this;
   }


   /*! Postincrement operator.

   return
      Iterator pointing to the value following the one pointed to by this iterator.
   */
   pointer_iterator operator++(int) {
      return pointer_iterator(m_ptval++);
   }


   /*! Predecrement operator.

   return
      *this after it’s moved to the value preceding the one currently pointed to by.
   */
   pointer_iterator & operator--() {
      --m_ptval;
      return *this;
   }


   /*! Postdecrement operator.

   return
      Iterator pointing to the value preceding the one pointed to by this iterator.
   */
   pointer_iterator operator--(int) {
      return pointer_iterator(m_ptval--);
   }


// Relational operators.
#define ABC_RELOP_IMPL(op) \
   template <typename TVal2> \
   bool operator op(pointer_iterator<TCont, TVal2> const & it) const { \
      return m_ptval op it.base(); \
   }
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
ABC_RELOP_IMPL(>)
ABC_RELOP_IMPL(>=)
ABC_RELOP_IMPL(<)
ABC_RELOP_IMPL(<=)
#undef ABC_RELOP_IMPL


   /*! Returns the underlying iterator type.

   return
      Pointer to the value pointed to by this iterator.
   */
   TVal * base() const {
      return m_ptval;
   }


protected:

   //! Underlying pointer to the current item.
   TVal * m_ptval;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

