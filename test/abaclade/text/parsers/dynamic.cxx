/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016 Raffaello D. Di Napoli

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
#include <abaclade/text/parsers/dynamic.hxx>
#include <abaclade/testing/test_case.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_one_char,
   "abc::text::parsers::dynamic – one-character pattern"
) {
   ABC_TRACE_FUNC(this);

   text::parsers::dynamic dp;
   auto ps = dp.create_state();
   ps->set_code_point('a');
   dp.set_initial_state(ps);

   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("a")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("aa")));
   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("b")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("ba")));
}

ABC_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_begin,
   "abc::text::parsers::dynamic – begin pattern (“^”)"
) {
   ABC_TRACE_FUNC(this);

   text::parsers::dynamic dp;
   auto ps = dp.create_state();
   ps->set_begin();
   dp.set_initial_state(ps);

   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("a")));
}

ABC_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_begin_anchored,
   "abc::text::parsers::dynamic – begin-anchored pattern (“^a”)"
) {
   ABC_TRACE_FUNC(this);

   text::parsers::dynamic dp;
   auto psA = dp.create_state();
   psA->set_code_point('a');
   auto psBegin = dp.create_state();
   psBegin->set_begin();
   psBegin->set_next(psA);
   dp.set_initial_state(psBegin);

   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("a")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("aa")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("ab")));
   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("b")));
   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("ba")));
}

ABC_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_end,
   "abc::text::parsers::dynamic – end pattern (“$”)"
) {
   ABC_TRACE_FUNC(this);

   text::parsers::dynamic dp;
   auto ps = dp.create_state();
   ps->set_end();
   dp.set_initial_state(ps);

   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("a")));
}

ABC_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_end_anchored,
   "abc::text::parsers::dynamic – end-anchored pattern (“a$”)"
) {
   ABC_TRACE_FUNC(this);

   text::parsers::dynamic dp;
   auto psEnd = dp.create_state();
   psEnd->set_end();
   auto psA = dp.create_state();
   psA->set_code_point('a');
   psA->set_next(psEnd);
   dp.set_initial_state(psA);

   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("a")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("aa")));
   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("ab")));
   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("b")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("ba")));
}

ABC_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_two_char,
   "abc::text::parsers::dynamic – two-character pattern"
) {
   ABC_TRACE_FUNC(this);

   text::parsers::dynamic dp;
   auto psB = dp.create_state();
   psB->set_code_point('b');
   auto psA = dp.create_state();
   psA->set_code_point('a');
   psA->set_next(psB);
   dp.set_initial_state(psA);

   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("")));
   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("a")));
   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("aa")));
   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("b")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("ab")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("bab")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("aab")));
}

}} //namespace abc::test
