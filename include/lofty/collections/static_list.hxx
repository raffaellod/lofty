/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2015, 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_HXX_INTERNAL
   #error "Please #include <lofty.hxx> instead of this file"
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections {

/*! Implements the template argument-independent methods of static_list. A final subclass must also inherit
from lofty::collections::static_list_impl or a subclass thereof. */
class LOFTY_SYM static_list_impl_base {
public:
   /*! Implemenets the template argument-independent methods of static_list::node. A final subclass must also
   inherit from lofty::collections::static_list_impl::node or a subclass thereof. */
   class node {
   private:
      friend class static_list_impl_base;

   private:
      /*! Returns a pointer to the next or previous node given the opposite one.

      @param sibling
         Pointer to the previous or next node.
      */
      node * get_other_sibling(node const * sibling) {
         return reinterpret_cast<node *>(prev_xor_next ^ reinterpret_cast<std::uintptr_t>(sibling));
      }

      /*! Updates the previous/next pointer.

      @param prev
         Pointer to the previous node.
      @param next
         Pointer to the next node.
      */
      void set_siblings(node const * prev, node const * next) {
         prev_xor_next = reinterpret_cast<std::uintptr_t>(prev) ^ reinterpret_cast<std::uintptr_t>(next);
      }

   private:
      //! Pointer to the previous node XOR pointer to the next node.
      std::uintptr_t prev_xor_next;
   };

   //! Non-template base class for static_list_impl::iterator.
   class LOFTY_SYM iterator {
   private:
      friend class static_list_impl_base;

   public:
      /*! Dereferencing operator.

      @return
         Reference to the current node.
      */
      node & operator*() const {
         validate();
         return *curr;
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current node.
      */
      node * operator->() const {
         validate();
         return curr;
      }

      /*! Preincrement operator.

      @return
         *this.
      */
      iterator & operator++() {
         increment();
         return *this;
      }

      /*! Postincrement operator.

      @return
         Iterator pointing to the node preceding the one referenced by this iterator.
      */
      iterator operator++(int) {
         node * prev = curr;
         increment();
         return iterator(prev, curr);
      }

      /*! Equality relational operator.

      @param other
         Object to compare to *this.
      @return
         true if *this refers to the same element as other, or false otherwise.
      */
      bool operator==(iterator const & other) const {
         return curr == other.curr;
      }

      /*! Inequality relational operator.

      @param other
         Object to compare to *this.
      @return
         true if *this refers to a different element than other, or false otherwise.
      */
      bool operator!=(iterator const & other) const {
         return !operator==(other);
      }

      /*! Returns the underlying pointer to the node.

      @return
         Pointer to the current node.
      */
      node * base() {
         return curr;
      }

      /*! Returns the underlying const pointer to the node.

      @return
         Const pointer to the current node.
      */
      node const * base() const {
         return curr;
      }

      /*! Returns a pointer to the next node.

      @return
         Pointer to the next node.
      */
      node * next_base() {
         return next;
      }

      /*! Returns a const pointer to the next node.

      @return
         Const pointer to the next node.
      */
      node const * next_base() const {
         return next;
      }

   protected:
      //! Default constructor.
      iterator() :
         curr(nullptr),
         next(nullptr) {
      }

      /*! Constructor.

      @param curr_
         Pointer to the current node.
      @param next_
         Pointer to the node following *curr_.
      */
      iterator(node * curr_, node * next_) :
         curr(curr_),
         next(next_) {
      }

   private:
      //! Moves the iterator to next node.
      void increment();

      /*! Throws a collections::out_of_range exception if the iterator is at the end of the container or has
      been invalidated by a change in the container. */
      void validate() const;

   private:
      //! Pointer to the current node.
      node * curr;
      //! Pointer to the next node, or the previous node if the iterator is reversed.
      node * next;
   };

   typedef iterator reverse_iterator;

public:
   /*! Boolean evaluation operator.

   @return
      true if the list is not empty, or false otherwise.
   */
   LOFTY_EXPLICIT_OPERATOR_BOOL() const {
      return first != nullptr;
   }

   /*! Returns a forward iterator to the start of the list.

   @return
      Iterator to the first node in the list.
   */
   iterator begin() const {
      return iterator(first, first ? first->get_other_sibling(nullptr) : nullptr);
   }

   /*! Returns true if the list contains no elements.

   @return
      true if the list is empty, or false otherwise.
   */
   bool empty() {
      return !first;
   }

   /*! Returns a forward iterator to the end of the list.

   @return
      Iterator to the beyond the last node in the list.
   */
   iterator end() const {
      return iterator();
   }

   /*! Inserts a node to the end of the list.

   @param nd
      Pointer to the node to become the last in the list.
   */
   void link_back(node * nd);

   /*! Inserts a node to the start of the list.

   @param nd
      Pointer to the node to become the first in the list.
   */
   void link_front(node * nd);

   /*! Returns a reverse iterator to the end of the list.

   @return
      Reverse iterator to the last node in the list.
   */
   reverse_iterator rbegin() const {
      return reverse_iterator(last, last ? last->get_other_sibling(nullptr) : nullptr);
   }

   /*! Returns a reverse iterator to the start of the list.

   @return
      Reverse iterator to the before the first node in the list.
   */
   reverse_iterator rend() const {
      return reverse_iterator();
   }

   /*! Returns the count of elements in the list.

   @return
      Count of elements.
   */
   std::size_t size() const;

   /*! Removes a node from the list.

   @param nd
      Pointer to the node to remove.
   */
   void unlink(node * nd);

   /*! Removes a node from the list.

   @param nd
      Pointer to the node to unlink.
   @param prev
      Pointer to the node preceding *nd.
   @param next
      Pointer to the node following *nd.
   */
   void unlink(node * nd, node * prev, node * next);

// All data members must be public to allow initialization via static initializer.
public:
   //! Pointer to the first node.
   node * first;
   //! Pointer to the last node.
   node * last;
};

//! Initial value for a lofty::collections::static_list_impl_base instance.
#define LOFTY_COLLECTIONS_STATIC_LIST_IMPL_BASE_INITIALIZER \
   { nullptr, nullptr }

}} //namespace lofty::collections

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections {

/*! Implements the template argument-dependent methods of static_list. A final subclass must also inherit from
lofty::collections::static_list_impl_base or a subclass thereof. */
template <class TContainer, class TValue>
class static_list_impl {
public:
   /*! Implements the template argument-dependent methods of static_list::node. A final subclass must also
   inherit from lofty::collections::static_list_impl_base::node or a subclass thereof. */
   class node {
   protected:
      //! Constructor.
      node() {
         TContainer::instance().link_back(static_cast<TValue *>(this));
      }

      //! Destructor.
      ~node() {
         TContainer::instance().unlink(static_cast<TValue *>(this));
      }
   };

   //! Iterator for XOR doubly-linked list node classes.
   class iterator :
      public static_list_impl_base::iterator,
      public _std::iterator<_std::forward_iterator_tag, TValue> {
   private:
      friend class static_list_impl;

      typedef static_list_impl_base::iterator iterator_base;

   public:
      typedef TValue * pointer;
      typedef TValue & reference;
      typedef TValue value_type;

   public:
      //! Default constructor.
      iterator() {
      }

      //! See static_list_impl_base::iterator::operator*().
      TValue & operator*() const {
         return static_cast<TValue &>(iterator_base::operator*());
      }

      //! See static_list_impl_base::iterator::operator->().
      TValue * operator->() const {
         return static_cast<TValue *>(iterator_base::operator->());
      }

      //! See static_list_impl_base::iterator::operator++().
      iterator & operator++() {
         return static_cast<iterator &>(iterator_base::operator++());
      }

      //! See static_list_impl_base::iterator::operator++(int).
      iterator operator++(int) {
         return iterator(iterator_base::operator++(0));
      }

      //! See static_list_impl_base::iterator::base().
      TValue * base() {
         return static_cast<TValue *>(iterator_base::base());
      }

      //! See static_list_impl_base::iterator::base().
      TValue const * base() const {
         return static_cast<TValue *>(iterator_base::base());
      }

      //! See static_list_impl_base::iterator::next_base().
      TValue * next_base() {
         return static_cast<TValue *>(iterator_base::next_base());
      }

      //! See static_list_impl_base::iterator::next_base().
      TValue const * next_base() const {
         return static_cast<TValue *>(iterator_base::next_base());
      }

   private:
      /*! Constructor that converts from an iterator_base.

      @param ii
         Source object.
      */
      iterator(iterator_base const & ii) :
         iterator_base(ii) {
      }
   };

   typedef iterator reverse_iterator;

public:
   /*! Returns a forward iterator to the start of the list.

   @return
      Iterator to the first node in the list.
   */
   iterator begin() const {
      static_list_impl_base const * pslib = static_cast<TContainer const *>(this);
      return iterator(pslib->begin());
   }

   /*! Returns a forward iterator to the end of the list.

   @return
      Iterator to the beyond the last node in the list.
   */
   iterator end() const {
      return iterator();
   }

   /*! Returns a reverse iterator to the end of the list.

   @return
      Reverse iterator to the last node in the list.
   */
   reverse_iterator rbegin() const {
      static_list_impl_base const * pslib = static_cast<TContainer const *>(this);
      return reverse_iterator(pslib->rbegin());
   }

   /*! Returns a reverse iterator to the start of the list.

   @return
      Reverse iterator to the before the first node in the list.
   */
   reverse_iterator rend() const {
      return reverse_iterator();
   }
};

}} //namespace lofty::collections

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections {

/*! Allows a subclass to contain a list of nodes (instances of a static_list::node subclass).
 
A subclass must be a singleton with an instance() static method.

Nodes are added to the singleton at program startup, and this class ensures that they are removed when the
program terminates. */
template <class TContainer, class TValue>
class static_list : public static_list_impl_base, public static_list_impl<TContainer, TValue> {
public:
   typedef static_list_impl_base data_members;

private:
   typedef static_list_impl<TContainer, TValue> impl;

public:
   /*! Base for classes implementing nodes of a static_list subclass. Makes each subclass instance add itself
   to the related static_list subclass singleton. */
   class node : public static_list_impl_base::node, public impl::node {
   };

   typedef typename impl::iterator iterator;
   typedef typename impl::reverse_iterator reverse_iterator;

public:
   using impl::begin;
   using impl::end;
   using impl::rbegin;
   using impl::rend;
};

//! Initial value for a lofty::collections::static_list_impl_base instance.
#define LOFTY_COLLECTIONS_STATIC_LIST_INITIALIZER \
   LOFTY_COLLECTIONS_STATIC_LIST_IMPL_BASE_INITIALIZER

}} //namespace lofty::collections
