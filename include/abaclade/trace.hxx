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
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
class _scope_trace;

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_scope_trace_impl


namespace abc {

class ABACLADE_SYM _scope_trace_impl {
public:

   /** Constructor.
   */
   _scope_trace_impl() :
      m_bScopeRenderingStarted(false) {
   }


   /** Destructor. It also completes the trace (started by a _scope_trace specialization) for this
   scope.
   */
   ~_scope_trace_impl();


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

   template <
      typename T0 = _std::_tuple_void, typename T1 = _std::_tuple_void,
      typename T2 = _std::_tuple_void, typename T3 = _std::_tuple_void,
      typename T4 = _std::_tuple_void, typename T5 = _std::_tuple_void,
      typename T6 = _std::_tuple_void, typename T7 = _std::_tuple_void,
      typename T8 = _std::_tuple_void, typename T9 = _std::_tuple_void
   >
   static _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> make(
      T0 const & t0 = _std::_tuple_void(), T1 const & t1 = _std::_tuple_void(),
      T2 const & t2 = _std::_tuple_void(), T3 const & t3 = _std::_tuple_void(),
      T4 const & t4 = _std::_tuple_void(), T5 const & t5 = _std::_tuple_void(),
      T6 const & t6 = _std::_tuple_void(), T7 const & t7 = _std::_tuple_void(),
      T8 const & t8 = _std::_tuple_void(), T9 const & t9 = _std::_tuple_void()
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

   /** Begins a new frame in the current scope trace, or prepare for the next argument in the
   current frame.

   return
      Pointer to the scope trace writer. The caller can use this to add the current value of a local
      variable in the current frame.
   */
   io::text::writer * scope_render_start_or_continue();


private:

   /** Function name. */
   char_t const * m_pszFunction;
   /** Source location. */
   source_location m_srcloc;
   /** If true, rendering of this scope trace has started (the function/scope name has been
   rendered). */
   bool m_bScopeRenderingStarted;

   /** Writer that collects the rendered scope trace when an exception is thrown. */
   static /*tls*/ std::unique_ptr<io::text::str_writer> sm_ptswScopeTrace;
   /** Number of the next stack frame to be added to the rendered trace. */
   static /*tls*/ unsigned sm_iStackDepth;
   /** Count of references to the current rendered trace. Managed by abc::exception. */
   static /*tls*/ unsigned sm_cScopeTraceRefs;
   /** true if the destructor (the only method that actually may do anything at all) is being run.
   If this is true, another call to the destructor should not try to do anything, otherwise we’ll
   get stuck in an infinite recursion. */
   static /*tls*/ bool sm_bReentering;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_scope_trace


namespace abc {

#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename ... Ts>
class _scope_trace :
   public _scope_trace_impl,
   protected std::tuple<Ts const & ...> {

   /** Tuple type used to store the trace variables. */
   typedef std::tuple<Ts const & ...> _tuple_base;
   /** Count of trace variables. */
   static size_t const smc_cTs = std::tuple_size<_tuple_base>::value;

public:

   /** Constructor.
   */
   _scope_trace(Ts const & ... ts) :
      _tuple_base(ts ...) {
   }


   /** Destructor.
   */
   ~_scope_trace() {
      try {
         // If this returns a valid pointer, a trace is being generated.
         if (io::text::writer * ptw = _scope_trace_impl::scope_render_start_or_continue()) {
            print<>(ptw);
         }
      } catch (...) {
         // Don’t allow a trace to interfere with the program flow.
         // FIXME: EXC-SWALLOW
      }
   }


protected:

   /** Prints the scope trace to the specified text writer.

   ptw
      Pointer to the writer to output to.
   */
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 0, io::text::writer *>::type ptw) {
      ABC_UNUSED_ARG(ptw);
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 1, io::text::writer *>::type ptw) {
      ptw->print(SL("{}"), std::get<0>(*this));
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 2, io::text::writer *>::type ptw) {
      ptw->print(SL("{}, {}"), std::get<0>(*this), std::get<1>(*this));
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 3, io::text::writer *>::type ptw) {
      ptw->print(SL("{}, {}, {}"), std::get<0>(*this), std::get<1>(*this), std::get<2>(*this));
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 4, io::text::writer *>::type ptw) {
      ptw->print(
         SL("{}, {}, {}, {}"),
         std::get<0>(*this), std::get<1>(*this), std::get<2>(*this), std::get<3>(*this)
      );
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 5, io::text::writer *>::type ptw) {
      ptw->print(
         SL("{}, {}, {}, {}, {}"),
         std::get<0>(*this), std::get<1>(*this), std::get<2>(*this), std::get<3>(*this),
         std::get<4>(*this)
      );
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 6, io::text::writer *>::type ptw) {
      ptw->print(
         SL("{}, {}, {}, {}, {}, {}"),
         std::get<0>(*this), std::get<1>(*this), std::get<2>(*this), std::get<3>(*this),
         std::get<4>(*this), std::get<5>(*this)
      );
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 7, io::text::writer *>::type ptw) {
      ptw->print(
         SL("{}, {}, {}, {}, {}, {}, {}"),
         std::get<0>(*this), std::get<1>(*this), std::get<2>(*this), std::get<3>(*this),
         std::get<4>(*this), std::get<5>(*this), std::get<6>(*this)
      );
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 8, io::text::writer *>::type ptw) {
      ptw->print(
         SL("{}, {}, {}, {}, {}, {}, {}, {}"),
         std::get<0>(*this), std::get<1>(*this), std::get<2>(*this), std::get<3>(*this),
         std::get<4>(*this), std::get<5>(*this), std::get<6>(*this), std::get<7>(*this)
      );
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 9, io::text::writer *>::type ptw) {
      ptw->print(
         SL("{}, {}, {}, {}, {}, {}, {}, {}, {}"),
         std::get<0>(*this), std::get<1>(*this), std::get<2>(*this), std::get<3>(*this),
         std::get<4>(*this), std::get<5>(*this), std::get<6>(*this), std::get<7>(*this),
         std::get<8>(*this)
      );
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 10, io::text::writer *>::type ptw) {
      ptw->print(
         SL("{}, {}, {}, {}, {}, {}, {}, {}, {}, {}"),
         std::get<0>(*this), std::get<1>(*this), std::get<2>(*this), std::get<3>(*this),
         std::get<4>(*this), std::get<5>(*this), std::get<6>(*this), std::get<7>(*this),
         std::get<8>(*this), std::get<9>(*this)
      );
   }
};

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
class _scope_trace :
   public _scope_trace_impl,
   protected _std::tuple<
      T0 const &, T1 const &, T2 const &, T3 const &, T4 const &, T5 const &, T6 const &,
      T7 const &, T8 const &, T9 const &
   > {

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


   /** Destructor.
   */
   ~_scope_trace() {
      try {
         // If this returns a valid pointer, a trace is being generated.
         if (io::text::writer * ptw = _scope_trace_impl::scope_render_start_or_continue()) {
            print<>(ptw);
         }
      } catch (...) {
         // Don’t allow a trace to interfere with the program flow.
         // FIXME: EXC-SWALLOW
      }
   }


protected:

   /** Prints the scope trace to the specified text writer.

   ptw
      Pointer to the writer to output to.
   */
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 0, io::text::writer *>::type ptw) {
      ABC_UNUSED_ARG(ptw);
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 1, io::text::writer *>::type ptw) {
      ptw->print(SL("{}"), _std::get<0>(*this));
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 2, io::text::writer *>::type ptw) {
      ptw->print(SL("{}, {}"), _std::get<0>(*this), _std::get<1>(*this));
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 3, io::text::writer *>::type ptw) {
      ptw->print(SL("{}, {}, {}"), _std::get<0>(*this), _std::get<1>(*this), _std::get<2>(*this));
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 4, io::text::writer *>::type ptw) {
      ptw->print(
         SL("{}, {}, {}, {}"),
         _std::get<0>(*this), _std::get<1>(*this), _std::get<2>(*this), _std::get<3>(*this)
      );
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 5, io::text::writer *>::type ptw) {
      ptw->print(
         SL("{}, {}, {}, {}, {}"),
         _std::get<0>(*this), _std::get<1>(*this), _std::get<2>(*this), _std::get<3>(*this),
         _std::get<4>(*this)
      );
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 6, io::text::writer *>::type ptw) {
      ptw->print(
         SL("{}, {}, {}, {}, {}, {}"),
         _std::get<0>(*this), _std::get<1>(*this), _std::get<2>(*this), _std::get<3>(*this),
         _std::get<4>(*this), _std::get<5>(*this)
      );
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 7, io::text::writer *>::type ptw) {
      ptw->print(
         SL("{}, {}, {}, {}, {}, {}, {}"),
         _std::get<0>(*this), _std::get<1>(*this), _std::get<2>(*this), _std::get<3>(*this),
         _std::get<4>(*this), _std::get<5>(*this), _std::get<6>(*this)
      );
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 8, io::text::writer *>::type ptw) {
      ptw->print(
         SL("{}, {}, {}, {}, {}, {}, {}, {}"),
         _std::get<0>(*this), _std::get<1>(*this), _std::get<2>(*this), _std::get<3>(*this),
         _std::get<4>(*this), _std::get<5>(*this), _std::get<6>(*this), _std::get<7>(*this)
      );
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 9, io::text::writer *>::type ptw) {
      ptw->print(
         SL("{}, {}, {}, {}, {}, {}, {}, {}, {}"),
         _std::get<0>(*this), _std::get<1>(*this), _std::get<2>(*this), _std::get<3>(*this),
         _std::get<4>(*this), _std::get<5>(*this), _std::get<6>(*this), _std::get<7>(*this),
         _std::get<8>(*this)
      );
   }
   template <size_t t_cTs = smc_cTs>
   void print(typename std::enable_if<t_cTs == 10, io::text::writer *>::type ptw) {
      ptw->print(
         SL("{}, {}, {}, {}, {}, {}, {}, {}, {}, {}"),
         _std::get<0>(*this), _std::get<1>(*this), _std::get<2>(*this), _std::get<3>(*this),
         _std::get<4>(*this), _std::get<5>(*this), _std::get<6>(*this), _std::get<7>(*this),
         _std::get<8>(*this), _std::get<9>(*this)
      );
   }
};

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else


// Now these can be implemented.

#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename ... Ts>
inline _scope_trace<Ts ...> _scope_trace_impl::make(Ts const & ... ts) {
   return _scope_trace<Ts ...>(ts ...);
}

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
inline /*static*/ _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> _scope_trace_impl::make(
   T0 const & t0 /*= _std::_tuple_void()*/, T1 const & t1 /*= _std::_tuple_void()*/,
   T2 const & t2 /*= _std::_tuple_void()*/, T3 const & t3 /*= _std::_tuple_void()*/,
   T4 const & t4 /*= _std::_tuple_void()*/, T5 const & t5 /*= _std::_tuple_void()*/,
   T6 const & t6 /*= _std::_tuple_void()*/, T7 const & t7 /*= _std::_tuple_void()*/,
   T8 const & t8 /*= _std::_tuple_void()*/, T9 const & t9 /*= _std::_tuple_void()*/
) {
   return _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(
      t0, t1, t2, t3, t4, t5, t6, t7, t8, t9
   );
}

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

