/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2015 Raffaello D. Di Napoli

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

#include <abaclade.hxx>
#include <abaclade/testing/test_case.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

//! Generates an abc::text::char_t literal followed by a comma.
#define _ABC_CHAR_COMMA(ch) ::abc::text::char_t(ch),

#if ABC_HOST_UTF == 8
   #define ABC_TESTING_ASSERT_text_str_traits_validate(b, ...) \
      do { \
         /* Append to the strings 6 nasty 0xff character, which will make validate() fail if they’re
         accessed. We don’t include them in the count of characters to validate, but an off-by-one
         (or more) error will incorrectly access them, causing validate() to fail.
         Why six? Because that’s the longest, albeit invalid, encoding possible in UTF-8, so even
         the longest (wrong) jump will still land on one of these characters. */ \
         \
         static char8_t const ach[] = { \
            ABC_CPP_LIST_WALK(_ABC_CHAR_COMMA, __VA_ARGS__) \
            char8_t(0xff), char8_t(0xff), char8_t(0xff), char8_t(0xff), char8_t(0xff), \
               char8_t(0xff) \
         }; \
         this->ABC_CPP_CAT(assert_, b)( \
            ABC_THIS_FILE_ADDRESS(), \
            text::str_traits::validate(ach, ach + ABC_COUNTOF(ach) - 6), \
            ABC_SL("text::str_traits::validate(") ABC_SL(# __VA_ARGS__) ABC_SL(")") \
         ); \
      } while (false)
#elif ABC_HOST_UTF == 16 //if ABC_HOST_UTF == 8
   #define ABC_TESTING_ASSERT_text_str_traits_validate(b, ...) \
      do { \
         /* Append to the string a second NUL terminator preceded by 2 invalid lead surrogates,
         which will make validate() fail if they’re accessed, which would mean that validate()
         erroneously skipped past the first NUL terminator. */ \
         \
         static char16_t const ach[] = { \
            ABC_CPP_LIST_WALK(_ABC_CHAR_COMMA, __VA_ARGS__) char16_t(0xd834), char16_t(0xd834) \
         }; \
         this->ABC_CPP_CAT(assert_, b)( \
            ABC_THIS_FILE_ADDRESS(), \
            text::str_traits::validate(ach, ach + ABC_COUNTOF(ach) - 2), \
            ABC_SL("text::str_traits::validate(") ABC_SL(# __VA_ARGS__) ABC_SL(")") \
         ); \
      } while (false)
#endif //if ABC_HOST_UTF == 8 … elif ABC_HOST_UTF == 16

#define ABC_TESTING_ASSERT_TRUE_text_str_traits_validate(...) \
   ABC_TESTING_ASSERT_text_str_traits_validate(true, __VA_ARGS__)

#define ABC_TESTING_ASSERT_FALSE_text_str_traits_validate(...) \
   ABC_TESTING_ASSERT_text_str_traits_validate(false, __VA_ARGS__)

ABC_TESTING_TEST_CASE_FUNC(
   text_str_traits_validation,
   "abc::text::str_traits – validity of counted strings"
) {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_UTF == 8

   // Valid single character.
   ABC_TESTING_ASSERT_TRUE_text_str_traits_validate(0x01);
   // Increasing run lengths.
   ABC_TESTING_ASSERT_TRUE_text_str_traits_validate(
      0x01, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2
   );
   // Decreasing run lengths.
   ABC_TESTING_ASSERT_TRUE_text_str_traits_validate(
      0xf0, 0xa4, 0xad, 0xa2, 0xe2, 0x82, 0xac, 0xc2, 0xa2, 0x01
   );

   // Invalid single character.
   ABC_TESTING_ASSERT_FALSE_text_str_traits_validate(0x81);
   // Invalid single character in the beginning of a valid string.
   ABC_TESTING_ASSERT_FALSE_text_str_traits_validate(
      0x81, 0x01, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2
   );
   // Invalid single character at the end of a valid string.
   ABC_TESTING_ASSERT_FALSE_text_str_traits_validate(
      0x01, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2, 0x81
   );

   // Invalid single overlong.
   ABC_TESTING_ASSERT_FALSE_text_str_traits_validate(0xc0, 0x81);
   // Invalid single overlong in the beginning of a valid string.
   ABC_TESTING_ASSERT_FALSE_text_str_traits_validate(
      0xc0, 0x81, 0x01, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2
   );
   // Invalid single overlong at the end of a valid string.
   ABC_TESTING_ASSERT_FALSE_text_str_traits_validate(
      0x01, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2, 0xc0, 0x81
   );

   // Technically possible, but not valid UTF-8.
   ABC_TESTING_ASSERT_FALSE_text_str_traits_validate(0xf9, 0x81, 0x81, 0x81, 0x81);
   ABC_TESTING_ASSERT_FALSE_text_str_traits_validate(0xfd, 0x81, 0x81, 0x81, 0x81, 0x81);

   /* Technically possible, but not valid UTF-8. Here the string continues with a few more valid
   characters, so we can detect if the invalid byte was interpreted as the lead byte of some
   UTF-8 sequence. */
   ABC_TESTING_ASSERT_FALSE_text_str_traits_validate(0xfe, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01);
   ABC_TESTING_ASSERT_FALSE_text_str_traits_validate(0xff, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01);

#elif ABC_HOST_UTF == 16 //if ABC_HOST_UTF == 8

   // Valid single character.
   ABC_TESTING_ASSERT_TRUE_text_str_traits_validate(0x007a);
   // Valid single character and surrogate pair.
   ABC_TESTING_ASSERT_TRUE_text_str_traits_validate(0x007a, 0xd834, 0xdd1e);
   // Valid surrogate pair and single character.
   ABC_TESTING_ASSERT_TRUE_text_str_traits_validate(0xd834, 0xdd1e, 0x007a);

   // Invalid lead surrogate.
   ABC_TESTING_ASSERT_FALSE_text_str_traits_validate(0xd834);
   // Invalid lead surrogate in the beginning of a valid string.
   ABC_TESTING_ASSERT_FALSE_text_str_traits_validate(0xd834, 0x0079, 0x007a);
   // Invalid lead surrogate at the end of a valid string.
   ABC_TESTING_ASSERT_FALSE_text_str_traits_validate(0x0079, 0x007a, 0xd834);

#endif //if ABC_HOST_UTF == 8 … elif ABC_HOST_UTF == 16
}

#undef ABC_TESTING_ASSERT_text_str_traits_validate
#undef ABC_TESTING_ASSERT_TRUE_text_str_traits_validate
#undef ABC_TESTING_ASSERT_FALSE_text_str_traits_validate

}} //namespace abc::test
