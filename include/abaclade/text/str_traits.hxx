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
// abc::text::str_traits


namespace abc {
namespace text {

/*! Low-level functions for dealing with character strings; used by abc::*str. Note that this class
is not modeled after std::char_traits.
*/
class ABACLADE_SYM str_traits {
public:

   /*! Builds a failure restart table for searches using the Knuth-Morris-Pratt algorithm. See
   [DOC:1502 KMP substring search] for how this is built and used.

   pchNeedleBegin
      Pointer to the beginning of the search string.
   pchNeedleEnd
      Pointer to the end of the search string.
   pvcchFailNext
      Pointer to a vector that will receive the failure restart indices.
   */
   static void _build_find_failure_restart_table(
      char_t const * pchNeedleBegin, char_t const * pchNeedleEnd,
      mvector<std::size_t> * pvcchFailNext
   );


   /*! Compares two strings.

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


   /*! Returns a pointer to the first occurrence of a character in a string, or pchHaystackEnd if no
   matches are found. The overload taking a char_t pointer allows to search for the encoded
   representation of any code point.

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
      char_t const * pchHaystackBegin, char_t const * pchHaystackEnd, char_t chNeedle
   ) {
//      ABC_TRACE_FUNC(pchHaystackBegin, pchHaystackEnd, chNeedle);

      for (char_t const * pch(pchHaystackBegin); pch < pchHaystackEnd; ++pch) {
         if (*pch == chNeedle) {
            return pch;
         }
      }
      return pchHaystackEnd;
   }
   static char_t const * find_char(
      char_t const * pchHaystackBegin, char_t const * pchHaystackEnd, char32_t chNeedle
   );
   static char_t const * find_char(
      char_t const * pchHaystackBegin, char_t const * pchHaystackEnd, char_t const * pchNeedle
   );


   /*! Returns a pointer to the last occurrence of a character in a string, or pchHaystackBegin if
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
      char_t const * pchHaystackBegin, char_t const * pchHaystackEnd, char_t chNeedle
   ) {
//      ABC_TRACE_FUNC(pchHaystackBegin, pchHaystackEnd, chNeedle);

      for (char_t const * pch(pchHaystackEnd); pch > pchHaystackBegin; ) {
         if (*--pch == chNeedle) {
            return pch;
         }
      }
      return pchHaystackBegin;
   }
   static char_t const * find_char_last(
      char_t const * pchHaystackBegin, char_t const * pchHaystackEnd, char32_t chNeedle
   );


   /*! Returns the character index of the first occurrence of a string into another.

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


   /*! Returns the character index of the last occurrence of a string into another.

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


   /*! Returns count of code points in a string.

   pchBegin
      Pointer to the beginning of the string.
   pchEnd
      Pointer to the end of the string.
   return
      Count of code points included in the string.
   */
   static std::size_t size_in_codepoints(char_t const * pchBegin, char_t const * pchEnd);


   /*! Validates the character in a string.

   pchBegin
      Pointer to the first character of the string to validate.
   pchEnd
      Pointer to beyond the last character of the string to validate.
   bThrowOnErrors
      If true, an exception of type abc::text::decode_error will be thrown if any invalid characters
      are found; otherwise the presence of errors will be reported via the return value of the
      function.
   return
      true if the string is valid UTF-8, or false otherwise.
   */
   static bool validate(
      char_t const * pchBegin, char_t const * pchEnd, bool bThrowOnErrors = false
   );
};

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

