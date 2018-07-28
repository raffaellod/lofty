/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/collections.hxx>
#include <lofty/collections/list.hxx>
#include <lofty/logging.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/testing/utility.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   collections_list_basic,
   "lofty::collections::list – basic operations"
) {
   LOFTY_TRACE_FUNC();

   collections::list<int> l;

   ASSERT(!l);
   ASSERT(l.size() == 0u);
   // These assertions target const begin/end.
   ASSERT((l.cbegin() == l.cend()));
   ASSERT((l.crbegin() == l.crend()));

   l.push_front(10);
   ASSERT(!!l);
   ASSERT(l.size() == 1u);
   {
      /* This uses begin(), not cbegin(), so we can test equality comparison between const/non-const
      iterators. */
      auto itr(l.begin());
      ASSERT(*itr == 10);
      ++itr;
      ASSERT((itr == l.cend()));
   }

   l.push_back(20);
   ASSERT(!!l);
   ASSERT(l.size() == 2u);
   {
      // This iterates backwards and is longer than, but symmetrical to, the block above.
      auto itr(l.rbegin());
      ASSERT(*itr == 20);
      ++itr;
      ASSERT(*itr == 10);
      ++itr;
      ASSERT((itr == l.crend()));
   }

   l.pop_front();
   ASSERT(!!l);
   ASSERT(l.size() == 1u);

   l.pop_back();
   ASSERT(!l);
   ASSERT(l.size() == 0u);
   // These assertions target non-const begin/end.
   ASSERT((l.begin() == l.end()));
   ASSERT((l.rbegin() == l.rend()));

   l.push_front(30);
   ASSERT(!!l);
   ASSERT(l.size() == 1u);

   l.clear();
   ASSERT(!l);
   ASSERT(l.size() == 0u);
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

#if 0
/*! Instantiates and returns a list. The list will contain one node, added in a way that should cause only one
new instance of instances_counter to be created, one moved and none copied.

return
   Newly-instantiated list.
*/
static collections::list<testing::utility::instances_counter> return_list() {
   LOFTY_TRACE_FUNC();

   collections::list<testing::utility::instances_counter> l;
   // New instance, immediately moved.
   l.push_back(testing::utility::instances_counter());
   // This will move the entire list, not each node individually.
   return _std::move(l);
}

LOFTY_TESTING_TEST_CASE_FUNC(
   collections_list_nodes_movement,
   "lofty::collections::list – nodes movement"
) {
   LOFTY_TRACE_FUNC();

   typedef testing::utility::instances_counter instances_counter;
   {
      /* This will move the elements from the returned list to l1, so no node copies or moves will occur other
      than the ones in return_list(). */
      auto l(return_list());
      ASSERT(instances_counter::new_insts() == 1u);
      ASSERT(instances_counter::moves() == 1u);
      ASSERT(instances_counter::copies() == 0u);
      instances_counter::reset_counts();

      /* This should create a new copy, with no intermediate moves because all passages are by reference or
      pointer. */
      l.push_back(l.front());
      ASSERT(instances_counter::new_insts() == 0u);
      ASSERT(instances_counter::moves() == 0u);
      ASSERT(instances_counter::copies() == 1u);
      instances_counter::reset_counts();
   }
}
#endif

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   collections_list_iterators,
   "lofty::collections::list – operations with iterators"
) {
   LOFTY_TRACE_FUNC();

   collections::list<int> l;

   // Should not allow to move an iterator to outside [begin, end].
   ASSERT_DOES_NOT_THROW(l.cbegin());
   ASSERT_DOES_NOT_THROW(l.cend());
   ASSERT_THROWS(collections::out_of_range, ++l.cbegin());
   ASSERT_THROWS(collections::out_of_range, ++l.cend());

   // Should not allow to dereference end().
   ASSERT_THROWS(collections::out_of_range, *l.cend());
}

}} //namespace lofty::test
