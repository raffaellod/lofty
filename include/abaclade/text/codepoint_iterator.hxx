/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014, 2015
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

namespace abc { namespace text { namespace detail {

//! Presents an abc::text::str character(s) as a char32_t const &.
class const_codepoint_proxy {
public:
   /*! Constructor.

   @param ps
      Pointer to the containing string.
   @param ich
      Index of the character(s) that this proxy will present as char32_t.
   */
   const_codepoint_proxy(str const * ps, std::size_t ich) :
      mc_ps(ps),
      mc_ich(ich) {
   }

   /*! Implicit conversion to a code point.

   @return
      Code point that the proxy is currently referencing.
   */
   operator char32_t() const;

protected:
   //! Pointer to the containing string.
   str const * const mc_ps;
   //! Index of the proxied character(s).
   std::size_t const mc_ich;
};

//! Presents an abc::text::str character(s) as a char32_t &.
class codepoint_proxy : public const_codepoint_proxy {
public:
   /*! Constructor.

   @param ps
      Pointer to the containing string.
   @param ich
      Index of the character(s) that this proxy will present as char32_t.
   */
   codepoint_proxy(str * ps, std::size_t ich) :
      const_codepoint_proxy(ps, ich) {
   }

   /*! Assignment operator.

   @param ch
      Source character.
   @return
      *this.
   */
   codepoint_proxy & operator=(char_t ch);

#if ABC_HOST_UTF > 8
   /*! Assignment operator that accepts ASCII characters.

   @param ch
      Source ASCII character.
   @return
      *this.
   */
   codepoint_proxy & operator=(char ch) {
      return operator=(host_char(ch));
   }
#endif

   /*! Assignment operator that accepts a code point.

   @param cp
      Source code point.
   @return
      *this.
   */
   codepoint_proxy & operator=(char32_t cp);

   /*! Copy-assignment operator. This copies the char32_t value, not the internal data members; this
   allows writing expressions like *itDst = *itSrc to copy code points from one iterator to another.

   @param cpp
      Source object to copy a code point from.
   @return
      *this.
   */
   codepoint_proxy & operator=(codepoint_proxy const & cpp) {
      return operator=(cpp.operator char32_t());
   }

   /*! Copy-assignment operator. This copies the char32_t value, not the internal data members; this
   allows writing expressions like *itDst = *itSrc to copy code points from one iterator to another.
   This overload supports copying a code point from a const iterator to a non-const iterator.

   @param cpp
      Source object to copy a code point from.
   @return
      *this.
   */
   codepoint_proxy & operator=(const_codepoint_proxy const & cpp) {
      return operator=(cpp.operator char32_t());
   }
};

/* Relational operators. Provided so that comparisons between char32_t (from codepoint_proxy) and
char{,8,16}_t don’t raise warnings. */
#define ABC_RELOP_IMPL(op) \
   inline bool operator op( \
      const_codepoint_proxy const & cppL, const_codepoint_proxy const & cppR \
   ) { \
      return cppL.operator char32_t() op cppR.operator char32_t(); \
   } \
   inline bool operator op(const_codepoint_proxy const & cpp, char_t ch) { \
      return cpp.operator char32_t() op codepoint(ch); \
   } \
   inline bool operator op(char_t ch, const_codepoint_proxy const & cpp) { \
      return codepoint(ch) op cpp.operator char32_t(); \
   } \
   inline bool operator op(const_codepoint_proxy const & cpp, char32_t cp) { \
      return cpp.operator char32_t() op cp; \
   } \
   inline bool operator op(char32_t cp, const_codepoint_proxy const & cpp) { \
      return cp op cpp.operator char32_t(); \
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
      inline bool operator op(const_codepoint_proxy const & cpp, char ch) { \
         return operator op(cpp, host_char(ch)); \
      } \
      inline bool operator op(char ch, const_codepoint_proxy const & cpp) { \
         return operator op(host_char(ch), cpp); \
      }
   ABC_RELOP_IMPL(==)
   ABC_RELOP_IMPL(!=)
   ABC_RELOP_IMPL(>)
   ABC_RELOP_IMPL(>=)
   ABC_RELOP_IMPL(<)
   ABC_RELOP_IMPL(<=)
   #undef ABC_RELOP_IMPL
#endif

}}} //namespace abc::text::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text { namespace detail {

/*! Base class for codepoint_iterator. Its specializations include all the members that don’t return
an iterator or a reference to one, so that those are only defined once in the “real” template
codepoint_iterator instead of once for each specialization. */
template <bool t_bConst>
class codepoint_iterator_impl;

// Const specialization.
template <>
class ABACLADE_SYM codepoint_iterator_impl<true> {
public:
   /*! Dereferencing operator.

   @return
      Reference to the current character.
   */
   const_codepoint_proxy operator*() const {
      return const_codepoint_proxy(m_ps, throw_if_end(m_ich));
   }

