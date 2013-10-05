﻿/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

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

#ifndef ABC_CORE_HXX
#define ABC_CORE_HXX


#if defined(__GNUC__)
	// Make version checks for GCC less cumbersome.
	#define _GCC_VER \
		(__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
	#if _GCC_VER < 40300
		#error Unsupported version of GCC (>= 4.3.0 required)
	#endif
#elif defined(_MSC_VER)
	#if _MSC_VER < 1600
		#error Unsupported version of MSC (>= MSC 16 / VC++ 10 / VS 2010 required)
	#endif
#endif

#ifdef _MSC_VER
	// Suppress unnecessary warnings.

	// “enumerator 'name' in switch of enum 'type' is not explicitly handled by a case label
	#pragma warning(disable: 4061)
	// “enumerator 'name' in switch of enum 'type' is not handled”
	#pragma warning(disable: 4062)
	// “conditional expression is constant”
	#pragma warning(disable: 4127)
	// “'class' : default constructor could not be generated”
	#pragma warning(disable: 4510)
	// “'class' : assignment operator could not be generated”
	#pragma warning(disable: 4512)
	// “class 'class' can never be instantiated - user defined constructor required”
	#pragma warning(disable: 4610)
	// “'class' : copy constructor could not be generated because a base class copy constructor is inaccessible”
	#pragma warning(disable: 4625)
	// “'class' : assignment operator could not be generated because a base class assignment operator is inaccessible”
	#pragma warning(disable: 4626)
	// 'function' : function not inlined
	#pragma warning(disable: 4710)

	// Silence warnings from system header files.
	#pragma warning(push)

	// “'id' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'”
	#pragma warning(disable: 4668)
#endif

#include <limits.h> // CHAR_BIT *_MAX *_MIN
#include <stdint.h> // *int*_t __WORDSIZE (if supported)
#include <stddef.h> // size_t

#ifdef _MSC_VER
	#pragma warning(pop)
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals - HOST_* and OUTPUT_*

#define ABC_HOST_API_WIN32 0
#define ABC_HOST_API_WIN64 0
#define ABC_HOST_API_LINUX 0
#define ABC_HOST_API_POSIX 0
#define ABC_OUTPUT_WIN32_DLL 0
#define ABC_OUTPUT_WIN32_EXE 0
#define ABC_OUTPUT_POSIX_EXE 0

#if defined(_WIN32)
	// Compiling for Win32.
	#undef ABC_HOST_API_WIN32
	#define ABC_HOST_API_WIN32 1
	#ifdef _WIN64
		// Compiling for Win64 (coexists with ABC_HOST_API_WIN32).
		#undef ABC_HOST_API_WIN64
		#define ABC_HOST_API_WIN64 1
	#endif
	#ifdef _WINDLL
		#undef ABC_OUTPUT_WIN32_DLL
		#define ABC_OUTPUT_WIN32_DLL 1
	#else
		#undef ABC_OUTPUT_WIN32_EXE
		#define ABC_OUTPUT_WIN32_EXE 1
	#endif
#elif defined(__linux__)
	// Compiling for Linux.
	#undef ABC_HOST_API_LINUX
	#define ABC_HOST_API_LINUX 1
	#undef ABC_HOST_API_POSIX
	#define ABC_HOST_API_POSIX 1
	#undef ABC_OUTPUT_POSIX_EXE
	#define ABC_OUTPUT_POSIX_EXE 1
#elif defined(__posix__)
	// Compiling for POSIX.
	#undef ABC_HOST_API_POSIX
	#define ABC_HOST_API_POSIX 1
	#undef ABC_OUTPUT_POSIX_EXE
	#define ABC_OUTPUT_POSIX_EXE 1
#endif


/** Machine word size for this microarchitecture. */
#if ABC_HOST_API_WIN32
	#ifdef _WIN64
		#define ABC_HOST_WORD_SIZE 64
	#else
		#define ABC_HOST_WORD_SIZE 32
	#endif
#elif defined(__WORDSIZE)
	#define ABC_HOST_WORD_SIZE __WORDSIZE
#else
	#error Unable to determine the word size for this microarchitecture
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals - platform-dependent fixes

#if ABC_HOST_API_POSIX

	// This prevents stat() from failing for files bigger than 2 GiB.
	#define _FILE_OFFSET_BITS 64

#elif ABC_HOST_API_WIN32

	// Make sure WINVER is defined.
	#ifndef WINVER
		// Pick a default Windows version.
		#ifdef _WIN64
			// The earliest Win64 implementations are Windows Server 2003 (5.2) and Windows XP x64
			// Edition (5.2).
			#define WINVER 0x0502
		#else
			// The earliest Win32 implementations are Windows 95 (4.0) and Windows NT 4 (4.0).
			#define WINVER 0x0400
		#endif
	#endif

	// Make sure _WIN32_WINNT is defined for WINVER values representing NT-only Windows versions.
	// The first NT-only version of Windows is 5.0 (Windows 2000; Windows Me is 4.9).
	#if !defined(_WIN32_WINNT) && WINVER >= 0x0500
		#define _WIN32_WINNT WINVER
	#endif

	// Make sure UNICODE and _UNICODE are coherent; UNICODE wins.
	#if defined(UNICODE) && !defined(_UNICODE)
		#define _UNICODE
	#elif !defined(UNICODE) && defined(_UNICODE)
		#undef _UNICODE
	#endif

	#ifdef _MSC_VER
		// Silence warnings from system header files.
		// These must be disabled until the end of the compilation unit, because that’s when they
		// are raised.

		// “Unreferenced inline function has been removed”
		#pragma warning(disable: 4514)

		// These can be restored after including header files.
		#pragma warning(push)

		// “'macro' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'”
		#pragma warning(disable: 4668)
	#endif

	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>

	#ifdef _MSC_VER
		#pragma warning(pop)
	#endif

	// Quoting MSDN:
	// “To avoid conflicts with min and max in WINDEF.H, use _MIN and _MAX instead. These macros
	// evaluate to _cpp_min and _cpp_max, respectively.”
	// Of course we don’t care for old compatibility macros, and want to use std::min/max instead, so
	// undefine these macros.
	#ifdef min
		#undef min
		#undef max
	#endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals - C++11 compiler support

/** If defined, the compiler supports defining conversion operators as explicit, to avoid executing
them implicitly (N2437). */
#if defined(_GCC_VER) && _GCC_VER >= 40500
	#define ABC_CXX_EXPLICIT_CONVERSION_OPERATORS
#endif

/** If defined, the compiler allows to delete a specific (overload of a) function, method or
constructor (N2346). */
#if defined(_GCC_VER) && _GCC_VER >= 40400
	#define ABC_CXX_FUNCTION_DELETE
#endif

/** If defined, the compiler supports template friend declarations (N1791). */
#if (defined(_GCC_VER) && _GCC_VER >= 40500) || defined(_MSC_VER)
	#define ABC_CXX_TEMPLATE_FRIENDS
#endif

/** If defined, the compiler supports variadic templates (N2242). */
#if defined(_GCC_VER)
	#define ABC_CXX_VARIADIC_TEMPLATES
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals - non-standard, but commonly available, extensions

/** If defined, the compiler supports #pragma once, which tells the preprocessor not to parsing a
(header) file more than once, speeding up compilation. */
#if defined(_MSC_VER)
	#define ABC_CXX_PRAGMA_ONCE

	// Use it now for this file.
	#pragma once
#endif

/** Defines a function as never returning (e.g. by causing the process to terminate, or by throwing
an exception). This allows optimizations based on the fact that code following its call cannot be
reached. */
#if defined(_GCC_VER)
	#define ABC_FUNC_NORETURN \
		__attribute__((noreturn))
#elif defined(_MSC_VER)
	#define ABC_FUNC_NORETURN \
		__declspec(noreturn)
#else
	#define ABC_FUNC_NORETURN
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals - extended features that can take advantage of C++11 or fallback to more risky, but
// still functional alternatives

/** Makes the class un-copiable. Move constructors and/or assignment operators may be still
available, though.

TODO: comment signature.
*/
#ifdef ABC_CXX_FUNCTION_DELETE
	#define ABC_CLASS_PREVENT_COPYING(cls) \
		public: \
		\
			cls(cls const &) = delete; \
			\
			cls & operator=(cls const &) = delete;
#else
	#define ABC_CLASS_PREVENT_COPYING(cls) \
		private: \
		\
			cls(cls const &); \
		\
			cls & operator=(cls const &);
#endif


/** Declares an explicit conversion operator to bool.
*/
#ifdef ABC_CXX_EXPLICIT_CONVERSION_OPERATORS

	#define explicit_operator_bool \
		explicit operator bool


	namespace abc {

	/** A class derived from this one receives support for C++11 explicit operator bool even on
	non-compliant compilers.
	*/
	template <typename T>
	struct support_explicit_operator_bool {};

	} //namespace abc

#else //ifdef ABC_CXX_EXPLICIT_CONVERSION_OPERATORS

	#define explicit_operator_bool \
		bool _explicit_operator_bool


	namespace abc {

	/** Non-template helper for support_explicit_operator_bool.
	*/
	struct _explob_helper {

		/** Non-bool boolean type. */
		typedef void (_explob_helper::* bool_type)() const;


		/** A pointer to this method is used as a boolean true by support_explicit_operator_bool.
		*/
		void bool_true() const;
	};



	/** A class derived from this one receives support for C++11 explicit operator bool even on
	non-compliant compilers.
	*/
	template <typename T>
	struct support_explicit_operator_bool {

		/** Non-bool boolean conversion operator, safer than operator bool(), and almost as good as
		explicit operator bool().

		TODO: comment signature.
		*/
		operator _explob_helper::bool_type() const {
			if (static_cast<T const *>(this)->_explicit_operator_bool()) {
				return &_explob_helper::bool_true;
			} else {
				return 0;
			}
		}
	};

	} //namespace abc

	// Disable relational operators for support_explicit_operator_bool.

	#ifdef ABC_CXX_FUNCTION_DELETE

		#define ABC_RELOP_IMPL(op) \
			template <typename T1, typename T2> \
			bool operator op( \
				abc::support_explicit_operator_bool<T1> const &,
				abc::support_explicit_operator_bool<T2> const & \
			) = delete;

	#else //ifdef ABC_CXX_FUNCTION_DELETE

		#define ABC_RELOP_IMPL(op) \
			template <typename T1, typename T2> \
			inline bool operator op( \
				abc::support_explicit_operator_bool<T1> const & lhs, \
				abc::support_explicit_operator_bool<T2> const & rhs \
			);

	#endif //ifdef ABC_CXX_FUNCTION_DELETE … else

	ABC_RELOP_IMPL(==)
	ABC_RELOP_IMPL(!=)
	#undef ABC_RELOP_IMPL

#endif //ifdef ABC_CXX_EXPLICIT_CONVERSION_OPERATORS … else


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals - other

namespace std {

/** Type whose alignment requirement is at least as large as that of every scalar type (see C++11 §
18.2 “<cstddef>”).
*/
union max_align_t {
	double d;
	long double ld;
	long long ll;
};

} //namespace std


// Support loose exception specifications.
#if defined(_GCC_VER) && _GCC_VER >= 40600
	#define noexcept_false \
		noexcept(false)
	#define noexcept_true \
		noexcept(true)
	#define noexcept_if(cond) \
		noexcept(cond)
#else
	#define noexcept_false
	#define noexcept_true \
		throw()
	#define noexcept_if(cond)
#endif

// Support for pre-C++11 declarations. Use double parentheses for the list.
#define decl_throw(list) \
	throw list


/** Avoids compiler warnings about purposely unused parameters. Win32 has UNREFERENCED_PARAMETER for
this purpose, but this is noticeably shorter :)

TODO: comment signature.
*/
#define UNUSED_ARG(x) \
	static_cast<void>(x)


/** Tells the compiler that a switch statement that apparently doesn’t test for all possible cases
for its argument (e.g. skips some values of an enum), does cover 100% of the actually possible
cases.

It can avoid warnings about not testing for all possible values for an enum, or cases where a
variable would seem to be left uninitialized if the switch argument has a value that’s not one of
those expected by the case labels.
*/
#if defined(_MSC_VER)
	#define no_default \
		default: \
			__assume(0)
#else
	#define no_default \
		default: \
			break
#endif


/** Returns the number of items in a (static) array.

TODO: comment signature.
*/
#undef countof
#define countof(array) \
	(sizeof(array) / sizeof((array)[0]))


/** Returns the offset, in bytes, of a struct/class member. It doesn’t trigger warnings, since it
doesn’t use NULL as the fake address.

TODO: comment signature.
*/
#undef offsetof
#define offsetof(type, member) \
	(reinterpret_cast<size_t>(&reinterpret_cast<type *>(1024)->member) - 1024)


/** Returns the compiler-computed alignment for the specified data type. When compiler support is
lacking, this can be inaccurate for long double.

TODO: comment signature.
*/
#undef alignof
#if defined(_GCC_VER)
	#define alignof(type) \
		__alignof__(type)
#elif defined(_MSC_VER)
	#define alignof(type) \
		__alignof(type)
#else
	#define alignof(type) \
		offsetof(abc::_alignof_helper<type>, t)

	namespace abc {

	template <typename T>
	struct _alignof_helper {
		int8_t misaligner;
		T t;
	};

	} //namespace abc
#endif


/** Returns a size rounded (ceiling) to a count of std::max_align_t units. This allows to declare
storage with alignment suitable for any type, just like ::malloc() does. Identical to
bitmanip::ceiling_to_pow2_multiple(cb, sizeof(std::max_align_t)).

TODO: comment signature.
*/
#define ABC_ALIGNED_SIZE(cb) \
	((size_t(cb) + sizeof(std::max_align_t) - 1) / sizeof(std::max_align_t))


namespace abc {

/** TODO: comment.

TODO maybe move to other header file?
*/
template <typename T>
union force_max_align {
public:

	/** Actual storage. */
	T t;


private:

	/** Forces the whole union to have the most generic alignment; on many architectures this will be
	2 * word size. In any case, this makes the union aligned the same way malloc() aligns the
	pointers it returns. */
	std::max_align_t aligner[ABC_ALIGNED_SIZE(sizeof(T))];
};


/** See unsafe. */
struct unsafe_t {};

/** Constant used as extra argument for functions to force clients to acknowledge they are
performing unsafe operations. */
extern unsafe_t const unsafe;

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_CORE_HXX

