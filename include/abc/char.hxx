/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#ifndef _ABC_CORE_HXX
   #error Please #include <abc/core.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals


/** Indicates the level of UTF-8 string literals support:
•  2 - The UTF-8 string literal prefix (u8) is supported;
•  1 - The u8 prefix is not supported, but the compiler will generate valid UTF-8 string literals if
       the source file is UTF-8+BOM-encoded;
•  0 - UTF-8 string literals are not supported in any way.
*/
#define ABC_CXX_UTF8LIT 0

/** Indicates how char16_t is defined:
•  2 - char16_t is a native type, distinct from uint16_t and wchar_t;
•  1 - abc::char16_t is a typedef for native 16-bit wchar_t, distinct from uint16_t;
•  0 - abc::char16_t is a typedef for uint16_t.
*/
#define ABC_CXX_CHAR16 0

/** Indicates how char32_t is defined:
•  2 - char32_t is a native type, distinct from uint32_t and wchar_t;
•  1 - abc::char32_t is a typedef for native 32-bit wchar_t, distinct from uint32_t;
•  0 - abc::char32_t is a typedef for uint32_t.
*/
#define ABC_CXX_CHAR32 0

#if ABC_HOST_GCC >= 40400
   // char16_t is a native type, different than uint16_t.
   #undef ABC_CXX_CHAR16
   #define ABC_CXX_CHAR16 2
   // char32_t is a native type, different than uint32_t.
   #undef ABC_CXX_CHAR32
   #define ABC_CXX_CHAR32 2

   #if ABC_HOST_GCC >= 40500
      // UTF-8 string literals are supported.
      #undef ABC_CXX_UTF8LIT
      #define ABC_CXX_UTF8LIT 2
   #endif
#else //if ABC_HOST_GCC >= 40400
   #if ABC_HOST_MSC
      #if !defined(_WCHAR_T_DEFINED) || !defined(_NATIVE_WCHAR_T_DEFINED)
         #error Please compile with /Zc:wchar_t
      #endif

      // char16_t is not a native type, but we can typedef it as wchar_t.
      #undef ABC_CXX_CHAR16
      #define ABC_CXX_CHAR16 1
   #else
      // MSC16 will transcode non-wchar_t string literals into whatever single-byte encoding is
      // selected for the user running cl.exe; a solution has been provided in form of a hotfix
      // (<http://support.microsoft.com/kb/2284668/en-us>), but it no longer seems available, and it
      // was not ported to MSC17/VS2012, thought it seems it was finally built into MSC18/VS2013
      // (<http://connect.microsoft.com/VisualStudio/feedback/details/773186/pragma-execution-
      // character-set-utf-8-didnt-support-in-vc-2012>).
      //
      // Here we assume that no other compiler exhibits such a random behavior, and they will all
      // emit valid UTF-8 string literals it the source file is UTF-8+BOM-encoded.
      #undef ABC_CXX_UTF8LIT
      #define ABC_CXX_UTF8LIT 1

      // char32_t is not a native type, but we can typedef it as wchar_t.
      #undef ABC_CXX_CHAR32
      #define ABC_CXX_CHAR32 1
   #endif
#endif //if ABC_HOST_GCC >= 40400 … else
#if ABC_CXX_CHAR16 == 0 && ABC_CXX_CHAR32 == 0
   #error ABC_CXX_CHAR16 and/or ABC_CXX_CHAR32 must be > 0; please fix detection logic
#endif


/** UTF-8 character type. */
typedef char char8_t;

/** UTF-16 character type. */
#if ABC_CXX_CHAR16 == 1
   typedef wchar_t char16_t;
#elif ABC_CXX_CHAR16 == 0
   typedef uint16_t char16_t;
#endif

/** UTF-32 character type. */
#if ABC_CXX_CHAR32 == 1
   typedef wchar_t char32_t;
#elif ABC_CXX_CHAR32 == 0
   typedef uint32_t char32_t;
