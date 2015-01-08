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
protected:
   /*! Internal node representation. This is lacking the actual data, which is provided by
   list::node. */
   class node_impl {
   public:
      /*! Returns a pointer to the next node.

      @param pnPrev
         Pointer to the previous node.
      */
      node_impl * get_next(node_impl * pnPrev) {
         return reinterpret_cast<node_impl *>(
            m_iPrevXorNext ^ reinterpret_cast<std::uintptr_t>(pnPrev)
         );
      }

      /*! Returns a pointer to the previous node.

      @param pnNext
         Pointer to the next node.
      */
      node_impl * get_prev(node_impl * pnNext) {
         return reinterpret_cast<node_impl *>(
            m_iPrevXorNext ^ reinterpret_cast<std::uintptr_t>(pnNext)
         );
      }

      /*! Updates the previous/next pointer.

      @param pnPrev
         Pointer to the previous node.
      @param pnNext
         Pointer to the next node.
      */
      void set_prev_next(node_impl * pnPrev, node_impl * pnNext) {
         m_iPrevXorNext = reinterpret_cast<std::uintptr_t>(pnPrev) ^
                          reinterpret_cast<std::uintptr_t>(pnNext);
      }

   private:
      //! Pointer to the previous node XOR pointer to the next node.
      std::uintptr_t m_iPrevXorNext;
   };


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
   void link_back(node_impl * pn);

   /*! Inserts a node to the start of the list.

   @param pn
      Pointer to the node to become the last in the list.
   */
   void link_front(node_impl * pn);

   /*! Unlinks and returns the first node in the list.

   @return
      Former first node.
   */
   node_impl * unlink_back();

   /*! Unlinks and returns the first node in the list.

   @return
      Former first node.
   */
   node_impl * unlink_front();

protected:
   //! Pointer to the first node.
   node_impl * m_pnFirst;
   //! Pointer to the last node.
   node_impl * m_pnLast;
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
protected:
   //! Base class for nodes of list.
   class node : public detail::list_impl::node_impl {
   public:
      //! Constructor.
      node(T t) :
         m_t(std::move(t)) {
      }

      //! See node_impl::get_next().
      node * get_next(node * pnPrev) {
         return static_cast<node *>(node_impl::get_next(pnPrev));
      }

      //! See node_impl::get_prev().
      node * get_prev(node * pnNext) {
         return static_cast<node *>(node_impl::get_prev(pnNext));
      }

      //! See node_impl::set_prev_next().
      void set_prev_next(node * pnPrev, node * pnNext) {
         node_impl::set_prev_next(pnPrev, pnNext);
      }

   public:
      //! Element contained within the node.
      T m_t;
   };

   //! Iterator for list::node subclasses.
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

      /*! Dereferencing operator.

      @return
         Reference to the current value.
      */
      TValue & operator*() const {
         return this->m_pnCurr->m_t;
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current value.
      */
      TValue * operator->() const {
         return &this->m_pnCurr->m_t;
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

      for (node * pnPrev = nullptr, * pnCurr = pnFirst; pnCurr; ) {
         node * pnNext = pnCurr->get_next(pnPrev);
         delete pnCurr;
         pnPrev = pnCurr;
         pnCurr = pnNext;
      }
      m_cNodes = 0;
   }
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_LIST_HXX
