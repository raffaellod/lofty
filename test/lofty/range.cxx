/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2015, 2017 Raffaello D. Di Napoli

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
#include <lofty/testing/test_case.hxx>
#include <lofty/range.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   range_basic,
   "lofty::range – basic operations"
) {
   LOFTY_TRACE_FUNC(this);

   range<int> range1;
   LOFTY_TESTING_ASSERT_EQUAL(range1.size(), 0u);
   LOFTY_TESTING_ASSERT_FALSE(range1.contains(-1));
   LOFTY_TESTING_ASSERT_FALSE(range1.contains(0));
   LOFTY_TESTING_ASSERT_FALSE(range1.contains(1));

   range<int> range2(1, 2);
   LOFTY_TESTING_ASSERT_EQUAL(range2.size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(*range2.begin(), 1);
   LOFTY_TESTING_ASSERT_FALSE(range2.contains(0));
   LOFTY_TESTING_ASSERT_TRUE(range2.contains(1));
   LOFTY_TESTING_ASSERT_FALSE(range2.contains(2));
}

}} //namespace lofty::test
