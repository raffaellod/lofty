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

/*! @file
Macros to generate proper Unicode characters.
*/


////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Indicates the level of UTF-8 string literals support:
•  2 - The UTF-8 string literal prefix (u8) is supported;
•  1 - The u8 prefix is not supported, but the compiler will generate valid UTF-8 string literals if
       the source file is UTF-8+BOM-encoded;
•  0 - UTF-8 string literals are not supported in any way.
*/
#define ABC_CXX_UTF8LIT 0

/*! Indicates how char16_t is defined:
•  2 - char16_t is a native type, distinct from std::uint16_t and wchar_t;
•  1 - abc::char16_t is a typedef for native 16-bit wchar_t, distinct from std::uint16_t;
•  0 - abc::char16_t is a typedef for std::uint16_t.
*/
#define ABC_CXX_CHAR16 0

/*! Indicates how char32_t is defined:
•  2 - char32_t is a native type, distinct from std::uint32_t and wchar_t;
•  1 - abc::char32_t is a typedef for native 32-bit wchar_t, distinct from std::uint32_t;
•  0 - abc::char32_t is a typedef for std::uint32_t.
*/
#define ABC_CXX_CHAR32 0

//! @cond

// Only support Unicode Windows programs.
// TODO: support non-Unicode Windows programs (Win9x and Win16). In a very, very distant future!
#ifndef UNICODE
   #define UNICODE
#endif

// Make sure UNICODE and _UNICODE are coherent; UNICODE wins.
#if defined(UNICODE) && !defined(_UNICODE)
   #define _UNICODE
#elif !defined(UNICODE) && defined(_UNICODE)
   #undef _UNICODE
#endif

//! @endcond

#if ABC_HOST_CXX_CLANG || ABC_HOST_CXX_GCC >= 40400
   // char16_t is a native type, different than std::uint16_t.
   #undef ABC_CXX_CHAR16
   #define ABC_CXX_CHAR16 2
   // char32_t is a native type, different than std::uint32_t.
   #undef ABC_CXX_CHAR32
   #define ABC_CXX_CHAR32 2

   #if (ABC_HOST_CXX_CLANG && __has_feature(cxx_unicode_literals)) || ABC_HOST_CXX_GCC >= 40500
      // UTF-8 string literals are supported.
      #undef ABC_CXX_UTF8LIT
      #define ABC_CXX_UTF8LIT 2
   #endif
#else //if ABC_HOST_CXX_GCC >= 40400
   #if ABC_HOST_CXX_MSC
      #if !defined(_WCHAR_T_DEFINED) || !defined(_NATIVE_WCHAR_T_DEFINED)
         #error "Please compile with /Zc:wchar_t"
      #endif

      // char16_t is not a native type, but we can typedef it as wchar_t.
      #undef ABC_CXX_CHAR16
      #define ABC_CXX_CHAR16 1
   #else
      /* MSC16 will transcode non-wchar_t string literals into whatever single-byte encoding is
      selected for the user running cl.exe; a solution has been provided in form of a hotfix
      (<http://support.microsoft.com/kb/2284668/en-us>), but it no longer seems available, and it
      was not ported to MSC17/VS2012, thought it seems it was finally built into MSC18/VS2013
      (<http://connect.microsoft.com/VisualStudio/feedback/details/773186/pragma-execution-
      character-set-utf-8-didnt-support-in-vc-2012>).

      Here we assume that no other compiler exhibits such a random behavior, and they will all emit
      valid UTF-8 string literals it the source file is UTF-8+BOM-encoded. */
      #undef ABC_CXX_UTF8LIT
      #define ABC_CXX_UTF8LIT 1

      // char32_t is not a native type, but we can typedef it as wchar_t.
      #undef ABC_CXX_CHAR32
      #define ABC_CXX_CHAR32 1
   #endif
#endif //if ABC_HOST_CXX_GCC >= 40400 … else
#if ABC_CXX_CHAR16 == 0 && ABC_CXX_CHAR32 == 0
   #error "ABC_CXX_CHAR16 and/or ABC_CXX_CHAR32 must be > 0; please fix detection logic"
#endif


//! UTF-8 character type.
typedef char char8_t;

//! UTF-16 character type.
#if ABC_CXX_CHAR16 == 1
   typedef wchar_t char16_t;
#elif ABC_CXX_CHAR16 == 0
   typedef std::uint16_t char16_t;
#endif

//! UTF-32 character type.
#if ABC_CXX_CHAR32 == 1
   typedef wchar_t char32_t;
#elif ABC_CXX_CHAR32 == 0
   typedef std::uint32_t char32_t;
#endif

//! UTF-* encoding supported by the host.
#if ABC_HOST_API_WIN32 && defined(UNICODE)
   #define ABC_HOST_UTF 16
#else
   #define ABC_HOST_UTF 8
#endif

namespace abc { namespace text {

/*! Default UTF character type for the host. Note that only UTF-8 and UTF-16 are supported as native
characters types. */
/* When introducing a new possible value for this constant, please make sure to update the value
selection logic for abc::text::encoding::host to provide the corresponding UTF encoding. */
#if ABC_HOST_UTF == 8
   typedef char8_t char_t;
#elif ABC_HOST_UTF == 16
   typedef char16_t char_t;
#endif

}} //namespace abc::text


/*! Use this to specify a non-ASCII character literal. When compiled, this will expand into a
character literal of the widest type supported by the compiler, which is char32_t in the best case
and wchar_t otherwise, which on Windows is limited to 16 bits (UCS-2).

@param ch
   Character literal.
@return
   Unicode character literal.
*/
#if ABC_CXX_CHAR32 == 2
   /* Use U so that the resulting literal is of type char32_t, which cuts down the number of
   overloads we need. */
   #define ABC_CHAR(ch) U ## ch
#else
   // Everybody else can only use wchar_t as the largest character literal type, so here it goes.
   #define ABC_CHAR(ch) L ## ch
#endif

/*! @cond
Implementation of ABC_SL(); allows for expansion of the argument prior to pasting it to the
appropriate string literal prefix, as is necessary for e.g. __FILE__.

@param sz
   String literal.
@return
   UTF string literal.
*/
#if ABC_HOST_UTF == 8
   #if ABC_CXX_UTF8LIT == 2
      #define _ABC_SL(sz) u8 ## sz
   #else
      #define _ABC_SL(s) sz
   #endif
#elif ABC_HOST_UTF == 16
   #define _ABC_SL(sz) L ## sz
#endif
//! @endcond

/*! Defines a string literal of the default host string literal type (UTF-8 or UTF-16).

@param sz
   String literal.
@return
   UTF string literal.
*/
#define ABC_SL(sz) _ABC_SL(sz)

/*! Returns the size of a string literal (character array), excluding the trailing NUL character, if
present.

@param ach
   String literal.
@return
   Size of ach, in characters, minus 1 if its last character is NUL.
*/
#define ABC_SL_SIZE(ach) \
   (ABC_COUNTOF(ach) - (ach[ABC_COUNTOF(ach) - 1 /*NUL*/] == '\0'))
