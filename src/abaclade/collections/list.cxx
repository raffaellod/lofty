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
// abc::collections::detail::list_impl

namespace abc {
namespace collections {
namespace detail {

list_impl & list_impl::operator=(list_impl && l) {
   ABC_TRACE_FUNC(this);

   /* Assume that the subclass has already made a copy of m_pn{First,Last} to be able to release
   them after calling this operator. */
   m_pnFirst = l.m_pnFirst;
   l.m_pnFirst = nullptr;
   m_pnLast = l.m_pnLast;
   l.m_pnLast = nullptr;
   m_cNodes = l.m_cNodes;
   l.m_cNodes = 0;
   return *this;
}

xor_list::node * list_impl::back() const {
   ABC_TRACE_FUNC(this);

   if (!m_pnLast) {
      ABC_THROW(null_pointer_error, ());
   }
   return m_pnLast;
}

xor_list::node * list_impl::front() const {
   ABC_TRACE_FUNC(this);

   if (!m_pnFirst) {
      ABC_THROW(null_pointer_error, ());
   }
   return m_pnFirst;
}

void list_impl::link_back(xor_list::node * pn) {
   ABC_TRACE_FUNC(this, pn);

   pn->set_prev_next(nullptr, m_pnLast);
   if (!m_pnFirst) {
      m_pnFirst = pn;
   } else if (xor_list::node * pnLast = m_pnLast) {
      pnLast->set_prev_next(pn, pnLast->get_next(nullptr));
   }
   m_pnLast = pn;
   ++m_cNodes;
}

void list_impl::link_front(xor_list::node * pn) {
   ABC_TRACE_FUNC(this, pn);

   pn->set_prev_next(m_pnFirst, nullptr);
   if (!m_pnLast) {
      m_pnLast = pn;
   } else if (xor_list::node * pnFirst = m_pnFirst) {
      pnFirst->set_prev_next(pnFirst->get_prev(nullptr), pn);
   }
   m_pnFirst = pn;
   ++m_cNodes;
}

xor_list::node * list_impl::unlink(
   xor_list::node * pn, xor_list::node * pnPrev, xor_list::node * pnNext
) {
   ABC_TRACE_FUNC(this, pn, pnPrev, pnNext);

   if (pnPrev) {
      pnPrev->set_prev_next(pnPrev->get_prev(pn), pnNext);
   } else if (m_pnFirst == pn) {
      m_pnFirst = pnNext;
   }
   if (pnNext) {
      pnNext->set_prev_next(pnPrev, pnNext->get_next(pn));
   } else if (m_pnLast == pn) {
      m_pnLast = pnPrev;
   }
   --m_cNodes;
   // Now the subclass must delete pn.
   return pn;
}

xor_list::node * list_impl::unlink_back() {
   ABC_TRACE_FUNC(this);

   return unlink(m_pnLast, m_pnLast->get_prev(nullptr), nullptr);
}

xor_list::node * list_impl::unlink_front() {
   ABC_TRACE_FUNC(this);

   return unlink(m_pnFirst, nullptr, m_pnFirst->get_next(nullptr));
}

} //namespace detail
} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
