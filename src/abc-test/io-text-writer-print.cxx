/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

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

#include <abc/core.hxx>
#include <abc/testing/test_case.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::text_writer_print_no_replacements

namespace abc {
namespace test {

class text_writer_print_no_replacements :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::io::text::writer::print() - no replacements"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      smstr<128> sWriterBuffer;
      io::text::str_writer stw(&sWriterBuffer);

      // Syntax errors.
      stw.clear();
      ABC_TESTING_ASSERT_THROWS(syntax_error, stw.print(SL("{")));
      stw.clear();
      ABC_TESTING_ASSERT_THROWS(syntax_error, stw.print(SL("{{{")));
      stw.clear();
      ABC_TESTING_ASSERT_THROWS(syntax_error, stw.print(SL("}")));
      stw.clear();
      ABC_TESTING_ASSERT_THROWS(syntax_error, stw.print(SL("}}}")));

      // No replacements.
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("")), stw.get_str()), SL(""));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("x")), stw.get_str()), SL("x"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("x"), SL("a")), stw.get_str()), SL("x"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("{{")), stw.get_str()), SL("{"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("}}")), stw.get_str()), SL("}"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("{{}}")), stw.get_str()), SL("{}"));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::text_writer_print_no_replacements)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::text_writer_print_one_replacement

namespace abc {
namespace test {

class text_writer_print_one_replacement :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::io::text_writer::print() - one replacement"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      smstr<128> sWriterBuffer;
      io::text::str_writer stw(&sWriterBuffer);

      // Single string replacement, deduced argument index.
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("{}"), SL("a")), stw.get_str()), SL("a"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("x{}"), SL("a")), stw.get_str()), SL("xa"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("{}x"), SL("a")), stw.get_str()), SL("ax"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("x{}x"), SL("a")), stw.get_str()), SL("xax"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("{{{}}}"), SL("a")), stw.get_str()), SL("{a}"));

      // Single string replacement, explicit index.
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("{0}"), SL("a")), stw.get_str()), SL("a"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("x{0}"), SL("a")), stw.get_str()), SL("xa"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("{0}x"), SL("a")), stw.get_str()), SL("ax"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("x{0}x"), SL("a")), stw.get_str()), SL("xax"));

      // Single integer replacement, various ways of reference, various format options.
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("{}"), 34), stw.get_str()), SL("34"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("{:x}"), 34), stw.get_str()), SL("22"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("{:#x}"), 34), stw.get_str()), SL("0x22"));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::text_writer_print_one_replacement)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::text_writer_print_two_replacements

namespace abc {
namespace test {

class text_writer_print_two_replacements :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::io::text_writer::print() - two replacements"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      smstr<128> sWriterBuffer;
      io::text::str_writer stw(&sWriterBuffer);

      // Single string replacement, referenced twice.
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("{0}{0}"), SL("a")), stw.get_str()), SL("aa"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("{0}x{0}"), SL("a")), stw.get_str()), SL("axa"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("x{0}x{0}"), SL("a")), stw.get_str()), SL("xaxa"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("{0}x{0}x"), SL("a")), stw.get_str()), SL("axax"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("x{0}x{0}x"), SL("a")), stw.get_str()), SL("xaxax"));

      // Two string replacements, various ways of reference.
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(SL("{}{}"), SL("a"), SL("b")), stw.get_str()), SL("ab"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(SL("{0}{1}"), SL("a"), SL("b")), stw.get_str()), SL("ab")
      );
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(SL("{1}{0}"), SL("a"), SL("b")), stw.get_str()), SL("ba")
      );
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(SL("{1}{1}"), SL("a"), SL("b")), stw.get_str()), SL("bb")
      );
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::text_writer_print_two_replacements)


////////////////////////////////////////////////////////////////////////////////////////////////////

