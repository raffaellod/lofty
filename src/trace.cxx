/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abc/core.hxx>
#include <abc/trace.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_scope_trace_impl


namespace abc {

//TODO: tls
/*tls*/ std::unique_ptr<str_ostream> _scope_trace_impl::sm_psosScopeTrace;
/*tls*/ unsigned _scope_trace_impl::sm_cScopeTraceRefs(0);
/*tls*/ unsigned _scope_trace_impl::sm_iStackDepth(0);
/*tls*/ bool _scope_trace_impl::sm_bReentering(false);


_scope_trace_impl::~_scope_trace_impl() {
   // If the rendering has already started, override sm_bReentering, because the most-derived class
   // must’ve been the one to set sm_bReentering (otherwise m_bScopeRenderingStarted wouldn’t be
   // set), so this trace must be completed.
   if (m_bScopeRenderingStarted || (!sm_bReentering && std::uncaught_exception())) {
      try {
         // First, prevent infinite recursions in case of exceptions occuring during the execution
         // of this method.
         if (!m_bScopeRenderingStarted) {
            sm_bReentering = true;
         }
         // Add this argument to the current trace.
         ostream * pso(get_trace_stream());
         if (m_bScopeRenderingStarted) {
            pso->print(SL(" at {}:{}\n"), m_pszFileName, m_iLine);
         } else {
            // First argument for the current function, so print the function name as well.
            pso->print(
               SL("#{} {} at {}:{}\n"),
               ++sm_iStackDepth, m_pszFunction, m_pszFileName, m_iLine
            );
         }
      } catch (...) {
         // The exception is not rethrown, because we don’t want a trace to interfere with the
         // program flow.
         // FIXME: EXC-SWALLOW
      }
      // If we’re here, it’s this object that set this to true (see first comment above), so reset
      // it now.
      sm_bReentering = false;
   }
}


ostream * _scope_trace_impl::scope_render_start_or_continue() {
   // See similar condition in ~_scope_trace_impl().
   if (m_bScopeRenderingStarted || (!sm_bReentering && std::uncaught_exception())) {
      // See similar condition in ~_scope_trace_impl().
      if (!m_bScopeRenderingStarted) {
         // Note: we don’t reset this variable in this method, but in ~_scope_trace_impl(), since
         // that will always be called last in the destructor sequence - and it will always be
         // called.
         sm_bReentering = true;
      }
      // Add this argument to the current trace.
      ostream * pso(get_trace_stream());
      if (m_bScopeRenderingStarted) {
         pso->write(SL(", "));
      } else {
         // First argument for the current function, so print the function name as well.
         m_bScopeRenderingStarted = true;
         pso->print(SL("#{} {} with args: "), ++sm_iStackDepth, m_pszFunction);
      }
      // Return the stream, so the caller can print its m_t0.
      return pso;
   }
   return NULL;
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

