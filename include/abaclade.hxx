/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2016 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

/*! @file
Top-level Abaclade header file; must always be included before any other Abaclade headers. */

#ifndef _ABACLADE_HXX
#define _ABACLADE_HXX
//! @cond
#define _ABACLADE_HXX_INTERNAL
//! @endcond


////////////////////////////////////////////////////////////////////////////////////////////////////

//! Abaclade’s top-level namespace.
namespace abc {
   //! Networking facilities.
   namespace net {}

   //! Support for performance tracking.
   namespace perf {}

   //! Top-level namespace for Abaclade’s testing framework.
   namespace testing {}

   namespace text {

      //! Classes implementing semi-complete parsers, ready to be built upon.
      namespace parsers {}

   } //namespace text
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <abaclade/detail/host.hxx>
//! @cond

// Host-dependent fixes.

// Make sure DEBUG and _DEBUG are coherent; DEBUG wins.
#if defined(DEBUG) && !defined(_DEBUG)
   #define _DEBUG
#elif !defined(DEBUG) && defined(_DEBUG)
   #undef _DEBUG
#endif

#include <abaclade/cpp.hxx>

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

#if ABC_HOST_STL_MSVCRT
   /* Prevent MSC headers from typedef-ining char16_t as unsigned short. This will also prevent
   char32_t from being typedef-ined to unsigned int, but we’ll do that anyway in char.hxx. */
   #define _CHAR16T
   #if ABC_HOST_CXX_MSC
      // Silence warnings from system header files.
      #pragma warning(push)
      // “'id' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'”
      #pragma warning(disable: 4668)
   #endif
#endif
#include <cstdint> // std::*int*_t
#if ABC_HOST_STL_MSVCRT && ABC_HOST_CXX_MSC
   #pragma warning(pop)
#endif
#if ABC_HOST_STL_LIBSTDCXX
   // As described in abaclade/detail/host.hxx, confirm that we’re really using GNU libstdc++.
   #undef ABC_HOST_STL_LIBSTDCXX
   #ifdef __GLIBCXX__
      /* __GLIBCXX__ is a just timestamp, not good for version checks; if compiling using G++,
      assume that the libstdc++ versions matches G++; otherwise we’ll just assume the worse and make
      it a mere 1. */
      #if ABC_HOST_CXX_GCC
         #define ABC_HOST_STL_LIBSTDCXX ABC_HOST_CXX_GCC
      #else
         #define ABC_HOST_STL_LIBSTDCXX 1
      #endif
   #else
      #define ABC_HOST_STL_LIBSTDCXX 0
   #endif
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

