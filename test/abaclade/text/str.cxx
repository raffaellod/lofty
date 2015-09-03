/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013, 2014, 2015
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

#include <abaclade.hxx>
#include <abaclade/testing/test_case.hxx>
#include <abaclade/testing/utility.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

//! Unicode Plane 0 code point.
char32_t const gc_cpP0(ABC_CHAR('\x20ac'));
//! Unicode Plane 2 code point.
char32_t const gc_cpP2(0x024b62);

/*! Returns the special string “acabaabca”, which has the following properties:
•  misleading start for “ab” at index 0 (it’s “ac” instead) and for “abc” at index 2 (it’s
   “aba” instead), to catch incorrect skip-last comparisons;
•  first and last characters match 'a', but other inner ones do too;
•  would match “abcd” were it not for the last character;
•  matches the self-repeating “abaabc” but not the (also self-repeating) “abaabcd”.
The only thing though is that we replace ‘b’ with the Unicode Plane 2 character defined
above and ‘c’ with the BMP (Plane 0) character above.

@return
   String described above.
*/
static str get_acabaabca() {
   return str::empty + 'a' + gc_cpP0 + 'a' + gc_cpP2 + 'a' + 'a' + gc_cpP2 + gc_cpP0 + 'a';
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   text_str_basic,
   "abc::text::str – basic operations"
) {
   ABC_TRACE_FUNC(this);

   str s;
   auto cdpt(testing::utility::make_container_data_ptr_tracker(&s));

   s += ABC_SL("ä");
   // true: operator+= must have created an item array (there was none).
   ABC_TESTING_ASSERT_TRUE(cdpt.changed());
   ABC_TESTING_ASSERT_THROWS(index_error, s[-1]);
   ABC_TESTING_ASSERT_DOES_NOT_THROW(s[0]);
   ABC_TESTING_ASSERT_THROWS(index_error, s[1]);
   ABC_TESTING_ASSERT_THROWS(iterator_error, --s.cbegin());
   ABC_TESTING_ASSERT_DOES_NOT_THROW(++s.cbegin());
   ABC_TESTING_ASSERT_DOES_NOT_THROW(--s.cend());
   ABC_TESTING_ASSERT_THROWS(iterator_error, ++s.cend());
   ABC_TESTING_ASSERT_EQUAL(s.size(), 1u);
   ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 1u);
   ABC_TESTING_ASSERT_EQUAL(s[0], ABC_CHAR('ä'));

   s = s + 'b' + s;
   // true: a new string is created by operator+, which replaces s by operator=.
   ABC_TESTING_ASSERT_TRUE(cdpt.changed());
   ABC_TESTING_ASSERT_EQUAL(s.size(), 3u);
   ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 3u);
   ABC_TESTING_ASSERT_EQUAL(s, ABC_SL("äbä"));

   s = s.substr(s.cbegin() + 1, s.cbegin() + 3);
   // true: s got replaced by operator=.
   ABC_TESTING_ASSERT_TRUE(cdpt.changed());
   ABC_TESTING_ASSERT_EQUAL(s.size(), 2u);
   ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 2u);
   ABC_TESTING_ASSERT_EQUAL(s, ABC_SL("bä"));

   s += 'c';
   // false: there should’ve been enough space for 'c'.
   ABC_TESTING_ASSERT_FALSE(cdpt.changed());
   ABC_TESTING_ASSERT_EQUAL(s.size(), 3u);
   ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 3u);
   ABC_TESTING_ASSERT_EQUAL(s, ABC_SL("bäc"));

   s = s.substr(s.cbegin(), s.cend() - 1);
   // true: s got replaced by operator=.
   ABC_TESTING_ASSERT_TRUE(cdpt.changed());
   ABC_TESTING_ASSERT_EQUAL(s.size(), 2u);
   ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 2u);
   ABC_TESTING_ASSERT_EQUAL(s[0], 'b');
   ABC_TESTING_ASSERT_EQUAL(s[1], ABC_CHAR('ä'));

   s += s;
   // false: there should’ve been enough space for “baba”.
   ABC_TESTING_ASSERT_FALSE(cdpt.changed());
   ABC_TESTING_ASSERT_EQUAL(s.size(), 4u);
   ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 4u);
   ABC_TESTING_ASSERT_EQUAL(s[0], 'b');
   ABC_TESTING_ASSERT_EQUAL(s[1], ABC_CHAR('ä'));
   ABC_TESTING_ASSERT_EQUAL(s[2], 'b');
   ABC_TESTING_ASSERT_EQUAL(s[3], ABC_CHAR('ä'));

   s = s.substr(s.cend() - 3, s.cend() - 2);
   // true: s got replaced by operator=.
   ABC_TESTING_ASSERT_TRUE(cdpt.changed());
   ABC_TESTING_ASSERT_EQUAL(s.size(), 1u);
   ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 1u);
   ABC_TESTING_ASSERT_EQUAL(s[0], ABC_CHAR('ä'));

   s = str(ABC_SL("ab")) + 'c';
   // true: s got replaced by operator=.
   ABC_TESTING_ASSERT_TRUE(cdpt.changed());
   ABC_TESTING_ASSERT_EQUAL(s.size(), 3u);
   ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 3u);
   ABC_TESTING_ASSERT_EQUAL(s[0], 'a');
   ABC_TESTING_ASSERT_EQUAL(s[1], 'b');
   ABC_TESTING_ASSERT_EQUAL(s[2], 'c');

   s += 'd';
   // false: there should’ve been enough space for “abcd”.
   ABC_TESTING_ASSERT_FALSE(cdpt.changed());
   ABC_TESTING_ASSERT_EQUAL(s.size(), 4u);
   ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 4u);
   ABC_TESTING_ASSERT_EQUAL(s[0], 'a');
   ABC_TESTING_ASSERT_EQUAL(s[1], 'b');
   ABC_TESTING_ASSERT_EQUAL(s[2], 'c');
   ABC_TESTING_ASSERT_EQUAL(s[3], 'd');

   s += ABC_SL("efghijklmnopqrstuvwxyz");
   /* Cannot assert (ABC_TESTING_ASSERT_*) on this to behave in any specific way, since the
   character array may or may not change depending on heap reallocation strategy. */
   cdpt.changed();
   ABC_TESTING_ASSERT_EQUAL(s.size(), 26u);
   ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 26u);
   ABC_TESTING_ASSERT_EQUAL(s, ABC_SL("abcdefghijklmnopqrstuvwxyz"));

   s = ABC_SL("a\0b");
   s += ABC_SL("\0ç");
   // false: there should have been plenty of storage allocated.
   ABC_TESTING_ASSERT_FALSE(cdpt.changed());
   ABC_TESTING_ASSERT_EQUAL(s.size(), 5u);
   ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 5u);
   // Test both ways to make sure that the char_t[] overload is always chosen over char *.
   ABC_TESTING_ASSERT_EQUAL(s, ABC_SL("a\0b\0ç"));
   ABC_TESTING_ASSERT_EQUAL(ABC_SL("a\0b\0ç"), s);

   /* Now that the string is not empty, validate that clear() truncates it without freeing its
   buffer. */
   s.clear();
   ABC_TESTING_ASSERT_EQUAL(s.size(), 0u);
   ABC_TESTING_ASSERT_GREATER(s.capacity(), 0u);

   {
      /* Note: all string operations here must involve as few characters as possible to avoid
      triggering a reallocation, which would break these tests. */

      str s1(ABC_SL("a"));
      // Write to the string to force it to stop using the string literal “a”.
      s1[0] = 'b';
      char_t const * pchCheck = s1.data();
      // Verify that the compiler selects operator+(str &&, …) when possible.
      str s2 = _std::move(s1) + ABC_SL("c");
      ABC_TESTING_ASSERT_EQUAL(s2.data(), pchCheck);
   }

   // While we’re at it, let’s also validate acabaabca.
   s = get_acabaabca();
   ABC_TESTING_ASSERT_EQUAL(s[0], 'a');
   ABC_TESTING_ASSERT_EQUAL(s[1], gc_cpP0);
   ABC_TESTING_ASSERT_EQUAL(s[2], 'a');
   ABC_TESTING_ASSERT_EQUAL(s[3], gc_cpP2);
   ABC_TESTING_ASSERT_EQUAL(s[4], 'a');
   ABC_TESTING_ASSERT_EQUAL(s[5], 'a');
   ABC_TESTING_ASSERT_EQUAL(s[6], gc_cpP2);
   ABC_TESTING_ASSERT_EQUAL(s[7], gc_cpP0);
   ABC_TESTING_ASSERT_EQUAL(s[8], 'a');
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   text_str_iterators,
   "abc::text::str – iterator-based character access"
) {
   ABC_TRACE_FUNC(this);

   str s;

   // No accessible characters.
   ABC_TESTING_ASSERT_THROWS(index_error, s[-1]);
   ABC_TESTING_ASSERT_THROWS(index_error, s[0]);

   // Should not allow to move an iterator to outside [begin, end].
   ABC_TESTING_ASSERT_DOES_NOT_THROW(s.cbegin());
   ABC_TESTING_ASSERT_DOES_NOT_THROW(s.cend());
   ABC_TESTING_ASSERT_THROWS(iterator_error, --s.cbegin());
   ABC_TESTING_ASSERT_THROWS(iterator_error, ++s.cbegin());
   ABC_TESTING_ASSERT_THROWS(iterator_error, --s.cend());
   ABC_TESTING_ASSERT_THROWS(iterator_error, ++s.cend());

   // Should not allow to dereference end().
   ABC_TESTING_ASSERT_THROWS(iterator_error, *s.cend());
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   text_str_transcoding,
   "abc::text::str – conversion to different encodings"
) {
   ABC_TRACE_FUNC(this);

   sstr<32> s;
   s += char32_t(0x000024);
   s += char32_t(0x0000a2);
   s += char32_t(0x0020ac);
   s += char32_t(0x024b62);
   collections::vector<std::uint8_t> vb;

   vb = s.encode(text::encoding::utf8, false);
   {
      collections::vector<std::uint8_t, 16> vbUtf8;
      vbUtf8.push_back(0x24);
      vbUtf8.push_back(0xc2);
      vbUtf8.push_back(0xa2);
      vbUtf8.push_back(0xe2);
      vbUtf8.push_back(0x82);
      vbUtf8.push_back(0xac);
      vbUtf8.push_back(0xf0);
      vbUtf8.push_back(0xa4);
      vbUtf8.push_back(0xad);
      vbUtf8.push_back(0xa2);
      ABC_TESTING_ASSERT_EQUAL(vb, vbUtf8);
   }

   vb = s.encode(text::encoding::utf16be, false);
   {
      collections::vector<std::uint8_t, 16> vbUtf16;
      vbUtf16.push_back(0x00);
      vbUtf16.push_back(0x24);
      vbUtf16.push_back(0x00);
      vbUtf16.push_back(0xa2);
      vbUtf16.push_back(0x20);
      vbUtf16.push_back(0xac);
      vbUtf16.push_back(0xd8);
      vbUtf16.push_back(0x52);
      vbUtf16.push_back(0xdf);
      vbUtf16.push_back(0x62);
      ABC_TESTING_ASSERT_EQUAL(vb, vbUtf16);
   }

   vb = s.encode(text::encoding::utf32le, false);
   {
      collections::vector<std::uint8_t, 16> vbUtf32;
      vbUtf32.push_back(0x24);
      vbUtf32.push_back(0x00);
      vbUtf32.push_back(0x00);
      vbUtf32.push_back(0x00);
      vbUtf32.push_back(0xa2);
      vbUtf32.push_back(0x00);
      vbUtf32.push_back(0x00);
      vbUtf32.push_back(0x00);
      vbUtf32.push_back(0xac);
      vbUtf32.push_back(0x20);
      vbUtf32.push_back(0x00);
      vbUtf32.push_back(0x00);
      vbUtf32.push_back(0x62);
      vbUtf32.push_back(0x4b);
      vbUtf32.push_back(0x02);
      vbUtf32.push_back(0x00);
      ABC_TESTING_ASSERT_EQUAL(vb, vbUtf32);
   }
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   text_str_char_replacement,
   "abc::text::str – character replacement"
) {
   ABC_TRACE_FUNC(this);

   sstr<8> s;

   // No replacements to be made.
   ABC_TESTING_ASSERT_EQUAL(((s = ABC_SL("aaa")).replace('b', 'c'), s), ABC_SL("aaa"));
   // Simple ASCII-to-ASCII replacement: no size change.
   ABC_TESTING_ASSERT_EQUAL(((s = ABC_SL("aaa")).replace('a', 'b'), s), ABC_SL("bbb"));
   /* Complex ASCII-to-char32_t replacement: size will increase beyond the embedded capacity, so the
   iterator used in abc::text::str::replace() must be intelligent enough to self-refresh with the
   new descriptor. */
   ABC_TESTING_ASSERT_EQUAL(
      ((s = ABC_SL("aaaaa")).replace(char32_t('a'), gc_cpP2), s),
      str::empty + gc_cpP2 + gc_cpP2 + gc_cpP2 + gc_cpP2 + gc_cpP2
   );
   // Less-complex char32_t-to-ASCII replacement: size will decrease.
   ABC_TESTING_ASSERT_EQUAL(
      ((s = str::empty + gc_cpP2 + gc_cpP2 + gc_cpP2 + gc_cpP2 + gc_cpP2).
         replace(gc_cpP2, char32_t('a')), s),
      ABC_SL("aaaaa")
   );
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   text_str_c_str,
   "abc::text::str – C string extraction"
) {
   ABC_TRACE_FUNC(this);

   str s;
   // Note: storing its return value in a variable is NOT a way to use c_str().
   auto psz(const_cast<str const &>(s).c_str());
   // s has no character array, so it should have returned the static NUL character.
   ABC_TESTING_ASSERT_EQUAL(static_cast<char_t const *>(psz), str::empty.data());
   ABC_TESTING_ASSERT_FALSE(psz._get().get_deleter().enabled());
   ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz), 0u);
   ABC_TESTING_ASSERT_EQUAL(psz[0], '\0');

   s = ABC_SL("");
   psz = const_cast<str const &>(s).c_str();
   /* s should have adopted the literal and therefore have a trailing NUL, so it should have
   returned its own character array. */
   ABC_TESTING_ASSERT_EQUAL(static_cast<char_t const *>(psz), s.data());
   ABC_TESTING_ASSERT_FALSE(psz._get().get_deleter().enabled());
   ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz), 0u);
   ABC_TESTING_ASSERT_EQUAL(psz[0], '\0');

   s = ABC_SL("a");
   psz = const_cast<str const &>(s).c_str();
   /* s should have adopted the literal and therefore have a trailing NUL, so it should have
   returned its own character array. */
   ABC_TESTING_ASSERT_EQUAL(static_cast<char_t const *>(psz), s.data());
   ABC_TESTING_ASSERT_FALSE(psz._get().get_deleter().enabled());
   ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz), 1u);
   ABC_TESTING_ASSERT_EQUAL(psz[0], 'a');
   ABC_TESTING_ASSERT_EQUAL(psz[1], '\0');

   s = text::str::empty;
   psz = s.c_str();
   // s has no character array, so it should have returned the static NUL character.
   ABC_TESTING_ASSERT_EQUAL(static_cast<char_t const *>(psz), str::empty.data());
   ABC_TESTING_ASSERT_FALSE(psz._get().get_deleter().enabled());
   ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz), 0u);
   ABC_TESTING_ASSERT_EQUAL(psz[0], '\0');

   s = ABC_SL("");
   psz = s.c_str();
   /* s should have adopted the literal and therefore have a trailing NUL, so it should have
   returned its own character array. */
   ABC_TESTING_ASSERT_EQUAL(static_cast<char_t const *>(psz), s.data());
   ABC_TESTING_ASSERT_FALSE(psz._get().get_deleter().enabled());
   ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz), 0u);
   ABC_TESTING_ASSERT_EQUAL(psz[0], '\0');

   s = ABC_SL("a");
   psz = s.c_str();
   /* s should have copied the literal but dropped its trailing NUL, to then add it back when
   c_str() was called. */
   ABC_TESTING_ASSERT_EQUAL(static_cast<char_t const *>(psz), s.data());
   ABC_TESTING_ASSERT_FALSE(psz._get().get_deleter().enabled());
   ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz), 1u);
   ABC_TESTING_ASSERT_EQUAL(psz[0], 'a');
   ABC_TESTING_ASSERT_EQUAL(psz[1], '\0');

   s += ABC_SL("b");
   psz = s.c_str();
   /* The character array should have grown, to then include a trailing NUL when c_str() was
   called. */
   ABC_TESTING_ASSERT_EQUAL(static_cast<char_t const *>(psz), s.data());
   ABC_TESTING_ASSERT_FALSE(psz._get().get_deleter().enabled());
   ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz), 2u);
   ABC_TESTING_ASSERT_EQUAL(psz[0], 'a');
   ABC_TESTING_ASSERT_EQUAL(psz[1], 'b');
   ABC_TESTING_ASSERT_EQUAL(psz[2], '\0');
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   text_str_find,
   "abc::text::str – character and substring search"
) {
   ABC_TRACE_FUNC(this);

   // Special characters.
   char32_t cp0 = gc_cpP0;
   char32_t cp2 = gc_cpP2;
   /* See get_acabaabca() for more information on its pattern. To make it more interesting, here we
   also duplicate it. */
   str const s(get_acabaabca() + get_acabaabca());

   ABC_TESTING_ASSERT_EQUAL(s.find(cp0), s.cbegin() + 1);
   ABC_TESTING_ASSERT_EQUAL(s.find('d'), s.cend());
   ABC_TESTING_ASSERT_EQUAL(s.find(str::empty + 'a' + cp2), s.cbegin() + 2);
   ABC_TESTING_ASSERT_EQUAL(s.find(str::empty + 'a' + cp2 + cp0 + 'a'), s.cbegin() + 5);
   ABC_TESTING_ASSERT_EQUAL(s.find(str::empty + 'a' + cp2 + cp0 + 'd'), s.cend());
   ABC_TESTING_ASSERT_EQUAL(s.find(str::empty + 'a' + cp2 + 'a' + 'a' + cp2 + cp0), s.cbegin() + 2);
   ABC_TESTING_ASSERT_EQUAL(s.find(str::empty + 'a' + cp2 + 'a' + 'a' + cp2 + cp0 + 'd'), s.cend());
   ABC_TESTING_ASSERT_EQUAL(s.find_last('a'), s.cend() - 1);
#if 0
   ABC_TESTING_ASSERT_EQUAL(s.find_last(cp2), s.cend() - 3);
   ABC_TESTING_ASSERT_EQUAL(s.find_last(ABC_SL("ab")), s.cend() - 4);
   ABC_TESTING_ASSERT_EQUAL(s.find_last(ABC_SL("ac")), s.cend() - 9);
   ABC_TESTING_ASSERT_EQUAL(s.find_last(ABC_SL("ca")), s.cend() - 2);
#endif
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   text_str_starts_with,
   "abc::text::str – initial matching"
) {
   ABC_TRACE_FUNC(this);

   // Special characters.
   char32_t cp0 = gc_cpP0;
   char32_t cp2 = gc_cpP2;
   // See get_acabaabca() for more information on its pattern.
   str const s(get_acabaabca());

   ABC_TESTING_ASSERT_TRUE(s.starts_with(str::empty));
   ABC_TESTING_ASSERT_TRUE(s.starts_with(str::empty + 'a'));
   ABC_TESTING_ASSERT_TRUE(s.starts_with(str::empty + 'a' + cp0));
   ABC_TESTING_ASSERT_FALSE(s.starts_with(str::empty + 'a' + cp2));
   ABC_TESTING_ASSERT_FALSE(s.starts_with(str::empty + cp0));
   ABC_TESTING_ASSERT_FALSE(s.starts_with(str::empty + cp2));
   ABC_TESTING_ASSERT_TRUE(s.starts_with(s));
   ABC_TESTING_ASSERT_FALSE(s.starts_with(s + '-'));
   ABC_TESTING_ASSERT_FALSE(s.starts_with('-' + s));
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   text_str_ends_with,
   "abc::text::str – final matching"
) {
   ABC_TRACE_FUNC(this);

   // Special characters.
   char32_t cp0 = gc_cpP0;
   char32_t cp2 = gc_cpP2;
   // See get_acabaabca() for more information on its pattern.
   str const s(get_acabaabca());

   ABC_TESTING_ASSERT_TRUE(s.ends_with(str::empty));
   ABC_TESTING_ASSERT_TRUE(s.ends_with(str::empty + 'a'));
   ABC_TESTING_ASSERT_TRUE(s.ends_with(str::empty + cp0 + 'a'));
   ABC_TESTING_ASSERT_FALSE(s.ends_with(str::empty + cp2 + 'a'));
   ABC_TESTING_ASSERT_FALSE(s.ends_with(str::empty + cp0));
   ABC_TESTING_ASSERT_FALSE(s.ends_with(str::empty + cp2));
   ABC_TESTING_ASSERT_TRUE(s.ends_with(s));
   ABC_TESTING_ASSERT_FALSE(s.ends_with(s + '-'));
   ABC_TESTING_ASSERT_FALSE(s.ends_with('-' + s));
}

}} //namespace abc::test
