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
   text_parsers_dynamic_onechar,
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
}

}} //namespace abc::test
