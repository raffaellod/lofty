/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2015 Raffaello D. Di Napoli

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
#include <abaclade/testing/test_case.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

namespace {
   
ABC_ENUM_AUTO_VALUES(auto_enum_test,
   value0,
   value1,
   value2
);

} //namespace

ABC_TESTING_TEST_CASE_FUNC(
   enum_auto_values,
   "ABC_ENUM_AUTO_VALUES() – generated member values"
) {
   ABC_TRACE_FUNC(this);

   ABC_TESTING_ASSERT_EQUAL(static_cast<int>(auto_enum_test::value0), 0);
   ABC_TESTING_ASSERT_EQUAL(static_cast<int>(auto_enum_test::value1), 1);
   ABC_TESTING_ASSERT_EQUAL(static_cast<int>(auto_enum_test::value2), 2);
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

namespace {

ABC_ENUM(test_enum,
   (value1, 15),
   (value2, 56),
   (value3, 91)
);

} //namespace

ABC_TESTING_TEST_CASE_FUNC(
   enum_basic,
   "abc::enum-derived classes – basic operations"
) {
   ABC_TRACE_FUNC(this);

   test_enum e(test_enum::value2);

   ABC_TESTING_ASSERT_TRUE(e == test_enum::value2);
   ABC_TESTING_ASSERT_EQUAL(to_str(e), ABC_SL("value2"));
}

}} //namespace abc::test
