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

#ifndef _ABACLADE_COLLECTIONS_QUEUE_HXX
#define _ABACLADE_COLLECTIONS_QUEUE_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::detail::queue_impl

namespace abc {
namespace collections {
namespace detail {

//! Non-template implementation class for abc::collections::queue.
class ABACLADE_SYM queue_impl : public support_explicit_operator_bool<queue_impl> {
public:
   //! Node of a singly-linked list.
   class node {
   private:
      friend class queue_impl;

   public:
      /*! Returns a pointer to the contained T.

      @param type
         Adapter for the value’s type.
      @return
         Pointer to the contained value.
      */
      void * value_ptr(type_void_adapter const & type) const {
         return type.align_pointer(&m_pnNext + 1);
      }

   protected:
      //! Pointer to the next node.
      node * m_pnNext;
      // The contained value of type T follow immediately, taking alignment into consideration.
   };

public:
   /*! Constructor.

   @param q
      Source object.
   */
   queue_impl() :
      m_pnFirst(nullptr),
      m_pnLast(nullptr),
      m_cNodes(0) {
   }
   queue_impl(queue_impl && q) :
      m_pnFirst(q.m_pnFirst),
      m_pnLast(q.m_pnLast),
      m_cNodes(q.m_cNodes) {
      q.m_cNodes = 0;
      q.m_pnFirst = nullptr;
      q.m_pnLast = nullptr;
   }

   //! Destructor.
   ~queue_impl() {
   }

   /*! Assignment operator.

   @param q
      Source object.
   */
   queue_impl & operator=(queue_impl && q) {
      /* Assume that the subclass has already made a copy of m_pn{First,Last} to be able to release
      them after calling this operator. */
      m_pnFirst = q.m_pnFirst;
      q.m_pnFirst = nullptr;
      m_pnLast = q.m_pnLast;
      q.m_pnLast = nullptr;
      m_cNodes = q.m_cNodes;
      q.m_cNodes = 0;
      return *this;
   }

   /*! Returns true if the list size is greater than 0.

   @return
      true if the list is not empty, or false otherwise.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return m_cNodes > 0;
   }

   /*! Removes all elements from the queue.

   @param type
      Adapter for the node value’s type.
   */
   void clear(type_void_adapter const & type) {
      destruct_list(type, m_pnFirst);
      m_pnFirst = m_pnLast = nullptr;
      m_cNodes = 0;
   }

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
   /*! Discards all elements from a list, given its first node.

   @param type
      Adapter for the node value’s type.
   @param pnFirst
      Pointer to the first node to destruct.
   */
   static void destruct_list(type_void_adapter const & type, node * pnFirst) {
      for (detail::queue_impl::node * pnCurr = pnFirst, * pnNext; pnCurr; pnCurr = pnNext) {
         pnNext = pnCurr->m_pnNext;
         std::int8_t const * pValue = static_cast<std::int8_t const *>(pnCurr->value_ptr(type));
         type.destruct(pValue, pValue + 1 /*yes, 1 byte – it works, as long as it’s > pValue*/);
         memory::_raw_free(pnCurr);
      }
   }

   /*! Inserts a node to the end of the list.

   @param type
      Adapter for the node value’s type.
   @param pSrc
      Pointer to the value item to push in.
   @param bMove
      true to move *pSrc to the new node’s value, or false to copy it instead.
   */
   void push_back(type_void_adapter const & type, void * pSrc, bool bMove) {
      /* To calculate the node size, add type.cb bytes to the offset of the value in a node at
      address 0. This allows packing the node optimally even if the unpadded node size is e.g. 6
      (sizeof will return 8 for that) and type.cb is 2, giving 8 instead of 10 (which would really
      mean at least 12 bytes, a 50% waste of memory). */
      std::unique_ptr<node, memory::freeing_deleter> pn(static_cast<node *>(memory::_raw_alloc(
         reinterpret_cast<std::size_t>(static_cast<node *>(0)->value_ptr(type)) + type.cb
      )));
      std::int8_t * pDst = static_cast<std::int8_t *>(pn->value_ptr(type));
      // Yes, 1 byte – it works, as long as it’s > pSrc.
      void * pSrcEnd = static_cast<std::int8_t *>(pSrc) + 1;
      if (bMove) {
         type.move_constr(pDst, pSrc, pSrcEnd);
      } else {
         type.copy_constr(pDst, pSrc, pSrcEnd);
      }

      pn->m_pnNext = nullptr;
      if (!m_pnFirst) {
         m_pnFirst = pn.get();
      } else if (m_pnLast) {
         m_pnLast->m_pnNext = pn.get();
      }
      // Transfer ownership of the node to the list.
      m_pnLast = pn.release();
      ++m_cNodes;
   }

