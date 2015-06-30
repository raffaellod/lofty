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
#include <abaclade/collections/detail/doubly_linked_list_impl.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

doubly_linked_list_impl::node::node(node * pnNext, node * pnPrev) :
   m_pnNext(pnNext),
   m_pnPrev(pnPrev) {
   if (pnNext) {
      pnNext->m_pnPrev = this;
   }
   if (pnPrev) {
      pnPrev->m_pnNext = this;
   }
}

doubly_linked_list_impl::node::~node() {
   // Unlink *this from the list it’s part of.
   if (m_pnNext) {
      m_pnNext->m_pnPrev = m_pnPrev;
   }
   if (m_pnPrev) {
      m_pnPrev->m_pnNext = m_pnNext;
   }
}

void * doubly_linked_list_impl::node::operator new(std::size_t cb, type_void_adapter const & type) {
   ABC_UNUSED_ARG(cb);
   /* To calculate the node size, add type.size() bytes to the offset of the value in a node at
   address 0. This allows packing the node optimally even if the unpadded node size is e.g. 6
   (sizeof will return 8 for that) and type.size() is 2, giving 8 instead of 10 (which would really
   mean at least 12 bytes, a 50% waste of memory). */
   return memory::_raw_alloc(reinterpret_cast<std::size_t>(
      static_cast<node *>(0)->value_ptr(type)
   ) + type.size());
}

void * doubly_linked_list_impl::node::value_ptr(type_void_adapter const & type) const {
   // Make sure that the argument it the address following the last member.
   return type.align_pointer(&m_pnPrev + 1);
}


/*static*/ void doubly_linked_list_impl::destruct_list(type_void_adapter const & type, node * pn) {
   ABC_TRACE_FUNC(/*type, */pn);

   while (pn) {
      node * pnNext = pn->next();
      type.destruct(pn->value_ptr(type));
      memory::_raw_free(pn);
      pn = pnNext;
   }
}

}}} //namespace abc::collections::detail
