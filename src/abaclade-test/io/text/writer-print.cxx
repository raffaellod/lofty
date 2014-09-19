﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

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
// abc::test::text_writer_print_no_replacements

namespace abc {
namespace test {

class text_writer_print_no_replacements : public testing::test_case {
public:
   //! See testing::test_case::title().
   virtual istr title() override {
      return istr(ABC_SL("abc::io::text::writer::print() – no replacements"));
   }

   //! See testing::test_case::run().
   virtual void run() override {
      ABC_TRACE_FUNC(this);

      smstr<128> sWriterBuffer;
      io::text::str_writer stw(&sWriterBuffer);

      // Syntax errors.
      stw.clear();
      ABC_TESTING_ASSERT_THROWS(syntax_error, stw.print(ABC_SL("{")));
      stw.clear();
      ABC_TESTING_ASSERT_THROWS(syntax_error, stw.print(ABC_SL("{{{")));
      stw.clear();
      ABC_TESTING_ASSERT_THROWS(syntax_error, stw.print(ABC_SL("}")));
      stw.clear();
      ABC_TESTING_ASSERT_THROWS(syntax_error, stw.print(ABC_SL("}}}")));

      // No replacements.
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(istr::empty), stw.get_str()), istr::empty);
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(ABC_SL("x")), stw.get_str()), ABC_SL("x"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(ABC_SL("x"), ABC_SL("a")), stw.get_str()), ABC_SL("x"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(ABC_SL("{{")), stw.get_str()), ABC_SL("{"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(ABC_SL("}}")), stw.get_str()), ABC_SL("}"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(ABC_SL("{{}}")), stw.get_str()), ABC_SL("{}"));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::text_writer_print_no_replacements)

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::text_writer_print_one_replacement

namespace abc {
namespace test {

class text_writer_print_one_replacement : public testing::test_case {
public:
   //! See testing::test_case::title().
   virtual istr title() override {
      return istr(ABC_SL("abc::io::text_writer::print() – one replacement"));
   }

   //! See testing::test_case::run().
   virtual void run() override {
      ABC_TRACE_FUNC(this);

      smstr<128> sWriterBuffer;
      io::text::str_writer stw(&sWriterBuffer);

      // Single string replacement, deduced argument index.
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(ABC_SL("{}"), ABC_SL("a")), stw.get_str()), ABC_SL("a")
      );
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(ABC_SL("x{}"), ABC_SL("a")), stw.get_str()), ABC_SL("xa")
      );
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(ABC_SL("{}x"), ABC_SL("a")), stw.get_str()), ABC_SL("ax")
      );
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(ABC_SL("x{}x"), ABC_SL("a")), stw.get_str()), ABC_SL("xax")
      );
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(ABC_SL("{{{}}}"), ABC_SL("a")), stw.get_str()), ABC_SL("{a}")
      );

      // Single string replacement, explicit index.
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(ABC_SL("{0}"), ABC_SL("a")), stw.get_str()), ABC_SL("a")
      );
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(ABC_SL("x{0}"), ABC_SL("a")), stw.get_str()), ABC_SL("xa")
      );
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(ABC_SL("{0}x"), ABC_SL("a")), stw.get_str()), ABC_SL("ax")
      );
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(ABC_SL("x{0}x"), ABC_SL("a")), stw.get_str()), ABC_SL("xax")
      );

      // Single integer replacement, various ways of reference, various format options.
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(ABC_SL("{}"), 34), stw.get_str()), ABC_SL("34"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(ABC_SL("{:x}"), 34), stw.get_str()), ABC_SL("22"));
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL((stw.print(ABC_SL("{:#x}"), 34), stw.get_str()), ABC_SL("0x22"));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::text_writer_print_one_replacement)

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::text_writer_print_two_replacements

namespace abc {
namespace test {

class text_writer_print_two_replacements : public testing::test_case {
public:
   //! See testing::test_case::title().
   virtual istr title() override {
      return istr(ABC_SL("abc::io::text_writer::print() – two replacements"));
   }

   //! See testing::test_case::run().
   virtual void run() override {
      ABC_TRACE_FUNC(this);

      smstr<128> sWriterBuffer;
      io::text::str_writer stw(&sWriterBuffer);

      // Single string replacement, referenced twice.
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(ABC_SL("{0}{0}"), ABC_SL("a")), stw.get_str()), ABC_SL("aa")
      );
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(ABC_SL("{0}x{0}"), ABC_SL("a")), stw.get_str()), ABC_SL("axa")
      );
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(ABC_SL("x{0}x{0}"), ABC_SL("a")), stw.get_str()), ABC_SL("xaxa")
      );
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(ABC_SL("{0}x{0}x"), ABC_SL("a")), stw.get_str()), ABC_SL("axax")
      );
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(ABC_SL("x{0}x{0}x"), ABC_SL("a")), stw.get_str()), ABC_SL("xaxax")
      );

      // Two string replacements, various ways of reference.
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(ABC_SL("{}{}"), ABC_SL("a"), ABC_SL("b")), stw.get_str()), ABC_SL("ab")
      );
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(ABC_SL("{0}{1}"), ABC_SL("a"), ABC_SL("b")), stw.get_str()), ABC_SL("ab")
      );
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(ABC_SL("{1}{0}"), ABC_SL("a"), ABC_SL("b")), stw.get_str()), ABC_SL("ba")
      );
      stw.clear();
      ABC_TESTING_ASSERT_EQUAL(
         (stw.print(ABC_SL("{1}{1}"), ABC_SL("a"), ABC_SL("b")), stw.get_str()), ABC_SL("bb")
      );
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::text_writer_print_two_replacements)

////////////////////////////////////////////////////////////////////////////////////////////////////

