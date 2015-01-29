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
// abc::test globals

namespace abc {
namespace test {

char32_t const gc_chP0(ABC_CHAR('\x20ac'));
char32_t const gc_chP2(0x024b62);
/* The string “acabaabca” has the following properties:
•  misleading start for “ab” at index 0 (it’s “ac” instead) and for “abc” at index 2 (it’s
   “aba” instead), to catch incorrect skip-last comparisons;
•  first and last characters match 'a', but other inner ones do too;
•  would match “abcd” were it not for the last character;
•  matches the self-repeating “abaabc” but not the (also self-repeating) “abaabcd”.
The only thing though is that we replace ‘b’ with the Unicode Plane 2 character defined
above and ‘c’ with the BMP (Plane 0) character above. */
istr const gc_sAcabaabca(
   istr::empty + 'a' + gc_chP0 + 'a' + gc_chP2 + 'a' + 'a' + gc_chP2 + gc_chP0 + 'a'
);

} //namespace test
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::text::*str classes – basic operations") {
   ABC_TRACE_FUNC(this);

   dmstr s;
   auto cdpt(testing::utility::make_container_data_ptr_tracker(s));

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

   s = s.substr(1, 3);
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

   s = s.substr(0, -1);
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

   s = s.substr(-3, -2);
   // true: s got replaced by operator=.
   ABC_TESTING_ASSERT_TRUE(cdpt.changed());
   ABC_TESTING_ASSERT_EQUAL(s.size(), 1u);
   ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 1u);
   ABC_TESTING_ASSERT_EQUAL(s[0], ABC_CHAR('ä'));

   s = dmstr(ABC_SL("ab")) + 'c';
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

      dmstr s1, s2(ABC_SL("a"));
      char_t const * pchCheck = s2.cbegin().base();
      // Verify that the compiler selects operator+(dmstr &&, …) when possible.
      s1 = std::move(s2) + ABC_SL("b");
      ABC_TESTING_ASSERT_EQUAL(s1.cbegin().base(), pchCheck);

      istr s3(std::move(s1));
      // Verify that the compiler selects operator+(istr &&, …) when possible.
      s1 = std::move(s3) + ABC_SL("c");
      ABC_TESTING_ASSERT_EQUAL(s1.cbegin().base(), pchCheck);
   }

   // While we’re at it, let’s also validate gc_sAcabaabca.
   ABC_TESTING_ASSERT_EQUAL(gc_sAcabaabca[0], 'a');
   ABC_TESTING_ASSERT_EQUAL(gc_sAcabaabca[1], gc_chP0);
   ABC_TESTING_ASSERT_EQUAL(gc_sAcabaabca[2], 'a');
   ABC_TESTING_ASSERT_EQUAL(gc_sAcabaabca[3], gc_chP2);
   ABC_TESTING_ASSERT_EQUAL(gc_sAcabaabca[4], 'a');
   ABC_TESTING_ASSERT_EQUAL(gc_sAcabaabca[5], 'a');
   ABC_TESTING_ASSERT_EQUAL(gc_sAcabaabca[6], gc_chP2);
   ABC_TESTING_ASSERT_EQUAL(gc_sAcabaabca[7], gc_chP0);
   ABC_TESTING_ASSERT_EQUAL(gc_sAcabaabca[8], 'a');
}

} //namespace test
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::text::*str classes – iterator-based character access") {
   ABC_TRACE_FUNC(this);

   dmstr s;

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

} //namespace test
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::text::*str classes – conversion to different encodings") {
   ABC_TRACE_FUNC(this);

   smstr<32> s;
   s += char32_t(0x000024);
   s += char32_t(0x0000a2);
   s += char32_t(0x0020ac);
   s += char32_t(0x024b62);
   collections::dmvector<std::uint8_t> vb;

   vb = s.encode(text::encoding::utf8, false);
   {
      collections::smvector<std::uint8_t, 16> vbUtf8;
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
      collections::smvector<std::uint8_t, 16> vbUtf16;
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
      collections::smvector<std::uint8_t, 16> vbUtf32;
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

} //namespace test
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::text::*str classes – character replacement") {
   ABC_TRACE_FUNC(this);

   smstr<8> s;

   // No replacements to be made.
   ABC_TESTING_ASSERT_EQUAL(((s = ABC_SL("aaa")).replace('b', 'c'), s), ABC_SL("aaa"));
   // Simple ASCII-to-ASCII replacement: no size change.
   ABC_TESTING_ASSERT_EQUAL(((s = ABC_SL("aaa")).replace('a', 'b'), s), ABC_SL("bbb"));
   /* Complex ASCII-to-char32_t replacement: size will increase beyond the embedded capacity, so the
   iterator used in abc::text::mstr::replace() must be intelligent enough to self-refresh with the
   new descriptor. */
   ABC_TESTING_ASSERT_EQUAL(
      ((s = ABC_SL("aaaaa")).replace(char32_t('a'), gc_chP2), s),
      istr::empty + gc_chP2 + gc_chP2 + gc_chP2 + gc_chP2 + gc_chP2
   );
   // Less-complex char32_t-to-ASCII replacement: size will decrease.
   ABC_TESTING_ASSERT_EQUAL(
      ((s = istr::empty + gc_chP2 + gc_chP2 + gc_chP2 + gc_chP2 + gc_chP2).
         replace(gc_chP2, char32_t('a')), s),
      ABC_SL("aaaaa")
   );
}

} //namespace test
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::text::*str classes – range() permutations") {
   ABC_TRACE_FUNC(this);

   istr sAB(ABC_SL("äb"));

   // Substring of empty string.
   ABC_TESTING_ASSERT_EQUAL(istr::empty.substr(-1, -1), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(istr::empty.substr(-1, 0), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(istr::empty.substr(-1, 1), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(istr::empty.substr(0, -1), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(istr::empty.substr(0, 0), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(istr::empty.substr(0, 1), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(istr::empty.substr(1, -1), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(istr::empty.substr(1, 0), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(istr::empty.substr(1, 1), istr::empty);

   // Substring of a 2-characer string.
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(-3, -3), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(-3, -2), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(-3, -1), ABC_SL("ä"));
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(-3, 0), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(-3, 1), ABC_SL("ä"));
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(-3, 2), ABC_SL("äb"));
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(-2, -3), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(-2, -2), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(-2, -1), ABC_SL("ä"));
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(-2, 0), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(-2, 1), ABC_SL("ä"));
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(-2, 2), ABC_SL("äb"));
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(-1, -3), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(-1, -2), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(-1, -1), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(-1, 0), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(-1, 1), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(-1, 2), ABC_SL("b"));
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(0, -3), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(0, -2), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(0, -1), ABC_SL("ä"));
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(0, 0), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(0, 1), ABC_SL("ä"));
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(0, 2), ABC_SL("äb"));
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(1, -3), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(1, -2), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(1, -1), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(1, 0), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(1, 1), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(1, 2), ABC_SL("b"));
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(2, -3), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(2, -2), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(2, -1), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(2, 0), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(2, 1), istr::empty);
   ABC_TESTING_ASSERT_EQUAL(sAB.substr(2, 2), istr::empty);
}

} //namespace test
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::text::istr – C string extraction") {
   ABC_TRACE_FUNC(this);

   istr s;
   // Note: storing its return value in a variable is NOT a way to use c_str().
   auto psz(s.c_str());
   // s has no character array, so it should have returned the static NUL character.
   ABC_TESTING_ASSERT_NOT_EQUAL(static_cast<char_t const *>(psz), s.cbegin().base());
   ABC_TESTING_ASSERT_FALSE(psz._get().get_deleter().enabled());
   ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz), 0u);
   ABC_TESTING_ASSERT_EQUAL(psz[0], '\0');

   s = ABC_SL("");
   psz = s.c_str();
   /* s should have adopted the literal and therefore have a trailing NUL, so it should have
   returned its own character array. */
   ABC_TESTING_ASSERT_EQUAL(static_cast<char_t const *>(psz), s.cbegin().base());
   ABC_TESTING_ASSERT_FALSE(psz._get().get_deleter().enabled());
   ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz), 0u);
   ABC_TESTING_ASSERT_EQUAL(psz[0], '\0');

   s = ABC_SL("a");
   psz = s.c_str();
   /* s should have adopted the literal and therefore have a trailing NUL, so it should have
   returned its own character array. */
   ABC_TESTING_ASSERT_EQUAL(static_cast<char_t const *>(psz), s.cbegin().base());
   ABC_TESTING_ASSERT_FALSE(psz._get().get_deleter().enabled());
   ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz), 1u);
   ABC_TESTING_ASSERT_EQUAL(psz[0], 'a');
   ABC_TESTING_ASSERT_EQUAL(psz[1], '\0');
}

} //namespace test
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::text::mstr – C string extraction") {
   ABC_TRACE_FUNC(this);

   dmstr s;
   // Note: storing its return value in a variable is NOT a way to use c_str().
   auto psz(s.c_str());
   // s has no character array, so it should have returned the static NUL character.
   ABC_TESTING_ASSERT_NOT_EQUAL(static_cast<char_t const *>(psz), s.cbegin().base());
   ABC_TESTING_ASSERT_FALSE(psz._get().get_deleter().enabled());
   ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz), 0u);
   ABC_TESTING_ASSERT_EQUAL(psz[0], '\0');

   s = ABC_SL("");
   psz = s.c_str();
   // s still has no character array, so it should have returned the static NUL character again.
   ABC_TESTING_ASSERT_NOT_EQUAL(static_cast<char_t const *>(psz), s.cbegin().base());
   ABC_TESTING_ASSERT_FALSE(psz._get().get_deleter().enabled());
   ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz), 0u);
   ABC_TESTING_ASSERT_EQUAL(psz[0], '\0');

   s = ABC_SL("a");
   psz = s.c_str();
   /* s should have copied the literal but dropped its trailing NUL, so it must’ve returned a
   distinct character array. */
   ABC_TESTING_ASSERT_NOT_EQUAL(static_cast<char_t const *>(psz), s.cbegin().base());
   ABC_TESTING_ASSERT_TRUE(psz._get().get_deleter().enabled());
   ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz), 1u);
   ABC_TESTING_ASSERT_EQUAL(psz[0], 'a');
   ABC_TESTING_ASSERT_EQUAL(psz[1], '\0');

   s += ABC_SL("b");
   psz = s.c_str();
   // The character array should have grown, but still lack the trailing NUL.
   ABC_TESTING_ASSERT_NOT_EQUAL(static_cast<char_t const *>(psz), s.cbegin().base());
   ABC_TESTING_ASSERT_TRUE(psz._get().get_deleter().enabled());
   ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz), 2u);
   ABC_TESTING_ASSERT_EQUAL(psz[0], 'a');
   ABC_TESTING_ASSERT_EQUAL(psz[1], 'b');
   ABC_TESTING_ASSERT_EQUAL(psz[2], '\0');
}

} //namespace test
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::text::*str classes – character and substring search") {
   ABC_TRACE_FUNC(this);

   // Special characters.
   char32_t ch0 = gc_chP0;
   char32_t ch2 = gc_chP2;
   /* See gc_sAcabaabca for more information on its pattern. To make it more interesting, here we
   also duplicate it. */
   istr const s(gc_sAcabaabca + gc_sAcabaabca);

   ABC_TESTING_ASSERT_EQUAL(s.find(ch0), s.cbegin() + 1);
   ABC_TESTING_ASSERT_EQUAL(s.find('d'), s.cend());
   ABC_TESTING_ASSERT_EQUAL(s.find(istr::empty + 'a' + ch2), s.cbegin() + 2);
   ABC_TESTING_ASSERT_EQUAL(s.find(istr::empty + 'a' + ch2 + ch0 + 'a'), s.cbegin() + 5);
   ABC_TESTING_ASSERT_EQUAL(s.find(istr::empty + 'a' + ch2 + ch0 + 'd'), s.cend());
   ABC_TESTING_ASSERT_EQUAL(
      s.find(istr::empty + 'a' + ch2 + 'a' + 'a' + ch2 + ch0), s.cbegin() + 2
   );
   ABC_TESTING_ASSERT_EQUAL(
      s.find(istr::empty + 'a' + ch2 + 'a' + 'a' + ch2 + ch0 + 'd'), s.cend()
   );
   ABC_TESTING_ASSERT_EQUAL(s.find_last('a'), s.cend() - 1);
