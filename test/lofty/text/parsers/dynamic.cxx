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
#include <lofty/collections.hxx>
#include <lofty/logging.hxx>
#include <lofty/text/parsers/dynamic.hxx>
#include <lofty/testing/test_case.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_empty,
   "lofty::text::parsers::dynamic – pattern “” (empty)"
) {
   LOFTY_TRACE_FUNC();

   text::parsers::dynamic parser;

   text::parsers::dynamic::match match;
   ASSERT(!!(match = parser.run(LOFTY_SL(""))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 0u);
   ASSERT(match.str() == LOFTY_SL(""));
   ASSERT(!!(match = parser.run(LOFTY_SL("a"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 0u);
   ASSERT(match.str() == LOFTY_SL(""));
   ASSERT(!!(match = parser.run(LOFTY_SL("aa"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 0u);
   ASSERT(match.str() == LOFTY_SL(""));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_a,
   "lofty::text::parsers::dynamic – pattern “a”"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(a_state, nullptr, nullptr, 'a');
   text::parsers::dynamic parser;
   parser.set_initial_state(&a_state.base);

   text::parsers::dynamic::match match;
   ASSERT(!parser.run(LOFTY_SL("")));
   ASSERT(!!(match = parser.run(LOFTY_SL("a"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(!!(match = parser.run(LOFTY_SL("aa"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(!parser.run(LOFTY_SL("b")));
   ASSERT(!!(match = parser.run(LOFTY_SL("ba"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(!!(match = parser.run(LOFTY_SL("ab"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_caret,
   "lofty::text::parsers::dynamic – pattern “^”"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TEXT_PARSERS_DYNAMIC_BEGIN_STATE(begin_state, nullptr, nullptr);
   text::parsers::dynamic parser;
   parser.set_initial_state(&begin_state.base);

   text::parsers::dynamic::match match;
   ASSERT(!!(match = parser.run(LOFTY_SL(""))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 0u);
   ASSERT(match.str() == LOFTY_SL(""));
   ASSERT(!!(match = parser.run(LOFTY_SL("a"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 0u);
   ASSERT(match.str() == LOFTY_SL(""));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_caret_a,
   "lofty::text::parsers::dynamic – pattern “^a”"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(a_state, nullptr, nullptr, 'a');
   LOFTY_TEXT_PARSERS_DYNAMIC_BEGIN_STATE(begin_state, &a_state.base, nullptr);
   text::parsers::dynamic parser;
   parser.set_initial_state(&begin_state.base);

   text::parsers::dynamic::match match;
   ASSERT(!parser.run(LOFTY_SL("")));
   ASSERT(!!(match = parser.run(LOFTY_SL("a"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(!!(match = parser.run(LOFTY_SL("aa"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(!!(match = parser.run(LOFTY_SL("ab"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(!parser.run(LOFTY_SL("b")));
   ASSERT(!parser.run(LOFTY_SL("ba")));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_dollar,
   "lofty::text::parsers::dynamic – pattern “$”"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TEXT_PARSERS_DYNAMIC_END_STATE(end_state, nullptr, nullptr);
   text::parsers::dynamic parser;
   parser.set_initial_state(&end_state.base);

   text::parsers::dynamic::match match;
   ASSERT(!!(match = parser.run(LOFTY_SL(""))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 0u);
   ASSERT(match.str() == LOFTY_SL(""));
   ASSERT(!!(match = parser.run(LOFTY_SL("a"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL(""));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_a_dollar,
   "lofty::text::parsers::dynamic – pattern “a$”"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TEXT_PARSERS_DYNAMIC_END_STATE(end_state, nullptr, nullptr);
   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(a_state, &end_state.base, nullptr, 'a');
   text::parsers::dynamic parser;
   parser.set_initial_state(&a_state.base);

   text::parsers::dynamic::match match;
   ASSERT(!parser.run(LOFTY_SL("")));
   ASSERT(!!(match = parser.run(LOFTY_SL("a"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(!!(match = parser.run(LOFTY_SL("aa"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(!parser.run(LOFTY_SL("ab")));
   ASSERT(!parser.run(LOFTY_SL("b")));
   ASSERT(!!(match = parser.run(LOFTY_SL("ba"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("a"));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_ab,
   "lofty::text::parsers::dynamic – pattern “ab”"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(b_state, nullptr, nullptr, 'b');
   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(a_state, &b_state.base, nullptr, 'a');
   text::parsers::dynamic parser;
   parser.set_initial_state(&a_state.base);

   text::parsers::dynamic::match match;
   ASSERT(!parser.run(LOFTY_SL("")));
   ASSERT(!parser.run(LOFTY_SL("a")));
   ASSERT(!parser.run(LOFTY_SL("aa")));
   ASSERT(!parser.run(LOFTY_SL("b")));
   ASSERT(!!(match = parser.run(LOFTY_SL("ab"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(!!(match = parser.run(LOFTY_SL("bab"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(!!(match = parser.run(LOFTY_SL("aab"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(!!(match = parser.run(LOFTY_SL("aaba"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(!!(match = parser.run(LOFTY_SL("aabab"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("ab"));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_abc,
   "lofty::text::parsers::dynamic – pattern “abc”"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TEXT_PARSERS_DYNAMIC_STRING_STATE(abc_state, nullptr, nullptr, LOFTY_SL("abc"));
   text::parsers::dynamic parser;
   parser.set_initial_state(&abc_state.base);

   text::parsers::dynamic::match match;
   ASSERT(!parser.run(LOFTY_SL("")));
   ASSERT(!parser.run(LOFTY_SL("a")));
   ASSERT(!parser.run(LOFTY_SL("aa")));
   ASSERT(!parser.run(LOFTY_SL("b")));
   ASSERT(!parser.run(LOFTY_SL("ab")));
   ASSERT(!!(match = parser.run(LOFTY_SL("abc"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("abc"));
   ASSERT(!parser.run(LOFTY_SL("aab")));
   ASSERT(!!(match = parser.run(LOFTY_SL("aabc"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 4u);
   ASSERT(match.str() == LOFTY_SL("abc"));
   ASSERT(!!(match = parser.run(LOFTY_SL("babc"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 4u);
   ASSERT(match.str() == LOFTY_SL("abc"));
   ASSERT(!parser.run(LOFTY_SL("aaba")));
   ASSERT(!!(match = parser.run(LOFTY_SL("aabca"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 4u);
   ASSERT(match.str() == LOFTY_SL("abc"));
   ASSERT(!parser.run(LOFTY_SL("aabab")));
   ASSERT(!!(match = parser.run(LOFTY_SL("aababc"))));
   ASSERT(match.begin_char_index() == 3u);
   ASSERT(match.end_char_index()   == 6u);
   ASSERT(match.str() == LOFTY_SL("abc"));
   ASSERT(!!(match = parser.run(LOFTY_SL("aabcabc"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 4u);
   ASSERT(match.str() == LOFTY_SL("abc"));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_a_qmark,
   "lofty::text::parsers::dynamic – pattern “a?”"
) {
   LOFTY_TRACE_FUNC();

   text::parsers::dynamic parser;
   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(a_state, nullptr, nullptr, 'a');
   LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_GROUP(a_rep_group, nullptr, nullptr, &a_state.base, 0, 1);
   parser.set_initial_state(&a_rep_group.base);

   text::parsers::dynamic::match match;
   ASSERT(!!(match = parser.run(LOFTY_SL(""))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 0u);
   ASSERT(match.str() == LOFTY_SL(""));
   ASSERT(match.repetition_group(0).size() == 0u);
   ASSERT(!!(match = parser.run(LOFTY_SL("a"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(!!(match = parser.run(LOFTY_SL("aa"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(!!(match = parser.run(LOFTY_SL("b"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 0u);
   ASSERT(match.str() == LOFTY_SL(""));
   ASSERT(match.repetition_group(0).size() == 0u);
   ASSERT(!!(match = parser.run(LOFTY_SL("ba"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 0u);
   ASSERT(match.str() == LOFTY_SL(""));
   ASSERT(match.repetition_group(0).size() == 0u);
   ASSERT(!!(match = parser.run(LOFTY_SL("ab"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0).size() == 1u);
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_a_plus,
   "lofty::text::parsers::dynamic – pattern “a+”"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(a_state, nullptr, nullptr, 'a');
   LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(a_rep_group, nullptr, nullptr, &a_state.base, 1);
   text::parsers::dynamic parser;
   parser.set_initial_state(&a_rep_group.base);

   text::parsers::dynamic::match match;
   ASSERT(!parser.run(LOFTY_SL("")));
   ASSERT(!!(match = parser.run(LOFTY_SL("a"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(!!(match = parser.run(LOFTY_SL("aa"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("aa"));
   ASSERT(match.repetition_group(0).size() == 2u);
   ASSERT(!parser.run(LOFTY_SL("b")));
   ASSERT(!!(match = parser.run(LOFTY_SL("ba"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(!!(match = parser.run(LOFTY_SL("ab"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0).size() == 1u);
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_backtracking_greedy_a_star_a,
   "lofty::text::parsers::dynamic – pattern “a*a”"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(a_state_2, nullptr, nullptr, 'a');
   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(a_state_1, nullptr, nullptr, 'a');
   LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(a_rep_group, &a_state_2.base, nullptr, &a_state_1.base, 0);
   text::parsers::dynamic parser;
   parser.set_initial_state(&a_rep_group.base);

   text::parsers::dynamic::match match;
   ASSERT(!parser.run(LOFTY_SL("")));
   ASSERT(!!(match = parser.run(LOFTY_SL("a"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0).size() == 0u);
   ASSERT(!!(match = parser.run(LOFTY_SL("aa"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("aa"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(!!(match = parser.run(LOFTY_SL("aaa"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("aaa"));
   ASSERT(match.repetition_group(0).size() == 2u);
   ASSERT(!parser.run(LOFTY_SL("b")));
   ASSERT(!!(match = parser.run(LOFTY_SL("ba"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0).size() == 0u);
   ASSERT(!!(match = parser.run(LOFTY_SL("baa"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("aa"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(!parser.run(LOFTY_SL("bb")));
   ASSERT(!!(match = parser.run(LOFTY_SL("ab"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0).size() == 0u);
   ASSERT(!!(match = parser.run(LOFTY_SL("aba"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0).size() == 0u);
   ASSERT(!!(match = parser.run(LOFTY_SL("aaba"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("aa"));
   ASSERT(match.repetition_group(0).size() == 1u);
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_a_plus_b_plus,
   "lofty::text::parsers::dynamic – pattern “a+b+”"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(b_state, nullptr, nullptr, 'b');
   LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(b_rep_group, nullptr, nullptr, &b_state.base, 1);
   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(a_state, nullptr, nullptr, 'a');
   LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(a_rep_group, &b_rep_group.base, nullptr, &a_state.base, 1);
   text::parsers::dynamic parser;
   parser.set_initial_state(&a_rep_group.base);

   text::parsers::dynamic::match match;
   ASSERT(!parser.run(LOFTY_SL("")));
   ASSERT(!parser.run(LOFTY_SL("a")));
   ASSERT(!parser.run(LOFTY_SL("aa")));
   ASSERT(!!(match = parser.run(LOFTY_SL("ab"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(match.repetition_group(1).size() == 1u);
   ASSERT(!!(match = parser.run(LOFTY_SL("aba"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(match.repetition_group(1).size() == 1u);
   ASSERT(!!(match = parser.run(LOFTY_SL("abb"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("abb"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(match.repetition_group(1).size() == 2u);
   ASSERT(!!(match = parser.run(LOFTY_SL("abab"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(match.repetition_group(1).size() == 1u);
   ASSERT(!parser.run(LOFTY_SL("b")));
   ASSERT(!parser.run(LOFTY_SL("ba")));
   ASSERT(!!(match = parser.run(LOFTY_SL("bab"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(match.repetition_group(1).size() == 1u);
   ASSERT(!!(match = parser.run(LOFTY_SL("baba"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(match.repetition_group(1).size() == 1u);
   ASSERT(!!(match = parser.run(LOFTY_SL("babb"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 4u);
   ASSERT(match.str() == LOFTY_SL("abb"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(match.repetition_group(1).size() == 2u);
   ASSERT(!!(match = parser.run(LOFTY_SL("babab"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(match.repetition_group(1).size() == 1u);
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_caret_a_plus_b_plus_dollar,
   "lofty::text::parsers::dynamic – pattern “^a+b+$”"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TEXT_PARSERS_DYNAMIC_END_STATE(end_state, nullptr, nullptr);
   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(b_state, nullptr, nullptr, 'b');
   LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(b_rep_group, &end_state.base, nullptr, &b_state.base, 1);
   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(a_state, nullptr, nullptr, 'a');
   LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(a_rep_group, &b_rep_group.base, nullptr, &a_state.base, 1);
   LOFTY_TEXT_PARSERS_DYNAMIC_BEGIN_STATE(begin_state, &a_rep_group.base, nullptr);
   text::parsers::dynamic parser;
   parser.set_initial_state(&begin_state.base);

   text::parsers::dynamic::match match;
   ASSERT(!parser.run(LOFTY_SL("")));
   ASSERT(!parser.run(LOFTY_SL("a")));
   ASSERT(!parser.run(LOFTY_SL("aa")));
   ASSERT(!!(match = parser.run(LOFTY_SL("aab"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("aab"));
   ASSERT(match.repetition_group(0).size() == 2u);
   ASSERT(match.repetition_group(1).size() == 1u);
   ASSERT(!!(match = parser.run(LOFTY_SL("aabb"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 4u);
   ASSERT(match.str() == LOFTY_SL("aabb"));
   ASSERT(match.repetition_group(0).size() == 2u);
   ASSERT(match.repetition_group(1).size() == 2u);
   ASSERT(!parser.run(LOFTY_SL("aabba")));
   ASSERT(!!(match = parser.run(LOFTY_SL("ab"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(match.repetition_group(1).size() == 1u);
   ASSERT(!parser.run(LOFTY_SL("aba")));
   ASSERT(!!(match = parser.run(LOFTY_SL("abb"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("abb"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(match.repetition_group(1).size() == 2u);
   ASSERT(!parser.run(LOFTY_SL("abab")));
   ASSERT(!parser.run(LOFTY_SL("b")));
   ASSERT(!parser.run(LOFTY_SL("ba")));
   ASSERT(!parser.run(LOFTY_SL("bab")));
   ASSERT(!parser.run(LOFTY_SL("baba")));
   ASSERT(!parser.run(LOFTY_SL("babb")));
   ASSERT(!parser.run(LOFTY_SL("babab")));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_abc_or_def,
   "lofty::text::parsers::dynamic – pattern “abc|def”"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TEXT_PARSERS_DYNAMIC_STRING_STATE(def_state, nullptr, nullptr, LOFTY_SL("def"));
   LOFTY_TEXT_PARSERS_DYNAMIC_STRING_STATE(abc_state, nullptr, &def_state.base, LOFTY_SL("abc"));
   text::parsers::dynamic parser;
   parser.set_initial_state(&abc_state.base);

   text::parsers::dynamic::match match;
   ASSERT(!parser.run(LOFTY_SL("")));
   ASSERT(!parser.run(LOFTY_SL("a")));
   ASSERT(!parser.run(LOFTY_SL("b")));
   ASSERT(!parser.run(LOFTY_SL("c")));
   ASSERT(!parser.run(LOFTY_SL("d")));
   ASSERT(!parser.run(LOFTY_SL("e")));
   ASSERT(!parser.run(LOFTY_SL("f")));
   ASSERT(!parser.run(LOFTY_SL("ab")));
   ASSERT(!parser.run(LOFTY_SL("bc")));
   ASSERT(!parser.run(LOFTY_SL("cd")));
   ASSERT(!parser.run(LOFTY_SL("de")));
   ASSERT(!parser.run(LOFTY_SL("ef")));
   ASSERT(!parser.run(LOFTY_SL("abd")));
   ASSERT(!parser.run(LOFTY_SL("bcd")));
   ASSERT(!parser.run(LOFTY_SL("cde")));
   ASSERT(!parser.run(LOFTY_SL("dea")));
   ASSERT(!parser.run(LOFTY_SL("eab")));
   ASSERT(!!(match = parser.run(LOFTY_SL("abc"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("abc"));
   ASSERT(!!(match = parser.run(LOFTY_SL("abcd"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("abc"));
   ASSERT(!!(match = parser.run(LOFTY_SL("fabc"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 4u);
   ASSERT(match.str() == LOFTY_SL("abc"));
   ASSERT(!!(match = parser.run(LOFTY_SL("fabcd"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 4u);
   ASSERT(match.str() == LOFTY_SL("abc"));
   ASSERT(!!(match = parser.run(LOFTY_SL("def"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("def"));
   ASSERT(!!(match = parser.run(LOFTY_SL("defa"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("def"));
   ASSERT(!!(match = parser.run(LOFTY_SL("cdef"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 4u);
   ASSERT(match.str() == LOFTY_SL("def"));
   ASSERT(!!(match = parser.run(LOFTY_SL("cdefa"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 4u);
   ASSERT(match.str() == LOFTY_SL("def"));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_a_or_b_plus,
   "lofty::text::parsers::dynamic – pattern “(?:a|b)+”"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(b_state, nullptr, nullptr, 'b');
   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(a_state, nullptr, &b_state.base, 'a');
   LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(a_or_b_rep_group, nullptr, nullptr, &a_state.base, 1);
   text::parsers::dynamic parser;
   parser.set_initial_state(&a_or_b_rep_group.base);

   text::parsers::dynamic::match match;
   ASSERT(!parser.run(LOFTY_SL("")));
   ASSERT(!!(match = parser.run(LOFTY_SL("a"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(!!(match = parser.run(LOFTY_SL("aa"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("aa"));
   ASSERT(match.repetition_group(0).size() == 2u);
   ASSERT(!!(match = parser.run(LOFTY_SL("ab"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(match.repetition_group(0).size() == 2u);
   ASSERT(!!(match = parser.run(LOFTY_SL("abc"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(match.repetition_group(0).size() == 2u);
   ASSERT(!!(match = parser.run(LOFTY_SL("b"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("b"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(!!(match = parser.run(LOFTY_SL("bb"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("bb"));
   ASSERT(match.repetition_group(0).size() == 2u);
   ASSERT(!!(match = parser.run(LOFTY_SL("ba"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("ba"));
   ASSERT(match.repetition_group(0).size() == 2u);
   ASSERT(!!(match = parser.run(LOFTY_SL("bac"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("ba"));
   ASSERT(match.repetition_group(0).size() == 2u);
   ASSERT(!parser.run(LOFTY_SL("c")));
   ASSERT(!!(match = parser.run(LOFTY_SL("ca"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(!!(match = parser.run(LOFTY_SL("cab"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(match.repetition_group(0).size() == 2u);
   ASSERT(!parser.run(LOFTY_SL("cc")));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_caret_a_or_b_plus_dollar,
   "lofty::text::parsers::dynamic – pattern “^(?:a|b)+$”"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TEXT_PARSERS_DYNAMIC_END_STATE(end_state, nullptr, nullptr);
   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(b_state, nullptr, nullptr, 'b');
   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(a_state, nullptr, &b_state.base, 'a');
   LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(
      a_or_b_rep_group, &end_state.base, nullptr, &a_state.base, 1
   );
   LOFTY_TEXT_PARSERS_DYNAMIC_BEGIN_STATE(begin_state, &a_or_b_rep_group.base, nullptr);
   text::parsers::dynamic parser;
   parser.set_initial_state(&begin_state.base);

   text::parsers::dynamic::match match;
   ASSERT(!parser.run(LOFTY_SL("")));
   ASSERT(!!(match = parser.run(LOFTY_SL("a"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(!!(match = parser.run(LOFTY_SL("aa"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("aa"));
   ASSERT(match.repetition_group(0).size() == 2u);
   ASSERT(!!(match = parser.run(LOFTY_SL("ab"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(match.repetition_group(0).size() == 2u);
   ASSERT(!parser.run(LOFTY_SL("abc")));
   ASSERT(!!(match = parser.run(LOFTY_SL("b"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("b"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(!!(match = parser.run(LOFTY_SL("bb"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("bb"));
   ASSERT(match.repetition_group(0).size() == 2u);
   ASSERT(!!(match = parser.run(LOFTY_SL("ba"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("ba"));
   ASSERT(match.repetition_group(0).size() == 2u);
   ASSERT(!parser.run(LOFTY_SL("bac")));
   ASSERT(!parser.run(LOFTY_SL("c")));
   ASSERT(!parser.run(LOFTY_SL("ca")));
   ASSERT(!parser.run(LOFTY_SL("cab")));
   ASSERT(!parser.run(LOFTY_SL("cc")));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_capture_a_capture_b_plus,
   "lofty::text::parsers::dynamic – pattern “(?:(a)(b))+”"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(b_state, nullptr, nullptr, 'b');
   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(a_state, nullptr, nullptr, 'a');
   LOFTY_TEXT_PARSERS_DYNAMIC_CAPTURE_GROUP(b_cap_group, nullptr, nullptr, &b_state.base);
   LOFTY_TEXT_PARSERS_DYNAMIC_CAPTURE_GROUP(a_cap_group, &b_cap_group.base, nullptr, &a_state.base);
   LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(a_b_rep_group, nullptr, nullptr, &a_cap_group.base, 1);
   text::parsers::dynamic parser;
   parser.set_initial_state(&a_b_rep_group.base);

   text::parsers::dynamic::match match;
   ASSERT(!parser.run(LOFTY_SL("")));
   ASSERT(!parser.run(LOFTY_SL("a")));
   ASSERT(!parser.run(LOFTY_SL("aa")));
   ASSERT(!!(match = parser.run(LOFTY_SL("ab"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(match.repetition_group(0)[0].capture_group(0).begin_char_index() == 0u);
   ASSERT(match.repetition_group(0)[0].capture_group(0).end_char_index()   == 1u);
   ASSERT(match.repetition_group(0)[0].capture_group(0).str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0)[0].capture_group(1).begin_char_index() == 1u);
   ASSERT(match.repetition_group(0)[0].capture_group(1).end_char_index()   == 2u);
   ASSERT(match.repetition_group(0)[0].capture_group(1).str() == LOFTY_SL("b"));
   ASSERT(!!(match = parser.run(LOFTY_SL("abc"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(match.repetition_group(0)[0].capture_group(0).begin_char_index() == 0u);
   ASSERT(match.repetition_group(0)[0].capture_group(0).end_char_index()   == 1u);
   ASSERT(match.repetition_group(0)[0].capture_group(0).str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0)[0].capture_group(1).begin_char_index() == 1u);
   ASSERT(match.repetition_group(0)[0].capture_group(1).end_char_index()   == 2u);
   ASSERT(match.repetition_group(0)[0].capture_group(1).str() == LOFTY_SL("b"));
   ASSERT(!parser.run(LOFTY_SL("b")));
   ASSERT(!parser.run(LOFTY_SL("bb")));
   ASSERT(!parser.run(LOFTY_SL("ba")));
   ASSERT(!parser.run(LOFTY_SL("bac")));
   ASSERT(!parser.run(LOFTY_SL("c")));
   ASSERT(!parser.run(LOFTY_SL("ca")));
   ASSERT(!!(match = parser.run(LOFTY_SL("cab"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(match.repetition_group(0)[0].capture_group(0).begin_char_index() == 1u);
   ASSERT(match.repetition_group(0)[0].capture_group(0).end_char_index()   == 2u);
   ASSERT(match.repetition_group(0)[0].capture_group(0).str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0)[0].capture_group(1).begin_char_index() == 2u);
   ASSERT(match.repetition_group(0)[0].capture_group(1).end_char_index()   == 3u);
   ASSERT(match.repetition_group(0)[0].capture_group(1).str() == LOFTY_SL("b"));
   ASSERT(!parser.run(LOFTY_SL("cc")));
   ASSERT(!!(match = parser.run(LOFTY_SL("aab"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 3u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(match.repetition_group(0)[0].capture_group(0).begin_char_index() == 1u);
   ASSERT(match.repetition_group(0)[0].capture_group(0).end_char_index()   == 2u);
   ASSERT(match.repetition_group(0)[0].capture_group(0).str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0)[0].capture_group(1).begin_char_index() == 2u);
   ASSERT(match.repetition_group(0)[0].capture_group(1).end_char_index()   == 3u);
   ASSERT(match.repetition_group(0)[0].capture_group(1).str() == LOFTY_SL("b"));
   ASSERT(!!(match = parser.run(LOFTY_SL("abb"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(match.repetition_group(0).size() == 1u);
   ASSERT(match.repetition_group(0)[0].capture_group(0).begin_char_index() == 0u);
   ASSERT(match.repetition_group(0)[0].capture_group(0).end_char_index()   == 1u);
   ASSERT(match.repetition_group(0)[0].capture_group(0).str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0)[0].capture_group(1).begin_char_index() == 1u);
   ASSERT(match.repetition_group(0)[0].capture_group(1).end_char_index()   == 2u);
   ASSERT(match.repetition_group(0)[0].capture_group(1).str() == LOFTY_SL("b"));
   ASSERT(!!(match = parser.run(LOFTY_SL("abab"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 4u);
   ASSERT(match.str() == LOFTY_SL("abab"));
   ASSERT(match.repetition_group(0).size() == 2u);
   ASSERT(match.repetition_group(0)[0].capture_group(0).begin_char_index() == 0u);
   ASSERT(match.repetition_group(0)[0].capture_group(0).end_char_index()   == 1u);
   ASSERT(match.repetition_group(0)[0].capture_group(0).str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0)[0].capture_group(1).begin_char_index() == 1u);
   ASSERT(match.repetition_group(0)[0].capture_group(1).end_char_index()   == 2u);
   ASSERT(match.repetition_group(0)[0].capture_group(1).str() == LOFTY_SL("b"));
   ASSERT(match.repetition_group(0)[1].capture_group(0).begin_char_index() == 2u);
   ASSERT(match.repetition_group(0)[1].capture_group(0).end_char_index()   == 3u);
   ASSERT(match.repetition_group(0)[1].capture_group(0).str() == LOFTY_SL("a"));
   ASSERT(match.repetition_group(0)[1].capture_group(1).begin_char_index() == 3u);
   ASSERT(match.repetition_group(0)[1].capture_group(1).end_char_index()   == 4u);
   ASSERT(match.repetition_group(0)[1].capture_group(1).str() == LOFTY_SL("b"));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_capture_a_or_capture_b,
   "lofty::text::parsers::dynamic – pattern “(a)|(b)”"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(b_state, nullptr, nullptr, 'b');
   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(a_state, nullptr, nullptr, 'a');
   LOFTY_TEXT_PARSERS_DYNAMIC_CAPTURE_GROUP(b_cap_group, nullptr, nullptr, &b_state.base);
   LOFTY_TEXT_PARSERS_DYNAMIC_CAPTURE_GROUP(a_cap_group, nullptr, &b_cap_group.base, &a_state.base);
   text::parsers::dynamic parser;
   parser.set_initial_state(&a_cap_group.base);

   text::parsers::dynamic::match match;
   ASSERT(!parser.run(LOFTY_SL("")));
   ASSERT(!!(match = parser.run(LOFTY_SL("a"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(match.capture_group(0).begin_char_index() == 0u);
   ASSERT(match.capture_group(0).end_char_index()   == 1u);
   ASSERT(match.capture_group(0).str() == LOFTY_SL("a"));
   ASSERT(!!(match = parser.run(LOFTY_SL("b"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("b"));
   ASSERT(match.capture_group(0).begin_char_index() == 0u);
   ASSERT(match.capture_group(0).end_char_index()   == 1u);
   ASSERT(match.capture_group(0).str() == LOFTY_SL("b"));
   ASSERT(!!(match = parser.run(LOFTY_SL("ab"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(match.capture_group(0).begin_char_index() == 0u);
   ASSERT(match.capture_group(0).end_char_index()   == 1u);
   ASSERT(match.capture_group(0).str() == LOFTY_SL("a"));
   ASSERT(!!(match = parser.run(LOFTY_SL("ba"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("b"));
   ASSERT(match.capture_group(0).begin_char_index() == 0u);
   ASSERT(match.capture_group(0).end_char_index()   == 1u);
   ASSERT(match.capture_group(0).str() == LOFTY_SL("b"));
   ASSERT(!parser.run(LOFTY_SL("c")));
   ASSERT(!!(match = parser.run(LOFTY_SL("ca"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT(match.capture_group(0).begin_char_index() == 1u);
   ASSERT(match.capture_group(0).end_char_index()   == 2u);
   ASSERT(match.capture_group(0).str() == LOFTY_SL("a"));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_capture_ab_or_capture_ac,
   "lofty::text::parsers::dynamic – pattern “(a+b)|(a+c)”"
) {
   LOFTY_TRACE_FUNC();

   text::parsers::dynamic parser;
   auto a_state = parser.create_code_point_state('a');
   auto a_alt_rep_group = parser.create_repetition_group(a_state, 1, 4);
   auto b_state = parser.create_code_point_state('b');
   auto a_rep_cap_group = parser.create_capture_group(a_alt_rep_group);
   a_rep_cap_group->set_next(b_state);

   auto a_rep_group = parser.create_repetition_group(a_state, 1, 5);
   auto c_state = parser.create_code_point_state('c');
   auto c_rep_group = parser.create_repetition_group(c_state, 0, 1);
   a_rep_group->set_next(c_rep_group);
   a_rep_group->set_alternative(a_rep_cap_group);

   auto all_cap_group = parser.create_capture_group(a_rep_group);
   all_cap_group->set_next(parser.create_end_state());
   auto begin_state = parser.create_begin_state()->set_next(all_cap_group);
   parser.set_initial_state(begin_state);

   text::parsers::dynamic::match match;
   ASSERT(!!(match = parser.run(LOFTY_SL("ab"))));
   ASSERT(match.str() == LOFTY_SL("ab"));
   ASSERT(match.capture_group(0).str() == LOFTY_SL("ab"));
   ASSERT(!!(match = parser.run(LOFTY_SL("aab"))));
   ASSERT(match.str() == LOFTY_SL("aab"));
   ASSERT(match.capture_group(0).str() == LOFTY_SL("aab"));
   ASSERT(!!(match = parser.run(LOFTY_SL("aaab"))));
   ASSERT(match.str() == LOFTY_SL("aaab"));
   ASSERT(match.capture_group(0).str() == LOFTY_SL("aaab"));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_a_or_capture_b_capture_c,
   "lofty::text::parsers::dynamic – pattern “a|(b)(c)”"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(c_state, nullptr, nullptr, 'c');
   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(b_state, nullptr, nullptr, 'b');
   LOFTY_TEXT_PARSERS_DYNAMIC_CAPTURE_GROUP(c_cap_group, nullptr, nullptr, &c_state.base);
   LOFTY_TEXT_PARSERS_DYNAMIC_CAPTURE_GROUP(b_cap_group, &c_cap_group.base, nullptr, &b_state.base);
   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(a_state, nullptr, &b_cap_group.base, 'a');
   text::parsers::dynamic parser;
   parser.set_initial_state(&a_state.base);

   text::parsers::dynamic::match match;
   ASSERT(!parser.run(LOFTY_SL("")));
   ASSERT(!!(match = parser.run(LOFTY_SL("a"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT_THROWS(collections::out_of_range, match.capture_group(0));
   ASSERT(!!(match = parser.run(LOFTY_SL("ab"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT_THROWS(collections::out_of_range, match.capture_group(0));
   ASSERT(!!(match = parser.run(LOFTY_SL("abc"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 1u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT_THROWS(collections::out_of_range, match.capture_group(0));
   ASSERT(!!(match = parser.run(LOFTY_SL("ba"))));
   ASSERT(match.begin_char_index() == 1u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("a"));
   ASSERT_THROWS(collections::out_of_range, match.capture_group(0));
   ASSERT(!parser.run(LOFTY_SL("b")));
   ASSERT(!parser.run(LOFTY_SL("c")));
   ASSERT(!!(match = parser.run(LOFTY_SL("bc"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("bc"));
   ASSERT(match.capture_group(0).begin_char_index() == 0u);
   ASSERT(match.capture_group(0).end_char_index()   == 1u);
   ASSERT(match.capture_group(0).str() == LOFTY_SL("b"));
   ASSERT(match.capture_group(1).begin_char_index() == 1u);
   ASSERT(match.capture_group(1).end_char_index()   == 2u);
   ASSERT(match.capture_group(1).str() == LOFTY_SL("c"));
   ASSERT(!!(match = parser.run(LOFTY_SL("bc"))));
   ASSERT(match.begin_char_index() == 0u);
   ASSERT(match.end_char_index()   == 2u);
   ASSERT(match.str() == LOFTY_SL("bc"));
   ASSERT(match.capture_group(0).begin_char_index() == 0u);
   ASSERT(match.capture_group(0).end_char_index()   == 1u);
   ASSERT(match.capture_group(0).str() == LOFTY_SL("b"));
   ASSERT(match.capture_group(1).begin_char_index() == 1u);
   ASSERT(match.capture_group(1).end_char_index()   == 2u);
   ASSERT(match.capture_group(1).str() == LOFTY_SL("c"));
}

}} //namespace lofty::test
