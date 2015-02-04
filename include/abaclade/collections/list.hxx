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

#ifndef _ABACLADE_COLLECTIONS_LIST_HXX
#define _ABACLADE_COLLECTIONS_LIST_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::detail::list_impl

namespace abc {
namespace collections {
namespace detail {

//! Non-template implementation class for abc::collections::list.
class ABACLADE_SYM list_impl : protected xor_list::data_members {
public:
   /*! Constructor.

   @param l
      Source object.
   */
   list_impl();
   list_impl(list_impl && l);

   //! Destructor.
   ~list_impl() {
   }

   /*! Assignment operator.

   @param l
      Source object.
   */
   list_impl & operator=(list_impl && l);

   /*! Returns true if the list contains no elements.

   @return
      true if the list is empty, or false otherwise.
   */
   bool empty() const {
      return !m_cNodes;
   }

   /*! Returns the count of elements in the list.

   @return
      Count of elements.
   */
   std::size_t size() const {
      return m_cNodes;
   }

protected:
   /*! Returns a pointer to the last node in the list, throwing an exception if the list is empty.

   @return
      Pointer to the last node.
   */
   xor_list::node * back() const;

   /*! Returns a pointer to the first node in the list, throwing an exception if the list is empty.

   @return
      Pointer to the first node.
   */
   xor_list::node * front() const;

   /*! Inserts a node to the end of the list.

   @param pn
      Pointer to the node to become the last in the list.
   */
   void link_back(xor_list::node * pn);

   /*! Inserts a node to the start of the list.

   @param pn
      Pointer to the node to become the first in the list.
   */
   void link_front(xor_list::node * pn);

   /*! Unlinks and returns a node in the list.

   @param pn
      Pointer to the node to unlink.
   @param pnNext
      Pointer to the node following *pn.
   @return
      Now-unlinked node.
   */
   xor_list::node * unlink(xor_list::node * pn, xor_list::node * pnNext);

   /*! Unlinks and returns the first node in the list.

   @return
      Former first node.
   */
   xor_list::node * unlink_back();

   /*! Unlinks and returns the first node in the list.

   @return
      Former first node.
   */
   xor_list::node * unlink_front();

protected:
   //! Count of nodes.
   std::size_t m_cNodes;
};

} //namespace detail
} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::list

namespace abc {
namespace collections {

//! Doubly-linked list.
template <typename T>
class list : public detail::list_impl {
protected:
   //! List node.
   class node : public detail::xor_list::node {
   public:
      //! Constructor.
      node(T const & t) :
         m_t(t) {
      }
      node(T && t) :
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

public:
   typedef detail::xor_list::iterator<node, T> iterator;
   typedef detail::xor_list::iterator<node, T const> const_iterator;
   typedef iterator reverse_iterator;
   typedef const_iterator const_reverse_iterator;

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

   /*! Returns a reference to the first element in the list.

   @return
      Reference to the first element in the list.
   */
   T & back() {
//      ABC_TRACE_FUNC(this);

      return *static_cast<node *>(detail::list_impl::back())->value_ptr();
   }
   T const & back() const {
//      ABC_TRACE_FUNC(this);

      return *static_cast<node *>(detail::list_impl::back())->value_ptr();
   }

   /*! Returns a forward iterator to the start of the list.

   @return
      Iterator to the first node in the list.
   */
   iterator begin() {
      return iterator(this, m_pnFirst, m_pnFirst ? m_pnFirst->get_other_sibling(nullptr) : nullptr);
   }
   const_iterator begin() const {
      return const_iterator(
         this, m_pnFirst, m_pnFirst ? m_pnFirst->get_other_sibling(nullptr) : nullptr
      );
   }

   /*! Returns a const forward iterator to the start of the list.

   @return
      Iterator to the first node in the list.
   */
   const_iterator cbegin() const {
      return const_iterator(
         this, m_pnFirst, m_pnFirst ? m_pnFirst->get_other_sibling(nullptr) : nullptr
      );
   }

   /*! Returns a const forward iterator to the end of the list.

   @return
      Iterator to the beyond the last node in the list.
   */
   const_iterator cend() const {
      return const_iterator();
   }

   //! Removes all elements from the list.
   void clear() {
//      ABC_TRACE_FUNC(this);

      destruct_list(static_cast<node *>(m_pnFirst));
      m_pnFirst = m_pnLast = nullptr;
      m_cNodes = 0;
   }

   /*! Returns a const reverse iterator to the end of the list.

   @return
      Reverse iterator to the last node in the list.
   */
   const_reverse_iterator crbegin() const {
      return const_reverse_iterator(
         this, m_pnLast, m_pnLast ? m_pnLast->get_other_sibling(nullptr) : nullptr
      );
   }

   /*! Returns a const reverse iterator to the start of the list.

   @return
      Reverse iterator to the before the first node in the list.
   */
   const_reverse_iterator crend() const {
      return const_reverse_iterator();
   }

