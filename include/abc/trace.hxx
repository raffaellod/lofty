/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

Copyright 2010, 2011, 2012, 2013
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

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/cppmacros.hxx>
#include <abc/string_iostream.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// Declarations

namespace abc {

/// DOC:8503 Stack tracing
//
// Any function that is not of negligible size and is not an hotspot should invoke, as its first
// line, abc_trace_fn((arg1, arg2, …)) in order to have its name show up in a post-exception stack
// trace.
//
// abc_trace_fn() initializes a local variable of type abc::_scope_trace which will store references
// to every provided argument.
//
// abc::_scope_trace::~_scope_trace() detects if the object is being destroyed due to an exceptional
// stack unwinding, in which case it will dump its contents into a thread-local stack trace buffer.
// The outermost catch block (main-level) will output the generated stack trace, if available, using
// abc::exception::_uncaught_exception_end().
//
// When a abc::exception is thrown (it becomes “in-flight”), it will request that the stack trace
// buffer be cleared and it will count itself a a reference to the new trace; when copied, the
// number of references will increase if the source was in-flight, in which case the copy will also
// consider itself in-flight; when an exception is destroyed, it will release a reference to the
// stack trace buffer if it was holding one. Reference counting is necessary due to platform-
// specific code that will copy a thrown exception to non-local storage and throwing that one
// instead of using the original one.
//
// This covers the following code flows:
//
// •  No exception thrown: no stack trace is generated.
//
// •  Exception is thrown and unwinds up to main(): each abc::_scope_trace adds itself to the stack
//    trace, which is then output; the exception is then destroyed, cleaning the trace buffer.
//
// •  Exception is thrown, then caught and blocked: one or more abc::_scope_trace might add
//    themselves to the stack trace, but the exception is blocked before it reaches main(), so no
//    output occurs.
//
// •  Exception is thrown, then caught and rethrown: one or more abc::_scope_trace might add
//    themselves to the stack trace, up to the point the exception is caught. Since the exception is
//    not destroyed, the stack trace buffer will keep the original point at which the exception was
//    thrown, resulting in an accurate stack trace in case the exception reaches main().
//
// •  Exception is thrown, then caught and a new one is thrown: similar to the previous case, except
//    the original exception is destroyed, so the stack trace buffer will not reveal where the
//    original exception was thrown. This is acceptable, since it cannot be determined whether the
//    two exceptions were related.
//
// See related diagram [DOC:8503 Stack tracing] for all code flows covered by this design.
// See also [DOC:8191 Exceptions] and abc::exception for the remainder of the implementation.
//
// Currently unsupported:
// •  TODO: storing a thrown exception, handling a different exception, and throwing back the first
//    one. In the current implementation, there’s nothing telling an exception that it’s no longer
//    in-flight (and there can’t be!); additionally, throwing the second exception and re-throwing
//    the first one (using throw x; instead of throw; because it’s no longer in-flight) will both
//    reset the stack trace buffer.
//
// •  TODO: properly handling exceptions occurring while generating a stack trace. The current
//    behavior swallows any nested exceptions, gracefully failing to generate a complete stack
//    trace.

/// Provides stack frame logging for the function in which it’s used.
//
#define abc_trace_fn(args) \
	_abc_trace_scope_impl(ABC_CPP_APPEND_UID(_scope_trace_), args)

/// Implementation of abc_trace_fn() and similar macros.
#define _abc_trace_scope_impl(var, args) \
	auto var(abc::_scope_trace<>::make args ); \
	var._set_context(__FILE__, __LINE__, _ABC_ASSERT_FN)


/// Tracks local variables, to be used during e.g. a stack unwind.
template <typename ... Ts>
class _scope_trace;

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_scope_trace


namespace abc {

// Base specialization.
template <>
class _scope_trace<> {
public:

	/// Constructor.
	//
	_scope_trace() :
		m_bScopeRenderingStarted(false) {
	}


	/// Destructor. It also completes the trace (possibly started by a derived _scope_trace
	// specialization) for this scope.
	~_scope_trace();


