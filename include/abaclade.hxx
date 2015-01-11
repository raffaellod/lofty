/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
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
#define _ABACLADE_HXX
#define _ABACLADE_HXX_INTERNAL


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc namespace hierarchy

//! Abaclade’s top-level namespace.
namespace abc {
   //! Bit manipulation functions.
   namespace bitmanip {}

   //! Byte-ordering functions.
   namespace byteorder {}

   /*! I/O classes and functions. For an overview of the class/namespace hierarchy, see [IMG:4872
   I/O class hierarchy] */
   namespace io {
      //! Classes and functions to perform I/O in binary mode (raw bytes).
      namespace binary {}

      //! Classes and functions to perform I/O in text mode (with encoding support).
      namespace text {}
   } //namespace io

   //! Mathematical functions and algorithms.
   namespace math {}

   //! Memory management functions and classes. Mostly a templated approach to C’s mem* functions.
   namespace memory {}

   //! Type traits and functions for numeric types.
   namespace numeric {}

   //! Provides facilities to interact with the underlying OS.
   namespace os {}

   //! Support for performance tracking.
   namespace perf {}

   /*! abc::_std contains STL implementation bits from ABC_STLIMPL that we may want to use when
   ABC_STLIMPL is not defined, as Abaclade-only alternatives to lacking/buggy host STL
   implementations. */
   namespace _std {}

   //! Top-level namespace for Abaclade’s testing framework.
   namespace testing {}

