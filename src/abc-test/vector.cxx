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



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::container_data_ptr_tracker

namespace abc {

namespace testing {

/** Tracks changes in the data() member of a container.

TODO: move to abc::testing, since it’s going to be useful for other containers as well.
*/
template <class T>
class container_data_ptr_tracker {
public:

   /** Constructor. Starts tracking changes in the specified object.

   t
      Object to track.
   */
   container_data_ptr_tracker(T const & t) :
      m_t(t),
      m_pti(t.data()) {
   }


   /** Checks if the monitored object’s data pointer has changed.

   return
      true if the data pointer has changed, or false otherwise.
   */
   bool changed() {
      ABC_TRACE_FN((this));

      typename T::const_pointer ptiOld(m_pti);
      // Update the data pointer for the next call.
      m_pti = m_t.data();
      // Check if the data pointer has changed.
      return m_pti != ptiOld;
   }


private:

   /** Reference to the T instance to be monitored. */
   T const & m_t;
   /** Pointer to m_t’s data. */
   typename T::const_pointer m_pti;
};

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::instances_counter

namespace abc {

namespace testing {

/** This class is meant for use in containers, to track when items are copied, when they’re moved,
and to check if individual instances have been copied instead of being moved.

TODO: move to abc::testing, since it’s going to be useful for other containers as well.
*/
class instances_counter {
public:

   /** Constructor. The copying overload doesn’t really use their argument, because the only
   non-static member (m_iUnique) is always generated.

   ic
      Source object.
   */
   instances_counter() :
      m_iUnique(++m_iNextUnique) {
      ++m_cNew;
   }
   instances_counter(instances_counter const & ic) :
      m_iUnique(++m_iNextUnique) {
      ABC_UNUSED_ARG(ic);
      ++m_cCopies;
   }
   instances_counter(instances_counter && ic) :
      m_iUnique(ic.m_iUnique) {
      ++m_cMoves;
   }


   /** Assigment operator. The copying overload doesn’t really use its argument, because the only
   non-static member (m_iUnique) is always generated.

   ic
      Source object.
   */
   instances_counter & operator=(instances_counter const & ic) {
      ABC_UNUSED_ARG(ic);
      m_iUnique = ++m_iNextUnique;
      ++m_cCopies;
      return *this;
   }
   instances_counter & operator=(instances_counter && ic) {
      m_iUnique = ic.m_iUnique;
      ++m_cMoves;
      return *this;
   }


   /** Returns the count of instances created, excluding moved ones.

   return
      Count of instances.
   */
   static size_t copies() {
      return m_cCopies;
   }


   /** Returns the count of moved instances.

   return
      Count of instances.
   */
   static size_t moves() {
      return m_cMoves;
   }


   /** Returns the count of new (not copied, not moved) instances. Useful to track how many
   instances have not been created from a source instance, perhaps only to be copy- or move-assigned
   later, which would be less efficient than just copy- or move-constructing, 

   return
      Count of instances.
   */
   static size_t new_insts() {
      return m_cNew;
   }


   /** Resets the copies/moves/new instance counts.
   */
   static void reset_counts() {
      m_cCopies = 0;
      m_cMoves = 0;
      m_cNew = 0;
   }


   /** Returns the unique value associated to this object.

   return
      Unique value.
   */
   int unique() const {
      return m_iUnique;
   }


private:

   int m_iUnique;
   static size_t m_cCopies;
   static size_t m_cMoves;
   static size_t m_cNew;
   static int m_iNextUnique;
};


size_t instances_counter::m_cCopies = 0;
size_t instances_counter::m_cMoves = 0;
size_t instances_counter::m_cNew = 0;
int instances_counter::m_iNextUnique = 0;

} //namespace testing

} //namespace abc


/** Equality comparison operator. Should always return false, since no two simultaneously-living
instances should have the same unique value.

oc1
   First object to compare.
oc2
   Second object to compare.
return
   true if oc1 has the same unique value as oc2, or false otherwise.
*/
bool operator==(
   abc::testing::instances_counter const & oc1, abc::testing::instances_counter const & oc2
);

bool operator==(
   abc::testing::instances_counter const & oc1, abc::testing::instances_counter const & oc2
) {
   return oc1.unique() == oc2.unique();
}


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

#if 0
      // Try mix’n’matching vectors of different sizes, and check that vectors using static
      // descriptors only switch to dynamic descriptors if necessary.
      {
         dmvector<int> v0;
         pi = v0.data();
         v0.append(0);
         if (v0.data() == pi) {
            return 50;
         }

         smvector<int, 3> v1;
         pi = v1.data();
         v1.append(1);
         if (v1.data() == pi) {
            return 51;
         }
         pi = v1.data();
         v1.append(2);
         if (v1.data() != pi) {
            return 52;
         }

         smvector<int, 1> v2;
         pi = v2.data();
         v2.append(3);
         if (v2.data() == pi) {
            return 53;
         }

         pi = v0.data();
         v0 = v1 + v2;
         if (v0.data() == pi || v0.size() != 3 || v0[0] != 1 || v0[1] != 2 || v0[2] != 3) {
            return 54;
         }

         pi = v1.data();
         v1 = v2 + v0;
         if (
            v1.data() == pi || v1.size() != 4 ||
            v1[0] != 3 || v1[1] != 1 || v1[2] != 2 || v1[3] != 3
         ) {
            return 55;
         }

         pi = v2.data();
         v2 = v0 + v1;
         if (
            v2.data() == pi || v2.size() != 7 ||
            v2[0] != 1 || v2[1] != 2 || v2[2] != 3 ||
            v2[3] != 3 || v2[4] != 1 || v2[5] != 2 || v2[6] != 3
         ) {
            return 56;
         }
      }

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

ABC_TESTING_REGISTER_TEST_CASE(abc::test::vector_basic)

