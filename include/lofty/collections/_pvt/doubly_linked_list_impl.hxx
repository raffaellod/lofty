/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_COLLECTIONS__PVT_DOUBLY_LINKED_LIST_IMPL_HXX
#define _LOFTY_COLLECTIONS__PVT_DOUBLY_LINKED_LIST_IMPL_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/type_void_adapter.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

//! Non-template implementation of a doubly-linked list.
class LOFTY_SYM doubly_linked_list_impl :
   public support_explicit_operator_bool<doubly_linked_list_impl> {
public:
   //! Doubly-linked list node that also stores a single value.
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
         true to move *p to the new node’s value, or false to copy it instead.
      */
      node(
         type_void_adapter const & type, node ** first_node, node ** last_node,
         node * prev, node * next, void const * value_src, bool move
      );

      /*! Returns a pointer to the next node.

      @return
         Pointer to the next node.
      */
      node * next() const {
         return next_;
      }

      /*! Returns a pointer to the previous node.

      @return
         Pointer to the previous node.
      */
      node * prev() const {
         return prev_;
      }

      /*! Removes the node from the list it’s in.

      @param first_node
         Pointer to the list’s first node pointer. May be nullptr.
      @param last_node
         Pointer to the list’s last node pointer. May be nullptr.
      */
      void unlink(node ** first_node, node ** last_node);

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
      node * next_;
      //! Pointer to the previous node.
      node * prev_;
      // The contained value follows immediately, taking alignment into consideration.
   };

protected:
   //! Base class for list iterator implementations.
   class LOFTY_SYM iterator_base {
   public:
      typedef std::ptrdiff_t difference_type;
      typedef _std::bidirectional_iterator_tag iterator_category;

   public:
      /*! Equality relational operator.

      @param other
         Object to compare to *this.
      @return
         true if *this refers to the same element as other, or false otherwise.
      */
      bool operator==(iterator_base const & other) const {
         return nd == other.nd;
      }

      /*! Inequality relational operator.

      @param other
         Object to compare to *this.
      @return
         true if *this refers to a different element than other, or false otherwise.
      */
      bool operator!=(iterator_base const & other) const {
         return !operator==(other);
      }

   protected:
      //! Default constructor.
      iterator_base() :
         nd(nullptr) {
      }

      /*! Constructor.

      @param nd_
         Pointer to the referred node.
      */
      iterator_base(node * nd_) :
         nd(nd_) {
      }

      /*! Moves the iterator to the previous or next node.

      @param forward
         If true, the iterator will move to the next node; if false, the iterator will move to the previous
         node.
      */
      void advance(bool forward);

      //! Throws a collections::out_of_range exception if the iterator cannot be dereferenced.
      void validate() const;

   protected:
      //! Pointer to the current node.
      node * nd;
   };

public:
   //! Default constructor.
   doubly_linked_list_impl();

   /*! Move constructor.

   @param src
      Source object.
   */
   doubly_linked_list_impl(doubly_linked_list_impl && src);

   //! Destructor.
   ~doubly_linked_list_impl() {
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   doubly_linked_list_impl & operator=(doubly_linked_list_impl && src);

   /*! Boolean evaluation operator.

   @return
      true if the list is not empty, or false otherwise.
   */
   LOFTY_EXPLICIT_OPERATOR_BOOL() const {
      return size_ > 0;
   }

   /*! Recursively destructs a list and all its value nodes.

   @param type
      Adapter for the value’s type.
   @param nd
      Pointer to the first list node.
   */
   static void destruct_list(type_void_adapter const & type, node * nd);

   /*! Inserts a node at the end of the list.

   @param type
      Adapter for the value’s type.
   @param first_node
      Pointer to the list’s first node pointer.
   @param last_node
      Pointer to the list’s last node pointer.
   @param value
      Pointer to the value to add.
   @param move
      true to move *value to the new node’s value, or false to copy it instead.
   @return
      Pointer to the newly-added node.
   */
   static node * push_back(
      type_void_adapter const & type, node ** first_node, node ** last_node, void const * value, bool move
   );

   /*! Inserts a node at the start of the list.

   @param type
      Adapter for the value’s type.
   @param first_node
      Pointer to the list’s first node pointer.
   @param last_node
      Pointer to the list’s last node pointer.
   @param value
      Pointer to the value to add.
   @param move
      true to move *value to the new node’s value, or false to copy it instead.
   @return
      Pointer to the newly-added node.
   */
   static node * push_front(
      type_void_adapter const & type, node ** first_node, node ** last_node, void const * value, bool move
   );

   /*! Unlinks and destructs a node from the list.

   @param type
      Adapter for the value’s type.
   @param first_node
      Pointer to the list’s first node pointer. May be nullptr.
   @param last_node
      Pointer to the list’s last node pointer. May be nullptr.
   @param nd
      Pointer to the node to unlink.
   */
   static void remove(type_void_adapter const & type, node ** first_node, node ** last_node, node * nd);

   /*! Returns the count of elements in the list.

   @return
      Count of elements.
   */
   std::size_t size() const {
      return size_;
   }

protected:
   /*! Returns a pointer to the last node in the list, throwing an exception if the list is empty.

   @return
      Pointer to the last node.
   */
   node * back() const;

   /*! Removes all elements from the list.

   @param type
      Adapter for the value’s type.
   */
   void clear(type_void_adapter const & type);

   /*! Returns a pointer to the first node in the list, throwing an exception if the list is empty.

   @return
      Pointer to the first node.
   */
   node * front() const;

   /*! Inserts a node to the end of the list.

   @param type
      Adapter for the value’s type.
   @param value
      Pointer to the value to add.
   @param move
      true to move *value to the new node’s value, or false to copy it instead.
   @return
      Pointer to the newly-added node.
   */
   node * push_back(type_void_adapter const & type, void const * value, bool move);

   /*! Inserts a node to the start of the list.

   @param type
      Adapter for the value’s type.
   @param value
      Pointer to the value to add.
   @param move
      true to move *value to the new node’s value, or false to copy it instead.
   @return
      Pointer to the newly-added node.
   */
   node * push_front(type_void_adapter const & type, void const * value, bool move);

   /*! Unlinks and destructs a node in the list.

   @param type
      Adapter for the value’s type.
   @param nd
      Pointer to the node to unlink.
   */
   void remove(type_void_adapter const & type, node * nd);

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

#endif //ifndef _LOFTY_COLLECTIONS__PVT_DOUBLY_LINKED_LIST_IMPL_HXX