   //! Contains classes and functions to work with text strings and characters.
   namespace text {}
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals – ABC_HOST_CXX_*

//! Version of Clang if building with it, or 0 otherwise.
#define ABC_HOST_CXX_CLANG 0
//! Version of GCC if building with it, or 0 otherwise.
#define ABC_HOST_CXX_GCC 0
//! Version of MSC if building with it, or 0 otherwise.
#define ABC_HOST_CXX_MSC 0

#if defined(__clang__)
   #undef ABC_HOST_CXX_CLANG
   #define ABC_HOST_CXX_CLANG \
      (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#elif defined(__GNUC__)
   #undef ABC_HOST_CXX_GCC
   #define ABC_HOST_CXX_GCC \
      (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
   #if ABC_HOST_CXX_GCC < 40400
      #error "Unsupported version of GCC: >= 4.4 required, 4.6 suggested"
   #endif
#elif defined(_MSC_VER)
   #undef ABC_HOST_CXX_MSC
   #define ABC_HOST_CXX_MSC _MSC_VER
   #if ABC_HOST_CXX_MSC < 1600
      #error "Unsupported version of MSC: >= MSC 16 / VC++ 10 / VS 2010 required"
   #endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals – ABC_HOST_API_*

#define ABC_HOST_API_BSD 0
#define ABC_HOST_API_DARWIN 0
#define ABC_HOST_API_FREEBSD 0
#define ABC_HOST_API_LINUX 0
#define ABC_HOST_API_POSIX 0
#define ABC_HOST_API_MACH 0
#define ABC_HOST_API_WIN32 0
#define ABC_HOST_API_WIN64 0

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
#elif defined(__MACH__) && defined(__APPLE__)
   // Compiling for Darwin (OSX/iOS), which looks like a BSD on top of a Mach kernel (XNU).
   #undef ABC_HOST_API_BSD
   #define ABC_HOST_API_BSD 1
   #undef ABC_HOST_API_DARWIN
   #define ABC_HOST_API_DARWIN 1
   #undef ABC_HOST_API_MACH
   #define ABC_HOST_API_MACH 1
   #undef ABC_HOST_API_POSIX
   #define ABC_HOST_API_POSIX 1
#elif defined(__unix__)
   #ifdef __FreeBSD__
      // Compiling for FreeBSD.
      #undef ABC_HOST_API_BSD
      #define ABC_HOST_API_BSD 1
      #undef ABC_HOST_API_FREEBSD
      #define ABC_HOST_API_FREEBSD 1
   #endif

   // In any case, approximate UNIX as POSIX.
   #undef ABC_HOST_API_POSIX
   #define ABC_HOST_API_POSIX 1
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals – ABC_HOST_ARCH_*

#define ABC_HOST_ARCH_ALPHA  0
#define ABC_HOST_ARCH_I386   0
#define ABC_HOST_ARCH_IA64   0
#define ABC_HOST_ARCH_PPC    0
#define ABC_HOST_ARCH_X86_64 0

#if defined(__alpha__) || defined(_M_ALPHA)
   #undef ABC_HOST_ARCH_ALPHA
   #define ABC_HOST_ARCH_ALPHA 1
#elif defined(__i386__) || defined(_M_IX86)
   #undef ABC_HOST_ARCH_I386
   #define ABC_HOST_ARCH_I386 1
#elif defined(__ia64__) || defined(_M_IA64)
   #undef ABC_HOST_ARCH_IA64
   #define ABC_HOST_ARCH_IA64 1
#elif defined(__powerpc__) || defined(_M_PPC) || defined(_M_MPPC)
   #undef ABC_HOST_ARCH_PPC
   #define ABC_HOST_ARCH_PPC 1
#elif defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
   #undef ABC_HOST_ARCH_X86_64
   #define ABC_HOST_ARCH_X86_64 1
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals – ABC_HOST_WORD_SIZE

//! Machine word size for this microarchitecture.
// TODO: the word/pointer size is much more easily detected by a configure program.
#if ABC_HOST_API_WIN64
   #define ABC_HOST_WORD_SIZE 64
#elif ABC_HOST_API_WIN32
   #define ABC_HOST_WORD_SIZE 32
#elif defined(__SIZEOF_POINTER__)
   #define ABC_HOST_WORD_SIZE (__SIZEOF_POINTER__ * 8)
#else
   #error "Unable to determine the word size for this microarchitecture"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals – ABC_HOST_*_ENDIAN

#define ABC_HOST_LITTLE_ENDIAN 0
#define ABC_HOST_BIG_ENDIAN    0

#if ABC_HOST_ARCH_ALPHA || ABC_HOST_ARCH_I386 || ABC_HOST_ARCH_IA64 || ABC_HOST_ARCH_X86_64
   #undef ABC_HOST_LITTLE_ENDIAN
   #define ABC_HOST_LITTLE_ENDIAN 1
#elif ABC_HOST_ARCH_PPC
   #undef ABC_HOST_BIG_ENDIAN
   #define ABC_HOST_BIG_ENDIAN 1
#else
   #error "TODO: HOST_ARCH"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals – platform-dependent fixes

// Compatibility with compilers that don’t support feature checking.
#ifndef __has_feature
   #define __has_feature(x) \
      0
#endif

#if ABC_HOST_CXX_MSC
   // Suppress unnecessary warnings.

   // “enumerator 'name' in switch of enum 'type' is not explicitly handled by a case label
   #pragma warning(disable: 4061)
   // “enumerator 'name' in switch of enum 'type' is not handled”
   #pragma warning(disable: 4062)
   // “conditional expression is constant”
   #pragma warning(disable: 4127)
   // “'class' : inherits 'base::member' via dominance” – it points out the obvious and intended.
   #pragma warning(disable: 4250)
   // “'class1 member' : class 'template class2' needs to have dll-interface to be used by clients
   // of class 'class1'”
   #pragma warning(disable: 4251)
   // “'class' : class has virtual functions, but destructor is not virtual”
   #pragma warning(disable: 4265)
   // “non dll-interface class 'class1' used as base for dll-interface class 'class2'”
   #pragma warning(disable: 4275)
   // “C++ exception specification ignored except to indicate a function is not __declspec(nothrow)”
   #pragma warning(disable: 4290)
   // “cast truncates constant value” – would be useful, but it’s raised too easily by MSC16.
   #pragma warning(disable: 4310)
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
   // “potentially uninitialized local variable 'var' used” – would be useful, but it’s raised too
   // easily by MSC16.
   #pragma warning(disable: 4701)
   // “'function' : function not inlined”
   #pragma warning(disable: 4710)
   // “function 'function' selected for automatic inline expansion”
   #pragma warning(disable: 4711)
   // “'struct' : 'n' bytes padding added after data member 'member'”
   #pragma warning(disable: 4820)
#endif //if ABC_HOST_CXX_MSC

// Make sure DEBUG and _DEBUG are coherent; DEBUG wins.
#if defined(DEBUG) && !defined(_DEBUG)
   #define _DEBUG
#elif !defined(DEBUG) && defined(_DEBUG)
   #undef _DEBUG
#endif

#include <abaclade/cppmacros.hxx>

#if ABC_HOST_CXX_MSC
   // Prevent MSC16’s yvals.h from typedef’ing char16_t as unsigned short.
   #define char16_t _ABC_MSC16_char16_t
   #include <yvals.h>
   #undef char16_t

   // Silence warnings from system header files.
   #pragma warning(push)
   // “'id' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'”
   #pragma warning(disable: 4668)
#endif //if ABC_HOST_CXX_MSC
#include <cstdint> // std::*int*_t
#if ABC_HOST_CXX_MSC
   #pragma warning(pop)
#endif
// This defines our “real” char16_t.
#include <abaclade/char.hxx>

#if ABC_HOST_API_POSIX
   // This prevents stat() from failing for files bigger than 2 GiB.
   #define _FILE_OFFSET_BITS 64
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   // Make sure WINVER is defined.
   #ifndef WINVER
      // Pick a default Windows version.
      #if ABC_HOST_API_WIN64
         /* The earliest Win64 implementations are Windows Server 2003 (5.2) and Windows XP x64
         Edition (5.2). */
         #define WINVER 0x0502
      #else
         /* The earliest supported Win32 implementations are Windows 95 (4.0) and Windows NT 4
         (4.0). */
         #define WINVER 0x0400
      #endif
   #endif

   // Make sure _WIN32_WINNT is defined for WINVER values representing NT-only Windows versions.
   // The first NT-only version of Windows is 5.0 (Windows 2000; Windows Me is 4.9).
   #if !defined(_WIN32_WINNT) && WINVER >= 0x0500
      #define _WIN32_WINNT WINVER
   #endif

   #if ABC_HOST_CXX_MSC
      // Silence warnings from system header files.

      /* “Unreferenced inline function has been removed” ‒ must be disabled until the end of the
      compilation unit, because that’s when it’s raised. */
      #pragma warning(disable: 4514)
      // These can be restored after including header files.
      #pragma warning(push)
      // “'macro' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'”
      #pragma warning(disable: 4668)
   #endif

   #define WIN32_LEAN_AND_MEAN
   #include <windows.h>

   #if ABC_HOST_CXX_MSC
      #pragma warning(pop)
   #endif

   /* Quoting MSDN:
      “To avoid conflicts with min and max in WINDEF.H, use _MIN and _MAX instead. These macros
      evaluate to _cpp_min and _cpp_max, respectively.”
   Of course we don’t care for old compatibility macros, and want to use std::min/max instead, so
   undefine these macros. */
   #ifdef min
      #undef min
      #undef max
   #endif
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals – C++11 compiler features detection

// Ensure RTTI support is enabled for MSC.
#if ABC_HOST_CXX_MSC && !defined(_CPPRTTI)
   #error "Please compile with /GR"
#endif

/*! If defined, the compiler supports defining conversion operators as explicit, to avoid executing
them implicitly (N2437). */
#if (ABC_HOST_CXX_CLANG && __has_feature(cxx_explicit_conversions)) || ABC_HOST_CXX_GCC >= 40500
   #define ABC_CXX_EXPLICIT_CONVERSION_OPERATORS
#endif

/*! If defined, the compiler allows to delete a specific (overload of a) function, method or
constructor (N2346). */
#if (ABC_HOST_CXX_CLANG && __has_feature(cxx_deleted_functions)) || ABC_HOST_CXX_GCC >= 40400
   #define ABC_CXX_FUNC_DELETE
#endif

//! If defined, the compiler supports the noexcept exception specification.
#if (ABC_HOST_CXX_CLANG && __has_feature(cxx_noexcept)) || ABC_HOST_CXX_GCC >= 40600
   #define ABC_CXX_NOEXCEPT
#endif

//! If defined, the compiler expects C++11 noexcept specifications for STL functions/methods.
#if (ABC_HOST_CXX_CLANG && __has_feature(cxx_noexcept)) || ABC_HOST_CXX_GCC >= 40700
   #define ABC_CXX_STL_USES_NOEXCEPT
#endif

//! If defined, the STL implements C++11 type traits (as opposed to early similar implementations).
#if ABC_HOST_CXX_CLANG && !defined(__GLIBCXX__)
   #define ABC_CXX_STL_CXX11_TYPE_TRAITS
#endif

/*! If defined, the STL implements part of the C++11 type traits. This is a special case for the GNU
libc++; see <https://gcc.gnu.org/onlinedocs/gcc-4.9.2/libstdc++/manual/manual/status.html> for the
supported type traits. */
#if (ABC_HOST_CXX_CLANG && defined(__GLIBCXX__)) || ABC_HOST_CXX_GCC >= 40800
   #define ABC_CXX_STL_CXX11_GLIBCXX_PARTIAL_TYPE_TRAITS
#endif

//! If defined, the compiler supports variadic templates (N2242).
#if (ABC_HOST_CXX_CLANG && __has_feature(cxx_variadic_templates)) || ABC_HOST_CXX_GCC
   #define ABC_CXX_VARIADIC_TEMPLATES
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals – compatibility layer for C++11 features that have been implemented in a non-standard
// way by other compilers

/*! Range-based for statement: for (for-range-declaration : expression) { … } .

rangedecl
   Declaration of the variable that will hold the values iterated over from the range; most commonly
   this is auto & or auto const &.
expr
   Expression of a type for which std::begin() and std::end() are defined.
*/
#if ( \
   ABC_HOST_CXX_CLANG && __has_feature(cxx_range_for) \
) || ABC_HOST_CXX_GCC || ABC_HOST_CXX_MSC >= 1700
   #define ABC_FOR_EACH(rangedecl, expr) \
      for (rangedecl : expr)
#elif ABC_HOST_CXX_MSC
   /* MSC16 has a pre-C++11 syntax that expects to assign expr to a non-const l-value reference; if
   expr is an r-value, an MSC non-standard extension allows to reference expr from the non-const
   l-value reference, raising warning C4239; here we suppress this possible warning. */
   #define ABC_FOR_EACH(rangedecl, expr) \
      __pragma(warning(suppress: 4239)) \
      for each (rangedecl in expr)
#endif

#if (ABC_HOST_CXX_CLANG && __has_feature(cxx_override_control)) || ABC_HOST_CXX_GCC >= 0x40700
   // Good, no need for fixes.
#elif ABC_HOST_CXX_MSC
   // MSC16 thinks that override is a non-standard extension, so we need to tell it otherwise.
   #define override \
      __pragma(warning(suppress: 4481)) override
#else
   // For everybody else, just disable override control.
   #define override
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals – compatibility layer for non-standard features available in all supported compilers

/*! Declares a function as using the same calling convention as the host C library/STL
implementation. */
#if ABC_HOST_API_WIN32 && !ABC_HOST_API_WIN64
   #define ABC_STL_CALLCONV __cdecl
#else
   #define ABC_STL_CALLCONV
#endif

/*! If defined, the compiler supports #pragma once, which tells the preprocessor not to parse a
(header) file more than once, speeding up compilation. */
#if ABC_HOST_CXX_CLANG || ABC_HOST_CXX_GCC || ABC_HOST_CXX_MSC
   #define ABC_CXX_PRAGMA_ONCE
   // Use it now for this file.
   #pragma once
#endif

/*! Declares a function as never returning (e.g. by causing the process to terminate, or by throwing
an exception). This allows optimizations based on the fact that code following its call cannot be
reached. */
#if ABC_HOST_CXX_CLANG || ABC_HOST_CXX_GCC
   #define ABC_FUNC_NORETURN \
      __attribute__((noreturn))
#elif ABC_HOST_CXX_MSC
   #define ABC_FUNC_NORETURN \
      __declspec(noreturn)
#else
   #define ABC_FUNC_NORETURN
#endif

//! Declares a symbol to be publicly visible (exported) in the shared library being built.
#if ABC_HOST_API_WIN32
   // TODO: how does Clang declare dllexport?
   #if ABC_HOST_CXX_GCC
      #define ABC_SYM_EXPORT \
         __attribute__((dllexport))
   #elif ABC_HOST_CXX_MSC
      #define ABC_SYM_EXPORT \
         __declspec(dllexport)
   #endif
#else
   #if ABC_HOST_CXX_CLANG || ABC_HOST_CXX_GCC
      #define ABC_SYM_EXPORT \
         __attribute__((visibility("default")))
   #endif
#endif

//! Declares a symbol to be imported from a shared library.
#if ABC_HOST_API_WIN32
   // TODO: how does Clang declare dllimport?
   #if ABC_HOST_CXX_GCC
      #define ABC_SYM_IMPORT \
         __attribute__((dllimport))
   #elif ABC_HOST_CXX_MSC
      #define ABC_SYM_IMPORT \
         __declspec(dllimport)
   #endif
#else
   #if ABC_HOST_CXX_CLANG || ABC_HOST_CXX_GCC
      #define ABC_SYM_IMPORT \
         __attribute__((visibility("default")))
   #endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals – symbol visibility

/*! Declares a symbol to be publicly visible (from the Abaclade shared library) or imported from
Abaclade’s shared library (into another library/executable). */
#ifdef ABAMAKE_BUILD_ABACLADE
   #define ABACLADE_SYM ABC_SYM_EXPORT
#else
   #define ABACLADE_SYM ABC_SYM_IMPORT
#endif

/*! Declares a symbol to be publicly visible (from the Abaclade testing shared library) or imported
from Abaclade’s testing shared library (into another library/executable). */
#ifdef ABAMAKE_BUILD_ABACLADE_TESTING
   #define ABACLADE_TESTING_SYM ABC_SYM_EXPORT
#else
   #define ABACLADE_TESTING_SYM ABC_SYM_IMPORT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals – extended features that can take advantage of C++11 or fallback to still-functional
// alternatives, plus a few compiler-specific STL fixes

#ifdef ABC_STLIMPL
// In case we’re reimplementing all of STL, just merge ::abc::_std into ::std.
namespace std {
using namespace ::abc::_std;
} //namespace std
#endif

namespace abc {

//! A class derived from this one is not copyable.
class ABACLADE_SYM noncopyable {
protected:
   noncopyable() {
   }

#ifdef ABC_CXX_FUNC_DELETE
protected:
   noncopyable(noncopyable const &) = delete;
   noncopyable & operator=(noncopyable const &) = delete;
#else //ifdef ABC_CXX_FUNC_DELETE
private:
   noncopyable(noncopyable const &);
   noncopyable & operator=(noncopyable const &);
#endif //ifdef ABC_CXX_FUNC_DELETE … else
};

} //namespace abc

#ifdef ABC_STLIMPL
   #include <abaclade/stl/type_traits.hxx>
#else
   #include <type_traits>
#endif

#if ( \
   (ABC_HOST_CXX_GCC && ABC_HOST_CXX_GCC < 40700) || (ABC_HOST_CXX_MSC && ABC_HOST_CXX_MSC < 1800) \
) && !defined(ABC_STLIMPL)

namespace std {

#if !ABC_HOST_CXX_GCC
// GCC does have a definition of std::declval, but MSC does not.
template <typename T>
typename add_rvalue_reference<T>::type declval();
#endif
#if ABC_HOST_CXX_GCC
// On the other hand, GCC lacks a definition of std::add_reference.
template <typename T>
struct add_reference {
   typedef T & type;
};
template <typename T>
struct add_reference<T &> {
   typedef T & type;
};
#endif

template <typename T, typename = void>
struct is_copy_constructible {
private:
   static int test(T &);
   static char test(...);

