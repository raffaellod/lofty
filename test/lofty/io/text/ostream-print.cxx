/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/logging.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/text.hxx>

#define PRINT_GET(...) (ostream.clear(), ostream.print(__VA_ARGS__), ostream.get_str())

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   io_text_ostream_print_0_replacements,
   "lofty::io::text::ostream::print() – no replacements"
) {
   LOFTY_TRACE_FUNC();

   sstr<128> buf;
   io::text::str_ostream ostream(external_buffer, buf.str_ptr());

   // Syntax errors.
   ostream.clear();
   ASSERT_THROWS(text::syntax_error, ostream.print(LOFTY_SL("{")));
   ostream.clear();
   ASSERT_THROWS(text::syntax_error, ostream.print(LOFTY_SL("{{{")));
   ostream.clear();
   ASSERT_THROWS(text::syntax_error, ostream.print(LOFTY_SL("}")));
   ostream.clear();
   ASSERT_THROWS(text::syntax_error, ostream.print(LOFTY_SL("}}}")));

   // No replacements.
   ASSERT(PRINT_GET(str::empty) == str::empty);
   ASSERT(PRINT_GET(LOFTY_SL("x")) == LOFTY_SL("x"));
   ASSERT(PRINT_GET(LOFTY_SL("x"), LOFTY_SL("a")) == LOFTY_SL("x"));
   ASSERT(PRINT_GET(LOFTY_SL("{{")) == LOFTY_SL("{"));
   ASSERT(PRINT_GET(LOFTY_SL("}}")) == LOFTY_SL("}"));
   ASSERT(PRINT_GET(LOFTY_SL("{{}}")) == LOFTY_SL("{}"));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   io_text_ostream_print_1_replacement,
   "lofty::io::text::ostream::print() – one replacement"
) {
   LOFTY_TRACE_FUNC();

   sstr<128> buf;
   io::text::str_ostream ostream(external_buffer, buf.str_ptr());

   // Single string replacement, deduced argument index.
   ASSERT(PRINT_GET(LOFTY_SL("{}"), LOFTY_SL("a")) == LOFTY_SL("a"));
   ASSERT(PRINT_GET(LOFTY_SL("x{}"), LOFTY_SL("a")) == LOFTY_SL("xa"));
   ASSERT(PRINT_GET(LOFTY_SL("{}x"), LOFTY_SL("a")) == LOFTY_SL("ax"));
   ASSERT(PRINT_GET(LOFTY_SL("x{}x"), LOFTY_SL("a")) == LOFTY_SL("xax"));
   ASSERT(PRINT_GET(LOFTY_SL("{{{}}}"), LOFTY_SL("a")) == LOFTY_SL("{a}"));

   // Single string replacement, explicit index.
   ASSERT(PRINT_GET(LOFTY_SL("{0}"), LOFTY_SL("a")) == LOFTY_SL("a"));
   ASSERT(PRINT_GET(LOFTY_SL("x{0}"), LOFTY_SL("a")) == LOFTY_SL("xa"));
   ASSERT(PRINT_GET(LOFTY_SL("{0}x"), LOFTY_SL("a")) == LOFTY_SL("ax"));
   ASSERT(PRINT_GET(LOFTY_SL("x{0}x"), LOFTY_SL("a")) == LOFTY_SL("xax"));

   // Single integer replacement, various ways of reference, various format options.
   ASSERT(PRINT_GET(LOFTY_SL("{}"), 34) == LOFTY_SL("34"));
   ASSERT(PRINT_GET(LOFTY_SL("{:x}"), 34) == LOFTY_SL("22"));
   ASSERT(PRINT_GET(LOFTY_SL("{:#x}"), 34) == LOFTY_SL("0x22"));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   io_text_ostream_print_2_replacements,
   "lofty::io::text::ostream::print() – two replacements"
) {
   LOFTY_TRACE_FUNC();

   sstr<128> buf;
   io::text::str_ostream ostream(external_buffer, buf.str_ptr());

   // Single string replacement, referenced twice.
   ASSERT(PRINT_GET(LOFTY_SL("{0}{0}"), LOFTY_SL("a")) == LOFTY_SL("aa"));
   ASSERT(PRINT_GET(LOFTY_SL("{0}x{0}"), LOFTY_SL("a")) == LOFTY_SL("axa"));
   ASSERT(PRINT_GET(LOFTY_SL("x{0}x{0}"), LOFTY_SL("a")) == LOFTY_SL("xaxa"));
   ASSERT(PRINT_GET(LOFTY_SL("{0}x{0}x"), LOFTY_SL("a")) == LOFTY_SL("axax"));
   ASSERT(PRINT_GET(LOFTY_SL("x{0}x{0}x"), LOFTY_SL("a")) == LOFTY_SL("xaxax"));

   // Two string replacements, various ways of reference.
   ASSERT(PRINT_GET(LOFTY_SL("{}{}"), LOFTY_SL("a"), LOFTY_SL("b")) == LOFTY_SL("ab"));
   ASSERT(PRINT_GET(LOFTY_SL("{0}{1}"), LOFTY_SL("a"), LOFTY_SL("b")) == LOFTY_SL("ab"));
   ASSERT(PRINT_GET(LOFTY_SL("{1}{0}"), LOFTY_SL("a"), LOFTY_SL("b")) == LOFTY_SL("ba"));
   ASSERT(PRINT_GET(LOFTY_SL("{1}{1}"), LOFTY_SL("a"), LOFTY_SL("b")) == LOFTY_SL("bb"));
}

}} //namespace lofty::test
