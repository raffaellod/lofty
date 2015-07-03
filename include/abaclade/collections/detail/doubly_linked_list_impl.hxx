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

#ifndef _ABACLADE_COLLECTIONS_DETAIL_DOUBLY_LINKED_LIST_IMPL_HXX
#define _ABACLADE_COLLECTIONS_DETAIL_DOUBLY_LINKED_LIST_IMPL_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/collections/type_void_adapter.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

//! Defines classes useful to implement doubly-linked list classes.
class ABACLADE_SYM doubly_linked_list_impl {
public:
   //! Doubly-linked node that also stores a single value.
   class node {
   private:
      friend class doubly_linked_list_impl;

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

      /*! Ensures that memory allocated by node::operator new() is freed correctly.

      @param p
         Pointer to free.
      */
      void operator delete(void * p) {
         memory::_raw_free(p);
      }

      /*! Returns a pointer to the next node.

      @return
         Pointer to the next node.
      */
      node * next() const {
         return m_pnNext;
      }

      /*! Returns a pointer to the previous node.

      @return
         Pointer to the previous node.
      */
      node * prev() const {
         return m_pnPrev;
      }

      /*! Returns a pointer to the contained value.

      @param type
         Adapter for the value’s type.
      @return
         Pointer to the contained value.
      */
      void * value_ptr(type_void_adapter const & type) const;

      /*! Returns a typed pointer to the contained TValue.

      @return
         Pointer to the contained value.
      */
      template <typename T>
      T * value_ptr() const {
         type_void_adapter type;
         type.set_align<T>();
         return static_cast<T *>(value_ptr(type));
      }

   private:
      //! Pointer to the next node.
      node * m_pnNext;
      //! Pointer to the previous node.
      node * m_pnPrev;
      // The contained value follows immediately, taking alignment into consideration.
   };

public:
   /*! Recursively destructs a list and all its value nodes.

   @param type
      Adapter for the value’s type.
   @param pn
      Pointer to the first list node.
   */
   static void destruct_list(type_void_adapter const & type, node * pn);

   /*! Inserts a node at the end of the list.

   @param ppnFirst
      Pointer to the list’s first node pointer.
   @param ppnLast
      Pointer to the list’s last node pointer.
   @param pn
      Pointer to the node to become the last in the list.
   */
   static void link_back(node ** ppnFirst, node ** ppnLast, node * pn);

   /*! Inserts a node at the start of the list.

   @param ppnFirst
      Pointer to the list’s first node pointer.
   @param ppnLast
      Pointer to the list’s last node pointer.
   @param pn
      Pointer to the node to become the first in the list.
   */
   static void link_front(node ** ppnFirst, node ** ppnLast, node * pn);

   /*! Unlinks and destructs a node from the list.

   @param type
      Adapter for the value’s type.
   @param ppnFirst
      Pointer to the list’s first node pointer.
   @param ppnLast
      Pointer to the list’s last node pointer.
   @param pn
      Pointer to the node to unlink.
   */
   static void remove(type_void_adapter const & type, node ** ppnFirst, node ** ppnLast, node * pn);
};

}}} //namespace abc::collections::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COLLECTIONS_DETAIL_DOUBLY_LINKED_LIST_IMPL_HXX
