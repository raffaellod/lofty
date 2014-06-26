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
// abc::text::_codepoint_proxy


namespace abc {
namespace text {

/** TODO: comment.
*/
template <bool t_bConst>
class _codepoint_proxy;

// Const specialization.
template <>
class _codepoint_proxy<true> {
public:

   /** Constructor.

   pch
      Pointer to the character(s) that this proxy will present as char32_t.
   ps
      Pointer to the string that contains *pch.
   cpp
      Source code point proxy to copy.
   */
   _codepoint_proxy(char_t const * pch, str_base const * ps) :
      m_pch(pch),
      m_ps(ps) {
   }
   _codepoint_proxy(_codepoint_proxy const & cpp) :
      m_pch(cpp.m_pch),
      m_ps(cpp.m_ps) {
   }


   /** Implicit conversion to a code point.

   return
      Code point that the proxy is currently referencing.
   */
   operator char32_t() const {
      return host_char_traits::chars_to_codepoint(m_pch);
   }


private:

   _codepoint_proxy & operator=(_codepoint_proxy const & cpp);


protected:

   char_t const * m_pch;
   str_base const * m_ps;
};

// Non-const specialization.
template <>
class _codepoint_proxy<false> :
   public _codepoint_proxy<true> {
public:

   /** See _codepoint_proxy<true>::_codepoint_proxy().
   */
   _codepoint_proxy(char_t * pch, str_base * ps) :
      _codepoint_proxy<true>(pch, ps) {
   }
   _codepoint_proxy(_codepoint_proxy const & cpp) :
      _codepoint_proxy<true>(cpp) {
   }


   /** Assignment operator.

   ch
      Source character.
   cpp
      Source code point proxy to copy a code point from.
   return
      *this.
   */
   _codepoint_proxy & operator=(char_t ch);
#if ABC_HOST_UTF > 8
   _codepoint_proxy & operator=(char ch) {
      return operator=(host_char(ch));
   }
#endif
   _codepoint_proxy & operator=(char32_t ch);
   _codepoint_proxy & operator=(_codepoint_proxy const & cpp) {
      return operator=(cpp.operator char32_t());
   }
};

} //namespace text
} //namespace abc


// Relational operators. Provided so that comparisons between char32_t (from _codepoint_proxy) and
// char{,8,16}_t don’t raise warnings.
#define ABC_RELOP_IMPL(op) \
   template <bool t_bConst1, bool t_bConst2> \
   inline bool operator op( \
      abc::text::_codepoint_proxy<t_bConst1> const & cpp1, \
      abc::text::_codepoint_proxy<t_bConst2> const & cpp2 \
   ) { \
      return cpp1.operator char32_t() op cpp2.operator char32_t(); \
   } \
   template <bool t_bConst> \
   inline bool operator op(abc::text::_codepoint_proxy<t_bConst> const & cpp, abc::char_t ch) { \
      return cpp.operator char32_t() op abc::text::codepoint(ch); \
   } \
   template <bool t_bConst> \
   inline bool operator op(abc::char_t ch, abc::text::_codepoint_proxy<t_bConst> const & cpp) { \
      return abc::text::codepoint(ch) op cpp.operator char32_t(); \
   } \
   template <bool t_bConst> \
   inline bool operator op(abc::text::_codepoint_proxy<t_bConst> const & cpp, char32_t ch) { \
      return cpp.operator char32_t() op ch; \
   } \
   template <bool t_bConst> \
   inline bool operator op(char32_t ch, abc::text::_codepoint_proxy<t_bConst> const & cpp) { \
      return ch op cpp.operator char32_t(); \
   }
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
ABC_RELOP_IMPL(>)
ABC_RELOP_IMPL(>=)
ABC_RELOP_IMPL(<)
ABC_RELOP_IMPL(<=)
#undef ABC_RELOP_IMPL

#if ABC_HOST_UTF > 8
   #define ABC_RELOP_IMPL(op) \
      template <bool t_bConst> \
      inline bool operator op(abc::text::_codepoint_proxy<t_bConst> const & cpp, char ch) { \
         return operator op(cpp, abc::text::host_char(ch)); \
      } \
      template <bool t_bConst> \
      inline bool operator op(char ch, abc::text::_codepoint_proxy<t_bConst> const & cpp) { \
         return operator op(abc::text::host_char(ch), cpp); \
      }
   ABC_RELOP_IMPL(==)
   ABC_RELOP_IMPL(!=)
   ABC_RELOP_IMPL(>)
   ABC_RELOP_IMPL(>=)
   ABC_RELOP_IMPL(<)
   ABC_RELOP_IMPL(<=)
   #undef ABC_RELOP_IMPL
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::_codepoint_iterator_impl


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
      Reference to the current character.
   */
   _codepoint_proxy<true> operator*() const {
      return _codepoint_proxy<true>(base(), _str());
   }


   /** Element access operator.

   i
      Index relative to *this. If the resulting index is outside of the string’s [begin, end) range,
      an index_error exception will be thrown.
   return
      Reference to the specified item.
   */
   _codepoint_proxy<true> operator[](ptrdiff_t i) const {
      return _codepoint_proxy<true>(advance(i, true), _str());
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


   /** Invokes m_ps->_advance_char_ptr(). See abc::str_base::_advance_char_ptr().
   */
   char_t const * advance(ptrdiff_t i, bool bIndex) const;


   /** Computes the distance from another iterator/pointer.

   pch
      Pointer from which to calculate the distance.
   return
      Distance between *this and pch, in code points.
   */
   ptrdiff_t distance(char_t const * pch) const;


protected:

   char_t const * m_pch;
   str_base const * m_ps;
};

