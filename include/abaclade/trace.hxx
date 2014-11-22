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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals

namespace abc {

/*! DOC:8503 Stack tracing

Any function that is not of negligible size and is not an hotspot should invoke, as its first line,
ABC_TRACE_FUNC(arg1, arg2, …) in order to have its name show up in a post-exception stack trace.

ABC_TRACE_FUNC() initializes a local variable of type abc::detail::scope_trace which will store
references to every provided argument.

abc::detail::scope_trace::~scope_trace() detects if the object is being destroyed due to an
exceptional stack unwinding, in which case it will dump its contents into a thread-local stack trace
buffer. The outermost catch block (main-level) will output the generated stack trace, if available,
using abc::exception::write_with_scope_trace().

When an abc::exception is thrown (it becomes “in-flight”), it will request that the stack trace
buffer be cleared and it will count itself a a reference to the new trace; when copied, the number
of references will increase if the source was in-flight, in which case the copy will also consider
itself in-flight; when an exception is destroyed, it will release a reference to the stack trace
buffer if it was holding one. Reference counting is necessary due to platform-specific code that
will copy a thrown exception to non-local storage and throwing that one instead of using the
original one.

This covers the following code flows:

•  No exception thrown: no stack trace is generated.

•  Exception is thrown and unwinds up to main(): each abc::detail::scope_trace adds itself to the
   stack trace, which is then output; the exception is then destroyed, cleaning the trace buffer.

•  Exception is thrown, then caught and blocked: one or more abc::detail::scope_trace might add
   themselves to the stack trace, but the exception is blocked before it reaches main(), so no
   output occurs.

•  Exception is thrown, then caught and rethrown: one or more abc::detail::scope_trace might add
   themselves to the stack trace, up to the point the exception is caught. Since the exception is
   not destroyed, the stack trace buffer will keep the original point at which the exception was
   thrown, resulting in an accurate stack trace in case the exception reaches main().

•  Exception is thrown, then caught and a new one is thrown: similar to the previous case, except
   the original exception is destroyed, so the stack trace buffer will not reveal where the original
   exception was thrown. This is acceptable, since it cannot be determined whether the two
   exceptions were related.

See related diagram [IMG:8503 Stack trace generation] for all code flows covered by this design.
See also [DOC:8191 Throwing exceptions] and abc::exception for the remainder of the implementation.

Currently unsupported:
•  TODO: storing a thrown exception, handling a different exception, and throwing back the first
   one. In the current implementation, there’s nothing telling an exception that it’s no longer
   in-flight (and there can’t be!); additionally, throwing the second exception and re-throwing the
   first one (using throw x; instead of throw; because it’s no longer in-flight) will both reset the
   stack trace buffer.

•  TODO: properly handling exceptions occurring while generating a stack trace. The current behavior
   swallows any nested exceptions, gracefully failing to generate a complete stack trace.
*/

/*! Provides stack frame logging for the function in which it’s used.

@param ...
   Arguments or variables to trace.
*/
#define ABC_TRACE_FUNC(...) \
   _ABC_TRACE_SCOPE_IMPL( \
      ABC_CPP_APPEND_UID(_scope_trace_), ABC_CPP_APPEND_UID(_scope_trace_tuple_), \
      ABC_CPP_APPEND_UID(_scope_trace_source_location_), __VA_ARGS__ \
   )

//! Implementation of ABC_TRACE_FUNC() and similar macros.
#define _ABC_TRACE_SCOPE_IMPL(st, tuple, srcloc, ...) \
   static ::abc::detail::scope_trace_source_location const srcloc = { \
      _ABC_THIS_FUNC, ABC_SL(__FILE__), __LINE__ \
   }; \
   auto tuple(::abc::detail::scope_trace_tuple::make(__VA_ARGS__)); \
   ::abc::detail::scope_trace st(&srcloc, &tuple) \

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
