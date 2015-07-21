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

namespace abc { namespace collections { namespace detail {

//! Defines classes useful to implement XOR-linked list classes.
class ABACLADE_SYM xor_list_impl {
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

      /*! Returns a pointer to the next or previous node given the opposite one.

      @param pnSibling
         Pointer to the previous or next node.
      */
      node * get_other_sibling(node const * pnSibling) {
         return reinterpret_cast<node *>(
            m_iPrevXorNext ^ reinterpret_cast<std::uintptr_t>(pnSibling)
         );
      }

      /*! Updates the previous/next pointer.

      @param pnPrev
         Pointer to the previous node.
      @param pnNext
         Pointer to the next node.
      */
      void set_siblings(node const * pnPrev, node const * pnNext) {
         m_iPrevXorNext = reinterpret_cast<std::uintptr_t>(pnPrev) ^
                          reinterpret_cast<std::uintptr_t>(pnNext);
      }

   private:
      //! Pointer to the previous node XOR pointer to the next node.
      std::uintptr_t m_iPrevXorNext;
   };

   //! Defines the minimal data members needed to implement a xor_list_impl subclass.
   struct data_members {
      //! Pointer to the first node.
      node * m_pnFirst;
      //! Pointer to the last node.
      node * m_pnLast;
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
      //! Default constructor.
      iterator_base() :
         m_pnCurr(nullptr),
         m_pnNext(nullptr) {
      }

      /*! Constructor.

      @param pnCurr
         Pointer to the current node.
      @param pnNext
         Pointer to the node following *pnCurr.
      */
      iterator_base(node * pnCurr, node * pnNext) :
         m_pnCurr(pnCurr),
         m_pnNext(pnNext) {
      }

      //! Moves the iterator to next node.
      void increment();

      /*! Throws an iterator_error exception if the iterator is at the end of the container or has
      been invalidated by a change in the container. */
      void validate() const;

   protected:
      //! Pointer to the current node.
      node * m_pnCurr;
      //! Pointer to the next node.
      node * m_pnNext;
   };

public:
   //! Iterator for XOR doubly-linked list node classes.
   template <typename TNode, typename TValue>
   class iterator : public iterator_base, public std::iterator<std::forward_iterator_tag, TValue> {
   public:
      typedef TValue * pointer;
      typedef TValue & reference;
      typedef TValue value_type;

   public:
      //! See iterator_base::iterator_base().
      iterator() {
      }
      iterator(node * pnCurr, node * pnNext) :
         iterator_base(pnCurr, pnNext) {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current node.
      */
      TValue & operator*() const {
         validate();
         return *static_cast<TValue *>(m_pnCurr);
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current node.
      */
      TValue * operator->() const {
         validate();
         return static_cast<TValue *>(m_pnCurr);
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
         Iterator pointing to the node preceding the one referenced by this iterator.
      */
      iterator operator++(int) {
         node * pnPrev = m_pnCurr;
         increment();
         return iterator(pnPrev, m_pnCurr);
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
   };

public:
   /*! Inserts a node to the end of the list.

   @param plxdm
      Pointer to the list data members.
   @param pn
      Pointer to the node to become the last in the list.
   */
   static void link_back(data_members * plxdm, node * pn);

   /*! Inserts a node to the start of the list.

   @param plxdm
      Pointer to the list data members.
   @param pn
      Pointer to the node to become the first in the list.
   */
   static void link_front(data_members * plxdm, node * pn);

   /*! Unlinks a node from the list.

   @param plxdm
      Pointer to the list data members.
   @param pn
      Pointer to the node to unlink.
   @param pnNext
      Pointer to the node following *pn.
   */
   static void unlink(data_members * plxdm, node * pn, node * pnNext);
};

}}} //namespace abc::collections::detail
