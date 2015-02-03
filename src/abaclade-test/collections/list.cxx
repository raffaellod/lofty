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
#include <abaclade/collections/list.hxx>
#include <abaclade/testing/test_case.hxx>
#include <abaclade/testing/utility.hxx>

#include <algorithm>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::collections::list – basic operations") {
   ABC_TRACE_FUNC(this);

   collections::list<int> l;

   ABC_TESTING_ASSERT_TRUE(l.empty());
   ABC_TESTING_ASSERT_EQUAL(l.size(), 0u);
   // These assertions target const begin/end.
   ABC_TESTING_ASSERT_TRUE(l.cbegin() == l.cend());
   ABC_TESTING_ASSERT_TRUE(l.crbegin() == l.crend());

   l.push_front(10);
   ABC_TESTING_ASSERT_FALSE(l.empty());
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
   ABC_TESTING_ASSERT_FALSE(l.empty());
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
   ABC_TESTING_ASSERT_FALSE(l.empty());
   ABC_TESTING_ASSERT_EQUAL(l.size(), 1u);
   {
      // Now iterate backwards using a forward iterator.
      auto it(l.end());
      --it;
      ABC_TESTING_ASSERT_EQUAL(*it, 20);
      ABC_TESTING_ASSERT_TRUE(it == l.cbegin());
   }

   l.pop_back();
   ABC_TESTING_ASSERT_TRUE(l.empty());
   ABC_TESTING_ASSERT_EQUAL(l.size(), 0u);
   // These assertions target non-const begin/end.
   ABC_TESTING_ASSERT_TRUE(l.begin() == l.end());
   ABC_TESTING_ASSERT_TRUE(l.rbegin() == l.rend());

   l.push_front(30);
   ABC_TESTING_ASSERT_FALSE(l.empty());
   ABC_TESTING_ASSERT_EQUAL(l.size(), 1u);

   l.clear();
   ABC_TESTING_ASSERT_TRUE(l.empty());
   ABC_TESTING_ASSERT_EQUAL(l.size(), 0u);
}

} //namespace test
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {
namespace test {

namespace {

/*! Instantiates and returns a list. The list will contain one node, added in a way that should
cause only one new instance of instances_counter to be created, one moved and none copied.

return
   Newly-instantiated list.
*/
collections::list<testing::utility::instances_counter> return_list() {
   ABC_TRACE_FUNC();

   collections::list<testing::utility::instances_counter> l;
   // New instance, immediately moved.
   l.push_back(testing::utility::instances_counter());
   // This will move the entire list, not each node individually.
   return std::move(l);
}

} //namespace

ABC_TESTING_TEST_CASE_FUNC("abc::collections::list – nodes movement") {
   ABC_TRACE_FUNC(this);

   typedef testing::utility::instances_counter instances_counter;
   {
      /* This will move the elements from the returned list to l1, so no node copies or moves will
      occur other than the ones in return_list(). */
      collections::list<instances_counter> l(return_list());
      ABC_TESTING_ASSERT_EQUAL(instances_counter::new_insts(), 1u);
      ABC_TESTING_ASSERT_EQUAL(instances_counter::moves(), 1u);
      ABC_TESTING_ASSERT_EQUAL(instances_counter::copies(), 0u);
      instances_counter::reset_counts();

      /* This should create a new copy, with no intermediate moves because all passages are by
      reference or pointer. */
      l.push_back(l.front());
      ABC_TESTING_ASSERT_EQUAL(instances_counter::new_insts(), 0u);
      ABC_TESTING_ASSERT_EQUAL(instances_counter::moves(), 0u);
      ABC_TESTING_ASSERT_EQUAL(instances_counter::copies(), 1u);
      instances_counter::reset_counts();
   }
}

} //namespace test
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::collections::list – operations with iterators") {
   ABC_TRACE_FUNC(this);

   collections::list<int> l;

   // Should not allow to move an iterator to outside [begin, end].
   ABC_TESTING_ASSERT_DOES_NOT_THROW(l.cbegin());
   ABC_TESTING_ASSERT_DOES_NOT_THROW(l.cend());
   ABC_TESTING_ASSERT_THROWS(iterator_error, --l.cbegin());
   ABC_TESTING_ASSERT_THROWS(iterator_error, ++l.cbegin());
   ABC_TESTING_ASSERT_THROWS(iterator_error, --l.cend());
   ABC_TESTING_ASSERT_THROWS(iterator_error, ++l.cend());

   // Should not allow to dereference end().
   ABC_TESTING_ASSERT_THROWS(iterator_error, *l.cend());

   l.push_back(1);
   l.push_back(2);
   l.push_back(3);
   l.push_back(4);

   {
      /* Remove an element by iterator while holding that iterator itself and the two following
      ones. */
      auto it1(l.begin());
      auto it2(std::find(l.cbegin(), l.cend(), 2)), it3(it2);
      ++it3;
      auto it4(it3);
      ++it4;
      /* This invalidates it2 and partially it1 and it3 (because they hold a pointer to *it2), but
      not it4. */
      l.remove_at(it2);

      // it1 cannot go forwards, and it never could go backwards.
      ABC_TESTING_ASSERT_DOES_NOT_THROW(*it1);
//    ABC_TESTING_ASSERT_THROWS(iterator_error, it1 + 1);
      ABC_TESTING_ASSERT_THROWS(iterator_error, it1 - 1);
      ABC_TESTING_ASSERT_EQUAL(*it1, 1);

      // it2 is completely invalidated.
//    ABC_TESTING_ASSERT_THROWS(iterator_error, *it2);
//    ABC_TESTING_ASSERT_THROWS(iterator_error, it2 + 1);
//    ABC_TESTING_ASSERT_THROWS(iterator_error, it2 - 1);

      // it3 cannot go backwards.
      ABC_TESTING_ASSERT_DOES_NOT_THROW(*it3);
      ABC_TESTING_ASSERT_DOES_NOT_THROW(it3 + 1);
//    ABC_TESTING_ASSERT_THROWS(iterator_error, it3 - 1);
      ABC_TESTING_ASSERT_EQUAL(*it3, 3);

      // it4 is unaffected.
      ABC_TESTING_ASSERT_DOES_NOT_THROW(*it4);
      ABC_TESTING_ASSERT_DOES_NOT_THROW(it4 + 1);
      ABC_TESTING_ASSERT_DOES_NOT_THROW(it4 - 1);
      ABC_TESTING_ASSERT_EQUAL(*it4, 4);
   }
}

} //namespace test
} //namespace abc
