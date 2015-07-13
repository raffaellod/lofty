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

#ifndef _ABACLADE_COLLECTIONS_DETAIL_SINGLY_LINKED_LIST_IMPL_HXX
#define _ABACLADE_COLLECTIONS_DETAIL_SINGLY_LINKED_LIST_IMPL_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/type_void_adapter.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

//! Non-template implementation of a singly-linked list.
class ABACLADE_SYM singly_linked_list_impl :
   public support_explicit_operator_bool<singly_linked_list_impl> {
protected:
   //! Singly-linked list node that also stores a single value.
   class ABACLADE_SYM node {
   public:
      /*! Allocates space for a node and its contained value.

      @param cb
         sizeof(node).
      @param type
         Adapter for the value’s type.
      @return
         Pointer to the allocated memory block.
      */
      void * operator new(std::size_t cb, type_void_adapter const & type);

      /*! Deallocates the memory occupied by a node.

      @param p
         Pointer to free.
      */
      void operator delete(void * p) {
         memory::_raw_free(p);
      }

      /*! Matches the custom operator new().

      @param p
         Pointer to free.
      */
      void operator delete(void * p, type_void_adapter const &) {
         operator delete(p);
      }

      /*! Constructor.

      @param type
         Adapter for the value’s type.
      @param ppnFirst
         Pointer to the list’s first node pointer.
      @param ppnLast
         Pointer to the list’s last node pointer.
      @param pnPrev
         Pointer to the previous node.
      @param pnNext
         Pointer to the next node.
      @param p
         Pointer to the value to add.
      @param bMove
         true to move *p to the new node’s value, or false to copy it instead.
      */
      node(
         type_void_adapter const & type, node ** ppnFirst, node ** ppnLast,
         node * pnPrev, node * pnNext, void const * p, bool bMove
      );

      /*! Returns a pointer to the next node.

      @return
         Pointer to the next node.
      */
      node * next() const {
         return m_pnNext;
      }

      /*! Removes the node from the list it’s in.

      @param ppnFirst
         Pointer to the list’s first node pointer. May be nullptr.
      @param ppnLast
         Pointer to the list’s last node pointer. May be nullptr.
      @param pnPrev
         Pointer to the previous node.
      */
      void unlink(node ** ppnFirst, node ** ppnLast, node * pnPrev);

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

   @param slli
      Source object.
   */
   singly_linked_list_impl() :
      m_pnFirst(nullptr),
      m_pnLast(nullptr),
      m_cNodes(0) {
   }
   singly_linked_list_impl(singly_linked_list_impl && slli);

   //! Destructor.
   ~singly_linked_list_impl() {
   }

   /*! Assignment operator.

   @param slli
      Source object.
   */
   singly_linked_list_impl & operator=(singly_linked_list_impl && slli);

   /*! Returns true if the list size is greater than 0.

   @return
      true if the list is not empty, or false otherwise.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return m_cNodes > 0;
   }

   /*! Removes all elements from the list.

   @param type
      Adapter for the node value’s type.
   */
   void clear(type_void_adapter const & type);

   /*! Returns true if the list contains no elements.

   @return
      true if the list is empty, or false otherwise.
   */
   bool empty() const {
      return m_cNodes == 0;
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
   static void destruct_list(type_void_adapter const & type, node * pn);

   /*! Inserts a node to the end of the list.

   @param type
      Adapter for the node value’s type.
   @param p
      Pointer to the value item to push in.
   @param bMove
      true to move *p to the new node’s value, or false to copy it instead.
   @return
      Pointer to the newly-added node.
   */
   node * push_back(type_void_adapter const & type, void const * p, bool bMove);

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

#endif //ifndef _ABACLADE_COLLECTIONS_DETAIL_SINGLY_LINKED_LIST_IMPL_HXX
