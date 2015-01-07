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
// abc::static_list

namespace abc {

/*! Allows a subclass (which must be a singleton, operating mostly via static members) to contain a
list of nodes (instances of a static_list::node subclass). Nodes are typically added to the
singleton at program startup, and this class ensures that they are removed when the program
terminates. */
template <class TContainer, class TValue>
class static_list {
public:
   //! Base class for nodes of static_list.
   class node {
   private:
      friend class static_list;
      friend class iterator;

   protected:
      //! Constructor.
      node() {
         static_list::push_back(this);
      }
      node(node const &) {
         // Skip copying the source’s links.
         static_list::push_back(this);
      }

      //! Destructor.
      ~node() {
         static_list::remove(this);
      }

      /*! Assignment operator.

      @return
         *this.
      */
      node & operator=(node const &) {
         // Skip copying the source’s links.
         return *this;
      }

   private:
      /*! Returns a pointer to the next node.

      @param pnPrev
         Pointer to the previous node.
      */
      node * get_next(node * pnPrev) {
         return reinterpret_cast<node *>(
            m_iPrevXorNext ^ reinterpret_cast<std::uintptr_t>(pnPrev)
         );
      }

      /*! Returns a pointer to the previous node.

      @param pnNext
         Pointer to the next node.
      */
      node * get_prev(node * pnNext) {
         return reinterpret_cast<node *>(
            m_iPrevXorNext ^ reinterpret_cast<std::uintptr_t>(pnNext)
         );
      }

      /*! Updates the previous/next pointer.

      @param pnPrev
         Pointer to the previous node.
      @param pnNext
         Pointer to the next node.
      */
      void set_prev_next(node * pnPrev, node * pnNext) {
         m_iPrevXorNext = reinterpret_cast<std::uintptr_t>(pnPrev) ^
                          reinterpret_cast<std::uintptr_t>(pnNext);
      }

   private:
      //! Pointer to the previous node XOR pointer to the next node.
      std::uintptr_t m_iPrevXorNext;
   };

