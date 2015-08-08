/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014, 2015
Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with Abaclade. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#include <abaclade/testing/test_case.hxx>
#include <abaclade/range.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   range_basic,
   "abc::range – basic operations"
) {
   ABC_TRACE_FUNC(this);

   range<int> r1;
   ABC_TESTING_ASSERT_EQUAL(r1.size(), 0u);
   ABC_TESTING_ASSERT_FALSE(r1.contains(-1));
   ABC_TESTING_ASSERT_FALSE(r1.contains(0));
   ABC_TESTING_ASSERT_FALSE(r1.contains(1));

   range<int> r2(1, 2);
   ABC_TESTING_ASSERT_EQUAL(r2.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(*r2.begin(), 1);
   ABC_TESTING_ASSERT_FALSE(r2.contains(0));
   ABC_TESTING_ASSERT_TRUE(r2.contains(1));
   ABC_TESTING_ASSERT_FALSE(r2.contains(2));
}

}} //namespace abc::test
