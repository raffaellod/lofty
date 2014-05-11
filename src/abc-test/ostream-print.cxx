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

#include <abc/testing/test_case.hxx>
#include <abc/testing/mock/iostream.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::ostream_print_no_replacements

namespace abc {

namespace test {

class ostream_print_no_replacements :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::io::ostream::print() - no replacements"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      testing::mock::ostream mos;

      // Syntax errors.
      mos.reset();
      ABC_TESTING_ASSERT_THROWS(syntax_error, mos.print(SL("{")));
      mos.reset();
      ABC_TESTING_ASSERT_THROWS(syntax_error, mos.print(SL("{{{")));
      mos.reset();
      ABC_TESTING_ASSERT_THROWS(syntax_error, mos.print(SL("}")));
      mos.reset();
      ABC_TESTING_ASSERT_THROWS(syntax_error, mos.print(SL("}}}")));

      // No replacements.
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("")), mos.contents()), SL(""));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("x")), mos.contents()), SL("x"));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("x"), SL("a")), mos.contents()), SL("x"));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("{{")), mos.contents()), SL("{"));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("}}")), mos.contents()), SL("}"));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("{{}}")), mos.contents()), SL("{}"));
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::ostream_print_no_replacements)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::ostream_print_one_replacement

namespace abc {

namespace test {

class ostream_print_one_replacement :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::io::ostream::print() - one replacement"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      testing::mock::ostream mos;

      // Single string replacement, deduced argument index.
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("{}"), SL("a")), mos.contents()), SL("a"));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("x{}"), SL("a")), mos.contents()), SL("xa"));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("{}x"), SL("a")), mos.contents()), SL("ax"));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("x{}x"), SL("a")), mos.contents()), SL("xax"));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("{{{}}}"), SL("a")), mos.contents()), SL("{a}"));

      // Single string replacement, explicit index.
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("{0}"), SL("a")), mos.contents()), SL("a"));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("x{0}"), SL("a")), mos.contents()), SL("xa"));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("{0}x"), SL("a")), mos.contents()), SL("ax"));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("x{0}x"), SL("a")), mos.contents()), SL("xax"));

      // Single integer replacement, various ways of reference, various format options.
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("{}"), 34), mos.contents()), SL("34"));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("{:x}"), 34), mos.contents()), SL("22"));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("{:#x}"), 34), mos.contents()), SL("0x22"));
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::ostream_print_one_replacement)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::ostream_print_two_replacements

namespace abc {

namespace test {

class ostream_print_two_replacements :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::io::ostream::print() - two replacements"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      testing::mock::ostream mos;

      // Single string replacement, referenced twice.
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("{0}{0}"), SL("a")), mos.contents()), SL("aa"));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("{0}x{0}"), SL("a")), mos.contents()), SL("axa"));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("x{0}x{0}"), SL("a")), mos.contents()), SL("xaxa"));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("{0}x{0}x"), SL("a")), mos.contents()), SL("axax"));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("x{0}x{0}x"), SL("a")), mos.contents()), SL("xaxax"));

      // Two string replacements, various ways of reference.
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL((mos.print(SL("{}{}"), SL("a"), SL("b")), mos.contents()), SL("ab"));
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL(
         (mos.print(SL("{0}{1}"), SL("a"), SL("b")), mos.contents()), SL("ab")
      );
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL(
         (mos.print(SL("{1}{0}"), SL("a"), SL("b")), mos.contents()), SL("ba")
      );
      mos.reset();
      ABC_TESTING_ASSERT_EQUAL(
         (mos.print(SL("{1}{1}"), SL("a"), SL("b")), mos.contents()), SL("bb")
      );
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::ostream_print_two_replacements)


////////////////////////////////////////////////////////////////////////////////////////////////////

