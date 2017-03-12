/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_COLLECTIONS__PVT_SINGLY_LINKED_LIST_IMPL_HXX
#define _LOFTY_COLLECTIONS__PVT_SINGLY_LINKED_LIST_IMPL_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declaration.
namespace lofty {

class type_void_adapter;

} //namespace lofty

namespace lofty { namespace collections { namespace _pvt {

//! Non-template implementation of a singly-linked list.
class LOFTY_SYM singly_linked_list_impl : public support_explicit_operator_bool<singly_linked_list_impl> {
protected:
   //! Singly-linked list node that also stores a single value.
   class LOFTY_SYM node {
   public:
      /*! Allocates space for a node and its contained value.

      @param alloc_size
         sizeof(node).
      @param type
         Adapter for the value’s type.
      @return
         Pointer to the allocated memory block.
      */
      void * operator new(std::size_t alloc_size, type_void_adapter const & type);

      /*! Deallocates the memory occupied by a node.

      @param p
         Pointer to free.
      */
      void operator delete(void * p) {
         memory::free(p);
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
      @param first_node
         Pointer to the list’s first node pointer.
      @param last_node
         Pointer to the list’s last node pointer.
      @param prev
         Pointer to the previous node.
      @param next
         Pointer to the next node.
      @param value_src
         Pointer to the value to assign.
      @param move
         true to move *value_src to the new node’s value, or false to copy it instead.
      */
      node(
         type_void_adapter const & type, node ** first_node, node ** last_node, node * prev, node * next,
         void const * value_src, bool move
      );

      /*! Returns a pointer to the next node.

      @return
         Pointer to the next node.
      */
      node * next() const {
         return next_;
      }

      /*! Removes the node from the list it’s in.

      @param first_node
         Pointer to the list’s first node pointer. May be nullptr.
      @param last_node
         Pointer to the list’s last node pointer. May be nullptr.
      @param prev
         Pointer to the previous node.
      */
      void unlink(node ** first_node, node ** last_node, node * prev);

      /*! Returns a pointer to the contained T.

      @param type
         Adapter for the value’s type.
      @return
         Pointer to the contained value.
      */
      void * value_ptr(type_void_adapter const & type) const;

   protected:
      //! Pointer to the next node.
      node * next_;
      // The contained value of type T follow immediately, taking alignment into consideration.
   };

public:
   //! Default constructor.
   singly_linked_list_impl() :
      first_node(nullptr),
      last_node(nullptr),
      size_(0) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   singly_linked_list_impl(singly_linked_list_impl && src);

   //! Destructor.
   ~singly_linked_list_impl() {
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   singly_linked_list_impl & operator=(singly_linked_list_impl && src);

   /*! Boolean evaluation operator.

   @return
      true if the list is not empty, or false otherwise.
   */
   LOFTY_EXPLICIT_OPERATOR_BOOL() const {
      return size_ > 0;
   }

   /*! Removes all elements from the list.

   @param type
      Adapter for the node value’s type.
   */
   void clear(type_void_adapter const & type);

   /*! Returns the count of elements in the list.

   @return
      Count of elements.
   */
   std::size_t size() const {
      return size_;
   }

protected:
   /*! Discards all elements from a list, given its first node.

   @param type
      Adapter for the node value’s type.
   @param nd
      Pointer to the first node to destruct.
   */
   static void destruct_list(type_void_adapter const & type, node * nd);

   /*! Inserts a node to the end of the list.

   @param type
      Adapter for the node value’s type.
   @param value
      Pointer to the value item to push in.
   @param move
      true to move *value to the new node’s value, or false to copy it instead.
   @return
      Pointer to the newly-added node.
   */
   node * push_back(type_void_adapter const & type, void const * value, bool move);

   /*! Unlinks and releases the first node in the list.

   @param type
      Adapter for the node value’s type.
   */
   void pop_front(type_void_adapter const & type);

protected:
   //! Pointer to the first node.
   node * first_node;
   //! Pointer to the last node.
   node * last_node;
   //! Count of nodes.
   std::size_t size_;
};

}}} //namespace lofty::collections::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_COLLECTIONS__PVT_SINGLY_LINKED_LIST_IMPL_HXX
