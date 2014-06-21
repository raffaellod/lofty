/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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
   #error Please #include <abaclade.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::utf8_str_traits


namespace abc {
namespace text {

/** UTF-8 string traits (constants and functions). Note that this class is not modeled after
std::char_traits.
*/
class ABACLADE_SYM utf8_str_traits {
public:

   /** Max length of a code point, in UTF-8 characters (bytes). Technically, 6 is also possible due
   to the way bits are encoded, but it’s illegal. */
   static unsigned const max_codepoint_length = 4;


public:

   /** Checks if a string is valid UTF.

   psz
      Pointer to the NUL-terminated character array to validate.
   pchBegin
      Pointer to the first character of the string to validate.
   pchEnd
      Pointer to beyond the last character of the string to validate.
   return
      true if the string is valid UTF, or false otherwise.
   */
   static bool is_valid(char8_t const * psz);
   static bool is_valid(char8_t const * pchBegin, char8_t const * pchEnd);


   /** Returns the length, in UTF characters, of a NUL-terminated string.

   psz
      Pointer to the NUL-terminated string of which to calculate the length.
   return
      Length of the string pointed to by psz, in characters.
   */
   static size_t size_in_chars(char8_t const * psz);


   /** Returns count of code points in a string.

   pchBegin
      Pointer to the beginning of the string.
   pchEnd
      Pointer beyond the end of the string.
   return
      Count of code points included in the string.
   */
   static size_t size_in_codepoints(char8_t const * pchBegin, char8_t const * pchEnd);


private:

   /** Bitmasks to be applied to the first trailing byte to check if a code point is using an
   overlong encoding. For example, even though 11100000 10100000 10000000 has all zeroes in the code
   point part of the lead byte (mask 1110xxxx), it cannot be encoded with fewer bytes because the
   second byte uses 6 bits and the 2-byte-long sequence lead byte only has 5 code point bits (mask
   110xxxxx); in this case the mask 00100000, applied to the second byte (10100000) allows to find
   out if a code point could have been encoded with fewer characters.
   The first element (index 0) is for 1-byte continuations. */
   static uint8_t const smc_aiOverlongDetectionMasks[];
};

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::utf16_str_traits


namespace abc {
namespace text {

/** UTF-16 string traits (constants and functions). Note that this class is not modeled after
std::char_traits.
*/
class ABACLADE_SYM utf16_str_traits {
public:

   /** Max length of a code point, in UTF-16 characters. */
   static unsigned const max_codepoint_length = 2;


public:

   /** See utf8_str_traits::is_valid().
   */
   static bool is_valid(char16_t const * psz);
   static bool is_valid(char16_t const * pchBegin, char16_t const * pchEnd);


   /** See utf8_str_traits::size_in_chars().
   */
   static size_t size_in_chars(char16_t const * psz);


   /** See utf8_str_traits::size_in_codepoints().
   */
   static size_t size_in_codepoints(char16_t const * pchBegin, char16_t const * pchEnd);
};

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::host_str_traits


namespace abc {
namespace text {

/** String traits for host string types (including abc::*str). Alias for either utf8_str_traits or
utf16_str_traits. */
class ABACLADE_SYM host_str_traits :
#if ABC_HOST_UTF == 8
   public utf8_str_traits {
#elif ABC_HOST_UTF == 16
   public utf16_str_traits {
#endif
};

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

