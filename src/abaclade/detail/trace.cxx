/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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
// abc::detail::scope_trace_tuple

namespace abc {
namespace detail {

/*static*/ void scope_trace_tuple::write_separator(io::text::writer * ptwOut) {
   ptwOut->write(ABC_SL(", "));
}

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::scope_trace

namespace abc {
namespace detail {

//TODO: tls
/*tls*/ scope_trace const * scope_trace::sm_pstHead = nullptr;
/*tls*/ std::unique_ptr<io::text::str_writer> scope_trace::sm_ptswScopeTrace;
/*tls*/ unsigned scope_trace::sm_cScopeTraceRefs = 0;
/*tls*/ unsigned scope_trace::sm_iStackDepth = 0;
/*tls*/ bool scope_trace::sm_bReentering = false;

scope_trace::scope_trace(
   source_location const & srcloc, char_t const * pszFunction, scope_trace_tuple const * ptplVars
) :
   m_pstPrev(sm_pstHead),
   m_ptplVars(ptplVars),
   m_pszFunction(pszFunction),
   m_srcloc(srcloc) {
   sm_pstHead = this;
}

scope_trace::~scope_trace() {
   /* The set-and-reset of sm_bReentering doesn’t need memory barriers because this is all contained
   in a single thread (sm_bReentering is in TLS). */
   if (!sm_bReentering && std::uncaught_exception()) {
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
   ptwOut->print(ABC_SL("#{} {} with args: "), iStackDepth, istr(external_buffer, m_pszFunction));
   // Write the variables tuple.
   m_ptplVars->write(ptwOut);
   ptwOut->print(ABC_SL(" at {}\n"), m_srcloc);
}

/*static*/ void scope_trace::write_list(io::text::writer * ptwOut) {
   unsigned iStackDepth = 0;
   for (scope_trace const * pst = sm_pstHead; pst; pst = pst->m_pstPrev) {
      pst->write(ptwOut, ++iStackDepth);
   }
}

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