   typedef typename add_reference<T>::type TRef;

public:
   static bool const value = (sizeof(test(declval<TRef>())) == sizeof(int));
};

template <typename T>
struct is_copy_constructible<T, typename enable_if<
   is_base_of< ::abc::noncopyable, T>::value
>::type> : public false_type {};

#define ABC_STLIMPL_IS_COPY_CONSTRUCTIBLE

} //namespace std

#endif //if ((ABC_HOST_CXX_GCC && ABC_HOST_CXX_GCC < 40700) ||
       //   (ABC_HOST_CXX_MSC && ABC_HOST_CXX_MSC < 1800) && !defined(ABC_STLIMPL)

//! Declares an explicit conversion operator to bool.
#ifdef ABC_CXX_EXPLICIT_CONVERSION_OPERATORS
   #define explicit_operator_bool \
      explicit operator bool

   namespace abc {

   /*! A class derived from this one receives support for C++11 explicit operator bool even on
   non-compliant compilers. */
   template <typename T>
   struct support_explicit_operator_bool {
   };

   } //namespace abc
#else //ifdef ABC_CXX_EXPLICIT_CONVERSION_OPERATORS
   #define explicit_operator_bool \
      bool _explicit_operator_bool

   namespace abc {
   namespace detail {

   //! Non-template helper for support_explicit_operator_bool.
   struct explob_helper {
      //! Non-bool boolean type.
      typedef void (explob_helper::* bool_type)() const;