	/// Returns the stack frames collected for this thread up to this point.
	//
	static wdstring get_trace_contents() {
		return get_trace_stream()->get_contents();
	}


	/// Returns a stream to which the stack frame can be output. The stream is thread-local, which is
	// why this can’t be just a static member variable.
	//
	static string_ostream * get_trace_stream() {
		if (!sm_psosScopeTrace) {
			sm_psosScopeTrace.reset(new string_ostream());
		}
		return sm_psosScopeTrace.get();
	}


	/// Similar to std::make_tuple(), allows to use the keyword auto to specify (omit, really) the
	// type of the variable, which would otherwise require knowing the types of the template
	// arguments.
	//
	template <typename ... Ts>
	static _scope_trace<Ts ...> make(Ts const & ... ts) {
		return _scope_trace<Ts ...>(ts ...);
	}


	/// Erases any collected stack frames.
	//
	static void trace_stream_addref() {
		++sm_cScopeTraceRefs;
	}


	/// Erases any collected stack frames.
	//
	static void trace_stream_release() {
		if (sm_cScopeTraceRefs == 1) {
			trace_stream_reset();
		} else if (sm_cScopeTraceRefs > 1) {
			--sm_cScopeTraceRefs;
		}
	}


	/// Erases any collected stack frames.
	//
	static void trace_stream_reset() {
		sm_psosScopeTrace.reset();
		sm_iStackDepth = 0;
		sm_cScopeTraceRefs = 0;
	}


	/// Assigns a context to the scope trace. These cannot be merged with the constructor because we
	// want the constructor to be invoked with all the arguments as a single parenthesis-delimited
	// tuple. See the implementation of abc_trace_fn() if this isn’t clear enough.
	//
	// Also, while we could make the filename and function char_range’s instead of char *, that would
	// waste nearly twice as much stack space for each _scope_trace object, so that’s not a viable
	// option.
	//
	void _set_context(char const * pszFileName, uint16_t iLine, char const * pszFunction) {
		m_pszFileName = pszFileName;
		m_iLine = iLine;
		m_pszFunction = pszFunction;
	}


protected:

	/// Starts or  to the trace stream the scope name.
	ostream * scope_render_start_or_continue();


private:

	/// Function name.
	char const * m_pszFunction;
	/// Name of the source file.
	char const * m_pszFileName;
	/// Number of the source line.
	uint16_t m_iLine;
	/// If true, rendering of this scope trace has started (the function/scope name has been
	// rendered).
	bool m_bScopeRenderingStarted;

	/// Stream that collects the rendered scope trace when an exception is thrown.
	static /*tls*/ std::unique_ptr<string_ostream> sm_psosScopeTrace;
	/// Number of the next stack frame to be added to the rendered trace.
	static /*tls*/ unsigned sm_iStackDepth;
	/// Count of references to the current rendered trace. Managed by abc::exception.
	static /*tls*/ unsigned sm_cScopeTraceRefs;
	/// true if the destructor (the only method that actually may do anything at all) is being run.
	// If this is true, another call to the destructor should not try to do anything, otherwise we’ll
	// get stuck in an infinite recursion.
	static /*tls*/ bool sm_bReentering;
};
// Recursive specialization.
template <typename T0, typename ... Ts>
class _scope_trace<T0, Ts ...> :
	public _scope_trace<Ts ...> {

	typedef _scope_trace<Ts ...> base_scope_trace;

public:

	/// Constructor.
	//
	_scope_trace(T0 const & t0, Ts const & ... ts) :
		base_scope_trace(ts ...),
		m_t0(t0) {
	}


	/// Destructor.
	//
	~_scope_trace() {
		try {
			ostream * pos(base_scope_trace::scope_render_start_or_continue());
			if (pos) {
				*pos << m_t0;
			}
		} catch (...) {
			// Don’t allow a trace to interfere with the program flow.
			// FIXME: EXC-SWALLOW
		}
	}


protected:

	/// Nth argument.
	T0 const & m_t0;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_TRACE_HXX

