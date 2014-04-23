/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#ifndef ABC_TRACE_HXX
#define ABC_TRACE_HXX

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abc/cppmacros.hxx>
#include <abc/str_iostream.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// Declarations

namespace abc {

/** DOC:8503 Stack tracing

Any function that is not of negligible size and is not an hotspot should invoke, as its first line,
ABC_TRACE_FN((arg1, arg2, …)) in order to have its name show up in a post-exception stack trace.

ABC_TRACE_FN() initializes a local variable of type abc::_scope_trace which will store references to
every provided argument.

abc::_scope_trace::~_scope_trace() detects if the object is being destroyed due to an exceptional
stack unwinding, in which case it will dump its contents into a thread-local stack trace buffer. The
outermost catch block (main-level) will output the generated stack trace, if available, using
abc::exception::write_with_scope_trace().

When a abc::exception is thrown (it becomes “in-flight”), it will request that the stack trace
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
#define ABC_TRACE_FN(args) \
   _ABC_TRACE_SCOPE_IMPL(ABC_CPP_APPEND_UID(_scope_trace_), args)

/** Implementation of ABC_TRACE_FN() and similar macros.
*/
#define _ABC_TRACE_SCOPE_IMPL(var, args) \
   auto var(::abc::_scope_trace_impl::make args ); \
   var._set_context(ABC_SOURCE_LOCATION(), _ABC_THIS_FUNC)


/** Tracks local variables, to be used during e.g. a stack unwind. */
#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename ... Ts>
class _scope_trace;

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

template <
   typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void,
   typename T4 = void, typename T5 = void, typename T6 = void, typename T7 = void,
   typename T8 = void, typename T9 = void
>
class _scope_trace;

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_scope_trace_impl


namespace abc {

class ABCAPI _scope_trace_impl {
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


