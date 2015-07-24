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


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

void xor_list_impl::link_back(xor_list_node * pn) {
   // TODO: enable use ABC_TRACE_FUNC(this, pn) by handling reentrancy.

   xor_list_node * pnLast = m_pnLast;
   pn->set_siblings(nullptr, pnLast);
   if (!m_pnFirst) {
      m_pnFirst = pn;
   } else if (pnLast) {
      pnLast->set_siblings(pn, pnLast->get_other_sibling(nullptr));
   }
   m_pnLast = pn;
}

void xor_list_impl::link_front(xor_list_node * pn) {
   // TODO: enable use ABC_TRACE_FUNC(this, pn) by handling reentrancy.

   xor_list_node * pnFirst = m_pnFirst;
   pn->set_siblings(pnFirst, nullptr);
   if (!m_pnLast) {
      m_pnLast = pn;
   } else if (pnFirst) {
      pnFirst->set_siblings(pnFirst->get_other_sibling(nullptr), pn);
   }
   m_pnFirst = pn;
}

void xor_list_impl::unlink(xor_list_node * pn) {
   // Find pn in the list.
   for (
      xor_list_node * pnPrev = nullptr,
                    * pnCurr = m_pnFirst,
                    * pnNext = pnCurr ? pnCurr->get_other_sibling(nullptr) : nullptr;
      pnCurr;
      pnPrev = pnCurr, pnCurr = pnNext, pnNext = pnCurr->get_other_sibling(pnPrev)
   ) {
      if (pnCurr == pn) {
         unlink(pn, pnPrev, pnNext);
         break;
      }
   }
}

void xor_list_impl::unlink(xor_list_node * pn, xor_list_node * pnPrev, xor_list_node * pnNext) {
   // TODO: enable use ABC_TRACE_FUNC(this, pn, pnNext) by handling reentrancy.

   if (pnPrev) {
      pnPrev->set_siblings(pnPrev->get_other_sibling(pn), pnNext);
   } else if (m_pnFirst == pn) {
      m_pnFirst = pnNext;
   }
   if (pnNext) {
      pnNext->set_siblings(pnPrev, pnNext->get_other_sibling(pn));
   } else if (m_pnLast == pn) {
      m_pnLast = pnPrev;
   }
}


void xor_list_impl::iterator_base::increment() {
   // TODO: enable use ABC_TRACE_FUNC(this) by handling reentrancy.

   /* Detect attempts to increment past the end() of the container, or increment a default-
   constructed iterator, or dereference an iterator after the list has invalidated them all. */
   if (!m_pnCurr) {
      ABC_THROW(iterator_error, ());
   }

   xor_list_node const * pnPrev = m_pnCurr;
   m_pnCurr = m_pnNext;
   m_pnNext = m_pnCurr ? m_pnCurr->get_other_sibling(pnPrev) : nullptr;
}

void xor_list_impl::iterator_base::validate() const {
   // TODO: enable use ABC_TRACE_FUNC(this) by handling reentrancy.

   if (!m_pnCurr) {
      ABC_THROW(iterator_error, ());
   }
}

}}} //namespace abc::collections::detail
