/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013, 2014
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

   /** Core implementation of ABC_TESTING_ASSERT_*.

   pszFileName
      Path to the source file containing the expression.
   iLine
      Source line number.
   bSuccess
      true if the assertion was valid, or false otherwise.
   sExpr
      Source representation of the expression being evaluated.
   sOp
      Applied comparison operator.
   sExpected
      If bSuccess, expression generating the expected value (i.e. C++ expression, as a string); if
      !bSuccess, computed expected value (i.e. the actual value returned by the C++ expression, as a
      string).
   sActual
      Only used if !bSuccess, this is the computed actual value (i.e. return value of sExpr), as a
      string.
   */
   void assert_impl(
      char const * pszFileName, unsigned iLine, bool bSuccess,
      istr const & sExpr, istr const & sOp, istr const & sExpected, istr const & sActual
   ) {
//    ABC_TRACE_FN((this, pszFileName, iLine, bSuccess, sExpr, sOp, sExpected, sActual));

      if (bSuccess) {
         m_prunner->log_assertion_pass(pszFileName, iLine, sExpr, sOp, sExpected);
      } else {
         m_prunner->log_assertion_fail(pszFileName, iLine, sExpr, sOp, sExpected, sActual);
      }
   }


   /** Implementation of ABC_TESTING_ASSERT_EQUAL.
   */
   template <typename TExpr, typename TEqual>
   void assert_equal(
      char const * pszFileName, unsigned iLine,
      TExpr const & tActual, TEqual const & tEqual, istr const & sExpr, istr const & sEqual
   ) {
      bool bSuccess = (tActual == tEqual);
      assert_impl(
         pszFileName, iLine, bSuccess, sExpr, SL("== "),
         bSuccess ? sEqual : istr(to_str(tEqual)), bSuccess ? istr() : istr(to_str(tActual))
      );
   }


   /** Implementation of ABC_TESTING_ASSERT_FALSE.
   */
   void assert_false(char const * pszFileName, unsigned iLine, bool bActual, istr const & sExpr) {
      assert_impl(
         pszFileName, iLine, !bActual, sExpr, istr(),
         !bActual ? istr() : SL("false"), SL("true")
      );
   }


   /** Implementation of ABC_TESTING_ASSERT_GREATER.
   */
   template <typename TExpr, typename TLBound>
   void assert_greater(
      char const * pszFileName, unsigned iLine,
      TExpr const & tActual, TLBound const & tLBound, istr const & sExpr, istr const & sLBound
   ) {
      bool bSuccess = (tActual > tLBound);
      assert_impl(
         pszFileName, iLine, bSuccess, sExpr, SL("> "),
         bSuccess ? sLBound : istr(to_str(tLBound)), bSuccess ? istr() : istr(to_str(tActual))
      );
   }


   /** Implementation of ABC_TESTING_ASSERT_GREATER_EQUAL.
   */
   template <typename TExpr, typename TLBound>
   void assert_greater_equal(
      char const * pszFileName, unsigned iLine,
      TExpr const & tActual, TLBound const & tLBound, istr const & sExpr, istr const & sLBound
   ) {
      bool bSuccess = (tActual > tLBound);
      assert_impl(
         pszFileName, iLine, bSuccess, sExpr, SL(">= "),
         bSuccess ? sLBound : istr(to_str(tLBound)), bSuccess ? istr() : istr(to_str(tActual))
      );
   }


   /** Implementation of ABC_TESTING_ASSERT_LESS.
   */
   template <typename TExpr, typename TUBound>
   void assert_less(
      char const * pszFileName, unsigned iLine,
      TExpr const & tActual, TUBound const & tUBound, istr const & sExpr, istr const & sUBound
   ) {
      bool bSuccess = (tActual < tUBound);
      assert_impl(
         pszFileName, iLine, bSuccess, sExpr, SL("<= "),
         bSuccess ? sUBound : istr(to_str(tUBound)), bSuccess ? istr() : istr(to_str(tActual))
      );
   }


   /** Implementation of ABC_TESTING_ASSERT_LESS_EQUAL.
   */
   template <typename TExpr, typename TUBound>
   void assert_less_equal(
      char const * pszFileName, unsigned iLine,
      TExpr const & tActual, TUBound const & tUBound, istr const & sExpr, istr const & sUBound
   ) {
      bool bSuccess = (tActual <= tUBound);
      assert_impl(
         pszFileName, iLine, bSuccess, sExpr, SL("<= "),
         bSuccess ? sUBound : istr(to_str(tUBound)), bSuccess ? istr() : istr(to_str(tActual))
      );
   }


   /** Implementation of ABC_TESTING_ASSERT_NOT_EQUAL.
   */
   template <typename TExpr, typename TNotEqual>
   void assert_not_equal(
      char const * pszFileName, unsigned iLine,
      TExpr const & tActual, TNotEqual const & tNotEqual, istr const & sExpr, istr const & sNotEqual
   ) {
      bool bSuccess = (tActual != tNotEqual);
      assert_impl(
         pszFileName, iLine, bSuccess, sExpr, SL("!= "),
         bSuccess ? sNotEqual : istr(to_str(tNotEqual)), bSuccess ? istr() : istr(to_str(tActual))
      );
   }


   /** Implementation of ABC_TESTING_ASSERT_TRUE.
   */
   void assert_true(char const * pszFileName, unsigned iLine, bool bActual, istr const & sExpr) {
      assert_impl(
         pszFileName, iLine, bActual, sExpr, istr(), 
         bActual ? istr() : SL("true"), SL("false")
      );
   }


