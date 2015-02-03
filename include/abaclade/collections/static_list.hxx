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
public:
   /*! Base class for nodes of static_list. Makes each subclass instance add itself to the related
   static_list subclass singleton. */
   class node : public detail::xor_list::node {
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

   typedef detail::xor_list::iterator<node, TValue> iterator;
   typedef iterator reverse_iterator;

public:
   /*! Returns a forward iterator to the start of the list.

   @return
      Iterator to the first node in the list.
   */
   static iterator begin() {
      detail::xor_list::node * pnFirst = TContainer::sm_pnFirst;
      return iterator(
         pnFirst, pnFirst ? pnFirst->get_other_sibling(nullptr) : nullptr, &TContainer::sm_iRev
      );
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
      return iterator();
   }

   /*! Returns a reverse iterator to the end of the list.

   @return
      Reverse iterator to the last node in the list.
   */
   static reverse_iterator rbegin() {
      detail::xor_list::node * pnLast = TContainer::sm_pnLast;
      return reverse_iterator(
         pnLast, pnLast ? pnLast->get_other_sibling(nullptr) : nullptr, &TContainer::sm_iRev
      );
   }

   /*! Returns a reverse iterator to the start of the list.

   @return
      Reverse iterator to the before the first node in the list.
   */
   static reverse_iterator rend() {
      return reverse_iterator();
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
   static void push_back(detail::xor_list::node * pn) {
      detail::xor_list::link_back(
         pn, &TContainer::sm_pnFirst, &TContainer::sm_pnLast, &TContainer::sm_iRev
      );
   }

   /*! Removes a node from the list.

   @param pn
      Pointer to the node to remove.
   */
   static void remove(detail::xor_list::node * pn) {
      // Find pn in the list.
      // TODO: this should be de-templated in xor_list for the most part.
      for (auto it(begin()); it != end(); ++it) {
         if (it.base() == pn) {
            detail::xor_list::unlink(
               pn, const_cast<node *>(it.next_base()),
               &TContainer::sm_pnFirst, &TContainer::sm_pnLast, &TContainer::sm_iRev
            );
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
   static ::abc::collections::detail::xor_list::node * sm_pnLast; \
   /*! Indicates the revision number of the list contents. */ \
   static ::abc::collections::detail::xor_list::rev_int_t sm_iRev;

/*! Defines the static member variables for the specified abc::collections::static_list subclass.

@param container
   Class derived from abc::collections::static_list.
*/
#define ABC_COLLECTIONS_STATIC_LIST_DEFINE_SUBCLASS_STATIC_MEMBERS(container) \
   ::abc::collections::detail::xor_list::node * container::sm_pnFirst = nullptr; \
   ::abc::collections::detail::xor_list::node * container::sm_pnLast = nullptr; \
   ::abc::collections::detail::xor_list::rev_int_t container::sm_iRev = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////
