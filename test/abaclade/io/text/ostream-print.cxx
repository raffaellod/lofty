/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2016 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#include <abaclade/testing/test_case.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   io_text_ostream_print_0_replacements,
   "abc::io::text::ostream::print() – no replacements"
) {
   ABC_TRACE_FUNC(this);

   sstr<128> sOStreamBuffer;
   io::text::str_ostream sos(external_buffer, sOStreamBuffer.str_ptr());

   // Syntax errors.
   sos.clear();
   ABC_TESTING_ASSERT_THROWS(syntax_error, sos.print(ABC_SL("{")));
   sos.clear();
   ABC_TESTING_ASSERT_THROWS(syntax_error, sos.print(ABC_SL("{{{")));
   sos.clear();
   ABC_TESTING_ASSERT_THROWS(syntax_error, sos.print(ABC_SL("}")));
   sos.clear();
   ABC_TESTING_ASSERT_THROWS(syntax_error, sos.print(ABC_SL("}}}")));

   // No replacements.
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL((sos.print(str::empty), sos.get_str()), str::empty);
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL((sos.print(ABC_SL("x")), sos.get_str()), ABC_SL("x"));
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL((sos.print(ABC_SL("x"), ABC_SL("a")), sos.get_str()), ABC_SL("x"));
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL((sos.print(ABC_SL("{{")), sos.get_str()), ABC_SL("{"));
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL((sos.print(ABC_SL("}}")), sos.get_str()), ABC_SL("}"));
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL((sos.print(ABC_SL("{{}}")), sos.get_str()), ABC_SL("{}"));
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   io_text_ostream_print_1_replacement,
   "abc::io::text::ostream::print() – one replacement"
) {
   ABC_TRACE_FUNC(this);

   sstr<128> sOStreamBuffer;
   io::text::str_ostream sos(external_buffer, sOStreamBuffer.str_ptr());

   // Single string replacement, deduced argument index.
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL(
      (sos.print(ABC_SL("{}"), ABC_SL("a")), sos.get_str()), ABC_SL("a")
   );
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL(
      (sos.print(ABC_SL("x{}"), ABC_SL("a")), sos.get_str()), ABC_SL("xa")
   );
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL(
      (sos.print(ABC_SL("{}x"), ABC_SL("a")), sos.get_str()), ABC_SL("ax")
   );
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL(
      (sos.print(ABC_SL("x{}x"), ABC_SL("a")), sos.get_str()), ABC_SL("xax")
   );
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL(
      (sos.print(ABC_SL("{{{}}}"), ABC_SL("a")), sos.get_str()), ABC_SL("{a}")
   );

   // Single string replacement, explicit index.
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL(
      (sos.print(ABC_SL("{0}"), ABC_SL("a")), sos.get_str()), ABC_SL("a")
   );
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL(
      (sos.print(ABC_SL("x{0}"), ABC_SL("a")), sos.get_str()), ABC_SL("xa")
   );
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL(
      (sos.print(ABC_SL("{0}x"), ABC_SL("a")), sos.get_str()), ABC_SL("ax")
   );
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL(
      (sos.print(ABC_SL("x{0}x"), ABC_SL("a")), sos.get_str()), ABC_SL("xax")
   );

   // Single integer replacement, various ways of reference, various format options.
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL((sos.print(ABC_SL("{}"), 34), sos.get_str()), ABC_SL("34"));
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL((sos.print(ABC_SL("{:x}"), 34), sos.get_str()), ABC_SL("22"));
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL((sos.print(ABC_SL("{:#x}"), 34), sos.get_str()), ABC_SL("0x22"));
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   io_text_ostream_print_2_replacements,
   "abc::io::text::ostream::print() – two replacements"
) {
   ABC_TRACE_FUNC(this);

   sstr<128> sOStreamBuffer;
   io::text::str_ostream sos(external_buffer, sOStreamBuffer.str_ptr());

   // Single string replacement, referenced twice.
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL(
      (sos.print(ABC_SL("{0}{0}"), ABC_SL("a")), sos.get_str()), ABC_SL("aa")
   );
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL(
      (sos.print(ABC_SL("{0}x{0}"), ABC_SL("a")), sos.get_str()), ABC_SL("axa")
   );
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL(
      (sos.print(ABC_SL("x{0}x{0}"), ABC_SL("a")), sos.get_str()), ABC_SL("xaxa")
   );
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL(
      (sos.print(ABC_SL("{0}x{0}x"), ABC_SL("a")), sos.get_str()), ABC_SL("axax")
   );
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL(
      (sos.print(ABC_SL("x{0}x{0}x"), ABC_SL("a")), sos.get_str()), ABC_SL("xaxax")
   );

   // Two string replacements, various ways of reference.
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL(
      (sos.print(ABC_SL("{}{}"), ABC_SL("a"), ABC_SL("b")), sos.get_str()), ABC_SL("ab")
   );
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL(
      (sos.print(ABC_SL("{0}{1}"), ABC_SL("a"), ABC_SL("b")), sos.get_str()), ABC_SL("ab")
   );
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL(
      (sos.print(ABC_SL("{1}{0}"), ABC_SL("a"), ABC_SL("b")), sos.get_str()), ABC_SL("ba")
   );
   sos.clear();
   ABC_TESTING_ASSERT_EQUAL(
      (sos.print(ABC_SL("{1}{1}"), ABC_SL("a"), ABC_SL("b")), sos.get_str()), ABC_SL("bb")
   );
}

}} //namespace abc::test
