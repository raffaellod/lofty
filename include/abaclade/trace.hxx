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

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// Declarations

namespace abc {

/** DOC:8503 Stack tracing

Any function that is not of negligible size and is not an hotspot should invoke, as its first line,
ABC_TRACE_FUNC(arg1, arg2, …) in order to have its name show up in a post-exception stack trace.

ABC_TRACE_FUNC() initializes a local variable of type abc::_scope_trace which will store references
to every provided argument.

abc::_scope_trace::~_scope_trace() detects if the object is being destroyed due to an exceptional
stack unwinding, in which case it will dump its contents into a thread-local stack trace buffer. The
outermost catch block (main-level) will output the generated stack trace, if available, using
abc::exception::write_with_scope_trace().

When an abc::exception is thrown (it becomes “in-flight”), it will request that the stack trace
buffer be cleared and it will count itself a a reference to the new trace; when copied, the number
of references will increase if the source was in-flight, in which case the copy will also consider
itself in-flight; when an exception is destroyed, it will release a reference to the stack trace
buffer if it was holding one. Reference counting is necessary due to platform-specific code that
will copy a thrown exception to non-local storage and throwing that one instead of using the
original one.

This covers the following code flows:

•  No exception thrown: no stack trace is generated.

•  Exception is thrown and unwinds up to main(): each abc::_scope_trace adds itself to the stack
   trace, which is then output; the exception is then destroyed, cleaning the trace buffer.

•  Exception is thrown, then caught and blocked: one or more abc::_scope_trace might add themselves
   to the stack trace, but the exception is blocked before it reaches main(), so no output occurs.

•  Exception is thrown, then caught and rethrown: one or more abc::_scope_trace might add themselves
   to the stack trace, up to the point the exception is caught. Since the exception is not
   destroyed, the stack trace buffer will keep the original point at which the exception was thrown,
   resulting in an accurate stack trace in case the exception reaches main().

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

/** Provides stack frame logging for the function in which it’s used.
*/
#define ABC_TRACE_FUNC(...) \
   _ABC_TRACE_SCOPE_IMPL(ABC_CPP_APPEND_UID(_scope_trace_), __VA_ARGS__)

/** Implementation of ABC_TRACE_FUNC() and similar macros.
*/
#define _ABC_TRACE_SCOPE_IMPL(var, ...) \
   auto var(::abc::_scope_trace_impl::make(__VA_ARGS__)); \
   var._set_context(ABC_SOURCE_LOCATION(), _ABC_THIS_FUNC)


/** Tracks local variables, to be used during e.g. a stack unwind. */
#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename ... Ts>
class _scope_trace;

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

template <
   typename T0 = _std::_tuple_void, typename T1 = _std::_tuple_void,
   typename T2 = _std::_tuple_void, typename T3 = _std::_tuple_void,
   typename T4 = _std::_tuple_void, typename T5 = _std::_tuple_void,
   typename T6 = _std::_tuple_void, typename T7 = _std::_tuple_void,
   typename T8 = _std::_tuple_void, typename T9 = _std::_tuple_void
>
class _scope_trace;

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_scope_trace_impl


namespace abc {

/** Largest part of the implementation of _scope_trace_impl. Helps avoid code bloat by being non-
template, using callbacks to display template-dependent variables.
*/
class ABACLADE_SYM _scope_trace_impl {
public:

   /** Returns a writer to which the stack frame can be output. The writer is thread-local, which is
   why this can’t be just a static member variable.
   */
   static io::text::str_writer * get_trace_writer() {
      if (!sm_ptswScopeTrace) {
         sm_ptswScopeTrace.reset(new io::text::str_writer());
      }
      return sm_ptswScopeTrace.get();
   }


   /** Similar to std::make_tuple(), allows to use the keyword auto to specify (omit, really) the
   type of the variable, which would otherwise require knowing the types of the template arguments.
   */
#ifdef ABC_CXX_VARIADIC_TEMPLATES

   template <typename ... Ts>
   static _scope_trace<Ts ...> make(Ts const & ... ts);

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

