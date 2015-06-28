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
#include <abaclade/collections/detail/queue_impl.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

void * queue_impl::node::value_ptr(type_void_adapter const & type) const {
   return type.align_pointer(&m_pnNext + 1);
}


queue_impl::queue_impl(queue_impl && q) :
   m_pnFirst(q.m_pnFirst),
   m_pnLast(q.m_pnLast),
   m_cNodes(q.m_cNodes) {
   q.m_cNodes = 0;
   q.m_pnFirst = nullptr;
   q.m_pnLast = nullptr;
}

queue_impl & queue_impl::operator=(queue_impl && q) {
   /* Assume that the subclass has already made a copy of m_pn{First,Last} to be able to release
   them after calling this operator. */
   m_pnFirst = q.m_pnFirst;
   q.m_pnFirst = nullptr;
   m_pnLast = q.m_pnLast;
   q.m_pnLast = nullptr;
   m_cNodes = q.m_cNodes;
   q.m_cNodes = 0;
   return *this;
}

void queue_impl::clear(type_void_adapter const & type) {
   destruct_list(type, m_pnFirst);
   m_pnFirst = m_pnLast = nullptr;
   m_cNodes = 0;
}

/*static*/ void queue_impl::destruct_list(type_void_adapter const & type, node * pnFirst) {
   for (detail::queue_impl::node * pnCurr = pnFirst, * pnNext; pnCurr; pnCurr = pnNext) {
      pnNext = pnCurr->m_pnNext;
      type.destruct(pnCurr->value_ptr(type));
      memory::_raw_free(pnCurr);
   }
}

void queue_impl::push_back(type_void_adapter const & type, void const * pSrc, bool bMove) {
   /* To calculate the node size, add type.size() bytes to the offset of the value in a node at
   address 0. This allows packing the node optimally even if the unpadded node size is e.g. 6
   (sizeof will return 8 for that) and type.size() is 2, giving 8 instead of 10 (which would really
   mean at least 12 bytes, a 50% waste of memory). */
   std::unique_ptr<node, memory::freeing_deleter> pn(static_cast<node *>(memory::_raw_alloc(
      reinterpret_cast<std::size_t>(static_cast<node *>(0)->value_ptr(type)) + type.size()
   )));
   void * pDst = pn->value_ptr(type);
   if (bMove) {
      type.move_construct(pDst, const_cast<void *>(pSrc));
   } else {
      type.copy_construct(pDst, pSrc);
   }

   pn->m_pnNext = nullptr;
   if (!m_pnFirst) {
      m_pnFirst = pn.get();
   } else if (m_pnLast) {
      m_pnLast->m_pnNext = pn.get();
   }
   // Transfer ownership of the node to the list.
   m_pnLast = pn.release();
   ++m_cNodes;
}

void queue_impl::pop_front(type_void_adapter const & type) {
   node * pn = m_pnFirst;
   m_pnFirst = pn->m_pnNext;
   if (!m_pnFirst) {
      m_pnLast = nullptr;
   }
   --m_cNodes;
   type.destruct(pn->value_ptr(type));
   memory::_raw_free(pn);
}

}}} //namespace abc::collections::detail
