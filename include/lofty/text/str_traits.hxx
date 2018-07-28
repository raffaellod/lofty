/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_TEXT_STR_TRAITS_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_TEXT_STR_TRAITS_HXX
#endif

#ifndef _LOFTY_TEXT_STR_TRAITS_HXX_NOPUB
#define _LOFTY_TEXT_STR_TRAITS_HXX_NOPUB

#include <lofty/collections/vector-0.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {
_LOFTY_PUBNS_BEGIN

/*! Low-level functions for dealing with character strings; used by lofty::text::*str. Note that this class is
not modeled after std::char_traits. */
class LOFTY_SYM str_traits {
public:
   /*! Builds a failure restart table for searches using the Knuth-Morris-Pratt algorithm. Each element in the
   returned vector is the count of characters that won’t be compared again in case a partial substring match
   is found to be not a match.

   @param substr_begin
      Pointer to the beginning of the search string.
   @param substr_end
      Pointer to the end of the search string.
   @param failure_restarts
      Pointer to a vector that will receive failure restart indices. Examples for different substrs:
      @verbatim
      ┌──────────────────┬───┬─────┬─────┬───────┬───────┬───────────────┬─────────────┐
      │ Substr index     │ 0 │ 0 1 │ 0 1 │ 0 1 2 │ 0 1 2 │ 0 1 2 3 4 5 6 │ 0 1 2 3 4 5 │
      ├──────────────────┼───┼─────┼─────┼───────┼───────┼───────────────┼─────────────┤
      │ substr_begin-end │ A │ A A │ A B │ A A A │ A A B │ A B A A B A C │ A B A B C D │
      │ failure_restarts │ 0 │ 0 0 │ 0 0 │ 0 0 0 │ 0 0 0 │ 0 0 0 0 1 2 3 │ 0 0 0 1 2 0 │
      └──────────────────┴───┴─────┴─────┴───────┴───────┴───────────────┴─────────────┘
      @endverbatim
   */
   static void _build_find_failure_restart_table(
      char_t const * substr_begin, char_t const * substr_end,
      collections::_LOFTY_PUBNS vector<std::size_t> * failure_restarts
   );

   /*! Compares two strings.

   @param left_begin
      Pointer to the first character of the left string.
   @param left_end
      Pointer to beyond the last character of the left string.
   @param right_begin
      Pointer to the first character of the right string.
   @param right_end
      Pointer to beyond the last character of the right string.
   @return
      Standard comparison result integer:
      •  > 0 if left >  right;
      •    0 if left == right;
      •  < 0 if left <  right.
   */
   static int compare(
      char_t const * left_begin, char_t const * left_end, char_t const * right_begin, char_t const * right_end
   );

   /*! Returns a pointer to the first occurrence of a character in a string, or str_end if no matches are
   found.

   @param str_begin
      Pointer to the first character of the string to be searched.
   @param str_end
      Pointer to beyond the last character of the string to be searched.
   @param ch
      Character to search for.
   @return
      Pointer to the beginning of the first match, in the string to be searched, of the code point to search
      for, or nullptr if no matches are found.
   */
   static char_t const * find_char(char_t const * str_begin, char_t const * str_end, char_t ch) {
      for (auto s = str_begin; s < str_end; ++s) {
         if (*s == ch) {
            return s;
         }
      }
      return str_end;
   }

   /*! Returns a pointer to the first occurrence of a code point in a string, or str_end if no matches are
   found.

   @param str_begin
      Pointer to the first character of the string to be searched.
   @param str_end
      Pointer to beyond the last character of the string to be searched.
   @param cp
      Code point to search for.
   @return
      Pointer to the beginning of the first match, in the string to be searched, of the code point to search
      for, or nullptr if no matches are found.
   */
   static char_t const * find_char(char_t const * str_begin, char_t const * str_end, char32_t cp);