      //! A pointer to this method is used as a boolean true by support_explicit_operator_bool.
      ABACLADE_SYM void bool_true() const;
   };

   } //namespace detail

   /*! A class derived from this one receives support for C++11 explicit operator bool even on
   non-compliant compilers. */
   template <typename T>
   struct support_explicit_operator_bool {
      /*! Non-bool boolean conversion operator, safer than operator bool(), and almost as good as
      explicit operator bool().

      @return
         A valid pointer if T::explicit_operator_bool() returns true, or nullptr otherwise.
      */
      operator detail::explob_helper::bool_type() const {
         if (static_cast<T const *>(this)->_explicit_operator_bool()) {
            return &detail::explob_helper::bool_true;
         } else {
            return nullptr;
         }
      }
   };

   // Disable relational operators for support_explicit_operator_bool.
   #ifdef ABC_CXX_FUNC_DELETE
      #define ABC_RELOP_IMPL(op) \
         template <typename T1, typename T2> \
         bool operator op( \
            support_explicit_operator_bool<T1> const &, support_explicit_operator_bool<T2> const & \
         ) = delete;
   #else //ifdef ABC_CXX_FUNC_DELETE
      #define ABC_RELOP_IMPL(op) \
         template <typename T1, typename T2> \
         inline bool operator op( \
            support_explicit_operator_bool<T1> const & lhs, \
            support_explicit_operator_bool<T2> const & rhs \
         );
   #endif //ifdef ABC_CXX_FUNC_DELETE … else

