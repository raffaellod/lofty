/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015 Raffaello D. Di Napoli

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
#include <abaclade/collections.hxx>
#include <abaclade/collections/trie_ordered_multimap.hxx>
#include <abaclade/testing/test_case.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   collections_trie_ordered_multimap_bitwise_basic,
   "abc::collections::trie_ordered_multimap (bitwise) – basic operations"
) {
   ABC_TRACE_FUNC(this);

   collections::trie_ordered_multimap<int, int> tomm;

   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 0u);
   ABC_TESTING_ASSERT_TRUE(tomm.begin() == tomm.cend());
   ABC_TESTING_ASSERT_TRUE(tomm.cbegin() == tomm.end());
   ABC_TESTING_ASSERT_THROWS(collections::out_of_range, ++tomm.end());
   ABC_TESTING_ASSERT_THROWS(collections::out_of_range, tomm.cend()++);
   ABC_TESTING_ASSERT_THROWS(collections::out_of_range, *tomm.cbegin());
   ABC_TESTING_ASSERT_THROWS(collections::out_of_range, *tomm.cend());
   ABC_TESTING_ASSERT_THROWS(collections::bad_access, tomm.front());
   ABC_TESTING_ASSERT_THROWS(collections::out_of_range, tomm.pop(tomm.begin()));
   ABC_TESTING_ASSERT_THROWS(collections::out_of_range, tomm.remove(tomm.begin()));
   ABC_TESTING_ASSERT_THROWS(collections::bad_access, tomm.pop_front());

   auto it400(tomm.add(40, 400));
   // {40: 400}
   ABC_TESTING_ASSERT_EQUAL(it400->key, 40);
   ABC_TESTING_ASSERT_EQUAL(it400->value, 400);
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 40);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 400);

   auto it200(tomm.add(20, 200));
   // {20: 200}, {40: 400}
   ABC_TESTING_ASSERT_EQUAL(it200->key, 20);
   ABC_TESTING_ASSERT_EQUAL(it200->value, 200);
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 2u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 20);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 200);

   auto it500(tomm.add(50, 500));
   // {20: 200}, {40: 400}, {50: 500}
   ABC_TESTING_ASSERT_EQUAL(it500->key, 50);
   ABC_TESTING_ASSERT_EQUAL(it500->value, 500);
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 3u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 20);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 200);

   auto it300(tomm.add(30, 300));
   // {20: 200}, {30: 300}, {40: 400}, {50: 500}
   ABC_TESTING_ASSERT_EQUAL(it300->key, 30);
   ABC_TESTING_ASSERT_EQUAL(it300->value, 300);
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 4u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 20);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 200);

   auto it201(tomm.add(20, 201));
   // {20: 201, 200}, {30: 300}, {40: 400}, {50: 500}
   ABC_TESTING_ASSERT_EQUAL(it201->key, 20);
   ABC_TESTING_ASSERT_EQUAL(it201->value, 201);
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 5u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 20);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 200);

   auto it301(tomm.add(30, 301));
   // {20: 200, 201}, {30: 300, 301}, {40: 400}, {50: 500}
   ABC_TESTING_ASSERT_EQUAL(it301->key, 30);
   ABC_TESTING_ASSERT_EQUAL(it301->value, 301);
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 6u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 20);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 200);
   {
      static int const sc_aiExpectedKeys  [] = {  20,  20,  30,  30,  40,  50, -1 };
      static int const sc_aiExpectedValues[] = { 200, 201, 300, 301, 400, 500, -1 };
      int const * piExpectedKey = &sc_aiExpectedKeys[0];
      int const * piExpectedValue = &sc_aiExpectedValues[0];
      ABC_FOR_EACH(auto kv, tomm) {
         ABC_TESTING_ASSERT_EQUAL(kv.key, *piExpectedKey++);
         ABC_TESTING_ASSERT_EQUAL(kv.value, *piExpectedValue++);
      }
   }

   auto it300Found(tomm.find(30));
   ABC_TESTING_ASSERT_EQUAL(it300Found->key, 30);
   ABC_TESTING_ASSERT_EQUAL(it300Found->value, 300);
   ABC_TESTING_ASSERT_TRUE(it300Found == it300);

   auto kvp200(tomm.pop_front());
   // {20: 201}, {30: 300, 301}, {40: 400}, {50: 500}
   ABC_TESTING_ASSERT_EQUAL(kvp200.key, 20);
   ABC_TESTING_ASSERT_EQUAL(kvp200.value, 200);
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 5u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 20);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 201);

   tomm.remove(it301);
   // {20: 201}, {30: 300}, {40, 400}, {50: 500}
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 4u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 20);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 201);

   auto kvp201(tomm.pop_front());
   // {30: 300}, {40, 400}, {50: 500}
   ABC_TESTING_ASSERT_EQUAL(kvp201.key, 20);
   ABC_TESTING_ASSERT_EQUAL(kvp201.value, 201);
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 3u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 30);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 300);

   auto it101(tomm.add(10, 101));
   // {10: 101}, {30: 300}, {40, 400}, {50: 500}
   ABC_TESTING_ASSERT_EQUAL(it101->key, 10);
   ABC_TESTING_ASSERT_EQUAL(it101->value, 101);
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 4u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 10);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 101);

   auto kvp300(tomm.pop(it300));
   // {10: 101}, {40, 400}, {50: 500}
   ABC_TESTING_ASSERT_EQUAL(kvp300.key, 30);
   ABC_TESTING_ASSERT_EQUAL(kvp300.value, 300);
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 3u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 10);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 101);

   auto it302(tomm.add(30, 302));
   // {10: 101}, {30: 302} {40, 400}, {50: 500}
   ABC_TESTING_ASSERT_EQUAL(it302->key, 30);
   ABC_TESTING_ASSERT_EQUAL(it302->value, 302);
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 4u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 10);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 101);

   tomm.clear();
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 0u);

   auto it102(tomm.add(10, 102));
   // {10: 102}
   ABC_TESTING_ASSERT_EQUAL(it102->key, 10);
   ABC_TESTING_ASSERT_EQUAL(it102->value, 102);
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 10);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 102);

   auto it401(tomm.add(40, 401));
   // {10: 102}, {40: 401}
   ABC_TESTING_ASSERT_EQUAL(it401->key, 40);
   ABC_TESTING_ASSERT_EQUAL(it401->value, 401);
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 2u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 10);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 102);

   tomm.remove(it102);
   // {40, 401}
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 40);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 401);

   auto kvp401(tomm.pop(it401));
   // empty
   ABC_TESTING_ASSERT_EQUAL(kvp401.key, 40);
   ABC_TESTING_ASSERT_EQUAL(kvp401.value, 401);
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 0u);
   ABC_TESTING_ASSERT_THROWS(collections::bad_access, tomm.front());

   tomm.clear();
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 0u);
}

}} //namespace abc::test
