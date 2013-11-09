/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013
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

#ifndef ABC_TESTING_TEST_CASE_HXX
#define ABC_TESTING_TEST_CASE_HXX

#include <abc/testing/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abc/testing/runner.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::test_case


namespace abc {

namespace testing {

/** Base class for test cases.
*/
class ABCTESTINGAPI test_case {
public:

   /** Constructor.
   */
   test_case();


   /** Destructor.
   */
   virtual ~test_case();


   /** Initializes the object. Split into a method separated from the constructor so that derived
   classes don’t need to declare a constructor just to forward its arguments.

   prunner
      Pointer to the test runner.
   */
   void init(runner * prunner);


   /** Executes the test case.
   */
   virtual void run() = 0;


   /** Returns a short description for the test case.

   return
      Test case title.
   */
   virtual istr title() = 0;


protected:

   /** Validates an assertion.

   bExpr
      Result of the assertion expression.
   pszExpr
      Assertion being tested.
   */
   void assert(bool bExpr, istr const & sExpr);


protected:

   /** Runner executing this test. */
   runner * m_prunner;
};

} //namespace testing

} //namespace abc


/** Asserts that the specified expression does not throw.

expr
   Expression that should not throw.
*/
#define ABC_TESTING_ASSERT_DOES_NOT_THROW(expr) \
   do { \
      bool _bCaught(false); \
      try { \
         static_cast<void>(expr); \
      } catch (...) { \
         _bCaught = true; \
      } \
      this->assert(!_bCaught, SL(#expr)); \
   } while (false)


/** Asserts that the specified expressions evaluate to the same value.

expr1
   First expression.
expr2
   Second expression.
*/
#define ABC_TESTING_ASSERT_EQUAL(expr1, expr2) \
   this->assert(expr1 == expr2, SL(#expr1) SL(" == ") SL(#expr2))


/** Asserts that the specified expression evaluates to false.

expr
   Expression that should evaulate to false.
*/
#define ABC_TESTING_ASSERT_FALSE(expr) \
   /* Use static_cast() to make the compiler raise warnings in case expr is not of type bool. */ \
   this->assert(!static_cast<bool>(expr), SL(#expr) SL(" == false"))


/** Asserts that the first expression evaluates to more than the second expression.

expr1
   First expression.
expr2
   Second expression.
*/
#define ABC_TESTING_ASSERT_GREATER(expr1, expr2) \
   this->assert(expr1 > expr2, SL(#expr1) SL(" > ") SL(#expr2))


/** Asserts that the first expression evaluates to at least the same value as the second expression.

expr1
   First expression.
expr2
   Second expression.
*/
#define ABC_TESTING_ASSERT_GREATER_EQUAL(expr1, expr2) \
   this->assert(expr1 >= expr2, SL(#expr1) SL(" >= ") SL(#expr2))


/** Asserts that the first expression evaluates to less than the second expression.

expr1
   First expression.
expr2
   Second expression.
*/
#define ABC_TESTING_ASSERT_LESS(expr1, expr2) \
   this->assert(expr1 < expr2, SL(#expr1) SL(" < ") SL(#expr2))


/** Asserts that the first expression evaluates to at most the same value as the second expression.

expr1
   First expression.
expr2
   Second expression.
*/
#define ABC_TESTING_ASSERT_LESS_EQUAL(expr1, expr2) \
   this->assert(expr1 <= expr2, SL(#expr1) SL(" <= ") SL(#expr2))


/** Asserts that the specified expressions don’t evaluate to the same value.

expr1
   First expression.
expr2
   Second expression.
*/
#define ABC_TESTING_ASSERT_NOT_EQUAL(expr1, expr2) \
   this->assert(expr1 != expr2, SL(#expr1) SL(" != ") SL(#expr2))


/** Asserts that the specified expression throws an exception of the specified type.

type
   Exception class that should be caught.
expr
   Expression that should throw.
*/
#define ABC_TESTING_ASSERT_THROWS(type, expr) \
   do { \
      bool _bCaught(false); \
      try { \
         static_cast<void>(expr); \
      } catch (type const &) { \
         _bCaught = true; \
      } \
      this->assert(_bCaught, SL(#expr)); \
   } while (false)


/** Asserts that the specified expression evaluates to true.

expr
   Expression that should evaulate to true.
*/
#define ABC_TESTING_ASSERT_TRUE(expr) \
   this->assert(expr, SL(#expr) SL(" == true"))


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::test_case_factory_impl


namespace abc {

namespace testing {

/** Maintains a list of abc::testing::test_case-derived classes that can be used by an
abc::testing::runner instance to instantiate and execute each test case.
*/
class ABCTESTINGAPI test_case_factory_impl {
public:

   /** Factory function, returning an abc::testing::test_case instance. */
   typedef std::unique_ptr<test_case> (* factory_fn)(runner * prunner);
   /** Linked list item. */
   struct list_item {
      list_item * pliNext;
      factory_fn pfnFactory;
   };


public:

   /** Constructor.

   pli
      Pointer to the derived class’s factory list item.
   */
   test_case_factory_impl(list_item * pli);


   /** Returns a pointer to the head of the list of factory functions, which the caller can then use
   to walk the entire list (ending when an item’s next pointer is NULL).

   return
      Pointer to the head of the list.
   */
   static list_item * get_factory_list_head() {
      return sm_pliHead;
   }


private:

   /** Pointer to the head of the list of factory functions. */
   static list_item * sm_pliHead;
   /** Pointer to the “next” pointer of the tail of the list of factory functions. */
   static list_item ** sm_ppliTailNext;
};

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::test_case_factory


namespace abc {

namespace testing {

/** Template version of abc::testing::test_case_factory_impl, able to instantiate classes derived
from abc::testing::test_case.
*/
template <class T>
class test_case_factory :
   public test_case_factory_impl {
public:

   /** Constructor.
   */
   test_case_factory() :
      test_case_factory_impl(&sm_li) {
   }


   /** Class factory for T.

   prunner
      Runner to provide to the test case.
   */
   static std::unique_ptr<test_case> factory(runner * prunner) {
      std::unique_ptr<T> pt(new T());
      pt->init(prunner);
      return std::move(pt);
   }


private:

   /** Entry in the list of factory functions for this class. */
   static list_item sm_li;
};

} //namespace testing

} //namespace abc


/** Registers an abc::testing::test_case-derived class for execution by an abc::testing::runner
instance.

cls
   Test case class.
*/
#define ABC_TESTING_REGISTER_TEST_CASE(cls) \
   namespace abc { \
   namespace testing { \
   \
   static test_case_factory<cls> ABC_CPP_APPEND_UID(g__test_case_factory_); \
   template <> \
   /*static*/ test_case_factory_impl::list_item test_case_factory<cls>::sm_li = { \
      NULL, \
      test_case_factory<cls>::factory \
   }; \
   \
   } /*namespace testing*/ \
   } /*namespace abc*/


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_TESTING_TEST_CASE_HXX

