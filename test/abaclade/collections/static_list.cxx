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
#include <abaclade/testing/test_case.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

namespace {

// Forward declaration.
class static_list_node_test;

//! Singleton static_list test subclass.
class static_list_test : public collections::static_list<static_list_test, static_list_node_test> {
public:
   /*! Returns the one and only instance of this class.

   @return
      *this.
   */
   static static_list_test & instance() {
      return *static_cast<static_list_test *>(&sm_dm);
   }

private:
   //! Only instance of this class’ data.
   static data_members sm_dm;
};

static_list_test::data_members static_list_test::sm_dm = ABC_COLLECTIONS_STATIC_LIST_INITIALIZER;


//! Element of static_list_test.
class static_list_node_test :
   public collections::static_list_node_base,
   public collections::static_list_node<static_list_test, static_list_node_test> {
public:
   /*! Constructor.

   @param i
      Value of the internal integer.
   */
   static_list_node_test(int i) :
      m_i(i) {
   }

   /*! Returns the internal integer.

   @return
      Internal integer.
   */
   int get() const {
      return m_i;
   }

private:
   //! Internal integer.
   int m_i;
};

} //namespace

ABC_TESTING_TEST_CASE_FUNC("abc::collections::static_list – basic operations") {
   ABC_TRACE_FUNC(this);

   auto & sl = static_list_test::instance();

   /* Since by design static_list elements are added automatically on instantiation and removed on
   destruction, additions and removals are governed by nested scopes. */

   ABC_TESTING_ASSERT_TRUE(sl.empty());
   ABC_TESTING_ASSERT_EQUAL(sl.size(), 0u);
   ABC_TESTING_ASSERT_TRUE(sl.begin() == sl.end());
   ABC_TESTING_ASSERT_TRUE(sl.rbegin() == sl.rend());

   {
      static_list_node_test n10(10);
      ABC_TESTING_ASSERT_FALSE(sl.empty());
      ABC_TESTING_ASSERT_EQUAL(sl.size(), 1u);
      {
         // Simple forward iteration.
         auto it(sl.begin());
         ABC_TESTING_ASSERT_EQUAL(it->get(), 10);
         ++it;
         ABC_TESTING_ASSERT_TRUE(it == sl.end());
      }

      {
         static_list_node_test n20(20);
         ABC_TESTING_ASSERT_FALSE(sl.empty());
         ABC_TESTING_ASSERT_EQUAL(sl.size(), 2u);
         {
            // Backwards iteration.
            auto it(sl.rbegin());
            ABC_TESTING_ASSERT_EQUAL(it->get(), 20);
            ++it;
            ABC_TESTING_ASSERT_EQUAL(it->get(), 10);
            ++it;
            ABC_TESTING_ASSERT_TRUE(it == sl.rend());
         }
      }

      ABC_TESTING_ASSERT_FALSE(sl.empty());
      ABC_TESTING_ASSERT_EQUAL(sl.size(), 1u);
   }

   ABC_TESTING_ASSERT_TRUE(sl.empty());
   ABC_TESTING_ASSERT_EQUAL(sl.size(), 0u);
   ABC_TESTING_ASSERT_TRUE(sl.begin() == sl.end());
   ABC_TESTING_ASSERT_TRUE(sl.rbegin() == sl.rend());

   {
      static_list_node_test n30(30);
      ABC_TESTING_ASSERT_FALSE(sl.empty());
      ABC_TESTING_ASSERT_EQUAL(sl.size(), 1u);
      ABC_TESTING_ASSERT_TRUE(sl.begin() != sl.end());
      ABC_TESTING_ASSERT_TRUE(sl.rbegin() != sl.rend());
   }

   ABC_TESTING_ASSERT_TRUE(sl.empty());
   ABC_TESTING_ASSERT_EQUAL(sl.size(), 0u);
   ABC_TESTING_ASSERT_TRUE(sl.begin() == sl.end());
}

}} //namespace abc::test
