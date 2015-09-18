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

#include <abaclade/type_void_adapter.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

//! Non-template implementation of a doubly-linked list.
class ABACLADE_SYM doubly_linked_list_impl :
   public support_explicit_operator_bool<doubly_linked_list_impl> {
public:
   //! Doubly-linked list node that also stores a single value.
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

      /*! Returns a pointer to the previous node.

      @return
         Pointer to the previous node.
      */
      node * prev() const {
         return m_pnPrev;
      }

      /*! Removes the node from the list it’s in.

      @param ppnFirst
         Pointer to the list’s first node pointer. May be nullptr.
      @param ppnLast
         Pointer to the list’s last node pointer. May be nullptr.
      */
      void unlink(node ** ppnFirst, node ** ppnLast);

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

protected:
   //! Base class for list iterator implementations.
   class ABACLADE_SYM iterator_base {
   public:
      typedef std::ptrdiff_t difference_type;
      typedef _std::bidirectional_iterator_tag iterator_category;

   public:
      /*! Equality relational operator.

      @param it
         Object to compare to *this.
      @return
         true if *this refers to the same element as it, or false otherwise.
      */
      bool operator==(iterator_base const & it) const {
         return m_pn == it.m_pn;
      }

      /*! Inequality relational operator.

      @param it
         Object to compare to *this.
      @return
         true if *this refers to a different element than it, or false otherwise.
      */
      bool operator!=(iterator_base const & it) const {
         return !operator==(it);
      }

   protected:
      //! Default constructor.
      iterator_base() :
         m_pn(nullptr) {
      }

      /*! Constructor.

      @param pn
         Pointer to the referred node.
      */
      iterator_base(node * pn) :
         m_pn(pn) {
      }

      /*! Moves the iterator to the previous or next node.

      @param bForward
         If true, the iterator will move to the next node; if false, the iterator will move to the
         previous node.
      */
      void advance(bool bForward);

      //! Throws a collections::out_of_range exception if the iterator cannot be dereferenced.
      void validate() const;

   protected:
      //! Pointer to the current node.
      node * m_pn;
   };

public:
   //! Default constructor.
   doubly_linked_list_impl();

   /*! Move constructor.

   @param dlli
      Source object.
   */
   doubly_linked_list_impl(doubly_linked_list_impl && dlli);

   //! Destructor.
   ~doubly_linked_list_impl() {
   }

   /*! Move-assignment operator.

   @param dlli
      Source object.
   @return
      *this.
   */
   doubly_linked_list_impl & operator=(doubly_linked_list_impl && dlli);

   /*! Boolean evaluation operator.

   @return
      true if the list is not empty, or false otherwise.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return m_cNodes > 0;
   }

   /*! Recursively destructs a list and all its value nodes.

   @param type
      Adapter for the value’s type.
   @param pn
      Pointer to the first list node.
   */
   static void destruct_list(type_void_adapter const & type, node * pn);

   /*! Returns true if the list contains no elements.

   @return
      true if the list is empty, or false otherwise.
   */
   bool empty() const {
      return m_cNodes == 0;
   }

   /*! Inserts a node at the end of the list.

   @param type
      Adapter for the value’s type.
   @param ppnFirst
      Pointer to the list’s first node pointer.
   @param ppnLast
      Pointer to the list’s last node pointer.
   @param p
      Pointer to the value to add.
   @param bMove
      true to move *p to the new node’s value, or false to copy it instead.
   @return
      Pointer to the newly-added node.
   */
   static node * push_back(
      type_void_adapter const & type, node ** ppnFirst, node ** ppnLast, void const * p, bool bMove
   );

   /*! Inserts a node at the start of the list.

   @param type
      Adapter for the value’s type.
   @param ppnFirst
      Pointer to the list’s first node pointer.
   @param ppnLast
      Pointer to the list’s last node pointer.
   @param p
      Pointer to the value to add.
   @param bMove
      true to move *p to the new node’s value, or false to copy it instead.
   @return
      Pointer to the newly-added node.
   */
   static node * push_front(
      type_void_adapter const & type, node ** ppnFirst, node ** ppnLast, void const * p, bool bMove
   );

   /*! Unlinks and destructs a node from the list.

   @param type
      Adapter for the value’s type.
   @param ppnFirst
      Pointer to the list’s first node pointer. May be nullptr.
   @param ppnLast
      Pointer to the list’s last node pointer. May be nullptr.
   @param pn
      Pointer to the node to unlink.
   */
   static void remove(type_void_adapter const & type, node ** ppnFirst, node ** ppnLast, node * pn);

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
   @param p
      Pointer to the value to add.
   @param bMove
      true to move *pValue to the new node’s value, or false to copy it instead.
   @return
      Pointer to the newly-added node.
   */
   node * push_back(type_void_adapter const & type, void const * p, bool bMove);

   /*! Inserts a node to the start of the list.

   @param type
      Adapter for the value’s type.
   @param p
      Pointer to the value to add.
   @param bMove
      true to move *p to the new node’s value, or false to copy it instead.
   @return
      Pointer to the newly-added node.
   */
   node * push_front(type_void_adapter const & type, void const * p, bool bMove);

   /*! Unlinks and destructs a node in the list.

   @param type
      Adapter for the value’s type.
   @param pn
      Pointer to the node to unlink.
   */
   void remove(type_void_adapter const & type, node * pn);

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

#endif //ifndef _ABACLADE_COLLECTIONS_DETAIL_DOUBLY_LINKED_LIST_IMPL_HXX
