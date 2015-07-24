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

//! Node for XOR doubly-linked list classes.
class xor_list_node {
public:
   /*! Returns a pointer to the next or previous node given the opposite one.

   @param pnSibling
      Pointer to the previous or next node.
   */
   xor_list_node * get_other_sibling(xor_list_node const * pnSibling) {
      return reinterpret_cast<xor_list_node *>(
         m_iPrevXorNext ^ reinterpret_cast<std::uintptr_t>(pnSibling)
      );
   }

   /*! Updates the previous/next pointer.

   @param pnPrev
      Pointer to the previous node.
   @param pnNext
      Pointer to the next node.
   */
   void set_siblings(xor_list_node const * pnPrev, xor_list_node const * pnNext) {
      m_iPrevXorNext = reinterpret_cast<std::uintptr_t>(pnPrev) ^
                       reinterpret_cast<std::uintptr_t>(pnNext);
   }

private:
   //! Pointer to the previous node XOR pointer to the next node.
   std::uintptr_t m_iPrevXorNext;
};

/*! Defines the minimal data members needed to implement a xor_list_impl subclass. Can’t be a member
of xor_list_impl because xor_list_impl needs to derive from this. */
struct xor_list_data_members {
   //! Pointer to the first node.
   xor_list_node * m_pnFirst;
   //! Pointer to the last node.
   xor_list_node * m_pnLast;
};

/*! Initial value for the data members of an abc::collections::detail::xor_list_impl::data_members
instance. */
#define ABC_COLLECTIONS_DETAIL_XOR_LIST_IMPL_INITIALIZER \
   { nullptr, nullptr }

//! Defines classes useful to implement XOR-linked list classes.
class ABACLADE_SYM xor_list_impl :
   public xor_list_data_members,
   public support_explicit_operator_bool<xor_list_impl> {
protected:
   //! Data members needed to implement a xor_list_impl subclass.
   typedef xor_list_data_members data_members;

private:
   //! Node type.
   typedef xor_list_node node;

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
      iterator_base(xor_list_node * pnCurr, xor_list_node * pnNext) :
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
      xor_list_node * m_pnCurr;
      //! Pointer to the next node.
      xor_list_node * m_pnNext;
   };

public:
   //! Iterator for XOR doubly-linked list node classes.
   template <typename T>
   class iterator : public iterator_base, public std::iterator<std::forward_iterator_tag, T> {
   public:
      typedef T * pointer;
      typedef T & reference;
      typedef T value_type;

   public:
      //! See iterator_base::iterator_base().
      iterator() {
      }
      iterator(xor_list_node * pnCurr, xor_list_node * pnNext) :
         iterator_base(pnCurr, pnNext) {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current node.
      */
      T & operator*() const {
         validate();
         return *static_cast<T *>(m_pnCurr);
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current node.
      */
      T * operator->() const {
         validate();
         return static_cast<T *>(m_pnCurr);
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
         xor_list_node * pnPrev = m_pnCurr;
         increment();
         return iterator(pnPrev, m_pnCurr);
      }

      /*! Returns the underlying pointer to the node.

      @return
         Pointer to the current node.
      */
      T * base() {
         return static_cast<T *>(m_pnCurr);
      }
      T const * base() const {
         return static_cast<T *>(m_pnCurr);
      }

      /*! Returns a pointer to the next node.

      @return
         Pointer to the next node.
      */
      T * next_base() {
         return static_cast<T *>(m_pnNext);
      }
      T const * next_base() const {
         return static_cast<T *>(m_pnNext);
      }
   };

public:
   /*! Returns true if the list size is greater than 0.

   @return
      true if the list is not empty, or false otherwise.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return m_pnFirst != nullptr;
   }

   /*! Returns true if the list contains no elements.

   @return
      true if the list is empty, or false otherwise.
   */
   bool empty() {
      return !m_pnFirst;
   }

protected:
   /*! Inserts a node to the end of the list.

   @param pn
      Pointer to the node to become the last in the list.
   */
   void link_back(xor_list_node * pn);

   /*! Inserts a node to the start of the list.

   @param pn
      Pointer to the node to become the first in the list.
   */
   void link_front(xor_list_node * pn);

   /*! Removes a node from the list.

   @param pn
      Pointer to the node to remove.
   */
   void unlink(xor_list_node * pn);

   /*! Removes a node from the list.

   @param pn
      Pointer to the node to unlink.
   @param pnPrev
      Pointer to the node preceding *pn.
   @param pnNext
      Pointer to the node following *pn.
   */
   void unlink(xor_list_node * pn, xor_list_node * pnPrev, xor_list_node * pnNext);
};

}}} //namespace abc::collections::detail