#endif


/** Defines an 8-bit character literal.

ch
   Character literal.
return
   8-bit character literal.
*/
#define U8CL(ch) ch


/** Defines a UCS-16 character literal.

ch
   Character literal.
return
   UCS-16 character literal.
*/
#if ABC_CXX_CHAR16 == 2
   #define U16CL(ch) u ## ch
#elif ABC_CXX_CHAR16 == 1
   #define U16CL(ch) L ## ch
#elif ABC_CXX_CHAR16 == 0
   // No native type for char16_t, but we can at least use 32-bit wchar_t to store any Unicode
   // character correctly, and then truncate that to our typedef’ed char16_t.
   // TODO: make the truncation explicit (compiler warning?).
   #define U16CL(ch) ::abc::char16_t(L ## ch)
#endif


/** Defines a UTF-32/UCS-32 character literal. Code points outside the Basic Multilingual Plane are
not supported on all platforms.

ch
   Character literal.
return
   UTF-32/UCS-32 character literal.
*/
#if ABC_CXX_CHAR32 == 2
   #define U32CL(ch) U ## ch
#elif ABC_CXX_CHAR32 == 1
   #define U32CL(ch) L ## ch
#elif ABC_CXX_CHAR32 == 0
   // No native type for char32_t, but we can at least use 16-bit wchar_t to store all Unicode BMP
   // characters correctly, and then cast that to our typedef’ed char32_t.
   #define U32CL(ch) ::abc::char32_t(L ## ch)
#endif


/** Defines a UTF-8 string literal. On some platforms, this relies on the source files being encoded
in UTF-8.

s
   String literal.
return
   UTF-8 string literal.
*/
#if ABC_CXX_UTF8LIT == 2
   #define U8SL(s) u8 ## s
#elif ABC_CXX_UTF8LIT == 1
   // Rely on the source files being encoded in UTF-8.
   #define U8SL(s) s
#endif


/** Defines a UTF-16 string literal. Not supported on all platforms; check with #ifdef before using.

s
   String literal.
return
   UTF-16 string literal.
*/
#if ABC_CXX_CHAR16 == 2
   #define U16SL(s) u ## s
#elif ABC_CXX_CHAR16 == 1
   #define U16SL(s) L ## s
#endif


/** Defines a UTF-32/UCS-32 string literal. Not supported on all platforms; check with #ifdef before
using.

s
   String literal.
return
   UTF-32 string literal.
*/
#if ABC_CXX_CHAR32 == 2
   #define U32SL(s) U ## s
#elif ABC_CXX_CHAR32 == 1
   #define U32SL(s) L ## s
#endif


/** UTF-* encoding supported by the host. */
#if ABC_HOST_API_WIN32 && defined(UNICODE)
   #define ABC_HOST_UTF 16
#else
   #define ABC_HOST_UTF 8
#endif

namespace abc {

/** Default UTF character type for the host. */
#if ABC_HOST_UTF == 8
   typedef char8_t char_t;
#elif ABC_HOST_UTF == 16
   typedef char16_t char_t;
#elif ABC_HOST_UTF == 32
   typedef char32_t char_t;
#endif

} //namespace abc


/** Defines a character literal of the default host character literal type.

ch
   Character literal.
return
   UCS character literal.
*/
#if ABC_HOST_UTF == 8
   #define CL(ch) U8CL(ch)
#elif ABC_HOST_UTF == 16
   #define CL(ch) U16CL(ch)
#elif ABC_HOST_UTF == 32
   #define CL(ch) U32CL(ch)
#endif


/** Defines a string literal of the default host string literal type.

s
   String literal.
return
   UTF string literal.
*/
#if ABC_HOST_UTF == 8
   #define SL(s) U8SL(s)
#elif ABC_HOST_UTF == 16
   #define SL(s) U16SL(s)
#elif ABC_HOST_UTF == 32
   #define SL(s) U32SL(s)
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////

