/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

/*! @file
Detects host compiler, host operating system, and other host features; it also provides some bits of
compatibility to make the life of the rest of Lofty easier. */

#ifndef _LOFTY_HOST_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_HOST_HXX
#endif

#ifndef _LOFTY_HOST_HXX_NOPUB
#define _LOFTY_HOST_HXX_NOPUB

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// C++ compiler detection.

//! Version of Clang if building with it, or 0 otherwise.
#define LOFTY_HOST_CXX_CLANG 0
//! Version of GCC if building with it, or 0 otherwise.
#define LOFTY_HOST_CXX_GCC 0
//! Version of MSC if building with it, or 0 otherwise.
#define LOFTY_HOST_CXX_MSC 0

#if defined(__clang__)
   #undef LOFTY_HOST_CXX_CLANG
   #define LOFTY_HOST_CXX_CLANG \
      (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#elif defined(__GNUC__)
   #undef LOFTY_HOST_CXX_GCC
   #define LOFTY_HOST_CXX_GCC \
      (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
   #if LOFTY_HOST_CXX_GCC < 40700
      #error "Unsupported version of GCC: >= 4.7 required"
   #endif
#elif defined(_MSC_VER)
   #undef LOFTY_HOST_CXX_MSC
   #define LOFTY_HOST_CXX_MSC _MSC_VER
   #if LOFTY_HOST_CXX_MSC < 1600
      #error "Unsupported version of MSC: >= MSC 16 / VC++ 10 / VS 2010 required"
   #endif
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// C++ STL detection.

//! 1 if building with Lofty’s STL subset implementation, or 0 otherwise.
#define LOFTY_HOST_STL_LOFTY 0
//! Version of Clang’s libc++ STL implementation if building against it, or 0 otherwise.
#define LOFTY_HOST_STL_LIBCXX 0
//! Version of GNU libstdc++ STL implementation if building against it, or 0 otherwise.
#define LOFTY_HOST_STL_LIBSTDCXX 0
//! Version of MSVCRT STL implementation if building against it, or 0 otherwise.
#define LOFTY_HOST_STL_MSVCRT 0

#ifdef _LOFTY_USE_STLIMPL
   #undef LOFTY_HOST_STL_LOFTY
   #define LOFTY_HOST_STL_LOFTY 1
#elif LOFTY_HOST_CXX_CLANG || LOFTY_HOST_CXX_GCC
   #if LOFTY_HOST_CXX_CLANG
      #if __has_include(<__config>)
         #include <__config>
      #endif
   #endif
   #ifdef _LIBCPP_VERSION
      #undef LOFTY_HOST_STL_LIBCXX
      #define LOFTY_HOST_STL_LIBCXX _LIBCPP_VERSION
   #else
      /* Not libc++; assume we’re using GNU libstdc++. This will be confirmed after we #include <cstdint> in
      lofty.hxx. */
      #undef LOFTY_HOST_STL_LIBSTDCXX
      #define LOFTY_HOST_STL_LIBSTDCXX 1
   #endif
#elif LOFTY_HOST_CXX_MSC
   // Assume the version of MSVCRT matches that of MSC.
   #undef LOFTY_HOST_STL_MSVCRT
   #define LOFTY_HOST_STL_MSVCRT LOFTY_HOST_CXX_MSC
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// OS API detection.

//! 1 if building for a BSD-like OS, or 0 otherwise. Implies LOFTY_HOST_API_POSIX.
#define LOFTY_HOST_API_BSD 0
/*! 1 if building for a Darwin-based OS such as macOS or iOS, or 0 otherwise. Implies LOFTY_HOST_API_BSD,
LOFTY_HOST_API_MACH, LOFTY_HOST_API_POSIX. */
#define LOFTY_HOST_API_DARWIN 0
//! 1 if building for FreeBSD, or 0 otherwise. Implies LOFTY_HOST_API_BSD, LOFTY_HOST_API_POSIX.
#define LOFTY_HOST_API_FREEBSD 0
//! 1 if building for Linux (the kernel), or 0 otherwise. Implies LOFTY_HOST_API_POSIX.
#define LOFTY_HOST_API_LINUX 0
//! 1 if building for a POSIX-compatible OS, or 0 otherwise.
#define LOFTY_HOST_API_POSIX 0
//! 1 if building for a Mach-based OS, or 0 otherwise.
#define LOFTY_HOST_API_MACH 0
//! 1 if building for the Win32 API, or 0 otherwise.
#define LOFTY_HOST_API_WIN32 0
//! 1 if building for the 64-bit variant of the Win32 API, or 0 otherwise. Implies LOFTY_HOST_API_WIN32.
#define LOFTY_HOST_API_WIN64 0

#if defined(_WIN32)
   // Compiling for Win32 or Win64.
   #undef LOFTY_HOST_API_WIN32
   #define LOFTY_HOST_API_WIN32 1
   #ifdef _WIN64
      // Compiling for Win64 (coexists with LOFTY_HOST_API_WIN32).
      #undef LOFTY_HOST_API_WIN64
      #define LOFTY_HOST_API_WIN64 1
   #endif
#elif defined(__linux__)
   // Compiling for GNU/Linux.
   #undef LOFTY_HOST_API_LINUX
   #define LOFTY_HOST_API_LINUX 1
   #undef LOFTY_HOST_API_POSIX
   #define LOFTY_HOST_API_POSIX 1
#elif defined(__MACH__) && defined(__APPLE__)
   /* Compiling for Darwin/XNU (commercially known as macOS and iOS), mostly compatible with BSD libc but
   using a Mach kernel. */
   #undef LOFTY_HOST_API_BSD
   #define LOFTY_HOST_API_BSD 1
   #undef LOFTY_HOST_API_DARWIN
   #define LOFTY_HOST_API_DARWIN 1
   #undef LOFTY_HOST_API_MACH
   #define LOFTY_HOST_API_MACH 1
   #undef LOFTY_HOST_API_POSIX
   #define LOFTY_HOST_API_POSIX 1
#elif defined(__unix__)
   #ifdef __FreeBSD__
      // Compiling for FreeBSD.
      #undef LOFTY_HOST_API_BSD
      #define LOFTY_HOST_API_BSD 1
      #undef LOFTY_HOST_API_FREEBSD
      #define LOFTY_HOST_API_FREEBSD 1
   #endif

   // In any case, approximate UNIX as POSIX.
   #undef LOFTY_HOST_API_POSIX
   #define LOFTY_HOST_API_POSIX 1
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// System architecture detection.

//! 1 if building for the Alpha architecture, or 0 otherwise.
#define LOFTY_HOST_ARCH_ALPHA 0
//! 1 if building for the ARM architecture, or 0 otherwise.
#define LOFTY_HOST_ARCH_ARM 0
//! 1 if building for the i386 architecture also known as x86, or 0 otherwise.
#define LOFTY_HOST_ARCH_I386 0
//! 1 if building for the IA64 architecture, or 0 otherwise.
#define LOFTY_HOST_ARCH_IA64 0
//! 1 if building for the PowerPC architecture, or 0 otherwise.
#define LOFTY_HOST_ARCH_PPC 0
//! 1 if building for the x86-64 architecture also known as AMD64, or 0 otherwise.
#define LOFTY_HOST_ARCH_X86_64 0

#if defined(__alpha__) || defined(_M_ALPHA)
   #undef LOFTY_HOST_ARCH_ALPHA
   #define LOFTY_HOST_ARCH_ALPHA 1
#elif defined(__arm__) || defined(_M_ARM)
   #undef LOFTY_HOST_ARCH_ARM
   #define LOFTY_HOST_ARCH_ARM 1
#elif defined(__i386__) || defined(_M_IX86)
   #undef LOFTY_HOST_ARCH_I386
   #define LOFTY_HOST_ARCH_I386 1
#elif defined(__ia64__) || defined(_M_IA64)
   #undef LOFTY_HOST_ARCH_IA64
   #define LOFTY_HOST_ARCH_IA64 1
#elif defined(__powerpc__) || defined(_M_PPC)
   #undef LOFTY_HOST_ARCH_PPC
   #define LOFTY_HOST_ARCH_PPC 1
#elif defined(__x86_64__) || defined(__amd64__) || defined(_M_X64) || defined(_M_AMD64)
   #undef LOFTY_HOST_ARCH_X86_64
   #define LOFTY_HOST_ARCH_X86_64 1
#else
   #error "TODO: HOST_ARCH"
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! Machine word size for this microarchitecture.
// TODO: the word/pointer size is much more easily detected by a configure program.
#if LOFTY_HOST_API_WIN64
   #define LOFTY_HOST_WORD_SIZE 64
#elif LOFTY_HOST_API_WIN32
   #define LOFTY_HOST_WORD_SIZE 32
#elif defined(__SIZEOF_POINTER__)
   #define LOFTY_HOST_WORD_SIZE (__SIZEOF_POINTER__ * 8)
#else
   #error "TODO: HOST_ARCH"
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! 1 if building for a little-endian processor architecture, or 0 otherwise. Implies !LOFTY_HOST_BIG_ENDIAN.
#define LOFTY_HOST_LITTLE_ENDIAN 0
//! 1 if building for a big-endian processor architecture, or 0 otherwise. Implies !LOFTY_HOST_LITTLE_ENDIAN.
#define LOFTY_HOST_BIG_ENDIAN 0

// Assume that ARM is always used in little-endian mode.
#if LOFTY_HOST_ARCH_ALPHA || LOFTY_HOST_ARCH_ARM || LOFTY_HOST_ARCH_I386 || LOFTY_HOST_ARCH_IA64 || \
      LOFTY_HOST_ARCH_X86_64
   #undef LOFTY_HOST_LITTLE_ENDIAN
   #define LOFTY_HOST_LITTLE_ENDIAN 1
#elif LOFTY_HOST_ARCH_PPC
   #undef LOFTY_HOST_BIG_ENDIAN
   #define LOFTY_HOST_BIG_ENDIAN 1
#else
   #error "TODO: HOST_ARCH"
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Apply fixes depending on LOFTY_HOST_CXX_*.

// Compatibility with compilers that don’t support feature/extension checking.
#ifndef __has_extension
   #define __has_extension(x) 0
#endif
#ifndef __has_feature
   #define __has_feature(x) 0
#endif

#if LOFTY_HOST_CXX_MSC
   // Suppress unnecessary warnings.

   // “enumerator 'name' in switch of enum 'type' is not explicitly handled by a case label”
   #pragma warning(disable: 4061)
   // “enumerator 'name' in switch of enum 'type' is not handled”
   #pragma warning(disable: 4062)
   // “conditional expression is constant”
   #pragma warning(disable: 4127)
   /* “nonstandard extension used : 'initializing' : conversion from 'type' to 'type &' A non-const reference
   may only be bound to an lvalue”: raised by LOFTY_FOR_EACH() when the iterated expression is not an l-value.
   No other compiler complains about it. */
   #pragma warning(disable: 4239)
   // “'class' : inherits 'base::member' via dominance”: it points out the obvious and intended.
   #pragma warning(disable: 4250)
   /* “'class1 member' : class 'template class2' needs to have dll-interface to be used by clients of class
   'class1'”: not sure why, but can’t imagine how it could possibly make sense to DLL-export a class
   template. */
   #pragma warning(disable: 4251)
   // “'class' : class has virtual functions, but destructor is not virtual”
   #pragma warning(disable: 4265)
   // “non dll-interface class 'base_class' used as base for dll-interface class 'derived_class'”
   #pragma warning(disable: 4275)
   // “C++ exception specification ignored except to indicate a function is not __declspec(nothrow)”
   #pragma warning(disable: 4290)
   // “cast truncates constant value”: would be useful, but it’s raised too easily by MSC16.
   #pragma warning(disable: 4310)
   /* “behavior change : 'stl_internal1' called instead of 'stl_internal2': this is raised by MSC’s STL header
   files, and Microsoft suggests to just ignore it; see <https://connect.microsoft.com/VisualStudio/feedback/
   details/767960/warning-c4350-behavior-change-when-including-string-and-no-precompiled-header>. */
   #pragma warning(disable: 4350)
   #if LOFTY_HOST_CXX_MSC >= 1700
      /* “'derived_class' : Object layout under /vd2 will change due to virtual base 'base_class'”: yet
      another problem related to calling virtual methods from a constructor. This warning could be used to
      detect the latter situation, but MSC raises it unconditionally, so just turn it off. */
      #pragma warning(disable: 4435)
      /* “dynamic_cast from virtual base 'base_class' to 'derived_class' could fail in some contexts”: only
      really applies if the code using dynamic_cast gets called on this in a constructor or destructor. This
      warning could be used to detect real errors, but MSC raises it unconditionally, so just turn it off.*/
      #pragma warning(disable: 4437)
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
   /* “'class' : assignment operator could not be generated because a base class assignment operator is
   inaccessible” */
   #pragma warning(disable: 4626)
   /* “potentially uninitialized local variable 'var' used”: would be useful, but it’s raised too easily by
   MSC16. */
   #pragma warning(disable: 4701)
   // “'function' : function not inlined”
   #pragma warning(disable: 4710)
   // “function 'function' selected for automatic inline expansion”
   #pragma warning(disable: 4711)
   // “'struct' : 'n' bytes padding added after data member 'member'”
   #pragma warning(disable: 4820)
   #if LOFTY_HOST_CXX_MSC < 1700
      /* “nonstandard extension used : 'type' : local types or unnamed types cannot be used as template
      arguments”. */
      #pragma warning(disable: 4836)
   #endif
#endif //if LOFTY_HOST_CXX_MSC

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Unicode support.

/*! Indicates the level of UTF-8 string literals support:
•  2 - The UTF-8 string literal prefix (u8) is supported;
•  1 - The u8 prefix is not supported, but the compiler will generate valid UTF-8 string literals if the
       source file is UTF-8+BOM-encoded;
•  0 - UTF-8 string literals are not supported in any way.
*/
#define LOFTY_CXX_UTF8LIT 0

/*! Indicates how char16_t is defined:
•  2 - char16_t is a native type of its own;
•  1 - char16_t is a typedef for the native 16-bit wchar_t;
•  0 - char16_t is a typedef for a regular 16-bit unsigned integer type.
*/
#define LOFTY_CXX_CHAR16 0

/*! Indicates how char32_t is defined:
•  2 - char32_t is a native type of its own;
•  1 - char32_t is a typedef for the native 32-bit wchar_t;
•  0 - char32_t is a typedef for a regular 32-bit unsigned integer type.
*/
#define LOFTY_CXX_CHAR32 0

//! @cond

// Make sure UNICODE and _UNICODE are set if either is.
#if defined(UNICODE) && !defined(_UNICODE)
   #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
   #define _UNICODE
#endif

//! @endcond

#if LOFTY_HOST_CXX_CLANG || LOFTY_HOST_CXX_GCC 
   // char16_t is a native type, different than std::uint16_t.
   #undef LOFTY_CXX_CHAR16
   #define LOFTY_CXX_CHAR16 2
   // char32_t is a native type, different than std::uint32_t.
   #undef LOFTY_CXX_CHAR32
   #define LOFTY_CXX_CHAR32 2

   #if __has_feature(cxx_unicode_literals) || LOFTY_HOST_CXX_GCC
      // UTF-8 string literals are supported.
      #undef LOFTY_CXX_UTF8LIT
      #define LOFTY_CXX_UTF8LIT 2
   #endif
#else
   #if LOFTY_HOST_CXX_MSC
      #if !defined(_WCHAR_T_DEFINED) || !defined(_NATIVE_WCHAR_T_DEFINED)
         #error "Please compile with /Zc:wchar_t"
      #endif

      // char16_t is not a native type, but we can typedef it as wchar_t.
      #undef LOFTY_CXX_CHAR16
      #define LOFTY_CXX_CHAR16 1
   #else
      /* MSC16 (handled above) will transcode non-wchar_t string literals into whatever single-byte encoding
      is selected for the user running cl.exe; a solution has been provided in form of a hotfix
      (<http://support.microsoft.com/kb/2284668/en-us>), but it no longer seems available, and it was not
      ported to MSC17/VS2012, thought it seems it was finally built into MSC18/VS2013
      (<http://connect.microsoft.com/VisualStudio/feedback/details/773186/pragma-execution-character-set-utf-
      8-didnt-support-in-vc-2012>).

      Here we assume that no other compiler exhibits such a random behavior, and they will all emit valid
      UTF-8 string literals if the source file is UTF-8+BOM-encoded. */
      #undef LOFTY_CXX_UTF8LIT
      #define LOFTY_CXX_UTF8LIT 1

      // char32_t is not a native type, but we can typedef it as wchar_t.
      #undef LOFTY_CXX_CHAR32
      #define LOFTY_CXX_CHAR32 1
   #endif
#endif
#if LOFTY_CXX_CHAR16 == 0 && LOFTY_CXX_CHAR32 == 0
   #error "LOFTY_CXX_CHAR16 and/or LOFTY_CXX_CHAR32 must be > 0; please fix detection logic"
#endif

//! UTF-* encoding supported by the host.
#if LOFTY_HOST_API_WIN32 && defined(UNICODE)
   #define LOFTY_HOST_UTF 16
#else
   #define LOFTY_HOST_UTF 8
#endif

// Note that here we don’t use std::uint*_t because we can’t pull in <cstdint> yet.

//! UTF-8 character type.
typedef char char8_t;

//! UTF-16 character type.
#if LOFTY_CXX_CHAR16 == 1
   typedef wchar_t char16_t;
#elif LOFTY_CXX_CHAR16 == 0
   // unsigned short is 16-bit wide on all supported platforms.
   typedef unsigned short char16_t;
#endif

//! UTF-32 character type.
#if LOFTY_CXX_CHAR32 == 1
   typedef wchar_t char32_t;
#elif LOFTY_CXX_CHAR32 == 0
   // unsigned short is 32-bit wide on all supported platforms.
   typedef unsigned int char32_t;
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Apply fixes depending on LOFTY_HOST_API_* and LOFTY_HOST_STL_*.

#if LOFTY_HOST_API_DARWIN
   #define _XOPEN_SOURCE
#endif

#if LOFTY_HOST_API_POSIX
   // Enable 64-bit offsets in file functions, and prevent stat() from failing for 2+ GiB files.
   #define _FILE_OFFSET_BITS 64
#elif LOFTY_HOST_API_WIN32 && LOFTY_HOST_ARCH_ARM
   /* Prevent crtdefs.h from raising #error “Compiling Desktop applications for the ARM platform is not
   supported.”, which seems to be an artificial restriction added to the SDK files to match the fact that
   Microsoft ended up requiring desktop apps to be digitally signed in order to be run on Windows RT (“Windows
   on ARM”, WoA). */
   #define _ARM_WINAPI_PARTITION_DESKTOP_SDK_AVAILABLE 1
#endif

#if LOFTY_HOST_STL_MSVCRT
   /* Prevent MSC headers from typedef-ining char16_t as unsigned short (which is not what we defined it to,
   above) and char32_t as unsigned int (which would be the same as what we did, but we can’t exclude only
   char16_t). */
   #define _CHAR16T
   #if LOFTY_HOST_CXX_MSC
      // Silence warnings from system header files.
      #pragma warning(push)
      // “'id' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'”
      #pragma warning(disable: 4668)
   #endif
#endif
#include <cstdint> // std::*int*_t
#if LOFTY_HOST_STL_MSVCRT && LOFTY_HOST_CXX_MSC
   #pragma warning(pop)
#endif

#if LOFTY_HOST_STL_LIBSTDCXX
   // Confirm that we’re really using GNU libstdc++.
   #undef LOFTY_HOST_STL_LIBSTDCXX
   #ifdef __GLIBCXX__
      /* __GLIBCXX__ is a just timestamp, not good for version checks; if compiling using G++, assume that the
      libstdc++ versions matches G++; otherwise we’ll just assume the worse and make it a mere 1. */
      #if LOFTY_HOST_CXX_GCC
         #define LOFTY_HOST_STL_LIBSTDCXX LOFTY_HOST_CXX_GCC
      #else
         #define LOFTY_HOST_STL_LIBSTDCXX 1
      #endif
   #else
      #define LOFTY_HOST_STL_LIBSTDCXX 0
   #endif
#endif

#if LOFTY_HOST_API_WIN32
   // Make sure WINVER is defined.
   #ifndef WINVER
      // Pick a default Windows version.
      #if LOFTY_HOST_API_WIN64
         // The earliest Win64 implementations are Windows Server 2003 (5.2) and Windows XP x64 Edition (5.2).
         #define WINVER 0x0502
      #else
         // The earliest supported Win32 implementations are Windows 95 (4.0) and Windows NT 4 (4.0).
         #define WINVER 0x0400
      #endif
   #endif

   /* Make sure _WIN32_WINNT is defined for WINVER values representing NT-only Windows versions. The first
   NT-only version of Windows is 5.0 (Windows 2000; Windows Me is 4.9). */
   #if !defined(_WIN32_WINNT) && WINVER >= 0x0500
      #define _WIN32_WINNT WINVER
   #endif

   #if LOFTY_HOST_CXX_MSC
      // Silence warnings from system header files.

      /* “Unreferenced inline function has been removed” – must be disabled until the end of the compilation
      unit, because that’s when it’s raised. */
      #pragma warning(disable: 4514)
      // These can be restored after including header files.
      #pragma warning(push)
      // “'macro' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'”
      #pragma warning(disable: 4668)
   #endif

   #define WIN32_LEAN_AND_MEAN
   #include <windows.h>

   #if LOFTY_HOST_CXX_MSC
      #pragma warning(pop)
   #endif

   /* Quoting MSDN:
      “To avoid conflicts with min and max in WINDEF.H, use _MIN and _MAX instead. These macros evaluate to
      _cpp_min and _cpp_max, respectively.”
   Of course we don’t care for old compatibility macros, and want to use std::min/max instead, so undefine
   these macros. */
   #ifdef min
      #undef min
      #undef max
   #endif
#endif //if LOFTY_HOST_API_WIN32

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// C++11 compiler features detection.

// Ensure RTTI support is enabled for MSC.
#if LOFTY_HOST_CXX_MSC && !defined(_CPPRTTI)
   #error "Please compile with /GR"
#endif

/*! If defined, the compiler supports defining conversion operators as explicit, to avoid executing them
implicitly (N2437). */
#if __has_feature(cxx_explicit_conversions) || LOFTY_HOST_CXX_GCC || LOFTY_HOST_CXX_MSC >= 1800
   #define LOFTY_CXX_EXPLICIT_CONVERSION_OPERATORS
#endif

/*! If defined, the compiler allows to delete a specific (overload of a) function, method or constructor
(N2346). */
#if __has_feature(cxx_deleted_functions) || LOFTY_HOST_CXX_GCC || LOFTY_HOST_CXX_MSC >= 1800
   #define LOFTY_CXX_FUNC_DELETE
#endif

//! If defined, the compiler supports the noexcept exception specification.
#if __has_feature(cxx_noexcept) || LOFTY_HOST_CXX_GCC
   #define LOFTY_CXX_NOEXCEPT
#endif

//! If defined, the compiler expects C++11 noexcept specifications for STL functions/methods.
#if __has_feature(cxx_noexcept) || LOFTY_HOST_CXX_GCC
   #define LOFTY_CXX_STL_USES_NOEXCEPT
#endif

//! If defined, the compiler supports variadic templates (N2242).
#if __has_feature(cxx_variadic_templates) || LOFTY_HOST_CXX_GCC || LOFTY_HOST_CXX_MSC >= 1800
   #define LOFTY_CXX_VARIADIC_TEMPLATES
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Compatibility layer for features that are available one way or another in all supported compilers.

/*! If defined, the compiler supports #pragma once, which tells the preprocessor not to parse a (header) file
more than once, speeding up compilation. */
#if LOFTY_HOST_CXX_CLANG || LOFTY_HOST_CXX_GCC || LOFTY_HOST_CXX_MSC
   #define LOFTY_CXX_PRAGMA_ONCE
   // If we won’t be parsing this file again, use it now for this file.
   #ifdef _LOFTY_HOST_HXX
      #pragma once
   #endif
#endif

#if LOFTY_HOST_CXX_MSC
   #if LOFTY_HOST_CXX_MSC < 1700
      /* MSC16 suffers from a bug that makes it compute a wrong __alignof() for a type that hasn’t been “used”
      yet (see <https://connect.microsoft.com/VisualStudio/feedback/details/682695>); work around it by
      forcing “using” the type by applying sizeof() to it. */
      #define alignof(type) \
         (sizeof(type) * 0 + __alignof(type))
   #else
      // Non-standard name, but analogous to C++11’s alignof().
      #define alignof(type) \
         __alignof(type)
   #endif
#endif

/*! Range-based for statement: for (for-range-declaration : expression) { … } .

@param range_decl
   Declaration of the variable that will hold the values iterated over from the range; most commonly this is
   auto & or auto const &.
@param expr
   Expression of a type for which std::begin() and std::end() are defined.
*/
#if __has_feature(cxx_range_for) || LOFTY_HOST_CXX_GCC || LOFTY_HOST_CXX_MSC >= 1700
   #define LOFTY_FOR_EACH(range_decl, expr) \
      for (range_decl : expr)
#elif LOFTY_HOST_CXX_MSC
   // MSC16 has a pre-C++11 syntax that seems to be functionally identical to the C++11 version.
   #define LOFTY_FOR_EACH(range_decl, expr) \
      for each (range_decl in expr)
#endif

/*! Declares a function as never returning (e.g. by causing the process to terminate, or by throwing an
exception). This allows optimizations based on the fact that code following its call cannot be reached. */
#if LOFTY_HOST_CXX_CLANG || LOFTY_HOST_CXX_GCC
   #define LOFTY_FUNC_NORETURN \
      __attribute__((noreturn))
#elif LOFTY_HOST_CXX_MSC
   #define LOFTY_FUNC_NORETURN \
      __declspec(noreturn)
#else
   #define LOFTY_FUNC_NORETURN
#endif

#if __has_feature(cxx_override_control) || LOFTY_HOST_CXX_GCC || LOFTY_HOST_CXX_MSC >= 1800
   // Good, no need for fixes.
#elif LOFTY_HOST_CXX_MSC
   // MSC16 thinks that override is a non-standard extension, so we need to tell it otherwise.
   #define override \
      __pragma(warning(suppress: 4481)) override
#else
   // For everybody else, just disable override control.
   #define override
#endif

/*! Declares a struct as having no padding added by the compiler. This macro expands into the name as-is, to
be optionally followed by the struct definition.

@param name
   Name of the struct to declare.
*/
#if LOFTY_HOST_CXX_MSC
   #define LOFTY_PACKED_STRUCT(name) \
      __pragma(pack(push, 1)) struct name __pragma(pack(pop))
#elif LOFTY_HOST_CXX_CLANG || LOFTY_HOST_CXX_GCC
   #define LOFTY_PACKED_STRUCT(name) \
      struct __attribute__((packed)) name
#endif

//! Declares a function as using the same calling convention as the host C library/STL implementation.
#if LOFTY_HOST_API_WIN32 && !LOFTY_HOST_API_WIN64
   #define LOFTY_STL_CALLCONV __cdecl
#else
   #define LOFTY_STL_CALLCONV
#endif

/*! Declares a function/method as throwing exceptions depending on a (template-dependent) condition. Supports
both C++11 noexcept specifier and pre-C++11 throw() exception specifications.

@param cond
   Condition under which the function/method should be considered “to-throw”.
@param old_throw_decl
   Parentheses-enclosed list of types the function/method may throw.
*/
#ifdef LOFTY_CXX_STL_USES_NOEXCEPT
   #define LOFTY_STL_NOEXCEPT_IF(cond, old_throw_decl) \
      noexcept(cond)
#else
   #define LOFTY_STL_NOEXCEPT_IF(cond, old_throw_decl) \
      throw old_throw_decl
#endif

/*! Declares a function/method as possibly throwing exceptions. Supports both C++11 noexcept specifier and
pre-C++11 throw() exception specifications.

@param old_throw_decl
   Parentheses-enclosed list of types the function/method may throw.
*/
#ifdef LOFTY_CXX_STL_USES_NOEXCEPT
   #define LOFTY_STL_NOEXCEPT_FALSE(old_throw_decl) \
      noexcept(false)
#else
   #define LOFTY_STL_NOEXCEPT_FALSE(old_throw_decl) \
      throw old_throw_decl
#endif

/*! Declares a function/method as never throwing exceptions. Supports both C++11 noexcept specifier and
pre-C++11 throw() exception specifications. */
#ifdef LOFTY_CXX_STL_USES_NOEXCEPT
   #define LOFTY_STL_NOEXCEPT_TRUE() \
      noexcept(true)
#else
   #define LOFTY_STL_NOEXCEPT_TRUE() \
      throw()
#endif

/*! Declares a function as never returning (e.g. by causing the process to terminate, or by throwing an
exception). This allows optimizations based on the fact that code following its call cannot be reached. */
#if LOFTY_HOST_CXX_CLANG || LOFTY_HOST_CXX_GCC
   #define LOFTY_SWITCH_WITHOUT_DEFAULT \
      default: \
         __builtin_unreachable();
#elif LOFTY_HOST_CXX_MSC
   #define LOFTY_SWITCH_WITHOUT_DEFAULT \
      default: \
         __assume(0);
#else
   #define LOFTY_FUNC_NORETURN
#endif

//! Declares a symbol to be publicly visible (exported) in the shared library being built.
#if LOFTY_HOST_API_WIN32
   #if LOFTY_HOST_CXX_CLANG || LOFTY_HOST_CXX_MSC
      // TODO: needs testing; Clang claims to need -fms-extensions to enable dllexport.
      #define LOFTY_SYM_EXPORT \
         __declspec(dllexport)
   #elif LOFTY_HOST_CXX_GCC
      #define LOFTY_SYM_EXPORT \
         __attribute__((dllexport))
   #endif
#else
   #if LOFTY_HOST_CXX_CLANG || LOFTY_HOST_CXX_GCC
      #define LOFTY_SYM_EXPORT \
         __attribute__((visibility("default")))
   #endif
#endif

//! Declares a symbol to be imported from a shared library.
#if LOFTY_HOST_API_WIN32
   #if LOFTY_HOST_CXX_CLANG || LOFTY_HOST_CXX_MSC
      // TODO: needs testing; Clang claims to need -fms-extensions to enable dllimport.
      #define LOFTY_SYM_IMPORT \
         __declspec(dllimport)
   #elif LOFTY_HOST_CXX_GCC
      #define LOFTY_SYM_IMPORT \
         __attribute__((dllimport))
   #endif
#else
   #if LOFTY_HOST_CXX_CLANG || LOFTY_HOST_CXX_GCC
      #define LOFTY_SYM_IMPORT \
         __attribute__((visibility("default")))
   #endif
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_STL_MSVCRT && LOFTY_HOST_CXX_MSC
   // Silence warnings from system header files.
   #pragma warning(push)
   // “expression before comma has no effect; expected expression with side-effect”
   #pragma warning(disable: 4548)
   // “'id' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'”
   #pragma warning(disable: 4668)
#endif
#include <cstddef> // std::nullptr_t std::ptrdiff_t std::size_t
#if LOFTY_HOST_STL_MSVCRT && LOFTY_HOST_CXX_MSC
   #pragma warning(pop)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_HOST_HXX_NOPUB

#ifdef _LOFTY_HOST_HXX
   #undef _LOFTY_NOPUB

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_HOST_HXX