   /*! Returns a pointer to the first occurrence of a char_t-encoded code point in a string, or str_end if no
   matches are found.

   @param str_begin
      Pointer to the first character of the string to be searched.
   @param str_end
      Pointer to beyond the last character of the string to be searched.
   @param cp_chars
      Pointer to the encoded code point (UTF character sequence) to search for; its length is deduced
      automatically.
   @return
      Pointer to the beginning of the first match, in the string to be searched, of the code point to search
      for, or nullptr if no matches are found.
   */
   static char_t const * find_char(char_t const * str_begin, char_t const * str_end, char_t const * cp_chars);

   /*! Returns a pointer to the last occurrence of a character in a string, or str_begin if no matches are
   found.

   @param str_begin
      Pointer to the first character of the string to be searched.
   @param str_end
      Pointer to beyond the last character of the string to be searched.
   @param cp_chars
      Character to search for.
   @return
      Pointer to the beginning of the last match, in the string to be searched, of the character to search
      for, or nullptr if no matches are found.
   */
   static char_t const * find_char_last(char_t const * str_begin, char_t const * str_end, char_t cp_chars) {
      for (auto s = str_end; s > str_begin; ) {
         if (*--s == cp_chars) {
            return s;
         }
      }
      return str_begin;
   }

   /*! Returns a pointer to the last occurrence of a code point in a string, or str_begin if no matches are
   found.

   @param str_begin
      Pointer to the first character of the string to be searched.
   @param str_end
      Pointer to beyond the last character of the string to be searched.
   @param cp
      Code point to search for.
   @return
      Pointer to the beginning of the last match, in the string to be searched, of the code point to search
      for, or nullptr if no matches are found.
   */
   static char_t const * find_char_last(char_t const * str_begin, char_t const * str_end, char32_t cp);

   /*! Returns the character index of the first occurrence of a string into another.

   @param str_begin
      Pointer to the first character of the string to be searched.
   @param str_end
      Pointer to beyond the last character of the string to be searched.
   @param substr_begin
      Pointer to the first character of the string to search for.
   @param substr_end
      Pointer to beyond the last character of the string to search for.
   @return
      Pointer to the beginning of the first match, in the string to be searched, of the string to search for,
      or nullptr if no matches are found.
   */
   static char_t const * find_substr(
      char_t const * str_begin, char_t const * str_end, char_t const * substr_begin, char_t const * substr_end
   );

   /*! Returns the character index of the last occurrence of a string into another.

   @param str_begin
      Pointer to the first character of the string to be searched.
   @param str_end
      Pointer to beyond the last character of the string to be searched.
   @param substr_begin
      Pointer to the first character of the string to search for.
   @param substr_end
      Pointer to beyond the last character of the string to search for.
   @return
      Pointer to the beginning of the last match, in the string to be searched, of the string to search for,
      or nullptr if no matches are found.
   */
   static char_t const * find_substr_last(
      char_t const * str_begin, char_t const * str_end, char_t const * substr_begin, char_t const * substr_end
   );

   /*! Returns count of code points in a string.

   @param begin
      Pointer to the beginning of the string.
   @param end
      Pointer to the end of the string.
   @return
      Count of code points included in the string.
   */
   static std::size_t size_in_codepoints(char_t const * begin, char_t const * end);

   /*! Validates the character in a string.

   @param begin
      Pointer to the first character of the string to validate.
   @param end
      Pointer to beyond the last character of the string to validate.
   @param throw_on_errors
      If true, an exception of type lofty::text::decode_error will be thrown if any invalid characters are
      found; otherwise the presence of errors will be reported via the return value of the function.
   @return
      true if the string is valid UTF-8, or false otherwise.
   */
   static bool validate(char_t const * begin, char_t const * end, bool throw_on_errors = false);
};

_LOFTY_PUBNS_END
}} //namespace lofty::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TEXT_STR_TRAITS_HXX_NOPUB

#ifdef _LOFTY_TEXT_STR_TRAITS_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace text {

   using _pub::str_traits;

   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_TEXT_STR_TRAITS_HXX
