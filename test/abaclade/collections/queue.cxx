/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015
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
#include <abaclade/collections/queue.hxx>
#include <abaclade/testing/test_case.hxx>
#include <abaclade/testing/utility.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::collections::queue – basic operations") {
   ABC_TRACE_FUNC(this);

   collections::queue<int> q;

   ABC_TESTING_ASSERT_TRUE(q.empty());
   ABC_TESTING_ASSERT_EQUAL(q.size(), 0);

   q.push_back(10);
   ABC_TESTING_ASSERT_FALSE(q.empty());
   ABC_TESTING_ASSERT_EQUAL(q.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(q.front(), 10);
   ABC_TESTING_ASSERT_EQUAL(q.back(), 10);

   q.push_back(20);
   ABC_TESTING_ASSERT_FALSE(q.empty());
   ABC_TESTING_ASSERT_EQUAL(q.size(), 2u);
   ABC_TESTING_ASSERT_EQUAL(q.front(), 10);
   ABC_TESTING_ASSERT_EQUAL(q.back(), 20);

   ABC_TESTING_ASSERT_EQUAL(q.pop_front(), 10);
   ABC_TESTING_ASSERT_FALSE(q.empty());
   ABC_TESTING_ASSERT_EQUAL(q.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(q.front(), 20);
   ABC_TESTING_ASSERT_EQUAL(q.back(), 20);

   ABC_TESTING_ASSERT_EQUAL(q.pop_front(), 20);
   ABC_TESTING_ASSERT_TRUE(q.empty());
   ABC_TESTING_ASSERT_EQUAL(q.size(), 0);

   q.push_back(30);
   ABC_TESTING_ASSERT_FALSE(q.empty());
   ABC_TESTING_ASSERT_EQUAL(q.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(q.front(), 30);
   ABC_TESTING_ASSERT_EQUAL(q.back(), 30);

   q.clear();
   ABC_TESTING_ASSERT_TRUE(q.empty());
   ABC_TESTING_ASSERT_EQUAL(q.size(), 0);
}

} //namespace test
} //namespace abc
