/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/collections/queue.hxx>
#include <lofty/logging.hxx>
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

   ASSERT(!q);
   ASSERT(q.size() == 0u);

   q.push_back(10);
   ASSERT(!!q);
   ASSERT(q.size() == 1u);
   ASSERT(q.front() == 10);
   ASSERT(q.back() == 10);

   q.push_back(20);
   ASSERT(!!q);
   ASSERT(q.size() == 2u);
   ASSERT(q.front() == 10);
   ASSERT(q.back() == 20);

   ASSERT(q.pop_front() == 10);
   ASSERT(!!q);
   ASSERT(q.size() == 1u);
   ASSERT(q.front() == 20);
   ASSERT(q.back() == 20);

   ASSERT(q.pop_front() == 20);
   ASSERT(!q);
   ASSERT(q.size() == 0u);

   q.push_back(30);
   ASSERT(!!q);
   ASSERT(q.size() == 1u);
   ASSERT(q.front() == 30);
   ASSERT(q.back() == 30);

   q.clear();
   ASSERT(!q);
   ASSERT(q.size() == 0u);
}

}} //namespace lofty::test
