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
// abc::_codepoint_proxy


namespace abc {
namespace text {

/** TODO: comment.
*/
class _codepoint_proxy {
public:

   /** Constructor.

   pch
      Pointer to the character(s) that this proxy will present as char32_t.
   ps
      Pointer to the string that contains *pch.
   cpp
      Code point proxy to replicate.
   */
   _codepoint_proxy(char_t * pch, str_base * ps) :
      m_pch(pch),
      m_ps(ps) {
   }
   _codepoint_proxy(_codepoint_proxy const & cpp) :
      m_pch(cpp.m_pch),
      m_ps(cpp.m_ps) {
   }


   /** Assignment operator.

   ch
      Source character.
   cpp
      Source code point proxy to copy a code point from.
   return
      *this.
   */
   _codepoint_proxy & operator=(char_t ch) {
      *m_pch = ch;
      return *this;
   }
   _codepoint_proxy & operator=(char32_t ch) {
      // TODO: convert to char32_t for real.
      *m_pch = char_t(ch);
      return *this;
   }
   _codepoint_proxy & operator=(_codepoint_proxy const & cpp) {
      char32_t cp(cpp.operator char32_t());
      return operator=(cp);
   }


   /** Implicit conversion to a code point.

   return
      Code point that the proxy is currently referencing.
   */
   operator char32_t() const {
      // TODO: convert to char32_t for real.
      return codepoint(*m_pch);
   }


protected:

   char_t * m_pch;
   str_base * m_ps;
};

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_codepoint_iterator_impl


namespace abc {
namespace text {

/** Base class for codepoint_iterator. Its specializations include all the members that don’t return
an iterator or a reference to one, so that those are only defined once in the “real” template
codepoint_iterator instead of once for each specialization.
*/
template <bool t_bConst>
class _codepoint_iterator_impl;

// Const specialization.
template <>
class ABACLADE_SYM _codepoint_iterator_impl<true> {
public:

   /** Dereferencing operator.

   return
      Reference to the current item.
   */
   char32_t operator*() const {
      // TODO: convert to char32_t for real.
      return codepoint(*m_pch);
   }


   /** Element access operator.

   i
      Index relative to *this.
   return
      Reference to the specified item.
   */
   char32_t operator[](ptrdiff_t i) const {
      // TODO: convert to char32_t for real.
      return codepoint(m_pch[i]);
   }


   /** Returns the underlying iterator type.

   return
      Pointer to the value pointed to by this iterator.
   */
   char_t const * base() const {
      return m_pch;
   }


   /** Returns the string that created this iterator.

   return
      Pointer to the value pointed to by this iterator.
   */
   str_base const * _str() const {
      return m_ps;
   }


protected:

   /** Constructor.

   pch
      Pointer to set the iterator to.
   ps
      Pointer to the string that is creating the iterator.
   */
   _codepoint_iterator_impl(char_t const * pch, str_base const * ps) :
      m_pch(pch),
      m_ps(ps) {
   }


   /** Computes the distance from another iterator/pointer.

   pch
      Pointer from which to calculate the distance.
   return
      Distance between *this and pch, in code points.
   */
   ptrdiff_t distance(char_t const * pch) const;


   /** Advances or rewinds the iterator by the specified number of code points. If the iterator is
   moved outside of the interval [begin, end) of the string it refers to, the results are undefined
   and most likely catastrophic.

   i
      Count of code points to move by.
   */
   void modify(ptrdiff_t i);


protected:

   char_t const * m_pch;
   str_base const * m_ps;
};

// Non-const specialization.
template <>
class _codepoint_iterator_impl<false> :
   public _codepoint_iterator_impl<true> {

   typedef _codepoint_iterator_impl<true> const_impl;

public:

   /** See const_impl::operator*().
   */
   _codepoint_proxy operator*() {
      return _codepoint_proxy(base(), _str());
   }
   char32_t operator*() const {
      return const_impl::operator*();
   }


   /** See const_impl::operator[]().
   */
   char_t & operator[](ptrdiff_t i) {
      // TODO: change to return a proxy that allows assignments as char32_t.
      return const_cast<char_t *>(m_pch)[i];
   }
   char32_t operator[](ptrdiff_t i) const {
      return const_impl::operator[](i);
   }


   /** See const_impl::base().
   */
   char_t * base() const {
      return const_cast<char_t *>(m_pch);
   }


   /** See const_impl::_str().
   */
   str_base * _str() const {
      return const_cast<str_base *>(m_ps);
   }


protected:

   /** See const_impl::const_impl().
   */
   _codepoint_iterator_impl(char_t * pch, str_base * ps) :
      const_impl(pch, ps) {
   }
};

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::codepoint_iterator


namespace abc {
namespace text {

/** Character iterator that hides the underlying encoded representation, presenting a string as an
array of code points (char32_t). Pointers/references are still char_t.
*/
template <bool t_bConst>
class codepoint_iterator :
   public _codepoint_iterator_impl<t_bConst>,
   public std::iterator<
      std::random_access_iterator_tag,
      typename std::conditional<t_bConst, char_t const, char_t>::type
   > {
public:

   /** Constructor.

   pch
      Pointer to set the iterator to.
   ps
      Pointer to the string that is creating the iterator.
   it
      Source iterator.
   */
   /*constexpr*/ codepoint_iterator() :
      _codepoint_iterator_impl<t_bConst>(nullptr, nullptr) {
   }
   codepoint_iterator(
      typename std::conditional<t_bConst, char_t const, char_t>::type * pch,
      typename std::conditional<t_bConst, str_base const, str_base>::type * ps
   ) :
      _codepoint_iterator_impl<t_bConst>(pch, ps) {
   }
   // Allows to convert from non-const to const iterator types.
   template <bool t_bConst2>
   codepoint_iterator(codepoint_iterator<t_bConst2> const & it) :
      _codepoint_iterator_impl<t_bConst>(it.base(), it._str()) {
   }


   /** Addition-assignment operator.

   i
      Count of positions by which to advance the iterator.
   return
      *this after it’s moved forward by i positions.
   */
   codepoint_iterator & operator+=(ptrdiff_t i) {
      this->modify(i);
      return *this;
   }


   /** Subtraction-assignment operator.

   i
      Count of positions by which to rewind the iterator.
   return
      *this after it’s moved backwards by i positions.
   */
   codepoint_iterator & operator-=(ptrdiff_t i) {
      this->modify(-i);
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
      it.modify(i);
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
      it.modify(-i);
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
      this->modify(1);
      return *this;
   }


   /** Postincrement operator.

   return
      Iterator pointing to the value following the one pointed to by this iterator.
   */
   codepoint_iterator operator++(int) {
      codepoint_iterator it(*this);
      this->modify(1);
      return std::move(it);
   }


   /** Predecrement operator.

   return
      *this after it’s moved to the value preceding the one currently pointed to by.
   */
   codepoint_iterator & operator--() {
      this->modify(-1);
      return *this;
   }


   /** Postdecrement operator.

   return
      Iterator pointing to the value preceding the one pointed to by this iterator.
   */
   codepoint_iterator operator--(int) {
      codepoint_iterator it(*this);
      this->modify(-1);
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

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

