/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/collections.hxx>
#include <lofty/collections/trie_ordered_multimap.hxx>
#include <lofty/logging.hxx>
#include <lofty/testing/test_case.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   collections_trie_ordered_multimap_bitwise_basic,
   "lofty::collections::trie_ordered_multimap (bitwise) – basic operations"
) {
   LOFTY_TRACE_FUNC();

   collections::trie_ordered_multimap<int, int> map;

   ASSERT(map.size() == 0u);
   ASSERT((map.begin() == map.cend()));
   ASSERT((map.cbegin() == map.end()));
   ASSERT_THROWS(collections::out_of_range, ++map.end());
   ASSERT_THROWS(collections::out_of_range, map.cend()++);
   ASSERT_THROWS(collections::out_of_range, *map.cbegin());
   ASSERT_THROWS(collections::out_of_range, *map.cend());
   ASSERT_THROWS(collections::bad_access, map.front());
   ASSERT_THROWS(collections::out_of_range, map.pop(map.begin()));
   ASSERT_THROWS(collections::out_of_range, map.remove(map.begin()));
   ASSERT_THROWS(collections::bad_access, map.pop_front());

   auto itr400(map.add(40, 400));
   // {40: 400}
   ASSERT(itr400->key == 40);
   ASSERT(itr400->value == 400);
   ASSERT(map.size() == 1u);
   ASSERT(map.front().key == 40);
   ASSERT(map.front().value == 400);

   auto itr200(map.add(20, 200));
   // {20: 200}, {40: 400}
   ASSERT(itr200->key == 20);
   ASSERT(itr200->value == 200);
   ASSERT(map.size() == 2u);
   ASSERT(map.front().key == 20);
   ASSERT(map.front().value == 200);

   auto itr500(map.add(50, 500));
   // {20: 200}, {40: 400}, {50: 500}
   ASSERT(itr500->key == 50);
   ASSERT(itr500->value == 500);
   ASSERT(map.size() == 3u);
   ASSERT(map.front().key == 20);
   ASSERT(map.front().value == 200);

   auto itr300(map.add(30, 300));
   // {20: 200}, {30: 300}, {40: 400}, {50: 500}
   ASSERT(itr300->key == 30);
   ASSERT(itr300->value == 300);
   ASSERT(map.size() == 4u);
   ASSERT(map.front().key == 20);
   ASSERT(map.front().value == 200);

   auto itr201(map.add(20, 201));
   // {20: 201, 200}, {30: 300}, {40: 400}, {50: 500}
   ASSERT(itr201->key == 20);
   ASSERT(itr201->value == 201);
   ASSERT(map.size() == 5u);
   ASSERT(map.front().key == 20);
   ASSERT(map.front().value == 200);

   auto itr301(map.add(30, 301));
   // {20: 200, 201}, {30: 300, 301}, {40: 400}, {50: 500}
   ASSERT(itr301->key == 30);
   ASSERT(itr301->value == 301);
   ASSERT(map.size() == 6u);
   ASSERT(map.front().key == 20);
   ASSERT(map.front().value == 200);
   {
      static int const expected_keys  [] = {  20,  20,  30,  30,  40,  50, -1 };
      static int const expected_values[] = { 200, 201, 300, 301, 400, 500, -1 };
      int const * expected_key = &expected_keys[0];
      int const * expected_value = &expected_values[0];
      LOFTY_FOR_EACH(auto kv, map) {
         ASSERT(kv.key == *expected_key++);
         ASSERT(kv.value == *expected_value++);
      }
   }

   auto itr300_found(map.find(30));
   ASSERT(itr300_found->key == 30);
   ASSERT(itr300_found->value == 300);
   ASSERT((itr300_found == itr300));

   auto kv200(map.pop_front());
   // {20: 201}, {30: 300, 301}, {40: 400}, {50: 500}
   ASSERT(kv200.key == 20);
   ASSERT(kv200.value == 200);
   ASSERT(map.size() == 5u);
   ASSERT(map.front().key == 20);
   ASSERT(map.front().value == 201);

   map.remove(itr301);
   // {20: 201}, {30: 300}, {40, 400}, {50: 500}
   ASSERT(map.size() == 4u);
   ASSERT(map.front().key == 20);
   ASSERT(map.front().value == 201);

   auto kv201(map.pop_front());
   // {30: 300}, {40, 400}, {50: 500}
   ASSERT(kv201.key == 20);
   ASSERT(kv201.value == 201);
   ASSERT(map.size() == 3u);
   ASSERT(map.front().key == 30);
   ASSERT(map.front().value == 300);

   auto itr101(map.add(10, 101));
   // {10: 101}, {30: 300}, {40, 400}, {50: 500}
   ASSERT(itr101->key == 10);
   ASSERT(itr101->value == 101);
   ASSERT(map.size() == 4u);
   ASSERT(map.front().key == 10);
   ASSERT(map.front().value == 101);

   auto kv300(map.pop(itr300));
   // {10: 101}, {40, 400}, {50: 500}
   ASSERT(kv300.key == 30);
   ASSERT(kv300.value == 300);
   ASSERT(map.size() == 3u);
   ASSERT(map.front().key == 10);
   ASSERT(map.front().value == 101);

   auto itr302(map.add(30, 302));
   // {10: 101}, {30: 302} {40, 400}, {50: 500}
   ASSERT(itr302->key == 30);
   ASSERT(itr302->value == 302);
   ASSERT(map.size() == 4u);
   ASSERT(map.front().key == 10);
   ASSERT(map.front().value == 101);

   map.clear();
   ASSERT(map.size() == 0u);

   auto itr102(map.add(10, 102));
   // {10: 102}
   ASSERT(itr102->key == 10);
   ASSERT(itr102->value == 102);
   ASSERT(map.size() == 1u);
   ASSERT(map.front().key == 10);
   ASSERT(map.front().value == 102);

   auto itr401(map.add(40, 401));
   // {10: 102}, {40: 401}
   ASSERT(itr401->key == 40);
   ASSERT(itr401->value == 401);
   ASSERT(map.size() == 2u);
   ASSERT(map.front().key == 10);
   ASSERT(map.front().value == 102);

   map.remove(itr102);
   // {40, 401}
   ASSERT(map.size() == 1u);
   ASSERT(map.front().key == 40);
   ASSERT(map.front().value == 401);

   auto kv401(map.pop(itr401));
   // empty
   ASSERT(kv401.key == 40);
   ASSERT(kv401.value == 401);
   ASSERT(map.size() == 0u);
   ASSERT_THROWS(collections::bad_access, map.front());

   map.clear();
   ASSERT(map.size() == 0u);
}

}} //namespace lofty::test