   ABC_RELOP_IMPL(==)
   ABC_RELOP_IMPL(!=)
   #undef ABC_RELOP_IMPL

   } //namespace abc
#endif //ifdef ABC_CXX_EXPLICIT_CONVERSION_OPERATORS … else

/*! Declares a function/method as never throwing exceptions. Supports both C++11 noexcept specifier
and pre-C++11 throw() exception specifications. */
#ifdef ABC_CXX_STL_USES_NOEXCEPT
   #define ABC_STL_NOEXCEPT_TRUE() \
      noexcept(true)
#else
   #define ABC_STL_NOEXCEPT_TRUE() \
      throw()
#endif

/*! Declares a function/method as possibly throwing exceptions. Supports both C++11 noexcept
specifier and pre-C++11 throw() exception specifications.

@param old_throw_decl
   Parentheses-enclosed list of types the function/method may throw.
*/
#ifdef ABC_CXX_STL_USES_NOEXCEPT
   #define ABC_STL_NOEXCEPT_FALSE(old_throw_decl) \
      noexcept(false)
#else
   #define ABC_STL_NOEXCEPT_FALSE(old_throw_decl) \
      throw old_throw_decl
#endif

/*! Declares a function/method as throwing exceptions depending on a (template-dependent) condition.
Supports both C++11 noexcept specifier and pre-C++11 throw() exception specifications.

@param old_throw_decl
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
// abc globals – other

namespace abc {

#ifdef ABC_STLIMPL

typedef _std::max_align_t max_align_t;

#else //ifdef ABC_STLIMPL

/*! Type whose alignment requirement is at least as large as that of any scalar type (see C++11 §
18.2 “<cstddef>”). */
union max_align_t {
   double d;
   long double ld;
   long long ll;
};

#endif //ifdef ABC_STLIMPL … else

} //namespace abc

