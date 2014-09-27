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
// abc::detail::scope_trace_source_location

namespace abc {
namespace detail {

//! Stores the source code location for a scope_trace instance.
struct scope_trace_source_location {
   //! Function name.
   char_t const * pszFunction;
   //! Path to the source file.
   char_t const * pszFilePath;
   //! Line number in pszFilePath.
   std::uint16_t iLine;
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::scope_trace_tuple

namespace abc {
namespace detail {

// Forward declaration.
#ifdef ABC_CXX_VARIADIC_TEMPLATES
template <typename... Ts>
class scope_trace_tuple_impl;
#else //ifdef ABC_CXX_VARIADIC_TEMPLATES
template <
   typename T0 = _std::_tuple_void, typename T1 = _std::_tuple_void,
   typename T2 = _std::_tuple_void, typename T3 = _std::_tuple_void,
   typename T4 = _std::_tuple_void, typename T5 = _std::_tuple_void,
   typename T6 = _std::_tuple_void, typename T7 = _std::_tuple_void,
   typename T8 = _std::_tuple_void, typename T9 = _std::_tuple_void
>
class scope_trace_tuple_impl;
#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

//! Stores and prints variables for a scope_trace instance.
class ABACLADE_SYM scope_trace_tuple : public noncopyable {
public:
   /*! Returns a scope_trace_tuple containing references to the provided arguments.

   ts
      Arguments.
   */
#ifdef ABC_CXX_VARIADIC_TEMPLATES
   template <typename... Ts>
   static scope_trace_tuple_impl<Ts ...> make(Ts const &... ts);
#else //ifdef ABC_CXX_VARIADIC_TEMPLATES
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
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
   >
   static scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6> make(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
      typename T7
   >
   static scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6, T7> make(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
      typename T7, typename T8
   >
   static scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6, T7, T8> make(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7, T8 const & t8
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
      typename T7, typename T8, typename T9
   >
   static scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> make(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
   );
#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

   /*! Writes the current value of the tuple’s variables.

   ptwOut
      Pointer to the writer to output to.
   */
   virtual void write(io::text::writer * ptwOut) const = 0;

protected:
   /*! Writes an argument separator.

   ptwOut
      Pointer to the writer to output to.
   */
   static void write_separator(io::text::writer * ptwOut);
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::scope_trace_tuple_impl

namespace abc {
namespace detail {

//! Implementation of scope_trace_tuple with actual data storage.
#ifdef ABC_CXX_VARIADIC_TEMPLATES
template <typename... Ts>
class scope_trace_tuple_impl : public scope_trace_tuple, public std::tuple<Ts const & ...> {
private:
   // Handy shortcuts.
   typedef std::tuple<Ts const & ...> tuple_type;
   static std::size_t const smc_cTs = std::tuple_size<tuple_type>::value;
#else //ifdef ABC_CXX_VARIADIC_TEMPLATES
template <
   typename T0 /*= _std::_tuple_void*/, typename T1 /*= _std::_tuple_void*/,
   typename T2 /*= _std::_tuple_void*/, typename T3 /*= _std::_tuple_void*/,
   typename T4 /*= _std::_tuple_void*/, typename T5 /*= _std::_tuple_void*/,
   typename T6 /*= _std::_tuple_void*/, typename T7 /*= _std::_tuple_void*/,
   typename T8 /*= _std::_tuple_void*/, typename T9 /*= _std::_tuple_void*/
>
class scope_trace_tuple_impl :
   public scope_trace_tuple,
   public _std::tuple<
      T0 const &, T1 const &, T2 const &, T3 const &, T4 const &, T5 const &, T6 const &,
      T7 const &, T8 const &, T9 const &
   > {
private:
   // Handy shortcuts.
   typedef _std::tuple<
      T0 const &, T1 const &, T2 const &, T3 const &, T4 const &, T5 const &, T6 const &,
      T7 const &, T8 const &, T9 const &
   > tuple_type;
   static std::size_t const smc_cTs = _std::tuple_size<_std::tuple<
      T0, T1, T2, T3, T4, T5, T6,T7, T8, T9
   >>::value;
#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else
public:
   //! Constructor.
#ifdef ABC_CXX_VARIADIC_TEMPLATES
   explicit scope_trace_tuple_impl(Ts const &... ts) :
      tuple_type(ts ...) {
   }
#else //ifdef ABC_CXX_VARIADIC_TEMPLATES
   explicit scope_trace_tuple_impl(
      T0 const & t0 = _std::_tuple_void(), T1 const & t1 = _std::_tuple_void(),
      T2 const & t2 = _std::_tuple_void(), T3 const & t3 = _std::_tuple_void(),
      T4 const & t4 = _std::_tuple_void(), T5 const & t5 = _std::_tuple_void(),
      T6 const & t6 = _std::_tuple_void(), T7 const & t7 = _std::_tuple_void(),
      T8 const & t8 = _std::_tuple_void(), T9 const & t9 = _std::_tuple_void()
   ) :
      tuple_type(t0, t1, t2, t3, t4, t5, t6, t7, t8, t9) {
   }
#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else
   scope_trace_tuple_impl(scope_trace_tuple_impl && tpl) :
      tuple_type(std::move(tpl)) {
   }

   //! See scope_trace_tuple::write().
   virtual void write(io::text::writer * ptwOut) const override {
      write_vars<0>(ptwOut);
   }

private:
   // This overload prints a variable followed by a comma and recurses.
   template <std::size_t t_i>
#if ABC_HOST_MSC
   // “'<' : expression is always false”.
   #pragma warning(suppress: 4296) 
#endif
   void write_vars(
      typename std::enable_if<t_i + 1 < smc_cTs, io::text::writer *>::type ptwOut
   ) const {
#ifdef ABC_CXX_VARIADIC_TEMPLATES
      ptwOut->write(std::get<t_i>(*this));
#else
      ptwOut->write(_std::get<t_i>(*this));
#endif
      // Write a separator and recurse to write the rest.
      write_separator(ptwOut);
      write_vars<t_i + 1>(ptwOut);
   }
   // This overload writes a variable without a comma to follow, and does not recurse.
   template <std::size_t t_i>
   void write_vars(
      typename std::enable_if<t_i + 1 == smc_cTs, io::text::writer *>::type ptwOut
   ) const {
#ifdef ABC_CXX_VARIADIC_TEMPLATES
      ptwOut->write(std::get<t_i>(*this));
#else
      ptwOut->write(_std::get<t_i>(*this));
#endif
   }
   // This overload does nothing. Only needed because the tuple may be empty, but write() will call
   // write_vars<0>() unconditionally.
   template <std::size_t t_i>
   void write_vars(typename std::enable_if<t_i == smc_cTs, io::text::writer *>::type ptwOut) const {
      ABC_UNUSED_ARG(ptwOut);
   }
};


// Now this can be implemented.
#ifdef ABC_CXX_VARIADIC_TEMPLATES
template <typename... Ts>
inline /*static*/ scope_trace_tuple_impl<Ts ...> scope_trace_tuple::make(Ts const &... ts) {
   return scope_trace_tuple_impl<Ts ...>(ts ...);
}
#else //ifdef ABC_CXX_VARIADIC_TEMPLATES
inline /*static*/ scope_trace_tuple_impl<> scope_trace_tuple::make() {
   return scope_trace_tuple_impl<>();
}
template <typename T0>
inline /*static*/ scope_trace_tuple_impl<T0> scope_trace_tuple::make(T0 const & t0) {
   return scope_trace_tuple_impl<T0>(t0);
}
template <typename T0, typename T1>
inline /*static*/ scope_trace_tuple_impl<T0, T1> scope_trace_tuple::make(
   T0 const & t0, T1 const & t1
) {
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
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6
) {
   return scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6>(t0, t1, t2, t3, t4, t5, t6);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7
>
inline /*static*/ scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6, T7> scope_trace_tuple::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6, T7 const & t7
) {
   return scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6, T7>(t0, t1, t2, t3, t4, t5, t6, t7);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8
>
inline /*static*/ scope_trace_tuple_impl<
   T0, T1, T2, T3, T4, T5, T6, T7, T8
> scope_trace_tuple::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6, T7 const & t7, T8 const & t8
) {
   return scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6, T7, T8>(
      t0, t1, t2, t3, t4, t5, t6, t7, t8
   );
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
inline /*static*/ scope_trace_tuple_impl<
   T0, T1, T2, T3, T4, T5, T6, T7, T8, T9
> scope_trace_tuple::make(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
) {
   return scope_trace_tuple_impl<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>(
      t0, t1, t2, t3, t4, t5, t6, t7, t8, t9
   );
}
#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::scope_trace

namespace abc {
namespace detail {

//! Tracks local variables, to be used during e.g. a stack unwind.
class ABACLADE_SYM scope_trace : public noncopyable {
public:
   /*! Constructor.

   srcloc
      Source location.
   pszFunction
      Function name.
   tplVars
      Variables to capture.
   */
   scope_trace(scope_trace_source_location const * psrcloc, scope_trace_tuple const * ptplVars);

   //! Destructor. Adds a scope in the current scope trace if an in-flight exception is detected.
   ~scope_trace();

   /*! Returns a writer to which the stack frame can be output. The writer is thread-local, which is
   why this can’t be just a static member variable.

   return
      Pointer to the string text writer containing the current stack trace.
   */
   static io::text::str_writer * get_trace_writer() {
      if (!sm_ptswScopeTrace) {
         sm_ptswScopeTrace.reset(new io::text::str_writer());
      }
      return sm_ptswScopeTrace.get();
   }

   //! Increments the reference count of the scope trace being generated.
   static void trace_writer_addref() {
      ++sm_cScopeTraceRefs;
   }

   /*! Decrements the reference count of the scope trace being generated. If the reference count
   reaches zero, trace_writer_clear() will be invoked. */
   static void trace_writer_release() {
      if (sm_cScopeTraceRefs == 1) {
         trace_writer_clear();
      } else if (sm_cScopeTraceRefs > 1) {
         --sm_cScopeTraceRefs;
      }
   }

   //! Erases any collected stack frames.
   static void trace_writer_clear() {
      sm_ptswScopeTrace.reset();
      sm_iStackDepth = 0;
      sm_cScopeTraceRefs = 0;
   }

   /*! Walks the single-linked list of scope_trace instances for the current thread, writing each
   one to the specified writer.

   ptwOut
      Pointer to the writer to output to.
   */
   static void write_list(io::text::writer * ptwOut);

private:
   /*! Writes the scope trace to the specified writer.

   ptwOut
      Pointer to the writer to output to.
   iStackDepth
      Stack index to print next to the trace.
   */
   void write(io::text::writer * ptwOut, unsigned iStackDepth) const;

private:
   //! Pointer to the previous scope_trace single-linked list item that *this replaced as the head.
   scope_trace const * m_pstPrev;
   //! Pointer to the statically-allocated source location.
   scope_trace_source_location const * m_psrcloc;
   //! Pointer to the caller-allocated tuple containing references to local variables in the scope.
   scope_trace_tuple const * m_ptplVars;
   //! Pointer to the head of the scope_trace single-linked list for each thread.
   static /*tls*/ scope_trace const * sm_pstHead;
   //! Writer that collects the rendered scope trace when an exception is thrown.
   static /*tls*/ std::unique_ptr<io::text::str_writer> sm_ptswScopeTrace;
   //! Number of the next stack frame to be added to the rendered trace.
   static /*tls*/ unsigned sm_iStackDepth;
   //! Count of references to the current rendered trace. Managed by abc::exception.
   static /*tls*/ unsigned sm_cScopeTraceRefs;
   /*! true if ~scope_trace() is being run; in that case, another call to it should not try to do
   anything, otherwise we may get stuck in an infinite recursion. */
   static /*tls*/ bool sm_bReentering;
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

