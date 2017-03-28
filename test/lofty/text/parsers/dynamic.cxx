/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016-2017 Raffaello D. Di Napoli

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
#include <lofty/text/parsers/dynamic.hxx>
#include <lofty/testing/test_case.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_empty,
   "lofty::text::parsers::dynamic – pattern “” (empty)"
) {
   LOFTY_TRACE_FUNC(this);

   text::parsers::dynamic parser;

   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("a")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("aa")));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_a,
   "lofty::text::parsers::dynamic – pattern “a”"
) {
   LOFTY_TRACE_FUNC(this);

   text::parsers::dynamic parser;
   parser.set_initial_state(parser.create_code_point_state('a'));

   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("a")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("aa")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("ba")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("ab")));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_caret,
   "lofty::text::parsers::dynamic – pattern “^”"
) {
   LOFTY_TRACE_FUNC(this);

   text::parsers::dynamic parser;
   parser.set_initial_state(parser.create_begin_state());

   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("a")));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_caret_a,
   "lofty::text::parsers::dynamic – pattern “^a”"
) {
   LOFTY_TRACE_FUNC(this);

   text::parsers::dynamic parser;
   parser.set_initial_state(parser.create_begin_state()->set_next(parser.create_code_point_state('a')));

   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("a")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("aa")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("ab")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("ba")));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_dollar,
   "lofty::text::parsers::dynamic – pattern “$”"
) {
   LOFTY_TRACE_FUNC(this);

   text::parsers::dynamic parser;
   parser.set_initial_state(parser.create_end_state());

   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("a")));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_a_dollar,
   "lofty::text::parsers::dynamic – pattern “a$”"
) {
   LOFTY_TRACE_FUNC(this);

   text::parsers::dynamic parser;
   parser.set_initial_state(parser.create_code_point_state('a')->set_next(parser.create_end_state()));

   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("a")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("aa")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("ab")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("ba")));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_ab,
   "lofty::text::parsers::dynamic – pattern “ab”"
) {
   LOFTY_TRACE_FUNC(this);

   text::parsers::dynamic parser;
   parser.set_initial_state(
      parser.create_code_point_state('a')->set_next(parser.create_code_point_state('b'))
   );

   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("a")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("aa")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("ab")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("bab")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("aab")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("aaba")));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_a_qmark,
   "lofty::text::parsers::dynamic – pattern “a?”"
) {
   LOFTY_TRACE_FUNC(this);

   text::parsers::dynamic parser;
   auto a_state = parser.create_code_point_state('a');
   auto a_rep_state = parser.create_repetition_state(a_state, 0, 1);
   a_state->set_next(a_rep_state);
   parser.set_initial_state(a_rep_state);

   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("a")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("aa")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("ba")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("ab")));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_a_plus,
   "lofty::text::parsers::dynamic – pattern “a+”"
) {
   LOFTY_TRACE_FUNC(this);

   text::parsers::dynamic parser;
   auto a_state = parser.create_code_point_state('a');
   auto a_rep_state = parser.create_repetition_state(a_state, 1);
   a_state->set_next(a_rep_state);
   parser.set_initial_state(a_rep_state);

   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("a")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("aa")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("ba")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("ab")));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_a_plus_b_plus,
   "lofty::text::parsers::dynamic – pattern “a+b+”"
) {
   LOFTY_TRACE_FUNC(this);

   text::parsers::dynamic parser;
   auto b_state = parser.create_code_point_state('b');
   auto b_rep_state = parser.create_repetition_state(b_state, 1);
   b_state->set_next(b_rep_state);
   auto a_state = parser.create_code_point_state('a');
   auto a_rep_state = parser.create_repetition_state(a_state, 1);
   a_state->set_next(a_rep_state);
   a_rep_state->set_next(b_rep_state);
   parser.set_initial_state(a_rep_state);

   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("a")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("aa")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("ab")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("aba")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("abb")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("abab")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("ba")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("bab")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("baba")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("babb")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("babab")));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_caret_a_plus_b_plus_dollar,
   "lofty::text::parsers::dynamic – pattern “^a+b+$”"
) {
   LOFTY_TRACE_FUNC(this);

   text::parsers::dynamic parser;
   auto b_state = parser.create_code_point_state('b');
   auto b_rep_state = parser.create_repetition_state(b_state, 1);
   b_rep_state->set_next(parser.create_end_state());
   b_state->set_next(b_rep_state);
   auto a_state = parser.create_code_point_state('a');
   auto a_rep_state = parser.create_repetition_state(a_state, 1);
   a_state->set_next(a_rep_state);
   a_rep_state->set_next(b_rep_state);
   parser.set_initial_state(parser.create_begin_state()->set_next(a_rep_state));

   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("a")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("aa")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("aab")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("aabb")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("aabba")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("ab")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("aba")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("abb")));
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
   LOFTY_TRACE_FUNC(this);

   text::parsers::dynamic parser;
   auto b_state = parser.create_code_point_state('b');
   auto a_state = parser.create_code_point_state('a');
   a_state->set_alternative(b_state);
   auto a_or_b_rep_state = parser.create_repetition_state(a_state, 1);
   a_state->set_next(a_or_b_rep_state);
   b_state->set_next(a_or_b_rep_state);
   parser.set_initial_state(a_or_b_rep_state);

   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("a")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("aa")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("ab")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("abc")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("bb")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("ba")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("bac")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("c")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("ca")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("cab")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("cc")));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_caret_a_or_b_plus_dollar,
   "lofty::text::parsers::dynamic – pattern “^(?:a|b)+$”"
) {
   LOFTY_TRACE_FUNC(this);

   text::parsers::dynamic parser;
   auto b_state = parser.create_code_point_state('b');
   auto a_state = parser.create_code_point_state('a');
   a_state->set_alternative(b_state);
   auto a_or_b_rep_state = parser.create_repetition_state(a_state, 1);
   a_or_b_rep_state->set_next(parser.create_end_state());
   a_state->set_next(a_or_b_rep_state);
   b_state->set_next(a_or_b_rep_state);
   parser.set_initial_state(parser.create_begin_state()->set_next(a_or_b_rep_state));

   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("a")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("aa")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("ab")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("abc")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("bb")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("ba")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("bac")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("c")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("ca")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("cab")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("cc")));
}

LOFTY_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_pattern_capturing_plus_capture_a_capture_b,
   "lofty::text::parsers::dynamic – pattern “((a)(b))+”"
) {
   LOFTY_TRACE_FUNC(this);

   text::parsers::dynamic parser;
   parser.set_initial_state(parser.create_repetition_state(parser.create_capture_group(
      parser.create_capture_group(parser.create_code_point_state('a'))->set_next(
         parser.create_capture_group(parser.create_code_point_state('b'))
      )
   ), 1));

   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("a")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("aa")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("ab")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("abc")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("b")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("bb")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("ba")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("bac")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("c")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("ca")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("cab")));
   LOFTY_TESTING_ASSERT_FALSE(parser.run(LOFTY_SL("cc")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("aab")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("abb")));
   LOFTY_TESTING_ASSERT_TRUE(parser.run(LOFTY_SL("abab")));
}

}} //namespace lofty::test
