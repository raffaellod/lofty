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

/** String traits for host string types (including abc::*str). Derives from either utf8_str_traits
or utf16_str_traits.
*/
class ABACLADE_SYM host_str_traits :
#if ABC_HOST_UTF == 8
   public utf8_str_traits {
#elif ABC_HOST_UTF == 16
   public utf16_str_traits {
#endif
public:

   /** Builds a failure restart table for searches using the Knuth-Morris-Pratt algorithm. See
   [DOC:1502 KMP substring search] for how this is built and used.

   pchNeedleBegin
      Pointer to the beginning of the search string.
   pchNeedleEnd
      Pointer beyond the end of the search string.
   pvcchFailNext
      Pointer to a vector that will receive the failure restart indices.
   */
   static void _build_find_failure_restart_table(
      char_t const * pchNeedleBegin, char_t const * pchNeedleEnd, mvector<size_t> * pvcchFailNext
   );


   /** Compares two strings.

   pch1Begin
      Pointer to the first character of the first string to compare.
   pch1End
      Pointer to beyond the last character of the string to compare.
   pch2Begin
      Pointer to the first character of the second string to compare.
   pch2End
      Pointer to beyond the last character of the second string to compare.
   return
      Standard comparison result integer:
      •  > 0 if string 1 > string 2;
      •    0 if string 1 == string 2;
      •  < 0 if string 1 < string 2.
   */
   static int compare(
      char_t const * pch1Begin, char_t const * pch1End,
      char_t const * pch2Begin, char_t const * pch2End
   );


   /** Returns a pointer to the first occurrence of a character in a string, or pchHaystackEnd if no
   matches are found. For the non-char32_t needle overload, the needle is a pointer because a code
   point can require more than one non-UTF-32 character to be encoded.

   pchHaystackBegin
      Pointer to the first character of the string to be searched.
   pchHaystackEnd
      Pointer to beyond the last character of the string to be searched.
   chNeedle
      Code point to search for.
   pchNeedle
      Pointer to the encoded code point (UTF character sequence) to search for; its length is
      deduced automatically.
   return
      Pointer to the beginning of the first match, in the string to be searched, of the code point
      to search for, or nullptr if no matches are found.
   */
   static char_t const * find_char(
      char_t const * pchHaystackBegin, char_t const * pchHaystackEnd, char32_t chNeedle
   );
   static char_t const * find_char(
      char_t const * pchHaystackBegin, char_t const * pchHaystackEnd, char_t const * pchNeedle
   );


   /** Returns a pointer to the last occurrence of a character in a string, or pchHaystackBegin if
   no matches are found.

   pchHaystackBegin
      Pointer to the first character of the string to be searched.
   pchHaystackEnd
      Pointer to beyond the last character of the string to be searched.
   chNeedle
      Code point to search for.
   return
      Pointer to the beginning of the last match, in the string to be searched, of the code point
      to search for, or nullptr if no matches are found.
   */
   static char_t const * find_char_last(
      char_t const * pchHaystackBegin, char_t const * pchHaystackEnd, char32_t chNeedle
   );


   /** Returns the character index of the first occurrence of a string into another.

   pchHaystackBegin
      Pointer to the first character of the string to be searched.
   pchHaystackEnd
      Pointer to beyond the last character of the string to be searched.
   pchNeedleBegin
      Pointer to the first character of the string to search for.
   pchNeedleEnd
      Pointer to beyond the last character of the string to search for.
   return
      Pointer to the beginning of the first match, in the string to be searched, of the string to
      search for, or nullptr if no matches are found.
   */
   static char_t const * find_substr(
      char_t const * pchHaystackBegin, char_t const * pchHaystackEnd,
      char_t const * pchNeedleBegin, char_t const * pchNeedleEnd
   );


   /** Returns the character index of the last occurrence of a string into another.

   pchHaystackBegin
      Pointer to the first character of the string to be searched.
   pchHaystackEnd
      Pointer to beyond the last character of the string to be searched.
   pchNeedleBegin
      Pointer to the first character of the string to search for.
   pchNeedleEnd
      Pointer to beyond the last character of the string to search for.
   return
      Pointer to the beginning of the last match, in the string to be searched, of the string to
      search for, or nullptr if no matches are found.
   */
   static char_t const * find_substr_last(
      char_t const * pchHaystackBegin, char_t const * pchHaystackEnd,
      char_t const * pchNeedleBegin, char_t const * pchNeedleEnd
   );
};

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

