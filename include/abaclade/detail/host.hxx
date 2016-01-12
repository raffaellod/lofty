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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

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
   #if ABC_HOST_CXX_GCC < 40700
      #error "Unsupported version of GCC: >= 4.7 required"
   #endif
#elif defined(_MSC_VER)
   #undef ABC_HOST_CXX_MSC
   #define ABC_HOST_CXX_MSC _MSC_VER
   #if ABC_HOST_CXX_MSC < 1600
      #error "Unsupported version of MSC: >= MSC 16 / VC++ 10 / VS 2010 required"
   #endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

//! 1 if building with Abaclade’s STL subset implementation, or 0 otherwise.
#define ABC_HOST_STL_ABACLADE 0
//! Version of Clang’s libc++ STL implementation if building against it, or 0 otherwise.
#define ABC_HOST_STL_LIBCXX 0
//! Version of GNU libstdc++ STL implementation if building against it, or 0 otherwise.
#define ABC_HOST_STL_LIBSTDCXX 0
//! Version of MSVCRT STL implementation if building against it, or 0 otherwise.
#define ABC_HOST_STL_MSVCRT 0

#ifdef _ABC_USE_STLIMPL
   #undef ABC_HOST_STL_ABACLADE
   #define ABC_HOST_STL_ABACLADE 1
#elif ABC_HOST_CXX_CLANG || ABC_HOST_CXX_GCC
   #if ABC_HOST_CXX_CLANG
      #if __has_include(<__config>)
         #include <__config>
      #endif
   #endif
   #ifdef _LIBCPP_VERSION
      #undef ABC_HOST_STL_LIBCXX
      #define ABC_HOST_STL_LIBCXX _LIBCPP_VERSION
   #else
      /* Not libc++; assume we’re using GNU libstdc++. This will be confirmed after we #include
      <cstdint> in abaclade.hxx. */
      #undef ABC_HOST_STL_LIBSTDCXX
      #define ABC_HOST_STL_LIBSTDCXX 1
   #endif
#elif ABC_HOST_CXX_MSC
   // Assume the version of MSVCRT matches that of MSC.
   #undef ABC_HOST_STL_MSVCRT
   #define ABC_HOST_STL_MSVCRT ABC_HOST_CXX_MSC
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

//! 1 if building for a BSD-like OS, or 0 otherwise. Implies ABC_HOST_API_POSIX.
#define ABC_HOST_API_BSD 0
/*! 1 if building for a Darwin-based OS such as OS X or iOS, or 0 otherwise. Implies
ABC_HOST_API_BSD, ABC_HOST_API_MACH, ABC_HOST_API_POSIX. */
#define ABC_HOST_API_DARWIN 0
//! 1 if building for FreeBSD, or 0 otherwise. Implies ABC_HOST_API_BSD, ABC_HOST_API_POSIX.
#define ABC_HOST_API_FREEBSD 0
//! 1 if building for Linux (the kernel), or 0 otherwise. Implies ABC_HOST_API_POSIX.
#define ABC_HOST_API_LINUX 0
//! 1 if building for a POSIX-compatible OS, or 0 otherwise.
#define ABC_HOST_API_POSIX 0
//! 1 if building for a Mach-based OS, or 0 otherwise.
#define ABC_HOST_API_MACH 0
//! 1 if building for the Win32 API, or 0 otherwise.
#define ABC_HOST_API_WIN32 0
/*! 1 if building for the 64-bit variant of the Win32 API, or 0 otherwise. Implies
ABC_HOST_API_WIN32. */
#define ABC_HOST_API_WIN64 0

#if defined(_WIN32)
   // Compiling for Win32 or Win64.
   #undef ABC_HOST_API_WIN32
   #define ABC_HOST_API_WIN32 1
   #ifdef _WIN64
      // Compiling for Win64 (coexists with ABC_HOST_API_WIN32).
      #undef ABC_HOST_API_WIN64
      #define ABC_HOST_API_WIN64 1
   #endif
#elif defined(__linux__)
   // Compiling for GNU/Linux.
   #undef ABC_HOST_API_LINUX
   #define ABC_HOST_API_LINUX 1
   #undef ABC_HOST_API_POSIX
   #define ABC_HOST_API_POSIX 1
#elif defined(__MACH__) && defined(__APPLE__)
   /* Compiling for Darwin/XNU (commercially known as OS X and iOS), mostly compatible with BSD libc
   but using a Mach kernel. */
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

//! 1 if building for the Alpha architecture, or 0 otherwise.
#define ABC_HOST_ARCH_ALPHA 0
//! 1 if building for the ARM architecture, or 0 otherwise.
#define ABC_HOST_ARCH_ARM 0
//! 1 if building for the i386 architecture also known as x86, or 0 otherwise.
#define ABC_HOST_ARCH_I386 0
//! 1 if building for the IA64 architecture, or 0 otherwise.
#define ABC_HOST_ARCH_IA64 0
//! 1 if building for the PowerPC architecture, or 0 otherwise.
#define ABC_HOST_ARCH_PPC 0
//! 1 if building for the x86-64 architecture also known as AMD64, or 0 otherwise.
#define ABC_HOST_ARCH_X86_64 0

#if defined(__alpha__) || defined(_M_ALPHA)
   #undef ABC_HOST_ARCH_ALPHA
   #define ABC_HOST_ARCH_ALPHA 1
