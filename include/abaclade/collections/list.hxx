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
#include <abaclade/collections/type_void_adapter.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

//! Non-template implementation class for abc::collections::list.
class ABACLADE_SYM list_impl : public support_explicit_operator_bool<list_impl> {
protected:
   //! Internal node type.
   typedef doubly_linked_list_impl::node node;

   //! Base class for list iterator implementations.
   class iterator_base {
   public:
      typedef std::ptrdiff_t difference_type;
      typedef std::bidirectional_iterator_tag iterator_category;

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
      void move_on(bool bForward);

      //! Throws an iterator_error exception if the iterator cannot be dereferenced.
      void validate() const;

   protected:
      //! Pointer to the current node.
      node * m_pn;
   };

public:
   /*! Constructor.

   @param l
      Source object.
   */
   list_impl();
   list_impl(list_impl && l);

   //! Destructor.
   ~list_impl() {
   }

   /*! Assignment operator.

   @param l
      Source object.
   */
   list_impl & operator=(list_impl && l);

   /*! Returns true if the list size is greater than 0.

   @return
      true if the list is not empty, or false otherwise.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return m_cNodes > 0;
   }

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
      true to move *pValue to the new node’s value, or false to copy it instead.
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

namespace abc { namespace collections {

//! Doubly-linked list.
template <typename T>
class list : public detail::list_impl {
private:
   template <bool t_bForward>
   class const_bidi_iterator : public detail::list_impl::iterator_base {
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
         detail::list_impl::iterator_base(pn) {
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
   /*! Constructor.

   @param l
      Source object.
   */
   list() {
   }
   list(list && l) :
      detail::list_impl(std::move(l)) {
   }

   //! Destructor.
   ~list() {
      clear();
   }

   /*! Assignment operator.

   @param l
      Source object.
   */
   list & operator=(list && l) {
      list lOld(std::move(*this));
      detail::list_impl::operator=(std::move(l));
      return *this;
   }

   /*! Returns a reference to the first element in the list.

   @return
      Reference to the first element in the list.
   */
   T & back() {
      return *detail::list_impl::back()->template value_ptr<T>();
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
      return detail::list_impl::clear(type);
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
      return *detail::list_impl::front()->template value_ptr<T>();
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
      node * pn = detail::list_impl::back();
      T tRet(std::move(*static_cast<T *>(pn->value_ptr(type))));
      detail::list_impl::remove(type, pn);
      return std::move(tRet);
   }

   /*! Removes the first element in the list.

   @return
      Former first element in the list.
   */
   T pop_front() {
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      node * pn = detail::list_impl::front();
      T tRet(std::move(*static_cast<T *>(pn->value_ptr(type))));
      detail::list_impl::remove(type, pn);
      return std::move(tRet);
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
      return iterator(detail::list_impl::push_back(type, &t, true));
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
      return iterator(detail::list_impl::push_front(type, &t, true));
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
      detail::list_impl::remove(type, it.m_pn);
   }

   //! Removes the last element in the list.
   void remove_back() {
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      detail::list_impl::remove(type, back());
   }

   //! Removes the first element in the list.
   void remove_front() {
      type_void_adapter type;
      type.set_align<T>();
      type.set_destruct<T>();
      detail::list_impl::remove(type, front());
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
