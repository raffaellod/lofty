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
// abc::_scope_trace_impl


namespace abc {

//TODO: tls
/*tls*/ std::unique_ptr<io::text::str_writer> _scope_trace_impl::sm_ptswScopeTrace;
/*tls*/ unsigned _scope_trace_impl::sm_cScopeTraceRefs(0);
/*tls*/ unsigned _scope_trace_impl::sm_iStackDepth(0);
/*tls*/ bool _scope_trace_impl::sm_bReentering(false);


void _scope_trace_impl::trace_scope(
   std::function<void (io::text::writer * ptwOut)> const & fnWriteVars
) {
   if (!sm_bReentering && std::uncaught_exception()) {
      sm_bReentering = true;
      try {
         io::text::writer * ptwOut(get_trace_writer());
         ptwOut->print(ABC_SL("#{} {} with args: "), ++sm_iStackDepth, istr(unsafe, m_pszFunction));
         // Allow the caller to write any scope variables.
         fnWriteVars(ptwOut);
         ptwOut->print(ABC_SL(" at {}\n"), m_srcloc);
      } catch (...) {
         // Don’t allow a trace to interfere with the program flow.
         // FIXME: EXC-SWALLOW
      }
      sm_bReentering = false;
   }
}


void _scope_trace_impl::write_separator(io::text::writer * ptwOut) {
   ptwOut->write(ABC_SL(", "));
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

