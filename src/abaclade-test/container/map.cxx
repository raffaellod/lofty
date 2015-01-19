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
#include <abaclade/container/map.hxx>


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
   int iKey = 11, iValue = 110;
   std::size_t iInitialCapacity = m.capacity();
   do {
      iKey += 11;
      iValue += 110;
      m.add(iKey, iValue);
   } while (m.capacity() == iInitialCapacity);
   /* Verify that some values are still there. Can’t check them all because we don’t know exactly
   how many we ended up adding. */
   ABC_TESTING_ASSERT_EQUAL(m[11], 110);
   ABC_TESTING_ASSERT_EQUAL(m[22], 220);
   ABC_TESTING_ASSERT_EQUAL(m[iKey - 11], iValue - 110);
   ABC_TESTING_ASSERT_EQUAL(m[iKey], iValue);
}

} //namespace test
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::map_collisions

namespace abc {
namespace test {

namespace {

/*! Inefficient hash function that results in 100% hash collisions. This also checks that hash 0
(which has a special meaning internally to abc::map) behaves no differently than any other value.

@param i
   Value to hash.
@return
   Hash of i.
*/
struct poor_hash {
   std::size_t operator()(int i) const {
      ABC_UNUSED_ARG(i);
      return 0;
   }
};

} //namespace

ABC_TESTING_TEST_CASE_FUNC(map_collisions, "abc::map – stress test with 100% collisions") {
   ABC_TRACE_FUNC(this);

   static int const sc_iMax = 1000;
   unsigned cErrors;
   map<int, int, poor_hash> m;

   // Verify that values are inserted correctly.
   cErrors = 0;
   for (int i = 0; i < sc_iMax; ++i) {
      m.add(i, i);
      if (m[i] != i) {
         ++cErrors;
      }
   }
   ABC_TESTING_ASSERT_EQUAL(cErrors, 0u);

   // Verify that the insertion of later values did not break previously-inserted values.
   cErrors = 0;
   for (int i = 0; i < sc_iMax; ++i) {
      if (m[i] != i) {
         ++cErrors;
      }
   }
   ABC_TESTING_ASSERT_EQUAL(cErrors, 0u);
}

} //namespace test
} //namespace abc
