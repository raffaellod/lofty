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

/* Compatibility layer for features that are available one way or another in all supported
compilers. */

/*! If defined, the compiler supports #pragma once, which tells the preprocessor not to parse a
(header) file more than once, speeding up compilation. */
#if ABC_HOST_CXX_CLANG || ABC_HOST_CXX_GCC || ABC_HOST_CXX_MSC
   #define ABC_CXX_PRAGMA_ONCE
#endif

#if ABC_HOST_CXX_MSC
   #if ABC_HOST_CXX_MSC < 1700
      /* MSC16 suffers from a bug that makes it compute a wrong __alignof() for a type that hasn’t
      been “used” yet (see <https://connect.microsoft.com/VisualStudio/feedback/details/682695>);
      work around it by forcing “using” the type by applying sizeof() to it. */
      #define alignof(type) \
         (sizeof(type) * 0 + __alignof(type))
   #else
      // Non-standard name, but analogous to C++11’s alignof().
      #define alignof(type) \
         __alignof(type)
   #endif
#endif

/*! Range-based for statement: for (for-range-declaration : expression) { … } .

@param rangedecl
   Declaration of the variable that will hold the values iterated over from the range; most commonly
   this is auto & or auto const &.
@param expr
   Expression of a type for which std::begin() and std::end() are defined.
*/
#if (ABC_HOST_CXX_CLANG && __has_feature(cxx_range_for)) || ABC_HOST_CXX_GCC || \
      ABC_HOST_CXX_MSC >= 1700
   #define ABC_FOR_EACH(rangedecl, expr) \
      for (rangedecl : expr)
#elif ABC_HOST_CXX_MSC
   // MSC16 has a pre-C++11 syntax that seems to be functionally identical to the C++11 version.
   #define ABC_FOR_EACH(rangedecl, expr) \
      for each (rangedecl in expr)
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

#if (ABC_HOST_CXX_CLANG && __has_feature(cxx_override_control)) || ABC_HOST_CXX_GCC >= 0x40700 || \
      ABC_HOST_CXX_MSC >= 1800
   // Good, no need for fixes.
#elif ABC_HOST_CXX_MSC
   // MSC16 thinks that override is a non-standard extension, so we need to tell it otherwise.
   #define override \
      __pragma(warning(suppress: 4481)) override
#else
   // For everybody else, just disable override control.
   #define override
#endif

/*! Declares a function as using the same calling convention as the host C library/STL
implementation. */
#if ABC_HOST_API_WIN32 && !ABC_HOST_API_WIN64
   #define ABC_STL_CALLCONV __cdecl
#else
   #define ABC_STL_CALLCONV
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

/*! Declares a function/method as never throwing exceptions. Supports both C++11 noexcept specifier
and pre-C++11 throw() exception specifications. */
#ifdef ABC_CXX_STL_USES_NOEXCEPT
   #define ABC_STL_NOEXCEPT_TRUE() \
      noexcept(true)
#else
   #define ABC_STL_NOEXCEPT_TRUE() \
      throw()
#endif

/*! Declares a function as never returning (e.g. by causing the process to terminate, or by throwing
an exception). This allows optimizations based on the fact that code following its call cannot be
reached. */
#if ABC_HOST_CXX_CLANG || ABC_HOST_CXX_GCC
   #define ABC_SWITCH_WITHOUT_DEFAULT \
      default: \
         __builtin_unreachable();
#elif ABC_HOST_CXX_MSC
   #define ABC_SWITCH_WITHOUT_DEFAULT \
      default: \
         __assume(0);
#else
   #define ABC_FUNC_NORETURN
#endif

//! Declares a symbol to be publicly visible (exported) in the shared library being built.
#if ABC_HOST_API_WIN32
   #if ABC_HOST_CXX_CLANG || ABC_HOST_CXX_MSC
      // TODO: needs testing; Clang claims to need -fms-extensions to enable dllexport.
      #define ABC_SYM_EXPORT \
         __declspec(dllexport)
   #elif ABC_HOST_CXX_GCC
      #define ABC_SYM_EXPORT \
         __attribute__((dllexport))
   #endif
#else
   #if ABC_HOST_CXX_CLANG || ABC_HOST_CXX_GCC
      #define ABC_SYM_EXPORT \
         __attribute__((visibility("default")))
   #endif
#endif

//! Declares a symbol to be imported from a shared library.
#if ABC_HOST_API_WIN32
   #if ABC_HOST_CXX_CLANG || ABC_HOST_CXX_MSC
      // TODO: needs testing; Clang claims to need -fms-extensions to enable dllimport.
      #define ABC_SYM_IMPORT \
         __declspec(dllimport)
   #elif ABC_HOST_CXX_GCC
      #define ABC_SYM_IMPORT \
         __attribute__((dllimport))
   #endif
#else
   #if ABC_HOST_CXX_CLANG || ABC_HOST_CXX_GCC
      #define ABC_SYM_IMPORT \
         __attribute__((visibility("default")))
   #endif
#endif
