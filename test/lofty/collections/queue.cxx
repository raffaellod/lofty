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
#include <lofty/collections/queue.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/testing/utility.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   collections_queue_basic,
   "lofty::collections::queue – basic operations"
) {
   LOFTY_TRACE_FUNC();

   collections::queue<int> q;

   LOFTY_TESTING_ASSERT_FALSE(q);
   LOFTY_TESTING_ASSERT_EQUAL(q.size(), 0u);

   q.push_back(10);
   LOFTY_TESTING_ASSERT_TRUE(q);
   LOFTY_TESTING_ASSERT_EQUAL(q.size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(q.front(), 10);
   LOFTY_TESTING_ASSERT_EQUAL(q.back(), 10);

   q.push_back(20);
   LOFTY_TESTING_ASSERT_TRUE(q);
   LOFTY_TESTING_ASSERT_EQUAL(q.size(), 2u);
   LOFTY_TESTING_ASSERT_EQUAL(q.front(), 10);
   LOFTY_TESTING_ASSERT_EQUAL(q.back(), 20);

   LOFTY_TESTING_ASSERT_EQUAL(q.pop_front(), 10);
   LOFTY_TESTING_ASSERT_TRUE(q);
   LOFTY_TESTING_ASSERT_EQUAL(q.size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(q.front(), 20);
   LOFTY_TESTING_ASSERT_EQUAL(q.back(), 20);

   LOFTY_TESTING_ASSERT_EQUAL(q.pop_front(), 20);
   LOFTY_TESTING_ASSERT_FALSE(q);
   LOFTY_TESTING_ASSERT_EQUAL(q.size(), 0u);

   q.push_back(30);
   LOFTY_TESTING_ASSERT_TRUE(q);
   LOFTY_TESTING_ASSERT_EQUAL(q.size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(q.front(), 30);
   LOFTY_TESTING_ASSERT_EQUAL(q.back(), 30);

   q.clear();
   LOFTY_TESTING_ASSERT_FALSE(q);
   LOFTY_TESTING_ASSERT_EQUAL(q.size(), 0u);
}

}} //namespace lofty::test
