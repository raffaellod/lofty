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

void xor_list::iterator_base::decrement() {
   m_pnNext = m_pnCurr;
   m_pnCurr = m_pnPrev;
   m_pnPrev = m_pnCurr ? m_pnCurr->get_prev(m_pnNext) : nullptr;
}

void xor_list::iterator_base::increment() {
   m_pnPrev = m_pnCurr;
   m_pnCurr = m_pnNext;
   m_pnNext = m_pnCurr ? m_pnCurr->get_next(m_pnPrev) : nullptr;
}

} //namespace detail
} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
