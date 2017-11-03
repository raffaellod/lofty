/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

/*! @file
Stack tracing infrastructure. */

#ifndef _LOFTY_HXX_INTERNAL
   #error "Please #include <lofty.hxx> instead of this file"
#endif

/*! @page stack-tracing Stack tracing
Automatic generation of stack traces whenever an exception occurs.

A function can opt into this system by invoking, as its first line, LOFTY_TRACE_FUNC() in order to have its
name show up in a post-exception stack trace. Methods should invoke LOFTY_TRACE_METHOD(). These macros result
in the instantiation of a local variable of type lofty::_pvt::scope_trace.

lofty::_pvt::scope_trace::~scope_trace() detects if the object is being destroyed due to an exceptional stack
unwinding, in which case it will dump its contents into a thread-local stack trace buffer. The outermost catch
block (main-level) will output the generated stack trace, if available, using
lofty::exception::write_with_scope_trace().

When a lofty::exception is thrown (it becomes “in-flight”), it will request that the stack trace buffer be
cleared and it will count itself a a reference to the new trace; when copied, the number of references will
increase if the source was in-flight, in which case the copy will also consider itself in-flight; when an
exception is destroyed, it will release a reference to the stack trace buffer if it was holding one. Reference
counting is necessary due to platform-specific code that will copy a thrown exception to non-local storage and
throwing that one instead of using the original one.

This covers the following code flows:

•  No exception thrown: no stack trace is generated.

•  Exception is thrown and escapes past lofty::app::main(): each lofty::_pvt::scope_trace adds itself to the
   stack trace, which is then output; the exception is then destroyed, clearing the trace buffer.

•  Exception is thrown, then caught and blocked: one or more lofty::_pvt::scope_trace might add themselves to
   the stack trace, but the exception is blocked before it escapes lofty::app::main(), so no output occurs.

•  Exception is thrown, then caught and rethrown: one or more lofty::_pvt::scope_trace might add themselves to
   the stack trace, up to the point the exception is caught. Since the exception is not destroyed, the stack
   trace buffer will keep the original point at which the exception was thrown, resulting in an accurate stack
   trace in case the exception reaches main().

•  Exception is thrown, then caught and a new one is thrown: similar to the previous case, except the original
   exception is destroyed, so the stack trace buffer will not reveal where the original exception was thrown.
   This is acceptable, since it cannot be determined whether the two exceptions were related.

See related diagram doc/Stack_trace_generation.svg for all code flow covered by this design.
See also LOFTY_THROW() and lofty::exception for the remainder of the implementation.

Currently unsupported:

•  TODO: storing a thrown exception, handling a different exception, and throwing back the first one. In the
   current implementation, there’s nothing telling an exception that it’s no longer in-flight (and there can’t
   be!); additionally, throwing the second exception and re-throwing the first one (using throw x; instead of
   throw; because it’s no longer in-flight) will both reset the stack trace buffer.

•  TODO: properly handling exceptions occurring while generating a stack trace. The current behavior swallows
   any nested exceptions, gracefully failing to generate a complete stack trace. */


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! Provides stack frame logging for the function in which it’s used.
#define LOFTY_TRACE_FUNC() \
   _LOFTY_TRACE_SCOPE_IMPL(LOFTY_CPP_APPEND_UID(__scope_trace_), nullptr)

//! Provides stack frame logging for the method in which it’s used.
#define LOFTY_TRACE_METHOD() \
   _LOFTY_TRACE_SCOPE_IMPL(LOFTY_CPP_APPEND_UID(__scope_trace_), this)

/*! Implementation of LOFTY_TRACE_FUNC() and LOFTY_TRACE_METHOD().

@param uid
   Unique ID for this scope trace.
@param this
   this pointer, or nullptr.
*/
#define _LOFTY_TRACE_SCOPE_IMPL(uid, this) \
   static ::lofty::_pvt::source_file_address_data const LOFTY_CPP_CAT(uid, _sfad) = { \
      LOFTY_THIS_FUNC, { LOFTY_SL(__FILE__), __LINE__ } \
   }; \
   ::lofty::_pvt::scope_trace uid(::lofty::source_file_address::from_data(&LOFTY_CPP_CAT(uid, _sfad)), this)
