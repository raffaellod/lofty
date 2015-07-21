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

/*static*/ void xor_list::link_back(data_members * plxdm, node * pn) {
   // TODO: enable use ABC_TRACE_FUNC(plxdm, pn) by handling reentrancy.

   node * pnLast = plxdm->m_pnLast;
   pn->set_siblings(nullptr, pnLast);
   if (!plxdm->m_pnFirst) {
      plxdm->m_pnFirst = pn;
   } else if (pnLast) {
      pnLast->set_siblings(pn, pnLast->get_other_sibling(nullptr));
   }
   plxdm->m_pnLast = pn;
}

/*static*/ void xor_list::link_front(data_members * plxdm, node * pn) {
   // TODO: enable use ABC_TRACE_FUNC(plxdm, pn) by handling reentrancy.

   node * pnFirst = plxdm->m_pnFirst;
   pn->set_siblings(pnFirst, nullptr);
   if (!plxdm->m_pnLast) {
      plxdm->m_pnLast = pn;
   } else if (pnFirst) {
      pnFirst->set_siblings(pnFirst->get_other_sibling(nullptr), pn);
   }
   plxdm->m_pnFirst = pn;
}

/*static*/ void xor_list::unlink(data_members * plxdm, node * pn, node * pnNext) {
   // TODO: enable use ABC_TRACE_FUNC(plxdm, pn, pnPrev, pnNext) by handling reentrancy.

   node * pnPrev = pn->get_other_sibling(pnNext);
   if (pnPrev) {
      pnPrev->set_siblings(pnPrev->get_other_sibling(pn), pnNext);
   } else if (plxdm->m_pnFirst == pn) {
      plxdm->m_pnFirst = pnNext;
   }
   if (pnNext) {
      pnNext->set_siblings(pnPrev, pnNext->get_other_sibling(pn));
   } else if (plxdm->m_pnLast == pn) {
      plxdm->m_pnLast = pnPrev;
   }
}


xor_list::iterator_base::iterator_base() :
   m_pnCurr(nullptr),
   m_pnNext(nullptr),
   m_pxldm(nullptr) {
}
xor_list::iterator_base::iterator_base(data_members const * pxldm, node * pnCurr, node * pnNext) :
   m_pnCurr(pnCurr),
   m_pnNext(pnNext),
   m_pxldm(pxldm) {
}

void xor_list::iterator_base::increment() {
   // TODO: enable use ABC_TRACE_FUNC(this) by handling reentrancy.

   /* Detect attempts to increment past the end() of the container, or increment a default-
   constructed iterator, or dereference an iterator after the list has invalidated them all. */
   if (!m_pnCurr) {
      ABC_THROW(iterator_error, ());
   }

   node const * pnPrev = m_pnCurr;
   m_pnCurr = m_pnNext;
   m_pnNext = m_pnCurr ? m_pnCurr->get_other_sibling(pnPrev) : nullptr;
}

void xor_list::iterator_base::validate() const {
   // TODO: enable use ABC_TRACE_FUNC(this) by handling reentrancy.

   if (!m_pnCurr) {
      ABC_THROW(iterator_error, ());
   }
}

}}} //namespace abc::collections::detail
