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
#include <abaclade/collections.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections {

void static_list_impl_base::link_back(node * pn) {
   // TODO: enable use ABC_TRACE_FUNC(this, pn) by handling reentrancy.

   node * pnLast = m_pnLast;
   pn->set_siblings(nullptr, pnLast);
   if (!m_pnFirst) {
      m_pnFirst = pn;
   } else if (pnLast) {
      pnLast->set_siblings(pn, pnLast->get_other_sibling(nullptr));
   }
   m_pnLast = pn;
}

void static_list_impl_base::link_front(node * pn) {
   // TODO: enable use ABC_TRACE_FUNC(this, pn) by handling reentrancy.

   node * pnFirst = m_pnFirst;
   pn->set_siblings(pnFirst, nullptr);
   if (!m_pnLast) {
      m_pnLast = pn;
   } else if (pnFirst) {
      pnFirst->set_siblings(pnFirst->get_other_sibling(nullptr), pn);
   }
   m_pnFirst = pn;
}

std::size_t static_list_impl_base::size() const {
   // TODO: enable use ABC_TRACE_FUNC(this) by handling reentrancy.

   std::size_t cNodes = 0;
   for (
      node * pnNext, * pnPrev = nullptr, * pnCurr = m_pnFirst;
      pnCurr;
      pnNext = pnCurr->get_other_sibling(pnPrev), pnPrev = pnCurr, pnCurr = pnNext
   ) {
      ++cNodes;
   }
   return cNodes;
}

void static_list_impl_base::unlink(node * pn) {
   // TODO: enable use ABC_TRACE_FUNC(this, pn) by handling reentrancy.

   /* Find pn in the list, scanning from the back to the front of the list. If nodes are added by
   link_back() in their order of construction and the order of removal is the order of their
   destruction, m_pnLast will be pn. This won’t be the case if shared libraries are not unloaded in
   the same order in which they are loaded. */
   for (
      node * pnPrev, * pnNext = nullptr, * pnCurr = m_pnLast;
      pnCurr;
      pnNext = pnCurr, pnCurr = pnPrev
   ) {
      pnPrev = pnCurr->get_other_sibling(pnNext);
      if (pnCurr == pn) {
         unlink(pn, pnPrev, pnNext);
         break;
      }
   }
}

void static_list_impl_base::unlink(node * pn, node * pnPrev, node * pnNext) {
   // TODO: enable use ABC_TRACE_FUNC(this, pn, pnPrev, pnNext) by handling reentrancy.

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


void static_list_impl_base::iterator::increment() {
   // TODO: enable use ABC_TRACE_FUNC(this) by handling reentrancy.

   /* Detect attempts to increment past the end() of the container, or increment a default-
   constructed iterator. */
   validate();

   node const * pnPrev = m_pnCurr;
   m_pnCurr = m_pnNext;
   m_pnNext = m_pnCurr ? m_pnCurr->get_other_sibling(pnPrev) : nullptr;
}

void static_list_impl_base::iterator::validate() const {
   // TODO: enable use ABC_TRACE_FUNC(this) by handling reentrancy.

   if (!m_pnCurr) {
      ABC_THROW(out_of_range, ());
   }
}

}} //namespace abc::collections
