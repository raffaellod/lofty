/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015
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
// abc::collections::detail::xor_list

namespace abc {
namespace collections {
namespace detail {

//! Defines classes useful to implement XOR-linked list classes.
class xor_list {
public:
   //! Node for XOR doubly-linked list classes.
   class node {
   public:
      //! Constructor.
      node() {
      }
      node(node const &) {
         // Skip copying the source’s links.
      }

      /*! Assignment operator.

      @return
         *this.
      */
      node & operator=(node const &) {
         // Skip copying the source’s links.
         return *this;
      }

      /*! Returns a pointer to the next node.

      @param pnPrev
         Pointer to the previous node.
      */
      node * get_next(node * pnPrev) {
         return reinterpret_cast<node *>(m_iPrevXorNext ^ reinterpret_cast<std::uintptr_t>(pnPrev));
      }

      /*! Returns a pointer to the previous node.

      @param pnNext
         Pointer to the next node.
      */
      node * get_prev(node * pnNext) {
         return reinterpret_cast<node *>(m_iPrevXorNext ^ reinterpret_cast<std::uintptr_t>(pnNext));
      }

      /*! Updates the previous/next pointer.

      @param pnPrev
         Pointer to the previous node.
      @param pnNext
         Pointer to the next node.
      */
      void set_prev_next(node * pnPrev, node * pnNext) {
         m_iPrevXorNext = reinterpret_cast<std::uintptr_t>(pnPrev) ^
                          reinterpret_cast<std::uintptr_t>(pnNext);
      }

   private:
      //! Pointer to the previous node XOR pointer to the next node.
      std::uintptr_t m_iPrevXorNext;
   };

protected:
   //! Non-template base for iterator.
   class ABACLADE_SYM iterator_base {
   public:
      /*! Equality relational operator.

      @param it
         Object to compare to *this.
      @return
         true if *this refers to the same element as it, or false otherwise.
      */
      bool operator==(iterator_base const & it) const {
         return m_pnCurr == it.m_pnCurr;
      }

      /*! Inequality relational operator.

      @param it
         Object to compare to *this.
      @return
         true if *this refers to a different element than it, or false otherwise.
      */
      bool operator!=(iterator_base const & it) const {
         return !operator==(it);
      }

   protected:
      /*! Constructor.

      @param pnPrev
         Pointer to the node preceding *pnCurr.
      @param pnCurr
         Pointer to the current node.
      @param pnNext
         Pointer to the node following *pnCurr.
      */
      iterator_base(node * pnPrev, node * pnCurr, node * pnNext) :
         m_pnPrev(pnPrev),
         m_pnCurr(pnCurr),
         m_pnNext(pnNext) {
      }

      //! Moves the iterator backwards.
      void decrement();

      //! Moves the iterator forwards.
      void increment();

   protected:
      //! Pointer to the previous node.
      node * m_pnPrev;
      //! Pointer to the current node.
      node * m_pnCurr;
      //! Pointer to the next node.
      node * m_pnNext;
   };

public:
   //! Iterator for XOR doubly-linked list node classes.
   template <typename TNode, typename TValue, bool t_bConst = std::is_const<TValue>::value>
   class iterator;

   // Partial specialization for const TValue.
   template <typename TNode, typename TValue>
   class iterator<TNode, TValue, true> :
      public iterator_base,
      public std::iterator<std::bidirectional_iterator_tag, TValue> {
   public:
      //! See iterator_base::iterator_base().
      iterator(node * pnPrev, node * pnCurr, node * pnNext) :
         iterator_base(pnPrev, pnCurr, pnNext) {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current node.
      */
      TValue & operator*() const {
         return *static_cast<TNode *>(m_pnCurr)->value_ptr();
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current node.
      */
      TValue * operator->() const {
         return static_cast<TNode *>(m_pnCurr)->value_ptr();
      }

      /*! Preincrement operator.

      @return
         *this.
      */
      iterator & operator++() {
         increment();
         return *this;
      }

      /*! Postincrement operator.

      @return
         Iterator pointing to the node following the one pointed to by this iterator.
      */
      iterator operator++(int) {
         node * pnPrevPrev = m_pnPrev;
         increment();
         return iterator(pnPrevPrev, m_pnPrev, m_pnCurr);
      }

      /*! Predecrement operator.

      @return
         *this.
      */
      iterator & operator--() {
         decrement();
         return *this;
      }

      /*! Postdecrement operator.

      @return
         Iterator pointing to the node preceding the one pointed to by this iterator.
      */
      iterator operator--(int) {
         node * pnNextNext = m_pnNext;
         decrement();
         return iterator(m_pnCurr, m_pnNext, pnNextNext);
      }

      /*! Returns the underlying pointer to the node.

      @return
         Pointer to the current node.
      */
      TNode const * base() const {
         return static_cast<TNode *>(m_pnCurr);
      }

      /*! Returns a pointer to the next node.

      @return
         Pointer to the next node.
      */
      TNode const * next_base() const {
         return static_cast<TNode *>(m_pnNext);
      }

      /*! Returns a pointer to the previous node.

      @return
         Pointer to the previous node.
      */
      TNode const * prev_base() const {
         return static_cast<TNode *>(m_pnPrev);
      }
   };

   // Partial specialization for non-const TValue.
   template <typename TNode, typename TValue>
   class iterator<TNode, TValue, false> :
      public iterator<TNode, typename std::add_const<TValue>::type, true>,
      public std::iterator<std::bidirectional_iterator_tag, TValue> {
   private:
      // Shortcuts.
      typedef iterator<TNode, typename std::add_const<TValue>::type, true> const_iterator;
      typedef std::iterator<std::bidirectional_iterator_tag, TValue> std_iterator;

   public:
      // These are inherited from both base classes, so resolve the ambiguity.
      using typename std_iterator::difference_type;
      using typename std_iterator::iterator_category;
      using typename std_iterator::pointer;
      using typename std_iterator::reference;
      using typename std_iterator::value_type;

   public:
      //! See const_iterator::const_iterator().
      iterator(node * pnPrev, node * pnCurr, node * pnNext) :
         const_iterator(pnPrev, pnCurr, pnNext) {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current node.
      */
      TValue & operator*() const {
         return const_cast<TValue &>(const_iterator::operator*());
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current node.
      */
      TValue * operator->() const {
         return const_cast<TValue *>(const_iterator::operator->());
      }

      /*! Preincrement operator.

      @return
         *this.
      */
      iterator & operator++() {
         return static_cast<iterator &>(const_iterator::operator++());
      }

      /*! Postincrement operator.

      @return
         Iterator pointing to the node following the one pointed to by this iterator.
      */
      iterator operator++(int) {
         return static_cast<iterator &>(const_iterator::operator++(0));
      }

      /*! Predecrement operator.

      @return
         *this.
      */
      iterator & operator--() {
         return static_cast<iterator &>(const_iterator::operator--());
      }

      /*! Postdecrement operator.

      @return
         Iterator pointing to the node preceding the one pointed to by this iterator.
      */
      iterator operator--(int) {
         return static_cast<iterator &>(const_iterator::operator--(0));
      }
   };
};

} //namespace detail
} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
