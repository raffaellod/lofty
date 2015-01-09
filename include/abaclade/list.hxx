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

#ifndef _ABACLADE_LIST_HXX
#define _ABACLADE_LIST_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::list_impl

namespace abc {
namespace detail {

//! Non-template implementation class for abc::list.
class ABACLADE_SYM list_impl {
public:
   /*! Constructor.

   @param l
      Source object.
   */
   list_impl() :
      m_pnFirst(nullptr),
      m_pnLast(nullptr),
      m_cNodes(0) {
   }
   list_impl(list_impl && l) :
      m_pnFirst(l.m_pnFirst),
      m_pnLast(l.m_pnLast),
      m_cNodes(l.m_cNodes) {
      l.m_pnFirst = nullptr;
      l.m_pnLast = nullptr;
      l.m_cNodes = 0;
   }

   //! Destructor.
   ~list_impl() {
   }

   /*! Assignment operator.

   @param l
      Source object.
   */
   list_impl & operator=(list_impl && l);

   /*! Returns the count of elements in the list.

   @return
      Count of elements.
   */
   std::size_t size() const {
      return m_cNodes;
   }

protected:
   /*! Inserts a node to the end of the list.

   @param pn
      Pointer to the node to become the first in the list.
   */
   void link_back(xor_list_node_impl * pn);

   /*! Inserts a node to the start of the list.

   @param pn
      Pointer to the node to become the last in the list.
   */
   void link_front(xor_list_node_impl * pn);

   /*! Unlinks and returns the first node in the list.

   @return
      Former first node.
   */
   xor_list_node_impl * unlink_back();

   /*! Unlinks and returns the first node in the list.

   @return
      Former first node.
   */
   xor_list_node_impl * unlink_front();

protected:
   //! Pointer to the first node.
   xor_list_node_impl * m_pnFirst;
   //! Pointer to the last node.
   xor_list_node_impl * m_pnLast;
   //! Count of nodes.
   std::size_t m_cNodes;
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::list

namespace abc {

//! Doubly-linked list.
template <typename T>
class list : public detail::list_impl {
private:
   typedef detail::xor_list_node_impl node_impl;

protected:
   //! List node.
   class node : public node_impl {
   public:
      //! Constructor.
      node(T t) :
         m_t(std::move(t)) {
      }

      /*! Returns a pointer to the contained T.

      @return
         Pointer to the contained value.
      */
      T * value_ptr() {
         return &m_t;
      }
      T const * value_ptr() const {
         return &m_t;
      }

   public:
      //! Element contained within the node.
      T m_t;
   };

   //! Nodes iterator.
   template <typename TValue>
   class iterator_impl :
      public detail::xor_list_iterator_impl<iterator_impl<TValue>, node, TValue> {
   public:
      //! See detail::xor_list_iterator_impl::xor_list_iterator_impl().
      iterator_impl(node * pnPrev, node * pnCurr, node * pnNext) :
         detail::xor_list_iterator_impl<iterator_impl<TValue>, node, TValue>(
            pnPrev, pnCurr, pnNext
         ) {
      }
   };

public:
   typedef iterator_impl<T> iterator;
   typedef iterator_impl<T const> const_iterator;
   typedef std::reverse_iterator<iterator> reverse_iterator;
   typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

public:
   /*! Constructor.

   @param l
      Source object.
   */
   list() {
   }
   list(list && l) :
      detail::list_impl(std::move(l)) {
   }

   //! Destructor.
   ~list() {
      destruct_list(static_cast<node *>(m_pnFirst));
   }

   /*! Assignment operator.

   @param l
      Source object.
   */
   list & operator=(list && l) {
      node * pnFirst = m_pnFirst;
      detail::list_impl::operator=(std::move(l));
      // Now that the *this has been successfully overwritten, destruct the old nodes.
      destruct_list(pnFirst);
      return *this;
   }

   /*! Returns a forward iterator to the start of the list.

   @return
      Iterator to the first node in the list.
   */
   iterator begin() {
      return iterator(nullptr, m_pnFirst, m_pnFirst ? m_pnFirst->get_next(nullptr) : nullptr);
   }

   //! Removes all elements from the list.
   void clear() {
      ABC_TRACE_FUNC(this);

      destruct_list(static_cast<node *>(m_pnFirst));
      m_cNodes = 0;
   }

   /*! Returns a forward iterator to the end of the list.

   @return
      Iterator to the beyond the last node in the list.
   */
   iterator end() {
      return iterator(m_pnLast, nullptr, nullptr);
   }

   /*! Removes and returns the last element in the list.

   @return
      Former last element in the list.
   */
   T pop_back() {
      ABC_TRACE_FUNC(this);

      std::unique_ptr<node> pn(static_cast<node *>(unlink_back()));
      return std::move(pn->m_t);
   }

   /*! Removes the first element in the list.

   @return
      Former first element in the list.
   */
   T pop_front() {
      ABC_TRACE_FUNC(this);

      std::unique_ptr<node> pn(static_cast<node *>(unlink_front()));
      return std::move(pn->m_t);
   }

   /*! Adds an element to the start of the list.

   @param t
      Element to add.
   */
   void push_front(T t) {
      ABC_TRACE_FUNC(this/*, t*/);

      /* Memory management must happen here instead of link_back() because the unique_ptr must be of
      node, since node_impl doesn’t have a virtual destructor. */
      std::unique_ptr<node> pn(new node(std::move(t)));
      link_front(pn.get());
      // No exceptions, so the node is managed as part of the list.
      pn.release();
   }

   /*! Adds an element to the end of the list.

   @param t
      Element to add.
   */
   void push_back(T t) {
      ABC_TRACE_FUNC(this/*, t*/);

      /* Memory management must happen here instead of link_back() because the unique_ptr must be of
      node, since node_impl doesn’t have a virtual destructor. */
      std::unique_ptr<node> pn(new node(std::move(t)));
      link_back(pn.get());
      // No exceptions, so the node is managed as part of the list.
      pn.release();
   }

   /*! Returns a reverse iterator to the end of the list.

   @return
      Reverse Iterator to the last node in the list.
   */
   reverse_iterator rbegin() {
      return reverse_iterator(end());
   }

   //! Removes the last element in the list.
   void remove_back() {
      ABC_TRACE_FUNC(this);

      delete static_cast<node *>(unlink_back());
   }

   //! Removes the first element in the list.
   void remove_front() {
      ABC_TRACE_FUNC(this);

      delete static_cast<node *>(unlink_front());
   }

   /*! Returns a reverse iterator to the start of the list.

   @return
      Reverse iterator to the before the first node in the list.
   */
   reverse_iterator rend() {
      return reverse_iterator(begin());
   }

private:
   /*! Removes all elements from a list, given its first node.

   @param pnFirst
      Pointer to the first node to destruct.
   */
   void destruct_list(node * pnFirst) {
      ABC_TRACE_FUNC(this);

      for (node_impl * pnPrev = nullptr, * pnCurr = pnFirst; pnCurr; ) {
         node_impl * pnNext = pnCurr->get_next(pnPrev);
         delete static_cast<node *>(pnCurr);
         pnPrev = pnCurr;
         pnCurr = pnNext;
      }
      m_cNodes = 0;
   }
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_LIST_HXX
