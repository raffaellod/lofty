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
   "abc::text::parsers::dynamic – one-character pattern “a”"
) {
   ABC_TRACE_FUNC(this);

   text::parsers::dynamic dp;
   auto pst = dp.create_state();
   pst->set_code_point('a');
   dp.set_initial_state(pst);

   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("a")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("aa")));
   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("b")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("ba")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("ab")));
}

ABC_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_begin,
   "abc::text::parsers::dynamic – begin pattern “^”"
) {
   ABC_TRACE_FUNC(this);

   text::parsers::dynamic dp;
   auto pst = dp.create_state();
   pst->set_begin();
   dp.set_initial_state(pst);

   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("a")));
}

ABC_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_begin_anchored,
   "abc::text::parsers::dynamic – begin-anchored pattern “^a”"
) {
   ABC_TRACE_FUNC(this);

   text::parsers::dynamic dp;
   auto pstA = dp.create_state();
   pstA->set_code_point('a');
   auto pstBegin = dp.create_state();
   pstBegin->set_begin();
   pstBegin->set_next(pstA);
   dp.set_initial_state(pstBegin);

   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("a")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("aa")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("ab")));
   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("b")));
   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("ba")));
}

ABC_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_end,
   "abc::text::parsers::dynamic – end pattern “$”"
) {
   ABC_TRACE_FUNC(this);

   text::parsers::dynamic dp;
   auto pst = dp.create_state();
   pst->set_end();
   dp.set_initial_state(pst);

   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("a")));
}

ABC_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_end_anchored,
   "abc::text::parsers::dynamic – end-anchored pattern “a$”"
) {
   ABC_TRACE_FUNC(this);

   text::parsers::dynamic dp;
   auto pstEnd = dp.create_state();
   pstEnd->set_end();
   auto pstA = dp.create_state();
   pstA->set_code_point('a');
   pstA->set_next(pstEnd);
   dp.set_initial_state(pstA);

   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("a")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("aa")));
   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("ab")));
   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("b")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("ba")));
}

ABC_TESTING_TEST_CASE_FUNC(
   text_parsers_dynamic_two_char,
   "abc::text::parsers::dynamic – two-character pattern “ab”"
) {
   ABC_TRACE_FUNC(this);

   text::parsers::dynamic dp;
   auto pstB = dp.create_state();
   pstB->set_code_point('b');
   auto pstA = dp.create_state();
   pstA->set_code_point('a');
   pstA->set_next(pstB);
   dp.set_initial_state(pstA);

   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("")));
   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("a")));
   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("aa")));
   ABC_TESTING_ASSERT_FALSE(dp.run(ABC_SL("b")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("ab")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("bab")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("aab")));
   ABC_TESTING_ASSERT_TRUE(dp.run(ABC_SL("aaba")));
}

}} //namespace abc::test
