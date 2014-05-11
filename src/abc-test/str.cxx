﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013, 2014
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
      ABC_TRACE_FN((this, s));

      m_psCheck = &s;
      m_pchCheck = s.data();
   }


   /** Checks if a string’s item array has been reallocated.

   return
      true if the string’s character array pointer has changed, or false otherwise.
   */
   bool str_ptr_changed() {
      ABC_TRACE_FN((this));

      // Update the item array pointer for the next call.
      char_t const * pchCheckOld(m_pchCheck);
      m_pchCheck = m_psCheck->data();
      // Check if the item array has changed.
      return pchCheckOld != m_psCheck->data();
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
      return istr(SL("abc::*str classes - basic operations"));
   }


   /** See str_test_case_base::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      dmstr s;
      init_str_ptr(s);

      // Only the trailing NUL character should be accessible.
      ABC_TESTING_ASSERT_THROWS(index_error, s[-1]);
      ABC_TESTING_ASSERT_THROWS(index_error, s[0]);

      s += SL("a");
      // true: operator+= must have created an item array (there was none).
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed());
      ABC_TESTING_ASSERT_DOES_NOT_THROW(s[0]);
      ABC_TESTING_ASSERT_EQUAL(s.size(), 1u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 1u);
      ABC_TESTING_ASSERT_EQUAL(s[0], CL('a'));

      s = s + CL('b') + s;
      // true: a new string is created by operator+, which replaces s by operator=.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 3u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 3u);
      ABC_TESTING_ASSERT_EQUAL(s, SL("aba"));

      s = s.substr(1, 3);
      // true: s got replaced by operator=.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 2u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 2u);
      ABC_TESTING_ASSERT_EQUAL(s, SL("ba"));

      s += CL('c');
      // false: there should’ve been enough space for 'c'.
      ABC_TESTING_ASSERT_FALSE(str_ptr_changed());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 3u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 3u);
      ABC_TESTING_ASSERT_EQUAL(s, SL("bac"));

      s = s.substr(0, -1);
      // true: s got replaced by operator=.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 2u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 2u);
      ABC_TESTING_ASSERT_EQUAL(s[0], CL('b'));
      ABC_TESTING_ASSERT_EQUAL(s[1], CL('a'));

      s += s;
      // false: there should’ve been enough space for “baba”.
      ABC_TESTING_ASSERT_FALSE(str_ptr_changed());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 4u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 4u);
      ABC_TESTING_ASSERT_EQUAL(s[0], CL('b'));
      ABC_TESTING_ASSERT_EQUAL(s[1], CL('a'));
      ABC_TESTING_ASSERT_EQUAL(s[2], CL('b'));
      ABC_TESTING_ASSERT_EQUAL(s[3], CL('a'));

      s = s.substr(-3, -2);
      // true: s got replaced by operator=.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 1u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 1u);
      ABC_TESTING_ASSERT_EQUAL(s[0], CL('a'));

      s = dmstr(SL("ab")) + CL('c');
      // true: s got replaced by operator=.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 3u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 3u);
      ABC_TESTING_ASSERT_EQUAL(s[0], CL('a'));
      ABC_TESTING_ASSERT_EQUAL(s[1], CL('b'));
      ABC_TESTING_ASSERT_EQUAL(s[2], CL('c'));

      s += CL('d');
      // false: there should’ve been enough space for “abcd”.
      ABC_TESTING_ASSERT_FALSE(str_ptr_changed());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 4u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 4u);
      ABC_TESTING_ASSERT_EQUAL(s[0], CL('a'));
      ABC_TESTING_ASSERT_EQUAL(s[1], CL('b'));
      ABC_TESTING_ASSERT_EQUAL(s[2], CL('c'));
      ABC_TESTING_ASSERT_EQUAL(s[3], CL('d'));

      s += SL("efghijklmnopqrstuvwxyz");
      // Cannot assert (ABC_TESTING_ASSERT_*) on this to behave in any specific way, since the
      // character array may or may not change depending on heap reallocation strategy.
      str_ptr_changed();
      ABC_TESTING_ASSERT_EQUAL(s.size(), 26u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 26u);
      ABC_TESTING_ASSERT_EQUAL(s, SL("abcdefghijklmnopqrstuvwxyz"));

      s = SL("a\0b");
      s += SL("\0c");
      // false: there should have been plenty of storage allocated.
      ABC_TESTING_ASSERT_FALSE(str_ptr_changed());
      ABC_TESTING_ASSERT_EQUAL(s.size(), 5u);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 5u);
      // Test both ways to make sure that the char_t[] overload is always chosen over char *.
      ABC_TESTING_ASSERT_EQUAL(s, SL("a\0b\0c"));
      ABC_TESTING_ASSERT_EQUAL(SL("a\0b\0c"), s);
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str_basic)


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
      return istr(SL("abc::*str classes - substring extraction"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

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
      return istr(SL("abc::istr - C string extraction"));
   }


   /** See str_test_case_base::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      istr s;
      auto psz(s.c_str());
      // s has no character array, so it should have returned the static NUL character.
      ABC_TESTING_ASSERT_NOT_EQUAL(psz.get(), s.data());
      ABC_TESTING_ASSERT_FALSE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::utf_traits<>::str_len(psz.get()), 0u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], CL('\0'));

      s = SL("");
      psz = s.c_str();
      // s should have adopted the literal and therefore have a trailing NUL, so it should have
      // returned its own character array.
      ABC_TESTING_ASSERT_EQUAL(psz.get(), s.data());
      ABC_TESTING_ASSERT_FALSE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::utf_traits<>::str_len(psz.get()), 0u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], CL('\0'));

      s = SL("a");
      psz = s.c_str();
      // s should have adopted the literal and therefore have a trailing NUL, so it should have
      // returned its own character array.
      ABC_TESTING_ASSERT_EQUAL(psz.get(), s.data());
      ABC_TESTING_ASSERT_FALSE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::utf_traits<>::str_len(psz.get()), 1u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], CL('a'));
      ABC_TESTING_ASSERT_EQUAL(psz[1], CL('\0'));
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
      return istr(SL("abc::mstr - C string extraction"));
   }


   /** See str_test_case_base::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      dmstr s;
      auto psz(s.c_str());
      // s has no character array, so it should have returned the static NUL character.
      ABC_TESTING_ASSERT_NOT_EQUAL(psz.get(), s.data());
      ABC_TESTING_ASSERT_FALSE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::utf_traits<>::str_len(psz.get()), 0u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], CL('\0'));

      s = SL("");
      psz = s.c_str();
      // s still has no character array, so it should have returned the static NUL character again.
      ABC_TESTING_ASSERT_NOT_EQUAL(psz.get(), s.data());
      ABC_TESTING_ASSERT_FALSE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::utf_traits<>::str_len(psz.get()), 0u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], CL('\0'));

      s = SL("a");
      psz = s.c_str();
      // s should have copied the literal but dropped its trailing NUL, so it must’ve returned a
      // distinct character array.
      ABC_TESTING_ASSERT_NOT_EQUAL(psz.get(), s.data());
      ABC_TESTING_ASSERT_TRUE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::utf_traits<>::str_len(psz.get()), 1u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], CL('a'));
      ABC_TESTING_ASSERT_EQUAL(psz[1], CL('\0'));

      s += SL("b");
      psz = s.c_str();
      // The character array should have grown, but still lack the trailing NUL.
      ABC_TESTING_ASSERT_NOT_EQUAL(psz.get(), s.data());
      ABC_TESTING_ASSERT_TRUE(psz.get_deleter().enabled());
      ABC_TESTING_ASSERT_EQUAL(text::utf_traits<>::str_len(psz.get()), 2u);
      ABC_TESTING_ASSERT_EQUAL(psz[0], CL('a'));
      ABC_TESTING_ASSERT_EQUAL(psz[1], CL('b'));
      ABC_TESTING_ASSERT_EQUAL(psz[2], CL('\0'));
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::mstr_c_str)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str8_substr_ascii

#ifdef U8SL

namespace abc {

namespace test {

class str8_substr_ascii :
   public testing::test_case {
public:

   /** See abc::testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::*str classes - ASCII character and substring search - UTF-8 strings"));
   }


   /** See abc::testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      // ASCII character and substring search.
      // The string “acabaabca” has the following properties:
      // •  misleading start for “ab” at index 0 (it’s “ac” instead) and for “abc” at index 2 (it’s
      //    “aba” instead), to catch incorrect skip-last comparisons;
      // •  first and last characters match 'a', but other inner ones do too;
      // •  would match “abcd” were it not for the last character;
      // •  matches the self-repeating “abaabc” but not the (also self-repeating) “abaabcd”.
      istr8 const s8(U8SL("acabaabca"));
      istr8::const_iterator it;

      ABC_TESTING_ASSERT_EQUAL(s8.find(U32CL('b')), s8.cbegin() + 3);
      ABC_TESTING_ASSERT_EQUAL(s8.find(U8SL("ab")), s8.cbegin() + 2);
      ABC_TESTING_ASSERT_EQUAL(s8.find(U8SL("abca")), s8.cbegin() + 5);
      ABC_TESTING_ASSERT_EQUAL(s8.find(U8SL("abcd")), s8.cend());
      ABC_TESTING_ASSERT_EQUAL(s8.find(U8SL("abaabc")), s8.cbegin() + 2);
      ABC_TESTING_ASSERT_EQUAL(s8.find(U8SL("abaabcd")), s8.cend());
      ABC_TESTING_ASSERT_EQUAL(s8.find_last(U32CL('b')), s8.cend() - 3);
#if 0
      ABC_TESTING_ASSERT_EQUAL(s8.find_last(U8SL("ab")), s8.cend() - 4);
      ABC_TESTING_ASSERT_EQUAL(s8.find_last(U8SL("ac")), s8.cend() - 9);
      ABC_TESTING_ASSERT_EQUAL(s8.find_last(U8SL("ca")), s8.cend() - 2);
#endif
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str8_substr_ascii)

#endif //ifdef U8SL


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str16_substr_ascii

#ifdef U16SL

namespace abc {

namespace test {

class str16_substr_ascii :
   public testing::test_case {
public:

   /** See abc::testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::*str classes - ASCII character and substring search - UTF-16 strings"));
   }


   /** See abc::testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      // ASCII character and substring search.
      // The string “acabaabca” has the following properties:
      // •  misleading start for “ab” at index 0 (it’s “ac” instead) and for “abc” at index 2 (it’s
      //    “aba” instead), to catch incorrect skip-last comparisons;
      // •  first and last characters match 'a', but other inner ones do too;
      // •  would match “abcd” were it not for the last character;
      // •  matches the self-repeating “abaabc” but not the (also self-repeating) “abaabcd”.
      istr16 const s16(U16SL("acabaabca"));
      istr16::const_iterator it;

      ABC_TESTING_ASSERT_EQUAL(s16.find(U32CL('b')), s16.cbegin() + 3);
      ABC_TESTING_ASSERT_EQUAL(s16.find(U16SL("ab")), s16.cbegin() + 2);
      ABC_TESTING_ASSERT_EQUAL(s16.find(U16SL("abca")), s16.cbegin() + 5);
      ABC_TESTING_ASSERT_EQUAL(s16.find(U16SL("abcd")), s16.cend());
      ABC_TESTING_ASSERT_EQUAL(s16.find(U16SL("abaabc")), s16.cbegin() + 2);
      ABC_TESTING_ASSERT_EQUAL(s16.find(U16SL("abaabcd")), s16.cend());
      ABC_TESTING_ASSERT_EQUAL(s16.find_last(U32CL('b')), s16.cend() - 3);
#if 0
      ABC_TESTING_ASSERT_EQUAL(s16.find_last(U16SL("ab")), s16.cend() - 4);
      ABC_TESTING_ASSERT_EQUAL(s16.find_last(U16SL("ac")), s16.cend() - 9);
      ABC_TESTING_ASSERT_EQUAL(s16.find_last(U16SL("ca")), s16.cend() - 2);
#endif
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str16_substr_ascii)

#endif //ifdef U16SL


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str32_substr_ascii

#ifdef U32SL

namespace abc {

namespace test {

class str32_substr_ascii :
   public testing::test_case {
public:

   /** See abc::testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::*str classes - ASCII character and substring search - UTF-32 strings"));
   }


   /** See abc::testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      // ASCII character and substring search.
      // The string “acabaabca” has the following properties:
      // •  misleading start for “ab” at index 0 (it’s “ac” instead) and for “abc” at index 2 (it’s
      //    “aba” instead), to catch incorrect skip-last comparisons;
      // •  first and last characters match 'a', but other inner ones do too;
      // •  would match “abcd” were it not for the last character;
      // •  matches the self-repeating “abaabc” but not the (also self-repeating) “abaabcd”.
      istr32 const s32(U32SL("acabaabca"));
      istr32::const_iterator it;

      ABC_TESTING_ASSERT_EQUAL(s32.find(U32CL('b')), s32.cbegin() + 3);
      ABC_TESTING_ASSERT_EQUAL(s32.find(U32SL("ab")), s32.cbegin() + 2);
      ABC_TESTING_ASSERT_EQUAL(s32.find(U32SL("abca")), s32.cbegin() + 5);
      ABC_TESTING_ASSERT_EQUAL(s32.find(U32SL("abcd")), s32.cend());
      ABC_TESTING_ASSERT_EQUAL(s32.find(U32SL("abaabc")), s32.cbegin() + 2);
      ABC_TESTING_ASSERT_EQUAL(s32.find(U32SL("abaabcd")), s32.cend());
      ABC_TESTING_ASSERT_EQUAL(s32.find_last(U32CL('b')), s32.cend() - 3);
#if 0
      ABC_TESTING_ASSERT_EQUAL(s32.find_last(U32SL("ab")), s32.cend() - 4);
      ABC_TESTING_ASSERT_EQUAL(s32.find_last(U32SL("ac")), s32.cend() - 9);
      ABC_TESTING_ASSERT_EQUAL(s32.find_last(U32SL("ca")), s32.cend() - 2);
#endif
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str32_substr_ascii)

#endif //ifdef U32SL


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str8_substr_nonascii

#ifdef U8SL

namespace abc {

namespace test {

class str8_substr_nonascii :
   public testing::test_case {
public:

   /** See abc::testing::test_case::title().
   */
   virtual istr title() {
      return istr(
         SL("abc::*str classes - non-ASCII character and substring search - UTF-8 strings")
      );
   }


   /** See abc::testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      // Non-ASCII character and substring search.
      istr8 const s8(U8SL("àßçàŒ"));
      istr8::const_iterator it;

      ABC_TESTING_ASSERT_EQUAL(s8.find(U32CL('ß')), s8.cbegin() + 2);
      ABC_TESTING_ASSERT_EQUAL(s8.find(U8SL("àß")), s8.cbegin() + 0);
      ABC_TESTING_ASSERT_EQUAL(s8.find(U8SL("àŒ")), s8.cbegin() + 6);
      ABC_TESTING_ASSERT_EQUAL(s8.find(U8SL("àü")), s8.cend());

      ABC_TESTING_ASSERT_TRUE(s8.starts_with(U8SL("")));
      ABC_TESTING_ASSERT_TRUE(s8.starts_with(U8SL("à")));
      ABC_TESTING_ASSERT_TRUE(s8.starts_with(U8SL("àß")));
      ABC_TESTING_ASSERT_FALSE(s8.starts_with(U8SL("ß")));
      ABC_TESTING_ASSERT_FALSE(s8.starts_with(U8SL("ßç")));
      ABC_TESTING_ASSERT_TRUE(s8.starts_with(s8));
      ABC_TESTING_ASSERT_FALSE(s8.starts_with(s8 + U8SL("-")));
      ABC_TESTING_ASSERT_FALSE(s8.starts_with(U8SL("-") + s8));

      ABC_TESTING_ASSERT_TRUE(s8.ends_with(U8SL("")));
      ABC_TESTING_ASSERT_TRUE(s8.ends_with(U8SL("Œ")));
      ABC_TESTING_ASSERT_TRUE(s8.ends_with(U8SL("àŒ")));
      ABC_TESTING_ASSERT_FALSE(s8.ends_with(U8SL("à")));
      ABC_TESTING_ASSERT_FALSE(s8.ends_with(U8SL("çà")));
      ABC_TESTING_ASSERT_TRUE(s8.ends_with(s8));
      ABC_TESTING_ASSERT_FALSE(s8.ends_with(s8 + U8SL("-")));
      ABC_TESTING_ASSERT_FALSE(s8.ends_with(U8SL("-") + s8));
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str8_substr_nonascii)

#endif //ifdef U8SL


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str16_substr_nonascii

#ifdef U16SL

namespace abc {

namespace test {

class str16_substr_nonascii :
   public testing::test_case {
public:

   /** See abc::testing::test_case::title().
   */
   virtual istr title() {
      return istr(
         SL("abc::*str classes - non-ASCII character and substring search - UTF-16 strings")
      );
   }


   /** See abc::testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      // Non-ASCII character and substring search.
      istr16 const s16(U16SL("àßçàŒ"));
      istr16::const_iterator it;

      ABC_TESTING_ASSERT_EQUAL(s16.find(U32CL('ß')), s16.cbegin() + 1);
      ABC_TESTING_ASSERT_EQUAL(s16.find(U16SL("àß")), s16.cbegin() + 0);
      ABC_TESTING_ASSERT_EQUAL(s16.find(U16SL("àŒ")), s16.cbegin() + 3);
      ABC_TESTING_ASSERT_EQUAL(s16.find(U16SL("àü")), s16.cend());

      ABC_TESTING_ASSERT_TRUE(s16.starts_with(U16SL("")));
      ABC_TESTING_ASSERT_TRUE(s16.starts_with(U16SL("à")));
      ABC_TESTING_ASSERT_TRUE(s16.starts_with(U16SL("àß")));
      ABC_TESTING_ASSERT_FALSE(s16.starts_with(U16SL("ß")));
      ABC_TESTING_ASSERT_FALSE(s16.starts_with(U16SL("ßç")));
      ABC_TESTING_ASSERT_TRUE(s16.starts_with(s16));
      ABC_TESTING_ASSERT_FALSE(s16.starts_with(s16 + U16SL("-")));
      ABC_TESTING_ASSERT_FALSE(s16.starts_with(U16SL("-") + s16));

      ABC_TESTING_ASSERT_TRUE(s16.ends_with(U16SL("")));
      ABC_TESTING_ASSERT_TRUE(s16.ends_with(U16SL("Œ")));
      ABC_TESTING_ASSERT_TRUE(s16.ends_with(U16SL("àŒ")));
      ABC_TESTING_ASSERT_FALSE(s16.ends_with(U16SL("à")));
      ABC_TESTING_ASSERT_FALSE(s16.ends_with(U16SL("çà")));
      ABC_TESTING_ASSERT_TRUE(s16.ends_with(s16));
      ABC_TESTING_ASSERT_FALSE(s16.ends_with(s16 + U16SL("-")));
      ABC_TESTING_ASSERT_FALSE(s16.ends_with(U16SL("-") + s16));
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str16_substr_nonascii)

#endif //ifdef U16SL


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str32_substr_nonascii

#ifdef U32SL

namespace abc {

namespace test {

class str32_substr_nonascii :
   public testing::test_case {
public:

   /** See abc::testing::test_case::title().
   */
   virtual istr title() {
      return istr(
         SL("abc::*str classes - non-ASCII character and substring search - UTF-32 strings")
      );
   }


   /** See abc::testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      // Non-ASCII character and substring search.
      istr32 const s32(U32SL("àßçàŒ"));
      istr32::const_iterator it;

      ABC_TESTING_ASSERT_EQUAL(s32.find(U32CL('ß')), s32.cbegin() + 1);
      ABC_TESTING_ASSERT_EQUAL(s32.find(U32SL("àß")), s32.cbegin() + 0);
      ABC_TESTING_ASSERT_EQUAL(s32.find(U32SL("àŒ")), s32.cbegin() + 3);
      ABC_TESTING_ASSERT_EQUAL(s32.find(U32SL("àü")), s32.cend());

      ABC_TESTING_ASSERT_TRUE(s32.starts_with(U32SL("")));
      ABC_TESTING_ASSERT_TRUE(s32.starts_with(U32SL("à")));
      ABC_TESTING_ASSERT_TRUE(s32.starts_with(U32SL("àß")));
      ABC_TESTING_ASSERT_FALSE(s32.starts_with(U32SL("ß")));
      ABC_TESTING_ASSERT_FALSE(s32.starts_with(U32SL("ßç")));
      ABC_TESTING_ASSERT_TRUE(s32.starts_with(s32));
      ABC_TESTING_ASSERT_FALSE(s32.starts_with(s32 + U32SL("-")));
      ABC_TESTING_ASSERT_FALSE(s32.starts_with(U32SL("-") + s32));

      ABC_TESTING_ASSERT_TRUE(s32.ends_with(U32SL("")));
      ABC_TESTING_ASSERT_TRUE(s32.ends_with(U32SL("Œ")));
      ABC_TESTING_ASSERT_TRUE(s32.ends_with(U32SL("àŒ")));
      ABC_TESTING_ASSERT_FALSE(s32.ends_with(U32SL("à")));
      ABC_TESTING_ASSERT_FALSE(s32.ends_with(U32SL("çà")));
      ABC_TESTING_ASSERT_TRUE(s32.ends_with(s32));
      ABC_TESTING_ASSERT_FALSE(s32.ends_with(s32 + U32SL("-")));
      ABC_TESTING_ASSERT_FALSE(s32.ends_with(U32SL("-") + s32));
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str32_substr_nonascii)

#endif //ifdef U32SL


////////////////////////////////////////////////////////////////////////////////////////////////////