   /** Returns a stream to which the stack frame can be output. The stream is thread-local, which is
   why this can’t be just a static member variable.
   */
   static str_ostream * get_trace_stream() {
      if (!sm_psosScopeTrace) {
         sm_psosScopeTrace.reset(new str_ostream());
      }
      return sm_psosScopeTrace.get();
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


   /** Erases any collected stack frames.
   */
   static void trace_stream_addref() {
      ++sm_cScopeTraceRefs;
   }


   /** Erases any collected stack frames.
   */
   static void trace_stream_release() {
      if (sm_cScopeTraceRefs == 1) {
         trace_stream_reset();
      } else if (sm_cScopeTraceRefs > 1) {
         --sm_cScopeTraceRefs;
      }
   }


   /** Erases any collected stack frames.
   */
   static void trace_stream_reset() {
      sm_psosScopeTrace.reset();
      sm_iStackDepth = 0;
      sm_cScopeTraceRefs = 0;
   }


   /** Assigns a context to the scope trace. These cannot be merged with the constructor because we
   want the constructor to be invoked with all the arguments as a single parenthesis-delimited
   tuple. See the implementation of ABC_TRACE_FN() if this isn’t clear enough.

   srcloc
      Source location.
   pszFunction
      Function name.
   */
   void _set_context(source_location const & srcloc, char const * pszFunction) {
      m_srcloc = srcloc;
      m_pszFunction = pszFunction;
   }


protected:

   /** Starts or  to the trace stream the scope name. */
   ostream * scope_render_start_or_continue();


private:

   /** Function name. */
   char const * m_pszFunction;
   /** Source location. */
   source_location m_srcloc;
   /** If true, rendering of this scope trace has started (the function/scope name has been
   rendered). */
   bool m_bScopeRenderingStarted;

   /** Stream that collects the rendered scope trace when an exception is thrown. */
   static /*tls*/ std::unique_ptr<str_ostream> sm_psosScopeTrace;
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

// Base specialization.
template <>
class _scope_trace<> :
   public _scope_trace_impl {
};

// Recursive specialization.
template <typename T0, typename ... Ts>
class _scope_trace<T0, Ts ...> :
   public _scope_trace<Ts ...> {

   typedef _scope_trace<Ts ...> base_scope_trace;

public:

   /** Constructor.
   */
   _scope_trace(T0 const & t0, Ts const & ... ts) :
      base_scope_trace(ts ...),
      m_t0(t0) {
   }


   /** Destructor.
   */
   ~_scope_trace() {
      try {
         ostream * pos(base_scope_trace::scope_render_start_or_continue());
         if (pos) {
            pos->write(m_t0);
         }
      } catch (...) {
         // Don’t allow a trace to interfere with the program flow.
         // FIXME: EXC-SWALLOW
      }
   }


private:

   /** Nth argument. */
   T0 const & m_t0;
};

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

#if 0

// Recursive implementation.
template <
   typename T0 /*= void*/, typename T1 /*= void*/, typename T2 /*= void*/, typename T3 /*= void*/,
   typename T4 /*= void*/, typename T5 /*= void*/, typename T6 /*= void*/, typename T7 /*= void*/,
   typename T8 /*= void*/, typename T9 /*= void*/
>
class _scope_trace :
   public _scope_trace<T1, T2, T3, T4, T5, T6, T7, T8, T9> {

   typedef _scope_trace<T1, T2, T3, T4, T5, T6, T7, T8, T9> base_scope_trace;

public:

   /** Constructor.
   */
   template <typename U0>
   _scope_trace(typename std::enable_if<!std::is_void<U0>::value, U0 const &>::type t0) :
      base_scope_trace(),
      m_t0(t0) {
   }
   _scope_trace(
      typename std::enable_if<!std::is_void<T1>::value, T0 const &>::type t0,
      T1 const & t1
   ) :
      base_scope_trace(t1),
      m_t0(t0) {
   }
   _scope_trace(
      typename std::enable_if<!std::is_void<T2>::value, T0 const &>::type t0,
      T1 const & t1, T2 const & t2
   ) :
      base_scope_trace(t1, t2),
      m_t0(t0) {
   }
   _scope_trace(
      typename std::enable_if<!std::is_void<T3>::value, T0 const &>::type t0,
      T1 const & t1, T2 const & t2, T3 const & t3
   ) :
      base_scope_trace(t1, t2, t3),
      m_t0(t0) {
   }
   _scope_trace(
      typename std::enable_if<!std::is_void<T4>::value, T0 const &>::type t0,
      T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4
   ) :
      base_scope_trace(t1, t2, t3, t4),
      m_t0(t0) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8, typename U9
   >
   _scope_trace(
      typename std::enable_if<!std::is_void<U9>::value, U0 const &>::type t0,
      U1 const & t1, U2 const & t2, U3 const & t3, U4 const & t4, U5 const & t5, U6 const & t6,
      U7 const & t7, U8 const & t8, U9 const & t9
   ) :
      base_scope_trace(t1, t2, t3, t4, t5, t6, t7, t8, t9),
      m_t0(t0) {
   }


   /** Destructor.
   */
   ~_scope_trace() {
      try {
         ostream * pos(base_scope_trace::scope_render_start_or_continue());
         if (pos) {
            pos->write(m_t0);
         }
      } catch (...) {
         // Don’t allow a trace to interfere with the program flow.
         // FIXME: EXC-SWALLOW
      }
   }


private:

   /** Nth argument. */
   T0 const & m_t0;
};

#else

template <
   typename T0 /*= void*/, typename T1 /*= void*/, typename T2 /*= void*/, typename T3 /*= void*/,
   typename T4 /*= void*/, typename T5 /*= void*/, typename T6 /*= void*/, typename T7 /*= void*/,
   typename T8 /*= void*/, typename T9 /*= void*/
>
class _scope_trace :
   public _scope_trace_impl {
public:

   /** Constructor.
   */
   _scope_trace(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
   ) :
      m_t0(t0),
      m_t1(t1),
      m_t2(t2),
      m_t3(t3),
      m_t4(t4),
      m_t5(t5),
      m_t6(t6),
      m_t7(t7),
      m_t8(t8),
      m_t9(t9) {
   }


   /** Destructor.
   */
   ~_scope_trace() {
      try {
         ostream * pos(_scope_trace_impl::scope_render_start_or_continue());
         if (pos) {
            pos->write(m_t0);
            pos->write(m_t1);
            pos->write(m_t2);
            pos->write(m_t3);
            pos->write(m_t4);
            pos->write(m_t5);
            pos->write(m_t6);
            pos->write(m_t7);
            pos->write(m_t8);
            pos->write(m_t9);
         }
      } catch (...) {
         // Don’t allow a trace to interfere with the program flow.
         // FIXME: EXC-SWALLOW
      }
   }


private:

   /** Nth argument. */
   T0 const & m_t0;
   T1 const & m_t1;
   T2 const & m_t2;
   T3 const & m_t3;
   T4 const & m_t4;
   T5 const & m_t5;
   T6 const & m_t6;
   T7 const & m_t7;
   T8 const & m_t8;
   T9 const & m_t9;
};

template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8
>
class _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7, T8> :
   public _scope_trace_impl {
public:

   /** Constructor.
   */
   _scope_trace(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7, T8 const & t8
   ) :
      m_t0(t0),
      m_t1(t1),
      m_t2(t2),
      m_t3(t3),
      m_t4(t4),
      m_t5(t5),
      m_t6(t6),
      m_t7(t7),
      m_t8(t8) {
   }


   /** Destructor.
   */
   ~_scope_trace() {
      try {
         ostream * pos(_scope_trace_impl::scope_render_start_or_continue());
         if (pos) {
            pos->write(m_t0);
            pos->write(m_t1);
            pos->write(m_t2);
            pos->write(m_t3);
            pos->write(m_t4);
            pos->write(m_t5);
            pos->write(m_t6);
            pos->write(m_t7);
            pos->write(m_t8);
         }
      } catch (...) {
         // Don’t allow a trace to interfere with the program flow.
         // FIXME: EXC-SWALLOW
      }
   }


private:

   /** Nth argument. */
   T0 const & m_t0;
   T1 const & m_t1;
   T2 const & m_t2;
   T3 const & m_t3;
   T4 const & m_t4;
   T5 const & m_t5;
   T6 const & m_t6;
   T7 const & m_t7;
   T8 const & m_t8;
};

template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7
>
class _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7> :
   public _scope_trace_impl {
public:

   /** Constructor.
   */
   _scope_trace(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7
   ) :
      m_t0(t0),
      m_t1(t1),
      m_t2(t2),
      m_t3(t3),
      m_t4(t4),
      m_t5(t5),
      m_t6(t6),
      m_t7(t7) {
   }


   /** Destructor.
   */
   ~_scope_trace() {
      try {
         ostream * pos(_scope_trace_impl::scope_render_start_or_continue());
         if (pos) {
            pos->write(m_t0);
            pos->write(m_t1);
            pos->write(m_t2);
            pos->write(m_t3);
            pos->write(m_t4);
            pos->write(m_t5);
            pos->write(m_t6);
            pos->write(m_t7);
         }
      } catch (...) {
         // Don’t allow a trace to interfere with the program flow.
         // FIXME: EXC-SWALLOW
      }
   }


private:

   /** Nth argument. */
   T0 const & m_t0;
   T1 const & m_t1;
   T2 const & m_t2;
   T3 const & m_t3;
   T4 const & m_t4;
   T5 const & m_t5;
   T6 const & m_t6;
   T7 const & m_t7;
};

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
class _scope_trace<T0, T1, T2, T3, T4, T5, T6> :
   public _scope_trace_impl {
public:

   /** Constructor.
   */
   _scope_trace(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6
   ) :
      m_t0(t0),
      m_t1(t1),
      m_t2(t2),
      m_t3(t3),
      m_t4(t4),
      m_t5(t5),
      m_t6(t6) {
   }


   /** Destructor.
   */
   ~_scope_trace() {
      try {
         ostream * pos(_scope_trace_impl::scope_render_start_or_continue());
         if (pos) {
            pos->write(m_t0);
            pos->write(m_t1);
            pos->write(m_t2);
            pos->write(m_t3);
            pos->write(m_t4);
            pos->write(m_t5);
            pos->write(m_t6);
         }
      } catch (...) {
         // Don’t allow a trace to interfere with the program flow.
         // FIXME: EXC-SWALLOW
      }
   }


private:

   /** Nth argument. */
   T0 const & m_t0;
   T1 const & m_t1;
   T2 const & m_t2;
   T3 const & m_t3;
   T4 const & m_t4;
   T5 const & m_t5;
   T6 const & m_t6;
};

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
class _scope_trace<T0, T1, T2, T3, T4, T5> :
   public _scope_trace_impl {
public:

   /** Constructor.
   */
   _scope_trace(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5
   ) :
      m_t0(t0),
      m_t1(t1),
      m_t2(t2),
      m_t3(t3),
      m_t4(t4),
      m_t5(t5) {
   }


   /** Destructor.
   */
   ~_scope_trace() {
      try {
         ostream * pos(_scope_trace_impl::scope_render_start_or_continue());
         if (pos) {
            pos->write(m_t0);
            pos->write(m_t1);
            pos->write(m_t2);
            pos->write(m_t3);
            pos->write(m_t4);
            pos->write(m_t5);
         }
      } catch (...) {
         // Don’t allow a trace to interfere with the program flow.
         // FIXME: EXC-SWALLOW
      }
   }


private:

   /** Nth argument. */
   T0 const & m_t0;
   T1 const & m_t1;
   T2 const & m_t2;
   T3 const & m_t3;
   T4 const & m_t4;
   T5 const & m_t5;
};

template <typename T0, typename T1, typename T2, typename T3, typename T4>
class _scope_trace<T0, T1, T2, T3, T4> :
   public _scope_trace_impl {
public:

   /** Constructor.
   */
   _scope_trace(T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4) :
      m_t0(t0),
      m_t1(t1),
      m_t2(t2),
      m_t3(t3),
      m_t4(t4) {
   }


   /** Destructor.
   */
   ~_scope_trace() {
      try {
         ostream * pos(_scope_trace_impl::scope_render_start_or_continue());
         if (pos) {
            pos->write(m_t0);
            pos->write(m_t1);
            pos->write(m_t2);
            pos->write(m_t3);
            pos->write(m_t4);
         }
      } catch (...) {
         // Don’t allow a trace to interfere with the program flow.
         // FIXME: EXC-SWALLOW
      }
   }


private:

   /** Nth argument. */
   T0 const & m_t0;
   T1 const & m_t1;
   T2 const & m_t2;
   T3 const & m_t3;
   T4 const & m_t4;
};

template <typename T0, typename T1, typename T2, typename T3>
class _scope_trace<T0, T1, T2, T3> :
   public _scope_trace_impl {
public:

   /** Constructor.
   */
   _scope_trace(T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3) :
      m_t0(t0),
      m_t1(t1),
      m_t2(t2),
      m_t3(t3) {
   }


   /** Destructor.
   */
   ~_scope_trace() {
      try {
         ostream * pos(_scope_trace_impl::scope_render_start_or_continue());
         if (pos) {
            pos->write(m_t0);
            pos->write(m_t1);
            pos->write(m_t2);
            pos->write(m_t3);
         }
      } catch (...) {
         // Don’t allow a trace to interfere with the program flow.
         // FIXME: EXC-SWALLOW
      }
   }


private:

   /** Nth argument. */
   T0 const & m_t0;
   T1 const & m_t1;
   T2 const & m_t2;
   T3 const & m_t3;
};

template <typename T0, typename T1, typename T2>
class _scope_trace<T0, T1, T2> :
   public _scope_trace_impl {
public:

   /** Constructor.
   */
   _scope_trace(T0 const & t0, T1 const & t1, T2 const & t2) :
      m_t0(t0),
      m_t1(t1),
      m_t2(t2) {
   }


   /** Destructor.
   */
   ~_scope_trace() {
      try {
         ostream * pos(_scope_trace_impl::scope_render_start_or_continue());
         if (pos) {
            pos->write(m_t0);
            pos->write(m_t1);
            pos->write(m_t2);
         }
      } catch (...) {
         // Don’t allow a trace to interfere with the program flow.
         // FIXME: EXC-SWALLOW
      }
   }


private:

   /** Nth argument. */
   T0 const & m_t0;
   T1 const & m_t1;
   T2 const & m_t2;
};

template <typename T0, typename T1>
class _scope_trace<T0, T1> :
   public _scope_trace_impl {
public:

   /** Constructor.
   */
   _scope_trace(T0 const & t0, T1 const & t1) :
      m_t0(t0),
      m_t1(t1) {
   }


   /** Destructor.
   */
   ~_scope_trace() {
      try {
         ostream * pos(_scope_trace_impl::scope_render_start_or_continue());
         if (pos) {
            pos->write(m_t0);
            pos->write(m_t1);
         }
      } catch (...) {
         // Don’t allow a trace to interfere with the program flow.
         // FIXME: EXC-SWALLOW
      }
   }


private:

   /** Nth argument. */
   T0 const & m_t0;
   T1 const & m_t1;
};

template <typename T0>
class _scope_trace<T0> :
   public _scope_trace_impl {
public:

   /** Constructor.
   */
   _scope_trace(T0 const & t0) :
      m_t0(t0) {
   }


   /** Destructor.
   */
   ~_scope_trace() {
      try {
         ostream * pos(_scope_trace_impl::scope_render_start_or_continue());
         if (pos) {
            pos->write(m_t0);
         }
      } catch (...) {
         // Don’t allow a trace to interfere with the program flow.
         // FIXME: EXC-SWALLOW
      }
   }


private:

   /** Nth argument. */
   T0 const & m_t0;
};

#endif

// Base specialization.
template <>
class _scope_trace<> :
   public _scope_trace_impl {
};

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

// Now these can be implemented.

#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename ... Ts>
inline _scope_trace<Ts ...> _scope_trace_impl::make(Ts const & ... ts) {
   return _scope_trace<Ts ...>(ts ...);
}

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

inline _scope_trace<> _scope_trace_impl::make() {
   return _scope_trace<>();
}
template <typename T0>
inline _scope_trace<T0> _scope_trace_impl::make(T0 const & t0) {
   return _scope_trace<T0>(t0);
}
template <typename T0, typename T1>
inline _scope_trace<T0, T1> _scope_trace_impl::make(T0 const & t0, T1 const & t1) {
   return _scope_trace<T0, T1>(t0, t1);
}
template <typename T0, typename T1, typename T2>
inline _scope_trace<T0, T1, T2> _scope_trace_impl::make(
   T0 const & t0, T1 const & t1, T2 const & t2
) {
   return _scope_trace<T0, T1, T2>(t0, t1, t2);
}
template <typename T0, typename T1, typename T2, typename T3>
inline _scope_trace<T0, T1, T2, T3> _scope_trace_impl::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3
) {
   return _scope_trace<T0, T1, T2, T3>(t0, t1, t2, t3);
}
template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline _scope_trace<T0, T1, T2, T3, T4> _scope_trace_impl::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4
) {
   return _scope_trace<T0, T1, T2, T3, T4>(t0, t1, t2, t3, t4);
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline _scope_trace<T0, T1, T2, T3, T4, T5> _scope_trace_impl::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5
) {
   return _scope_trace<T0, T1, T2, T3, T4, T5>(t0, t1, t2, t3, t4, t5);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
>
inline _scope_trace<T0, T1, T2, T3, T4, T5, T6> _scope_trace_impl::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6
) {
   return _scope_trace<T0, T1, T2, T3, T4, T5, T6>(t0, t1, t2, t3, t4, t5, t6);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7
>
inline _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7> _scope_trace_impl::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6, T7 const & t7
) {
   return _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7>(t0, t1, t2, t3, t4, t5, t6, t7);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8
>
inline _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7, T8> _scope_trace_impl::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6, T7 const & t7, T8 const & t8
) {
   return _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7, T8>(t0, t1, t2, t3, t4, t5, t6, t7, t8);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
inline _scope_trace<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> _scope_trace_impl::make(
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


#endif //ifndef ABC_TRACE_HXX

