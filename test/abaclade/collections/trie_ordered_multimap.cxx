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
#include <abaclade/collections/trie_ordered_multimap.hxx>
#include <abaclade/testing/test_case.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::collections::trie_ordered_multimap (scalar) – basic operations") {
   ABC_TRACE_FUNC(this);

   collections::trie_ordered_multimap<unsigned, int> tomm;

   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 0u);

   auto it200(tomm.add(20, 200));
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 20);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 200);

   tomm.add(30, 300);
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 2u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 20);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 200);

   tomm.add(20, 222);
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 3u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 20);
   // 222 was inserted after 200, so front() should still return the 20/200 pair.
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 200);

   tomm.remove(it200);
   ABC_TESTING_ASSERT_EQUAL(tomm.size(), 2u);
   ABC_TESTING_ASSERT_EQUAL(tomm.front().key, 20);
   // Now that 200 is gone, front() should return the 20/222 pair.
   ABC_TESTING_ASSERT_EQUAL(tomm.front().value, 222);
}

}} //namespace abc::test