#elif defined(__arm__) || defined(_M_ARM)
   #undef ABC_HOST_ARCH_ARM
   #define ABC_HOST_ARCH_ARM 1
#elif defined(__i386__) || defined(_M_IX86)
   #undef ABC_HOST_ARCH_I386
   #define ABC_HOST_ARCH_I386 1
#elif defined(__ia64__) || defined(_M_IA64)
   #undef ABC_HOST_ARCH_IA64
   #define ABC_HOST_ARCH_IA64 1
#elif defined(__powerpc__) || defined(_M_PPC)
   #undef ABC_HOST_ARCH_PPC
   #define ABC_HOST_ARCH_PPC 1
#elif defined(__x86_64__) || defined(__amd64__) || defined(_M_X64) || defined(_M_AMD64)
   #undef ABC_HOST_ARCH_X86_64
   #define ABC_HOST_ARCH_X86_64 1
#else
   #error "TODO: HOST_ARCH"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

//! Machine word size for this microarchitecture.
// TODO: the word/pointer size is much more easily detected by a configure program.
#if ABC_HOST_API_WIN64
   #define ABC_HOST_WORD_SIZE 64
#elif ABC_HOST_API_WIN32
   #define ABC_HOST_WORD_SIZE 32
#elif defined(__SIZEOF_POINTER__)
   #define ABC_HOST_WORD_SIZE (__SIZEOF_POINTER__ * 8)
#else
   #error "TODO: HOST_ARCH"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

/*! 1 if building for a little-endian processor architecture, or 0 otherwise. Implies
!ABC_HOST_BIG_ENDIAN. */
#define ABC_HOST_LITTLE_ENDIAN 0
/*! 1 if building for a big-endian processor architecture, or 0 otherwise. Implies
!ABC_HOST_LITTLE_ENDIAN. */
#define ABC_HOST_BIG_ENDIAN 0

// Assume that ARM is always used in little-endian mode.
#if ABC_HOST_ARCH_ALPHA || ABC_HOST_ARCH_ARM || ABC_HOST_ARCH_I386 || ABC_HOST_ARCH_IA64 || \
      ABC_HOST_ARCH_X86_64
   #undef ABC_HOST_LITTLE_ENDIAN
   #define ABC_HOST_LITTLE_ENDIAN 1
#elif ABC_HOST_ARCH_PPC
   #undef ABC_HOST_BIG_ENDIAN
   #define ABC_HOST_BIG_ENDIAN 1
#else
   #error "TODO: HOST_ARCH"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

// Apply fixes depending on ABC_HOST_CXX_*.

// Compatibility with compilers that don’t support feature/extension checking.
#ifndef __has_extension
   #define __has_extension(x) 0
#endif
#ifndef __has_feature
   #define __has_feature(x) 0
#endif

#if ABC_HOST_CXX_MSC
   // Suppress unnecessary warnings.

   // “enumerator 'name' in switch of enum 'type' is not explicitly handled by a case label”
   #pragma warning(disable: 4061)
   // “enumerator 'name' in switch of enum 'type' is not handled”
   #pragma warning(disable: 4062)
   // “conditional expression is constant”
   #pragma warning(disable: 4127)
   /* “nonstandard extension used : 'initializing' : conversion from 'type' to 'type &' A non-const
   reference may only be bound to an lvalue”: raised by ABC_FOR_EACH() when the iterated expression
   is not an l-value. No other compiler complains about it. */
   #pragma warning(disable: 4239)
   // “'class' : inherits 'base::member' via dominance”: it points out the obvious and intended.
   #pragma warning(disable: 4250)
   /* “'class1 member' : class 'template class2' needs to have dll-interface to be used by clients
   of class 'class1'”: not sure why, but can’t imagine how it could possibly make sense to DLL-
   export a class template. */
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
      /* “'derived_class' : Object layout under /vd2 will change due to virtual base 'base_class'”:
      yet another problem related to calling virtual methods from a constructor. This warning could
      be used to detect the latter situation, but MSC raises it unconditionally, so just turn it
      off. */
      #pragma warning(disable: 4435)
      /* “dynamic_cast from virtual base 'base_class' to 'derived_class' could fail in some
      contexts”: only really applies if the code using dynamic_cast gets called on this in a
      constructor or destructor. This warning could be used to detect real errors, but MSC raises it
      unconditionally, so just turn it off.*/
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
   /* “'class' : assignment operator could not be generated because a base class assignment operator
   is inaccessible” */
   #pragma warning(disable: 4626)
   /* “potentially uninitialized local variable 'var' used”: would be useful, but it’s raised too
   easily by MSC16. */
   #pragma warning(disable: 4701)
   // “'function' : function not inlined”
   #pragma warning(disable: 4710)
   // “function 'function' selected for automatic inline expansion”
   #pragma warning(disable: 4711)
   // “'struct' : 'n' bytes padding added after data member 'member'”
   #pragma warning(disable: 4820)
   #if ABC_HOST_CXX_MSC < 1700
      /* “nonstandard extension used : 'type' : local types or unnamed types cannot be used as
      template arguments”. */
      #pragma warning(disable: 4836)
   #endif
#endif //if ABC_HOST_CXX_MSC