/*! Avoids compiler warnings about purposely unused parameters. Win32 has UNREFERENCED_PARAMETER for
this purpose, but this is noticeably shorter :)

@param x
   Unused argument.
*/
#define ABC_UNUSED_ARG(x) \
   static_cast<void>(x)

/*! Returns the number of items in a (static) array.

@param array
   Array for which to compute the count of items.
@return
   Count of items in array.
*/
#define ABC_COUNTOF(array) \
   (sizeof(array) / sizeof((array)[0]))

/*! Returns a size rounded (ceiling) to a count of abc::max_align_t units. This allows to declare
storage with alignment suitable for any type, just like std::malloc() does. Identical to
bitmanip::ceiling_to_pow2_multiple(cb, sizeof(abc::max_align_t)).

@param cb
   Size to be aligned to sizeof(abc::max_align_t).
@return
   Multiple of sizeof(abc::max_align_t) not smaller than cb.
*/
#define ABC_ALIGNED_SIZE(cb) \
   ((static_cast<std::size_t>(cb) + sizeof(::abc::max_align_t) - 1) / sizeof(::abc::max_align_t))

////////////////////////////////////////////////////////////////////////////////////////////////////
// #include other core header files that require a special order

#if ABC_HOST_CXX_MSC
   // Silence warnings from system header files.
   #pragma warning(push)
   // “expression before comma has no effect; expected expression with side-effect”
   #pragma warning(disable: 4548)
   // “'id' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'”
   #pragma warning(disable: 4668)
