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
#include <abaclade/collections/detail/singly_linked_list_impl.hxx>
#include <abaclade/type_void_adapter.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

void * singly_linked_list_impl::node::operator new(std::size_t cb, type_void_adapter const & type) {
   ABC_UNUSED_ARG(cb);
   /* To calculate the node size, pack the value against the end of the node, potentially using
   space that cb (== sizeof(node)) would reserve as padding. */
   return memory::alloc<void>(type.align_offset(ABC_UNPADDED_SIZEOF(node, m_pnNext)) + type.size());
}

singly_linked_list_impl::node::node(
   type_void_adapter const & type, node ** ppnFirst, node ** ppnLast, node * pnPrev, node * pnNext,
   void const * p, bool bMove
) :
   m_pnNext(pnNext) {
   ABC_TRACE_FUNC(this/*, type*/, ppnFirst, ppnLast, pnPrev, pnNext, p, bMove);

   // Copy- or move-onstruct the value of the node.
   void * pDst = value_ptr(type);
   if (bMove) {
      type.move_construct(pDst, const_cast<void *>(p));
   } else {
      type.copy_construct(pDst, p);
   }
   // If no exceptions were thrown, link the node into the list.
   if (pnPrev) {
      pnPrev->m_pnNext = this;
   } else {
      *ppnFirst = this;
   }
   if (!pnNext) {
      *ppnLast = this;
   }
}

void singly_linked_list_impl::node::unlink(node ** ppnFirst, node ** ppnLast, node * pnPrev) {
   node * pnNext = m_pnNext;
   if (pnPrev) {
      pnPrev->m_pnNext = pnNext;
   } else if (ppnFirst) {
      *ppnFirst = pnNext;
   }
   if (!pnNext && ppnLast) {
      *ppnLast = pnPrev;
   }
}

void * singly_linked_list_impl::node::value_ptr(type_void_adapter const & type) const {
   // Make sure that the argument is the address following the last member.
   return type.align_pointer(&m_pnNext + 1);
}


singly_linked_list_impl::singly_linked_list_impl(singly_linked_list_impl && slli) :
   m_pnFirst(slli.m_pnFirst),
   m_pnLast(slli.m_pnLast),
   m_cNodes(slli.m_cNodes) {
   slli.m_cNodes = 0;
   slli.m_pnFirst = nullptr;
   slli.m_pnLast = nullptr;
}

singly_linked_list_impl & singly_linked_list_impl::operator=(singly_linked_list_impl && slli) {
   /* Assume that the subclass has already made a copy of m_pn{First,Last} to be able to release
   them after calling this operator. */
   m_pnFirst = slli.m_pnFirst;
   slli.m_pnFirst = nullptr;
   m_pnLast = slli.m_pnLast;
   slli.m_pnLast = nullptr;
   m_cNodes = slli.m_cNodes;
   slli.m_cNodes = 0;
   return *this;
}

void singly_linked_list_impl::clear(type_void_adapter const & type) {
   destruct_list(type, m_pnFirst);
   m_pnFirst = nullptr;
   m_pnLast = nullptr;
   m_cNodes = 0;
}

/*static*/ void singly_linked_list_impl::destruct_list(type_void_adapter const & type, node * pn) {
   ABC_TRACE_FUNC(/*type, */pn);

   while (pn) {
      node * pnNext = pn->next();
      type.destruct(pn->value_ptr(type));
      delete pn;
      pn = pnNext;
   }
}

singly_linked_list_impl::node * singly_linked_list_impl::push_back(
   type_void_adapter const & type, void const * p, bool bMove
) {
   ABC_TRACE_FUNC(this/*, type*/, p, bMove);

   node * pn = new(type) node(type, &m_pnFirst, &m_pnLast, m_pnLast, nullptr, p, bMove);
   ++m_cNodes;
   return pn;
}

void singly_linked_list_impl::pop_front(type_void_adapter const & type) {
   ABC_TRACE_FUNC(this/*, type*/);

   node * pn = m_pnFirst;
   pn->unlink(&m_pnFirst, &m_pnLast, nullptr);
   type.destruct(pn->value_ptr(type));
   --m_cNodes;
   delete pn;
}

}}} //namespace abc::collections::detail
