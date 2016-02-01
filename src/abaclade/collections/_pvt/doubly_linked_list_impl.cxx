/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2016 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#include <abaclade/collections.hxx>
#include <abaclade/collections/_pvt/doubly_linked_list_impl.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace _pvt {

void * doubly_linked_list_impl::node::operator new(std::size_t cb, type_void_adapter const & type) {
   ABC_UNUSED_ARG(cb);
   /* To calculate the node size, pack the value against the end of the node, potentially using
   space that cb (== sizeof(node)) would reserve as padding. */
   return memory::alloc_bytes(type.align_offset(ABC_UNPADDED_SIZEOF(node, m_pnPrev)) + type.size());
}

doubly_linked_list_impl::node::node(
   type_void_adapter const & type, node ** ppnFirst, node ** ppnLast, node * pnPrev, node * pnNext,
   void const * p, bool bMove
) :
   m_pnNext(pnNext),
   m_pnPrev(pnPrev) {
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
   if (pnNext) {
      pnNext->m_pnPrev = this;
   } else {
      *ppnLast = this;
   }
}

void doubly_linked_list_impl::node::unlink(node ** ppnFirst, node ** ppnLast) {
   node * pnNext = m_pnNext, * pnPrev = m_pnPrev;
   if (pnPrev) {
      pnPrev->m_pnNext = pnNext;
   } else if (ppnFirst) {
      *ppnFirst = pnNext;
   }
   if (pnNext) {
      pnNext->m_pnPrev = pnPrev;
   } else if (ppnLast) {
      *ppnLast = pnPrev;
   }
}

void * doubly_linked_list_impl::node::value_ptr(type_void_adapter const & type) const {
   // Make sure that the argument is the address following the last member.
   return type.align_pointer(&m_pnPrev + 1);
}


void doubly_linked_list_impl::iterator_base::advance(bool bForward) {
   ABC_TRACE_FUNC(this, bForward);

   validate();
   m_pn = bForward ? m_pn->next() : m_pn->prev();
}

void doubly_linked_list_impl::iterator_base::validate() const {
   if (!m_pn) {
      ABC_THROW(out_of_range, ());
   }
}


doubly_linked_list_impl::doubly_linked_list_impl() :
   m_pnFirst(nullptr),
   m_pnLast(nullptr),
   m_cNodes(0) {
}
doubly_linked_list_impl::doubly_linked_list_impl(doubly_linked_list_impl && dlli) :
   m_pnFirst(dlli.m_pnFirst),
   m_pnLast(dlli.m_pnLast),
   m_cNodes(dlli.m_cNodes) {
   dlli.m_cNodes = 0;
   dlli.m_pnFirst = nullptr;
   dlli.m_pnLast = nullptr;
}

doubly_linked_list_impl & doubly_linked_list_impl::operator=(doubly_linked_list_impl && dlli) {
   ABC_TRACE_FUNC(this);

   // Assume that the subclass has already moved *this out.
   m_pnFirst = dlli.m_pnFirst;
   dlli.m_pnFirst = nullptr;
   m_pnLast = dlli.m_pnLast;
   dlli.m_pnLast = nullptr;
   m_cNodes = dlli.m_cNodes;
   dlli.m_cNodes = 0;
   return *this;
}

doubly_linked_list_impl::node * doubly_linked_list_impl::back() const {
   if (!m_pnLast) {
      ABC_THROW(bad_access, ());
   }
   return m_pnLast;
}

void doubly_linked_list_impl::clear(type_void_adapter const & type) {
   ABC_TRACE_FUNC(this/*, type*/);

   destruct_list(type, m_pnFirst);
   m_pnFirst = nullptr;
   m_pnLast = nullptr;
   m_cNodes = 0;
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

doubly_linked_list_impl::node * doubly_linked_list_impl::front() const {
   if (!m_pnFirst) {
      ABC_THROW(bad_access, ());
   }
   return m_pnFirst;
}

/*static*/ doubly_linked_list_impl::node * doubly_linked_list_impl::push_back(
   type_void_adapter const & type, node ** ppnFirst, node ** ppnLast, void const * p, bool bMove
) {
   ABC_TRACE_FUNC(/*type, */ppnFirst, ppnLast, p, bMove);

   return new(type) node(type, ppnFirst, ppnLast, *ppnLast, nullptr, p, bMove);
}

doubly_linked_list_impl::node * doubly_linked_list_impl::push_back(
   type_void_adapter const & type, void const * p, bool bMove
) {
   node * pn = push_back(type, &m_pnFirst, &m_pnLast, p, bMove);
   ++m_cNodes;
   return pn;
}

/*static*/ doubly_linked_list_impl::node * doubly_linked_list_impl::push_front(
   type_void_adapter const & type, node ** ppnFirst, node ** ppnLast, void const * p, bool bMove
) {
   ABC_TRACE_FUNC(/*type, */ppnFirst, ppnLast, p, bMove);

   return new(type) node(type, ppnFirst, ppnLast, nullptr, *ppnFirst, p, bMove);
}

doubly_linked_list_impl::node * doubly_linked_list_impl::push_front(
   type_void_adapter const & type, void const * p, bool bMove
) {
   node * pn = push_front(type, &m_pnFirst, &m_pnLast, p, bMove);
   ++m_cNodes;
   return pn;
}

/*static*/ void doubly_linked_list_impl::remove(
   type_void_adapter const & type, node ** ppnFirst, node ** ppnLast, node * pn
) {
   ABC_TRACE_FUNC(/*type, */ppnFirst, ppnLast, pn);

   pn->unlink(ppnFirst, ppnLast);
   type.destruct(pn->value_ptr(type));
   delete pn;
}

void doubly_linked_list_impl::remove(type_void_adapter const & type, node * pn) {
   remove(type, &m_pnFirst, &m_pnLast, pn);
   --m_cNodes;
}

}}} //namespace abc::collections::_pvt
