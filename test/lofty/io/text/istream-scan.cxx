/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/text.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   io_text_istream_scan_0_captures,
   "lofty::io::text::istream::scan() – no captures"
) {
   LOFTY_TRACE_FUNC();

   // Syntax errors.
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, io::text::str_istream(str::empty).scan(LOFTY_SL("+")));
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, io::text::str_istream(str::empty).scan(LOFTY_SL("(")));

   // No captures.
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(str::empty).scan(str::empty));
   //? LOFTY_TESTING_ASSERT_DOES_NOT_THROW(io::text::str_istream(LOFTY_SL("x")).scan(str::empty));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("x")).scan(LOFTY_SL("x")));
   //? LOFTY_TESTING_ASSERT_DOES_NOT_THROW(io::text::str_istream(LOFTY_SL("xx")).scan(LOFTY_SL("x")));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("x")).scan(LOFTY_SL("x+")));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("x")).scan(LOFTY_SL("^x$")));
   //? LOFTY_TESTING_ASSERT_DOES_NOT_THROW(io::text::str_istream(LOFTY_SL("xx")).scan(LOFTY_SL("^x$")));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("x")).scan(LOFTY_SL("^x+$")));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("a")).scan(LOFTY_SL("^[a]$")));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("aa")).scan(LOFTY_SL("^[a]+$")));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("ab")).scan(LOFTY_SL("^[ab]+$")));
//   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("ba")).scan(LOFTY_SL("^[ab]+$")));
   LOFTY_TESTING_ASSERT_FALSE(io::text::str_istream(LOFTY_SL("a")).scan(LOFTY_SL("^[b]$")));
   LOFTY_TESTING_ASSERT_FALSE(io::text::str_istream(LOFTY_SL("ab")).scan(LOFTY_SL("^[b]+$")));
   LOFTY_TESTING_ASSERT_FALSE(io::text::str_istream(LOFTY_SL("ba")).scan(LOFTY_SL("^[b]+$")));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("a")).scan(LOFTY_SL("^[^m]$")));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("ab")).scan(LOFTY_SL("^[^m]+$")));
   LOFTY_TESTING_ASSERT_FALSE(io::text::str_istream(LOFTY_SL("m")).scan(LOFTY_SL("^[^m]$")));
   LOFTY_TESTING_ASSERT_FALSE(io::text::str_istream(LOFTY_SL("lm")).scan(LOFTY_SL("^[^m]+$")));
   LOFTY_TESTING_ASSERT_FALSE(io::text::str_istream(LOFTY_SL("mn")).scan(LOFTY_SL("^[^m]+$")));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("z")).scan(LOFTY_SL("^[^m]$")));
//   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("yz")).scan(LOFTY_SL("^[^m]+$")));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   io_text_istream_scan_1_capture,
   "lofty::io::text::istream::scan() – one capture"
) {
   LOFTY_TRACE_FUNC();

   str captured1;
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("a")).scan(LOFTY_SL("^()$"), &captured1));
   LOFTY_TESTING_ASSERT_EQUAL(captured1, LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("xb")).scan(LOFTY_SL("^x()$"), &captured1));
   LOFTY_TESTING_ASSERT_EQUAL(captured1, LOFTY_SL("b"));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("cx")).scan(LOFTY_SL("^()x$"), &captured1));
   LOFTY_TESTING_ASSERT_EQUAL(captured1, LOFTY_SL("c"));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("xdx")).scan(LOFTY_SL("^x()x$"), &captured1));
   LOFTY_TESTING_ASSERT_EQUAL(captured1, LOFTY_SL("d"));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("(e)")).scan(LOFTY_SL("^\\(()\\)$"), &captured1));
   LOFTY_TESTING_ASSERT_EQUAL(captured1, LOFTY_SL("e"));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("f")).scan(LOFTY_SL("^(f+)$"), &captured1));
   LOFTY_TESTING_ASSERT_EQUAL(captured1, LOFTY_SL("f"));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("g")).scan(LOFTY_SL("^([a-z]+)$"), &captured1));
   LOFTY_TESTING_ASSERT_EQUAL(captured1, LOFTY_SL("g"));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("h")).scan(LOFTY_SL("^([^ ]+)$"), &captured1));
   LOFTY_TESTING_ASSERT_EQUAL(captured1, LOFTY_SL("h"));

   int captured2;
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("31")).scan(LOFTY_SL("^()$"), &captured2));
   LOFTY_TESTING_ASSERT_EQUAL(captured2, 31);
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("20")).scan(LOFTY_SL("^(x)$"), &captured2));
   LOFTY_TESTING_ASSERT_EQUAL(captured2, 32);
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("0x21")).scan(LOFTY_SL("^(#)$"), &captured2));
   LOFTY_TESTING_ASSERT_EQUAL(captured2, 33);
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("0x22")).scan(LOFTY_SL("^(#x)$"), &captured2));
   LOFTY_TESTING_ASSERT_EQUAL(captured2, 34);
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   io_text_istream_scan_2_captures,
   "lofty::io::text::istream::scan() – two captures"
) {
   LOFTY_TRACE_FUNC();

   str captured1, captured2;
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("a b")).scan(LOFTY_SL("^([^ ]+) ([^ ]+)$"), &captured1, &captured2));
   LOFTY_TESTING_ASSERT_EQUAL(captured1, LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(captured2, LOFTY_SL("b"));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("cd ef")).scan(LOFTY_SL("^([^ ]+) ([^ ]+)$"), &captured1, &captured2));
   LOFTY_TESTING_ASSERT_EQUAL(captured1, LOFTY_SL("cd"));
   LOFTY_TESTING_ASSERT_EQUAL(captured2, LOFTY_SL("ef"));

   int captured3, captured4;
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("1 2")).scan(LOFTY_SL("^() ()$"), &captured3, &captured4));
   LOFTY_TESTING_ASSERT_EQUAL(captured3, 1);
   LOFTY_TESTING_ASSERT_EQUAL(captured4, 2);
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("34 56")).scan(LOFTY_SL("^() ()$"), &captured3, &captured4));
   LOFTY_TESTING_ASSERT_EQUAL(captured3, 34);
   LOFTY_TESTING_ASSERT_EQUAL(captured4, 56);
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   io_text_istream_scan_competing_str_with_format,
   "lofty::io::text::istream::scan() – competing string captures with format"
) {
   LOFTY_TRACE_FUNC();

   str captured1, captured2;

   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("ab")).scan(LOFTY_SL("^(.)(.)$"), &captured1, &captured2));
   LOFTY_TESTING_ASSERT_EQUAL(captured1, LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(captured2, LOFTY_SL("b"));
   // Both formats are greedy, but the first one will consume characters first.
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("abcd")).scan(LOFTY_SL("^(.+)(.+)$"), &captured1, &captured2));
   LOFTY_TESTING_ASSERT_EQUAL(captured1, LOFTY_SL("abc"));
   LOFTY_TESTING_ASSERT_EQUAL(captured2, LOFTY_SL("d"));
   LOFTY_TESTING_ASSERT_TRUE(io::text::str_istream(LOFTY_SL("axb")).scan(LOFTY_SL("^()x()$"), &captured1, &captured2));
   LOFTY_TESTING_ASSERT_EQUAL(captured1, LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(captured2, LOFTY_SL("b"));
}

}} //namespace lofty::test
