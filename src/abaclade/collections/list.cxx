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

#include <abaclade.hxx>
#include <abaclade/collections/list.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

list_impl::list_impl() :
   m_pnFirst(nullptr),
   m_pnLast(nullptr),
   m_cNodes(0) {
}
list_impl::list_impl(list_impl && li) :
   m_pnFirst(li.m_pnFirst),
   m_pnLast(li.m_pnLast),
   m_cNodes(li.m_cNodes) {
   li.m_cNodes = 0;
   li.m_pnFirst = nullptr;
   li.m_pnLast = nullptr;
}

list_impl & list_impl::operator=(list_impl && li) {
   ABC_TRACE_FUNC(this);

   // Assume that the subclass has already moved *this out.
   m_pnFirst = li.m_pnFirst;
   li.m_pnFirst = nullptr;
   m_pnLast = li.m_pnLast;
   li.m_pnLast = nullptr;
   m_cNodes = li.m_cNodes;
   li.m_cNodes = 0;
   return *this;
}

list_impl::node * list_impl::back() const {
   if (!m_pnLast) {
      ABC_THROW(null_pointer_error, ());
   }
   return m_pnLast;
}

void list_impl::clear(type_void_adapter const & type) {
   ABC_TRACE_FUNC(this/*, type*/);

   doubly_linked_list_impl::destruct_list(type, m_pnFirst);
   m_pnFirst = nullptr;
   m_pnLast = nullptr;
   m_cNodes = 0;
}

list_impl::node * list_impl::front() const {
   if (!m_pnFirst) {
      ABC_THROW(null_pointer_error, ());
   }
   return m_pnFirst;
}

list_impl::node * list_impl::push_back(type_void_adapter const & type, void const * p, bool bMove) {
   std::unique_ptr<node> pn(new(type) node);
   doubly_linked_list_impl::link_back(&m_pnFirst, &m_pnLast, pn.get());
   // Construct the node’s value.
   void * pDst = pn->value_ptr(type);
   if (bMove) {
      type.move_construct(pDst, const_cast<void *>(p));
   } else {
      type.copy_construct(pDst, p);
   }
   // Transfer ownership of *pn to the list.
   ++m_cNodes;
   return pn.release();
}

list_impl::node * list_impl::push_front(
   type_void_adapter const & type, void const * p, bool bMove
) {
   std::unique_ptr<node> pn(new(type) node);
   doubly_linked_list_impl::link_front(&m_pnFirst, &m_pnLast, pn.get());
   // Construct the node’s value.
   void * pDst = pn->value_ptr(type);
   if (bMove) {
      type.move_construct(pDst, const_cast<void *>(p));
   } else {
      type.copy_construct(pDst, p);
   }
   // Transfer ownership of *pn to the list.
   ++m_cNodes;
   return pn.release();
}

void list_impl::remove(type_void_adapter const & type, node * pn) {
   doubly_linked_list_impl::remove(type, &m_pnFirst, &m_pnLast, pn);
   --m_cNodes;
}


void list_impl::iterator_base::move_on(bool bForward) {
   ABC_TRACE_FUNC(this, bForward);

   /* Detect attempts to move past the end() of the container, or move a default-constructed
   iterator. */
   if (!m_pn) {
      ABC_THROW(iterator_error, ());
   }
   m_pn = bForward ? m_pn->next() : m_pn->prev();
}

void list_impl::iterator_base::validate() const {
   if (!m_pn) {
      ABC_THROW(iterator_error, ());
   }
}

}}} //namespace abc::collections::detail