// Non-const specialization.
template <>
class _codepoint_iterator_impl<false> :
   public _codepoint_iterator_impl<true> {
public:

   /** See _codepoint_iterator_impl<true>::operator*().
   */
   _codepoint_proxy<false> operator*() {
      return _codepoint_proxy<false>(base(), _str());
   }
   _codepoint_proxy<true> operator*() const {
      return _codepoint_proxy<true>(base(), _str());
   }


   /** See _codepoint_iterator_impl<true>::operator[]().
   */
   _codepoint_proxy<false> operator[](ptrdiff_t i) {
      return _codepoint_proxy<false>(advance(i, true), _str());
   }
   _codepoint_proxy<true> operator[](ptrdiff_t i) const {
      return _codepoint_proxy<true>(advance(i, true), _str());
   }


   /** See _codepoint_iterator_impl<true>::base().
   */
   char_t * base() const {
      return const_cast<char_t *>(m_pch);
   }


   /** See _codepoint_iterator_impl<true>::_str().
   */
   str_base * _str() const {
      return const_cast<str_base *>(m_ps);
   }


protected:

   /** See _codepoint_iterator_impl<true>::_codepoint_iterator_impl().
   */
   _codepoint_iterator_impl(char_t * pch, str_base * ps) :
      _codepoint_iterator_impl<true>(pch, ps) {
   }


   /** See _codepoint_iterator_impl<true>::advance().
   */
   char_t * advance(ptrdiff_t i, bool bIndex) const {
      return const_cast<char_t *>(_codepoint_iterator_impl<true>::advance(i, bIndex));
   }
};

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::codepoint_iterator


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
      Count of positions by which to advance the iterator. If the resulting iterator is outside of
      the string’s [begin, end] range, an iterator_error exception will be thrown.
   return
      *this after it’s moved forward by i positions.
   */
   codepoint_iterator & operator+=(ptrdiff_t i) {
      this->m_pch = this->advance(i, false);
      return *this;
   }


   /** Subtraction-assignment operator.

   i
      Count of positions by which to rewind the iterator. If the resulting iterator is outside of
      the string’s [begin, end] range, an iterator_error exception will be thrown.
   return
      *this after it’s moved backwards by i positions.
   */
   codepoint_iterator & operator-=(ptrdiff_t i) {
      this->m_pch = this->advance(i, false);
      return *this;
   }


   /** Addition operator.

   i
      Count of positions by which to advance the iterator. If the resulting iterator is outside of
      the string’s [begin, end] range, an iterator_error exception will be thrown.
   return
      Iterator that’s i items ahead of *this.
   */
   codepoint_iterator operator+(ptrdiff_t i) const {
      return codepoint_iterator(this->advance(i, false), this->_str());
   }


   /** Subtraction/difference operator.

   i
      Count of positions by which to rewind the iterator. If the resulting iterator is outside of
      the string’s [begin, end] range, an iterator_error exception will be thrown.
   it
      Iterator from which to calculate the distance.
   return
      Iterator that’s i items behind *this (subtraction) or distance between *this and it, in code
      points (difference).
   */
   codepoint_iterator operator-(ptrdiff_t i) const {
      return codepoint_iterator(this->advance(-i, false), this->_str());
   }
   template <bool t_bConst2>
   ptrdiff_t operator-(codepoint_iterator<t_bConst2> it) const {
      return this->distance(it.base());
   }


   /** Preincrement operator. If the resulting iterator was already at the string’s end, an
   iterator_error exception will be thrown.

   return
      *this after it’s moved to the value following the one currently pointed to by.
   */
   codepoint_iterator & operator++() {
      this->m_pch = this->advance(1, false);
      return *this;
   }


   /** Postincrement operator. If the resulting iterator was already at the string’s end, an
   iterator_error exception will be thrown.

   return
      Iterator pointing to the value following the one pointed to by this iterator.
   */
   codepoint_iterator operator++(int) {
      auto pch(this->base());
      this->m_pch = this->advance(1, false);
      return codepoint_iterator(pch, this->_str());
   }


   /** Predecrement operator. If the resulting iterator was already at the string’s beginning, an
   iterator_error exception will be thrown.

   return
      *this after it’s moved to the value preceding the one currently pointed to by.
   */
   codepoint_iterator & operator--() {
      this->m_pch = this->advance(-1, false);
      return *this;
   }


   /** Postdecrement operator. If the resulting iterator was already at the string’s beginning, an
   iterator_error exception will be thrown.

   return
      Iterator pointing to the value preceding the one pointed to by this iterator.
   */
   codepoint_iterator operator--(int) {
      auto pch(this->base());
      this->m_pch = this->advance(-1, false);
      return codepoint_iterator(pch, this->_str());
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

