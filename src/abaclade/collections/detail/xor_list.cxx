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
// abc::collections::detail::xor_list

namespace abc {
namespace collections {
namespace detail {

/*static*/ void xor_list::link_back(node * pn, node ** ppnFirst, node ** ppnLast) {
   // TODO: enable use ABC_TRACE_FUNC(pn, ppnFirst, ppnLast) by handling reentrancy.

   node * pnLast = *ppnLast;
   pn->set_prev_next(nullptr, pnLast);
   if (!*ppnFirst) {
      *ppnFirst = pn;
   } else if (pnLast) {
      pnLast->set_prev_next(pn, pnLast->get_next(nullptr));
   }
   *ppnLast = pn;
}

/*static*/ void xor_list::link_front(node * pn, node ** ppnFirst, node ** ppnLast) {
   // TODO: enable use ABC_TRACE_FUNC(pn, ppnFirst, ppnLast) by handling reentrancy.

   node * pnFirst = *ppnFirst;
   pn->set_prev_next(pnFirst, nullptr);
   if (!*ppnLast) {
      *ppnLast = pn;
   } else if (pnFirst) {
      pnFirst->set_prev_next(pnFirst->get_prev(nullptr), pn);
   }
   *ppnFirst = pn;
}

/*static*/ void xor_list::unlink(
   node * pn, node * pnPrev, node * pnNext, node ** ppnFirst, node ** ppnLast
) {
   // TODO: enable use ABC_TRACE_FUNC(pn, pnPrev, pnNext, ppnFirst, ppnLast) by handling reentrancy.

   if (pnPrev) {
      pnPrev->set_prev_next(pnPrev->get_prev(pn), pnNext);
   } else if (*ppnFirst == pn) {
      *ppnFirst = pnNext;
   }
   if (pnNext) {
      pnNext->set_prev_next(pnPrev, pnNext->get_next(pn));
   } else if (*ppnLast == pn) {
      *ppnLast = pnPrev;
   }
}


void xor_list::iterator_base::advance(std::ptrdiff_t i) {
   // TODO: enable use ABC_TRACE_FUNC(this, i) by handling reentrancy.

   if (i > 0) {
      do {
         if (!m_pnCurr) {
            ABC_THROW(iterator_error, ());
         }
         m_pnPrev = m_pnCurr;
         m_pnCurr = m_pnNext;
         m_pnNext = m_pnCurr ? m_pnCurr->get_next(m_pnPrev) : nullptr;
      } while (--i > 0);
   } else if (i < 0) {
      do {
         if (!m_pnPrev) {
            ABC_THROW(iterator_error, ());
         }
         m_pnNext = m_pnCurr;
         m_pnCurr = m_pnPrev;
         m_pnPrev = m_pnCurr ? m_pnCurr->get_prev(m_pnNext) : nullptr;
      } while (++i < 0);
   }
}

void xor_list::iterator_base::throw_if_end() const {
   // TODO: enable use ABC_TRACE_FUNC(this) by handling reentrancy.

   if (!m_pnCurr) {
      ABC_THROW(iterator_error, ());
   }
}

} //namespace detail
} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