      /* “Unreferenced inline function has been removed” – must be disabled until the end of the
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
//! @endcond

////////////////////////////////////////////////////////////////////////////////////////////////////

// C++11 compiler features detection.

// Ensure RTTI support is enabled for MSC.
#if ABC_HOST_CXX_MSC && !defined(_CPPRTTI)
   #error "Please compile with /GR"
#endif

/*! If defined, the compiler supports defining conversion operators as explicit, to avoid executing
them implicitly (N2437). */
#if __has_feature(cxx_explicit_conversions) || ABC_HOST_CXX_GCC || ABC_HOST_CXX_MSC >= 1800
   #define ABC_CXX_EXPLICIT_CONVERSION_OPERATORS
#endif

/*! If defined, the compiler allows to delete a specific (overload of a) function, method or
constructor (N2346). */
#if __has_feature(cxx_deleted_functions) || ABC_HOST_CXX_GCC || ABC_HOST_CXX_MSC >= 1800
   #define ABC_CXX_FUNC_DELETE
#endif

//! If defined, the compiler supports the noexcept exception specification.
#if __has_feature(cxx_noexcept) || ABC_HOST_CXX_GCC
   #define ABC_CXX_NOEXCEPT
#endif

//! If defined, the compiler expects C++11 noexcept specifications for STL functions/methods.
#if __has_feature(cxx_noexcept) || ABC_HOST_CXX_GCC
   #define ABC_CXX_STL_USES_NOEXCEPT
#endif

//! If defined, the compiler supports variadic templates (N2242).
#if __has_feature(cxx_variadic_templates) || ABC_HOST_CXX_GCC || ABC_HOST_CXX_MSC >= 1800
   #define ABC_CXX_VARIADIC_TEMPLATES
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

/* Compatibility layer for features that are available one way or another in all supported
compilers. */

#include <abaclade/detail/host-cxx-compat.hxx>

// If #pragma once is supported, use it now for this file.
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

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

/*! Returns a size rounded (ceiling) to a count of _std::max_align_t units. This allows to declare
storage with alignment suitable for any type, just like std::malloc() does. Identical to
bitmanip::ceiling_to_pow2_multiple(cb, sizeof(_std::max_align_t)).

@param cb
   Size to be aligned to sizeof(_std::max_align_t).
@return
   Multiple of sizeof(_std::max_align_t) not smaller than cb.
*/
#define ABC_ALIGNED_SIZE(cb) \
   (\
      (static_cast< ::std::size_t>(cb) + sizeof(::abc::_std::max_align_t) - 1) / \
      sizeof(::abc::_std::max_align_t) \
   )

/** Returns the offset of a member in a struct/class.

@param type
   Type containing the member.
@param member
   Identifier of the last member in type.
@return
   Offset of the specified member, in bytes.
*/
#define ABC_OFFSETOF(type, member) \
   (reinterpret_cast< ::std::size_t>(&reinterpret_cast<type *>(8192)->member) - 8192)

/** Returns the size of a struct/class, without padding added.

@param type
   Type of which to calculate the exact size.
@param lastmember
   Identifier of the last member in type.
@return
   Size of type, in bytes. Guaranteed to be at most equal to sizeof(type).
*/
#if ABC_HOST_CXX_MSC
   /* MSC16/MSC18 BUG: sizeof lastmember results in C2070 “illegal sizeof operand”, so we forcibly
   qualify lastmember by pretending to retrieve it from an instance of the containing type. */
   #define ABC_UNPADDED_SIZEOF(type, lastmember) \
      (ABC_OFFSETOF(type, lastmember) + sizeof(reinterpret_cast<type *>(8192)->lastmember))
#else
   #define ABC_UNPADDED_SIZEOF(type, lastmember) \
      (ABC_OFFSETOF(type, lastmember) + sizeof lastmember)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

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

// #include other core header files that require a special order.

#if ABC_HOST_STL_MSVCRT && ABC_HOST_CXX_MSC
   // Silence warnings from system header files.
   #pragma warning(push)
   // “expression before comma has no effect; expected expression with side-effect”
   #pragma warning(disable: 4548)
   // “'id' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'”
   #pragma warning(disable: 4668)
#endif
#include <cstddef> // std::ptrdiff_t std::size_t
#if ABC_HOST_STL_MSVCRT && ABC_HOST_CXX_MSC
   #pragma warning(pop)
#endif

#include <abaclade/_std-before-noncopyable.hxx>
#include <abaclade/noncopyable.hxx>
#include <abaclade/explicit_operator_bool.hxx>
#include <abaclade/_std-before-memory.hxx>
#include <abaclade/memory.hxx>
#include <abaclade/_std.hxx>

// Forward declarations.

namespace abc { namespace collections {

/*! Abaclade’s memory-efficient vector/array class.

Instead of forcing users to choose between std::vector and fixed-size arrays, Abaclade’s vector
class employs different allocation strategies, and is able to use a dynamically-allocated and
-resized element array, while also taking advantage of an optional fixed-size array embedded into
it (when the template argument t_ciEmbeddedCapacity is provided and greater than 0).

Let’s look at the underlying data storage in different scenarios. Key:
   @verbatim
   ┌─────────────┬────────────┬─────────────┬────────────────┬──────────────────┬───────────────┐
   │ Pointer to  │ Pointer to │ P if array  │ T if array is  │ E if followed by │ D if array is │
   │ array start │ array end  │ is prefixed │ NUL-terminated │ fixed-side array │ dynamically-  │
   │             │            │             │                │                  │ allocated     │
   └─────────────┴────────────┴─────────────┴────────────────┴──────────────────┴───────────────┘
   @endverbatim
Additionally, this may be following the above in case t_ciEmbeddedCapacity is greater than 0:
   @verbatim
   ┌────────────────────┬─────────────────┐
   │ Count of slots     │ Array contents… │
   │ (bytes in reality) │                 │
   └────────────────────┴─────────────────┘
   @endverbatim

1. vector<int> v1(): no element array.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┐
   │ nullptr │ nullptr │ - │ - │ - │
   └─────────┴─────────┴───┴───┴───┘
   @endverbatim

2. vector<int, 2> v2(): has an embedded prefixed fixed-size array, but does not use it yet.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───╥───┬─────┐
   │ nullptr │ nullptr │ - │ E │ - ║ 2 │ - - │
   └─────────┴─────────┴───┴───┴───╨───┴─────┘
   @endverbatim

3. v1.push_back(1): switches to a dynamically-allocated prefixed array to perform the insertion.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┐           ┌───┬─────────────────┐
   │ 0xptr   │ 0xptr   │ P │ - │ D │           │ 8 │ 1 - - - - - - - │
   └─────────┴─────────┴───┴───┴───┘           └───┴─────────────────┘
     │         │                                   ▲   ▲
     │         └───────────────────────────────────│───┘
     └─────────────────────────────────────────────┘
   @endverbatim

4. v2.push_back(1), v2.push_back(2): starts using the embedded prefixed fixed-size array to perform
   the insertions.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───╥───┬─────┐
   │ 0xptr   │ 0xptr   │ P │ E │ - ║ 2 │ 1 2 │
   └─────────┴─────────┴───┴───┴───╨───┴─────┘
     │         │                       ▲     ▲
     │         └───────────────────────│─────┘
     └─────────────────────────────────┘
   @endverbatim

5. v2.push_back(1): switches to a dynamically-allocated prefixed array to perform an insertion that
   requires more space than the embedded fixed-size array allows to store.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───╥───┬─────┐ ┌───┬─────────────────┐
   │ 0xptr   │ 0xptr   │ P │ - │ D ║ 2 │ - - │ │ 8 │ 1 2 3 - - - - - │
   └─────────┴─────────┴───┴───┴───╨───┴─────┘ └───┴─────────────────┘
     │         │                                   ▲       ▲
     │         └───────────────────────────────────│───────┘
     └─────────────────────────────────────────────┘
   @endverbatim

See abc::collections::detail::vextr_impl_base for more implementation details. */
template <typename T, std::size_t t_ciEmbeddedCapacity = 0>
class vector;

}} //namespace abc::collections

namespace abc { namespace io { namespace text {

class char_ptr_ostream;
class ostream;

}}} //namespace abc::io::text

namespace abc { namespace text {

/*! abc::text::str subclass that includes a fixed-size character array.

This class offers clients the option to avoid dynamic memory allocation when then strings to be
manipulated are expected to be of a known small size. If the string expands to more than what the
fixed-size character array can hold, the object will seamlessly switch to behaving like
abc::text::str.

The abc::text::sstr class derives from abc::text::str, but the inheritance is private to prevent
sstr instances from being accidentally passed as abc::text::str && arguments, which would be a
problem since moving an sstr object to a str object may throw exceptions due to potentially having
to allocate dynamic memory.

To enable using an sstr object as a str instance, the accessor methods sstr::str() and
sstr::str_ptr() are provided. Note that while either method may be indirectly used to get an r-value
reference to the object as an abc::text::str instance, doing so is discouraged, for the reasons
explained above. */
template <std::size_t t_cchEmbeddedCapacity>
class sstr;

/*! Abaclade’s memory-efficient string class.

Instead of forcing users to choose between std::string and fixed-size buffers, Abaclade’s string
class employs different allocation strategies, and is able to use a dynamically-allocated and
-resized character array, while also taking advantage of an optional fixed-size array embedded into
it (see the abc::text::sstr subclass).

Unlike C or STL strings, instances do not implicitly have an accessible trailing NUL character; to
get a pointer to a NUL-terminated string, use abc::text::str::c_str().

Let’s look at the underlying data storage in different scenarios.
These are the fields in an abc::text::str instance:
   @verbatim
   ┌─────────────┬────────────┬─────────────┬────────────────┬──────────────────┬───────────────┐
   │ Pointer to  │ Pointer to │ P if array  │ T if array is  │ E if followed by │ D if array is │
   │ array start │ array end  │ is prefixed │ NUL-terminated │ fixed-side array │ dynamically-  │
   │             │            │             │                │                  │ allocated     │
   └─────────────┴────────────┴─────────────┴────────────────┴──────────────────┴───────────────┘
   @endverbatim
Additionally, this may be following the above in case of an abc::text::sstr instance, which has an
embedded fixed-side character array:
   @verbatim
   ┌────────────────────┬─────────────────┐
   │ Count of slots     │ Array contents… │
   │ (bytes in reality) │                 │
   └────────────────────┴─────────────────┘
   @endverbatim

1. str s1(): no character array.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┬───┐
   │ nullptr │ nullptr │ - │ - │ - │ - │
   └─────────┴─────────┴───┴───┴───┴───┘
   @endverbatim

2. sstr<4> s2(): has an embedded prefixed fixed-size array, but does not use it yet.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┬───╥───┬─────────┐
   │ nullptr │ nullptr │ - │ - │ E │ - ║ 4 │ - - - - │
   └─────────┴─────────┴───┴───┴───┴───╨───┴─────────┘
   @endverbatim

3. str s3("abc"): points to a non-prefixed character array in read-only memory, which also has a NUL
   terminator.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┬───┐                    ┌──────────┐
   │ 0xptr   │ 0xptr   │ - │ T │ - │ - │                    │ a b c \0 │
   └─────────┴─────────┴───┴───┴───┴───┘                    └──────────┘
     │         │                                            ▲       ▲
     │         └────────────────────────────────────────────│───────┘
     └──────────────────────────────────────────────────────┘
   @endverbatim

4. s3 += "def": switches to a dynamically-allocated prefixed array to perform the concatenation.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┬───┐                ┌───┬─────────────────┐
   │ 0xptr   │ 0xptr   │ P │ - │ - │ D │                │ 8 │ a b c d e f - - │
   └─────────┴─────────┴───┴───┴───┴───┘                └───┴─────────────────┘
     │         │                                            ▲             ▲
     │         └────────────────────────────────────────────│─────────────┘
     └──────────────────────────────────────────────────────┘
   @endverbatim

5. s2 += "abc": starts using the embedded prefixed fixed-size array to perform the concatenation.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┬───╥───┬─────────┐
   │ 0xptr   │ 0xptr   │ P │ - │ E │ - ║ 4 │ a b c - │
   └─────────┴─────────┴───┴───┴───┴───╨───┴─────────┘
     │         │                           ▲       ▲
     │         └───────────────────────────│───────┘
     └─────────────────────────────────────┘
   @endverbatim

7. s2 += "def": switches to a dynamically-allocated prefixed array because the embedded one is not
   large enough.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┬───╥───┬─────────┐  ┌───┬─────────────────┐
   │ 0xptr   │ 0xptr   │ P │ - │ E │ D ║ 4 │ - - - - │  │ 8 │ a b c d e f - - │
   └─────────┴─────────┴───┴───┴───┴───╨───┴─────────┘  └───┴─────────────────┘
     │         │                                            ▲             ▲
     │         └────────────────────────────────────────────│─────────────┘
     └──────────────────────────────────────────────────────┘
   @endverbatim

See abc::collections::detail::vextr_impl_base for more implementation details. */
typedef sstr<0> str;

}} //namespace abc::text

namespace abc {

using text::char_t;
using text::str;
using text::sstr;

} //namespace abc

#include <abaclade/collections/static_list.hxx>
#include <abaclade/detail/context_local.hxx>
#include <abaclade/coroutine_local.hxx>
#include <abaclade/thread_local.hxx>
#include <abaclade/enum.hxx>
#include <abaclade/text-before-exception-and-str.hxx>
#include <abaclade/exception.hxx>
#include <abaclade/memory-after-exception.hxx>
#include <abaclade/collections/detail/vextr_impl.hxx>
#include <abaclade/text/char_traits.hxx>
#include <abaclade/text/str_traits.hxx>
#include <abaclade/text/str.hxx>
#include <abaclade/to_str.hxx>
#include <abaclade/from_str.hxx>

#include <abaclade/enum-after-to_str.hxx>
#include <abaclade/text/str-after-to_str.hxx>
#include <abaclade/to_str-after-str-after-to_str.hxx>
#include <abaclade/exception-after-to_str.hxx>
#include <abaclade/io/text/stream-istream-ostream.hxx>
#include <abaclade/io/text/str.hxx>

#include <abaclade/text/str-after-str_ostream.hxx>
#include <abaclade/to_str-after-str_ostream.hxx>
#include <abaclade/from_str-after-str_istream.hxx>

#include <abaclade/detail/trace.hxx>
#include <abaclade/trace.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

#undef _ABACLADE_HXX_INTERNAL
#endif //ifndef _ABACLADE_HXX
