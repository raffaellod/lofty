/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015 Raffaello D. Di Napoli

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


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

/*static*/ void scope_trace_tuple::write_separator(io::text::writer * ptwOut) {
   ptwOut->write(ABC_SL(", "));
}

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

coroutine_local_value<scope_trace const *> scope_trace::sm_pstHead /*= nullptr*/;
coroutine_local_value<bool> scope_trace::sm_bReentering /*= false*/;
coroutine_local_ptr<io::text::str_writer> scope_trace::sm_ptswScopeTrace;
coroutine_local_value<unsigned> scope_trace::sm_cScopeTraceRefs /*= 0*/;
coroutine_local_value<unsigned> scope_trace::sm_iStackDepth /*= 0*/;

scope_trace::scope_trace(
   source_file_address_data const * psfad, scope_trace_tuple const * ptplVars
) :
   m_pstPrev(sm_pstHead),
   m_psfad(psfad),
   m_ptplVars(ptplVars) {
   sm_pstHead = this;
}

scope_trace::~scope_trace() {
   /* The set-and-reset of sm_bReentering doesn’t need memory barriers because this is all contained
   in a single thread (sm_bReentering is in TLS). */
   if (!sm_bReentering && _std::uncaught_exception()) {
      sm_bReentering = true;
      try {
         write(get_trace_writer(), ++sm_iStackDepth);
      } catch (...) {
         // Don’t allow a trace to interfere with the program flow.
         // FIXME: EXC-SWALLOW
      }
      sm_bReentering = false;
   }
   // Restore the previous scope trace single-linked list head.
   sm_pstHead = m_pstPrev;
}

void scope_trace::write(io::text::writer * ptwOut, unsigned iStackDepth) const {
   ptwOut->print(
      ABC_SL("#{} {} with args: "), iStackDepth, str(external_buffer, m_psfad->m_pszFunction)
   );
   // Write the variables tuple.
   m_ptplVars->write(ptwOut);
   ptwOut->print(
      ABC_SL(" at {}\n"), text::file_address(m_psfad->m_tfad.m_pszFilePath, m_psfad->m_tfad.m_iLine)
   );
}

/*static*/ void scope_trace::write_list(io::text::writer * ptwOut) {
   unsigned iStackDepth = 0;
   for (scope_trace const * pst = sm_pstHead; pst; pst = pst->m_pstPrev) {
      pst->write(ptwOut, ++iStackDepth);
   }
}

}} //namespace abc::detail
