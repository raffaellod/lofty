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

#ifndef ABC_EXCEPTION_HXX
#define ABC_EXCEPTION_HXX

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/core.hxx>
#include <abc/char.hxx>
#include <exception>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::exception

namespace abc {


/** DOC:8191 Exceptions

Combined with [DOC:8503 Stack tracing], the use of abc_throw() augments the stack trace with the
exact line where the throw statement occurred.

Only instances of abc::exception (or a derived class) can be thrown using abc_throw(), because of
the additional members that the latter expects to be able to set in the former.

The class abc::exception also implements the actual stack trace printing for abc::_stack_trace,
since this file is included in virtually every file whereas trace.hxx is not.
*/

/** Pretty-printed name of the current function. */
#if defined(_GCC_VER)
	#define _ABC_THIS_FUNC \
		__PRETTY_FUNCTION__
#elif defined(_MSC_VER)
	#define _ABC_THIS_FUNC \
		__FUNCTION__
#else
	#define _ABC_THIS_FUNC \
		NULL
#endif


/** Throws the specified object, after providing it with debug information.

x
	Exception instance to be thrown.
*/
#define abc_throw(x) \
	do { \
		auto _x((x)); \
		_x._before_throw(__FILE__, __LINE__, _ABC_THIS_FUNC); \
		throw _x; \
	} while (false)


/** Verifies an expression at compile time; failure is reported as a compiler error. See C++11 § 7
“Declarations” point 4.

expr
	bool-convertible constant expression to be evaluated.
msg
	Diagnostic message to be output in case expr evaluates to false.
*/
#if !defined(_GCC_VER) && !defined(_MSC_VER)
	#define static_assert(expr, msg) \
		extern char _static_assert_failed[(expr) ? 1 : -1]
#endif


// Forward declaration from iostream.hxx.
class ostream;
// Methods here need to use ostream * instead of ostream &, because at this point ostream has only
// been forward-declared above, but not defined yet (a pointer to a forward-declared type is legal,
// but a reference to it is not).


/** Base for all exceptions classes.
*/
class exception :
	public std::exception {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	exception();
	exception(exception const & x);


	/** Destructor.
	*/
	virtual ~exception() decl_throw(());


	/** Assignment operator. See std::exception::operator=().
	*/
	exception & operator=(exception const & x);


	/** Stores context information to be displayed if the exception is not caught.

	pszFileName
		Name of the file in which the exception is being thrown.
	iLine
		Line in pszFileName where the throw statement is located.
	pszFunction
		Function that is throwing the exception.
	*/
	void _before_throw(char const * pszFileName, uint16_t iLine, char const * pszFunction);


	/** Shows a stack trace after an exception has unwound the stack up to the main() level.

	[pstdx]
		Caught exception.
	*/
	static void _uncaught_exception_end(std::exception const * pstdx = NULL);


	/** See std::exception::what().

	return
		Name of the exception class.
	*/
	virtual char const * what() const decl_throw(());


protected:

	/** Prints extended information for the exception.

	pos
		Pointer to a stream to write to.
	*/
	virtual void _print_extended_info(ostream * pos) const;


public:

	/** Establishes, and restores upon destruction, special-case handlers to convert non-C++
	asynchronous error events (POSIX signals, Win32 Structured Exceptions) into C++ exceptions.

	For unsupported OSes, this class is empty. A little silly, but avoids conditional code in other
	files that shouldn’t care whether the target OS is supported in this respect or not.

	Note: this class uses global or thread-local variables (OS-dependent) for all its member
	variables, since their types cannot be specified without #including a lot of files into this one.
	*/
	class async_handler_manager {
#if ABC_HOST_API_LINUX || ABC_HOST_API_WIN32
	public:

		/** Constructor.
		*/
		async_handler_manager();

		/** Destructor.
		*/
		~async_handler_manager();
#endif
	};


protected:

	/** String to be returned by what(). Derived classes can overwrite this instead of overriding the
	entire std::exception::what() method. */
	char const * m_pszWhat;


private:

	/** Source function name. */
	char const * m_pszSourceFunction;
	/** Name of the source file. */
	char const * m_pszSourceFileName;
	/** Number of the source line. */
	uint16_t m_iSourceLine;
	/** true if *this is an in-flight exception (it has been thrown) or is a copy of one. */
	bool m_bInFlight;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::assertion_error


namespace abc {

/** Verifies a condition at runtime, throwing a assertion_error exception if the assertion turns out
to be incorrect.

TODO: comment signature.
*/
#undef assert
#ifdef DEBUG
	#define assert(expr) \
		do { \
			if (!(expr)) { \
				abc::assertion_error::_assertion_failed(__FILE__, __LINE__, _ABC_THIS_FUNC, #expr); \
			} \
		} while (0)
#else
	#define assert(expr) \
		static_cast<void>(0)
#endif


/** An assertion failed.
*/
class assertion_error :
	public exception {
public:

	/** TODO: comment.
	*/
	static ABC_FUNC_NORETURN void _assertion_failed(
		char const * pszFileName, unsigned iLine, char const * pszFunction, char const * pszExpr
	);


protected:

	/** Set to true for the duration of the execution of _assertion_failed(). If another assertion
	fails due to code executed during the call to _assertion_failed(), the latter will just throw,
	without printing anything; otherwise we’ll most likely get stuck in an infinite recursion. */
	static /*tls*/ bool sm_bReentering;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::user_interrupt


namespace abc {

/** The user hit an interrupt key (usually Ctrl-C or Del).
*/
class user_interrupt :
	public exception {
public:
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::generic_error


namespace abc {

/** Integer type used by the OS to represent error numbers. */
#if ABC_HOST_API_POSIX
	typedef int errint_t;
#elif ABC_HOST_API_WIN32
	typedef DWORD errint_t;
#else
	#error TODO-PORT: HOST_API
#endif


#if ABC_HOST_API_POSIX || ABC_HOST_API_WIN32

/** Throws an exception matching a specified OS-defined error, or the last reported by the OS.

err
	OS-defined error number.
*/
ABC_FUNC_NORETURN void throw_os_error();
ABC_FUNC_NORETURN void throw_os_error(errint_t err);

#endif


/** Base for all error-related exceptions classes.
*/
class generic_error :
	public exception {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	generic_error(errint_t err = 0);
	generic_error(generic_error const & x);


	/** Assignment operator. See abc::exception::operator=().
	*/
	generic_error & operator=(generic_error const & x);


	/** Returns the OS-defined error number, if any.

	return
		OS-defined error number.
	*/
	errint_t os_error() const {
		return m_err;
	}


protected:

	/** OS-specific error wrapped by this exception. */
	errint_t m_err;
};

} //namespace abc


// Relational operators.
#define ABC_RELOP_IMPL(op) \
	inline bool operator op(abc::generic_error const & ge1, abc::generic_error const & ge2) { \
		return ge1.os_error() op ge2.os_error(); \
	} \
	inline bool operator op(abc::generic_error const & ge, abc::errint_t err) { \
		return ge.os_error() op err; \
	} \
	inline bool operator op(abc::errint_t err, abc::generic_error const & ge) { \
		return err op ge.os_error(); \
	}
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
#undef ABC_RELOP_IMPL


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::os_error_mapping


namespace abc {

/** Defines a member mapped_error to the default OS-specific error code associated to an exception
class.
*/
template <class TError>
struct os_error_mapping {

	/** Default error code the class errclass maps from. */
	static errint_t const mapped_error = 0;
};


/** Defines an OS-specific error code to be the default for an exception class.
*/
#define ABC_MAP_ERROR_CLASS_TO_ERRINT(errclass, err) \
	template <> \
	class os_error_mapping<errclass> { \
	public: \
	\
		static errint_t const mapped_error = err; \
	}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::argument_error


namespace abc {

/** A function/method received an argument that had an inappropriate value.
*/
class argument_error :
	public virtual generic_error {
public:

	/** Constructor.

	TODO: add arguments name/value, to be passed by macro abc_throw_argument_error(argname).
	*/
	argument_error(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::arithmetic_error


namespace abc {

/** Base for arithmetic errors.
*/
class arithmetic_error :
	public virtual generic_error {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	arithmetic_error(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::division_by_zero_error


namespace abc {

/** The divisor of a division or modulo operation was zero.
*/
class division_by_zero_error :
	public virtual arithmetic_error {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	division_by_zero_error(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::domain_error


namespace abc {

class domain_error :
	public virtual generic_error {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	domain_error(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::floating_point_error


namespace abc {

/** A floating point operation failed.
*/
class floating_point_error :
	public virtual arithmetic_error {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	floating_point_error(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::lookup_error


namespace abc {

/** Base for errors due to an invalid key or index being used on a mapping or sequence.
*/
class lookup_error :
	public virtual generic_error {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	lookup_error(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::index_error


namespace abc {

/** Sequence subscript out of range.
*/
class index_error :
	public virtual lookup_error {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	index_error(intptr_t iInvalid, errint_t err = 0);
	index_error(index_error const & x);


	/** Assignment operator. See abc::lookup_error::operator=().
	*/
	index_error & operator=(index_error const & x);


	/** Returns the invalid index.

	return
		Index that was not valid in the context in which it was used.
	*/
	intptr_t index() const {
		return m_iInvalid;
	}


protected:

	/** See exception::_print_extended_info().
	*/
	virtual void _print_extended_info(ostream * pos) const;


private:

	/** Index that caused the error. */
	intptr_t m_iInvalid;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory_address_error


namespace abc {

/** An attempt was made to access an invalid memory location.
*/
class memory_address_error :
	public virtual generic_error {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	memory_address_error(errint_t err = 0);
	memory_address_error(void const * pInvalid, errint_t err = 0);
	memory_address_error(memory_address_error const & x);


	/** Assignment operator. See abc::generic_error::operator=().
	*/
	memory_address_error & operator=(memory_address_error const & x);


	/** Returns the faulty address.

	return
		Value of the pointer that was dereferenced.
	*/
	void const * address() const {
		return m_pInvalid;
	}


protected:

	/** See exception::_print_extended_info().
	*/
	virtual void _print_extended_info(ostream * pos) const;


private:

	/** Address that could not be accessed. */
	void const * m_pInvalid;
	/** String used as special value for when the address is not available. */
	static char_t const smc_achUnknownAddress[];
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory_access_error


namespace abc {

/** An invalid memory access (e.g. misaligned pointer) was detected.
*/
class memory_access_error :
	public virtual memory_address_error {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	memory_access_error(void const * pInvalid, errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory_allocation_error


namespace abc {

/** A memory allocation request could not be satisfied.
*/
class memory_allocation_error :
	public virtual generic_error,
	public virtual std::bad_alloc {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	memory_allocation_error(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::null_pointer_error


namespace abc {

/** An attempt was made to access the memory location 0 (NULL).
*/
class null_pointer_error :
	public virtual memory_address_error {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	null_pointer_error(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::overflow_error


namespace abc {

/** Result of an arithmetic operation too large to be represented. Because of the lack of
standardization of floating point exception handling in C, most floating point operations are also
not checked.
*/
class overflow_error :
	public virtual arithmetic_error {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	overflow_error(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::syntax_error


namespace abc {

/** The syntax for the specified expression is invalid.
*/
class syntax_error :
	public virtual generic_error {
public:

	/** Constructor.

	All arguments are optional, and can be specified leaving defaulted gaps in between; the resulting
	exception message will not contain omitted arguments.

	The order of line and character is inverted, so that this single overload can be used to
	differenciate between cases in which pszSource is the single line containing the failing
	expression (the thrower would not pass iLine) and cases where pszSource is the source file
	containing the error (the thrower would pass the non-zero line number).

	Examples:

		syntax_error(SL("expression cannot be empty"))
		syntax_error(SL("unmatched '{'"), sExpr, iChar)
		syntax_error(SL("expected expression"), char_range(), iChar, iLine)
		syntax_error(SL("unexpected end of file"), fpSource, iChar, iLine)

	TODO: comment signature.
	*/
	syntax_error(
		char_range const & crDescription = char_range(), 
		char_range const & crSource = char_range(), unsigned iChar = 0, unsigned iLine = 0,
		errint_t err = 0
	);
	syntax_error(syntax_error const & x);


	/** Assignment operator. See abc::generic_error::operator=().
	*/
	syntax_error & operator=(syntax_error const & x);


protected:

	/** See exception::_print_extended_info().
	*/
	virtual void _print_extended_info(ostream * pos) const;


private:

	/** Description of the syntax error. */
	char_range m_crDescription;
	/** Source of the syntax error (whole or individual line). */
	char_range m_crSource;
	/** Character at which the error is located. */
	unsigned m_iChar;
	/** Line where the error is located. */
	unsigned m_iLine;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc other exception classes


namespace abc {

#define ABC_DERIVE_ERROR_CLASS(derived, base) \
	class derived : \
		public virtual base { \
	public: \
	\
		derived(errint_t err = 0) : \
			generic_error(err) { \
		} \
	}


#define ABC_DERIVE_ERROR_CLASS2(derived, base1, base2) \
	class derived : \
		public virtual base1, \
		public virtual base2 { \
	public: \
	\
		derived(errint_t err = 0) : \
			generic_error(err) { \
		} \
	}

/** An attribute reference or assignment failed. */
ABC_DERIVE_ERROR_CLASS(attribute_error, generic_error);
/** Base for errors that occur in the outer system. */
ABC_DERIVE_ERROR_CLASS(environment_error, generic_error);
/** A file could not be found. */
ABC_DERIVE_ERROR_CLASS(file_not_found_error, environment_error);
/** An I/O operation failed for an I/O-related reason. */
ABC_DERIVE_ERROR_CLASS(io_error, environment_error);
/** A buffer-related I/O operation could not be performed. */
ABC_DERIVE_ERROR_CLASS(buffer_error, io_error);
/** The specified file path is not a valid path. */
ABC_DERIVE_ERROR_CLASS(invalid_path_error, generic_error);
/** Mapping (dictionary) key not found in the set of existing keys. */
ABC_DERIVE_ERROR_CLASS(key_error, lookup_error);
/** A network-related error occurred. */
ABC_DERIVE_ERROR_CLASS(network_error, environment_error);
/** An I/O operation failed for a network-related reason. */
ABC_DERIVE_ERROR_CLASS2(network_io_error, io_error, network_error);
/** Method not implemented for this class. Usually thrown when a class is not able to provide a full
implementation of an interface; in practice, this should be avoided. */
ABC_DERIVE_ERROR_CLASS(not_implemented_error, generic_error);
/** An operation failed to prevent a security hazard. */
ABC_DERIVE_ERROR_CLASS(security_error, environment_error);
/** A text encoding or decoding error occurred. */
ABC_DERIVE_ERROR_CLASS(text_error, generic_error);
/** A text decoding error occurred. */
ABC_DERIVE_ERROR_CLASS(text_decode_error, text_error);
/** A text encoding error occurred. */
ABC_DERIVE_ERROR_CLASS(text_encode_error, text_error);

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_EXCEPTION_HXX

