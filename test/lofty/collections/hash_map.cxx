/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/collections.hxx>
#include <lofty/collections/hash_map.hxx>
#include <lofty/logging.hxx>
#include <lofty/_std/memory.hxx>
#include <lofty/testing/test_case.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   collections_hash_map_basic,
   "lofty::collections::hash_map – basic operations"
) {
   LOFTY_TRACE_FUNC();

   collections::hash_map<int, int> map;

   ASSERT(map.size() == 0u);
   // These assertions target const begin/end.
   ASSERT((map.cbegin() == map.cend()));

   map.add_or_assign(10, 100);
   ASSERT(map.size() == 1u);
   ASSERT(map[10] == 100);
   {
      /* This uses begin(), not cbegin(), so we can test equality comparison between const/non-const
      iterators. */
      auto itr(map.begin());
      ASSERT(itr->key == 10);
      ASSERT(itr->value == 100);
      ++itr;
      ASSERT((itr == map.cend()));
   }

   map.add_or_assign(20, 200);
   ASSERT(map.size() == 2u);
   ASSERT(map[10] == 100);
   ASSERT(map[20] == 200);

   ASSERT(map.remove_if_found(10));
   ASSERT(!map.remove_if_found(10));
   ASSERT_THROWS(collections::bad_key, map.remove(10));
   ASSERT(map.size() == 1u);
   ASSERT(map[20] == 200);
   ASSERT(!map.remove_if_found(10));

   map.add_or_assign(22, 220);
   ASSERT(map.size() == 2u);
   ASSERT(map[20] == 200);
   ASSERT(map[22] == 220);
   {
      // A little clunky, but neecessary since the order is not guaranteed.
      bool found20 = false, found22 = false;
      for (auto itr(map.begin()); itr != map.cend(); ++itr) {
         ASSERT((itr->key == 20 || itr->key == 22));
         if (itr->key == 20) {
            ASSERT(!found20);
            ASSERT(itr->value == 200);
            found20 = true;
         } else if (itr->key == 22) {
            ASSERT(!found22);
            ASSERT(itr->value == 220);
            found22 = true;
         }
      }
      ASSERT(found20);
      ASSERT(found22);
   }

   map.clear();
   ASSERT(map.size() == 0u);
   // These assertions target non-const begin/end.
   ASSERT((map.begin() == map.end()));

   map.add_or_assign(11, 110);
   ASSERT(map.size() == 1u);
   ASSERT(map[11] == 110);

   // Add enough key/value pairs until a resize occurs.
   int key = 11, value = 110;
   std::size_t initial_capacity = map.capacity();
   do {
      key += 11;
      value += 110;
      map.add_or_assign(key, value);
   } while (map.capacity() == initial_capacity);
   /* Verify that some values are still there. Can’t check them all because we don’t know exactly how many we
   ended up adding. */
   ASSERT(map[11] == 110);
   ASSERT(map[22] == 220);
   ASSERT(map[key - 11] == value - 110);
   ASSERT(map[key] == value);

   // Validate that non-copyable types can be stored in a map.
   {
      collections::hash_map<int, _std::unique_ptr<int>> map2;
      map2.add_or_assign(1, _std::unique_ptr<int>(new int(10)));
   }
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

namespace {

/*! Inefficient hash function that results in 100% hash collisions. This also checks that hash 0 (which has a
special meaning internally to lofty::collections::hash_map) behaves no differently than any other value.

@param i
   Value to hash.
@return
   Hash of i.
*/
struct poor_hash {
   std::size_t operator()(int i) const {
      LOFTY_UNUSED_ARG(i);
      return 0;
   }
};

} //namespace

LOFTY_TESTING_TEST_CASE_FUNC(
   collections_hash_map_collisions_stress,
   "lofty::collections::hash_map – stress test with 100% collisions"
) {
   LOFTY_TRACE_FUNC();

   static int const max = 1000;
   unsigned errors;
   collections::hash_map<int, int, poor_hash> map;

   // Verify that values are inserted correctly.
   errors = 0;
   for (int i = 0; i < max; ++i) {
      map.add_or_assign(i, i);
      if (map[i] != i) {
         ++errors;
      }
   }
   ASSERT(errors == 0u);

   // Verify that the insertion of later values did not break previously-inserted values.
   errors = 0;
   for (int i = 0; i < max; ++i) {
      if (map[i] != i) {
         ++errors;
      }
   }
   ASSERT(errors == 0u);
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   collections_hash_map_iterators,
   "lofty::collections::hash_map – operations with iterators"
) {
   LOFTY_TRACE_FUNC();

   collections::hash_map<int, int> map;

   // Should not allow to move an iterator to outside [begin, end].
   ASSERT_DOES_NOT_THROW(map.cbegin());
   ASSERT_DOES_NOT_THROW(map.cend());
   ASSERT_THROWS(collections::out_of_range, ++map.cbegin());
   ASSERT_THROWS(collections::out_of_range, ++map.cend());

   // Should not allow to dereference end().
   ASSERT_THROWS(collections::out_of_range, *map.cend());

   {
      auto itr(map.cbegin());
      map.add_or_assign(10, 100);
      // itr has been invalidated by add_or_assign().
      ASSERT_THROWS(collections::out_of_range, *itr);
   }

   LOFTY_FOR_EACH(auto kv, map) {
      ASSERT(kv.key == 10);
      ASSERT(kv.value == 100);
   }

   {
      auto itr(map.cbegin());
      map.remove(10);
      // itr has been invalidated by remove().
      ASSERT_THROWS(collections::out_of_range, *itr);
   }
}

}} //namespace lofty::test
