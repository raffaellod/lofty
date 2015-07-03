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

void * doubly_linked_list_impl::node::operator new(std::size_t cb, type_void_adapter const & type) {
   ABC_UNUSED_ARG(cb);
   /* To calculate the node size, add type.size() bytes to the offset of the value in a node at
   address 0. This allows packing the value against the end of the node, potentially using space
   that sizeof(node) would reserve as padding. */
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
      delete pn;
      pn = pnNext;
   }
}

/*static*/ void doubly_linked_list_impl::link_back(node ** ppnFirst, node ** ppnLast, node * pn) {
   ABC_TRACE_FUNC(ppnFirst, ppnLast, pn);

   pn->m_pnNext = nullptr;
   node * pnLast = *ppnLast;
   pn->m_pnPrev = pnLast;
   if (!*ppnFirst) {
      *ppnFirst = pn;
   } else if (pnLast) {
      pnLast->m_pnNext = pn;
   }
   *ppnLast = pn;
}

/*static*/ void doubly_linked_list_impl::link_front(node ** ppnFirst, node ** ppnLast, node * pn) {
   ABC_TRACE_FUNC(ppnFirst, ppnLast, pn);

   pn->m_pnPrev = nullptr;
   node * pnFirst = *ppnFirst;
   pn->m_pnNext = pnFirst;
   if (!*ppnLast) {
      *ppnLast = pn;
   } else if (pnFirst) {
      pnFirst->m_pnPrev = pn;
   }
   *ppnFirst = pn;
}

/*static*/ void doubly_linked_list_impl::remove(
   type_void_adapter const & type, node ** ppnFirst, node ** ppnLast, node * pn
) {
   ABC_TRACE_FUNC(/*type, */ppnFirst, ppnLast, pn);

   node * pnNext = pn->m_pnNext, * pnPrev = pn->m_pnPrev;
   if (pnPrev) {
      pnPrev->m_pnNext = pnNext;
   } else {
      // *ppnFirst == pn at this point – if ppnFirst was provided.
      if (*ppnFirst == pn) {
         *ppnFirst = pnNext;
      }
   }
   if (pnNext) {
      pnNext->m_pnPrev = pnPrev;
   } else {
      // *ppnLast == pn at this point – if ppnLast was provided.
      if (ppnLast) {
         *ppnLast = pnPrev;
      }
   }
   type.destruct(pn->value_ptr(type));
   delete pn;
}

}}} //namespace abc::collections::detail
