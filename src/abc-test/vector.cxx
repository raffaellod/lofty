/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013, 2014
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abc/testing/test_case.hxx>
#include <abc/trace.hxx>
#include <algorithm>
#include <abc/testing/utility.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::vector_basic


namespace abc {

namespace test {

class vector_basic :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::*vector classes - basic operations"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      dmvector<int> v;

      ABC_TESTING_ASSERT_EQUAL(v.size(), 0u);

      v.append(1);
      ABC_TESTING_ASSERT_EQUAL(v.size(), 1u);
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

      v.append(3);
      ABC_TESTING_ASSERT_EQUAL(v.size(), 3u);
      ABC_TESTING_ASSERT_EQUAL(v[0], 2);
      ABC_TESTING_ASSERT_EQUAL(v[1], 1);
      ABC_TESTING_ASSERT_EQUAL(v[2], 3);

      ABC_TESTING_ASSERT_EQUAL(v.index_of(1), 1);

      ABC_TESTING_ASSERT_EQUAL(v.last_index_of(1), 1);

      ABC_TESTING_ASSERT_EQUAL(std::find(v.cbegin(), v.cend(), 1), v.cbegin() + 1);

      v.remove_at(std::find(v.cbegin(), v.cend(), 1));
      ABC_TESTING_ASSERT_EQUAL(v.size(), 2u);
      ABC_TESTING_ASSERT_EQUAL(v[0], 2);
      ABC_TESTING_ASSERT_EQUAL(v[1], 3);
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::vector_basic)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::vector_memory_mgmt


namespace abc {

namespace test {

class vector_memory_mgmt :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::*vector classes - memory management"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      dmvector<int> v1;
      auto cdpt1(testing::utility::make_container_data_ptr_tracker(v1));
      // Note: the embedded item array size will probably be > 2.
      smvector<int, 2> v2;
      auto cdpt2(testing::utility::make_container_data_ptr_tracker(v2));
      // Note: the embedded item array size will probably be > 10.
      smvector<int, 10> v3;
      auto cdpt3(testing::utility::make_container_data_ptr_tracker(v3));

      // Add one element to each vector.

      // Should allocate a new item array.
      v1.append(10);
      ABC_TESTING_ASSERT_TRUE(cdpt1.changed());
      ABC_TESTING_ASSERT_EQUAL(v1.size(), 1u);
      ABC_TESTING_ASSERT_EQUAL(v1[0], 10);

      // Should begin using the embedded item array.
      v2.append(20);
      ABC_TESTING_ASSERT_TRUE(cdpt2.changed());
      ABC_TESTING_ASSERT_EQUAL(v2.size(), 1u);
      ABC_TESTING_ASSERT_EQUAL(v2[0], 20);

      // Should begin using the embedded item array.
      v3.append(30);
      ABC_TESTING_ASSERT_TRUE(cdpt3.changed());
      ABC_TESTING_ASSERT_EQUAL(v3.size(), 1u);
      ABC_TESTING_ASSERT_EQUAL(v3[0], 30);

      // Add more elements to each vector.

      // These are too many for the newly-allocated item array, so a new one should be allocated.
      v1.append(11);
      v1.append(12);
      v1.append(13);
      v1.append(14);
      v1.append(15);
      v1.append(16);
      v1.append(17);
      v1.append(18);
      v1.append(19);
      ABC_TESTING_ASSERT_TRUE(cdpt1.changed());
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
      v2.append(21);
      v2.append(22);
      v2.append(23);
      v2.append(24);
      v2.append(25);
      v2.append(26);
      v2.append(27);
      v2.append(28);
      v2.append(29);
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
      v3.append(31);
      ABC_TESTING_ASSERT_FALSE(cdpt3.changed());
      ABC_TESTING_ASSERT_EQUAL(v3.size(), 2u);
      ABC_TESTING_ASSERT_EQUAL(v3[0], 30);
      ABC_TESTING_ASSERT_EQUAL(v3[1], 31);

      // Check assignment from larger to smaller static vectors.

      // Should keep the current item array, copying v2’s items over.
      v1 = v2;
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
      v2 = v3;
      ABC_TESTING_ASSERT_TRUE(cdpt2.changed());
      ABC_TESTING_ASSERT_EQUAL(v2.size(), 2u);
      ABC_TESTING_ASSERT_EQUAL(v2[0], 30);
      ABC_TESTING_ASSERT_EQUAL(v2[1], 31);
      // “Rebrand” the items as 2x.
      v2[0] = 20;
      v2[1] = 21;

      // The current item array should still be large enough, but this should drop it to use the
      // temporary one created by operator+().
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
   }

#if 0
      // Check that returning a vector with a dynamically allocated descriptor does not cause a new
      // descriptor to be allocated, nor copies the items.
      {
         dmvector<test_with_ptr> v(move_constr_test(&pi));
         if (v[0].get_ptr() != pi) {
            return 100;
         }
         if (test_with_ptr::get_count() != 1) {
            return 110 + test_with_ptr::get_count();
         }
         // Also check that add(T &&) doesn’t make extra copies.
         v.append(test_with_ptr());
         if (test_with_ptr::get_count() != 2) {
            return 120 + test_with_ptr::get_count();
         }
      }

      // Check that returning a vector with a dynamically allocated descriptor into a vector with a
      // statically allocated descriptor causes the items to be moved to the static descriptor.
      {
         smvector<test_with_ptr, 2> v(move_constr_test(&pi));
         if (v[0].get_ptr() != pi) {
            return 130;
         }
         if (test_with_ptr::get_count() != 3) {
            return 140 + test_with_ptr::get_count();
         }
      }

      return 0;
   }


   /** Creates a local vector<test_with_ptr> that’s modified in place, and adds to it a temporary
   item (which should cause no item copies to be made) whose internal pointer is stored in *ppi; the
   vector is then returned (which, again, should cause no item copies to be made).

   TODO: comment signature.
   */
   dmvector<test_with_ptr> move_constr_test(int const ** ppi) {
      // vector::vector();
      dmvector<test_with_ptr> v;
      // test_with_ptr::test_with_ptr();
      // vector::add(T && t);
      v.append(test_with_ptr());
      // vector::operator[]();
      // test_with_ptr::get_ptr();
      *ppi = v[0].get_ptr();
      // vector::vector(vector && v);
      return std::move(v);
      // vector::~vector();
   }
#endif
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::vector_memory_mgmt)

