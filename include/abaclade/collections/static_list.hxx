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
// abc::collections::static_list

namespace abc {
namespace collections {

/*! Allows a subclass (which must be a singleton, operating mostly via static members) to contain a
list of nodes (instances of a static_list::node subclass). Nodes are typically added to the
singleton at program startup, and this class ensures that they are removed when the program
terminates. */
template <class TContainer, class TValue>
class static_list {
private:
   typedef detail::xor_list::node node_impl;

public:
   /*! Base class for nodes of static_list. Makes each subclass instance add itself to the related
   static_list subclass singleton. */
   class node : public node_impl {
   public:
      /*! Returns a pointer to the contained TValue.

      @return
         Pointer to the contained value.
      */
      TValue * value_ptr() {
         return static_cast<TValue *>(this);
      }
      TValue const * get_value() const {
         return static_cast<TValue const *>(this);
      }

   protected:
      //! Constructor.
      node() {
         static_list::push_back(this);
      }
      node(node const &) {
         static_list::push_back(this);
      }

      //! Destructor.
      ~node() {
         static_list::remove(this);
      }
   };

   //! Iterator for static_list::node subclasses.
   class iterator : public detail::xor_list::iterator<iterator, node, TValue> {
   public:
      //! See detail::xor_list::iterator::iterator().
      iterator(node_impl * pnPrev, node_impl * pnCurr, node_impl * pnNext) :
         detail::xor_list::iterator<iterator, node, TValue>(pnPrev, pnCurr, pnNext) {
      }
   };

   typedef std::reverse_iterator<iterator> reverse_iterator;

public:
   /*! Returns a forward iterator to the start of the list.

   @return
      Iterator to the first node in the list.
   */
   static iterator begin() {
      node_impl * pnFirst = TContainer::sm_pnFirst;
      return iterator(nullptr, pnFirst, pnFirst ? pnFirst->get_next(nullptr) : nullptr);
   }

   /*! Returns true if the list contains no elements.

   @return
      true if the list is empty, or false otherwise.
   */
   static bool empty() {
      return !TContainer::sm_pnFirst && !TContainer::sm_pnLast;
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

   /*! Returns the count of elements in the list.

   @return
      Count of elements.
   */
   static std::size_t size() {
      return static_cast<std::size_t>(std::distance(begin(), end()));
   }

private:
   /*! Adds a node to the end of the list.

   @param pn
      Pointer to the node to add.
   */
   static void push_back(node_impl * pn) {
      pn->set_prev_next(nullptr, TContainer::sm_pnLast);
      if (!TContainer::sm_pnFirst) {
         TContainer::sm_pnFirst = pn;
      } else if (node_impl * pnLast = TContainer::sm_pnLast) {
         pnLast->set_prev_next(pn, pnLast->get_next(nullptr));
      }
      TContainer::sm_pnLast = pn;
   }

   /*! Removes a node from the list.

   @param pn
      Pointer to the node to remove.
   */
   static void remove(node_impl * pn) {
      // Find pn in the list.
      for (
         node_impl * pnPrev = nullptr, * pnCurr = TContainer::sm_pnFirst, * pnNext;
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

} //namespace collections
} //namespace abc

/*! Declares the static member variables for the specified abc::collections::static_list subclass.

@param container
   Class derived from abc::collections::static_list.
*/
#define ABC_COLLECTIONS_STATIC_LIST_DECLARE_SUBCLASS_STATIC_MEMBERS(container) \
   /*! Pointer to the first node. */ \
   static ::abc::collections::detail::xor_list::node * sm_pnFirst; \
   /*! Pointer to the last node. */ \
   static ::abc::collections::detail::xor_list::node * sm_pnLast;

/*! Defines the static member variables for the specified abc::collections::static_list subclass.

@param container
   Class derived from abc::collections::static_list.
*/
#define ABC_COLLECTIONS_STATIC_LIST_DEFINE_SUBCLASS_STATIC_MEMBERS(container) \
   ::abc::collections::detail::xor_list::node * container::sm_pnFirst = nullptr; \
   ::abc::collections::detail::xor_list::node * container::sm_pnLast = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////////
