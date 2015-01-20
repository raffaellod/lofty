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
#include <abaclade/testing/test_case.hxx>
#include <abaclade/collections/list.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::list_basic

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC(list_basic, "abc::list – basic operations") {
   ABC_TRACE_FUNC(this);

   list<int> l;

   ABC_TESTING_ASSERT_EQUAL(l.size(), 0u);
   // These assertions target const begin/end.
   ABC_TESTING_ASSERT_TRUE(l.cbegin() == l.cend());
   ABC_TESTING_ASSERT_TRUE(l.crbegin() == l.crend());

   l.push_front(10);
   ABC_TESTING_ASSERT_EQUAL(l.size(), 1u);
   {
      /* This uses begin(), not cbegin(), so we can test equality comparison between const/non-const
      iterators. */
      auto it(l.begin());
      ABC_TESTING_ASSERT_EQUAL(*it, 10);
      ++it;
      ABC_TESTING_ASSERT_TRUE(it == l.cend());
   }

   l.push_back(20);
   ABC_TESTING_ASSERT_EQUAL(l.size(), 2u);
   {
      // This iterates backwards and is longer than, but symmetrical to, the block above.
      auto it(l.rbegin());
      ABC_TESTING_ASSERT_EQUAL(*it, 20);
      ++it;
      ABC_TESTING_ASSERT_EQUAL(*it, 10);
      ++it;
      ABC_TESTING_ASSERT_TRUE(it == l.crend());
   }

   l.pop_front();
   ABC_TESTING_ASSERT_EQUAL(l.size(), 1u);
   {
      // Now iterate backwards using a forward iterator.
      auto it(l.end());
      --it;
      ABC_TESTING_ASSERT_EQUAL(*it, 20);
      ABC_TESTING_ASSERT_TRUE(it == l.cbegin());
   }

   l.pop_back();
   ABC_TESTING_ASSERT_EQUAL(l.size(), 0u);
   // These assertions target non-const begin/end.
   ABC_TESTING_ASSERT_TRUE(l.begin() == l.end());
   ABC_TESTING_ASSERT_TRUE(l.rbegin() == l.rend());

   l.push_front(30);
   ABC_TESTING_ASSERT_EQUAL(l.size(), 1u);

   l.clear();
   ABC_TESTING_ASSERT_EQUAL(l.size(), 0u);
}

} //namespace test
} //namespace abc
