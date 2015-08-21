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

#ifndef _ABACLADE_COLLECTIONS_LIST_HXX
#define _ABACLADE_COLLECTIONS_LIST_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/collections/detail/doubly_linked_list_impl.hxx>
#include <abaclade/type_void_adapter.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections {

//! Doubly-linked list.
template <typename T>
class list : public detail::doubly_linked_list_impl {
private:
   template <bool t_bForward>
   class const_bidi_iterator : public detail::doubly_linked_list_impl::iterator_base {
   private:
      friend class list;

   public:
      typedef T * pointer;
      typedef T & reference;
      typedef T value_type;

   public:
      //! See iterator_base::iterator_base().
      const_bidi_iterator() {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current node.
      */
      T & operator*() const {
         validate();
         return *m_pn->value_ptr<T>();
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current node.
      */
      T * operator->() const {
         validate();
         return m_pn->value_ptr<T>();
      }

      /*! Preincrement operator.

      @return
         *this.
      */
      const_bidi_iterator & operator++() {
         move_on(t_bForward);
         return *this;
      }

      /*! Postincrement operator.

      @return
         Iterator pointing to the node preceding the one referenced by this iterator.
      */
      const_bidi_iterator operator++(int) {
         node * pnPrev = m_pn;
         operator++();
         return const_bidi_iterator(pnPrev);
      }

      /*! Predecrement operator.

      @return
         *this.
      */
      const_bidi_iterator & operator--() {
         move_on(!t_bForward);
         return *this;
      }

      /*! Postdecrement operator.

      @return
         Iterator pointing to the node preceding the one referenced by this iterator.
      */
      const_bidi_iterator operator--(int) {
         node * pnPrev = m_pn;
         operator--();
         return const_bidi_iterator(pnPrev);
      }

   protected:
      //! See iterator_base::iterator_base().
      const_bidi_iterator(node * pn) :
         detail::doubly_linked_list_impl::iterator_base(pn) {
      }
   };

   template <bool t_bForward>
   class bidi_iterator : public const_bidi_iterator<t_bForward> {
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
         return const_cast<T &>(const_bidi_iterator<t_bForward>::operator*());
      }

      //! See const_bidi_iterator::operator->().
      T * operator->() const {
         return const_cast<T *>(const_bidi_iterator<t_bForward>::operator->());
      }

      //! See const_bidi_iterator.operator++().
      bidi_iterator & operator++() {
         return static_cast<bidi_iterator &>(const_bidi_iterator<t_bForward>::operator++());
      }

      //! See const_bidi_iterator::operator++(int).
      bidi_iterator operator++(int) {
         return static_cast<bidi_iterator &>(const_bidi_iterator<t_bForward>::operator++(0));
      }

      //! See const_bidi_iterator.operator--().
      bidi_iterator & operator--() {
         return static_cast<bidi_iterator &>(const_bidi_iterator<t_bForward>::operator--());
      }

      //! See const_bidi_iterator::operator--(int).
      bidi_iterator operator--(int) {
         return static_cast<bidi_iterator &>(const_bidi_iterator<t_bForward>::operator--(0));
      }

   private:
      //! See const_bidi_iterator::const_bidi_iterator().
      bidi_iterator(node * pn) :
         const_bidi_iterator<t_bForward>(pn) {
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

   @param l
      Source object.
   */
   list(list && l) :
      detail::doubly_linked_list_impl(_std::move(l)) {
   }

   //! Destructor.
   ~list() {
      clear();
   }

   /*! Move-assignment operator.

   @param l
      Source object.
   @return
      *this.
   */
   list & operator=(list && l) {
      list lOld(_std::move(*this));
      detail::doubly_linked_list_impl::operator=(_std::move(l));
      return *this;
   }

   /*! Returns a reference to the first element in the list.

   @return
      Reference to the first element in the list.
   */
   T & back() {
      return *detail::doubly_linked_list_impl::back()->template value_ptr<T>();
   }
   T const & back() const {
      return const_cast<list *>(this)->back();
   }

   /*! Returns a forward iterator to the start of the list.

   @return
      Iterator to the first node in the list.
   */
   iterator begin() {
      return iterator(m_pnFirst);
   }
   const_iterator begin() const {
      return const_cast<list *>(this)->begin();
   }

   /*! Returns a const forward iterator to the start of the list.

   @return
      Iterator to the first node in the list.
   */
   const_iterator cbegin() const {
      return const_cast<list *>(this)->begin();
   }

   /*! Returns a const forward iterator to the end of the list.

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
      return detail::doubly_linked_list_impl::clear(type);
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

   /*! Returns a forward iterator to the end of the list.

   @return
      Iterator to the beyond the last node in the list.
   */
   iterator end() {
      return iterator();
   }
   const_iterator end() const {
      return const_cast<list *>(this)->end();
   }

   /*! Returns a reference to the last element in the list.

   @return
      Reference to the last element in the list.
   */
   T & front() {
      return *detail::doubly_linked_list_impl::front()->template value_ptr<T>();
   }
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
      node * pn = detail::doubly_linked_list_impl::back();
      T tRet(_std::move(*static_cast<T *>(pn->value_ptr(type))));
      detail::doubly_linked_list_impl::remove(type, pn);
      return _std::move(tRet);
   }

   /*! Removes the first element in the list.

   @return
      Former first element in the list.
   */
   T pop_front() {
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      node * pn = detail::doubly_linked_list_impl::front();
      T tRet(_std::move(*static_cast<T *>(pn->value_ptr(type))));
      detail::doubly_linked_list_impl::remove(type, pn);
      return _std::move(tRet);
   }

   /*! Adds an element to the end of the list.

   @param t
      Element to add.
   */
   iterator push_back(T t) {
      type_void_adapter type;
      type.set_align<T>();
      //type.set_copy_construct<T>();
      type.set_move_construct<T>();
      type.set_size<T>();
      return iterator(detail::doubly_linked_list_impl::push_back(type, &t, true));
   }

   /*! Adds an element to the start of the list.

   @param t
      Element to add.
   */
   iterator push_front(T t) {
      type_void_adapter type;
      type.set_align<T>();
      //type.set_copy_construct<T>();
      type.set_move_construct<T>();
      type.set_size<T>();
      return iterator(detail::doubly_linked_list_impl::push_front(type, &t, true));
   }

   /*! Returns a reverse iterator to the end of the list.

   @return
      Reverse iterator to the last node in the list.
   */
   reverse_iterator rbegin() {
      return reverse_iterator(m_pnLast);
   }
   const_reverse_iterator rbegin() const {
      return const_cast<list *>(this)->rbegin();
   }

   /*! Removes the element at the specified position.

   @param it
      Iterator to the element to remove.
   */
   void remove_at(const_iterator const & it) {
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      detail::doubly_linked_list_impl::remove(type, it.m_pn);
   }

   //! Removes the last element in the list.
   void remove_back() {
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      detail::doubly_linked_list_impl::remove(type, back());
   }

   //! Removes the first element in the list.
   void remove_front() {
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      detail::doubly_linked_list_impl::remove(type, front());
   }

   /*! Returns a reverse iterator to the start of the list.

   @return
      Reverse iterator to the before the first node in the list.
   */
   reverse_iterator rend() {
      return reverse_iterator();
   }
   const_reverse_iterator rend() const {
      return const_cast<list *>(this)->rend();
   }
};

}} //namespace abc::collections

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COLLECTIONS_LIST_HXX
