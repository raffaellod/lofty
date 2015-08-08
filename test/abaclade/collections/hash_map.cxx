/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014, 2015
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
#include <abaclade/collections/hash_map.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   collections_hash_map_basic,
   "abc::collections::hash_map – basic operations"
) {
   ABC_TRACE_FUNC(this);

   collections::hash_map<int, int> hm;

   ABC_TESTING_ASSERT_EQUAL(hm.size(), 0u);
   // These assertions target const begin/end.
   ABC_TESTING_ASSERT_TRUE(hm.cbegin() == hm.cend());

   hm.add_or_assign(10, 100);
   ABC_TESTING_ASSERT_EQUAL(hm.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(hm[10], 100);
   {
      /* This uses begin(), not cbegin(), so we can test equality comparison between const/non-const
      iterators. */
      auto it(hm.begin());
      ABC_TESTING_ASSERT_EQUAL(it->key, 10);
      ABC_TESTING_ASSERT_EQUAL(it->value, 100);
      ++it;
      ABC_TESTING_ASSERT_TRUE(it == hm.cend());
   }

   hm.add_or_assign(20, 200);
   ABC_TESTING_ASSERT_EQUAL(hm.size(), 2u);
   ABC_TESTING_ASSERT_EQUAL(hm[10], 100);
   ABC_TESTING_ASSERT_EQUAL(hm[20], 200);

   ABC_TESTING_ASSERT_TRUE(hm.remove_if_found(10));
   ABC_TESTING_ASSERT_FALSE(hm.remove_if_found(10));
   ABC_TESTING_ASSERT_THROWS(key_error, hm.remove(10));
   ABC_TESTING_ASSERT_EQUAL(hm.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(hm[20], 200);
   ABC_TESTING_ASSERT_FALSE(hm.remove_if_found(10));

   hm.add_or_assign(22, 220);
   ABC_TESTING_ASSERT_EQUAL(hm.size(), 2u);
   ABC_TESTING_ASSERT_EQUAL(hm[20], 200);
   ABC_TESTING_ASSERT_EQUAL(hm[22], 220);
   {
      // A little clunky, but neecessary since the order is not guaranteed.
      bool bFound20 = false, bFound22 = false;
      for (auto it(hm.begin()); it != hm.cend(); ++it) {
         ABC_TESTING_ASSERT_TRUE(it->key == 20 || it->key == 22);
         if (it->key == 20) {
            ABC_TESTING_ASSERT_FALSE(bFound20);
            ABC_TESTING_ASSERT_EQUAL(it->value, 200);
            bFound20 = true;
         } else if (it->key == 22) {
            ABC_TESTING_ASSERT_FALSE(bFound22);
            ABC_TESTING_ASSERT_EQUAL(it->value, 220);
            bFound22 = true;
         }
      }
      ABC_TESTING_ASSERT_TRUE(bFound20);
      ABC_TESTING_ASSERT_TRUE(bFound22);
   }

   hm.clear();
   ABC_TESTING_ASSERT_EQUAL(hm.size(), 0u);
   // These assertions target non-const begin/end.
   ABC_TESTING_ASSERT_TRUE(hm.begin() == hm.end());

   hm.add_or_assign(11, 110);
   ABC_TESTING_ASSERT_EQUAL(hm.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(hm[11], 110);

   // Add enough key/value pairs until a resize occurs.
   int iKey = 11, iValue = 110;
   std::size_t iInitialCapacity = hm.capacity();
   do {
      iKey += 11;
      iValue += 110;
      hm.add_or_assign(iKey, iValue);
   } while (hm.capacity() == iInitialCapacity);
   /* Verify that some values are still there. Can’t check them all because we don’t know exactly
   how many we ended up adding. */
   ABC_TESTING_ASSERT_EQUAL(hm[11], 110);
   ABC_TESTING_ASSERT_EQUAL(hm[22], 220);
   ABC_TESTING_ASSERT_EQUAL(hm[iKey - 11], iValue - 110);
   ABC_TESTING_ASSERT_EQUAL(hm[iKey], iValue);

   // Validate that non-copyable types can be stored in a map.
   {
      collections::hash_map<int, std::unique_ptr<int>> hm2;
      hm2.add_or_assign(1, std::unique_ptr<int>(new int(10)));
   }
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

namespace {

/*! Inefficient hash function that results in 100% hash collisions. This also checks that hash 0
(which has a special meaning internally to abc::collections::hash_map) behaves no differently than
any other value.

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

ABC_TESTING_TEST_CASE_FUNC(
   collections_hash_map_collisions_stress,
   "abc::collections::hash_map – stress test with 100% collisions"
) {
   ABC_TRACE_FUNC(this);

   static int const sc_iMax = 1000;
   unsigned cErrors;
   collections::hash_map<int, int, poor_hash> hm;

   // Verify that values are inserted correctly.
   cErrors = 0;
   for (int i = 0; i < sc_iMax; ++i) {
      hm.add_or_assign(i, i);
      if (hm[i] != i) {
         ++cErrors;
      }
   }
   ABC_TESTING_ASSERT_EQUAL(cErrors, 0u);

   // Verify that the insertion of later values did not break previously-inserted values.
   cErrors = 0;
   for (int i = 0; i < sc_iMax; ++i) {
      if (hm[i] != i) {
         ++cErrors;
      }
   }
   ABC_TESTING_ASSERT_EQUAL(cErrors, 0u);
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   collections_hash_map_iterators,
   "abc::collections::hash_map – operations with iterators"
) {
   ABC_TRACE_FUNC(this);

   collections::hash_map<int, int> hm;

   // Should not allow to move an iterator to outside [begin, end].
   ABC_TESTING_ASSERT_DOES_NOT_THROW(hm.cbegin());
   ABC_TESTING_ASSERT_DOES_NOT_THROW(hm.cend());
   ABC_TESTING_ASSERT_THROWS(iterator_error, ++hm.cbegin());
   ABC_TESTING_ASSERT_THROWS(iterator_error, ++hm.cend());

   // Should not allow to dereference end().
   ABC_TESTING_ASSERT_THROWS(iterator_error, *hm.cend());

   {
      auto it(hm.cbegin());
      hm.add_or_assign(10, 100);
      // it has been invalidated by add_or_assign().
      ABC_TESTING_ASSERT_THROWS(iterator_error, *it);
   }

   ABC_FOR_EACH(auto kv, hm) {
      ABC_TESTING_ASSERT_EQUAL(kv.key, 10);
      ABC_TESTING_ASSERT_EQUAL(kv.value, 100);
   }

   {
      auto it(hm.cbegin());
      hm.remove(10);
      // it has been invalidated by remove().
      ABC_TESTING_ASSERT_THROWS(iterator_error, *it);
   }
}

}} //namespace abc::test