#if 0
   ABC_TESTING_ASSERT_EQUAL(s.find_last(ch2), s.cend() - 3);
   ABC_TESTING_ASSERT_EQUAL(s.find_last(ABC_SL("ab")), s.cend() - 4);
   ABC_TESTING_ASSERT_EQUAL(s.find_last(ABC_SL("ac")), s.cend() - 9);
   ABC_TESTING_ASSERT_EQUAL(s.find_last(ABC_SL("ca")), s.cend() - 2);
#endif
}

} //namespace test
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::text::*str classes – initial matching") {
   ABC_TRACE_FUNC(this);

   // Special characters.
   char32_t ch0 = gc_chP0;
   char32_t ch2 = gc_chP2;
   // See gc_sAcabaabca for more information on its pattern.
   istr const & s = gc_sAcabaabca;

   ABC_TESTING_ASSERT_TRUE(s.starts_with(istr::empty));
   ABC_TESTING_ASSERT_TRUE(s.starts_with(istr::empty + 'a'));
   ABC_TESTING_ASSERT_TRUE(s.starts_with(istr::empty + 'a' + ch0));
   ABC_TESTING_ASSERT_FALSE(s.starts_with(istr::empty + 'a' + ch2));
   ABC_TESTING_ASSERT_FALSE(s.starts_with(istr::empty + ch0));
   ABC_TESTING_ASSERT_FALSE(s.starts_with(istr::empty + ch2));
   ABC_TESTING_ASSERT_TRUE(s.starts_with(s));
   ABC_TESTING_ASSERT_FALSE(s.starts_with(s + '-'));
   ABC_TESTING_ASSERT_FALSE(s.starts_with('-' + s));
}

} //namespace test
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::text::*str classes – final matching") {
   ABC_TRACE_FUNC(this);

   // Special characters.
   char32_t ch0 = gc_chP0;
   char32_t ch2 = gc_chP2;
   // See gc_sAcabaabca for more information on its pattern.
   istr const & s = gc_sAcabaabca;

   ABC_TESTING_ASSERT_TRUE(s.ends_with(istr::empty));
   ABC_TESTING_ASSERT_TRUE(s.ends_with(istr::empty + 'a'));
   ABC_TESTING_ASSERT_TRUE(s.ends_with(istr::empty + ch0 + 'a'));
   ABC_TESTING_ASSERT_FALSE(s.ends_with(istr::empty + ch2 + 'a'));
   ABC_TESTING_ASSERT_FALSE(s.ends_with(istr::empty + ch0));
   ABC_TESTING_ASSERT_FALSE(s.ends_with(istr::empty + ch2));
   ABC_TESTING_ASSERT_TRUE(s.ends_with(s));
   ABC_TESTING_ASSERT_FALSE(s.ends_with(s + '-'));
   ABC_TESTING_ASSERT_FALSE(s.ends_with('-' + s));
}

} //namespace test
} //namespace abc