   /*! Element access operator.

   @param i
      Index relative to *this. If the resulting index is outside of the string’s [begin, end) range,
      an index_error exception will be thrown.
   @return
      Reference to the specified item.
   */
   const_codepoint_proxy operator[](std::ptrdiff_t i) const {
      return const_codepoint_proxy(m_ps, throw_if_end(advance(i, true)));
   }

   /*! Returns the underlying iterator type.

   @return
      Pointer to the value pointed to by this iterator.
   */
   char_t const * base() const;

protected:
   /*! Constructor.

   @param ich
      Index to set the iterator to.
   @param ps
      Pointer to the string that is creating the iterator.
   */
   codepoint_iterator_impl(std::size_t ich, str const * ps) :
      m_ich(ich),
      m_ps(const_cast<str *>(ps)) {
   }

   //! Invokes m_ps->_advance_char_index(). See str::_advance_char_index().
   std::size_t advance(std::ptrdiff_t iDelta, bool bIndex) const;

   /*! Computes the distance from another iterator/index.

   @param ich
      Index from which to calculate the distance.
   @return
      Distance between *this and ich, in code points.
   */
   std::ptrdiff_t distance(std::size_t ich) const;

   /*! Throws an iterator_error if the specified index is at or beyond the end of the string.

   @param ich
      Index to validate.
   @return
      ich.
   */
   std::size_t throw_if_end(std::size_t ich) const;

public:
   //! Index of the current character.
   std::size_t m_ich;
   //! Pointer to the source string.
   str * m_ps;
};

// Non-const specialization.
template <>
class codepoint_iterator_impl<false> : public codepoint_iterator_impl<true> {
public:
   //! See codepoint_iterator_impl<true>::operator*().
   using codepoint_iterator_impl<true>::operator*;
   codepoint_proxy operator*() {
      return codepoint_proxy(const_cast<str *>(m_ps), throw_if_end(m_ich));
   }

   //! See codepoint_iterator_impl<true>::operator[]().
   using codepoint_iterator_impl<true>::operator[];
   codepoint_proxy operator[](std::ptrdiff_t i) {
      return codepoint_proxy(const_cast<str *>(m_ps), throw_if_end(advance(i, true)));
   }

   //! See codepoint_iterator_impl<true>::base().
   char_t * base() const;

protected:
   //! See codepoint_iterator_impl<true>::codepoint_iterator_impl().
   codepoint_iterator_impl(std::size_t ich, str const * ps) :
      codepoint_iterator_impl<true>(ich, const_cast<str *>(ps)) {
   }
};

}}} //namespace abc::text::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text {

