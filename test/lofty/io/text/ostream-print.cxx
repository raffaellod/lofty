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
#include <lofty/testing/test_case.hxx>
#include <lofty/text.hxx>

#define PRINT_GET(...) (ostream.clear(), ostream.print(__VA_ARGS__), ostream.get_str())

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   io_text_ostream_print_0_replacements,
   "lofty::io::text::ostream::print() – no replacements"
) {
   LOFTY_TRACE_FUNC(this);

   sstr<128> buf;
   io::text::str_ostream ostream(external_buffer, buf.str_ptr());

   // Syntax errors.
   ostream.clear();
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, ostream.print(LOFTY_SL("{")));
   ostream.clear();
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, ostream.print(LOFTY_SL("{{{")));
   ostream.clear();
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, ostream.print(LOFTY_SL("}")));
   ostream.clear();
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, ostream.print(LOFTY_SL("}}}")));

   // No replacements.
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(str::empty), str::empty);
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("x")), LOFTY_SL("x"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("x"), LOFTY_SL("a")), LOFTY_SL("x"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("{{")), LOFTY_SL("{"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("}}")), LOFTY_SL("}"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("{{}}")), LOFTY_SL("{}"));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   io_text_ostream_print_1_replacement,
   "lofty::io::text::ostream::print() – one replacement"
) {
   LOFTY_TRACE_FUNC(this);

   sstr<128> buf;
   io::text::str_ostream ostream(external_buffer, buf.str_ptr());

   // Single string replacement, deduced argument index.
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("{}"), LOFTY_SL("a")), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("x{}"), LOFTY_SL("a")), LOFTY_SL("xa"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("{}x"), LOFTY_SL("a")), LOFTY_SL("ax"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("x{}x"), LOFTY_SL("a")), LOFTY_SL("xax"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("{{{}}}"), LOFTY_SL("a")), LOFTY_SL("{a}"));

   // Single string replacement, explicit index.
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("{0}"), LOFTY_SL("a")), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("x{0}"), LOFTY_SL("a")), LOFTY_SL("xa"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("{0}x"), LOFTY_SL("a")), LOFTY_SL("ax"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("x{0}x"), LOFTY_SL("a")), LOFTY_SL("xax"));

   // Single integer replacement, various ways of reference, various format options.
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("{}"), 34), LOFTY_SL("34"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("{:x}"), 34), LOFTY_SL("22"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("{:#x}"), 34), LOFTY_SL("0x22"));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   io_text_ostream_print_2_replacements,
   "lofty::io::text::ostream::print() – two replacements"
) {
   LOFTY_TRACE_FUNC(this);

   sstr<128> buf;
   io::text::str_ostream ostream(external_buffer, buf.str_ptr());

   // Single string replacement, referenced twice.
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("{0}{0}"), LOFTY_SL("a")), LOFTY_SL("aa"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("{0}x{0}"), LOFTY_SL("a")), LOFTY_SL("axa"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("x{0}x{0}"), LOFTY_SL("a")), LOFTY_SL("xaxa"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("{0}x{0}x"), LOFTY_SL("a")), LOFTY_SL("axax"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("x{0}x{0}x"), LOFTY_SL("a")), LOFTY_SL("xaxax"));

   // Two string replacements, various ways of reference.
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("{}{}"), LOFTY_SL("a"), LOFTY_SL("b")), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("{0}{1}"), LOFTY_SL("a"), LOFTY_SL("b")), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("{1}{0}"), LOFTY_SL("a"), LOFTY_SL("b")), LOFTY_SL("ba"));
   LOFTY_TESTING_ASSERT_EQUAL(PRINT_GET(LOFTY_SL("{1}{1}"), LOFTY_SL("a"), LOFTY_SL("b")), LOFTY_SL("bb"));
}

}} //namespace lofty::test
