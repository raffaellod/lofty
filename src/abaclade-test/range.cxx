/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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
// abc::test::range_basic

namespace abc {
namespace test {

class range_basic : public testing::test_case {
public:
   //! See testing::test_case::title().
   virtual istr title() override {
      return istr(ABC_SL("abc::range – basic operations"));
   }

   //! See testing::test_case::run().
   virtual void run() override {
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
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::range_basic)
