/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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
#include <abaclade/perf/stopwatch.hxx>
#include <abaclade/map.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::map_basic

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC(map_basic, "abc::map – basic operations") {
   ABC_TRACE_FUNC(this);

   map<int, int> m;

   ABC_TESTING_ASSERT_EQUAL(m.size(), 0u);

   m.add(10, 100);
   ABC_TESTING_ASSERT_EQUAL(m.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(m[10], 100);

   m.add(20, 200);
   ABC_TESTING_ASSERT_EQUAL(m.size(), 2u);
   ABC_TESTING_ASSERT_EQUAL(m[10], 100);
   ABC_TESTING_ASSERT_EQUAL(m[20], 200);

   m.remove(10);
   ABC_TESTING_ASSERT_EQUAL(m.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(m[20], 200);

   m.add(22, 220);
   ABC_TESTING_ASSERT_EQUAL(m.size(), 2u);
   ABC_TESTING_ASSERT_EQUAL(m[20], 200);
   ABC_TESTING_ASSERT_EQUAL(m[22], 220);

   m.clear();
   ABC_TESTING_ASSERT_EQUAL(m.size(), 0u);

   m.add(11, 110);
   ABC_TESTING_ASSERT_EQUAL(m.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(m[11], 110);

   // Add enough key/value pairs until a resize occurs.
   perf::stopwatch sw;
   sw.start();
   int iKey = 11, iValue = 110;
   std::size_t iInitialCapacity = m.capacity();
   do {
      iKey += 11;
      iValue += 110;
      m.add(iKey, iValue);
   } while (m.capacity() == iInitialCapacity);
   log_duration(sw.stop());
   /* Verify that some values are still there. Can’t check them all because we don’t know exactly
   how many we ended up adding. */
   ABC_TESTING_ASSERT_EQUAL(m[11], 110);
   ABC_TESTING_ASSERT_EQUAL(m[22], 220);
   ABC_TESTING_ASSERT_EQUAL(m[iKey - 11], iValue - 110);
   ABC_TESTING_ASSERT_EQUAL(m[iKey], iValue);
}

} //namespace test
} //namespace abc
