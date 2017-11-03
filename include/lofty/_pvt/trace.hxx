/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_HXX_INTERNAL
   #error "Please #include <lofty.hxx> instead of this file"
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

//! Tracks local variables, to be used during e.g. a stack unwind.
class LOFTY_SYM scope_trace : public noncopyable {
public:
   /*! Constructor.

   @param source_file_addr
      Source location.
   @param local_this
      this in the context of the caller; may be nullptr.
   */
   scope_trace(source_file_address const * source_file_addr, void const * local_this);

   //! Destructor. Adds a scope in the current scope trace if an in-flight exception is detected.
   ~scope_trace();

   /*! Returns a stream to which the stack frame can be output. The stream is thread-local, which is why this
   can’t be just a static member variable.

   @return
      Pointer to the text stream containing the current stack trace.
   */
   static io::text::str_ostream * get_trace_ostream() {
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
   static void write_list(io::text::ostream * dst);

private:
   /*! Writes the scope trace to the specified stream.

   @param dst
      Pointer to the stream to output to.
   @param stack_depth
      Stack index to print next to the trace.
   */
   void write(io::text::ostream * dst, unsigned stack_depth) const;

private:
   //! Pointer to the previous scope_trace single-linked list item that *this replaced as the head.
   scope_trace const * prev_scope_trace;
   //! Pointer to the statically-allocated source location.
   source_file_address const * source_file_addr;
   //! this in the context of the caller; may be nullptr.
   void const * local_this;
   //! Pointer to the head of the scope_trace single-linked list for each thread.
   static coroutine_local_value<scope_trace const *> scope_traces_head;
   /*! true if ~scope_trace() is being run; in that case, another call to it should not try to do anything,
   otherwise we may get stuck in an infinite recursion. */
   static coroutine_local_value<bool> reentering;
   //! Stream that collects the rendered scope trace when an exception is thrown.
   static coroutine_local_ptr<io::text::str_ostream> trace_ostream;
   //! Number of the next stack frame to be added to the rendered trace.
   static coroutine_local_value<unsigned> curr_stack_depth;
   //! Count of references to the current rendered trace. Managed by lofty::exception.
   static coroutine_local_value<unsigned> trace_ostream_refs;
};

}} //namespace lofty::_pvt
