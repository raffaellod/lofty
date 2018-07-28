/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

/*! @file
Logging and stack tracing infrastructure. */

#ifndef _LOFTY_LOGGING_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_LOGGING_HXX
#endif

#ifndef _LOFTY_LOGGING_HXX_NOPUB
#define _LOFTY_LOGGING_HXX_NOPUB

#include <lofty/coroutine_local.hxx>
#include <lofty/enum.hxx>
#include <lofty/exception.hxx>
#include <lofty/io/text/str.hxx>
#include <lofty/noncopyable.hxx>

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

namespace lofty { namespace logging { namespace _pvt {

//! Tracks local variables, to be used during e.g. a stack unwind.
class LOFTY_SYM scope_trace : public lofty::_LOFTY_PUBNS noncopyable {
public:
   /*! Constructor.

   @param source_file_addr
      Source location.
   @param local_this
      this in the context of the caller; may be nullptr.
   */
   scope_trace(lofty::_LOFTY_PUBNS source_file_address const * source_file_addr, void const * local_this);

   //! Destructor. Adds a scope in the current scope trace if an in-flight exception is detected.
   ~scope_trace();

   /*! Returns a stream to which the stack frame can be output. The stream is thread-local, which is why this
   can’t be just a static member variable.

   @return
      Pointer to the text stream containing the current stack trace.
   */
   static io::text::_LOFTY_PUBNS str_ostream * get_trace_ostream() {
      if (!trace_ostream) {
         trace_ostream.reset_new();
      }
      return trace_ostream.get();
   }

   //! Increments the reference count of the scope trace being generated.
   static void trace_ostream_addref() {
      ++trace_ostream_refs;
   }

   /*! Decrements the reference count of the scope trace being generated. If the reference count reaches zero,
   trace_ostream_clear() will be invoked. */
   static void trace_ostream_release() {
      if (trace_ostream_refs == 1) {
         trace_ostream_clear();
      } else if (trace_ostream_refs > 1) {
         --trace_ostream_refs;
      }
   }

   //! Erases any collected stack frames.
   static void trace_ostream_clear() {
      trace_ostream.reset();
      curr_stack_depth = 0;
      trace_ostream_refs = 0;
   }

   /*! Walks the single-linked list of scope_trace instances for the current thread, writing each one to the
   specified stream.

   @param dst
      Pointer to the stream to output to.
   */
   static void write_list(io::text::_LOFTY_PUBNS ostream * dst);

private:
   /*! Writes the scope trace to the specified stream.

   @param dst
      Pointer to the stream to output to.
   @param stack_depth
      Stack index to print next to the trace.
   */
   void write(io::text::_LOFTY_PUBNS ostream * dst, unsigned stack_depth) const;

private:
   //! Pointer to the previous scope_trace single-linked list item that *this replaced as the head.
   scope_trace const * prev_scope_trace;
   //! Pointer to the statically-allocated source location.
   lofty::_LOFTY_PUBNS source_file_address const * source_file_addr;
   //! this in the context of the caller; may be nullptr.
   void const * local_this;
   //! Pointer to the head of the scope_trace single-linked list for each thread.
   static _LOFTY_PUBNS coroutine_local_value<scope_trace const *> scope_traces_head;
   //! Stream that collects the rendered scope trace when an exception is thrown.
   static _LOFTY_PUBNS coroutine_local_ptr<io::text::_LOFTY_PUBNS str_ostream> trace_ostream;
   //! Number of the next stack frame to be added to the rendered trace.
   static _LOFTY_PUBNS coroutine_local_value<unsigned> curr_stack_depth;
   //! Count of references to the current rendered trace. Managed by lofty::exception.
   static _LOFTY_PUBNS coroutine_local_value<unsigned> trace_ostream_refs;
};

}}} //namespace lofty::logging::_pvt

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
   ::lofty::logging::_pvt::scope_trace uid( \
      ::lofty::_pub::source_file_address::from_data(&LOFTY_CPP_CAT(uid, _sfad)), this \
   )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace logging {
_LOFTY_PUBNS_BEGIN

//! Logging levels. Enumeration members ara available both in full and as short forms.
LOFTY_ENUM(level,
   //! Interruption in the code flow, such as an exception.
   (error,   0),
   //! Short form of error.
   (err,     0),
   //! Unexpected situation that can be recovered from.
   (warning, 1),
   //! Short form of warning.
   (warn,    1),
   //! Informational message, useful to keep track of the state of the application.
   (info,    2),
   //! Detailed information that may be used to track down application errors.
   (debug,   3),
   //! Short form of debug.
   (dbg,     3)
);

LOFTY_SYM io::text::_LOFTY_PUBNS ostream * get_ostream_if(level level_);

_LOFTY_PUBNS_END
}} //namespace lofty::logging

/*! Outputs a message to the application’s log.

@param level
   The message will only be output if the current application-wide logging level is at least the specified
   value.
@param format
   Format string to parse for replacements.
@param ...
   Replacement values.
*/
#define LOFTY_LOG(level_, ...) \
   do { \
      if (auto __log = ::lofty::logging::_pub::get_ostream_if( \
         ::lofty::logging::_pub::level::enum_type::level_ \
      )) { \
         __log->print(__VA_ARGS__); \
      } \
   } while (false)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_LOGGING_HXX_NOPUB

#ifdef _LOFTY_LOGGING_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace logging {

   using _pub::get_ostream_if;
   using _pub::level;

   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_LOGGING_HXX