   static _scope_trace<> make();
   template <typename T0>
   static _scope_trace<T0> make(T0 const & t0);
   template <typename T0, typename T1>
   static _scope_trace<T0, T1> make(T0 const & t0, T1 const & t1);
   template <typename T0, typename T1, typename T2>
   static _scope_trace<T0, T1, T2> make(T0 const & t0, T1 const & t1, T2 const & t2);
   template <typename T0, typename T1, typename T2, typename T3>
   static _scope_trace<T0, T1, T2, T3> make(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3
   );
   template <typename T0, typename T1, typename T2, typename T3, typename T4>
   static _scope_trace<T0, T1, T2, T3, T4> make(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4
   );
   template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
   static _scope_trace<T0, T1, T2, T3, T4, T5> make(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
   >
   static _scope_trace<T0, T1, T2, T3, T4, T5, T6> make(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
      typename T7
   >
   static _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7> make(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
      typename T7, typename T8
   >
   static _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7, T8> make(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7, T8 const & t8
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
      typename T7, typename T8, typename T9
   >
   static _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> make(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
   );

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else


   /** Increments the reference count of the scope trace being generated.
   */
   static void trace_writer_addref() {
      ++sm_cScopeTraceRefs;
   }


   /** Decrements the reference count of the scope trace being generated. If the reference count
   reaches zero, trace_writer_clear() will be invoked.
   */
   static void trace_writer_release() {
      if (sm_cScopeTraceRefs == 1) {
         trace_writer_clear();
      } else if (sm_cScopeTraceRefs > 1) {
         --sm_cScopeTraceRefs;
      }
   }


   /** Erases any collected stack frames.
   */
   static void trace_writer_clear() {
      sm_ptswScopeTrace.reset();
      sm_iStackDepth = 0;
      sm_cScopeTraceRefs = 0;
   }


   /** Assigns a context to the scope trace. These cannot be merged with the constructor because we
   want the constructor to be invoked with all the arguments as a single parenthesis-delimited
   tuple. See the implementation of ABC_TRACE_FUNC() if this isn’t clear enough.

   srcloc
      Source location.
   pszFunction
      Function name.
   */
   void _set_context(source_location const & srcloc, char_t const * pszFunction) {
      m_srcloc = srcloc;
      m_pszFunction = pszFunction;
   }


protected:

   /** Adds a scope in the current scope trace if an in-flight exception is detected.

   fnWriteVars
      Callback that is invoked to write the current value of local variables in the scope.
      ptwOut
         Pointer to the writer to output to.
   */
   void trace_scope(std::function<void (io::text::writer * ptwOut)> const & fnWriteVars);


   /** Writes an argument separator.

   ptwOut
      Pointer to the writer to output to.
   */
   void write_separator(io::text::writer * ptwOut);


private:

   /** Function name. */
   char_t const * m_pszFunction;
   /** Source location. */
   source_location m_srcloc;

   /** Writer that collects the rendered scope trace when an exception is thrown. */
   static /*tls*/ std::unique_ptr<io::text::str_writer> sm_ptswScopeTrace;
   /** Number of the next stack frame to be added to the rendered trace. */
   static /*tls*/ unsigned sm_iStackDepth;
   /** Count of references to the current rendered trace. Managed by abc::exception. */
   static /*tls*/ unsigned sm_cScopeTraceRefs;
   /** true if trace_scope() (the only method that actually may do anything at all) is being run.
   If this is true, another call to it should not try to do anything, otherwise we may get stuck in
   an infinite recursion. */
   static /*tls*/ bool sm_bReentering;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_scope_trace


namespace abc {

#ifdef ABC_CXX_VARIADIC_TEMPLATES

/** Helper to write a single variable out of a _scope_trace, recursing to print any remaining ones.
*/
template <class TScopeTrace, typename ... Ts>
class _scope_trace_vars_impl;

// Base case for the template recursion.
template <class TScopeTrace>
class _scope_trace_vars_impl<TScopeTrace> :
   public _scope_trace_impl {
protected:

   /** Writes the current variable to the specified text writer, then recurses to write the rest.

   ptwOut
      Pointer to the writer to output to.
   */
   void write_vars(io::text::writer * ptwOut) {
      ABC_UNUSED_ARG(ptwOut);
   }
};

// Template recursion step.
template <class TScopeTrace, typename T0, typename ... Ts>
class _scope_trace_vars_impl<TScopeTrace, T0, Ts ...> :
   public _scope_trace_vars_impl<TScopeTrace, Ts ...> {
protected:

   /** See _scope_trace_vars_impl<TScopeTrace>::write_vars().
   */
   void write_vars(io::text::writer * ptwOut);
};


template <typename ... Ts>
class _scope_trace :
   public _scope_trace_vars_impl<_scope_trace<Ts ...>, Ts ...>,
   public std::tuple<Ts const & ...>,
   public noncopyable {

   typedef _scope_trace_vars_impl<_scope_trace<Ts ...>, Ts ...> scope_trace_vars_impl;


public:

   /** Tuple type used to store the trace variables. */
   typedef std::tuple<Ts const & ...> _tuple_base;
   /** Count of trace variables. */
   static size_t const smc_cTs = sizeof ...(Ts);


public:

   /** Constructor.
   */
   _scope_trace(Ts const & ... ts) :
      _tuple_base(ts ...) {
   }
   _scope_trace(_scope_trace && st) :
      scope_trace_vars_impl(static_cast<scope_trace_vars_impl &&>(st)),
      _tuple_base(static_cast<_tuple_base &&>(st)) {
   }


   /** Destructor.
   */
   ~_scope_trace() {
      _scope_trace_impl::trace_scope([this] (io::text::writer * ptwOut) -> void {
         this->write_vars(ptwOut);
      });
   }
};


// Now these can be implemented.

template <class TScopeTrace, typename T0, typename ... Ts>
inline void _scope_trace_vars_impl<TScopeTrace, T0, Ts ...>::write_vars(
   io::text::writer * ptwOut
) {
   // Write the current (T0) tuple element. *this is part of a _scope_trace, which in turn contains
   // a tuple, so a single static_cast gives access to the tuple.
   ptwOut->write(std::get<
      TScopeTrace::smc_cTs - (1 /*T0*/ + sizeof ...(Ts))
   >(*static_cast<TScopeTrace *>(this)));
   // If there are any remaining variables, write a separator and recurse to write the rest.
   if (sizeof ...(Ts)) {
      this->write_separator(ptwOut);
      _scope_trace_vars_impl<TScopeTrace, Ts ...>::write_vars(ptwOut);
   }
}


template <typename ... Ts>
inline _scope_trace<Ts ...> _scope_trace_impl::make(Ts const & ... ts) {
   return _scope_trace<Ts ...>(ts ...);
}

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

/** Helper to write a single variable out of a _scope_trace, recursing to print any remaining ones.
*/
// Template recursion step.
template <
   class TScopeTrace, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5,
   typename T6, typename T7, typename T8, typename T9
>
class _scope_trace_vars_impl :
   public _scope_trace_vars_impl<
      TScopeTrace, T1, T2, T3, T4, T5, T6, T7, T8, T9, _std::_tuple_void
   > {
protected:

   /** See _scope_trace_vars_impl<TTuple>::write_vars().
   */
   void write_vars(io::text::writer * ptwOut);
};

// Base case for the template recursion.
template <class TScopeTrace>
class _scope_trace_vars_impl<
   TScopeTrace, _std::_tuple_void, _std::_tuple_void, _std::_tuple_void, _std::_tuple_void,
   _std::_tuple_void, _std::_tuple_void, _std::_tuple_void, _std::_tuple_void, _std::_tuple_void,
   _std::_tuple_void
> :
   public _scope_trace_impl {
protected:

   /** Writes the current element to the specified text writer, then recurses to write them.

   ptwOut
      Pointer to the writer to output to.
   */
   void write_vars(io::text::writer * ptwOut) {
      ABC_UNUSED_ARG(ptwOut);
   }
};


template <
   typename T0 /*= _std::_tuple_void*/, typename T1 /*= _std::_tuple_void*/,
   typename T2 /*= _std::_tuple_void*/, typename T3 /*= _std::_tuple_void*/,
   typename T4 /*= _std::_tuple_void*/, typename T5 /*= _std::_tuple_void*/,
   typename T6 /*= _std::_tuple_void*/, typename T7 /*= _std::_tuple_void*/,
   typename T8 /*= _std::_tuple_void*/, typename T9 /*= _std::_tuple_void*/
>
class _scope_trace :
   public _scope_trace_vars_impl<
      _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9
   >,
   public _std::tuple<
      T0 const &, T1 const &, T2 const &, T3 const &, T4 const &, T5 const &, T6 const &,
      T7 const &, T8 const &, T9 const &
   >,
   public noncopyable {

   typedef _scope_trace_vars_impl<
      _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9
   > scope_trace_vars_impl;


public:

   /** Tuple type used to store the trace variables. */
   typedef _std::tuple<
      T0 const &, T1 const &, T2 const &, T3 const &, T4 const &, T5 const &, T6 const &,
      T7 const &, T8 const &, T9 const &
   > _tuple_base;
   /** Count of trace variables. */
   static size_t const smc_cTs = _std::tuple_size<_std::tuple<
      T0, T1, T2, T3, T4, T5, T6, T7, T8, T9
   >>::value;


public:

   /** Constructor.

   t0...t9
      Variables to trace.
   */
   _scope_trace(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
   ) :
      _tuple_base(t0, t1, t2, t3, t4, t5, t6, t7, t8, t9) {
   }
   _scope_trace(_scope_trace && st) :
      scope_trace_vars_impl(static_cast<scope_trace_vars_impl &&>(st)),
      _tuple_base(static_cast<_tuple_base &&>(st)) {
   }


   /** Destructor.
   */
   ~_scope_trace() {
      _scope_trace_impl::trace_scope([this] (io::text::writer * ptwOut) -> void {
         this->write_vars(ptwOut);
      });
   }
};


// Now these can be implemented.

template <
   class TScopeTrace, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5,
   typename T6, typename T7, typename T8, typename T9
>
inline void _scope_trace_vars_impl<
   TScopeTrace, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9
>::write_vars(io::text::writer * ptwOut) {
   static size_t const sc_cTs(
      _std::tuple_size<_std::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9>>::value
   );
   // Write the current (T0) tuple element. *this is part of a _scope_trace, which in turn contains
   // a tuple, so a single static_cast gives access to the tuple.
   ptwOut->write(_std::get<
      TScopeTrace::smc_cTs - (1 /*T0*/ + sc_cTs)
   >(*static_cast<TScopeTrace *>(this)));
   // If there are any remaining elements, write a separator and recurse to write them.
   if (sc_cTs) {
      this->write_separator(ptwOut);
      _scope_trace_vars_impl<
         TScopeTrace, T1, T2, T3, T4, T5, T6, T7, T8, T9, _std::_tuple_void
      >::write_vars(ptwOut);
   }
}


inline /*static*/ _scope_trace<> _scope_trace_impl::make() {
   return _scope_trace<>(
      _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void(),
      _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void(),
      _std::_tuple_void(), _std::_tuple_void()
   );
}
template <typename T0>
inline /*static*/ _scope_trace<T0> _scope_trace_impl::make(T0 const & t0) {
   return _scope_trace<T0>(
      t0, _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void(),
      _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void(),
      _std::_tuple_void()
   );
}
template <typename T0, typename T1>
inline /*static*/ _scope_trace<T0, T1> _scope_trace_impl::make(T0 const & t0, T1 const & t1) {
   return _scope_trace<T0, T1>(
      t0, t1, _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void(),
      _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void()
   );
}
template <typename T0, typename T1, typename T2>
inline /*static*/ _scope_trace<T0, T1, T2> _scope_trace_impl::make(
   T0 const & t0, T1 const & t1, T2 const & t2
) {
   return _scope_trace<T0, T1, T2>(
      t0, t1, t2, _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void(),
      _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void()
   );
}
template <typename T0, typename T1, typename T2, typename T3>
inline /*static*/ _scope_trace<T0, T1, T2, T3> _scope_trace_impl::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3
) {
   return _scope_trace<T0, T1, T2, T3>(
      t0, t1, t2, t3, _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void(),
      _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void()
   );
}
template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline /*static*/ _scope_trace<T0, T1, T2, T3, T4> _scope_trace_impl::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4
) {
   return _scope_trace<T0, T1, T2, T3, T4>(
      t0, t1, t2, t3, t4, _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void(),
      _std::_tuple_void(), _std::_tuple_void()
   );
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline /*static*/ _scope_trace<T0, T1, T2, T3, T4, T5> _scope_trace_impl::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5
) {
   return _scope_trace<T0, T1, T2, T3, T4, T5>(
      t0, t1, t2, t3, t4, t5, _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void(),
      _std::_tuple_void()
   );
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
>
inline /*static*/ _scope_trace<T0, T1, T2, T3, T4, T5, T6> _scope_trace_impl::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6
) {
   return _scope_trace<T0, T1, T2, T3, T4, T5, T6>(
      t0, t1, t2, t3, t4, t5, t6, _std::_tuple_void(), _std::_tuple_void(), _std::_tuple_void()
   );
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7
>
inline /*static*/ _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7> _scope_trace_impl::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6, T7 const & t7
) {
   return _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7>(
      t0, t1, t2, t3, t4, t5, t6, t7, _std::_tuple_void(), _std::_tuple_void()
   );
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8
>
inline /*static*/ _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7, T8> _scope_trace_impl::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6, T7 const & t7, T8 const & t8
) {
   return _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7, T8>(
      t0, t1, t2, t3, t4, t5, t6, t7, t8, _std::_tuple_void()
   );
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
inline /*static*/ _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> _scope_trace_impl::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
) {
   return _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(
      t0, t1, t2, t3, t4, t5, t6, t7, t8, t9
   );
}

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

