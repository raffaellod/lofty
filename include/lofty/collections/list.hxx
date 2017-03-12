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

#ifndef _LOFTY_COLLECTIONS_LIST_HXX
#define _LOFTY_COLLECTIONS_LIST_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/collections/_pvt/doubly_linked_list_impl.hxx>
#include <lofty/type_void_adapter.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections {

//! Doubly-linked list.
template <typename T>
class list : public _pvt::doubly_linked_list_impl {
private:
   template <bool forward>
   class const_bidi_iterator : public _pvt::doubly_linked_list_impl::iterator_base {
   private:
      friend class list;

   public:
      typedef T * pointer;
      typedef T & reference;
      typedef T value_type;

   public:
      //! Default constructor.
      const_bidi_iterator() {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current node.
      */
      T & operator*() const {
         validate();
         return *nd->value_ptr<T>();
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current node.
      */
      T * operator->() const {
         validate();
         return nd->value_ptr<T>();
      }

      /*! Preincrement operator.

      @return
         *this.
      */
      const_bidi_iterator & operator++() {
         advance(forward);
         return *this;
      }

      /*! Postincrement operator.

      @return
         Iterator pointing to the node preceding the one referenced by this iterator.
      */
      const_bidi_iterator operator++(int) {
         node * prev = nd;
         operator++();
         return const_bidi_iterator(prev);
      }

      /*! Predecrement operator.

      @return
         *this.
      */
      const_bidi_iterator & operator--() {
         advance(!forward);
         return *this;
      }

      /*! Postdecrement operator.

      @return
         Iterator pointing to the node preceding the one referenced by this iterator.
      */
      const_bidi_iterator operator--(int) {
         node * prev = nd;
         operator--();
         return const_bidi_iterator(prev);
      }

   protected:
      //! See iterator_base::iterator_base().
      const_bidi_iterator(node * nd_) :
         _pvt::doubly_linked_list_impl::iterator_base(nd_) {
      }
   };

   template <bool forward>
   class bidi_iterator : public const_bidi_iterator<forward> {
   private:
      friend class list;

   public:
      typedef T * pointer;
      typedef T & reference;
      typedef T value_type;

   public:
      //! See const_bidi_iterator::const_bidi_iterator().
      bidi_iterator() {
      }

      //! See const_bidi_iterator::operator*().
      T & operator*() const {
         return const_cast<T &>(const_bidi_iterator<forward>::operator*());
      }

      //! See const_bidi_iterator::operator->().
      T * operator->() const {
         return const_cast<T *>(const_bidi_iterator<forward>::operator->());
      }

      //! See const_bidi_iterator.operator++().
      bidi_iterator & operator++() {
         return static_cast<bidi_iterator &>(const_bidi_iterator<forward>::operator++());
      }

      //! See const_bidi_iterator::operator++(int).
      bidi_iterator operator++(int) {
         return static_cast<bidi_iterator &>(const_bidi_iterator<forward>::operator++(0));
      }

      //! See const_bidi_iterator.operator--().
      bidi_iterator & operator--() {
         return static_cast<bidi_iterator &>(const_bidi_iterator<forward>::operator--());
      }

      //! See const_bidi_iterator::operator--(int).
      bidi_iterator operator--(int) {
         return static_cast<bidi_iterator &>(const_bidi_iterator<forward>::operator--(0));
      }

   private:
      //! See const_bidi_iterator::const_bidi_iterator().
      bidi_iterator(node * nd_) :
         const_bidi_iterator<forward>(nd_) {
      }
   };

public:
   typedef bidi_iterator<true> iterator;
   typedef const_bidi_iterator<true> const_iterator;
   typedef bidi_iterator<false> reverse_iterator;
   typedef const_bidi_iterator<false> const_reverse_iterator;

public:
   //! Default constructor.
   list() {
   }

   /*! Constructor.

   @param src
      Source object.
   */
   list(list && src) :
      _pvt::doubly_linked_list_impl(_std::move(src)) {
   }

   //! Destructor.
   ~list() {
      clear();
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   list & operator=(list && src) {
      list old(_std::move(*this));
      _pvt::doubly_linked_list_impl::operator=(_std::move(src));
      return *this;
   }

   /*! Returns a reference to the first element in the list.

   @return
      Reference to the first element in the list.
   */
   T & back() {
      return *_pvt::doubly_linked_list_impl::back()->template value_ptr<T>();
   }

   /*! Returns a const reference to the first element in the list.

   @return
      Const reference to the first element in the list.
   */
   T const & back() const {
      return const_cast<list *>(this)->back();
   }

   /*! Returns an iterator to the start of the list.

   @return
      Iterator to the first node in the list.
   */
   iterator begin() {
      return iterator(first_node);
   }

   /*! Returns a const iterator to the start of the list.

   @return
      Iterator to the first node in the list.
   */
   const_iterator begin() const {
      return const_cast<list *>(this)->begin();
   }

   /*! Returns a const iterator to the start of the list.

   @return
      Iterator to the first node in the list.
   */
   const_iterator cbegin() const {
      return const_cast<list *>(this)->begin();
   }

   /*! Returns a const iterator to the end of the list.

   @return
      Iterator to the beyond the last node in the list.
   */
   const_iterator cend() const {
      return const_cast<list *>(this)->end();
   }

   //! Removes all elements from the list.
   void clear() {
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      return _pvt::doubly_linked_list_impl::clear(type);
   }

   /*! Returns a const reverse iterator to the end of the list.

   @return
      Reverse iterator to the last node in the list.
   */
   const_reverse_iterator crbegin() const {
      return const_cast<list *>(this)->rbegin();
   }

   /*! Returns a const reverse iterator to the start of the list.

   @return
      Reverse iterator to the before the first node in the list.
   */
   const_reverse_iterator crend() const {
      return const_cast<list *>(this)->rend();
   }

   /*! Returns an iterator to the end of the list.

   @return
      Iterator to the beyond the last node in the list.
   */
   iterator end() {
      return iterator();
   }

   /*! Returns a const iterator to the end of the list.

   @return
      Const iterator to the beyond the last node in the list.
   */
   const_iterator end() const {
      return const_cast<list *>(this)->end();
   }

   /*! Returns a reference to the last element in the list.

   @return
      Reference to the last element in the list.
   */
   T & front() {
      return *_pvt::doubly_linked_list_impl::front()->template value_ptr<T>();
   }

   /*! Returns a const reference to the last element in the list.

   @return
      Const reference to the last element in the list.
   */
   T const & front() const {
      return const_cast<list *>(this)->front();
   }

   /*! Removes and returns the last element in the list.

   @return
      Former last element in the list.
   */
   T pop_back() {
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      node * nd = _pvt::doubly_linked_list_impl::back();
      T ret(_std::move(*static_cast<T *>(nd->value_ptr(type))));
      _pvt::doubly_linked_list_impl::remove(type, nd);
      return _std::move(ret);
   }

   /*! Removes the first element in the list.

   @return
      Former first element in the list.
   */
   T pop_front() {
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      node * nd = _pvt::doubly_linked_list_impl::front();
      T ret(_std::move(*static_cast<T *>(nd->value_ptr(type))));
      _pvt::doubly_linked_list_impl::remove(type, nd);
      return _std::move(ret);
   }

   /*! Adds an element to the end of the list.

   @param t
      Element to add.
   @return
      Iterator to the newly-added element.
   */
   iterator push_back(T t) {
      type_void_adapter type;
      type.set_align<T>();
      //type.set_copy_construct<T>();
      type.set_move_construct<T>();
      type.set_size<T>();
      return iterator(_pvt::doubly_linked_list_impl::push_back(type, &t, true));
   }

   /*! Adds an element to the start of the list.

   @param t
      Element to add.
   @return
      Iterator to the newly-added element.
   */
   iterator push_front(T t) {
      type_void_adapter type;
      type.set_align<T>();
      //type.set_copy_construct<T>();
      type.set_move_construct<T>();
      type.set_size<T>();
      return iterator(_pvt::doubly_linked_list_impl::push_front(type, &t, true));
   }

   /*! Returns a reverse iterator to the end of the list.

   @return
      Reverse iterator to the last node in the list.
   */
   reverse_iterator rbegin() {
      return reverse_iterator(last_node);
   }

   /*! Returns a reverse iterator to the end of the list.

   @return
      Reverse iterator to the last node in the list.
   */
   const_reverse_iterator rbegin() const {
      return const_cast<list *>(this)->rbegin();
   }

   /*! Removes the element at the specified position.

   @param itr
      Iterator to the element to remove.
   */
   void remove_at(const_iterator const & itr) {
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      _pvt::doubly_linked_list_impl::remove(type, itr.nd);
   }

   //! Removes the last element in the list.
   void remove_back() {
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      _pvt::doubly_linked_list_impl::remove(type, back());
   }

   //! Removes the first element in the list.
   void remove_front() {
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      _pvt::doubly_linked_list_impl::remove(type, front());
   }

   /*! Returns a reverse iterator to the start of the list.

   @return
      Reverse iterator to the before the first node in the list.
   */
   reverse_iterator rend() {
      return reverse_iterator();
   }

   /*! Returns a const reverse iterator to the start of the list.

   @return
      Const reverse iterator to the before the first node in the list.
   */
   const_reverse_iterator rend() const {
      return const_cast<list *>(this)->rend();
   }
};

}} //namespace lofty::collections

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_COLLECTIONS_LIST_HXX
