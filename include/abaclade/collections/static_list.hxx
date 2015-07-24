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

//! Non-template base class for static_list_node.
typedef detail::xor_list_node static_list_node_base;

/*! Base class for nodes of static_list. Makes each subclass instance add itself to the related
static_list subclass singleton; subclasses must also inherit from static_list_node_base. */
template <class TContainer, class TValue>
class static_list_node {
protected:
   //! Constructor.
   static_list_node() {
      TContainer::instance().link_back(static_cast<TValue *>(this));
   }

   //! Destructor.
   ~static_list_node() {
      TContainer::instance().unlink(static_cast<TValue *>(this));
   }
};

//! Defines the minimal data members needed to implement a static_list subclass.
typedef detail::xor_list_data_members static_list_data_members;

//! Initial value for the data members of an abc::collections::static_list::data_members instance.
#define ABC_COLLECTIONS_STATIC_LIST_INITIALIZER \
   ABC_COLLECTIONS_DETAIL_XOR_LIST_IMPL_INITIALIZER

/*! Allows a subclass (which must be a singleton with an instance() static method) to contain a list
of nodes (instances of a static_list_node subclass). Nodes are typically added to the singleton at
program startup, and this class ensures that they are removed when the program terminates. */
template <class TContainer, class TValue>
class static_list : public detail::xor_list_impl {
private:
   friend class static_list_node<TContainer, TValue>;

public:
   typedef detail::xor_list_impl::iterator<TValue> iterator;
   typedef iterator reverse_iterator;

protected:
   //! Data members needed to implement a static_list subclass.
   typedef static_list_data_members data_members;

public:
   /*! Returns a forward iterator to the start of the list.

   @return
      Iterator to the first node in the list.
   */
   iterator begin() const {
      static_list_node_base * pnFirst = m_pnFirst;
      return iterator(pnFirst, pnFirst ? pnFirst->get_other_sibling(nullptr) : nullptr);
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
      static_list_node_base * pnLast = m_pnLast;
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
   std::size_t size() const {
      return static_cast<std::size_t>(std::distance(begin(), end()));
   }
};

}} //namespace abc::collections
