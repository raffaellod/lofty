/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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

#include <abc/testing/test_case.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// Helpers


/** Generates a UTF-8 character literal followed by a comma.
*/
#define _ABC_CHAR8_COMMA(ch) char8_t(ch),


/** Generates a UTF-16 character literal followed by a comma.
*/
#define _ABC_CHAR16_COMMA(ch) char16_t(ch),


/** Generates a UTF-32 character literal followed by a comma.
*/
#define _ABC_CHAR32_COMMA(ch) char32_t(ch),


#if ABC_HOST_MSC
   // Silence warnings from system header files.

   // “cast truncates constant value”
   #pragma warning(disable: 4310)
#endif //if ABC_HOST_MSC



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::utf8_traits_validity_nult

namespace abc {

namespace test {

#define ABC_TESTING_ASSERT_text_utf8_traits_is_valid_nult(b, ...) \
   do { \
      /* Append to the string a second NUL terminator preceded by 6 nasty 0xff character, which will
      make is_valid() fail if they’re accessed, which would mean that is_valid() erroneously skipped
      past the first NUL terminator.
      Why six? Because that’s the longest, albeit invalid, encoding possible in UTF-8, so even the
      longest (wrong) jump will still land on one of these characters. */ \
      \
      static char8_t const psz[] = { \
         ABC_CPP_LIST_WALK(_ABC_CHAR8_COMMA, __VA_ARGS__) \
         char8_t(0xff), char8_t(0xff), char8_t(0xff), char8_t(0xff), char8_t(0xff), char8_t(0xff), \
         char8_t(0x00) \
      }; \
      this->ABC_CPP_CAT(assert_, b)( \
         ABC_SOURCE_LOCATION(), \
         text::utf8_traits::is_valid(psz), \
         SL("text::utf8_traits::is_valid(") SL(# __VA_ARGS__) SL(")") \
      ); \
   } while (false)

#define ABC_TESTING_ASSERT_TRUE_text_utf8_traits_is_valid_nult(...) \
   ABC_TESTING_ASSERT_text_utf8_traits_is_valid_nult(true, __VA_ARGS__)

#define ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_nult(...) \
   ABC_TESTING_ASSERT_text_utf8_traits_is_valid_nult(false, __VA_ARGS__)


class utf8_traits_validity_nult :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::text::utf8_traits - validity of NUL-terminated strings"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      // Strings here have a second NUL terminator preceded by 6 nasty 0xff character, which will
      // make is_valid() fail if they’re accessed, which would mean that is_valid() erroneously
      // skipped past the first NUL terminator.
      // Why six? Because that’s the longest, albeit invalid, character sequence possible in UTF-8.

      // Valid single character.
      ABC_TESTING_ASSERT_TRUE_text_utf8_traits_is_valid_nult(
         0x24, 0x00
      );
      // Increasing run lengths.
      ABC_TESTING_ASSERT_TRUE_text_utf8_traits_is_valid_nult(
         0x24, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2, 0x00
      );
      // Decreasing run lengths.
      ABC_TESTING_ASSERT_TRUE_text_utf8_traits_is_valid_nult(
         0xf0, 0xa4, 0xad, 0xa2, 0xe2, 0x82, 0xac, 0xc2, 0xa2, 0x24, 0x00
      );

      // Invalid single character.
      ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_nult(
         0x81, 0x00
      );
      // Invalid single character in the beginning of a valid string.
      ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_nult(
         0x81, 0x24, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2, 0x00
      );
      // Invalid single character at the end of a valid string.
      ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_nult(
         0x24, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2, 0x81, 0x00
      );

      // Invalid single overlong.
      ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_nult(
         0xc0, 0x81, 0x00
      );
      // Invalid single overlong in the beginning of a valid string.
      ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_nult(
         0xc0, 0x81, 0x24, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2, 0x00
      );
      // Invalid single overlong at the end of a valid string.
      ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_nult(
         0x24, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2, 0xc0, 0x81, 0x00
      );

      // Technically possible, but not valid UTF-8.
      // TODO: decide whether is_valid() should reject these strings.
      /*ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_nult(
         0xf9, 0x81, 0x81, 0x81, 0x81, 0x00
      );
      ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_nult(
         0xfd, 0x81, 0x81, 0x81, 0x81, 0x81, 0x00
      );*/

      // Technically possible, but not valid UTF-8. Here the string continues in a *valid* second
      // NUL-terminated string, so we can detect if the invalid byte was interpreted as the lead
      // byte of some UTF-8 sequence.
      ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_nult(
         0xfe, 0x00,
         0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x00
      );
      ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_nult(
         0xff, 0x00,
         0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x00
      );
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::utf8_traits_validity_nult)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::utf8_traits_validity_cch

namespace abc {

namespace test {

#define ABC_TESTING_ASSERT_text_utf8_traits_is_valid_cch(b, ...) \
   do { \
      /* Append to the strings 6 nasty 0xff character, which will make is_valid() fail if they’re
      accessed. We don’t include them in the count of characters to validate, but an off-by-one (or
      more) error will incorrectly access them, causing is_valid() to fail.
      Why six? Because that’s the longest, albeit invalid, encoding possible in UTF-8, so even the
      longest (wrong) jump will still land on one of these characters. */ \
      \
      static char8_t const ach[] = { \
         ABC_CPP_LIST_WALK(_ABC_CHAR8_COMMA, __VA_ARGS__) \
         char8_t(0xff), char8_t(0xff), char8_t(0xff), char8_t(0xff), char8_t(0xff), char8_t(0xff) \
      }; \
      this->ABC_CPP_CAT(assert_, b)( \
         ABC_SOURCE_LOCATION(), \
         text::utf8_traits::is_valid(ach, ach + ABC_COUNTOF(ach) - 6), \
         SL("text::utf8_traits::is_valid(") SL(# __VA_ARGS__) SL(")") \
      ); \
   } while (false)

#define ABC_TESTING_ASSERT_TRUE_text_utf8_traits_is_valid_cch(...) \
   ABC_TESTING_ASSERT_text_utf8_traits_is_valid_cch(true, __VA_ARGS__)

#define ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_cch(...) \
   ABC_TESTING_ASSERT_text_utf8_traits_is_valid_cch(false, __VA_ARGS__)


class utf8_traits_validity_cch :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::text::utf8_traits - validity of counted strings"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      // Valid single character.
      ABC_TESTING_ASSERT_TRUE_text_utf8_traits_is_valid_cch(
         0x01
      );
      // Increasing run lengths.
      ABC_TESTING_ASSERT_TRUE_text_utf8_traits_is_valid_cch(
         0x01, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2
      );
      // Decreasing run lengths.
      ABC_TESTING_ASSERT_TRUE_text_utf8_traits_is_valid_cch(
         0xf0, 0xa4, 0xad, 0xa2, 0xe2, 0x82, 0xac, 0xc2, 0xa2, 0x01
      );

      // Invalid single character.
      ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_cch(
         0x81
      );
      // Invalid single character in the beginning of a valid string.
      ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_cch(
         0x81, 0x01, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2
      );
      // Invalid single character at the end of a valid string.
      ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_cch(
         0x01, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2, 0x81
      );

      // Invalid single overlong.
      ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_cch(
         0xc0, 0x81
      );
      // Invalid single overlong in the beginning of a valid string.
      ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_cch(
         0xc0, 0x81, 0x01, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2
      );
      // Invalid single overlong at the end of a valid string.
      ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_cch(
         0x01, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2, 0xc0, 0x81
      );

      // Technically possible, but not valid UTF-8.
      // TODO: decide whether is_valid() should reject these strings.
      /*ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_cch(
         0xf9, 0x81, 0x81, 0x81, 0x81
      );
      ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_cch(
         0xfd, 0x81, 0x81, 0x81, 0x81, 0x81
      );*/

      // Technically possible, but not valid UTF-8. Here the string continues in a *valid* second
      // NUL-terminated string, so we can detect if the invalid byte was interpreted as the lead
      // byte of some UTF-8 sequence.
      ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_cch(
         0xfe,
         0x01, 0x01, 0x01, 0x01, 0x01, 0x01
      );
      ABC_TESTING_ASSERT_FALSE_text_utf8_traits_is_valid_cch(
         0xff,
         0x01, 0x01, 0x01, 0x01, 0x01, 0x01
      );
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::utf8_traits_validity_cch)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::utf16_traits_validity_nult

namespace abc {

namespace test {

#define ABC_TESTING_ASSERT_text_utf16_traits_is_valid_nult(b, ...) \
   do { \
      /* Append to the string a second NUL terminator preceded by 2 invalid lead surrogates, which
      will make is_valid() fail if they’re accessed, which would mean that is_valid() erroneously
      skipped past the first NUL terminator. */ \
      \
      static char16_t const psz[] = { \
         ABC_CPP_LIST_WALK(_ABC_CHAR16_COMMA, __VA_ARGS__) \
         char16_t(0xd834), char16_t(0xd834), char16_t(0x00) \
      }; \
      this->ABC_CPP_CAT(assert_, b)( \
         ABC_SOURCE_LOCATION(), \
         text::utf16_traits::is_valid(psz), \
         SL("text::utf16_traits::is_valid(") SL(# __VA_ARGS__) SL(")") \
      ); \
   } while (false)

#define ABC_TESTING_ASSERT_TRUE_text_utf16_traits_is_valid_nult(...) \
   ABC_TESTING_ASSERT_text_utf16_traits_is_valid_nult(true, __VA_ARGS__)

#define ABC_TESTING_ASSERT_FALSE_text_utf16_traits_is_valid_nult(...) \
   ABC_TESTING_ASSERT_text_utf16_traits_is_valid_nult(false, __VA_ARGS__)


class utf16_traits_validity_nult :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::text::utf16_traits - validity of NUL-terminated strings"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      // Valid single character.
      ABC_TESTING_ASSERT_TRUE_text_utf16_traits_is_valid_nult(0x007a, 0x0000);
      // Valid single character and surrogate pair.
      ABC_TESTING_ASSERT_TRUE_text_utf16_traits_is_valid_nult(0x007a, 0xd834, 0xdd1e, 0x0000);
      // Valid surrogate pair and single character.
      ABC_TESTING_ASSERT_TRUE_text_utf16_traits_is_valid_nult(0xd834, 0xdd1e, 0x007a, 0x0000);

      // Invalid lead surrogate.
      ABC_TESTING_ASSERT_FALSE_text_utf16_traits_is_valid_nult(0xd834, 0x0000);
      // Invalid lead surrogate in the beginning of a valid string.
      ABC_TESTING_ASSERT_FALSE_text_utf16_traits_is_valid_nult(0xd834, 0x0079, 0x007a, 0x0000);
      // Invalid lead surrogate at the end of a valid string.
      ABC_TESTING_ASSERT_FALSE_text_utf16_traits_is_valid_nult(0x0079, 0x007a, 0xd834, 0x0000);
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::utf16_traits_validity_nult)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::utf16_traits_validity_cch

namespace abc {

namespace test {

#define ABC_TESTING_ASSERT_text_utf16_traits_is_valid_cch(b, ...) \
   do { \
      /* Append to the string a second NUL terminator preceded by 2 invalid lead surrogates, which
      will make is_valid() fail if they’re accessed, which would mean that is_valid() erroneously
      skipped past the first NUL terminator. */ \
      \
      static char16_t const ach[] = { \
         ABC_CPP_LIST_WALK(_ABC_CHAR16_COMMA, __VA_ARGS__) char16_t(0xd834), char16_t(0xd834) \
      }; \
      this->ABC_CPP_CAT(assert_, b)( \
         ABC_SOURCE_LOCATION(), \
         text::utf16_traits::is_valid(ach, ach + ABC_COUNTOF(ach) - 2), \
         SL("text::utf16_traits::is_valid(") SL(# __VA_ARGS__) SL(")") \
      ); \
   } while (false)

#define ABC_TESTING_ASSERT_TRUE_text_utf16_traits_is_valid_cch(...) \
   ABC_TESTING_ASSERT_text_utf16_traits_is_valid_cch(true, __VA_ARGS__)

#define ABC_TESTING_ASSERT_FALSE_text_utf16_traits_is_valid_cch(...) \
   ABC_TESTING_ASSERT_text_utf16_traits_is_valid_cch(false, __VA_ARGS__)


class utf16_traits_validity_cch :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::text::utf16_traits - validity of counted strings"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      // Valid single character.
      ABC_TESTING_ASSERT_TRUE_text_utf16_traits_is_valid_cch(0x007a);
      // Valid single character and surrogate pair.
      ABC_TESTING_ASSERT_TRUE_text_utf16_traits_is_valid_cch(0x007a, 0xd834, 0xdd1e);
      // Valid surrogate pair and single character.
      ABC_TESTING_ASSERT_TRUE_text_utf16_traits_is_valid_cch(0xd834, 0xdd1e, 0x007a);

      // Invalid lead surrogate.
      ABC_TESTING_ASSERT_FALSE_text_utf16_traits_is_valid_cch(0xd834);
      // Invalid lead surrogate in the beginning of a valid string.
      ABC_TESTING_ASSERT_FALSE_text_utf16_traits_is_valid_cch(0xd834, 0x0079, 0x007a);
      // Invalid lead surrogate at the end of a valid string.
      ABC_TESTING_ASSERT_FALSE_text_utf16_traits_is_valid_cch(0x0079, 0x007a, 0xd834);
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::utf16_traits_validity_cch)


////////////////////////////////////////////////////////////////////////////////////////////////////