protected:

   /** Runner executing this test. */
   runner * m_prunner;
};

} //namespace testing

} //namespace abc


/** Asserts that an expression does not throw.

expr
   Expression to evaluate.
*/
#define ABC_TESTING_ASSERT_DOES_NOT_THROW(expr) \
   do { \
      bool _bCaught(false); \
      try { \
         static_cast<void>(expr); \
      } catch (...) { \
         _bCaught = true; \
      } \
      this->assert_false(__FILE__, __LINE__, _bCaught, SL(#expr)); \
   } while (false)


/** Asserts that the value of an expression differs from a specified value.

expr
   Expression to evaluate.
value
   Value that expr should not evaluate to.
*/
#define ABC_TESTING_ASSERT_EQUAL(expr, value) \
   this->assert_equal(__FILE__, __LINE__, expr, value, SL(#expr), SL(#value))


/** Asserts that an expression evaluates to false.

expr
   Expression to evaulate.
*/
#define ABC_TESTING_ASSERT_FALSE(expr) \
   /* Use static_cast() to make the compiler raise warnings in case expr is not of type bool. */ \
   this->assert_false(__FILE__, __LINE__, expr, SL(#expr))


/** Asserts that the value of an expression is strictly greater than a lower bound.

expr
   Expression to evaluate.
lbound
   Exclusive lower bound.
*/
#define ABC_TESTING_ASSERT_GREATER(expr, lbound) \
   this->assert_greater(__FILE__, __LINE__, expr, lbound, SL(#expr), SL(#lbound))


/** Asserts that the value of an expression is greater-than or equal-to a lower bound.

expr
   Expression to evaluate.
lbound
   Inclusive lower bound.
*/
#define ABC_TESTING_ASSERT_GREATER_EQUAL(expr, lbound) \
   this->assert_greater_equal(__FILE__, __LINE__, expr, lbound, SL(#expr), SL(#lbound))


/** Asserts that the value of an expression is strictly less than an upper bound.

expr
   Expression to evaluate.
ubound
   Exclusive upper bound.
*/
#define ABC_TESTING_ASSERT_LESS(expr, ubound) \
   this->assert_less_equal(__FILE__, __LINE__, expr, expected, SL(#expr), SL(#ubound))


/** Asserts that the value of an expression is less-than or equal-to an upper bound.

expr
   Expression to evaluate.
ubound
   Inclusive upper bound.
*/
#define ABC_TESTING_ASSERT_LESS_EQUAL(expr, ubound) \
   this->assert_less_equal(__FILE__, __LINE__, expr, ubound, SL(#expr), SL(#ubound))


/** Asserts that the value of an expression differs from a specified value.

expr
   Expression to evaluate.
value
   Value that expr should not evaluate to.
*/
#define ABC_TESTING_ASSERT_NOT_EQUAL(expr, value) \
   this->assert_not_equal(__FILE__, __LINE__, expr, value, SL(#expr), SL(#value))


/** Asserts that an expression throws a specific type of exception.

type
   Exception class that should be caught.
expr
   Expression to evaluate.
*/
#define ABC_TESTING_ASSERT_THROWS(type, expr) \
   do { \
      bool _bCaught(false); \
      try { \
         static_cast<void>(expr); \
      } catch (type const &) { \
         _bCaught = true; \
      } \
      this->assert_true(__FILE__, __LINE__, _bCaught, SL(#expr)); \
   } while (false)


/** Asserts that an expression evaluates to true.

expr
   Expression to evaulate.
*/
#define ABC_TESTING_ASSERT_TRUE(expr) \
   this->assert_true(__FILE__, __LINE__, expr, SL(#expr))


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
   to walk the entire list (ending when an item’s next pointer is nullptr).

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
      nullptr, \
      test_case_factory<cls>::factory \
   }; \
   \
   } /*namespace testing*/ \
   } /*namespace abc*/


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_TESTING_TEST_CASE_HXX

