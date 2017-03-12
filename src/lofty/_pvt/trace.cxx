/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/text.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

/*static*/ void scope_trace_tuple::write_separator(io::text::ostream * dst) {
   dst->write(LOFTY_SL(", "));
}

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

coroutine_local_value<scope_trace const *> scope_trace::scope_traces_head /*= nullptr*/;
coroutine_local_value<bool> scope_trace::reentering /*= false*/;
coroutine_local_ptr<io::text::str_ostream> scope_trace::trace_ostream;
coroutine_local_value<unsigned> scope_trace::trace_ostream_refs /*= 0*/;
coroutine_local_value<unsigned> scope_trace::curr_stack_depth /*= 0*/;

scope_trace::scope_trace(source_file_address const * source_file_addr_, scope_trace_tuple const * vars_) :
   prev_scope_trace(scope_traces_head),
   source_file_addr(source_file_addr_),
   vars(vars_) {
   scope_traces_head = this;
}

scope_trace::~scope_trace() {
   /* The set-and-reset of reentering doesn’t need memory barriers because this is all contained in a single
   thread (reentering is in TLS). */
   if (!reentering && _std::uncaught_exception()) {
      reentering = true;
      try {
         write(get_trace_ostream(), ++curr_stack_depth);
      } catch (...) {
         // Don’t allow a trace to interfere with the program flow.
         // FIXME: EXC-SWALLOW
      }
      reentering = false;
   }
   // Restore the previous scope trace single-linked list head.
   scope_traces_head = prev_scope_trace;
}

void scope_trace::write(io::text::ostream * dst, unsigned stack_depth) const {
   dst->print(
      LOFTY_SL("#{} {} with args: "), stack_depth, str(external_buffer, source_file_addr->function())
   );
   // Write the variables tuple.
   vars->write(dst);
   dst->print(LOFTY_SL(" at {}\n"), source_file_addr->file_address());
}

/*static*/ void scope_trace::write_list(io::text::ostream * dst) {
   unsigned stack_depth = curr_stack_depth;
   for (scope_trace const * st = scope_traces_head; st; st = st->prev_scope_trace) {
      st->write(dst, ++stack_depth);
   }
}

}} //namespace lofty::_pvt
