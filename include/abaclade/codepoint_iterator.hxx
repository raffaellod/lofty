/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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
// abc::_codepoint_iterator_impl


namespace abc {

/** Base class for codepoint_iterator. Its specializations include all the members that don’t return
an iterator or a reference to one, so that those are only defined once in the “real” template
codepoint_iterator instead of once for each specialization.
*/
template <bool t_bConst>
class _codepoint_iterator_impl;

// Const specialization.
template <>
class _codepoint_iterator_impl<true> {
public:

   /** Type of encoded character. */
   typedef char_t const character;
   /** Type of code point. */
   typedef char_t const codepoint;


public:

   /** Dereferencing operator.

   return
      Reference to the current item.
   */
   character const & operator*() const {
      return *m_pch;
   }


   /** Element access operator.

   i
      Index relative to *this.
   return
      Reference to the specified item.
   */
   character const & operator[](ptrdiff_t i) const {
      return m_pch[i];
   }


   /** Returns the underlying iterator type.

   return
      Pointer to the value pointed to by this iterator.
   */
   character const * base() const {
      return m_pch;
   }


protected:

   /** Constructor.

   pch
      Pointer to set the iterator to.
   */
   explicit _codepoint_iterator_impl(character const * pch) :
      m_pch(pch) {
   }


   /** Advances the iterator by the specified number of code points.

   i
      Count of code points to advance by.
   */
   void add(ptrdiff_t i) {
      m_pch += i;
   }


   /** Rewinds the iterator by the specified number of code points.

   i
      Count of code points to rewind by.
   */
   void subtract(ptrdiff_t i) {
      m_pch -= i;
   }


   /** Computes the distance from another iterator/pointer.

   pch
      Pointer from which to calculate the distance.
   return
      Distance between *this and pch, in code points.
   */
   ptrdiff_t distance(character const * pch) const {
      return m_pch - pch;
   }


protected:

   character const * m_pch;
};

// Non-const specialization.
template <>
class _codepoint_iterator_impl<false> :
   public _codepoint_iterator_impl<true> {

   typedef _codepoint_iterator_impl<true> const_impl;

public:

   /** See const_impl::character. */
   typedef char_t character;
   /** See const_impl::codepoint. */
   typedef char_t codepoint;


public:

   /** See const_impl::operator*().
   */
   character & operator*() const {
      return const_cast<character &>(const_impl::operator*());
   }


   /** See const_impl::operator[]().
   */
   character const & operator[](ptrdiff_t i) const {
      return const_cast<character &>(const_impl::operator[](i));
   }


   /** See const_impl::base().
   */
   character * base() const {
      return const_cast<character *>(const_impl::base());
   }


protected:

   /** See const_impl::const_impl().
   */
   explicit _codepoint_iterator_impl(character * pch) :
      const_impl(pch) {
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::codepoint_iterator


namespace abc {

/** Character iterator that hides the underlying encoded representation, presenting a string as an
array of code points (char32_t). Pointers/references are still char_t.
*/
template <bool t_bConst>
class codepoint_iterator :
   public _codepoint_iterator_impl<t_bConst>,
   public std::iterator<
      std::random_access_iterator_tag, typename _codepoint_iterator_impl<t_bConst>::character
   > {

   typedef _codepoint_iterator_impl<t_bConst> cpii;

public:

   /** Constructor.

   pch
      Pointer to set the iterator to.
   it
      Source iterator.
   */
   /*constexpr*/ codepoint_iterator() :
      cpii(nullptr) {
   }
   explicit codepoint_iterator(typename cpii::codepoint * pch) :
      cpii(pch) {
   }
   // Allows to convert between non-const to const iterator types.
   template <bool t_bConst2>
   codepoint_iterator(codepoint_iterator<t_bConst2> const & it) :
      cpii(it.base()) {
   }


   /** Addition-assignment operator.

   i
      Count of positions by which to advance the iterator.
   return
      *this after it’s moved forward by i positions.
   */
   codepoint_iterator & operator+=(ptrdiff_t i) {
      this->add(i);
      return *this;
   }


   /** Subtraction-assignment operator.

   i
      Count of positions by which to rewind the iterator.
   return
      *this after it’s moved backwards by i positions.
   */
   codepoint_iterator & operator-=(ptrdiff_t i) {
      this->subtract(i);
      return *this;
   }


   /** Addition operator.

   i
      Count of positions by which to advance the iterator.
   return
      Iterator that’s i items ahead of *this.
   */
   codepoint_iterator operator+(ptrdiff_t i) const {
      codepoint_iterator it(*this);
      it.add(i);
      return std::move(it);
   }


   /** Subtraction/difference operator.

   i
      Count of positions by which to rewind the iterator.
   it
      Iterator from which to calculate the distance.
   return
      Iterator that’s i items behind *this (subtraction) or distance between *this and it, in code
      points (difference).
   */
   codepoint_iterator operator-(ptrdiff_t i) const {
      codepoint_iterator it(*this);
      it.subtract(i);
      return std::move(it);
   }
   template <bool t_bConst2>
   ptrdiff_t operator-(codepoint_iterator<t_bConst2> it) const {
      return this->distance(it.base());
   }


   /** Preincrement operator.

   return
      *this after it’s moved to the value following the one currently pointed to by.
   */
   codepoint_iterator & operator++() {
      this->add(1);
      return *this;
   }


   /** Postincrement operator.

   return
      Iterator pointing to the value following the one pointed to by this iterator.
   */
   codepoint_iterator operator++(int) {
      codepoint_iterator it(*this);
      this->add(1);
      return std::move(it);
   }


   /** Predecrement operator.

   return
      *this after it’s moved to the value preceding the one currently pointed to by.
   */
   codepoint_iterator & operator--() {
      this->subtract(1);
      return *this;
   }


   /** Postdecrement operator.

   return
      Iterator pointing to the value preceding the one pointed to by this iterator.
   */
   codepoint_iterator operator--(int) {
      codepoint_iterator it(*this);
      this->subtract(1);
      return std::move(it);
   }


// Relational operators.
#define ABC_RELOP_IMPL(op) \
   template <bool t_bConst2> \
   bool operator op(codepoint_iterator<t_bConst2> const & it) const { \
      return this->base() op it.base(); \
   }
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
ABC_RELOP_IMPL(>)
ABC_RELOP_IMPL(>=)
ABC_RELOP_IMPL(<)
ABC_RELOP_IMPL(<=)
#undef ABC_RELOP_IMPL
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