/*! Character iterator that hides the underlying encoded representation, presenting a string as an
array of code points (char32_t). Pointers/references are still char_t. */
template <bool t_bConst>
class codepoint_iterator :
   public detail::codepoint_iterator_impl<t_bConst>,
   public _std::iterator<
      _std::random_access_iterator_tag,
      typename _std::conditional<t_bConst, char_t const, char_t>::type
   > {
public:
   //! Default constructor.
   /*constexpr*/ codepoint_iterator() :
      detail::codepoint_iterator_impl<t_bConst>(0, nullptr) {
   }

   /*! Constructor.

   @param ich
      Character index to set the iterator to.
   @param ps
      Pointer to the string that is creating the iterator.
   */
   codepoint_iterator(
      std::size_t ich, typename _std::conditional<t_bConst, str const, str>::type * ps
   ) :
      detail::codepoint_iterator_impl<t_bConst>(ich, ps) {
   }

   /*! Copy constructor. Allows to convert from non-const to const iterator types.

   @param it
      Source object.
   */
   template <bool t_bConst2>
   codepoint_iterator(codepoint_iterator<t_bConst2> const & it) :
      detail::codepoint_iterator_impl<t_bConst>(it) {
   }

   /*! Addition-assignment operator.

   @param i
      Count of positions by which to advance the iterator. If the resulting iterator is outside of
      the string’s [begin, end] range, an iterator_error exception will be thrown.
   @return
      *this after it’s moved forward by i positions.
   */
   codepoint_iterator & operator+=(std::ptrdiff_t i) {
      this->m_ich = this->advance(i, false);
      return *this;
   }

   /*! Subtraction-assignment operator.

   @param i
      Count of positions by which to rewind the iterator. If the resulting iterator is outside of
      the string’s [begin, end] range, an iterator_error exception will be thrown.
   @return
      *this after it’s moved backwards by i positions.
   */
   codepoint_iterator & operator-=(std::ptrdiff_t i) {
      this->m_ich = this->advance(-i, false);
      return *this;
   }

   /*! Addition operator.

   @param i
      Count of positions by which to advance the iterator. If the resulting iterator is outside of
      the string’s [begin, end] range, an iterator_error exception will be thrown.
   @return
      Iterator that’s i items ahead of *this.
   */
   codepoint_iterator operator+(std::ptrdiff_t i) const {
      return codepoint_iterator(this->advance(i, false), this->m_ps);
   }

   /*! Subtraction/difference operator.

   @param i
      Count of positions by which to rewind the iterator. If the resulting iterator is outside of
      the string’s [begin, end] range, an iterator_error exception will be thrown.
   @param it
      Iterator from which to calculate the distance.
   @return
      Iterator that’s i items behind *this (subtraction) or distance between *this and it, in code
      points (difference).
   */
   codepoint_iterator operator-(std::ptrdiff_t i) const {
      return codepoint_iterator(this->advance(-i, false), this->m_ps);
   }
   template <bool t_bConst2>
   std::ptrdiff_t operator-(codepoint_iterator<t_bConst2> it) const {
      return this->distance(it.m_ich);
   }

   /*! Preincrement operator. If the resulting iterator was already at the string’s end, an
   iterator_error exception will be thrown.

   @return
      *this after it’s moved to the value following the one currently pointed to.
   */
   codepoint_iterator & operator++() {
      this->m_ich = this->advance(1, false);
      return *this;
   }

   /*! Postincrement operator. If the resulting iterator was already at the string’s end, an
   iterator_error exception will be thrown.

   @return
      Iterator pointing to the value following the one pointed to by this iterator.
   */
   codepoint_iterator operator++(int) {
      auto ich(this->m_ich);
      this->m_ich = this->advance(1, false);
      return codepoint_iterator(ich, this->m_ps);
   }

   /*! Predecrement operator. If the resulting iterator was already at the string’s beginning, an
   iterator_error exception will be thrown.

   @return
      *this after it’s moved to the value preceding the one currently pointed to.
   */
   codepoint_iterator & operator--() {
      this->m_ich = this->advance(-1, false);
      return *this;
   }

   /*! Postdecrement operator. If the resulting iterator was already at the string’s beginning, an
   iterator_error exception will be thrown.

   @return
      Iterator pointing to the value preceding the one pointed to by this iterator.
   */
   codepoint_iterator operator--(int) {
      auto ich(this->m_ich);
      this->m_ich = this->advance(-1, false);
      return codepoint_iterator(ich, this->m_ps);
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

}} //namespace abc::text