   /*! Returns a forward iterator to the end of the list.

   @return
      Iterator to the beyond the last node in the list.
   */
   iterator end() {
      return iterator();
   }
   const_iterator end() const {
      return const_iterator();
   }

   /*! Returns a reference to the last element in the list.

   @return
      Reference to the last element in the list.
   */
   T & front() {
//      ABC_TRACE_FUNC(this);

      return *static_cast<node *>(detail::list_impl::front())->value_ptr();
   }
   T const & front() const {
//      ABC_TRACE_FUNC(this);

      return *static_cast<node *>(detail::list_impl::front())->value_ptr();
   }

   /*! Removes and returns the last element in the list.

   @return
      Former last element in the list.
   */
   T pop_back() {
//      ABC_TRACE_FUNC(this);

      std::unique_ptr<node> pn(static_cast<node *>(unlink_back()));
      return std::move(pn->m_t);
   }

   /*! Removes the first element in the list.

   @return
      Former first element in the list.
   */
   T pop_front() {
//      ABC_TRACE_FUNC(this);

      std::unique_ptr<node> pn(static_cast<node *>(unlink_front()));
      return std::move(pn->m_t);
   }

   /*! Adds an element to the start of the list.

   @param t
      Element to add.
   */
   void push_front(T const & t) {
//      ABC_TRACE_FUNC(this/*, t*/);

      /* Memory management must happen here instead of link_back() because the unique_ptr must be of
      node, since detail::xor_list::node doesn’t have a virtual destructor. */
      std::unique_ptr<node> pn(new node(t));
      link_front(pn.get());
      // No exceptions, so the node is managed as part of the list.
      pn.release();
   }
   void push_front(T && t) {
//      ABC_TRACE_FUNC(this/*, t*/);

      /* Memory management must happen here instead of link_back() because the unique_ptr must be of
      node, since detail::xor_list::node doesn’t have a virtual destructor. */
      std::unique_ptr<node> pn(new node(std::move(t)));
      link_front(pn.get());
      // No exceptions, so the node is managed as part of the list.
      pn.release();
   }

   /*! Adds an element to the end of the list.

   @param t
      Element to add.
   */
   void push_back(T const & t) {
//      ABC_TRACE_FUNC(this/*, t*/);

      /* Memory management must happen here instead of link_back() because the unique_ptr must be of
      node, since detail::xor_list::node doesn’t have a virtual destructor. */
      std::unique_ptr<node> pn(new node(t));
      link_back(pn.get());
      // No exceptions, so the node is managed as part of the list.
      pn.release();
   }
   void push_back(T && t) {
//      ABC_TRACE_FUNC(this/*, t*/);

      /* Memory management must happen here instead of link_back() because the unique_ptr must be of
      node, since detail::xor_list::node doesn’t have a virtual destructor. */
      std::unique_ptr<node> pn(new node(std::move(t)));
      link_back(pn.get());
      // No exceptions, so the node is managed as part of the list.
      pn.release();
   }

   /*! Returns a reverse iterator to the end of the list.

   @return
      Reverse iterator to the last node in the list.
   */
   reverse_iterator rbegin() {
      return reverse_iterator(
         this, m_pnLast, m_pnLast ? m_pnLast->get_other_sibling(nullptr) : nullptr
      );
   }
   const_reverse_iterator rbegin() const {
      return const_reverse_iterator(
         this, m_pnLast, m_pnLast ? m_pnLast->get_other_sibling(nullptr) : nullptr
      );
   }

   /*! Removes the element at the specified position.

   @param it
      Iterator to the element to remove.
   */
   void remove_at(const_iterator const & it) {
//      ABC_TRACE_FUNC(this);

      delete static_cast<node *>(unlink(
         const_cast<node *>(it.base()),
         const_cast<node *>(it.next_base())
      ));
   }

   //! Removes the last element in the list.
   void remove_back() {
//      ABC_TRACE_FUNC(this);

      delete static_cast<node *>(unlink_back());
   }

   //! Removes the first element in the list.
   void remove_front() {
//      ABC_TRACE_FUNC(this);

      delete static_cast<node *>(unlink_front());
   }

   /*! Returns a reverse iterator to the start of the list.

   @return
      Reverse iterator to the before the first node in the list.
   */
   reverse_iterator rend() {
      return reverse_iterator();
   }
   const_reverse_iterator rend() const {
      return const_reverse_iterator();
   }

private:
   /*! Removes all elements from a list, given its first node.

   @param pnFirst
      Pointer to the first node to destruct.
   */
   static void destruct_list(node * pnFirst) {
//      ABC_TRACE_FUNC(this);

      for (detail::xor_list::node * pnPrev = nullptr, * pnCurr = pnFirst; pnCurr; ) {
         detail::xor_list::node * pnNext = pnCurr->get_other_sibling(pnPrev);
         delete static_cast<node *>(pnCurr);
         pnPrev = pnCurr;
         pnCurr = pnNext;
      }
   }
};

} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COLLECTIONS_LIST_HXX
