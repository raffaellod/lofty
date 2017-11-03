/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015, 2017 Raffaello D. Di Napoli

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

   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 0u);
   LOFTY_TESTING_ASSERT_TRUE(map.begin() == map.cend());
   LOFTY_TESTING_ASSERT_TRUE(map.cbegin() == map.end());
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, ++map.end());
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, map.cend()++);
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, *map.cbegin());
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, *map.cend());
   LOFTY_TESTING_ASSERT_THROWS(collections::bad_access, map.front());
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, map.pop(map.begin()));
   LOFTY_TESTING_ASSERT_THROWS(collections::out_of_range, map.remove(map.begin()));
   LOFTY_TESTING_ASSERT_THROWS(collections::bad_access, map.pop_front());

   auto itr400(map.add(40, 400));
   // {40: 400}
   LOFTY_TESTING_ASSERT_EQUAL(itr400->key, 40);
   LOFTY_TESTING_ASSERT_EQUAL(itr400->value, 400);
   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().key, 40);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().value, 400);

   auto itr200(map.add(20, 200));
   // {20: 200}, {40: 400}
   LOFTY_TESTING_ASSERT_EQUAL(itr200->key, 20);
   LOFTY_TESTING_ASSERT_EQUAL(itr200->value, 200);
   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 2u);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().key, 20);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().value, 200);

   auto itr500(map.add(50, 500));
   // {20: 200}, {40: 400}, {50: 500}
   LOFTY_TESTING_ASSERT_EQUAL(itr500->key, 50);
   LOFTY_TESTING_ASSERT_EQUAL(itr500->value, 500);
   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 3u);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().key, 20);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().value, 200);

   auto itr300(map.add(30, 300));
   // {20: 200}, {30: 300}, {40: 400}, {50: 500}
   LOFTY_TESTING_ASSERT_EQUAL(itr300->key, 30);
   LOFTY_TESTING_ASSERT_EQUAL(itr300->value, 300);
   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 4u);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().key, 20);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().value, 200);

   auto itr201(map.add(20, 201));
   // {20: 201, 200}, {30: 300}, {40: 400}, {50: 500}
   LOFTY_TESTING_ASSERT_EQUAL(itr201->key, 20);
   LOFTY_TESTING_ASSERT_EQUAL(itr201->value, 201);
   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 5u);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().key, 20);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().value, 200);

   auto itr301(map.add(30, 301));
   // {20: 200, 201}, {30: 300, 301}, {40: 400}, {50: 500}
   LOFTY_TESTING_ASSERT_EQUAL(itr301->key, 30);
   LOFTY_TESTING_ASSERT_EQUAL(itr301->value, 301);
   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 6u);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().key, 20);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().value, 200);
   {
      static int const expected_keys  [] = {  20,  20,  30,  30,  40,  50, -1 };
      static int const expected_values[] = { 200, 201, 300, 301, 400, 500, -1 };
      int const * expected_key = &expected_keys[0];
      int const * expected_value = &expected_values[0];
      LOFTY_FOR_EACH(auto kv, map) {
         LOFTY_TESTING_ASSERT_EQUAL(kv.key, *expected_key++);
         LOFTY_TESTING_ASSERT_EQUAL(kv.value, *expected_value++);
      }
   }

   auto itr300_found(map.find(30));
   LOFTY_TESTING_ASSERT_EQUAL(itr300_found->key, 30);
   LOFTY_TESTING_ASSERT_EQUAL(itr300_found->value, 300);
   LOFTY_TESTING_ASSERT_TRUE(itr300_found == itr300);

   auto kv200(map.pop_front());
   // {20: 201}, {30: 300, 301}, {40: 400}, {50: 500}
   LOFTY_TESTING_ASSERT_EQUAL(kv200.key, 20);
   LOFTY_TESTING_ASSERT_EQUAL(kv200.value, 200);
   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 5u);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().key, 20);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().value, 201);

   map.remove(itr301);
   // {20: 201}, {30: 300}, {40, 400}, {50: 500}
   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 4u);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().key, 20);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().value, 201);

   auto kv201(map.pop_front());
   // {30: 300}, {40, 400}, {50: 500}
   LOFTY_TESTING_ASSERT_EQUAL(kv201.key, 20);
   LOFTY_TESTING_ASSERT_EQUAL(kv201.value, 201);
   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 3u);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().key, 30);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().value, 300);

   auto itr101(map.add(10, 101));
   // {10: 101}, {30: 300}, {40, 400}, {50: 500}
   LOFTY_TESTING_ASSERT_EQUAL(itr101->key, 10);
   LOFTY_TESTING_ASSERT_EQUAL(itr101->value, 101);
   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 4u);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().key, 10);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().value, 101);

   auto kv300(map.pop(itr300));
   // {10: 101}, {40, 400}, {50: 500}
   LOFTY_TESTING_ASSERT_EQUAL(kv300.key, 30);
   LOFTY_TESTING_ASSERT_EQUAL(kv300.value, 300);
   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 3u);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().key, 10);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().value, 101);

   auto itr302(map.add(30, 302));
   // {10: 101}, {30: 302} {40, 400}, {50: 500}
   LOFTY_TESTING_ASSERT_EQUAL(itr302->key, 30);
   LOFTY_TESTING_ASSERT_EQUAL(itr302->value, 302);
   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 4u);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().key, 10);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().value, 101);

   map.clear();
   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 0u);

   auto itr102(map.add(10, 102));
   // {10: 102}
   LOFTY_TESTING_ASSERT_EQUAL(itr102->key, 10);
   LOFTY_TESTING_ASSERT_EQUAL(itr102->value, 102);
   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().key, 10);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().value, 102);

   auto itr401(map.add(40, 401));
   // {10: 102}, {40: 401}
   LOFTY_TESTING_ASSERT_EQUAL(itr401->key, 40);
   LOFTY_TESTING_ASSERT_EQUAL(itr401->value, 401);
   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 2u);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().key, 10);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().value, 102);

   map.remove(itr102);
   // {40, 401}
   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().key, 40);
   LOFTY_TESTING_ASSERT_EQUAL(map.front().value, 401);

   auto kv401(map.pop(itr401));
   // empty
   LOFTY_TESTING_ASSERT_EQUAL(kv401.key, 40);
   LOFTY_TESTING_ASSERT_EQUAL(kv401.value, 401);
   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 0u);
   LOFTY_TESTING_ASSERT_THROWS(collections::bad_access, map.front());

   map.clear();
   LOFTY_TESTING_ASSERT_EQUAL(map.size(), 0u);
}

}} //namespace lofty::test