   //! Iterator for static_list::node subclasses.
   class iterator : public std::iterator<std::bidirectional_iterator_tag, TValue> {
   public:
      /*! Constructor.

      @param pnPrev
         Pointer to the node preceding *pnCurr.
      @param pnCurr
         Pointer to the current node.
      @param pnNext
         Pointer to the node following *pnCurr.
      */
      iterator(node * pnPrev, node * pnCurr, node * pnNext) :
         m_pnPrev(pnPrev),
         m_pnCurr(pnCurr),
         m_pnNext(pnNext) {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current node.
      */
      TValue & operator*() const {
         return *static_cast<TValue *>(m_pnCurr);
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current node.
      */
      TValue * operator->() const {
         return static_cast<TValue *>(m_pnCurr);
      }

      /*! Preincrement operator.

      @return
         *this after it’s moved to the node following the one currently pointed to by.
      */
      iterator & operator++() {
         m_pnPrev = m_pnCurr;
         m_pnCurr = m_pnNext;
         m_pnNext = m_pnCurr ? m_pnCurr->get_next(m_pnPrev) : nullptr;
         return *this;
      }

      /*! Postincrement operator.

      @return
         Iterator pointing to the node following the one pointed to by this iterator.
      */
      iterator operator++(int) {
         node * pnPrevPrev = m_pnPrev;
         operator++();
         return iterator(pnPrevPrev, m_pnPrev, m_pnCurr);
      }

      /*! Predecrement operator.

      @return
         *this after it’s moved to the node preceding the one currently pointed to by.
      */
      iterator & operator--() {
         m_pnNext = m_pnCurr;
         m_pnCurr = m_pnPrev;
         m_pnPrev = m_pnCurr ? m_pnCurr->get_prev(m_pnNext) : nullptr;
         return *this;
      }

      /*! Postdecrement operator.

      @return
         Iterator pointing to the node preceding the one pointed to by this iterator.
      */
      iterator operator--(int) {
         node * pnNextNext = m_pnNext;
         operator--();
         return iterator(m_pnCurr, m_pnNext, pnNextNext);
      }

// Relational operators.
#define ABC_RELOP_IMPL(op) \
      bool operator op(iterator const & it) const { \
         return m_pnCurr op it.m_pnCurr; \
      }
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
#undef ABC_RELOP_IMPL

      /*! Returns the underlying iterator type.

      @return
         Pointer to the current node.
      */
      node * base() const {
         return m_pnCurr;
      }

   private:
      //! Pointer to the previous node.
      node * m_pnPrev;
      //! Pointer to the current node.
      node * m_pnCurr;
      //! Pointer to the next node.
      node * m_pnNext;
   };

   typedef std::reverse_iterator<iterator> reverse_iterator;

public:
   /*! Returns a forward iterator to the start of the list.

   @return
      Iterator to the first node in the list.
   */
   static iterator begin() {
      node * pnFirst = TContainer::sm_pnFirst;
      return iterator(nullptr, pnFirst, pnFirst ? pnFirst->get_next(nullptr) : nullptr);
   }

   /*! Returns a forward iterator to the end of the list.

   @return
      Iterator to the beyond the last node in the list.
   */
   static iterator end() {
      return iterator(TContainer::sm_pnLast, nullptr, nullptr);
   }

   /*! Returns a reverse iterator to the end of the list.

   @return
      Reverse Iterator to the last node in the list.
   */
   static reverse_iterator rbegin() {
      return reverse_iterator(end());
   }

   /*! Returns a reverse iterator to the start of the list.

   @return
      Reverse iterator to the before the first node in the list.
   */
   static reverse_iterator rend() {
      return reverse_iterator(begin());
   }

private:
   /*! Adds a node to the end of the list.

   @param pn
      Pointer to the node to add.
   */
   static void push_back(node * pn) {
      pn->set_prev_next(nullptr, TContainer::sm_pnLast);
      if (!TContainer::sm_pnFirst) {
         TContainer::sm_pnFirst = pn;
      } else if (node * pnLast = TContainer::sm_pnLast) {
         pnLast->set_prev_next(pn, pnLast->get_next(nullptr));
      }
      TContainer::sm_pnLast = pn;
   }

   /*! Removes a node from the list.

   @param pn
      Pointer to the node to remove.
   */
   static void remove(node * pn) {
      // Find pn in the list.
      for (
         node * pnPrev = nullptr, * pnCurr = TContainer::sm_pnFirst, * pnNext;
         pnCurr;
         pnPrev = pnCurr, pnCurr = pnNext
      ) {
         pnNext = pnCurr->get_next(pnPrev);
         if (pnCurr == pn) {
            if (pnPrev) {
               pnPrev->set_prev_next(pnPrev->get_prev(pnCurr), pnNext);
            } else if (TContainer::sm_pnFirst == pnCurr) {
               TContainer::sm_pnFirst = pnNext;
            }
            if (pnNext) {
               pnNext->set_prev_next(pnPrev, pnNext->get_next(pnCurr));
            } else if (TContainer::sm_pnLast == pnCurr) {
               TContainer::sm_pnLast = pnPrev;
            }
            pnCurr->set_prev_next(nullptr, nullptr);
            break;
         }
      }
   }
};

} //namespace abc

/*! Declares the static member variables for the specified abc::static_list-derived class.

@param container
   Class derived from abc::static_list.
*/
#define ABC_STATIC_LIST_DECLARE_SUBCLASS_STATIC_MEMBERS(container) \
   /*! Pointer to the first node. */ \
   static node * sm_pnFirst; \
   /*! Pointer to the last node. */ \
   static node * sm_pnLast;

/*! Defines the static member variables for the specified abc::static_list-derived class.

@param container
   Class derived from abc::static_list.
*/
#define ABC_STATIC_LIST_DEFINE_SUBCLASS_STATIC_MEMBERS(container) \
   container::node * container::sm_pnFirst = nullptr; \
   container::node * container::sm_pnLast = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////////