   /*! Unlinks and releases the first node in the list.

   @param type
      Adapter for the node value’s type.
   */
   void pop_front(type_void_adapter const & type) {
      node * pn = m_pnFirst;
      m_pnFirst = pn->m_pnNext;
      if (!m_pnFirst) {
         m_pnLast = nullptr;
      }
      --m_cNodes;
      std::int8_t const * pValue = static_cast<std::int8_t const *>(pn->value_ptr(type));
      type.destruct(pValue, pValue + 1 /*yes, 1 byte – it works, as long as it’s > pValue*/);
      memory::_raw_free(pn);
   }

protected:
   //! Pointer to the first node.
   node * m_pnFirst;
   //! Pointer to the last node.
   node * m_pnLast;
   //! Count of nodes.
   std::size_t m_cNodes;
};

} //namespace detail
} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::queue

namespace abc {
namespace collections {

/*! List-based queue. Offers constant insert-at-end time and constant extraction time of its first
element. */
template <typename T>
class queue : public detail::queue_impl {
public:
   /*! Constructor.

   @param q
      Source object.
   */
   queue() {
   }
   queue(queue && q) :
      detail::queue_impl(std::move(q)) {
   }

   //! Destructor.
   ~queue() {
      detail::type_void_adapter type;
      type.set_align<T>();
      type.set_destr_fn<T>();
      destruct_list(type, m_pnFirst);
   }

   /*! Assignment operator.

   @param q
      Source object.
   */
   queue & operator=(queue && q) {
      node * pnFirst = m_pnFirst;
      detail::queue_impl::operator=(std::move(q));
      // Now that *this has been successfully overwritten, destruct the old nodes.
      detail::type_void_adapter type;
      type.set_align<T>();
      type.set_destr_fn<T>();
      destruct_list(type, pnFirst);
      return *this;
   }

   /*! Returns a reference to the last element in the queue.

   @return
      Reference to the last element in the queue.
   */
   T & back() {
      detail::type_void_adapter type;
      type.set_align<T>();
      return *static_cast<T *>(m_pnLast->value_ptr(type));
   }
   T const & back() const {
      return const_cast<queue *>(this)->back();
   }

   //! Removes all elements from the queue.
   void clear() {
      detail::type_void_adapter type;
      type.set_align<T>();
      type.set_destr_fn<T>();
      detail::queue_impl::clear(type);
   }

   /*! Returns a reference to the first element in the queue.

   @return
      Reference to the first element in the queue.
   */
   T & front() {
      detail::type_void_adapter type;
      type.set_align<T>();
      return *static_cast<T *>(m_pnFirst->value_ptr(type));
   }
   T const & front() const {
      return const_cast<queue *>(this)->front();
   }

   /*! Removes the first element in the queue.

   @return
      Former first element in the queue.
   */
   T pop_front() {
      detail::type_void_adapter type;
      type.set_align<T>();
      type.set_destr_fn<T>();
      // Move the value of *m_pnFirst into t, then unlink and discard *m_pnFirst.
      T t(std::move(*static_cast<T *>(m_pnFirst->value_ptr(type))));
      detail::queue_impl::pop_front(type);
      return std::move(t);
   }

   /*! Adds an element to the end of the queue.

   @param t
      Element to add.
   */
   void push_back(T const & t) {
      detail::type_void_adapter type;
      type.set_align<T>();
      type.set_copy_fn<T>();
      type.set_size<T>();
      detail::queue_impl::push_back(type, &t, false);
   }
   void push_back(T && t) {
      detail::type_void_adapter type;
      type.set_align<T>();
      type.set_move_fn<T>();
      type.set_size<T>();
      detail::queue_impl::push_back(type, &t, true);
   }
};

} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COLLECTIONS_QUEUE_HXX
