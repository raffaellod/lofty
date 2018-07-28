/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/logging.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/text.hxx>
#include <lofty/text/str_traits.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

#if LOFTY_HOST_UTF == 8
   /* Append to the strings 6 nasty 0xff character, which will make validate() fail if they’re accessed.
   We don’t include them in the count of characters to validate, but an off-by-one (or more) error will
   incorrectly access them, causing validate() to fail.
   Why 6? Because that’s the longest, albeit invalid, encoding possible in UTF-8, so even the longest
   (wrong) jump will still land on one of these characters. */ \
   #define GARBAGE_CHARS \
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff
#elif LOFTY_HOST_UTF == 16
   /* Append to the string a second NUL terminator preceded by 2 invalid lead surrogates, which will make
   validate() fail if they’re accessed, which would mean that validate() erroneously skipped past the first
   NUL terminator. */ \
   #define GARBAGE_CHARS \
      0xd834, 0xd834
#endif

static text::char_t chars_buf[32], * char_ptr;
#define CHAR_COPY(char) *char_ptr++ = text::char_t(char),

#define CL(...) \
   chars_buf, chars_buf + ( \
      char_ptr = chars_buf, \
      LOFTY_CPP_LIST_WALK(CHAR_COPY, __VA_ARGS__) \
      LOFTY_CPP_LIST_WALK(CHAR_COPY, GARBAGE_CHARS) \
      LOFTY_CPP_LIST_COUNT(__VA_ARGS__) \
   )

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_traits_validation,
   "lofty::text::str_traits – validity of counted strings"
) {
   LOFTY_TRACE_FUNC();

#if LOFTY_HOST_UTF == 8

   // Valid single character.
   ASSERT(text::str_traits::validate(CL(0x01)));
   // Increasing run lengths.
   ASSERT(text::str_traits::validate(CL(0x01, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2)));
   // Decreasing run lengths.
   ASSERT(text::str_traits::validate(CL(0xf0, 0xa4, 0xad, 0xa2, 0xe2, 0x82, 0xac, 0xc2, 0xa2, 0x01)));

   // Invalid single character.
   ASSERT(!text::str_traits::validate(CL(0x81)));
   // Invalid single character in the beginning of a valid string.
   ASSERT(!text::str_traits::validate(CL(0x81, 0x01, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2)));
   // Invalid single character at the end of a valid string.
   ASSERT(!text::str_traits::validate(CL(0x01, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2, 0x81)));

   // Invalid single overlong.
   ASSERT(!text::str_traits::validate(CL(0xc0, 0x81)));
   // Invalid single overlong in the beginning of a valid string.
   ASSERT(!text::str_traits::validate(CL(
      0xc0, 0x81, 0x01, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2
   )));
   // Invalid single overlong at the end of a valid string.
   ASSERT(!text::str_traits::validate(CL(
      0x01, 0xc2, 0xa2, 0xe2, 0x82, 0xac, 0xf0, 0xa4, 0xad, 0xa2, 0xc0, 0x81
   )));

   // Technically possible, but not valid UTF-8.
   ASSERT(!text::str_traits::validate(CL(0xf9, 0x81, 0x81, 0x81, 0x81)));
   ASSERT(!text::str_traits::validate(CL(0xfd, 0x81, 0x81, 0x81, 0x81, 0x81)));

   /* Technically possible, but not valid UTF-8. Here the string continues with a few more valid characters,
   so we can detect if the invalid byte was interpreted as the lead byte of some UTF-8 sequence. */
   ASSERT(!text::str_traits::validate(CL(0xfe, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01)));
   ASSERT(!text::str_traits::validate(CL(0xff, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01)));

#elif LOFTY_HOST_UTF == 16 //if LOFTY_HOST_UTF == 8

   // Valid single character.
   ASSERT(text::str_traits::validate(CL(0x007a)));
   // Valid single character and surrogate pair.
   ASSERT(text::str_traits::validate(CL(0x007a, 0xd834, 0xdd1e)));
   // Valid surrogate pair and single character.
   ASSERT(text::str_traits::validate(CL(0xd834, 0xdd1e, 0x007a)));

   // Invalid lead surrogate.
   ASSERT(!text::str_traits::validate(CL(0xd834)));
   // Invalid lead surrogate in the beginning of a valid string.
   ASSERT(!text::str_traits::validate(CL(0xd834, 0x0079, 0x007a)));
   // Invalid lead surrogate at the end of a valid string.
   ASSERT(!text::str_traits::validate(CL(0x0079, 0x007a, 0xd834)));

#endif //if LOFTY_HOST_UTF == 8 … elif LOFTY_HOST_UTF == 16
}

}} //namespace lofty::test
