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

#include <abaclade/collections/detail/singly_linked_list_impl.hxx>
#include <abaclade/type_void_adapter.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections {

/*! List-based queue. Offers constant insert-at-end time and constant extraction time of its first
element. */
template <typename T>
class queue : public detail::singly_linked_list_impl {
public:
   //! Default constructor.
   queue() {
   }

   /*! Move constructor.

   @param q
      Source object.
   */
   queue(queue && q) :
      detail::singly_linked_list_impl(_std::move(q)) {
   }

   //! Destructor.
   ~queue() {
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      destruct_list(type, m_pnFirst);
   }

   /*! Move-assignment operator.

   @param q
      Source object.
   @return
      *this.
   */
   queue & operator=(queue && q) {
      node * pnFirst = m_pnFirst;
      detail::singly_linked_list_impl::operator=(_std::move(q));
      // Now that *this has been successfully overwritten, destruct the old nodes.
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      destruct_list(type, pnFirst);
      return *this;
   }

   /*! Returns a reference to the last element in the queue.

   @return
      Reference to the last element in the queue.
   */
   T & back() {
      type_void_adapter type;
      type.set_align<T>();
      return *static_cast<T *>(m_pnLast->value_ptr(type));
   }
   T const & back() const {
      return const_cast<queue *>(this)->back();
   }

   //! Removes all elements from the queue.
   void clear() {
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      detail::singly_linked_list_impl::clear(type);
   }

   /*! Returns a reference to the first element in the queue.

   @return
      Reference to the first element in the queue.
   */
   T & front() {
      type_void_adapter type;
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
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      // Move the value of *m_pnFirst into t, then unlink and discard *m_pnFirst.
      T t(_std::move(*static_cast<T *>(m_pnFirst->value_ptr(type))));
      detail::singly_linked_list_impl::pop_front(type);
      return _std::move(t);
   }

   /*! Adds an element to the end of the queue.

   @param t
      Element to add.
   */
   void push_back(T const & t) {
      type_void_adapter type;
      type.set_align<T>();
      type.set_copy_construct<T>();
      type.set_size<T>();
      detail::singly_linked_list_impl::push_back(type, &t, false);
   }
   void push_back(T && t) {
      type_void_adapter type;
      type.set_align<T>();
      type.set_move_construct<T>();
      type.set_size<T>();
      detail::singly_linked_list_impl::push_back(type, &t, true);
   }
};

}} //namespace abc::collections

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COLLECTIONS_QUEUE_HXX
