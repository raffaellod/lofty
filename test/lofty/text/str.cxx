/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/collections.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/testing/utility.hxx>


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
static str get_acabaabca() {
   return str::empty + 'a' + plane0_cp + 'a' + plane2_cp + 'a' + 'a' + plane2_cp + plane0_cp + 'a';
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_basic,
   "lofty::text::str – basic operations"
) {
   LOFTY_TRACE_FUNC(this);

   str s;
   auto tracker(testing::utility::make_container_data_ptr_tracker(&s));

   s += LOFTY_SL("ä");
   // true: operator+= must have created an item array (there was none).
   LOFTY_TESTING_ASSERT_TRUE(tracker.changed());
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, s[-1]);
   LOFTY_TESTING_ASSERT_DOES_NOT_THROW(s[0]);
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, s[1]);
   LOFTY_TESTING_ASSERT_EQUAL(s.size(), 1u);
   LOFTY_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(s[0], LOFTY_CHAR('ä'));

   s = s + 'b' + s;
   // true: a new string is created by operator+, which replaces s by operator=.
   LOFTY_TESTING_ASSERT_TRUE(tracker.changed());
   LOFTY_TESTING_ASSERT_EQUAL(s.size(), 3u);
   LOFTY_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 3u);
   LOFTY_TESTING_ASSERT_EQUAL(s, LOFTY_SL("äbä"));

   s = s.substr(s.cbegin() + 1, s.cbegin() + 3);
   // true: s got replaced by operator=.
   LOFTY_TESTING_ASSERT_TRUE(tracker.changed());
   LOFTY_TESTING_ASSERT_EQUAL(s.size(), 2u);
   LOFTY_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 2u);
   LOFTY_TESTING_ASSERT_EQUAL(s, LOFTY_SL("bä"));

   s += 'c';
   // false: there should’ve been enough space for 'c'.
   LOFTY_TESTING_ASSERT_FALSE(tracker.changed());
   LOFTY_TESTING_ASSERT_EQUAL(s.size(), 3u);
   LOFTY_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 3u);
   LOFTY_TESTING_ASSERT_EQUAL(s, LOFTY_SL("bäc"));

   s = s.substr(s.cbegin(), s.cend() - 1);
   // true: s got replaced by operator=.
   LOFTY_TESTING_ASSERT_TRUE(tracker.changed());
   LOFTY_TESTING_ASSERT_EQUAL(s.size(), 2u);
   LOFTY_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 2u);
   LOFTY_TESTING_ASSERT_EQUAL(s[0], 'b');
   LOFTY_TESTING_ASSERT_EQUAL(s[1], LOFTY_CHAR('ä'));

   s += s;
   // false: there should’ve been enough space for “baba”.
   LOFTY_TESTING_ASSERT_FALSE(tracker.changed());
   LOFTY_TESTING_ASSERT_EQUAL(s.size(), 4u);
   LOFTY_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 4u);
   LOFTY_TESTING_ASSERT_EQUAL(s[0], 'b');
   LOFTY_TESTING_ASSERT_EQUAL(s[1], LOFTY_CHAR('ä'));
   LOFTY_TESTING_ASSERT_EQUAL(s[2], 'b');
   LOFTY_TESTING_ASSERT_EQUAL(s[3], LOFTY_CHAR('ä'));

   s = s.substr(s.cend() - 3, s.cend() - 2);
   // true: s got replaced by operator=.
   LOFTY_TESTING_ASSERT_TRUE(tracker.changed());
   LOFTY_TESTING_ASSERT_EQUAL(s.size(), 1u);
   LOFTY_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(s[0], LOFTY_CHAR('ä'));

   s = str(LOFTY_SL("ab")) + 'c';
   // true: s got replaced by operator=.
   LOFTY_TESTING_ASSERT_TRUE(tracker.changed());
   LOFTY_TESTING_ASSERT_EQUAL(s.size(), 3u);
   LOFTY_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 3u);
   LOFTY_TESTING_ASSERT_EQUAL(s[0], 'a');
   LOFTY_TESTING_ASSERT_EQUAL(s[1], 'b');
   LOFTY_TESTING_ASSERT_EQUAL(s[2], 'c');

   s += 'd';
   // false: there should’ve been enough space for “abcd”.
   LOFTY_TESTING_ASSERT_FALSE(tracker.changed());
   LOFTY_TESTING_ASSERT_EQUAL(s.size(), 4u);
   LOFTY_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 4u);
   LOFTY_TESTING_ASSERT_EQUAL(s[0], 'a');
   LOFTY_TESTING_ASSERT_EQUAL(s[1], 'b');
   LOFTY_TESTING_ASSERT_EQUAL(s[2], 'c');
   LOFTY_TESTING_ASSERT_EQUAL(s[3], 'd');

   s += LOFTY_SL("efghijklmnopqrstuvwxyz");
   /* Cannot assert (LOFTY_TESTING_ASSERT_*) on this to behave in any specific way, since the character array
   may or may not change depending on heap reallocation strategy. */
   tracker.changed();
   LOFTY_TESTING_ASSERT_EQUAL(s.size(), 26u);
   LOFTY_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 26u);
   LOFTY_TESTING_ASSERT_EQUAL(s, LOFTY_SL("abcdefghijklmnopqrstuvwxyz"));

   s = LOFTY_SL("a\0b");
   // true: s got replaced by operator=.
   LOFTY_TESTING_ASSERT_TRUE(tracker.changed());
   s += LOFTY_SL("\0ç");
   // true: switched to writable copy.
   LOFTY_TESTING_ASSERT_TRUE(tracker.changed());
   LOFTY_TESTING_ASSERT_EQUAL(s.size(), 5u);
   LOFTY_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 5u);
   // Test both ways to make sure that the char_t[] overload is always chosen over char *.
   LOFTY_TESTING_ASSERT_EQUAL(s, LOFTY_SL("a\0b\0ç"));
   LOFTY_TESTING_ASSERT_EQUAL(LOFTY_SL("a\0b\0ç"), s);

   // Now that the string is not empty, validate that clear() truncates it without freeing its buffer.
   s.clear();
   LOFTY_TESTING_ASSERT_EQUAL(s.size(), 0u);
   LOFTY_TESTING_ASSERT_GREATER(s.capacity(), 0u);

   {
      /* Note: all string operations here must involve as few characters as possible to avoid triggering a
      reallocation, which would break these tests. */

      str s1(LOFTY_SL("a"));
      // Write to the string to force it to stop using the string literal “a”.
      s1[0] = 'b';
      char_t const * old_data = s1.data();
      // Verify that the compiler selects operator+(str &&, …) when possible.
      str s2 = _std::move(s1) + LOFTY_SL("c");
      LOFTY_TESTING_ASSERT_EQUAL(s2.data(), old_data);
   }

   // While we’re at it, let’s also validate acabaabca.
   s = get_acabaabca();
   LOFTY_TESTING_ASSERT_EQUAL(s[0], 'a');
   LOFTY_TESTING_ASSERT_EQUAL(s[1], plane0_cp);
   LOFTY_TESTING_ASSERT_EQUAL(s[2], 'a');
   LOFTY_TESTING_ASSERT_EQUAL(s[3], plane2_cp);
   LOFTY_TESTING_ASSERT_EQUAL(s[4], 'a');
   LOFTY_TESTING_ASSERT_EQUAL(s[5], 'a');
   LOFTY_TESTING_ASSERT_EQUAL(s[6], plane2_cp);
   LOFTY_TESTING_ASSERT_EQUAL(s[7], plane0_cp);
   LOFTY_TESTING_ASSERT_EQUAL(s[8], 'a');
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_iterators,
   "lofty::text::str – operations with iterators"
) {
   LOFTY_TRACE_FUNC(this);

   // Default-constructed iterator.
   str::const_iterator itr;
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, *itr);
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, --itr);
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, ++itr);
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, --itr);
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, ++itr);
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, itr[-1]);
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, itr[0]);
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, itr[1]);

   str s;
   LOFTY_TESTING_ASSERT_EQUAL(s.cbegin(), s.end());

   // No accessible characters.
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, s[-1]);
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, s[0]);
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, s[1]);

   // Should not allow to move an iterator to outside [begin, end].
   LOFTY_TESTING_ASSERT_DOES_NOT_THROW(s.cbegin());
   LOFTY_TESTING_ASSERT_DOES_NOT_THROW(s.cend());
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, --s.cbegin());
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, ++s.cbegin());
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, --s.cend());
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, ++s.cend());
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, s.cbegin()[-1]);
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, s.cbegin()[0]);
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, s.cbegin()[1]);

   // Should not allow to dereference begin() or end() of an empty string.
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, *s.cbegin());
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, *s.cend());

   s += 'a';
   LOFTY_TESTING_ASSERT_NOT_EQUAL(s.begin(), s.cend());

   // One accessible character.
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, s[-1]);
   LOFTY_TESTING_ASSERT_DOES_NOT_THROW(s[0]);
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, s[1]);

   // Should not allow to move an iterator to outside [begin, end].
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, --s.cbegin());
   LOFTY_TESTING_ASSERT_DOES_NOT_THROW(++s.cbegin());
   LOFTY_TESTING_ASSERT_DOES_NOT_THROW(--s.cend());
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, ++s.cend());
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, s.cbegin()[-1]);
   LOFTY_TESTING_ASSERT_DOES_NOT_THROW(s.cbegin()[0]);
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, s.cbegin()[1]);

   // Should allow to dereference begin(), but not end() of a non-empty string.
   LOFTY_TESTING_ASSERT_DOES_NOT_THROW(*s.cbegin());
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, *s.cend());
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_transcoding,
   "lofty::text::str – conversion to different encodings"
) {
   LOFTY_TRACE_FUNC(this);

   sstr<32> s;
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
      LOFTY_TESTING_ASSERT_EQUAL(encoded_bytes, utf8_bytes);
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
      LOFTY_TESTING_ASSERT_EQUAL(encoded_bytes, utf16_bytes);
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
      LOFTY_TESTING_ASSERT_EQUAL(encoded_bytes, utf32_bytes);
   }
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_char_replacement,
   "lofty::text::str – character replacement"
) {
   LOFTY_TRACE_FUNC(this);

   sstr<8> s;

   // No replacements to be made.
   LOFTY_TESTING_ASSERT_EQUAL(((s = LOFTY_SL("aaa")).replace('b', 'c'), s), LOFTY_SL("aaa"));
   // Simple ASCII-to-ASCII replacement: no size change.
   LOFTY_TESTING_ASSERT_EQUAL(((s = LOFTY_SL("aaa")).replace('a', 'b'), s), LOFTY_SL("bbb"));
   /* Complex ASCII-to-char32_t replacement: size will increase beyond the embedded capacity, so the iterator
   used in lofty::text::str::replace() must be intelligent enough to self-refresh with the new descriptor. */
   LOFTY_TESTING_ASSERT_EQUAL(
      ((s = LOFTY_SL("aaaaa")).replace(char32_t('a'), plane2_cp), s),
      str::empty + plane2_cp + plane2_cp + plane2_cp + plane2_cp + plane2_cp
   );
   // Less-complex char32_t-to-ASCII replacement: size will decrease.
   s = str::empty + plane2_cp + plane2_cp + plane2_cp + plane2_cp + plane2_cp;
   LOFTY_TESTING_ASSERT_EQUAL((s.replace(plane2_cp, char32_t('a')), s), LOFTY_SL("aaaaa"));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_c_str,
   "lofty::text::str – C string extraction"
) {
   LOFTY_TRACE_FUNC(this);

   str s;
   // Note: storing its return value in a variable is NOT a way to use c_str().
   auto c_str(const_cast<str const &>(s).c_str());
   // s has no character array, so it should have returned the static NUL character.
   LOFTY_TESTING_ASSERT_EQUAL(static_cast<char_t const *>(c_str), str::empty.data());
   LOFTY_TESTING_ASSERT_FALSE(c_str._get().get_deleter().enabled());
   LOFTY_TESTING_ASSERT_EQUAL(text::size_in_chars(c_str), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(c_str[0], '\0');

   s = LOFTY_SL("");
   c_str = const_cast<str const &>(s).c_str();
   /* s should have adopted the literal and therefore have a trailing NUL, so it should have returned its own
   character array. */
   LOFTY_TESTING_ASSERT_EQUAL(static_cast<char_t const *>(c_str), s.data());
   LOFTY_TESTING_ASSERT_FALSE(c_str._get().get_deleter().enabled());
   LOFTY_TESTING_ASSERT_EQUAL(text::size_in_chars(c_str), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(c_str[0], '\0');

   s = LOFTY_SL("a");
   c_str = const_cast<str const &>(s).c_str();
   /* s should have adopted the literal and therefore have a trailing NUL, so it should have returned its own
   character array. */
   LOFTY_TESTING_ASSERT_EQUAL(static_cast<char_t const *>(c_str), s.data());
   LOFTY_TESTING_ASSERT_FALSE(c_str._get().get_deleter().enabled());
   LOFTY_TESTING_ASSERT_EQUAL(text::size_in_chars(c_str), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(c_str[0], 'a');
   LOFTY_TESTING_ASSERT_EQUAL(c_str[1], '\0');

   s = text::str::empty;
   c_str = s.c_str();
   // s has no character array, so it should have returned the static NUL character.
   LOFTY_TESTING_ASSERT_EQUAL(static_cast<char_t const *>(c_str), str::empty.data());
   LOFTY_TESTING_ASSERT_FALSE(c_str._get().get_deleter().enabled());
   LOFTY_TESTING_ASSERT_EQUAL(text::size_in_chars(c_str), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(c_str[0], '\0');

   s = LOFTY_SL("");
   c_str = s.c_str();
   /* s should have adopted the literal and therefore have a trailing NUL, so it should have returned its own
   character array. */
   LOFTY_TESTING_ASSERT_EQUAL(static_cast<char_t const *>(c_str), s.data());
   LOFTY_TESTING_ASSERT_FALSE(c_str._get().get_deleter().enabled());
   LOFTY_TESTING_ASSERT_EQUAL(text::size_in_chars(c_str), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(c_str[0], '\0');

   s = LOFTY_SL("a");
   c_str = s.c_str();
   /* s should have copied the literal but dropped its trailing NUL, to then add it back when c_str() was
   called. */
   LOFTY_TESTING_ASSERT_EQUAL(static_cast<char_t const *>(c_str), s.data());
   LOFTY_TESTING_ASSERT_FALSE(c_str._get().get_deleter().enabled());
   LOFTY_TESTING_ASSERT_EQUAL(text::size_in_chars(c_str), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(c_str[0], 'a');
   LOFTY_TESTING_ASSERT_EQUAL(c_str[1], '\0');

   s += LOFTY_SL("b");
   c_str = s.c_str();
   // The character array should have grown, to then include a trailing NUL when c_str() was called.
   LOFTY_TESTING_ASSERT_EQUAL(static_cast<char_t const *>(c_str), s.data());
   LOFTY_TESTING_ASSERT_FALSE(c_str._get().get_deleter().enabled());
   LOFTY_TESTING_ASSERT_EQUAL(text::size_in_chars(c_str), 2u);
   LOFTY_TESTING_ASSERT_EQUAL(c_str[0], 'a');
   LOFTY_TESTING_ASSERT_EQUAL(c_str[1], 'b');
   LOFTY_TESTING_ASSERT_EQUAL(c_str[2], '\0');
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_find,
   "lofty::text::str – character and substring search"
) {
   LOFTY_TRACE_FUNC(this);

   // Special characters.
   char32_t cp0 = plane0_cp;
   char32_t cp2 = plane2_cp;
   /* See get_acabaabca() for more information on its pattern. To make it more interesting, here we also
   duplicate it. */
   str const s(get_acabaabca() + get_acabaabca());

   LOFTY_TESTING_ASSERT_EQUAL(s.find(cp0), s.cbegin() + 1);
   LOFTY_TESTING_ASSERT_EQUAL(s.find('d'), s.cend());
   LOFTY_TESTING_ASSERT_EQUAL(s.find(str::empty + 'a' + cp2), s.cbegin() + 2);
   LOFTY_TESTING_ASSERT_EQUAL(s.find(str::empty + 'a' + cp2 + cp0 + 'a'), s.cbegin() + 5);
   LOFTY_TESTING_ASSERT_EQUAL(s.find(str::empty + 'a' + cp2 + cp0 + 'd'), s.cend());
   LOFTY_TESTING_ASSERT_EQUAL(s.find(str::empty + 'a' + cp2 + 'a' + 'a' + cp2 + cp0), s.cbegin() + 2);
   LOFTY_TESTING_ASSERT_EQUAL(s.find(str::empty + 'a' + cp2 + 'a' + 'a' + cp2 + cp0 + 'd'), s.cend());
   LOFTY_TESTING_ASSERT_EQUAL(s.find_last('a'), s.cend() - 1);
#if 0
   LOFTY_TESTING_ASSERT_EQUAL(s.find_last(cp2), s.cend() - 3);
   LOFTY_TESTING_ASSERT_EQUAL(s.find_last(LOFTY_SL("ab")), s.cend() - 4);
   LOFTY_TESTING_ASSERT_EQUAL(s.find_last(LOFTY_SL("ac")), s.cend() - 9);
   LOFTY_TESTING_ASSERT_EQUAL(s.find_last(LOFTY_SL("ca")), s.cend() - 2);
#endif
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_starts_with,
   "lofty::text::str – initial matching"
) {
   LOFTY_TRACE_FUNC(this);

   // Special characters.
   char32_t cp0 = plane0_cp;
   char32_t cp2 = plane2_cp;
   // See get_acabaabca() for more information on its pattern.
   str const s(get_acabaabca());

   LOFTY_TESTING_ASSERT_TRUE(s.starts_with(str::empty));
   LOFTY_TESTING_ASSERT_TRUE(s.starts_with(str::empty + 'a'));
   LOFTY_TESTING_ASSERT_TRUE(s.starts_with(str::empty + 'a' + cp0));
   LOFTY_TESTING_ASSERT_FALSE(s.starts_with(str::empty + 'a' + cp2));
   LOFTY_TESTING_ASSERT_FALSE(s.starts_with(str::empty + cp0));
   LOFTY_TESTING_ASSERT_FALSE(s.starts_with(str::empty + cp2));
   LOFTY_TESTING_ASSERT_TRUE(s.starts_with(s));
   LOFTY_TESTING_ASSERT_FALSE(s.starts_with(s + '-'));
   LOFTY_TESTING_ASSERT_FALSE(s.starts_with('-' + s));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_str_ends_with,
   "lofty::text::str – final matching"
) {
   LOFTY_TRACE_FUNC(this);

   // Special characters.
   char32_t cp0 = plane0_cp;
   char32_t cp2 = plane2_cp;
   // See get_acabaabca() for more information on its pattern.
   str const s(get_acabaabca());

   LOFTY_TESTING_ASSERT_TRUE(s.ends_with(str::empty));
   LOFTY_TESTING_ASSERT_TRUE(s.ends_with(str::empty + 'a'));
   LOFTY_TESTING_ASSERT_TRUE(s.ends_with(str::empty + cp0 + 'a'));
   LOFTY_TESTING_ASSERT_FALSE(s.ends_with(str::empty + cp2 + 'a'));
   LOFTY_TESTING_ASSERT_FALSE(s.ends_with(str::empty + cp0));
   LOFTY_TESTING_ASSERT_FALSE(s.ends_with(str::empty + cp2));
   LOFTY_TESTING_ASSERT_TRUE(s.ends_with(s));
   LOFTY_TESTING_ASSERT_FALSE(s.ends_with(s + '-'));
   LOFTY_TESTING_ASSERT_FALSE(s.ends_with('-' + s));
}

}} //namespace lofty::test
