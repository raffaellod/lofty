/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

/*! @file
Defines macros and types for platform-independent Unicode characters and strings.

Specifically, it declares members of the lofty::text namespace that have no dependencies, so this file can be
pulled early in the inclusion chain. */

#ifndef _LOFTY_TEXT_0_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_TEXT_0_HXX
#endif

#ifndef _LOFTY_TEXT_0_HXX_NOPUB
#define _LOFTY_TEXT_0_HXX_NOPUB

#include <lofty/_pvt/lofty.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {
_LOFTY_PUBNS_BEGIN

/*! Default UTF character type for the host. Note that only UTF-8 and UTF-16 are supported as native
characters types. */
/* When introducing a new possible value for this constant, please make sure to update the value selection
logic for lofty::text::encoding::host to provide the corresponding UTF encoding. */
#if LOFTY_HOST_UTF == 8
   typedef char8_t char_t;
#elif LOFTY_HOST_UTF == 16
   typedef char16_t char_t;
#endif

_LOFTY_PUBNS_END
}}

/*! Use this to specify a non-ASCII character literal. When compiled, this will expand into a character
literal of the widest type supported by the compiler, which is char32_t in the best case and wchar_t
otherwise, which on Windows is limited to 16 bits (UCS-2).

@param ch
   Character literal.
@return
   Unicode character literal.
*/
#if LOFTY_CXX_CHAR32 == 2
   /* Use U so that the resulting literal is of type char32_t, which cuts down the number of overloads we
   need. */
   #define LOFTY_CHAR(ch) U ## ch
#else
   // Everybody else can only use wchar_t as the largest character literal type, so here it goes.
   #define LOFTY_CHAR(ch) L ## ch
#endif

/*! @cond
Implementation of LOFTY_SL(); allows for expansion of the argument prior to pasting it to the appropriate
string literal prefix, as is necessary for e.g. __FILE__.

@param s
   String literal.
@return
   UTF string literal.
*/
#if LOFTY_HOST_UTF == 8
   #if LOFTY_CXX_UTF8LIT == 2
      #define _LOFTY_SL(s) u8 ## s
   #else
      #define _LOFTY_SL(s) s
   #endif
#elif LOFTY_HOST_UTF == 16
   #define _LOFTY_SL(s) L ## s
#endif
//! @endcond

/*! Defines a string literal of the default host string literal type (UTF-8 or UTF-16).

@param s
   String literal.
@return
   UTF string literal.
*/
#define LOFTY_SL(s) _LOFTY_SL(s)

/*! Returns the size of a string literal (character array), excluding the trailing NUL character, if present.

@param s
   String literal.
@return
   Size of s, in characters, minus 1 if its last character is NUL.
*/
#define LOFTY_SL_SIZE(s) \
   (LOFTY_COUNTOF(s) - (s[LOFTY_COUNTOF(s) - 1 /*NUL*/] == '\0'))

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {
_LOFTY_PUBNS_BEGIN

template <std::size_t embedded_capacity>
class sstr;

typedef sstr<0> str;

_LOFTY_PUBNS_END
}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TEXT_0_HXX_NOPUB

#ifdef _LOFTY_TEXT_0_HXX
   #undef _LOFTY_NOPUB

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_TEXT_0_HXX
