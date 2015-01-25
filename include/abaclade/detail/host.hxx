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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


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
// abc globals – ABC_HOST_ARCH_*

#define ABC_HOST_ARCH_ALPHA  0
#define ABC_HOST_ARCH_ARM    0
#define ABC_HOST_ARCH_I386   0
#define ABC_HOST_ARCH_IA64   0
#define ABC_HOST_ARCH_PPC    0
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
   #error "TODO: HOST_ARCH"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals – ABC_HOST_*_ENDIAN

#define ABC_HOST_LITTLE_ENDIAN 0
#define ABC_HOST_BIG_ENDIAN    0

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
