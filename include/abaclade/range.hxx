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

#ifndef _ABACLADE_RANGE_HXX
#define _ABACLADE_RANGE_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::range

namespace abc {

/*! Represents an iterable interval of values defined by a beginning and an end, inclusive and
exclusive respectively. */
template <typename T>
class range : public support_explicit_operator_bool<range<T>> {
public:
   //! Iterator for range values.
   class iterator : public std::iterator<std::bidirectional_iterator_tag, T> {
   public:
      /*! Constructor.

      @param t
         Current value.
      */
      explicit iterator(T t) :
         m_t(t) {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current node.
      */
      T operator*() const {
         return m_t;
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current value.
      */
      T * operator->() {
         return &m_t;
      }
      T const * operator->() const {
         return &m_t;
      }

      /*! Preincrement operator.

      @return
         *this after it’s moved to the value following the one currently referenced.
      */
      iterator & operator++() {
         ++m_t;
         return *this;
      }

      /*! Postincrement operator.

      @return
         Iterator to the value following the one referenced by this iterator.
      */
      iterator operator++(int) {
         return iterator(m_t++);
      }

      /*! Predecrement operator.

      @return
         *this after it’s moved to the value preceding the one currently referenced.
      */
      iterator & operator--() {
         --m_t;
         return *this;
      }

      /*! Postdecrement operator.

      @return
         Iterator to the value preceding the one referenced by this iterator.
      */
      iterator operator--(int) {
         return iterator(m_t--);
      }

// Relational operators.
#define ABC_RELOP_IMPL(op) \
      bool operator op(iterator const & it) const { \
         return m_t op it.m_t; \
      }
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
#undef ABC_RELOP_IMPL

      /*! Returns the underlying iterator type.

      @return
         Pointer to the current node.
      */
      T base() const {
         return m_t;
      }

   private:
      //! Current value.
      T m_t;
   };

   typedef std::reverse_iterator<iterator> reverse_iterator;

public:
   /*! Constructor.

   @param tBegin
      First value in the range.
   @param tEnd
      Value beyond the last one in the range.
   */
   range() :
      m_tBegin(),
      m_tEnd() {
   }
   range(T tBegin, T tEnd) :
      m_tBegin(tBegin),
      m_tEnd(tEnd) {
   }

   /*! Boolean evaluation operator.

   @return
      true if the range is non-empty, or false if it’s empty.
   */
   explicit_operator_bool() const {
      return m_tBegin != m_tEnd;
   }

   /*! Returns the interval not included in the range, defined as [end(), begin()).

   @return
      Inverted range.
   */
   range operator~() const {
      return range(m_tEnd, m_tBegin);
   }

   /*! Returns an iterator to the start of the range.

   @return
      Iterator to the first value in the range.
   */
   iterator begin() const {
      return iterator(m_tBegin);
   }

   /*! Returns true if the specified value is included in the range.

   @param t
      Value to check for inclusion.
   @return
      true if t is included in [begin(), end()), or false otherwise.
   */
   bool contains(T t) const {
      return t >= m_tBegin && t < m_tEnd;
   }

   /*! Returns an iterator to the end of the range.

   @return
      Value beyond the last in the range.
   */
   iterator end() const {
      return iterator(m_tEnd);
   }

private:
   //! First value in the range.
   T m_tBegin;
   //! Value beyond the last one in the range.
   T m_tEnd;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_RANGE_HXX
