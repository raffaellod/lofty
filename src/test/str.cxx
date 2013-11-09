/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013
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
#include <abc/trace.hxx>


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


   /** Checks if a string’s item array has been reallocated, and verifies its character count.

   bPtrChanged
      Validates that the string’s character array pointer has changed if true, or that it has not
      changed if false.
   return
      true if the validation is successful, or false otherwise.
   */
   bool str_ptr_changed(bool bPtrChanged) {
      ABC_TRACE_FN((this, bPtrChanged));

      // Update the item array pointer for the next call.
      char_t const * pchCheckOld(m_pchCheck);
      m_pchCheck = m_psCheck->data();
      // Check if the item array has changed in accordance to the expectation.
      if ((pchCheckOld != m_psCheck->data()) != bPtrChanged) {
         return false;
      }
      return true;
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
      return istr(SL("str classes - basic operations"));
   }


   /** See str_test_case_base::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      dmstr s;
      init_str_ptr(s);

      // Only the trailing NUL character should be accessible.
      ABC_TESTING_ASSERT_THROWS(index_error, s[-1]);
      ABC_TESTING_ASSERT_DOES_NOT_THROW(s[0]);
      ABC_TESTING_ASSERT_THROWS(index_error, s[1]);

      s += SL("a");
      // true: operator+= must have created an item array (there was none).
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed(true));
      ABC_TESTING_ASSERT_EQUAL(s.size(), 1);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 1);
      ABC_TESTING_ASSERT_EQUAL(s[0], CL('a'));

      s = s + CL('b') + s;
      // true: a new string is created by operator+, which replaces s by operator=.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed(true));
      ABC_TESTING_ASSERT_EQUAL(s.size(), 3);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 3);
      ABC_TESTING_ASSERT_EQUAL(s, SL("aba"));

      s = s.substr(1, 3);
      // true: s got replaced by operator=.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed(true));
      ABC_TESTING_ASSERT_EQUAL(s.size(), 2);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 2);
      ABC_TESTING_ASSERT_EQUAL(s, SL("ba"));

      s += CL('c');
      // false: there should’ve been enough space for 'c'.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed(false));
      ABC_TESTING_ASSERT_EQUAL(s.size(), 3);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 3);
      ABC_TESTING_ASSERT_EQUAL(s, SL("bac"));

      s = s.substr(0, -1);
      // true: s got replaced by operator=.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed(true));
      ABC_TESTING_ASSERT_EQUAL(s.size(), 2);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 2);
      ABC_TESTING_ASSERT_EQUAL(s[0], CL('b'));
      ABC_TESTING_ASSERT_EQUAL(s[1], CL('a'));

      s += s;
      // false: there should’ve been enough space for “baba”.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed(false));
      ABC_TESTING_ASSERT_EQUAL(s.size(), 4);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 4);
      ABC_TESTING_ASSERT_EQUAL(s[0], CL('b'));
      ABC_TESTING_ASSERT_EQUAL(s[1], CL('a'));
      ABC_TESTING_ASSERT_EQUAL(s[2], CL('b'));
      ABC_TESTING_ASSERT_EQUAL(s[3], CL('a'));

      s = s.substr(-3, -2);
      // true: s got replaced by operator=.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed(true));
      ABC_TESTING_ASSERT_EQUAL(s.size(), 1);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 1);
      ABC_TESTING_ASSERT_EQUAL(s[0], CL('a'));

      s = dmstr(SL("ab")) + CL('c');
      // true: s got replaced by operator=.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed(true));
      ABC_TESTING_ASSERT_EQUAL(s.size(), 3);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 3);
      ABC_TESTING_ASSERT_EQUAL(s[0], CL('a'));
      ABC_TESTING_ASSERT_EQUAL(s[1], CL('b'));
      ABC_TESTING_ASSERT_EQUAL(s[2], CL('c'));

      s += CL('d');
      // false: there should’ve been enough space for “abcd”.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed(false));
      ABC_TESTING_ASSERT_EQUAL(s.size(), 4);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 4);
      ABC_TESTING_ASSERT_EQUAL(s[0], CL('a'));
      ABC_TESTING_ASSERT_EQUAL(s[1], CL('b'));
      ABC_TESTING_ASSERT_EQUAL(s[2], CL('c'));
      ABC_TESTING_ASSERT_EQUAL(s[3], CL('d'));

      s += SL("efghijklmnopqrstuvwxyz");
      // Cannot assert (ABC_TESTING_ASSERT_*) on this to behave in any specific way, since the
      // character array may or may not change depending on heap reallocation strategy.
      str_ptr_changed(false);
      ABC_TESTING_ASSERT_EQUAL(s.size(), 26);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 26);
      ABC_TESTING_ASSERT_EQUAL(s, SL("abcdefghijklmnopqrstuvwxyz"));

      s = SL("a\0b");
      s += SL("\0c");
      // false: there should have been plenty of storage allocated.
      ABC_TESTING_ASSERT_TRUE(str_ptr_changed(false));
      ABC_TESTING_ASSERT_EQUAL(s.size(), 5);
      ABC_TESTING_ASSERT_GREATER_EQUAL(s.capacity(), 5);
      // Test both ways to make sure that the char_t[] overload is always chosen over char *.
      ABC_TESTING_ASSERT_EQUAL(s, SL("a\0b\0c"));
      ABC_TESTING_ASSERT_EQUAL(SL("a\0b\0c"), s);
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str_basic)


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
      return istr(SL("str classes - ASCII character and substring search - UTF-8 strings"));
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
      return istr(SL("str classes - ASCII character and substring search - UTF-16 strings"));
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
      return istr(SL("str classes - ASCII character and substring search - UTF-32 strings"));
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
      return istr(SL("str classes - non-ASCII character and substring search - UTF-8 strings"));
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
      return istr(SL("str classes - non-ASCII character and substring search - UTF-16 strings"));
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
      return istr(SL("str classes - non-ASCII character and substring search - UTF-32 strings"));
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
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str32_substr_nonascii)

#endif //ifdef U32SL


////////////////////////////////////////////////////////////////////////////////////////////////////

