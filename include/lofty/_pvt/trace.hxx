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

// Forward declaration.
#ifdef LOFTY_CXX_VARIADIC_TEMPLATES
template <typename... Ts>
class scope_trace_tuple_impl;
#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES
template <
   typename T0 = _std::_pvt::tuple_void, typename T1 = _std::_pvt::tuple_void,
   typename T2 = _std::_pvt::tuple_void, typename T3 = _std::_pvt::tuple_void,
   typename T4 = _std::_pvt::tuple_void, typename T5 = _std::_pvt::tuple_void,
   typename T6 = _std::_pvt::tuple_void, typename T7 = _std::_pvt::tuple_void,
   typename T8 = _std::_pvt::tuple_void, typename T9 = _std::_pvt::tuple_void
>
class scope_trace_tuple_impl;
#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

//! Stores and prints variables for a scope_trace instance.
class LOFTY_SYM scope_trace_tuple : public noncopyable {
public:
   /*! Returns a scope_trace_tuple containing references to the provided arguments.

   @param ts
      Arguments.
   @return
      New tuple containing references to the specified arguments.
   */
#ifdef LOFTY_CXX_VARIADIC_TEMPLATES
   template <typename... Ts>
   static scope_trace_tuple_impl<Ts ...> make(Ts const &... ts);
#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES
   static scope_trace_tuple_impl<> make();
   template <typename T0>
   static scope_trace_tuple_impl<T0> make(T0 const & t0);
   template <typename T0, typename T1>
   static scope_trace_tuple_impl<T0, T1> make(T0 const & t0, T1 const & t1);
   template <typename T0, typename T1, typename T2>
   static scope_trace_tuple_impl<T0, T1, T2> make(T0 const & t0, T1 const & t1, T2 const & t2);
   template <typename T0, typename T1, typename T2, typename T3>
   static scope_trace_tuple_impl<T0, T1, T2, T3> make(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3
   );
   template <typename T0, typename T1, typename T2, typename T3, typename T4>
   static scope_trace_tuple_impl<T0, T1, T2, T3, T4> make(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4
   );
   template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
   static scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5> make(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5
   );
   template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
   static scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6> make(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5, T6 const & t6
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7
   >
   static scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6, T7> make(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5, T6 const & t6,
      T7 const & t7
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
      typename T8
   >
   static scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6, T7, T8> make(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5, T6 const & t6,
      T7 const & t7, T8 const & t8
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
      typename T8, typename T9
   >
   static scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> make(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5, T6 const & t6,
      T7 const & t7, T8 const & t8, T9 const & t9
   );
#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

   /*! Writes the current value of the tuple’s variables.

   @param dst
      Pointer to the stream to output to.
   */
   virtual void write(io::text::ostream * dst) const = 0;

protected:
   /*! Writes an argument separator.

   @param dst
      Pointer to the stream to output to.
   */
   static void write_separator(io::text::ostream * dst);
};

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

//! Implementation of scope_trace_tuple with actual data storage.
#ifdef LOFTY_CXX_VARIADIC_TEMPLATES
template <typename... Ts>
class scope_trace_tuple_impl : public scope_trace_tuple, public _std::tuple<Ts const & ...> {
private:
   // Handy shortcuts.
   typedef _std::tuple<Ts const & ...> tuple_type;
   static std::size_t const size = _std::tuple_size<tuple_type>::value;
#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES
template <
   typename T0 /*= _std::_pvt::tuple_void*/, typename T1 /*= _std::_pvt::tuple_void*/,
   typename T2 /*= _std::_pvt::tuple_void*/, typename T3 /*= _std::_pvt::tuple_void*/,
   typename T4 /*= _std::_pvt::tuple_void*/, typename T5 /*= _std::_pvt::tuple_void*/,
   typename T6 /*= _std::_pvt::tuple_void*/, typename T7 /*= _std::_pvt::tuple_void*/,
   typename T8 /*= _std::_pvt::tuple_void*/, typename T9 /*= _std::_pvt::tuple_void*/
>
class scope_trace_tuple_impl :
   public scope_trace_tuple,
   public _std::tuple<
      T0 const &, T1 const &, T2 const &, T3 const &, T4 const &, T5 const &, T6 const &, T7 const &,
      T8 const &, T9 const &
   > {
private:
   // Handy shortcuts.
   typedef _std::tuple<
      T0 const &, T1 const &, T2 const &, T3 const &, T4 const &, T5 const &, T6 const &, T7 const &,
      T8 const &, T9 const &
   > tuple_type;
   static std::size_t const size = _std::tuple_size<_std::tuple<
      T0, T1, T2, T3, T4, T5, T6,T7, T8, T9
   >>::value;
#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else
public:
   /*! Constructor.

   @param ts
      Objects to include in the tuple.
   */
#ifdef LOFTY_CXX_VARIADIC_TEMPLATES
   explicit scope_trace_tuple_impl(Ts const &... ts) :
      tuple_type(ts ...) {
   }
#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES
   explicit scope_trace_tuple_impl(
      T0 const & t0 = _std::_pvt::tuple_void(), T1 const & t1 = _std::_pvt::tuple_void(),
      T2 const & t2 = _std::_pvt::tuple_void(), T3 const & t3 = _std::_pvt::tuple_void(),
      T4 const & t4 = _std::_pvt::tuple_void(), T5 const & t5 = _std::_pvt::tuple_void(),
      T6 const & t6 = _std::_pvt::tuple_void(), T7 const & t7 = _std::_pvt::tuple_void(),
      T8 const & t8 = _std::_pvt::tuple_void(), T9 const & t9 = _std::_pvt::tuple_void()
   ) :
      tuple_type(t0, t1, t2, t3, t4, t5, t6, t7, t8, t9) {
   }
#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

   /*! Move constructor.

   @param src
      Source object.
   */
   scope_trace_tuple_impl(scope_trace_tuple_impl && src) :
      tuple_type(_std::move(src)) {
   }

   //! See scope_trace_tuple::write().
   virtual void write(io::text::ostream * dst) const override {
      write_vars<0>(dst);
   }

private:
   // This overload prints a variable followed by a comma and recurses.
   template <std::size_t i>
#if LOFTY_HOST_CXX_MSC
   // “'<' : expression is always false”.
   #pragma warning(suppress: 4296) 
#endif
   void write_vars(typename _std::enable_if<(i + 1 < size), io::text::ostream *>::type dst) const {
      {
         to_text_ostream<typename _std::tuple_element<i, tuple_type>::type> to_text;
         to_text.write(_std::get<i>(*this), dst);
      }
      // Write a separator and recurse to write the rest.
      write_separator(dst);
      write_vars<i + 1>(dst);
   }
   // This overload writes a variable without a comma to follow, and does not recurse.
   template <std::size_t i>
   void write_vars(typename _std::enable_if<(i + 1 == size), io::text::ostream *>::type dst) const {
      to_text_ostream<typename _std::tuple_element<i, tuple_type>::type> to_text;
      to_text.write(_std::get<i>(*this), dst);
   }
   /* This overload does nothing. Only needed because the tuple may be empty, but write() will call
   write_vars<0>() unconditionally. */
   template <std::size_t i>
   void write_vars(typename _std::enable_if<(i == size), io::text::ostream *>::type dst) const {
      LOFTY_UNUSED_ARG(dst);
   }
};


// Now this can be defined.

#ifdef LOFTY_CXX_VARIADIC_TEMPLATES
template <typename... Ts>
inline /*static*/ scope_trace_tuple_impl<Ts ...> scope_trace_tuple::make(Ts const &... ts) {
   return scope_trace_tuple_impl<Ts ...>(ts ...);
}
#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES
inline /*static*/ scope_trace_tuple_impl<> scope_trace_tuple::make() {
   return scope_trace_tuple_impl<>();
}
template <typename T0>
inline /*static*/ scope_trace_tuple_impl<T0> scope_trace_tuple::make(T0 const & t0) {
   return scope_trace_tuple_impl<T0>(t0);
}
template <typename T0, typename T1>
inline /*static*/ scope_trace_tuple_impl<T0, T1> scope_trace_tuple::make(T0 const & t0, T1 const & t1) {
   return scope_trace_tuple_impl<T0, T1>(t0, t1);
}
template <typename T0, typename T1, typename T2>
inline /*static*/ scope_trace_tuple_impl<T0, T1, T2> scope_trace_tuple::make(
   T0 const & t0, T1 const & t1, T2 const & t2
) {
   return scope_trace_tuple_impl<T0, T1, T2>(t0, t1, t2);
}
template <typename T0, typename T1, typename T2, typename T3>
inline /*static*/ scope_trace_tuple_impl<T0, T1, T2, T3> scope_trace_tuple::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3
) {
   return scope_trace_tuple_impl<T0, T1, T2, T3>(t0, t1, t2, t3);
}
template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline /*static*/ scope_trace_tuple_impl<T0, T1, T2, T3, T4> scope_trace_tuple::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4
) {
   return scope_trace_tuple_impl<T0, T1, T2, T3, T4>(t0, t1, t2, t3, t4);
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline /*static*/ scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5> scope_trace_tuple::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5
) {
   return scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5>(t0, t1, t2, t3, t4, t5);
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline /*static*/ scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6> scope_trace_tuple::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5, T6 const & t6
) {
   return scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6>(t0, t1, t2, t3, t4, t5, t6);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7
>
inline /*static*/ scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6, T7> scope_trace_tuple::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5, T6 const & t6,
   T7 const & t7
) {
   return scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6, T7>(t0, t1, t2, t3, t4, t5, t6, t7);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
   typename T8
>
inline /*static*/ scope_trace_tuple_impl<
   T0, T1, T2, T3, T4, T5, T6, T7, T8
> scope_trace_tuple::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5, T6 const & t6,
   T7 const & t7, T8 const & t8
) {
   return scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6, T7, T8>(t0, t1, t2, t3, t4, t5, t6, t7, t8);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
   typename T8, typename T9
>
inline /*static*/ scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> scope_trace_tuple::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5, T6 const & t6,
   T7 const & t7, T8 const & t8, T9 const & t9
) {
   return scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(
      t0, t1, t2, t3, t4, t5, t6, t7, t8, t9
   );
}
#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

//! Tracks local variables, to be used during e.g. a stack unwind.
class LOFTY_SYM scope_trace : public noncopyable {
public:
   /*! Constructor.

   @param source_file_addr
      Source location.
   @param vars
      Variables to capture.
   */
   scope_trace(source_file_address const * source_file_addr, scope_trace_tuple const * vars);

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
   //! Pointer to the caller-allocated tuple containing references to local variables in the scope.
   scope_trace_tuple const * vars;
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
