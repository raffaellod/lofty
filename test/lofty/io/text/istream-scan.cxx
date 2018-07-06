/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016-2018 Raffaello D. Di Napoli

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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   io_text_istream_scan_0_captures,
   "lofty::io::text::istream::scan() – no captures"
) {
   LOFTY_TRACE_FUNC();

   // Syntax errors.
   ASSERT_THROWS(text::syntax_error, io::text::str_istream(str::empty).scan(LOFTY_SL("+")));
   ASSERT_THROWS(text::syntax_error, io::text::str_istream(str::empty).scan(LOFTY_SL("(")));

   // No captures.
   ASSERT(io::text::str_istream(str::empty).scan(str::empty));
   //? ASSERT_DOES_NOT_THROW(io::text::str_istream(LOFTY_SL("x")).scan(str::empty));
   ASSERT(io::text::str_istream(LOFTY_SL("x")).scan(LOFTY_SL("x")));
   //? ASSERT_DOES_NOT_THROW(io::text::str_istream(LOFTY_SL("xx")).scan(LOFTY_SL("x")));
   ASSERT(io::text::str_istream(LOFTY_SL("x")).scan(LOFTY_SL("x+")));
   ASSERT(io::text::str_istream(LOFTY_SL("x")).scan(LOFTY_SL("^x$")));
   //? ASSERT_DOES_NOT_THROW(io::text::str_istream(LOFTY_SL("xx")).scan(LOFTY_SL("^x$")));
   ASSERT(io::text::str_istream(LOFTY_SL("x")).scan(LOFTY_SL("^x+$")));
   ASSERT(io::text::str_istream(LOFTY_SL("a")).scan(LOFTY_SL("^[a]$")));
   ASSERT(io::text::str_istream(LOFTY_SL("aa")).scan(LOFTY_SL("^[a]+$")));
   ASSERT(io::text::str_istream(LOFTY_SL("ab")).scan(LOFTY_SL("^[ab]+$")));
//   ASSERT(io::text::str_istream(LOFTY_SL("ba")).scan(LOFTY_SL("^[ab]+$")));
   ASSERT(!io::text::str_istream(LOFTY_SL("a")).scan(LOFTY_SL("^[b]$")));
   ASSERT(!io::text::str_istream(LOFTY_SL("ab")).scan(LOFTY_SL("^[b]+$")));
   ASSERT(!io::text::str_istream(LOFTY_SL("ba")).scan(LOFTY_SL("^[b]+$")));
   ASSERT(io::text::str_istream(LOFTY_SL("a")).scan(LOFTY_SL("^[^m]$")));
   ASSERT(io::text::str_istream(LOFTY_SL("ab")).scan(LOFTY_SL("^[^m]+$")));
   ASSERT(!io::text::str_istream(LOFTY_SL("m")).scan(LOFTY_SL("^[^m]$")));
   ASSERT(!io::text::str_istream(LOFTY_SL("lm")).scan(LOFTY_SL("^[^m]+$")));
   ASSERT(!io::text::str_istream(LOFTY_SL("mn")).scan(LOFTY_SL("^[^m]+$")));
   ASSERT(io::text::str_istream(LOFTY_SL("z")).scan(LOFTY_SL("^[^m]$")));
//   ASSERT(io::text::str_istream(LOFTY_SL("yz")).scan(LOFTY_SL("^[^m]+$")));
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
   ASSERT(io::text::str_istream(LOFTY_SL("a")).scan(LOFTY_SL("^()$"), &captured1));
   ASSERT(captured1 == LOFTY_SL("a"));
   ASSERT(io::text::str_istream(LOFTY_SL("xb")).scan(LOFTY_SL("^x()$"), &captured1));
   ASSERT(captured1 == LOFTY_SL("b"));
   ASSERT(io::text::str_istream(LOFTY_SL("cx")).scan(LOFTY_SL("^()x$"), &captured1));
   ASSERT(captured1 == LOFTY_SL("c"));
   ASSERT(io::text::str_istream(LOFTY_SL("xdx")).scan(LOFTY_SL("^x()x$"), &captured1));
   ASSERT(captured1 == LOFTY_SL("d"));
   ASSERT(io::text::str_istream(LOFTY_SL("(e)")).scan(LOFTY_SL("^\\(()\\)$"), &captured1));
   ASSERT(captured1 == LOFTY_SL("e"));
   ASSERT(io::text::str_istream(LOFTY_SL("f")).scan(LOFTY_SL("^(f+)$"), &captured1));
   ASSERT(captured1 == LOFTY_SL("f"));
   ASSERT(io::text::str_istream(LOFTY_SL("g")).scan(LOFTY_SL("^([a-z]+)$"), &captured1));
   ASSERT(captured1 == LOFTY_SL("g"));
   ASSERT(io::text::str_istream(LOFTY_SL("h")).scan(LOFTY_SL("^([^ ]+)$"), &captured1));
   ASSERT(captured1 == LOFTY_SL("h"));

   int captured2;
   ASSERT(io::text::str_istream(LOFTY_SL("31")).scan(LOFTY_SL("^()$"), &captured2));
   ASSERT(captured2 == 31);
   ASSERT(io::text::str_istream(LOFTY_SL("20")).scan(LOFTY_SL("^(x)$"), &captured2));
   ASSERT(captured2 == 32);
   ASSERT(io::text::str_istream(LOFTY_SL("0x21")).scan(LOFTY_SL("^(#)$"), &captured2));
   ASSERT(captured2 == 33);
   ASSERT(io::text::str_istream(LOFTY_SL("0x22")).scan(LOFTY_SL("^(#x)$"), &captured2));
   ASSERT(captured2 == 34);
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
   ASSERT(io::text::str_istream(LOFTY_SL("a b")).scan(LOFTY_SL("^([^ ]+) ([^ ]+)$"), &captured1, &captured2));
   ASSERT(captured1 == LOFTY_SL("a"));
   ASSERT(captured2 == LOFTY_SL("b"));
   ASSERT(io::text::str_istream(LOFTY_SL("cd ef")).scan(LOFTY_SL("^([^ ]+) ([^ ]+)$"), &captured1, &captured2));
   ASSERT(captured1 == LOFTY_SL("cd"));
   ASSERT(captured2 == LOFTY_SL("ef"));

   int captured3, captured4;
   ASSERT(io::text::str_istream(LOFTY_SL("1 2")).scan(LOFTY_SL("^() ()$"), &captured3, &captured4));
   ASSERT(captured3 == 1);
   ASSERT(captured4 == 2);
   ASSERT(io::text::str_istream(LOFTY_SL("34 56")).scan(LOFTY_SL("^() ()$"), &captured3, &captured4));
   ASSERT(captured3 == 34);
   ASSERT(captured4 == 56);
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

   ASSERT(io::text::str_istream(LOFTY_SL("ab")).scan(LOFTY_SL("^(.)(.)$"), &captured1, &captured2));
   ASSERT(captured1 == LOFTY_SL("a"));
   ASSERT(captured2 == LOFTY_SL("b"));
   // Both formats are greedy, but the first one will consume characters first.
   ASSERT(io::text::str_istream(LOFTY_SL("abcd")).scan(LOFTY_SL("^(.+)(.+)$"), &captured1, &captured2));
   ASSERT(captured1 == LOFTY_SL("abc"));
   ASSERT(captured2 == LOFTY_SL("d"));
   ASSERT(io::text::str_istream(LOFTY_SL("axb")).scan(LOFTY_SL("^()x()$"), &captured1, &captured2));
   ASSERT(captured1 == LOFTY_SL("a"));
   ASSERT(captured2 == LOFTY_SL("b"));
}

}} //namespace lofty::test
