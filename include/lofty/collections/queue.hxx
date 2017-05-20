/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_COLLECTIONS_QUEUE_HXX
#define _LOFTY_COLLECTIONS_QUEUE_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/collections/_pvt/singly_linked_list_impl.hxx>
#include <lofty/type_void_adapter.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections {

//! List-based queue. Offers constant insert-at-end time and constant extraction time of its first element.
template <typename T>
class queue : public _pvt::singly_linked_list_impl {
public:
   //! Default constructor.
   queue() {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   queue(queue && src) :
      _pvt::singly_linked_list_impl(_std::move(src)) {
   }

   //! Destructor.
   ~queue() {
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      destruct_list(type, first_node);
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   queue & operator=(queue && src) {
      node * old_first_node = first_node;
      _pvt::singly_linked_list_impl::operator=(_std::move(src));
      // Now that *this has been successfully overwritten, destruct the old nodes.
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      destruct_list(type, old_first_node);
      return *this;
   }

   /*! Returns a reference to the last element in the queue.

   @return
      Reference to the last element in the queue.
   */
   T & back() {
      type_void_adapter type;
      type.set_align<T>();
      return *static_cast<T *>(last_node->value_ptr(type));
   }

   /*! Returns a const reference to the last element in the queue.

   @return
      Const reference to the last element in the queue.
   */
   T const & back() const {
      return const_cast<queue *>(this)->back();
   }

   //! Removes all elements from the queue.
   void clear() {
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      _pvt::singly_linked_list_impl::clear(type);
   }

   /*! Returns a reference to the first element in the queue.

   @return
      Reference to the first element in the queue.
   */
   T & front() {
      type_void_adapter type;
      type.set_align<T>();
      return *static_cast<T *>(first_node->value_ptr(type));
   }

   /*! Returns a const reference to the first element in the queue.

   @return
      Const reference to the first element in the queue.
   */
   T const & front() const {
      return const_cast<queue *>(this)->front();
   }

   /*! Removes the first element in the queue.

   @return
      Former first element in the queue.
   */
   T pop_front() {
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      // Move the value of *first_node into t, then unlink and discard *first_node.
      T ret(_std::move(*static_cast<T *>(first_node->value_ptr(type))));
      _pvt::singly_linked_list_impl::pop_front(type);
      return _std::move(ret);
   }

   /*! Copies an element to the end of the queue.

   @param t
      Element to add.
   */
   void push_back(T const & t) {
      type_void_adapter type;
      type.set_align<T>();
      type.set_copy_construct<T>();
      type.set_size<T>();
      _pvt::singly_linked_list_impl::push_back(type, &t, false);
   }

   /*! Moves an element to the end of the queue.

   @param t
      Element to add.
   */
   void push_back(T && t) {
      type_void_adapter type;
      type.set_align<T>();
      type.set_move_construct<T>();
      type.set_size<T>();
      _pvt::singly_linked_list_impl::push_back(type, &t, true);
   }
};

}} //namespace lofty::collections

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_COLLECTIONS_QUEUE_HXX
