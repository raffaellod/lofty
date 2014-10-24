/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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
   #error Please #include <abaclade.hxx> instead of this file
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::static_list

namespace abc {

// Forward declaration.
template <typename T>
class static_list_iterator;

//! Base for classes containing static lists of other (node) classes.
template <typename T>
class static_list {
public:
   //! Base class for nodes of static_list.
   class node {
   private:
      friend class static_list;
      friend class static_list_iterator<T>;

   protected:
      //! Constructor.
      node() {
         static_list::append(this);
      }
      node(node const &) {
         // Skip copying the source’s links.
         static_list::append(this);
      }

      //! Destructor.
      ~node() {
         static_list::remove(this);
      }

      /*! Assignment operator.

      return
         *this.
      */
      node & operator=(node const &) {
         // Skip copying the source’s links.
         return *this;
      }

   private:
      /*! Returns a pointer to the next node.

      pnPrev
         Pointer to the previous node.
      */
      node * get_next(node * pnPrev) {
         return reinterpret_cast<node *>(
            m_iPrevXorNext ^ reinterpret_cast<std::uintptr_t>(pnPrev)
         );
      }

      /*! Returns a pointer to the previous node.

      pnNext
         Pointer to the next node.
      */
      node * get_prev(node * pnNext) {
         return reinterpret_cast<node *>(
            m_iPrevXorNext ^ reinterpret_cast<std::uintptr_t>(pnNext)
         );
      }

      /*! Updates the previous/next pointer.

      pnPrev
         Pointer to the previous node.
      pnNext
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

   typedef static_list_iterator<T> iterator;
   typedef std::reverse_iterator<iterator> reverse_iterator;

private:
   friend class node;

public:
   /*! Returns a forward iterator to the start of the list.

   return
      Iterator to the first node in the list.
   */
   static iterator begin();

   /*! Returns a forward iterator to the end of the list.

   return
      Iterator to the beyond the last node in the list.
   */
   static iterator end();

   /*! Returns a reverse iterator to the end of the list.

   return
      Reverse Iterator to the last node in the list.
   */
   static reverse_iterator rbegin() {
      return reverse_iterator(end());
   }

   /*! Returns a reverse iterator to the start of the list.

   return
      Reverse iterator to the before the first node in the list.
   */
   static reverse_iterator rend() {
      return reverse_iterator(begin());
   }

private:
   /*! Add a node to the end of the list.

   pn
      Pointer to the node to append.
   */
   static void append(node * pn) {
      pn->set_prev_next(nullptr, sm_pnLast);
      if (!sm_pnFirst) {
         sm_pnFirst = pn;
      } else if (sm_pnLast) {
         sm_pnLast->set_prev_next(pn, sm_pnLast->get_next(nullptr));
      }
      sm_pnLast = pn;
   }

   /*! Removes a node from the list.

   pn
      Pointer to the node to remove.
   */
   static void remove(node * pn) {
      // Find pn in the list.
      for (
         node * pnCurr = sm_pnFirst, * pnPrev = nullptr;
         pnCurr != sm_pnLast;
         std::tie(pnPrev, pnCurr) = std::make_tuple(pnCurr, pnCurr->get_next(pnPrev))
      ) {
         if (pnCurr == pn) {
            node * pnNext = pn->get_next(pnPrev);
            if (pnPrev) {
               pnPrev->set_prev_next(pnPrev->get_prev(pn), pnNext);
            } else if (sm_pnFirst == pn) {
               sm_pnFirst = pnNext;
            }
            if (pnNext) {
               pnNext->set_prev_next(pnPrev, pnNext->get_next(pn));
            } else if (sm_pnLast == pn) {
               sm_pnLast = pnPrev;
            }
            pn->set_prev_next(nullptr, nullptr);
            break;
         }
      }
   }

private:
   //! Pointer to the first node instance.
   static node * sm_pnFirst;
   //! Pointer to the last node instance.
   static node * sm_pnLast;
};

} //namespace abc

/*! Defines the static member variables for the specified abc::static_list specialization.

T
   Template argument for the target abc::static_list specialiation.
*/
#define ABC_STATIC_LIST_DEFINE_STATIC_MEMBERS(T) \
   template <> \
   static_list<T>::node * static_list<T>::sm_pnFirst = nullptr; \
   template <> \
   static_list<T>::node * static_list<T>::sm_pnLast = nullptr

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::static_list_iterator

namespace abc {

//! Iterator based on a plain pointer.
template <typename T>
class static_list_iterator :
   public std::iterator<std::bidirectional_iterator_tag, T> {
private:
   // Handy shortcut.
   typedef typename static_list<T>::node node;

public:
   /*! Constructor.

   pnCurr
      Pointer to the current node.
   pnNext
      Pointer to the node following *pnCurr.
   */
   static_list_iterator(node * pnCurr, node * pnNext) :
      m_pnCurr(pnCurr),
      m_pnNext(pnNext) {
   }

   /*! Dereferencing operator.

   return
      Reference to the current node.
   */
   T & operator*() const {
      return *static_cast<T *>(m_pnCurr);
   }

   /*! Dereferencing member access operator.

   return
      Pointer to the current node.
   */
   T * operator->() const {
      return static_cast<T *>(m_pnCurr);
   }

   /*! Preincrement operator.

   return
      *this after it’s moved to the node following the one currently pointed to by.
   */
   static_list_iterator & operator++() {
      node * pnNewPrev = m_pnCurr;
      m_pnCurr = m_pnNext;
      m_pnNext = m_pnCurr->get_next(pnNewPrev);
      return *this;
   }

   /*! Postincrement operator.

   return
      Iterator pointing to the node following the one pointed to by this iterator.
   */
   static_list_iterator operator++(int) {
      node * pnNewPrev = m_pnCurr;
      m_pnCurr = m_pnNext;
      m_pnNext = m_pnCurr->get_next(pnNewPrev);
      return static_list_iterator(pnNewPrev, m_pnCurr);
   }

   /*! Predecrement operator.

   return
      *this after it’s moved to the node preceding the one currently pointed to by.
   */
   static_list_iterator & operator--() {
      node * pnNewNext = m_pnCurr;
      m_pnCurr = m_pnCurr->get_prev(m_pnNext);
      m_pnNext = pnNewNext;
      return *this;
   }

   /*! Postdecrement operator.

   return
      Iterator pointing to the node preceding the one pointed to by this iterator.
   */
   static_list_iterator operator--(int) {
      node * pnNewNextNext = m_pnNext;
      m_pnNext = m_pnCurr;
      m_pnCurr = m_pnCurr->get_prev(pnNewNextNext);
      return static_list_iterator(m_pnNext, pnNewNextNext);
   }

// Relational operators.
#define ABC_RELOP_IMPL(op) \
   bool operator op(static_list_iterator<T> const & it) const { \
      return m_pnCurr op it.m_pnCurr; \
   }
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
#undef ABC_RELOP_IMPL

   /*! Returns the underlying iterator type.

   return
      Pointer to the current node.
   */
   node * base() const {
      return m_pnCurr;
   }

private:
   //! Pointer to the current node.
   node * m_pnCurr;
   //! Pointer to the next node.
   node * m_pnNext;
};


// Now these can be implemented.
template <typename T>
inline typename static_list<T>::iterator static_list<T>::begin() {
   return iterator(sm_pnFirst, sm_pnFirst ? sm_pnFirst->get_next(nullptr) : nullptr);
}

template <typename T>
inline typename static_list<T>::iterator static_list<T>::end() {
   return iterator(sm_pnLast, nullptr);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

