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

#ifndef _ABACLADE_COLLECTIONS_DETAIL_QUEUE_IMPL_HXX
#define _ABACLADE_COLLECTIONS_DETAIL_QUEUE_IMPL_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/type_void_adapter.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

//! Non-template implementation class for abc::collections::queue.
class ABACLADE_SYM queue_impl : public support_explicit_operator_bool<queue_impl> {
protected:
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
      void * value_ptr(type_void_adapter const & type) const;

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
   queue_impl(queue_impl && q);

   //! Destructor.
   ~queue_impl() {
   }

   /*! Assignment operator.

   @param q
      Source object.
   */
   queue_impl & operator=(queue_impl && q);

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
   void clear(type_void_adapter const & type);

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
   static void destruct_list(type_void_adapter const & type, node * pnFirst);

   /*! Inserts a node to the end of the list.

   @param type
      Adapter for the node value’s type.
   @param pSrc
      Pointer to the value item to push in.
   @param bMove
      true to move *pSrc to the new node’s value, or false to copy it instead.
   */
   void push_back(type_void_adapter const & type, void const * pSrc, bool bMove);

   /*! Unlinks and releases the first node in the list.

   @param type
      Adapter for the node value’s type.
   */
   void pop_front(type_void_adapter const & type);

protected:
   //! Pointer to the first node.
   node * m_pnFirst;
   //! Pointer to the last node.
   node * m_pnLast;
   //! Count of nodes.
   std::size_t m_cNodes;
};

}}} //namespace abc::collections::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COLLECTIONS_DETAIL_QUEUE_IMPL_HXX
