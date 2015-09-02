/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013, 2014, 2015
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
#include <algorithm>
#include <abaclade/testing/utility.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   collections_vector_basic,
   "abc::collections::vector – basic operations"
) {
   ABC_TRACE_FUNC(this);

   collections::vector<int> v;

   /* Note: do not replace the item-by-item assertions with comparisons against manually-populated
   vectors as here we’re also guaranteeing that we can prepare a manually-populated vector. For
   example:

      collections::vector<int> v1, v2;
      v1.push_back(1);
      v1.push_back(2);
      v2.push_back(1);
      v2.push_back(1);
      ABC_TESTING_ASSERT_EQUAL(v1, v2);

   The assertion above will succeed if any of these error conditions is true:
   •  vector<int>::operator==() always returns true;
   •  vector<int>::push_back() never appends any elements;
   •  vector<int>::push_back() always appends more elements than it should. */

   ABC_TESTING_ASSERT_EQUAL(v.size(), 0u);
   ABC_TESTING_ASSERT_THROWS(exception, v.front());
   ABC_TESTING_ASSERT_THROWS(exception, v.back());
   ABC_TESTING_ASSERT_THROWS(index_error, v[0]);

   v.push_back(1);
   ABC_TESTING_ASSERT_EQUAL(v.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(&v.front(), v.data());
   ABC_TESTING_ASSERT_EQUAL(v.front(), 1);
   ABC_TESTING_ASSERT_EQUAL(v.back(), 1);
   ABC_TESTING_ASSERT_EQUAL(v[0], 1);

   v = v + v;
   ABC_TESTING_ASSERT_EQUAL(v.size(), 2u);
   ABC_TESTING_ASSERT_EQUAL(v[0], 1);
   ABC_TESTING_ASSERT_EQUAL(v[1], 1);

   v.insert(1, 2);
   ABC_TESTING_ASSERT_EQUAL(v.size(), 3u);
   ABC_TESTING_ASSERT_EQUAL(v[0], 1);
   ABC_TESTING_ASSERT_EQUAL(v[1], 2);
   ABC_TESTING_ASSERT_EQUAL(v[2], 1);

   v = v.slice(1, 3);
   ABC_TESTING_ASSERT_EQUAL(v.size(), 2u);
   ABC_TESTING_ASSERT_EQUAL(v[0], 2);
   ABC_TESTING_ASSERT_EQUAL(v[1], 1);

   v.push_back(3);
   ABC_TESTING_ASSERT_EQUAL(v.size(), 3u);
   ABC_TESTING_ASSERT_EQUAL(v[0], 2);
   ABC_TESTING_ASSERT_EQUAL(v[1], 1);
   ABC_TESTING_ASSERT_EQUAL(v[2], 3);

   v.remove_at(1);
   ABC_TESTING_ASSERT_EQUAL(v.size(), 2u);
   ABC_TESTING_ASSERT_EQUAL(&v.front(), v.data());
   ABC_TESTING_ASSERT_EQUAL(v.front(), 2);
   ABC_TESTING_ASSERT_EQUAL(v.back(), 3);
   ABC_TESTING_ASSERT_EQUAL(v[0], 2);
   ABC_TESTING_ASSERT_EQUAL(v[1], 3);

   int i3 = v.pop_back();
   ABC_TESTING_ASSERT_EQUAL(v.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(v.front(), 2);
   ABC_TESTING_ASSERT_EQUAL(v.back(), 2);
   ABC_TESTING_ASSERT_EQUAL(v[0], 2);
   ABC_TESTING_ASSERT_EQUAL(i3, 3);

   v.clear();
   ABC_TESTING_ASSERT_EQUAL(v.size(), 0u);
   ABC_TESTING_ASSERT_THROWS(exception, v.front());
   ABC_TESTING_ASSERT_THROWS(exception, v.back());
   ABC_TESTING_ASSERT_THROWS(index_error, v[0]);
   ABC_TESTING_ASSERT_THROWS(exception, v.pop_back());
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   collections_vector_relational_operators,
   "abc::collections::vector – relational operators"
) {
   ABC_TRACE_FUNC(this);

   collections::vector<int> v1a, v1b, v2, v3;
   v1a.push_back(1);
   v1a.push_back(2);
   v1b.push_back(1);
   v1b.push_back(2);
   v2.push_back(2);
   v2.push_back(3);
   v3.push_back(1);

   ABC_TESTING_ASSERT_EQUAL(v1a, v1a);
   ABC_TESTING_ASSERT_EQUAL(v1a, v1b);
   ABC_TESTING_ASSERT_NOT_EQUAL(v1a, v2);
   ABC_TESTING_ASSERT_NOT_EQUAL(v1a, v3);
   ABC_TESTING_ASSERT_EQUAL(v1b, v1a);
   ABC_TESTING_ASSERT_EQUAL(v1b, v1b);
   ABC_TESTING_ASSERT_NOT_EQUAL(v1b, v2);
   ABC_TESTING_ASSERT_NOT_EQUAL(v1b, v3);
   ABC_TESTING_ASSERT_NOT_EQUAL(v2, v1a);
   ABC_TESTING_ASSERT_NOT_EQUAL(v2, v1b);
   ABC_TESTING_ASSERT_EQUAL(v2, v2);
   ABC_TESTING_ASSERT_NOT_EQUAL(v2, v3);
   ABC_TESTING_ASSERT_NOT_EQUAL(v3, v1a);
   ABC_TESTING_ASSERT_NOT_EQUAL(v3, v1b);
   ABC_TESTING_ASSERT_NOT_EQUAL(v3, v2);
   ABC_TESTING_ASSERT_EQUAL(v3, v3);
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   collections_vector_iterators,
   "abc::collections::vector – operations with iterators"
) {
   ABC_TRACE_FUNC(this);

   collections::vector<int> v;
   v.push_back(1);
   v.push_back(2);
   v.push_back(3);

   // Remove an element by iterator.
   v.remove_at(std::find(v.cbegin(), v.cend(), 2));
   ABC_TESTING_ASSERT_EQUAL(v.size(), 2u);
   ABC_TESTING_ASSERT_EQUAL(v[0], 1);
   ABC_TESTING_ASSERT_EQUAL(v[1], 3);

   // Remove an element with an invalid iterator.
   ABC_TESTING_ASSERT_THROWS(index_error, v.remove_at(v.begin() - 1));
   ABC_TESTING_ASSERT_THROWS(index_error, v.remove_at(v.end()));
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   collections_vector_trivial_removal,
   "abc::collections::vector – removal of trivial elements"
) {
   ABC_TRACE_FUNC(this);

   collections::vector<int> v, vZero, vOne, vTwo, vOneTwo;
   vOne.push_back(1);
   vTwo.push_back(2);
   vOneTwo.push_back(1);
   vOneTwo.push_back(2);

   v = vZero;

   // Remove from empty vector by index.
   ABC_TESTING_ASSERT_THROWS(index_error, v.remove_at(-1));
   ABC_TESTING_ASSERT_THROWS(index_error, v.remove_at(0));
   ABC_TESTING_ASSERT_THROWS(index_error, v.remove_at(1));

   v = vOneTwo;

   // Remove from 2-element vector by index.
   ABC_TESTING_ASSERT_THROWS(index_error, v.remove_at(-3));
   ABC_TESTING_ASSERT_EQUAL((v.remove_at(-2), v), vTwo);
   v = vOneTwo;
   ABC_TESTING_ASSERT_EQUAL((v.remove_at(-1), v), vOne);
   v = vOneTwo;
   ABC_TESTING_ASSERT_EQUAL((v.remove_at(0), v), vTwo);
   v = vOneTwo;
   ABC_TESTING_ASSERT_EQUAL((v.remove_at(1), v), vOne);
   v = vOneTwo;
   ABC_TESTING_ASSERT_THROWS(index_error, v.remove_at(2));
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   collections_vector_memory,
   "abc::collections::vector – memory management"
) {
   ABC_TRACE_FUNC(this);

   using testing::utility::make_container_data_ptr_tracker;

   collections::vector<int> v1;
   auto cdpt1(make_container_data_ptr_tracker(&v1));
   // Note: the embedded item array size will probably be > 2.
   collections::vector<int, 2> v2;
   auto cdpt2(make_container_data_ptr_tracker(&v2));
   // Note: the embedded item array size will probably be > 10.
   collections::vector<int, 10> v3;
   auto cdpt3(make_container_data_ptr_tracker(&v3));

   /* Add one element to each vector, so they all allocate a new item array or begin using their own
   embedded one. */

   // Should allocate a new item array.
   v1.push_back(10);
   ABC_TESTING_ASSERT_TRUE(cdpt1.changed());
   ABC_TESTING_ASSERT_EQUAL(v1.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(v1[0], 10);

   // Should begin using the embedded item array.
   v2.push_back(20);
   ABC_TESTING_ASSERT_TRUE(cdpt2.changed());
   ABC_TESTING_ASSERT_EQUAL(v2.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(v2[0], 20);
   int const * const p2Static(v2.data());

   // Should begin using the embedded item array.
   v3.push_back(30);
   ABC_TESTING_ASSERT_TRUE(cdpt3.changed());
   ABC_TESTING_ASSERT_EQUAL(v3.size(), 1u);
   ABC_TESTING_ASSERT_EQUAL(v3[0], 30);
   int const * const p3Static(v3.data());

   // Add more elements to each vector.

   // These are too many for the newly-allocated item array, so a new one should be allocated.
   v1.push_back(11);
   v1.push_back(12);
   v1.push_back(13);
   v1.push_back(14);
   v1.push_back(15);
   v1.push_back(16);
   v1.push_back(17);
   v1.push_back(18);
   v1.push_back(19);
   // Cannot ASSERT_TRUE on this change, because the item array may be resized in place.
   cdpt1.changed();
   ABC_TESTING_ASSERT_EQUAL(v1.size(), 10u);
   ABC_TESTING_ASSERT_EQUAL(v1[0], 10);
   ABC_TESTING_ASSERT_EQUAL(v1[1], 11);
   ABC_TESTING_ASSERT_EQUAL(v1[2], 12);
   ABC_TESTING_ASSERT_EQUAL(v1[3], 13);
   ABC_TESTING_ASSERT_EQUAL(v1[4], 14);
   ABC_TESTING_ASSERT_EQUAL(v1[5], 15);
   ABC_TESTING_ASSERT_EQUAL(v1[6], 16);
   ABC_TESTING_ASSERT_EQUAL(v1[7], 17);
   ABC_TESTING_ASSERT_EQUAL(v1[8], 18);
   ABC_TESTING_ASSERT_EQUAL(v1[9], 19);

   // These are too many for the embedded item array, so a new item array should be allocated.
   v2.push_back(21);
   v2.push_back(22);
   v2.push_back(23);
   v2.push_back(24);
   v2.push_back(25);
   v2.push_back(26);
   v2.push_back(27);
   v2.push_back(28);
   v2.push_back(29);
   ABC_TESTING_ASSERT_TRUE(cdpt2.changed());
   ABC_TESTING_ASSERT_EQUAL(v2.size(), 10u);
   ABC_TESTING_ASSERT_EQUAL(v2[0], 20);
   ABC_TESTING_ASSERT_EQUAL(v2[1], 21);
   ABC_TESTING_ASSERT_EQUAL(v2[2], 22);
   ABC_TESTING_ASSERT_EQUAL(v2[3], 23);
   ABC_TESTING_ASSERT_EQUAL(v2[4], 24);
   ABC_TESTING_ASSERT_EQUAL(v2[5], 25);
   ABC_TESTING_ASSERT_EQUAL(v2[6], 26);
   ABC_TESTING_ASSERT_EQUAL(v2[7], 27);
   ABC_TESTING_ASSERT_EQUAL(v2[8], 28);
   ABC_TESTING_ASSERT_EQUAL(v2[9], 29);

   // The embedded item array has room for this, so no reallocation is needed.
   v3.push_back(31);
   ABC_TESTING_ASSERT_EQUAL(v3.data(), p3Static);
   ABC_TESTING_ASSERT_FALSE(cdpt3.changed());
   ABC_TESTING_ASSERT_EQUAL(v3.size(), 2u);
   ABC_TESTING_ASSERT_EQUAL(v3[0], 30);
   ABC_TESTING_ASSERT_EQUAL(v3[1], 31);

   // Check assignment from larger to smaller embedded vectors.

   // Should keep the current item array, copying v2’s items over.
   v1 = v2.vector0();
   ABC_TESTING_ASSERT_FALSE(cdpt1.changed());
   ABC_TESTING_ASSERT_EQUAL(v1.size(), 10u);
   ABC_TESTING_ASSERT_EQUAL(v1[0], 20);
   ABC_TESTING_ASSERT_EQUAL(v1[1], 21);
   ABC_TESTING_ASSERT_EQUAL(v1[2], 22);
   ABC_TESTING_ASSERT_EQUAL(v1[3], 23);
   ABC_TESTING_ASSERT_EQUAL(v1[4], 24);
   ABC_TESTING_ASSERT_EQUAL(v1[5], 25);
   ABC_TESTING_ASSERT_EQUAL(v1[6], 26);
   ABC_TESTING_ASSERT_EQUAL(v1[7], 27);
   ABC_TESTING_ASSERT_EQUAL(v1[8], 28);
   ABC_TESTING_ASSERT_EQUAL(v1[9], 29);

   // Should return to using the embedded item array, copying v3’s items over.
   v2 = v3.vector0();
   ABC_TESTING_ASSERT_EQUAL(v2.data(), p2Static);
   ABC_TESTING_ASSERT_TRUE(cdpt2.changed());
   ABC_TESTING_ASSERT_EQUAL(v2.size(), 2u);
   ABC_TESTING_ASSERT_EQUAL(v2[0], 30);
   ABC_TESTING_ASSERT_EQUAL(v2[1], 31);
   // “Rebrand” the items as 2x.
   v2[0] = 20;
   v2[1] = 21;

   /* The current item array should still be large enough, but this should drop it to use the
   temporary one created by operator+(). */
   v1 = v2 + v3;
   ABC_TESTING_ASSERT_TRUE(cdpt1.changed());
   ABC_TESTING_ASSERT_EQUAL(v1.size(), 4u);
   ABC_TESTING_ASSERT_EQUAL(v1[0], 20);
   ABC_TESTING_ASSERT_EQUAL(v1[1], 21);
   ABC_TESTING_ASSERT_EQUAL(v1[2], 30);
   ABC_TESTING_ASSERT_EQUAL(v1[3], 31);
   // “Rebrand” the items as 1x.
   v1[0] = 10;
   v1[1] = 11;
   v1[2] = 12;
   v1[3] = 13;

   // This should be too much for the embedded item array, so a new one should be allocated.
   v3 += v1 + v2 + v1 + v3 + v1;
   ABC_TESTING_ASSERT_TRUE(cdpt3.changed());
   ABC_TESTING_ASSERT_EQUAL(v3.size(), 18u);
   ABC_TESTING_ASSERT_EQUAL(v3[0], 30);
   ABC_TESTING_ASSERT_EQUAL(v3[1], 31);
   ABC_TESTING_ASSERT_EQUAL(v3[2], 10);
   ABC_TESTING_ASSERT_EQUAL(v3[3], 11);
   ABC_TESTING_ASSERT_EQUAL(v3[4], 12);
   ABC_TESTING_ASSERT_EQUAL(v3[5], 13);
   ABC_TESTING_ASSERT_EQUAL(v3[6], 20);
   ABC_TESTING_ASSERT_EQUAL(v3[7], 21);
   ABC_TESTING_ASSERT_EQUAL(v3[8], 10);
   ABC_TESTING_ASSERT_EQUAL(v3[9], 11);
   ABC_TESTING_ASSERT_EQUAL(v3[10], 12);
   ABC_TESTING_ASSERT_EQUAL(v3[11], 13);
   ABC_TESTING_ASSERT_EQUAL(v3[12], 30);
   ABC_TESTING_ASSERT_EQUAL(v3[13], 31);
   ABC_TESTING_ASSERT_EQUAL(v3[14], 10);
   ABC_TESTING_ASSERT_EQUAL(v3[15], 11);
   ABC_TESTING_ASSERT_EQUAL(v3[16], 12);
   ABC_TESTING_ASSERT_EQUAL(v3[17], 13);

   // Ensure that the vector doesn’t automatically shrink to fit when downsized.
   std::size_t iHighestCapacity = v3.capacity();
   v3.set_size(0);
   ABC_TESTING_ASSERT_EQUAL(v3.size(), 0u);
   ABC_TESTING_ASSERT_EQUAL(v3.capacity(), iHighestCapacity);
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

/*! Instantiates and returns a dynamic vector. The vector will contain one item, added in a way that
should cause only one new instance of instances_counter to be created, one moved and none copied.
Additional copies/moved may occur upon return.

return
   Newly-instantiated dynamic vector.
*/
static collections::vector<testing::utility::instances_counter> return_vector() {
   ABC_TRACE_FUNC();

   collections::vector<testing::utility::instances_counter> v;
   // New instance, immediately moved.
   v.push_back(testing::utility::instances_counter());
   /* This will move the item array or the items in it, depending on the destination type (embedded
   or dynamic item array). */
   return _std::move(v);
}

ABC_TESTING_TEST_CASE_FUNC(
   collections_vector_movement,
   "abc::collections::vector – item and item array movement"
) {
   ABC_TRACE_FUNC(this);

   typedef testing::utility::instances_counter instances_counter;
   {
      /* This will move the item array from the returned vector to v1, so no item copies or moves
      will occur other than the ones in return_vector(). */
      collections::vector<instances_counter> v(return_vector());
      ABC_TESTING_ASSERT_EQUAL(instances_counter::new_insts(), 1u);
      ABC_TESTING_ASSERT_EQUAL(instances_counter::moves(), 1u);
      ABC_TESTING_ASSERT_EQUAL(instances_counter::copies(), 0u);
      instances_counter::reset_counts();

      /* This should create a new copy, with no intermediate moves because all passages are by
      reference or pointer. */
      v.push_back(v[0]);
      ABC_TESTING_ASSERT_EQUAL(instances_counter::new_insts(), 0u);
      ABC_TESTING_ASSERT_EQUAL(instances_counter::moves(), 0u);
      ABC_TESTING_ASSERT_EQUAL(instances_counter::copies(), 1u);
      instances_counter::reset_counts();
   }

   {
      collections::vector<instances_counter, 9> v;
      /* This will move the individual items from the returned vector to v2’s embedded item array.
      Can’t just construct v2 with return_vector() because v2 would merely use that item array
      instead of its own embedded one, resulting in no additional moves other than the one in
      return_vector(). */
      v += return_vector();
      ABC_TESTING_ASSERT_EQUAL(instances_counter::new_insts(), 1u);
      ABC_TESTING_ASSERT_EQUAL(instances_counter::moves(), 2u);
      ABC_TESTING_ASSERT_EQUAL(instances_counter::copies(), 0u);
      instances_counter::reset_counts();
   }
}

}} //namespace abc::test
