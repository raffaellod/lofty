/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

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


/** Version of GCC if building with it, or 0 otherwise. */
#define ABC_HOST_GCC 0
/** Version of MSC if building with it, or 0 otherwise. */
#define ABC_HOST_MSC 0

#if defined(__GNUC__)
   #undef ABC_HOST_GCC
   #define ABC_HOST_GCC \
      (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
   #if ABC_HOST_GCC < 40300
      #error Unsupported version of GCC (>= 4.3.0 required)
   #endif
#elif defined(_MSC_VER)
   #if _MSC_VER < 1600
      #error Unsupported version of MSC (>= MSC 16 / VC++ 10 / VS 2010 required)
   #endif
   #undef ABC_HOST_MSC
   #define ABC_HOST_MSC _MSC_VER
#endif

#if ABC_HOST_MSC
   // Suppress unnecessary warnings.

   // “enumerator 'name' in switch of enum 'type' is not explicitly handled by a case label
   #pragma warning(disable: 4061)
   // “enumerator 'name' in switch of enum 'type' is not handled”
   #pragma warning(disable: 4062)
   // “conditional expression is constant”
   #pragma warning(disable: 4127)
   // “'class1 member' : class 'template class2' needs to have dll-interface to be used by clients of class 'class1'”
   #pragma warning(disable: 4251)
   // “C++ exception specification ignored except to indicate a function is not __declspec(nothrow)”
   #pragma warning(disable: 4290)
   // “'class' : default constructor could not be generated”
   #pragma warning(disable: 4510)
   // “'class' : assignment operator could not be generated”
   #pragma warning(disable: 4512)
   // “class 'class' can never be instantiated - user defined constructor required”
   #pragma warning(disable: 4610)
   // “'class' : copy constructor could not be generated because a base class copy constructor is
   // inaccessible”
   #pragma warning(disable: 4625)
   // “'class' : assignment operator could not be generated because a base class assignment operator
   // is inaccessible”
   #pragma warning(disable: 4626)
   // “throwing 'abc::_exception_aggregator<TAbc>' the following types will not be considered at the
   // catch site”
   #pragma warning(disable: 4673)
   // “'function' : function not inlined”
   #pragma warning(disable: 4710)
   // “function 'function' selected for automatic inline expansion”
   #pragma warning(disable: 4711)
   // “'struct' : 'n' bytes padding added after data member 'member'”
   #pragma warning(disable: 4820)

   // Silence warnings from system header files.
   #pragma warning(push)

   // “'id' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'”
   #pragma warning(disable: 4668)
#endif //if ABC_HOST_MSC

#include <limits.h> // CHAR_BIT *_MAX *_MIN
#include <stdint.h> // *int*_t __WORDSIZE (if supported)
#include <stddef.h> // size_t

#if ABC_HOST_MSC
   #pragma warning(pop)
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals - ABC_HOST_*

#define ABC_HOST_API_WIN32 0
#define ABC_HOST_API_WIN64 0
#define ABC_HOST_API_LINUX 0
#define ABC_HOST_API_POSIX 0

#if defined(_WIN32)
   // Compiling for Win32.
   #undef ABC_HOST_API_WIN32
   #define ABC_HOST_API_WIN32 1
   #ifdef _WIN64
      // Compiling for Win64 (coexists with ABC_HOST_API_WIN32).
      #undef ABC_HOST_API_WIN64
      #define ABC_HOST_API_WIN64 1
   #endif
#elif defined(__linux__)
   // Compiling for Linux.
   #undef ABC_HOST_API_LINUX
   #define ABC_HOST_API_LINUX 1
   #undef ABC_HOST_API_POSIX
   #define ABC_HOST_API_POSIX 1
#elif defined(__posix__)
   // Compiling for POSIX.
   #undef ABC_HOST_API_POSIX
   #define ABC_HOST_API_POSIX 1
#endif


/** Machine word size for this microarchitecture. */
#if ABC_HOST_API_WIN64
   #define ABC_HOST_WORD_SIZE 64
#elif ABC_HOST_API_WIN32
   #define ABC_HOST_WORD_SIZE 32
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

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX

   // Make sure WINVER is defined.
   #ifndef WINVER
      // Pick a default Windows version.
      #if ABC_HOST_API_WIN64
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

   #if ABC_HOST_MSC
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

   #if ABC_HOST_MSC
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
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals - C++11 compiler support

/** If defined, the compiler supports defining conversion operators as explicit, to avoid executing
them implicitly (N2437). */
#if ABC_HOST_GCC >= 40500
   #define ABC_CXX_EXPLICIT_CONVERSION_OPERATORS
#endif

/** If defined, the compiler allows to delete a specific (overload of a) function, method or
constructor (N2346). */
#if ABC_HOST_GCC >= 40400
   #define ABC_CXX_FUNCTION_DELETE
#endif

/** If defined, the compiler supports the noexcept exception specification. */
#if ABC_HOST_GCC >= 40600
   #define ABC_CXX_NOEXCEPT
#endif

/** If defined, the compiler expects C++11 noexcept specifications for STL functions/methods.
*/
#if ABC_HOST_GCC >= 40700
   #define ABC_CXX_STL_USES_NOEXCEPT
#endif

/** If defined, the STL implements C++11 type traits (as opposed to the early implementations).
*/
#if ABC_HOST_GCC >= 40700
   #define ABC_CXX_STL_CXX11_TYPE_TRAITS
#endif

/** If defined, the compiler supports template friend declarations (N1791). */
#if ABC_HOST_GCC >= 40500 || ABC_HOST_MSC
   #define ABC_CXX_TEMPLATE_FRIENDS
#endif

/** If defined, the compiler supports variadic templates (N2242). */
#if ABC_HOST_GCC
   #define ABC_CXX_VARIADIC_TEMPLATES
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals - non-standard, but commonly available, extensions

/** Calling convention for ABC functions/methods. */
#if ABC_HOST_API_WIN32 && !ABC_HOST_API_WIN64
   #define ABCFNCC __stdcall
#else
   #define ABCFNCC
#endif

/** Declares a function as using the same calling convention as the host C library/STL
implementation. */
#if ABC_HOST_API_WIN32 && !ABC_HOST_API_WIN64
   #define ABC_STL_CALLCONV __cdecl
#else
   #define ABC_STL_CALLCONV
#endif

/** If defined, the compiler supports #pragma once, which tells the preprocessor not to parse a
(header) file more than once, speeding up compilation. */
#if ABC_HOST_GCC || ABC_HOST_MSC
   #define ABC_CXX_PRAGMA_ONCE

   // Use it now for this file.
   #pragma once
#endif

/** Declares a function as never returning (e.g. by causing the process to terminate, or by throwing
an exception). This allows optimizations based on the fact that code following its call cannot be
reached. */
#if ABC_HOST_GCC
   #define ABC_FUNC_NORETURN \
      __attribute__((noreturn))
#elif ABC_HOST_MSC
   #define ABC_FUNC_NORETURN \
      __declspec(noreturn)
#else
   #define ABC_FUNC_NORETURN
#endif

/** Declares a symbol to be publicly visible (exported) in the shared library being built. */
#if ABC_HOST_API_WIN32
   #if ABC_HOST_GCC
      #define ABC_SYM_EXPORT \
         __attribute__((dllexport))
   #elif ABC_HOST_MSC
      #define ABC_SYM_EXPORT \
         __declspec(dllexport)
   #endif
#else
   #if ABC_HOST_GCC
      #define ABC_SYM_EXPORT \
         __attribute__((visibility("default")))
   #endif
#endif

/** Declares a symbol to be imported from a shared library. */
#if ABC_HOST_API_WIN32
   #if ABC_HOST_GCC
      #define ABC_SYM_IMPORT \
         __attribute__((dllimport))
   #elif ABC_HOST_MSC
      #define ABC_SYM_IMPORT \
         __declspec(dllimport)
   #endif
#else
   #if ABC_HOST_GCC
      #define ABC_SYM_IMPORT \
         __attribute__((visibility("default")))
   #endif
#endif

/** Declares a symbol to be publicly visible (from the ABC shared library) or imported from ABC’s
shared library (into another library/executable). */
#ifdef _ABC_LIB_BUILD
   #define ABCAPI ABC_SYM_EXPORT
#else
   #define ABCAPI ABC_SYM_IMPORT
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals - extended features that can take advantage of C++11 or fallback to still-functional
// alternatives

namespace abc {

/** A class derived from this one is not copyable.
*/
class ABCAPI noncopyable {
protected:

   noncopyable() {
   }

#ifdef ABC_CXX_FUNCTION_DELETE

protected:

   noncopyable(noncopyable const &) = delete;

   noncopyable & operator=(noncopyable const &) = delete;

#else //ifdef ABC_CXX_FUNCTION_DELETE

private:

   noncopyable(noncopyable const &);

   noncopyable & operator=(noncopyable const &);

#endif //ifdef ABC_CXX_FUNCTION_DELETE … else
};

} //namespace abc

#include <type_traits>

#if ABC_HOST_MSC && ABC_HOST_MSC < 1800

namespace std {

template <typename T>
typename std::add_rvalue_reference<T>::type declval();

template <typename T, bool t_bIsNoncopyable = false>
struct is_copy_constructible {
private:

   static int test(T &);
   static char test(...);


public:

   static bool const value =
      (sizeof(test(declval<typename add_reference<T>::type>())) == sizeof(int)) &&
      !is_base_of< ::abc::noncopyable, T>::value;
};

#define ABC_STLIMPL_IS_COPY_CONSTRUCTIBLE

} //namespace std

#endif //if ABC_HOST_MSC < 1800


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
   struct support_explicit_operator_bool {
   };

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
      ABCAPI void bool_true() const;
   };



   /** A class derived from this one receives support for C++11 explicit operator bool even on
   non-compliant compilers.
   */
   template <typename T>
   struct support_explicit_operator_bool {

      /** Non-bool boolean conversion operator, safer than operator bool(), and almost as good as
      explicit operator bool().

      return
         A valid pointer if T::explicit_operator_bool() returns true, or NULL otherwise.
      */
      operator _explob_helper::bool_type() const {
         if (static_cast<T const *>(this)->_explicit_operator_bool()) {
            return &_explob_helper::bool_true;
         } else {
            return NULL;
         }
      }
   };

   } //namespace abc

   // Disable relational operators for support_explicit_operator_bool.

   #ifdef ABC_CXX_FUNCTION_DELETE

      #define ABC_RELOP_IMPL(op) \
         template <typename T1, typename T2> \
         bool operator op( \
            abc::support_explicit_operator_bool<T1> const &, \
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


/** Declares a function/method as never throwing exceptions. Supports both C++11 noexcept specifier
and pre-C++11 throw() exception specifications.
*/
#ifdef ABC_CXX_STL_USES_NOEXCEPT
   #define ABC_STL_NOEXCEPT_TRUE() \
      noexcept(true)
#else
   #define ABC_STL_NOEXCEPT_TRUE() \
      throw()
#endif


/** Declares a function/method as possibly throwing exceptions. Supports both C++11 noexcept
specifier and pre-C++11 throw() exception specifications.

old_throw_decl
   Parentheses-enclosed list of types the function/method may throw.
*/
#ifdef ABC_CXX_STL_USES_NOEXCEPT
   #define ABC_STL_NOEXCEPT_FALSE(old_throw_decl) \
      noexcept(false)
#else
   #define ABC_STL_NOEXCEPT_FALSE(old_throw_decl) \
      throw old_throw_decl
#endif


/** Declares a function/method as throwing exceptions depending on a (template-dependent) condition.
Supports both C++11 noexcept specifier and pre-C++11 throw() exception specifications.

old_throw_decl
   Parentheses-enclosed list of types the function/method may throw.
*/
#ifdef ABC_CXX_STL_USES_NOEXCEPT
   #define ABC_STL_NOEXCEPT_IF(cond, old_throw_decl) \
      noexcept(cond)
#else
   #define ABC_STL_NOEXCEPT_IF(cond, old_throw_decl) \
      throw old_throw_decl
#endif


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


/** Avoids compiler warnings about purposely unused parameters. Win32 has UNREFERENCED_PARAMETER for
this purpose, but this is noticeably shorter :)

x
   Unused argument.
*/
#define ABC_UNUSED_ARG(x) \
   static_cast<void>(x)


/** Returns the number of items in a (static) array.

array
   Array for which to compute the count of items.
return
   Count of items in array.
*/
#undef ABC_COUNTOF
#define ABC_COUNTOF(array) \
   (sizeof(array) / sizeof((array)[0]))


/** Returns a size rounded (ceiling) to a count of std::max_align_t units. This allows to declare
storage with alignment suitable for any type, just like ::malloc() does. Identical to
bitmanip::ceiling_to_pow2_multiple(cb, sizeof(std::max_align_t)).

cb
   Size to be aligned to sizeof(std::max_align_t).
return
   Multiple of sizeof(std::max_align_t) not smaller than cb.
*/
#define ABC_ALIGNED_SIZE(cb) \
   ((size_t(cb) + sizeof(std::max_align_t) - 1) / sizeof(std::max_align_t))


namespace abc {

/** See abc::unsafe. */
struct unsafe_t {};

/** Constant used as extra argument for functions to force clients to acknowledge they are
performing unsafe operations. Use as an extra first argument, similary to std::nothrow. */
unsafe_t const unsafe;

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_CORE_HXX

