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
class static_list_test :
   public collections::static_list<static_list_test, static_list_node_test> {
public:
   ABC_COLLECTIONS_STATIC_LIST_DECLARE_SUBCLASS_STATIC_MEMBERS(static_list_test)
};

//! Element of static_list_test.
class static_list_node_test :
   public collections::static_list<static_list_test, static_list_node_test>::node {
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

ABC_COLLECTIONS_STATIC_LIST_DEFINE_SUBCLASS_STATIC_MEMBERS(static_list_test)

} //namespace

ABC_TESTING_TEST_CASE_FUNC("abc::collections::static_list – basic operations") {
   ABC_TRACE_FUNC(this);

   /* Since by design static_list elements are added automatically on instantiation and removed on
   destruction, additions and removals are governed by nested scopes. */

   ABC_TESTING_ASSERT_TRUE(static_list_test::empty());
   ABC_TESTING_ASSERT_EQUAL(static_list_test::size(), 0u);
   ABC_TESTING_ASSERT_TRUE(static_list_test::begin() == static_list_test::end());
   ABC_TESTING_ASSERT_TRUE(static_list_test::rbegin() == static_list_test::rend());

   {
      static_list_node_test n10(10);
      ABC_TESTING_ASSERT_FALSE(static_list_test::empty());
      ABC_TESTING_ASSERT_EQUAL(static_list_test::size(), 1u);
      {
         // Simple forward iteration.
         auto it(static_list_test::begin());
         ABC_TESTING_ASSERT_EQUAL(it->get(), 10);
         ++it;
         ABC_TESTING_ASSERT_TRUE(it == static_list_test::end());
      }

      {
         static_list_node_test n20(20);
         ABC_TESTING_ASSERT_FALSE(static_list_test::empty());
         ABC_TESTING_ASSERT_EQUAL(static_list_test::size(), 2u);
         {
            // Backwards iteration.
            auto it(static_list_test::rbegin());
            ABC_TESTING_ASSERT_EQUAL(it->get(), 20);
            ++it;
            ABC_TESTING_ASSERT_EQUAL(it->get(), 10);
            ++it;
            ABC_TESTING_ASSERT_TRUE(it == static_list_test::rend());
         }
      }

      ABC_TESTING_ASSERT_FALSE(static_list_test::empty());
      ABC_TESTING_ASSERT_EQUAL(static_list_test::size(), 1u);
   }

   ABC_TESTING_ASSERT_TRUE(static_list_test::empty());
   ABC_TESTING_ASSERT_EQUAL(static_list_test::size(), 0u);
   ABC_TESTING_ASSERT_TRUE(static_list_test::begin() == static_list_test::end());
   ABC_TESTING_ASSERT_TRUE(static_list_test::rbegin() == static_list_test::rend());

   {
      static_list_node_test n30(30);
      ABC_TESTING_ASSERT_FALSE(static_list_test::empty());
      ABC_TESTING_ASSERT_EQUAL(static_list_test::size(), 1u);
      ABC_TESTING_ASSERT_TRUE(static_list_test::begin() != static_list_test::end());
      ABC_TESTING_ASSERT_TRUE(static_list_test::rbegin() != static_list_test::rend());
   }

   ABC_TESTING_ASSERT_TRUE(static_list_test::empty());
   ABC_TESTING_ASSERT_EQUAL(static_list_test::size(), 0u);
   ABC_TESTING_ASSERT_TRUE(static_list_test::begin() == static_list_test::end());
}

}} //namespace abc::test
