/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/collections.hxx>
#include <lofty/collections/vector.hxx>
#include <lofty/from_str.hxx>
#include <lofty/logging.hxx>
#include <lofty/_std/utility.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/testing/utility.hxx>
#include <lofty/text.hxx>
#include <lofty/text/str.hxx>
#include <lofty/to_str.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

//! Unicode Plane 0 code point.
char32_t const plane0_cp(LOFTY_CHAR('\x20ac'));
//! Unicode Plane 2 code point.
char32_t const plane2_cp(0x024b62);

/*! Returns the special string “acabaabca”, which has the following properties:
•  misleading start for “ab” at index 0 (it’s “ac” instead) and for “abc” at index 2 (it’s “aba” instead), to
   catch incorrect skip-last comparisons;
•  first and last characters match 'a', but other inner ones do too;
•  would match “abcd” were it not for the last character;
•  matches the self-repeating “abaabc” but not the (also self-repeating) “abaabcd”.
The only thing though is that we replace ‘b’ with the Unicode Plane 2 character defined above and ‘c’ with the
BMP (Plane 0) character above.

@return
   String described above.
*/
static text::str get_acabaabca() {
   return text::str::empty + 'a' + plane0_cp + 'a' + plane2_cp + 'a' + 'a' + plane2_cp + plane0_cp + 'a';
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_basic,
   "lofty::text::str – basic operations"
) {
   LOFTY_TRACE_FUNC();

   text::str s;
   auto tracker(testing::utility::make_container_data_ptr_tracker(&s));

   s += LOFTY_SL("ä");
   // true: operator+= must have created an item array (there was none).
   ASSERT(tracker.changed());
   ASSERT_THROWS(collections::out_of_range, s[-1]);
   ASSERT_DOES_NOT_THROW(s[0]);
   ASSERT_THROWS(collections::out_of_range, s[1]);
   ASSERT(s.size() == 1u);
   ASSERT(s.capacity() >= 1u);
   ASSERT(s[0] == LOFTY_CHAR('ä'));

   s = s + 'b' + s;
   // true: a new string is created by operator+, which replaces s by operator=.
   ASSERT(tracker.changed());
   ASSERT(s.size() == 3u);
   ASSERT(s.capacity() >= 3u);
   ASSERT(s == LOFTY_SL("äbä"));

   s = s.substr(s.cbegin() + 1, s.cbegin() + 3);
   // true: s got replaced by operator=.
   ASSERT(tracker.changed());
   ASSERT(s.size() == 2u);
   ASSERT(s.capacity() >= 2u);
   ASSERT(s == LOFTY_SL("bä"));

   s += 'c';
   // false: there should’ve been enough space for 'c'.
   ASSERT(!tracker.changed());
   ASSERT(s.size() == 3u);
   ASSERT(s.capacity() >= 3u);
   ASSERT(s == LOFTY_SL("bäc"));

   s = s.substr(s.cbegin(), s.cend() - 1);
   // true: s got replaced by operator=.
   ASSERT(tracker.changed());
   ASSERT(s.size() == 2u);
   ASSERT(s.capacity() >= 2u);
   ASSERT(s[0] == 'b');
   ASSERT(s[1] == LOFTY_CHAR('ä'));

   s += s;
   // false: there should’ve been enough space for “baba”.
   ASSERT(!tracker.changed());
   ASSERT(s.size() == 4u);
   ASSERT(s.capacity() >= 4u);
   ASSERT(s[0] == 'b');
   ASSERT(s[1] == LOFTY_CHAR('ä'));
   ASSERT(s[2] == 'b');
   ASSERT(s[3] == LOFTY_CHAR('ä'));

   s = s.substr(s.cend() - 3, s.cend() - 2);
   // true: s got replaced by operator=.
   ASSERT(tracker.changed());
   ASSERT(s.size() == 1u);
   ASSERT(s.capacity() >= 1u);
   ASSERT(s[0] == LOFTY_CHAR('ä'));

   s = text::str(LOFTY_SL("ab")) + 'c';
   // true: s got replaced by operator=.
   ASSERT(tracker.changed());
   ASSERT(s.size() == 3u);
   ASSERT(s.capacity() >= 3u);
   ASSERT(s[0] == 'a');
   ASSERT(s[1] == 'b');
   ASSERT(s[2] == 'c');

   s += 'd';
   // false: there should’ve been enough space for “abcd”.
   ASSERT(!tracker.changed());
   ASSERT(s.size() == 4u);
   ASSERT(s.capacity() >= 4u);
   ASSERT(s[0] == 'a');
   ASSERT(s[1] == 'b');
   ASSERT(s[2] == 'c');
   ASSERT(s[3] == 'd');

   s += LOFTY_SL("efghijklmnopqrstuvwxyz");
   /* Cannot ASSERT() on this to behave in any specific way, since the character array may or may not change
   depending on heap reallocation strategy. */
   tracker.changed();
   ASSERT(s.size() == 26u);
   ASSERT(s.capacity() >= 26u);
   ASSERT(s == LOFTY_SL("abcdefghijklmnopqrstuvwxyz"));

   s = LOFTY_SL("a\0b");
   // true: s got replaced by operator=.
   ASSERT(tracker.changed());
   s += LOFTY_SL("\0ç");
   // true: switched to writable copy.
   ASSERT(tracker.changed());
   ASSERT(s.size() == 5u);
   ASSERT(s.capacity() >= 5u);
   // Test both ways to make sure that the char_t[] overload is always chosen over char *.
   ASSERT(s == LOFTY_SL("a\0b\0ç"));
   ASSERT(LOFTY_SL("a\0b\0ç") == s);

   // Now that the string is not empty, validate that clear() truncates it without freeing its buffer.
   s.clear();
   ASSERT(s.size() == 0u);
   ASSERT(s.capacity() > 0u);

   {
      /* Note: all string operations here must involve as few characters as possible to avoid triggering a
      reallocation, which would break these tests. */

      text::str s1(LOFTY_SL("a"));
      // Write to the string to force it to stop using the string literal “a”.
      s1[0] = 'b';
      text::char_t const * old_data = s1.data();
      // Verify that the compiler selects operator+(str &&, …) when possible.
      text::str s2 = _std::move(s1) + LOFTY_SL("c");
      ASSERT(s2.data() == old_data);
   }

   // While we’re at it, let’s also validate acabaabca.
   s = get_acabaabca();
   ASSERT(s[0] == 'a');
   ASSERT(s[1] == plane0_cp);
   ASSERT(s[2] == 'a');
   ASSERT(s[3] == plane2_cp);
   ASSERT(s[4] == 'a');
   ASSERT(s[5] == 'a');
   ASSERT(s[6] == plane2_cp);
   ASSERT(s[7] == plane0_cp);
   ASSERT(s[8] == 'a');
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_iterators,
   "lofty::text::str – operations with iterators"
) {
   LOFTY_TRACE_FUNC();

   // Default-constructed iterator.
   text::str::const_iterator itr;
   ASSERT_THROWS(collections::out_of_range, *itr);
   ASSERT_THROWS(collections::out_of_range, --itr);
   ASSERT_THROWS(collections::out_of_range, ++itr);
   ASSERT_THROWS(collections::out_of_range, --itr);
   ASSERT_THROWS(collections::out_of_range, ++itr);
   ASSERT_THROWS(collections::out_of_range, itr[-1]);
   ASSERT_THROWS(collections::out_of_range, itr[0]);
   ASSERT_THROWS(collections::out_of_range, itr[1]);

   text::str s;
   ASSERT(s.cbegin() == s.end());

   // No accessible characters.
   ASSERT_THROWS(collections::out_of_range, s[-1]);
   ASSERT_THROWS(collections::out_of_range, s[0]);
   ASSERT_THROWS(collections::out_of_range, s[1]);

   // Should not allow to move an iterator to outside [begin, end].
   ASSERT_DOES_NOT_THROW(s.cbegin());
   ASSERT_DOES_NOT_THROW(s.cend());
   ASSERT_THROWS(collections::out_of_range, --s.cbegin());
   ASSERT_THROWS(collections::out_of_range, ++s.cbegin());
   ASSERT_THROWS(collections::out_of_range, --s.cend());
   ASSERT_THROWS(collections::out_of_range, ++s.cend());
   ASSERT_THROWS(collections::out_of_range, s.cbegin()[-1]);
   ASSERT_THROWS(collections::out_of_range, s.cbegin()[0]);
   ASSERT_THROWS(collections::out_of_range, s.cbegin()[1]);

   // Should not allow to dereference begin() or end() of an empty string.
   ASSERT_THROWS(collections::out_of_range, *s.cbegin());
   ASSERT_THROWS(collections::out_of_range, *s.cend());

   s += 'a';
   ASSERT(s.begin() != s.cend());

   // One accessible character.
   ASSERT_THROWS(collections::out_of_range, s[-1]);
   ASSERT_DOES_NOT_THROW(s[0]);
   ASSERT_THROWS(collections::out_of_range, s[1]);

   // Should not allow to move an iterator to outside [begin, end].
   ASSERT_THROWS(collections::out_of_range, --s.cbegin());
   ASSERT_DOES_NOT_THROW(++s.cbegin());
   ASSERT_DOES_NOT_THROW(--s.cend());
   ASSERT_THROWS(collections::out_of_range, ++s.cend());
   ASSERT_THROWS(collections::out_of_range, s.cbegin()[-1]);
   ASSERT_DOES_NOT_THROW(s.cbegin()[0]);
   ASSERT_THROWS(collections::out_of_range, s.cbegin()[1]);

   // Should allow to dereference begin(), but not end() of a non-empty string.
   ASSERT_DOES_NOT_THROW(*s.cbegin());
   ASSERT_THROWS(collections::out_of_range, *s.cend());
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_transcoding,
   "lofty::text::str – conversion to different encodings"
) {
   LOFTY_TRACE_FUNC();

   text::sstr<32> s;
   s += char32_t(0x000024);
   s += char32_t(0x0000a2);
   s += char32_t(0x0020ac);
   s += char32_t(0x024b62);
   collections::vector<std::uint8_t> encoded_bytes;

   encoded_bytes = s.encode(text::encoding::utf8, false);
   {
      collections::vector<std::uint8_t, 16> utf8_bytes;
      utf8_bytes.push_back(0x24);
      utf8_bytes.push_back(0xc2);
      utf8_bytes.push_back(0xa2);
      utf8_bytes.push_back(0xe2);
      utf8_bytes.push_back(0x82);
      utf8_bytes.push_back(0xac);
      utf8_bytes.push_back(0xf0);
      utf8_bytes.push_back(0xa4);
      utf8_bytes.push_back(0xad);
      utf8_bytes.push_back(0xa2);
      ASSERT(encoded_bytes == utf8_bytes);
   }

   encoded_bytes = s.encode(text::encoding::utf16be, false);
   {
      collections::vector<std::uint8_t, 16> utf16_bytes;
      utf16_bytes.push_back(0x00);
      utf16_bytes.push_back(0x24);
      utf16_bytes.push_back(0x00);
      utf16_bytes.push_back(0xa2);
      utf16_bytes.push_back(0x20);
      utf16_bytes.push_back(0xac);
      utf16_bytes.push_back(0xd8);
      utf16_bytes.push_back(0x52);
      utf16_bytes.push_back(0xdf);
      utf16_bytes.push_back(0x62);
      ASSERT(encoded_bytes == utf16_bytes);
   }

   encoded_bytes = s.encode(text::encoding::utf32le, false);
   {
      collections::vector<std::uint8_t, 16> utf32_bytes;
      utf32_bytes.push_back(0x24);
      utf32_bytes.push_back(0x00);
      utf32_bytes.push_back(0x00);
      utf32_bytes.push_back(0x00);
      utf32_bytes.push_back(0xa2);
      utf32_bytes.push_back(0x00);
      utf32_bytes.push_back(0x00);
      utf32_bytes.push_back(0x00);
      utf32_bytes.push_back(0xac);
      utf32_bytes.push_back(0x20);
      utf32_bytes.push_back(0x00);
      utf32_bytes.push_back(0x00);
      utf32_bytes.push_back(0x62);
      utf32_bytes.push_back(0x4b);
      utf32_bytes.push_back(0x02);
      utf32_bytes.push_back(0x00);
      ASSERT(encoded_bytes == utf32_bytes);
   }
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_char_replacement,
   "lofty::text::str – character replacement"
) {
   LOFTY_TRACE_FUNC();

   text::sstr<8> s;

   // No replacements to be made.
   ASSERT(((s = LOFTY_SL("aaa")).replace('b', 'c'), s) == LOFTY_SL("aaa"));
   // Simple ASCII-to-ASCII replacement: no size change.
   ASSERT(((s = LOFTY_SL("aaa")).replace('a', 'b'), s) == LOFTY_SL("bbb"));
   /* Complex ASCII-to-char32_t replacement: size will increase beyond the embedded capacity, so the iterator
   used in lofty::text::str::replace() must be intelligent enough to self-refresh with the new descriptor. */
   ASSERT(
      ((s = LOFTY_SL("aaaaa")).replace(char32_t('a'), plane2_cp), s) ==
      text::str::empty + plane2_cp + plane2_cp + plane2_cp + plane2_cp + plane2_cp
   );
   // Less-complex char32_t-to-ASCII replacement: size will decrease.
   s = text::str::empty + plane2_cp + plane2_cp + plane2_cp + plane2_cp + plane2_cp;
   ASSERT((s.replace(plane2_cp, char32_t('a')), s) == LOFTY_SL("aaaaa"));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_c_str,
   "lofty::text::str – C string extraction"
) {
   LOFTY_TRACE_FUNC();

   text::str s;
   // Note: storing its return value in a variable is NOT a way to use c_str().
   auto c_str(const_cast<text::str const &>(s).c_str());
   // s has no character array, so it should have returned the static NUL character.
   ASSERT(static_cast<text::char_t const *>(c_str) == text::str::empty.data());
   ASSERT(!c_str._get().get_deleter().enabled());
   ASSERT(text::size_in_chars(c_str) == 0u);
   ASSERT(c_str[0] == '\0');

   s = LOFTY_SL("");
   c_str = const_cast<text::str const &>(s).c_str();
   /* s should have adopted the literal and therefore have a trailing NUL, so it should have returned its own
   character array. */
   ASSERT(static_cast<text::char_t const *>(c_str) == s.data());
   ASSERT(!c_str._get().get_deleter().enabled());
   ASSERT(text::size_in_chars(c_str) == 0u);
   ASSERT(c_str[0] == '\0');

   s = LOFTY_SL("a");
   c_str = const_cast<text::str const &>(s).c_str();
   /* s should have adopted the literal and therefore have a trailing NUL, so it should have returned its own
   character array. */
   ASSERT(static_cast<text::char_t const *>(c_str) == s.data());
   ASSERT(!c_str._get().get_deleter().enabled());
   ASSERT(text::size_in_chars(c_str) == 1u);
   ASSERT(c_str[0] == 'a');
   ASSERT(c_str[1] == '\0');

   s = text::str::empty;
   c_str = s.c_str();
   // s has no character array, so it should have returned the static NUL character.
   ASSERT(static_cast<text::char_t const *>(c_str) == text::str::empty.data());
   ASSERT(!c_str._get().get_deleter().enabled());
   ASSERT(text::size_in_chars(c_str) == 0u);
   ASSERT(c_str[0] == '\0');

   s = LOFTY_SL("");
   c_str = s.c_str();
   /* s should have adopted the literal and therefore have a trailing NUL, so it should have returned its own
   character array. */
   ASSERT(static_cast<text::char_t const *>(c_str) == s.data());
   ASSERT(!c_str._get().get_deleter().enabled());
   ASSERT(text::size_in_chars(c_str) == 0u);
   ASSERT(c_str[0] == '\0');

   s = LOFTY_SL("a");
   c_str = s.c_str();
   /* s should have copied the literal but dropped its trailing NUL, to then add it back when c_str() was
   called. */
   ASSERT(static_cast<text::char_t const *>(c_str) == s.data());
   ASSERT(!c_str._get().get_deleter().enabled());
   ASSERT(text::size_in_chars(c_str) == 1u);
   ASSERT(c_str[0] == 'a');
   ASSERT(c_str[1] == '\0');

   s += LOFTY_SL("b");
   c_str = s.c_str();
   // The character array should have grown, to then include a trailing NUL when c_str() was called.
   ASSERT(static_cast<text::char_t const *>(c_str) == s.data());
   ASSERT(!c_str._get().get_deleter().enabled());
   ASSERT(text::size_in_chars(c_str) == 2u);
   ASSERT(c_str[0] == 'a');
   ASSERT(c_str[1] == 'b');
   ASSERT(c_str[2] == '\0');
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_find,
   "lofty::text::str – character and substring search"
) {
   LOFTY_TRACE_FUNC();

   // Special characters.
   char32_t cp0 = plane0_cp;
   char32_t cp2 = plane2_cp;
   /* See get_acabaabca() for more information on its pattern. To make it more interesting, here we also
   duplicate it. */
   text::str const s(get_acabaabca() + get_acabaabca());

   ASSERT(s.find(cp0) == s.cbegin() + 1);
   ASSERT(s.find('d') == s.cend());
   ASSERT(s.find(text::str::empty + 'a' + cp2) == s.cbegin() + 2);
   ASSERT(s.find(text::str::empty + 'a' + cp2 + cp0 + 'a') == s.cbegin() + 5);
   ASSERT(s.find(text::str::empty + 'a' + cp2 + cp0 + 'd') == s.cend());
   ASSERT(s.find(text::str::empty + 'a' + cp2 + 'a' + 'a' + cp2 + cp0) == s.cbegin() + 2);
   ASSERT(s.find(text::str::empty + 'a' + cp2 + 'a' + 'a' + cp2 + cp0 + 'd') == s.cend());
   ASSERT(s.find_last('a') == s.cend() - 1);
#if 0
   ASSERT(s.find_last(cp2) == s.cend() - 3);
   ASSERT(s.find_last(LOFTY_SL("ab")) == s.cend() - 4);
   ASSERT(s.find_last(LOFTY_SL("ac")) == s.cend() - 9);
   ASSERT(s.find_last(LOFTY_SL("ca")) == s.cend() - 2);
#endif
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_starts_with,
   "lofty::text::str – initial matching"
) {
   LOFTY_TRACE_FUNC();

   // Special characters.
   char32_t cp0 = plane0_cp;
   char32_t cp2 = plane2_cp;
   // See get_acabaabca() for more information on its pattern.
   text::str const s(get_acabaabca());

   ASSERT(s.starts_with(text::str::empty));
   ASSERT(s.starts_with(text::str::empty + 'a'));
   ASSERT(s.starts_with(text::str::empty + 'a' + cp0));
   ASSERT(!s.starts_with(text::str::empty + 'a' + cp2));
   ASSERT(!s.starts_with(text::str::empty + cp0));
   ASSERT(!s.starts_with(text::str::empty + cp2));
   ASSERT(s.starts_with(s));
   ASSERT(!s.starts_with(s + '-'));
   ASSERT(!s.starts_with('-' + s));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_ends_with,
   "lofty::text::str – final matching"
) {
   LOFTY_TRACE_FUNC();

   // Special characters.
   char32_t cp0 = plane0_cp;
   char32_t cp2 = plane2_cp;
   // See get_acabaabca() for more information on its pattern.
   text::str const s(get_acabaabca());

   ASSERT(s.ends_with(text::str::empty));
   ASSERT(s.ends_with(text::str::empty + 'a'));
   ASSERT(s.ends_with(text::str::empty + cp0 + 'a'));
   ASSERT(!s.ends_with(text::str::empty + cp2 + 'a'));
   ASSERT(!s.ends_with(text::str::empty + cp0));
   ASSERT(!s.ends_with(text::str::empty + cp2));
   ASSERT(s.ends_with(s));
   ASSERT(!s.ends_with(s + '-'));
   ASSERT(!s.ends_with('-' + s));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_from_str,
   "lofty::text::str – from_str()"
) {
   LOFTY_TRACE_FUNC();

   ASSERT(from_str<text::str>(LOFTY_SL("")) == LOFTY_SL(""));
   ASSERT(from_str<text::str>(LOFTY_SL("abc")) == LOFTY_SL("abc"));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_to_str,
   "lofty::text::str – to_str()"
) {
   LOFTY_TRACE_FUNC();

   ASSERT(to_str<text::str>(LOFTY_SL("")) == LOFTY_SL(""));
   ASSERT(to_str<text::str>(LOFTY_SL("abc")) == LOFTY_SL("abc"));
}

}} //namespace lofty::test
