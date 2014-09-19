/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013, 2014
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



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::enum_basic

namespace abc {
namespace test {

ABC_ENUM(test_enum,
   (value1, 15),
   (value2, 56),
   (value3, 91)
);


class enum_basic : public testing::test_case {
public:

   //! See testing::test_case::title().
   virtual istr title() override {
      return istr(ABC_SL("abc::enum-derived classes – basic operations"));
   }

   //! See testing::test_case::run().
   virtual void run() override {
      ABC_TRACE_FUNC(this);

      test_enum e(test_enum::value2);

      ABC_TESTING_ASSERT_TRUE(e == test_enum::value2);
      ABC_TESTING_ASSERT_EQUAL(to_str(e), ABC_SL("value2"));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::enum_basic)


////////////////////////////////////////////////////////////////////////////////////////////////////

