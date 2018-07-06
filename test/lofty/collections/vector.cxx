/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/collections.hxx>
#include <lofty/from_str.hxx>
#include <lofty/logging.hxx>
#include <lofty/testing/test_case.hxx>
#include <algorithm>
#include <lofty/testing/utility.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   collections_vector_basic,
   "lofty::collections::vector – basic operations"
) {
   LOFTY_TRACE_FUNC();

   collections::vector<int> v;

   /* Note: do not replace the item-by-item assertions with comparisons against manually-populated vectors as
   here we’re also guaranteeing that we can prepare a manually-populated vector. For example:

      collections::vector<int> v1, v2;
      v1.push_back(1);
      v1.push_back(2);
      v2.push_back(1);
      v2.push_back(1);
      ASSERT(v1 == v2);

   The assertion above will succeed if any of these error conditions is true:
   •  vector<int>::operator==() always returns true;
   •  vector<int>::push_back() never appends any elements;
   •  vector<int>::push_back() always appends more elements than it should. */

   ASSERT(v.size() == 0u);
   ASSERT_THROWS(collections::bad_access, v.front());
   ASSERT_THROWS(collections::bad_access, v.back());
   ASSERT_THROWS(collections::out_of_range, v[0]);
   ASSERT(v.find(1) == v.cend());

   v.push_back(1);
   ASSERT(v.size() == 1u);
   ASSERT(&v.front() == v.data());
   ASSERT(v.front() == 1);
   ASSERT(v.back() == 1);
   ASSERT(v[0] == 1);
   ASSERT(v.find(1) == v.cbegin());

   v = v + v;
   ASSERT(v.size() == 2u);
   ASSERT(v[0] == 1);
   ASSERT(v[1] == 1);
   ASSERT(v.find(1) == v.cbegin());

   v.insert(v.cbegin() + 1, 2);
   ASSERT(v.size() == 3u);
   ASSERT(v[0] == 1);
   ASSERT(v[1] == 2);
   ASSERT(v[2] == 1);
   ASSERT(v.find(1) == v.cbegin());

   v = v.slice(v.cbegin() + 1, v.cbegin() + 3);
   ASSERT(v.size() == 2u);
   ASSERT(v[0] == 2);
   ASSERT(v[1] == 1);
   ASSERT(v.find(1) == v.cbegin() + 1);

   v.push_back(3);
   ASSERT(v.size() == 3u);
   ASSERT(v[0] == 2);
   ASSERT(v[1] == 1);
   ASSERT(v[2] == 3);
   ASSERT(v.find(1) == v.cbegin() + 1);

   v.remove_at(v.cbegin() + 1);
   ASSERT(v.size() == 2u);
   ASSERT(&v.front() == v.data());
   ASSERT(v.front() == 2);
   ASSERT(v.back() == 3);
   ASSERT(v[0] == 2);
   ASSERT(v[1] == 3);
   ASSERT(v.find(1) == v.cend());

   int i3 = v.pop_back();
   ASSERT(v.size() == 1u);
   ASSERT(v.front() == 2);
   ASSERT(v.back() == 2);
   ASSERT(v[0] == 2);
   ASSERT(i3 == 3);
   ASSERT(v.find(1) == v.cend());

   v.clear();
   ASSERT(v.size() == 0u);
   ASSERT_THROWS(collections::bad_access, v.front());
   ASSERT_THROWS(collections::bad_access, v.back());
   ASSERT_THROWS(collections::out_of_range, v[0]);
   ASSERT_THROWS(collections::bad_access, v.pop_back());
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   collections_vector_relational_operators,
   "lofty::collections::vector – relational operators"
) {
   LOFTY_TRACE_FUNC();

   collections::vector<int> v1a, v1b, v2, v3;
   v1a.push_back(1);
   v1a.push_back(2);
   v1b.push_back(1);
   v1b.push_back(2);
   v2.push_back(2);
   v2.push_back(3);
   v3.push_back(1);

   ASSERT(v1a == v1a);
   ASSERT(v1a == v1b);
   ASSERT(v1a != v2);
   ASSERT(v1a != v3);
   ASSERT(v1b == v1a);
   ASSERT(v1b == v1b);
   ASSERT(v1b != v2);
   ASSERT(v1b != v3);
   ASSERT(v2 != v1a);
   ASSERT(v2 != v1b);
   ASSERT(v2 == v2);
   ASSERT(v2 != v3);
   ASSERT(v3 != v1a);
   ASSERT(v3 != v1b);
   ASSERT(v3 != v2);
   ASSERT(v3 == v3);
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   collections_vector_iterators,
   "lofty::collections::vector – operations with iterators"
) {
   LOFTY_TRACE_FUNC();

   // Default-constructed iterator.
   collections::vector<int>::const_iterator itr;
   ASSERT_THROWS(collections::out_of_range, *itr);
   ASSERT_THROWS(collections::out_of_range, --itr);
   ASSERT_THROWS(collections::out_of_range, ++itr);
   ASSERT_THROWS(collections::out_of_range, --itr);
   ASSERT_THROWS(collections::out_of_range, ++itr);
   ASSERT_THROWS(collections::out_of_range, itr[-1]);
   ASSERT_THROWS(collections::out_of_range, itr[0]);
   ASSERT_THROWS(collections::out_of_range, itr[1]);

   collections::vector<int> v;
   ASSERT(v.cbegin() == v.end());

   // No accessible elements.
   ASSERT_THROWS(collections::out_of_range, v[-1]);
   ASSERT_THROWS(collections::out_of_range, v[0]);
   ASSERT_THROWS(collections::out_of_range, v[1]);

   // Should not allow to move an iterator to outside [begin, end].
   ASSERT_DOES_NOT_THROW(v.cbegin());
   ASSERT_DOES_NOT_THROW(v.cend());
   ASSERT_THROWS(collections::out_of_range, --v.cbegin());
   ASSERT_THROWS(collections::out_of_range, ++v.cbegin());
   ASSERT_THROWS(collections::out_of_range, --v.cend());
   ASSERT_THROWS(collections::out_of_range, ++v.cend());
   ASSERT_THROWS(collections::out_of_range, v.cbegin()[-1]);
   ASSERT_THROWS(collections::out_of_range, v.cbegin()[0]);
   ASSERT_THROWS(collections::out_of_range, v.cbegin()[1]);

   // Should not allow to dereference begin() or end() of an empty vector.
   ASSERT_THROWS(collections::out_of_range, *v.cbegin());
   ASSERT_THROWS(collections::out_of_range, *v.cend());

   v.push_back(1);
   ASSERT(v.begin() != v.cend());

   // One accessible element.
   ASSERT_THROWS(collections::out_of_range, v[-1]);
   ASSERT_DOES_NOT_THROW(v[0]);
   ASSERT_THROWS(collections::out_of_range, v[1]);

   // Should not allow to move an iterator to outside [begin, end].
   ASSERT_THROWS(collections::out_of_range, --v.cbegin());
   ASSERT_DOES_NOT_THROW(++v.cbegin());
   ASSERT_DOES_NOT_THROW(--v.cend());
   ASSERT_THROWS(collections::out_of_range, ++v.cend());
   ASSERT_THROWS(collections::out_of_range, v.cbegin()[-1]);
   ASSERT_DOES_NOT_THROW(v.cbegin()[0]);
   ASSERT_THROWS(collections::out_of_range, v.cbegin()[1]);

   // Should allow to dereference begin(), but not end() of a non-empty vector.
   ASSERT_DOES_NOT_THROW(*v.cbegin());
   ASSERT_THROWS(collections::out_of_range, *v.cend());

   v.push_back(2);
   v.push_back(3);

   // Remove an element by iterator.
   v.remove_at(std::find(v.cbegin(), v.cend(), 2));
   ASSERT(v.size() == 2u);
   ASSERT(v[0] == 1);
   ASSERT(v[1] == 3);

   // Remove an element with an invalid iterator.
   ASSERT_THROWS(collections::out_of_range, v.remove_at(v.begin() - 1));
   ASSERT_THROWS(collections::out_of_range, v.remove_at(v.end()));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   collections_vector_trivial_removal,
   "lofty::collections::vector – removal of trivial elements"
) {
   LOFTY_TRACE_FUNC();

   collections::vector<int> v, zero, one, two, one_two;
   one.push_back(1);
   two.push_back(2);
   one_two.push_back(1);
   one_two.push_back(2);

   v = zero;

   // Remove from empty vector by index.
   ASSERT_THROWS(collections::out_of_range, v.remove_at(v.cend() - 1));
   ASSERT_THROWS(collections::out_of_range, v.remove_at(v.cbegin()));
   ASSERT_THROWS(collections::out_of_range, v.remove_at(v.cbegin() + 1));

   v = one_two;

   // Remove from 2-element vector by index.
   ASSERT_THROWS(collections::out_of_range, v.remove_at(v.cend() - 3));
   ASSERT((v.remove_at(v.cend() - 2), v) == two);
   v = one_two;
   ASSERT((v.remove_at(v.cend() - 1), v) == one);
   v = one_two;
   ASSERT((v.remove_at(v.cbegin()), v) == two);
   v = one_two;
   ASSERT((v.remove_at(v.cbegin() + 1), v) == one);
   v = one_two;
   ASSERT_THROWS(collections::out_of_range, v.remove_at(v.cbegin() + 2));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   collections_vector_memory,
   "lofty::collections::vector – memory management"
) {
   LOFTY_TRACE_FUNC();

   using testing::utility::make_container_data_ptr_tracker;

   collections::vector<int> v1;
   auto tracker1(make_container_data_ptr_tracker(&v1));
   // Note: the embedded item array size will probably be > 2.
   collections::vector<int, 2> v2;
   auto tracker2(make_container_data_ptr_tracker(&v2));
   // Note: the embedded item array size will probably be > 10.
   collections::vector<int, 10> v3;
   auto tracker3(make_container_data_ptr_tracker(&v3));

   /* Add one element to each vector, so they all allocate a new item array or begin using their own embedded
   one. */

   // Should allocate a new item array.
   v1.push_back(10);
   ASSERT(tracker1.changed());
   ASSERT(v1.size() == 1u);
   ASSERT(v1[0] == 10);

   // Should begin using the embedded item array.
   v2.push_back(20);
   ASSERT(tracker2.changed());
   ASSERT(v2.size() == 1u);
   ASSERT(v2[0] == 20);
   int const * const p2Static(v2.data());

   // Should begin using the embedded item array.
   v3.push_back(30);
   ASSERT(tracker3.changed());
   ASSERT(v3.size() == 1u);
   ASSERT(v3[0] == 30);
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
   tracker1.changed();
   ASSERT(v1.size() == 10u);
   ASSERT(v1[0] == 10);
   ASSERT(v1[1] == 11);
   ASSERT(v1[2] == 12);
   ASSERT(v1[3] == 13);
   ASSERT(v1[4] == 14);
   ASSERT(v1[5] == 15);
   ASSERT(v1[6] == 16);
   ASSERT(v1[7] == 17);
   ASSERT(v1[8] == 18);
   ASSERT(v1[9] == 19);

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
   ASSERT(tracker2.changed());
   ASSERT(v2.size() == 10u);
   ASSERT(v2[0] == 20);
   ASSERT(v2[1] == 21);
   ASSERT(v2[2] == 22);
   ASSERT(v2[3] == 23);
   ASSERT(v2[4] == 24);
   ASSERT(v2[5] == 25);
   ASSERT(v2[6] == 26);
   ASSERT(v2[7] == 27);
   ASSERT(v2[8] == 28);
   ASSERT(v2[9] == 29);

   // The embedded item array has room for this, so no reallocation is needed.
   v3.push_back(31);
   ASSERT(v3.data() == p3Static);
   ASSERT(!tracker3.changed());
   ASSERT(v3.size() == 2u);
   ASSERT(v3[0] == 30);
   ASSERT(v3[1] == 31);

   // Check assignment from larger to smaller embedded vectors.

   // Should keep the current item array, copying v2’s items over.
   v1 = v2.vector0();
   ASSERT(!tracker1.changed());
   ASSERT(v1.size() == 10u);
   ASSERT(v1[0] == 20);
   ASSERT(v1[1] == 21);
   ASSERT(v1[2] == 22);
   ASSERT(v1[3] == 23);
   ASSERT(v1[4] == 24);
   ASSERT(v1[5] == 25);
   ASSERT(v1[6] == 26);
   ASSERT(v1[7] == 27);
   ASSERT(v1[8] == 28);
   ASSERT(v1[9] == 29);

   // Should return to using the embedded item array, copying v3’s items over.
   v2 = v3.vector0();
   ASSERT(v2.data() == p2Static);
   ASSERT(tracker2.changed());
   ASSERT(v2.size() == 2u);
   ASSERT(v2[0] == 30);
   ASSERT(v2[1] == 31);
   // “Rebrand” the items as 2x.
   v2[0] = 20;
   v2[1] = 21;

   /* The current item array should still be large enough, but this should drop it to use the temporary one
   created by operator+(). */
   v1 = v2 + v3;
   ASSERT(tracker1.changed());
   ASSERT(v1.size() == 4u);
   ASSERT(v1[0] == 20);
   ASSERT(v1[1] == 21);
   ASSERT(v1[2] == 30);
   ASSERT(v1[3] == 31);
   // “Rebrand” the items as 1x.
   v1[0] = 10;
   v1[1] = 11;
   v1[2] = 12;
   v1[3] = 13;

   // This should be too much for the embedded item array, so a new one should be allocated.
   v3 += v1 + v2 + v1 + v3 + v1;
   ASSERT(tracker3.changed());
   ASSERT(v3.size() == 18u);
   ASSERT(v3[0] == 30);
   ASSERT(v3[1] == 31);
   ASSERT(v3[2] == 10);
   ASSERT(v3[3] == 11);
   ASSERT(v3[4] == 12);
   ASSERT(v3[5] == 13);
   ASSERT(v3[6] == 20);
   ASSERT(v3[7] == 21);
   ASSERT(v3[8] == 10);
   ASSERT(v3[9] == 11);
   ASSERT(v3[10] == 12);
   ASSERT(v3[11] == 13);
   ASSERT(v3[12] == 30);
   ASSERT(v3[13] == 31);
   ASSERT(v3[14] == 10);
   ASSERT(v3[15] == 11);
   ASSERT(v3[16] == 12);
   ASSERT(v3[17] == 13);

   // Ensure that the vector doesn’t automatically shrink to fit when downsized.
   std::size_t highest_capacity = v3.capacity();
   v3.set_size(0);
   ASSERT(v3.size() == 0u);
   ASSERT(v3.capacity() == highest_capacity);
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

/*! Instantiates and returns a dynamic vector. The vector will contain one item, added in a way that should
cause only one new instance of instances_counter to be created, one moved and none copied. Additional
copies/moves may occur upon return.

return
   Newly-instantiated dynamic vector.
*/
static collections::vector<testing::utility::instances_counter> return_vector() {
   LOFTY_TRACE_FUNC();

   collections::vector<testing::utility::instances_counter> v;
   // New instance, immediately moved.
   v.push_back(testing::utility::instances_counter());
   /* This will move the item array or the items in it, depending on the destination type (embedded or dynamic
   item array). */
   return _std::move(v);
}

LOFTY_TESTING_TEST_CASE_FUNC(
   collections_vector_movement,
   "lofty::collections::vector – item and item array movement"
) {
   LOFTY_TRACE_FUNC();

   typedef testing::utility::instances_counter instances_counter;
   {
      /* This will move the item array from the returned vector to v, so no item copies or moves will occur
      other than the ones in return_vector(). */
      collections::vector<instances_counter> v(return_vector());
      ASSERT(instances_counter::new_insts() == 1u);
      ASSERT(instances_counter::moves() == 1u);
      ASSERT(instances_counter::copies() == 0u);
      instances_counter::reset_counts();

      /* This should create a new copy, with no intermediate moves because all passages are by reference or
      pointer. */
      v.push_back(v[0]);
      ASSERT(instances_counter::new_insts() == 0u);
      ASSERT(instances_counter::moves() == 0u);
      ASSERT(instances_counter::copies() == 1u);
      instances_counter::reset_counts();
   }

   {
      collections::vector<instances_counter, 9> v;
      /* This will move the individual items from the returned vector to v’s embedded item array. Can’t just
      construct v with return_vector() because v would merely use that item array instead of its own embedded
      one, resulting in no additional moves other than the one in return_vector(). */
      v += return_vector();
      ASSERT(instances_counter::new_insts() == 1u);
      ASSERT(instances_counter::moves() == 2u);
      ASSERT(instances_counter::copies() == 0u);
      instances_counter::reset_counts();
   }
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   from_text_istream_vector,
   "lofty::from_text_istream – lofty::collections::vector"
) {
   LOFTY_TRACE_FUNC();

   collections::vector<int> v;

   ASSERT_DOES_NOT_THROW((v = from_str<collections::vector<int>>(LOFTY_SL("{}"))));
   ASSERT(v.size() == 0u);

   ASSERT_DOES_NOT_THROW((v = from_str<collections::vector<int>>(LOFTY_SL("{5}"))));
   ASSERT(v.size() == 1u);
   ASSERT(v[0] == 5);

   ASSERT_DOES_NOT_THROW((v = from_str<collections::vector<int>>(LOFTY_SL("{3, 50}"))));
   ASSERT(v.size() == 2u);
   ASSERT(v[0] == 3);
   ASSERT(v[1] == 50);

   ASSERT_DOES_NOT_THROW((v = from_str<collections::vector<int>>(LOFTY_SL("{16, 8, 4}"))));
   ASSERT(v.size() == 3u);
   ASSERT(v[0] == 16);
   ASSERT(v[1] == 8);
   ASSERT(v[2] == 4);
}

}} //namespace lofty::test
