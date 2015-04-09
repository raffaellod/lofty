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

   //! Templated container data structures.
   namespace collections {}

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

   //! Networking facilities.
   namespace net {}

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
// abc globals – ABC_HOST_*

#include <abaclade/detail/host.hxx>

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
   /* “'class1 member' : class 'template class2' needs to have dll-interface to be used by clients
   of class 'class1'” */
   #pragma warning(disable: 4251)
   // “'class' : class has virtual functions, but destructor is not virtual”
   #pragma warning(disable: 4265)
   // “non dll-interface class 'base_class' used as base for dll-interface class 'derived_class'”
   #pragma warning(disable: 4275)
   // “C++ exception specification ignored except to indicate a function is not __declspec(nothrow)”
   #pragma warning(disable: 4290)
   // “cast truncates constant value”: would be useful, but it’s raised too easily by MSC16.
   #pragma warning(disable: 4310)
   /* “behavior change : 'stl_internal1' called instead of 'stl_internal2': this is raised by MSC’s
   STL header files, and Microsoft suggests to just ignore it; see <https://connect.microsoft.com/
   VisualStudio/feedback/details/767960/warning-c4350-behavior-change-when-including-string-and-no-
   precompiled-header>. */
   #pragma warning(disable: 4350)
   #if ABC_HOST_CXX_MSC >= 1700
      /* “'derived_class' : Object layout under / vd2 will change due to virtual base 'base_class'”:
      yet another problem related to calling virtual methods from a constructor. This warning could
      be used to detect the latter situation, but MSC raises it unconditionally, so just turn it
      off. */
      #pragma warning(disable: 4435)
   #endif
   // “'class' : default constructor could not be generated”
   #pragma warning(disable: 4510)
   // “'class' : assignment operator could not be generated”
   #pragma warning(disable: 4512)
   // “class 'class' can never be instantiated - user defined constructor required”
   #pragma warning(disable: 4610)
   /* “'class' : copy constructor could not be generated because a base class copy constructor is
   inaccessible” */
   #pragma warning(disable: 4625)
   /* “'class' : assignment operator could not be generated because a base class assignment operator
   is inaccessible” */
   #pragma warning(disable: 4626)
   /* “throwing 'abc::_exception_aggregator<TAbc>' the following types will not be considered at the
   catch site” */
   #pragma warning(disable: 4673)
   /* “potentially uninitialized local variable 'var' used”: would be useful, but it’s raised too
   easily by MSC16. */
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

#if ABC_HOST_API_POSIX
   // Enable 64-bit offsets in file functions, and prevent stat() from failing for 2+ GiB files.
   #define _FILE_OFFSET_BITS 64
#elif ABC_HOST_API_WIN32 && ABC_HOST_ARCH_ARM
   /* Prevent crtdefs.h from raising #error “Compiling Desktop applications for the ARM platform is
   not supported.”, which seems to be an artificial restriction added to the SDK files to match the
   fact that Microsoft ended up requiring desktop apps to be digitally signed in order to be run on
   Windows RT (“Windows on ARM”, WOA). */
   #define _ARM_WINAPI_PARTITION_DESKTOP_SDK_AVAILABLE 1
#endif

#if ABC_HOST_CXX_MSC
   /* Prevent MSC headers from typedef-ining char16_t as unsigned short. This will also prevent
   char32_t from being typedef-ined to unsigned int, but we’ll do that anyway in char.hxx. */
   #define _CHAR16T

   // Silence warnings from system header files.
   #pragma warning(push)
   // “'id' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'”
   #pragma warning(disable: 4668)
#endif
#include <cstdint> // std::*int*_t
#if ABC_HOST_CXX_MSC
   #pragma warning(pop)
#endif
// Under Win32, this also defines char16_t to be wchar_t, which is quite appropriate.
#include <abaclade/text/char.hxx>

#if ABC_HOST_API_WIN32
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
#endif //if ABC_HOST_API_WIN32

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals – C++11 compiler features detection

// Ensure RTTI support is enabled for MSC.
#if ABC_HOST_CXX_MSC && !defined(_CPPRTTI)
   #error "Please compile with /GR"
#endif

/*! If defined, the compiler supports defining conversion operators as explicit, to avoid executing
them implicitly (N2437). */
#if (ABC_HOST_CXX_CLANG && __has_feature(cxx_explicit_conversions)) || \
      ABC_HOST_CXX_GCC >= 40500 || ABC_HOST_CXX_MSC >= 1800
   #define ABC_CXX_EXPLICIT_CONVERSION_OPERATORS
