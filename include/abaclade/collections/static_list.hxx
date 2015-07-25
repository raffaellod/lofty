/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014, 2015
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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections {

/*! Implements the template argument-independent methods of static_list. A final subclass must also
inherit from abc::collections::static_list_impl or a subclass thereof. */
class ABACLADE_SYM static_list_impl_base {
public:
   /*! Implemenets the template argument-independent methods of static_list::node. A final subclass
   must also inherit from abc::collections::static_list_impl::node or a subclass thereof. */
   class node {
   public:
      /*! Returns a pointer to the next or previous node given the opposite one.

      @param pnSibling
         Pointer to the previous or next node.
      */
      node * get_other_sibling(node const * pnSibling) {
         return reinterpret_cast<node *>(
            m_iPrevXorNext ^ reinterpret_cast<std::uintptr_t>(pnSibling)
         );
      }

      /*! Updates the previous/next pointer.

      @param pnPrev
         Pointer to the previous node.
      @param pnNext
         Pointer to the next node.
      */
      void set_siblings(node const * pnPrev, node const * pnNext) {
         m_iPrevXorNext = reinterpret_cast<std::uintptr_t>(pnPrev) ^
                          reinterpret_cast<std::uintptr_t>(pnNext);
      }

   private:
      //! Pointer to the previous node XOR pointer to the next node.
      std::uintptr_t m_iPrevXorNext;
   };

   //! Non-template base class for static_list_impl::iterator.
   class ABACLADE_SYM iterator {
   public:
      /*! Dereferencing operator.

      @return
         Reference to the current node.
      */
      node & operator*() const {
         validate();
         return *m_pnCurr;
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current node.
      */
      node * operator->() const {
         validate();
         return m_pnCurr;
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
         node * pnPrev = m_pnCurr;
         increment();
         return iterator(pnPrev, m_pnCurr);
      }

      /*! Equality relational operator.

      @param it
         Object to compare to *this.
      @return
         true if *this refers to the same element as it, or false otherwise.
      */
      bool operator==(iterator const & it) const {
         return m_pnCurr == it.m_pnCurr;
      }

      /*! Inequality relational operator.

      @param it
         Object to compare to *this.
      @return
         true if *this refers to a different element than it, or false otherwise.
      */
      bool operator!=(iterator const & it) const {
         return !operator==(it);
      }

      /*! Returns the underlying pointer to the node.

      @return
         Pointer to the current node.
      */
      node * base() {
         return m_pnCurr;
      }
      node const * base() const {
         return m_pnCurr;
      }

      /*! Returns a pointer to the next node.

      @return
         Pointer to the next node.
      */
      node * next_base() {
         return m_pnNext;
      }
      node const * next_base() const {
         return m_pnNext;
      }

   //protected:
      //! Default constructor.
      iterator() :
         m_pnCurr(nullptr),
         m_pnNext(nullptr) {
      }

      /*! Constructor.

      @param pnCurr
         Pointer to the current node.
      @param pnNext
         Pointer to the node following *pnCurr.
      */
      iterator(node * pnCurr, node * pnNext) :
         m_pnCurr(pnCurr),
         m_pnNext(pnNext) {
      }

   private:
      //! Moves the iterator to next node.
      void increment();

      /*! Throws an iterator_error exception if the iterator is at the end of the container or has
      been invalidated by a change in the container. */
      void validate() const;

   protected:
      //! Pointer to the current node.
      node * m_pnCurr;
      //! Pointer to the next node.
      node * m_pnNext;
   };

   typedef iterator reverse_iterator;

public:
   /*! Returns true if the list size is greater than 0.

   @return
      true if the list is not empty, or false otherwise.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return m_pnFirst != nullptr;
   }

   /*! Returns a forward iterator to the start of the list.

   @return
      Iterator to the first node in the list.
   */
   iterator begin() const {
      node * pnFirst = m_pnFirst;
      return iterator(pnFirst, pnFirst ? pnFirst->get_other_sibling(nullptr) : nullptr);
   }

   /*! Returns true if the list contains no elements.

   @return
      true if the list is empty, or false otherwise.
   */
   bool empty() {
      return !m_pnFirst;
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
      node * pnLast = m_pnLast;
      return reverse_iterator(pnLast, pnLast ? pnLast->get_other_sibling(nullptr) : nullptr);
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

//protected:
   /*! Inserts a node to the end of the list.

   @param pn
      Pointer to the node to become the last in the list.
   */
   void link_back(node * pn);

   /*! Inserts a node to the start of the list.

   @param pn
      Pointer to the node to become the first in the list.
   */
   void link_front(node * pn);

   /*! Removes a node from the list.

   @param pn
      Pointer to the node to remove.
   */
   void unlink(node * pn);

   /*! Removes a node from the list.

   @param pn
      Pointer to the node to unlink.
   @param pnPrev
      Pointer to the node preceding *pn.
   @param pnNext
      Pointer to the node following *pn.
   */
   void unlink(node * pn, node * pnPrev, node * pnNext);

// All data members must be public to allow initialization via static initializer.
public:
   //! Pointer to the first node.
   node * m_pnFirst;
   //! Pointer to the last node.
   node * m_pnLast;
};

//! Initial value for an abc::collections::static_list_impl_base instance.
#define ABC_COLLECTIONS_STATIC_LIST_IMPL_BASE_INITIALIZER \
   { nullptr, nullptr }

}} //namespace abc::collections

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections {

/*! Implements the template argument-dependent methods of static_list. A final subclass must also
inherit from abc::collections::static_list_impl_base or a subclass thereof. */
template <class TContainer, class TValue>
class static_list_impl {
public:
   /*! Implements the template argument-dependent methods of static_list::node. A final subclass
   must also inherit from abc::collections::static_list_impl_base::node or a subclass thereof. */
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
      public std::iterator<std::forward_iterator_tag, TValue> {
   private:
      typedef static_list_impl_base::iterator iterator_base;

   public:
      typedef TValue * pointer;
      typedef TValue & reference;
      typedef TValue value_type;

   public:
      //! Default constructor.
      iterator() {
      }

      /*! Constructor that converts a iterator_base.

      @param ii
         Source object.
      */
      iterator(iterator_base const & ii) :
         iterator_base(ii) {
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
      TValue const * base() const {
         return static_cast<TValue *>(iterator_base::base());
      }

      //! See static_list_impl_base::iterator::next_base().
      TValue * next_base() {
         return static_cast<TValue *>(iterator_base::next_base());
      }
      TValue const * next_base() const {
         return static_cast<TValue *>(iterator_base::next_base());
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
      return iterator(pslib->rbegin());
   }

   /*! Returns a reverse iterator to the start of the list.

   @return
      Reverse iterator to the before the first node in the list.
   */
   reverse_iterator rend() const {
      return reverse_iterator();
   }
};

}} //namespace abc::collections

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections {

/*! Allows a subclass to contain a list of nodes (instances of a static_list::node subclass).

A subclass must be a singleton with an instance() static method.

Nodes are added to the singleton at program startup, and this class ensures that they are removed
when the program terminates. */
template <class TContainer, class TValue>
class static_list : public static_list_impl_base, public static_list_impl<TContainer, TValue> {
public:
   typedef static_list_impl_base data_members;

private:
   typedef static_list_impl<TContainer, TValue> impl;

public:
   /*! Base for classes implementing nodes of a static_list subclass. Makes each subclass instance
   add itself to the related static_list subclass singleton. */
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

//! Initial value for an abc::collections::static_list_impl_base instance.
#define ABC_COLLECTIONS_STATIC_LIST_INITIALIZER \
   ABC_COLLECTIONS_STATIC_LIST_IMPL_BASE_INITIALIZER

}} //namespace abc::collections
