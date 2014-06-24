/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013, 2014
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



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str_test_case_base

namespace abc {
namespace test {

class str_test_case_base :
   public testing::test_case {
protected:

   /** Initializes the private data members used by str_ptr_changed().

   s
      String to initialize with.
   */
   void init_str_ptr(istr const & s) {
      ABC_TRACE_FUNC(this, s);

      m_psCheck = &s;
      m_pchCheck = s.cbegin().base();
   }


   /** Checks if a string’s item array has been reallocated.

   return
      true if the string’s character array pointer has changed, or false otherwise.
   */
   bool str_ptr_changed() {
      ABC_TRACE_FUNC(this);

      // Update the item array pointer for the next call.
      char_t const * pchCheckOld(m_pchCheck);
      m_pchCheck = m_psCheck->cbegin().base();
      // Check if the item array has changed.
      return pchCheckOld != m_pchCheck;
   }


private:

   /** Pointer to the local string variable to be checked. */
   istr const * m_psCheck;
   /** Pointer to m_psCheck’s item array. */
   char_t const * m_pchCheck;
};

} //namespace test
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str_basic

namespace abc {
namespace test {

class str_basic :
   public str_test_case_base {
public:

   /** See str_test_case_base::title().
   */
   virtual istr title() {
      return istr(SL("abc::*str classes – basic operations"));
   }


   /** See str_test_case_base::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      dmstr s;
      init_str_ptr(s);

      // Only the trailing NUL character should be accessible.
      ABC_TESTING_ASSERT_THROWS(index_error, s[-1]);
      ABC_TESTING_ASSERT_THROWS(index_error, s[0]);

      // Should not allow to move an iterator to outside [begin, end].
      ABC_TESTING_ASSERT_DOES_NOT_THROW(s.cbegin());
      ABC_TESTING_ASSERT_DOES_NOT_THROW(s.cend());
      ABC_TESTING_ASSERT_THROWS(iterator_error, --s.cbegin());
      ABC_TESTING_ASSERT_THROWS(iterator_error, ++s.cbegin());
      ABC_TESTING_ASSERT_THROWS(iterator_error, --s.cend());
      ABC_TESTING_ASSERT_THROWS(iterator_error, ++s.cend());

      s += SL("a");
      // true: operator+= must have created an item array (there was none).
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed());
      ABC_TESTING_ASSERT_THROWS(index_error, s[-1]);
      ABC_TESTING_ASSERT_DOES_NOT_THROW(s[0]);
      ABC_TESTING_ASSERT_THROWS(index_error, s[1]);
      ABC_TESTING_ASSERT_THROWS(iterator_error, --s.cbegin());
      ABC_TESTING_ASSERT_DOES_NOT_THROW(++s.cbegin());
      ABC_TESTING_ASSERT_DOES_NOT_THROW(--s.cend());
      ABC_TESTING_ASSERT_THROWS(iterator_error, ++s.cend());
      ABC_TESTING_ASSERT_EQUAL(s.size_in_codepoints(), 1u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 1u);
      ABC_TESTING_ASSERT_EQUAL(s[0], 'a');

      s = s + 'b' + s;
      // true: a new string is created by operator+, which replaces s by operator=.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed());
      ABC_TESTING_ASSERT_EQUAL(s.size_in_codepoints(), 3u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 3u);
      ABC_TESTING_ASSERT_EQUAL(s, SL("aba"));

      s = s.substr(1, 3);
      // true: s got replaced by operator=.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed());
      ABC_TESTING_ASSERT_EQUAL(s.size_in_codepoints(), 2u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 2u);
      ABC_TESTING_ASSERT_EQUAL(s, SL("ba"));

      s += 'c';
      // false: there should’ve been enough space for 'c'.
      ABC_TESTING_ASSERT_FALSE(str_ptr_changed());
      ABC_TESTING_ASSERT_EQUAL(s.size_in_codepoints(), 3u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 3u);
      ABC_TESTING_ASSERT_EQUAL(s, SL("bac"));

      s = s.substr(0, -1);
      // true: s got replaced by operator=.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed());
      ABC_TESTING_ASSERT_EQUAL(s.size_in_codepoints(), 2u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 2u);
      ABC_TESTING_ASSERT_EQUAL(s[0], 'b');
      ABC_TESTING_ASSERT_EQUAL(s[1], 'a');

      s += s;
      // false: there should’ve been enough space for “baba”.
      ABC_TESTING_ASSERT_FALSE(str_ptr_changed());
      ABC_TESTING_ASSERT_EQUAL(s.size_in_codepoints(), 4u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 4u);
      ABC_TESTING_ASSERT_EQUAL(s[0], 'b');
      ABC_TESTING_ASSERT_EQUAL(s[1], 'a');
      ABC_TESTING_ASSERT_EQUAL(s[2], 'b');
      ABC_TESTING_ASSERT_EQUAL(s[3], 'a');

      s = s.substr(-3, -2);
      // true: s got replaced by operator=.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed());
      ABC_TESTING_ASSERT_EQUAL(s.size_in_codepoints(), 1u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 1u);
      ABC_TESTING_ASSERT_EQUAL(s[0], 'a');

      s = dmstr(SL("ab")) + 'c';
      // true: s got replaced by operator=.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed());
      ABC_TESTING_ASSERT_EQUAL(s.size_in_codepoints(), 3u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 3u);
      ABC_TESTING_ASSERT_EQUAL(s[0], 'a');
      ABC_TESTING_ASSERT_EQUAL(s[1], 'b');
      ABC_TESTING_ASSERT_EQUAL(s[2], 'c');

      s += 'd';
      // false: there should’ve been enough space for “abcd”.
      ABC_TESTING_ASSERT_FALSE(str_ptr_changed());
      ABC_TESTING_ASSERT_EQUAL(s.size_in_codepoints(), 4u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 4u);
      ABC_TESTING_ASSERT_EQUAL(s[0], 'a');
      ABC_TESTING_ASSERT_EQUAL(s[1], 'b');
      ABC_TESTING_ASSERT_EQUAL(s[2], 'c');
      ABC_TESTING_ASSERT_EQUAL(s[3], 'd');

      s += SL("efghijklmnopqrstuvwxyz");
      // Cannot assert (ABC_TESTING_ASSERT_*) on this to behave in any specific way, since the
      // character array may or may not change depending on heap reallocation strategy.
      str_ptr_changed();
      ABC_TESTING_ASSERT_EQUAL(s.size_in_codepoints(), 26u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 26u);
      ABC_TESTING_ASSERT_EQUAL(s, SL("abcdefghijklmnopqrstuvwxyz"));

      s = SL("a\0b");
      s += SL("\0c");
      // false: there should have been plenty of storage allocated.
      ABC_TESTING_ASSERT_FALSE(str_ptr_changed());
      ABC_TESTING_ASSERT_EQUAL(s.size_in_codepoints(), 5u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 5u);
      // Test both ways to make sure that the char_t[] overload is always chosen over char *.
      ABC_TESTING_ASSERT_EQUAL(s, SL("a\0b\0c"));
      ABC_TESTING_ASSERT_EQUAL(SL("a\0b\0c"), s);

      {
         // Note: all string operations here must involve as few characters as possible to avoid
         // triggering a reallocation, which would break these tests.

         dmstr s1, s2(SL("a"));
         char_t const * pchCheck(s2.cbegin().base());
         // Verify that the compiler selects operator+(dmstr &&, …) when possible.
         s1 = std::move(s2) + SL("b");
         ABC_TESTING_ASSERT_EQUAL(s1.cbegin().base(), pchCheck);

         istr s3(std::move(s1));
         // Verify that the compiler selects operator+(istr &&, …) when possible.
         s1 = std::move(s3) + SL("c");
         ABC_TESTING_ASSERT_EQUAL(s1.cbegin().base(), pchCheck);
      }
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str_basic)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str_encode

namespace abc {
namespace test {

class str_encode :
   public testing::test_case {
public:

   /** See abc::testing::test_case::title().
   */
   virtual istr title() {
      return istr(
         SL("abc::*str classes – conversion to different encodings")
      );
   }


   /** See abc::testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      smstr<32> s;
      s += char32_t(0x000024);
      s += char32_t(0x0000a2);
      s += char32_t(0x0020ac);
      s += char32_t(0x024b62);
      dmvector<uint8_t> vb;

      vb = s.encode(text::encoding::utf8, false);
      {
         smvector<uint8_t, 16> vbUtf8;
         vbUtf8.append(0x24);
         vbUtf8.append(0xc2);
         vbUtf8.append(0xa2);
         vbUtf8.append(0xe2);
         vbUtf8.append(0x82);
         vbUtf8.append(0xac);
         vbUtf8.append(0xf0);
         vbUtf8.append(0xa4);
         vbUtf8.append(0xad);
         vbUtf8.append(0xa2);
         ABC_TESTING_ASSERT_EQUAL(vb, vbUtf8);
      }

      vb = s.encode(text::encoding::utf16be, false);
      {
         smvector<uint8_t, 16> vbUtf16;
         vbUtf16.append(0x00);
         vbUtf16.append(0x24);
         vbUtf16.append(0x00);
         vbUtf16.append(0xa2);
         vbUtf16.append(0x20);
         vbUtf16.append(0xac);
         vbUtf16.append(0xd8);
         vbUtf16.append(0x52);
         vbUtf16.append(0xdf);
         vbUtf16.append(0x62);
         ABC_TESTING_ASSERT_EQUAL(vb, vbUtf16);
      }

      vb = s.encode(text::encoding::utf32le, false);
      {
         smvector<uint8_t, 16> vbUtf32;
         vbUtf32.append(0x24);
         vbUtf32.append(0x00);
         vbUtf32.append(0x00);
         vbUtf32.append(0x00);
         vbUtf32.append(0xa2);
         vbUtf32.append(0x00);
         vbUtf32.append(0x00);
         vbUtf32.append(0x00);
         vbUtf32.append(0xac);
         vbUtf32.append(0x20);
         vbUtf32.append(0x00);
         vbUtf32.append(0x00);
         vbUtf32.append(0x62);
         vbUtf32.append(0x4b);
         vbUtf32.append(0x02);
         vbUtf32.append(0x00);
         ABC_TESTING_ASSERT_EQUAL(vb, vbUtf32);
      }
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str_encode)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str_substr


namespace abc {
namespace test {

class str_substr :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::*str classes – substring extraction"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      istr sEmpty, sAB(SL("ab"));

      // Substring of empty string.
      ABC_TESTING_ASSERT_EQUAL(sEmpty.substr(-1, -1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sEmpty.substr(-1,  0), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sEmpty.substr(-1,  1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sEmpty.substr( 0, -1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sEmpty.substr( 0,  0), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sEmpty.substr( 0,  1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sEmpty.substr( 1, -1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sEmpty.substr( 1,  0), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sEmpty.substr( 1,  1), SL(""));

      // Substring of a 2-characer string.
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-3, -3), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-3, -2), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-3, -1), SL("a"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-3,  0), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-3,  1), SL("a"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-3,  2), SL("ab"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-2, -3), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-2, -2), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-2, -1), SL("a"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-2,  0), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-2,  1), SL("a"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-2,  2), SL("ab"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-1, -3), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-1, -2), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-1, -1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-1,  0), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-1,  1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr(-1,  2), SL("b"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr( 0, -3), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr( 0, -2), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr( 0, -1), SL("a"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr( 0,  0), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr( 0,  1), SL("a"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr( 0,  2), SL("ab"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr( 1, -3), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr( 1, -2), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr( 1, -1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr( 1,  0), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr( 1,  1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr( 1,  2), SL("b"));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr( 2, -3), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr( 2, -2), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr( 2, -1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr( 2,  0), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr( 2,  1), SL(""));
      ABC_TESTING_ASSERT_EQUAL(sAB.substr( 2,  2), SL(""));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str_substr)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::istr_c_str

namespace abc {
namespace test {

class istr_c_str :
   public str_test_case_base {
public:

   /** See str_test_case_base::title().
   */
   virtual istr title() {
      return istr(SL("abc::istr – C string extraction"));
   }


   /** See str_test_case_base::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      istr s;
      auto psz(s.c_str());
      // s has no character array, so it should have returned the static NUL character.
      ABC_TESTING_ASSERT_NOT_EQUAL(psz.get(), s.cbegin().base());
      ABC_TESTING_ASSERT_FALSE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz.get()), 0u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], '\0');

      s = SL("");
      psz = s.c_str();
      // s should have adopted the literal and therefore have a trailing NUL, so it should have
      // returned its own character array.
      ABC_TESTING_ASSERT_EQUAL(psz.get(), s.cbegin().base());
      ABC_TESTING_ASSERT_FALSE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz.get()), 0u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], '\0');

      s = SL("a");
      psz = s.c_str();
      // s should have adopted the literal and therefore have a trailing NUL, so it should have
      // returned its own character array.
      ABC_TESTING_ASSERT_EQUAL(psz.get(), s.cbegin().base());
      ABC_TESTING_ASSERT_FALSE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz.get()), 1u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], 'a');
      ABC_TESTING_ASSERT_EQUAL(psz[1], '\0');
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::istr_c_str)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::mstr_c_str

namespace abc {
namespace test {

class mstr_c_str :
   public str_test_case_base {
public:

   /** See str_test_case_base::title().
   */
   virtual istr title() {
      return istr(SL("abc::mstr – C string extraction"));
   }


   /** See str_test_case_base::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      dmstr s;
      auto psz(s.c_str());
      // s has no character array, so it should have returned the static NUL character.
      ABC_TESTING_ASSERT_NOT_EQUAL(psz.get(), s.cbegin().base());
      ABC_TESTING_ASSERT_FALSE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz.get()), 0u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], '\0');

      s = SL("");
      psz = s.c_str();
      // s still has no character array, so it should have returned the static NUL character again.
      ABC_TESTING_ASSERT_NOT_EQUAL(psz.get(), s.cbegin().base());
      ABC_TESTING_ASSERT_FALSE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz.get()), 0u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], '\0');

      s = SL("a");
      psz = s.c_str();
      // s should have copied the literal but dropped its trailing NUL, so it must’ve returned a
      // distinct character array.
      ABC_TESTING_ASSERT_NOT_EQUAL(psz.get(), s.cbegin().base());
      ABC_TESTING_ASSERT_TRUE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz.get()), 1u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], 'a');
      ABC_TESTING_ASSERT_EQUAL(psz[1], '\0');

      s += SL("b");
      psz = s.c_str();
      // The character array should have grown, but still lack the trailing NUL.
      ABC_TESTING_ASSERT_NOT_EQUAL(psz.get(), s.cbegin().base());
      ABC_TESTING_ASSERT_TRUE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::size_in_chars(psz.get()), 2u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], 'a');
      ABC_TESTING_ASSERT_EQUAL(psz[1], 'b');
      ABC_TESTING_ASSERT_EQUAL(psz[2], '\0');
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::mstr_c_str)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str_substr_ascii

namespace abc {
namespace test {

class str_substr_ascii :
   public testing::test_case {
public:

   /** See abc::testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::*str classes – ASCII character and substring search"));
   }


   /** See abc::testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      // ASCII character and substring search.
      // The string “acabaabca” has the following properties:
      // •  misleading start for “ab” at index 0 (it’s “ac” instead) and for “abc” at index 2 (it’s
      //    “aba” instead), to catch incorrect skip-last comparisons;
      // •  first and last characters match 'a', but other inner ones do too;
      // •  would match “abcd” were it not for the last character;
      // •  matches the self-repeating “abaabc” but not the (also self-repeating) “abaabcd”.
      istr const s(SL("acabaabca"));
      istr::const_iterator it;

      ABC_TESTING_ASSERT_EQUAL(s.find('b'), s.cbegin() + 3);
      ABC_TESTING_ASSERT_EQUAL(s.find(SL("ab")), s.cbegin() + 2);
      ABC_TESTING_ASSERT_EQUAL(s.find(SL("abca")), s.cbegin() + 5);
      ABC_TESTING_ASSERT_EQUAL(s.find(SL("abcd")), s.cend());
      ABC_TESTING_ASSERT_EQUAL(s.find(SL("abaabc")), s.cbegin() + 2);
      ABC_TESTING_ASSERT_EQUAL(s.find(SL("abaabcd")), s.cend());
      ABC_TESTING_ASSERT_EQUAL(s.find_last('b'), s.cend() - 3);
#if 0
      ABC_TESTING_ASSERT_EQUAL(s.find_last(SL("ab")), s.cend() - 4);
      ABC_TESTING_ASSERT_EQUAL(s.find_last(SL("ac")), s.cend() - 9);
      ABC_TESTING_ASSERT_EQUAL(s.find_last(SL("ca")), s.cend() - 2);
#endif
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str_substr_ascii)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str_substr_nonascii

namespace abc {
namespace test {

class str_substr_nonascii :
   public testing::test_case {
public:

   /** See abc::testing::test_case::title().
   */
   virtual istr title() {
      return istr(
         SL("abc::*str classes – non-ASCII character and substring search")
      );
   }


   /** See abc::testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FUNC(this);

      // Non-ASCII character and substring search.
      istr const s(SL("àßçàŒ"));
      istr::const_iterator it;

      ABC_TESTING_ASSERT_EQUAL(s.find(ABC_CHAR('ß')), s.cbegin() + 1);
      ABC_TESTING_ASSERT_EQUAL(s.find(SL("àß")), s.cbegin());
      ABC_TESTING_ASSERT_EQUAL(s.find(SL("àŒ")), s.cbegin() + 3);
      ABC_TESTING_ASSERT_EQUAL(s.find(SL("àü")), s.cend());

      ABC_TESTING_ASSERT_TRUE(s.starts_with(SL("")));
      ABC_TESTING_ASSERT_TRUE(s.starts_with(SL("à")));
      ABC_TESTING_ASSERT_TRUE(s.starts_with(SL("àß")));
      ABC_TESTING_ASSERT_FALSE(s.starts_with(SL("ß")));
      ABC_TESTING_ASSERT_FALSE(s.starts_with(SL("ßç")));
      ABC_TESTING_ASSERT_TRUE(s.starts_with(s));
      ABC_TESTING_ASSERT_FALSE(s.starts_with(s + SL("-")));
      ABC_TESTING_ASSERT_FALSE(s.starts_with(SL("-") + s));

      ABC_TESTING_ASSERT_TRUE(s.ends_with(SL("")));
      ABC_TESTING_ASSERT_TRUE(s.ends_with(SL("Œ")));
      ABC_TESTING_ASSERT_TRUE(s.ends_with(SL("àŒ")));
      ABC_TESTING_ASSERT_FALSE(s.ends_with(SL("à")));
      ABC_TESTING_ASSERT_FALSE(s.ends_with(SL("çà")));
      ABC_TESTING_ASSERT_TRUE(s.ends_with(s));
      ABC_TESTING_ASSERT_FALSE(s.ends_with(s + SL("-")));
      ABC_TESTING_ASSERT_FALSE(s.ends_with(SL("-") + s));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str_substr_nonascii)


////////////////////////////////////////////////////////////////////////////////////////////////////