#endif //if ABC_HOST_CXX_MSC
#include <climits> // CHAR_BIT *_MAX *_MIN
#include <cstddef> // std::ptrdiff_t std::size_t
#if ABC_HOST_CXX_MSC
   #pragma warning(pop)
#endif

#if defined(ABC_STLIMPL) || !defined(ABC_CXX_VARIADIC_TEMPLATES)
   #include <abaclade/stl/tuple.hxx>
#endif
#ifdef ABC_STLIMPL
   #include <abaclade/stl/memory.hxx>
#else //ifdef ABC_STLIMPL
   #include <tuple>
   #if ABC_HOST_CXX_MSC
      // Silence warnings from system header files.
      #pragma warning(push)
      // “expression before comma has no effect; expected expression with side-effect”
      #pragma warning(disable: 4548)
      // “'function': exception specification does not match previous declaration”
      #pragma warning(disable: 4986)
   #endif //if ABC_HOST_CXX_MSC
   #include <memory>
   #if ABC_HOST_CXX_MSC
      #pragma warning(pop)
   #endif
   #ifdef ABC_STLIMPL_IS_COPY_CONSTRUCTIBLE
      namespace std {

      // (Partially-) specialize is_copy_constructible for MSC-provided STL types.
      template <typename T, typename TDeleter>
      struct is_copy_constructible<unique_ptr<T, TDeleter>> : public false_type {};

      } //namespace std
   #endif
#endif //ifdef ABC_STLIMPL … else
#include <abaclade/memory.hxx>

// Forward declarations.

namespace abc {

class str_base;
class istr;
class dmstr;

namespace io {
namespace text {

class writer;

} //namespace text
} //namespace io
} //namespace abc

#ifdef ABC_STLIMPL
   #include <abaclade/stl/exception.hxx>
   #include <abaclade/stl/functional.hxx>
   #include <abaclade/stl/iterator.hxx>
#else
   #include <exception>
   #include <functional>
   #include <iterator>
#endif
#include <abaclade/detail/xor_list_node_impl.hxx>
#include <abaclade/detail/xor_list_iterator_impl.hxx>
#include <abaclade/static_list.hxx>
#include <abaclade/thread_local.hxx>
#include <abaclade/exception.hxx>
#include <abaclade/enum.hxx>
#include <abaclade/pointer_iterator.hxx>
#include <abaclade/numeric.hxx>
#include <abaclade/type_void_adapter.hxx>
#include <abaclade/detail/vextr.hxx>
#include <abaclade/vector.hxx>
#include <abaclade/byteorder.hxx>
#include <abaclade/text.hxx>
#include <abaclade/text/char_traits.hxx>
#include <abaclade/text/str_traits.hxx>
#include <abaclade/text/codepoint_iterator.hxx>
#include <abaclade/str.hxx>
#include <abaclade/text-after-str.hxx>
#include <abaclade/to_str.hxx>
#include <abaclade/from_str.hxx>

#include <abaclade/enum-after-to_str.hxx>
#include <abaclade/exception-after-to_str.hxx>
#include <abaclade/str-after-to_str.hxx>
#include <abaclade/to_str-after-str-after-to_str.hxx>
#include <abaclade/pointer_iterator-after-to_str.hxx>
#include <abaclade/text/codepoint_iterator-after-to_str.hxx>
#include <abaclade/vector-after-to_str.hxx>

#include <abaclade/os/path.hxx>
#include <abaclade/io.hxx>
#include <abaclade/io/binary.hxx>
#include <abaclade/io/binary/file.hxx>
#include <abaclade/list.hxx>
#include <abaclade/io/binary/buffered.hxx>
#include <abaclade/io/text.hxx>
#include <abaclade/io/text/binbuf.hxx>
#include <abaclade/io/text/str.hxx>

#include <abaclade/str-after-str_writer.hxx>
#include <abaclade/to_str-after-str_writer.hxx>
#include <abaclade/from_str-after-str_reader.hxx>

#include <abaclade/detail/trace.hxx>
#include <abaclade/trace.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

#undef _ABACLADE_HXX_INTERNAL
#endif //ifndef _ABACLADE_HXX