#endif

/*! If defined, the compiler allows to delete a specific (overload of a) function, method or
constructor (N2346). */
#if (ABC_HOST_CXX_CLANG && __has_feature(cxx_deleted_functions)) || ABC_HOST_CXX_GCC >= 40400 || \
      ABC_HOST_CXX_MSC >= 1800
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
#if (ABC_HOST_CXX_CLANG && __has_feature(cxx_variadic_templates)) || ABC_HOST_CXX_GCC || \
      ABC_HOST_CXX_MSC >= 1800
   #define ABC_CXX_VARIADIC_TEMPLATES
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals – compatibility layer for features that are available one way or another in all
// supported compilers

#include <abaclade/detail/host-cxx-compat.hxx>

// If #pragma once is supported, use it now for this file.
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
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

// This will also #include <type_traits> .
#include <abaclade/noncopyable.hxx>
#include <abaclade/explicit_operator_bool.hxx>

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
// #include other core header files that require a special order

#if ABC_HOST_CXX_MSC
   // Silence warnings from system header files.
   #pragma warning(push)
   // “expression before comma has no effect; expected expression with side-effect”
   #pragma warning(disable: 4548)
   // “'id' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'”
   #pragma warning(disable: 4668)
#endif //if ABC_HOST_CXX_MSC
#include <cstddef> // std::ptrdiff_t std::size_t
#if ABC_HOST_CXX_MSC
   #pragma warning(pop)
#endif

#if defined(ABC_STLIMPL) || !defined(ABC_CXX_VARIADIC_TEMPLATES)
   #include <abaclade/_std/tuple.hxx>
#endif
#ifdef ABC_STLIMPL
   #include <abaclade/_std/memory.hxx>
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

namespace text {

class dmstr;
class istr;
class mstr;
template <std::size_t t_cchEmbeddedCapacity>
class smstr;

} //namespace text

using text::char_t;
using text::dmstr;
using text::istr;
using text::mstr;
using text::smstr;

namespace io {
namespace text {

class writer;

} //namespace text
} //namespace io

} //namespace abc

#ifdef ABC_STLIMPL
   #include <abaclade/_std/exception.hxx>
   #include <abaclade/_std/functional.hxx>
   #include <abaclade/_std/iterator.hxx>
#else
   #include <exception>
   #include <functional>
   #include <iterator>
#endif
#include <abaclade/collections/detail/xor_list.hxx>
#include <abaclade/collections/static_list.hxx>
#include <abaclade/detail/context_local.hxx>
#include <abaclade/thread_local.hxx>
#include <abaclade/coroutine_local.hxx>
#include <abaclade/enum.hxx>
#include <abaclade/exception.hxx>
#include <abaclade/collections/pointer_iterator.hxx>
#include <abaclade/collections/detail/type_void_adapter.hxx>
#include <abaclade/collections/detail/vextr.hxx>
#include <abaclade/collections/vector.hxx>
#include <abaclade/text.hxx>
#include <abaclade/text/char_traits.hxx>
#include <abaclade/text/str_traits.hxx>
#include <abaclade/text/codepoint_iterator.hxx>
#include <abaclade/text/str.hxx>
#include <abaclade/text-after-str.hxx>
#include <abaclade/to_str.hxx>
#include <abaclade/from_str.hxx>

#include <abaclade/enum-after-to_str.hxx>
#include <abaclade/exception-after-to_str.hxx>
#include <abaclade/text/str-after-to_str.hxx>
#include <abaclade/to_str-after-str-after-to_str.hxx>
#include <abaclade/collections/pointer_iterator-after-to_str.hxx>
#include <abaclade/text/codepoint_iterator-after-to_str.hxx>
#include <abaclade/collections/vector-after-to_str.hxx>

#include <abaclade/os/path.hxx>
#include <abaclade/io.hxx>
#include <abaclade/io/binary.hxx>
#include <abaclade/io/binary/file.hxx>
#include <abaclade/collections/list.hxx>
#include <abaclade/io/binary/buffered.hxx>
#include <abaclade/io/text.hxx>
#include <abaclade/io/text/binbuf.hxx>
#include <abaclade/io/text/str.hxx>

#include <abaclade/text/str-after-str_writer.hxx>
#include <abaclade/to_str-after-str_writer.hxx>
#include <abaclade/from_str-after-str_reader.hxx>

#include <abaclade/detail/trace.hxx>
#include <abaclade/trace.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

#undef _ABACLADE_HXX_INTERNAL
#endif //ifndef _ABACLADE_HXX
