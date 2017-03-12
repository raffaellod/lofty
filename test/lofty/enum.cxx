/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2017 Raffaello D. Di Napoli

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
#include <lofty/to_str.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

namespace {
   
LOFTY_ENUM_AUTO_VALUES(auto_enum_test,
   value0,
   value1,
   value2
);

} //namespace

LOFTY_TESTING_TEST_CASE_FUNC(
   enum_auto_values,
   "LOFTY_ENUM_AUTO_VALUES() – generated member values"
) {
   LOFTY_TRACE_FUNC(this);

   LOFTY_TESTING_ASSERT_EQUAL(static_cast<int>(auto_enum_test::value0), 0);
   LOFTY_TESTING_ASSERT_EQUAL(static_cast<int>(auto_enum_test::value1), 1);
   LOFTY_TESTING_ASSERT_EQUAL(static_cast<int>(auto_enum_test::value2), 2);
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

namespace {

LOFTY_ENUM(test_enum,
   (value1, 15),
   (value2, 56),
   (value3, 91)
);

} //namespace

LOFTY_TESTING_TEST_CASE_FUNC(
   enum_basic,
   "lofty::enum-derived classes – basic operations"
) {
   LOFTY_TRACE_FUNC(this);

   test_enum e(test_enum::value2);

   LOFTY_TESTING_ASSERT_TRUE(e == test_enum::value2);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(e), LOFTY_SL("value2"));
}

}} //namespace lofty::test
