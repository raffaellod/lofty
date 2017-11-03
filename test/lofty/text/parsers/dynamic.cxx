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
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL(""))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL(""));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("a"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL(""));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aa"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL(""));
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
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("a"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aa"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ba"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
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
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL(""))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL(""));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("a"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL(""));
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
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("a"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aa"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("ba")));
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
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL(""))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL(""));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("a"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL(""));
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
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("a"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aa"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("ab")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ba"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
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
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("a")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("aa")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("bab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aaba"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aabab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
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
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("a")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("aa")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("ab")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("abc"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("abc"));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("aab")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aabc"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   4u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("abc"));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("babc"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   4u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("abc"));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("aaba")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aabca"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   4u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("abc"));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("aabab")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aababc"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   6u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("abc"));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aabcabc"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   4u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("abc"));
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
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL(""))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL(""));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 0u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("a"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aa"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("b"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL(""));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 0u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ba"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL(""));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 0u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
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
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("a"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aa"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("aa"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 2u);
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ba"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
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
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("a"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 0u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aa"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("aa"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aaa"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("aaa"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 2u);
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ba"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 0u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("baa"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("aa"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("bb")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 0u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aba"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 0u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aaba"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("aa"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
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
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("a")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("aa")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(1).size(), 1u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aba"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(1).size(), 1u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("abb"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("abb"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(1).size(), 2u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("abab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(1).size(), 1u);
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("ba")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("bab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(1).size(), 1u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("baba"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(1).size(), 1u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("babb"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   4u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("abb"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(1).size(), 2u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("babab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(1).size(), 1u);
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
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("a")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("aa")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("aab"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(1).size(), 1u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aabb"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   4u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("aabb"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(1).size(), 2u);
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("aabba")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(1).size(), 1u);
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("aba")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("abb"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("abb"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(1).size(), 2u);
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("abab")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("ba")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("bab")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("baba")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("babb")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("babab")));
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
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("a"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aa"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("aa"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 2u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 2u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("abc"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 2u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("b"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("b"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("bb"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("bb"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 2u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ba"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ba"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 2u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("bac"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ba"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 2u);
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("c")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ca"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("cab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 2u);
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("cc")));
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
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("a"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aa"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("aa"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 2u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 2u);
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("abc")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("b"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("b"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("bb"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("bb"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 2u);
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ba"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ba"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 2u);
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("bac")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("c")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("ca")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("cab")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("cc")));
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
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("a")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("aa")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("ab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(0).begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(0).end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(0).str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(1).begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(1).end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(1).str(), LOFTY_SL("b"));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("abc"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(0).begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(0).end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(0).str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(1).begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(1).end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(1).str(), LOFTY_SL("b"));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("bb")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("ba")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("bac")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("c")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("ca")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("cab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(0).begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(0).end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(0).str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(1).begin_char_index(), 2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(1).end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(1).str(), LOFTY_SL("b"));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("cc")));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("aab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(0).begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(0).end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(0).str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(1).begin_char_index(), 2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(1).end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(1).str(), LOFTY_SL("b"));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("abb"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(0).begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(0).end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(0).str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(1).begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(1).end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(1).str(), LOFTY_SL("b"));
   LOFTY_TESTING_ASSERT_TRUE((match = parser.run(LOFTY_SL("abab"))));
   LOFTY_TESTING_ASSERT_EQUAL(match.begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.end_char_index(),   4u);
   LOFTY_TESTING_ASSERT_EQUAL(match.str(), LOFTY_SL("abab"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0).size(), 2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(0).begin_char_index(), 0u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(0).end_char_index(),   1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(0).str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(1).begin_char_index(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(1).end_char_index(),   2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[0].capture_group(1).str(), LOFTY_SL("b"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[1].capture_group(0).begin_char_index(), 2u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[1].capture_group(0).end_char_index(),   3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[1].capture_group(0).str(), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[1].capture_group(1).begin_char_index(), 3u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[1].capture_group(1).end_char_index(),   4u);
   LOFTY_TESTING_ASSERT_EQUAL(match.repetition_group(0)[1].capture_group(1).str(), LOFTY_SL("b"));
}

}} //namespace lofty::test
