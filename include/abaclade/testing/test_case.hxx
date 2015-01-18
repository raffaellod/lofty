/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013, 2014
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

#ifndef _ABACLADE_TESTING_TEST_CASE_HXX
#define _ABACLADE_TESTING_TEST_CASE_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/testing/runner.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::test_case

namespace abc {
namespace testing {

//! Base class for test cases.
class ABACLADE_TESTING_SYM test_case {
public:
   //! Constructor.
   test_case();

   //! Destructor.
   virtual ~test_case();

   /*! Initializes the object. Split into a method separated from the constructor so that derived
   classes don’t need to declare a constructor just to forward its arguments.

   @param prunner
      Pointer to the test runner.
   */
   void init(runner * prunner);

   //! Executes the test case.
   virtual void run() = 0;

   /*! Returns a short description for the test case.

   @return
      Test case title.
   */
   virtual istr title() = 0;

protected:
   /*! Implementation of ABC_TESTING_ASSERT_DOES_NOT_THROW.

   @param srcloc
      Location of the expression.
   @param fnExpr
      Functor wrapping the expression to evaluate.
   @param sExpr
      Source representation of the expression being evaluated.
   */
   void assert_does_not_throw(
      source_location const & srcloc, std::function<void ()> const & fnExpr, istr const & sExpr
   );

   /*! Implementation of ABC_TESTING_ASSERT_EQUAL.

   @param srcloc
      Location of the expression.
   @param tActual
      Actual value of the evaluated expression.
   @param tEqual
      Value that the expression should evaluate to.
   @param sExpr
      C++ code evaluating to tActual.
   @param sEqual
      C++ code evaluating to tEqual.
   */
   template <typename TExpr, typename TEqual>
   void assert_equal(
      source_location const & srcloc,
      TExpr const & tActual, TEqual const & tEqual, istr const & sExpr, istr const & sEqual
   ) {
      bool bPass = (tActual == tEqual);
      m_prunner->log_assertion(
         srcloc, bPass, sExpr, ABC_SL("== "),
         bPass ? sEqual : istr(to_str(tEqual)), bPass ? istr::empty : istr(to_str(tActual))
      );
   }

   /*! Implementation of ABC_TESTING_ASSERT_FALSE.

   @param srcloc
      Location of the expression.
   @param bActual
      Actual value of the evaluated expression.
   @param sExpr
      C++ code evaluating to bActual.
   */
   void assert_false(source_location const & srcloc, bool bActual, istr const & sExpr);

   /*! Implementation of ABC_TESTING_ASSERT_GREATER.

   @param srcloc
      Location of the expression.
   @param tActual
      Actual value of the evaluated expression.
   @param tLBound
      Exclusive lower bound.
   @param sExpr
      C++ code evaluating to tActual.
   @param sLBound
      C++ code evaluating to tLBound.
   */
   template <typename TExpr, typename TLBound>
   void assert_greater(
      source_location const & srcloc,
      TExpr const & tActual, TLBound const & tLBound, istr const & sExpr, istr const & sLBound
   ) {
      bool bPass = (tActual > tLBound);
      m_prunner->log_assertion(
         srcloc, bPass, sExpr, ABC_SL("> "),
         bPass ? sLBound : istr(to_str(tLBound)), bPass ? istr::empty : istr(to_str(tActual))
      );
   }

   /*! Implementation of ABC_TESTING_ASSERT_GREATER_EQUAL.

   @param srcloc
      Location of the expression.
   @param tActual
      Actual value of the evaluated expression.
   @param tLBound
      Inclusive lower bound.
   @param sExpr
      C++ code evaluating to tActual.
   @param sLBound
      C++ code evaluating to tLBound.
   */
   template <typename TExpr, typename TLBound>
   void assert_greater_equal(
      source_location const & srcloc,
      TExpr const & tActual, TLBound const & tLBound, istr const & sExpr, istr const & sLBound
   ) {
      bool bPass = (tActual >= tLBound);
      m_prunner->log_assertion(
         srcloc, bPass, sExpr, ABC_SL(">= "),
         bPass ? sLBound : istr(to_str(tLBound)), bPass ? istr::empty : istr(to_str(tActual))
      );
   }

   /*! Implementation of ABC_TESTING_ASSERT_LESS.

   @param srcloc
      Location of the expression.
   @param tActual
      Actual value of the evaluated expression.
   @param tUBound
      Exclusive upper bound.
   @param sExpr
      C++ code evaluating to tActual.
   @param sUBound
      C++ code evaluating to tUBound.
   */
   template <typename TExpr, typename TUBound>
   void assert_less(
      source_location const & srcloc,
      TExpr const & tActual, TUBound const & tUBound, istr const & sExpr, istr const & sUBound
   ) {
      bool bPass = (tActual < tUBound);
      m_prunner->log_assertion(
         srcloc, bPass, sExpr, ABC_SL("<= "),
         bPass ? sUBound : istr(to_str(tUBound)), bPass ? istr::empty : istr(to_str(tActual))
      );
   }

   /*! Implementation of ABC_TESTING_ASSERT_LESS_EQUAL.

   @param srcloc
      Location of the expression.
   @param tActual
      Actual value of the evaluated expression.
   @param tUBound
      Inclusive upper bound.
   @param sExpr
      C++ code evaluating to tActual.
   @param sUBound
      C++ code evaluating to tUBound.
   */
   template <typename TExpr, typename TUBound>
   void assert_less_equal(
      source_location const & srcloc,
      TExpr const & tActual, TUBound const & tUBound, istr const & sExpr, istr const & sUBound
   ) {
      bool bPass = (tActual <= tUBound);
      m_prunner->log_assertion(
         srcloc, bPass, sExpr, ABC_SL("<= "),
         bPass ? sUBound : istr(to_str(tUBound)), bPass ? istr::empty : istr(to_str(tActual))
      );
   }

   /*! Implementation of ABC_TESTING_ASSERT_NOT_EQUAL.

   @param srcloc
      Location of the expression.
   @param tActual
      Actual value of the evaluated expression.
   @param tNotEqual
      Value that the expression should not evaluate to.
   @param sExpr
      C++ code evaluating to tActual.
   @param sNotEqual
      C++ code evaluating to tNotEqual.
   */
   template <typename TExpr, typename TNotEqual>
   void assert_not_equal(
      source_location const & srcloc,
      TExpr const & tActual, TNotEqual const & tNotEqual, istr const & sExpr, istr const & sNotEqual
   ) {
      bool bPass = (tActual != tNotEqual);
      m_prunner->log_assertion(
         srcloc, bPass, sExpr, ABC_SL("!= "),
         bPass ? sNotEqual : istr(to_str(tNotEqual)), bPass ? istr::empty : istr(to_str(tActual))
      );
   }

   /*! Implementation of ABC_TESTING_ASSERT_THROWS.

   @param srcloc
      Location of the expression.
   @param fnExpr
      Functor wrapping the expression to evaluate.
   @param sExpr
      Source representation of the expression being evaluated.
   @param fnMatchType
      Functor that checks whether an std::exception instance is of the desired derived type.
   @param pszExpectedWhat
      Return value of std::exception::what(), as overridden by the desired derived class.
   */
   void assert_throws(
      source_location const & srcloc, std::function<void ()> const & fnExpr, istr const & sExpr,
      std::function<bool (std::exception const &)> const & fnMatchType, char const * pszExpectedWhat
   );

   /*! Implementation of ABC_TESTING_ASSERT_TRUE.

   @param srcloc
      Location of the expression.
   @param bActual
      Actual value of the evaluated expression.
   @param sExpr
      C++ code evaluating to bActual.
   */
   void assert_true(source_location const & srcloc, bool bActual, istr const & sExpr);

protected:
   //! Runner executing this test.
   runner * m_prunner;
};

} //namespace testing
} //namespace abc

/*! Asserts that an expression does not throw.

@param expr
   Expression to evaluate.
*/
#define ABC_TESTING_ASSERT_DOES_NOT_THROW(expr) \
   /* Wrap the expression to evaluate in a lambda with access to any variable in the scope. */ \
   this->assert_does_not_throw(ABC_SOURCE_LOCATION(), [&] () -> void { \
      static_cast<void>(expr); \
   }, ABC_SL(#expr))

/*! Asserts that the value of an expression equals a specific value.

@param expr
   Expression to evaluate.
@param value
   Value that expr should evaluate to.
*/
#define ABC_TESTING_ASSERT_EQUAL(expr, value) \
   this->assert_equal(ABC_SOURCE_LOCATION(), (expr), value, ABC_SL(#expr), ABC_SL(#value))

/*! Asserts that an expression evaluates to false.

@param expr
   Expression to evaulate.
*/
#define ABC_TESTING_ASSERT_FALSE(expr) \
   this->assert_false(ABC_SOURCE_LOCATION(), (expr), ABC_SL(#expr))

/*! Asserts that the value of an expression is strictly greater than a specific lower bound.

@param expr
   Expression to evaluate.
@param lbound
   Exclusive lower bound.
*/
#define ABC_TESTING_ASSERT_GREATER(expr, lbound) \
   this->assert_greater(ABC_SOURCE_LOCATION(), (expr), lbound, ABC_SL(#expr), ABC_SL(#lbound))

/*! Asserts that the value of an expression is greater-than or equal-to a specific lower bound.

@param expr
   Expression to evaluate.
@param lbound
   Inclusive lower bound.
*/
#define ABC_TESTING_ASSERT_GREATER_EQUAL(expr, lbound) \
   this->assert_greater_equal(ABC_SOURCE_LOCATION(), (expr), lbound, ABC_SL(#expr), ABC_SL(#lbound))

/*! Asserts that the value of an expression is strictly less than a specific upper bound.

@param expr
   Expression to evaluate.
@param ubound
   Exclusive upper bound.
*/
#define ABC_TESTING_ASSERT_LESS(expr, ubound) \
   this->assert_less_equal(ABC_SOURCE_LOCATION(), (expr), expected, ABC_SL(#expr), ABC_SL(#ubound))

/*! Asserts that the value of an expression is less-than or equal-to a specific upper bound.

@param expr
   Expression to evaluate.
@param ubound
   Inclusive upper bound.
*/
#define ABC_TESTING_ASSERT_LESS_EQUAL(expr, ubound) \
   this->assert_less_equal(ABC_SOURCE_LOCATION(), (expr), ubound, ABC_SL(#expr), ABC_SL(#ubound))

/*! Asserts that the value of an expression differs from a specific value.

@param expr
   Expression to evaluate.
@param value
   Value that expr should not evaluate to.
*/
#define ABC_TESTING_ASSERT_NOT_EQUAL(expr, value) \
   this->assert_not_equal(ABC_SOURCE_LOCATION(), (expr), value, ABC_SL(#expr), ABC_SL(#value))

/*! Asserts that an expression throws a specific type of exception.

@param type
   Exception class that should be caught.
@param expr
   Expression to evaluate.
*/
#define ABC_TESTING_ASSERT_THROWS(type, expr) \
   /* Wrap the expression to evaluate in a lambda with access to any variable in the scope; also
   wrap the dynamic_cast in a lambda, so the caller doesn’t need to be a template to catch the
   desired type of exception. */ \
   this->assert_throws(ABC_SOURCE_LOCATION(), [&] () -> void { \
      static_cast<void>(expr); \
   }, ABC_SL(#expr), [] (::std::exception const & x) -> bool { \
      return dynamic_cast<type const *>(&x) != nullptr; \
   }, type().what())

/*! Asserts that an expression evaluates to true.

@param expr
   Expression to evaulate.
*/
#define ABC_TESTING_ASSERT_TRUE(expr) \
   this->assert_true(ABC_SOURCE_LOCATION(), (expr), ABC_SL(#expr))

/*! Declares and defines a simple test case, consisting of a single function the body of which must
follow the invocation of this macro.

@param name
   Name of the abc::testing::test_case subclass.
@param test_title
   Title of the test, as a string literal.
*/
#define ABC_TESTING_TEST_CASE_FUNC(name, test_title) \
   class name : public ::abc::testing::test_case { \
   public: \
      /*! See abc::testing::test_case::title(). */ \
      virtual ::abc::istr title() override { \
         return ::abc::istr(ABC_SL(test_title)); \
      } \
   \
      /*! See abc::testing::test_case::run(). */ \
      virtual void run() override; \
   }; \
   \
   ABC_TESTING_REGISTER_TEST_CASE(name) \
   \
   /*virtual*/ void name::run() /*override*/

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::test_case_factory_list

namespace abc {
namespace testing {

// Forward declaration.
class test_case_factory_impl;

/*! List of abc::testing::test_case-derived classes that can be used by an abc::testing::runner
instance to instantiate and execute each test case. */
class ABACLADE_TESTING_SYM test_case_factory_list :
   public static_list<test_case_factory_list, test_case_factory_impl> {
public:
   ABC_STATIC_LIST_DECLARE_SUBCLASS_STATIC_MEMBERS(test_case_factory_list)
};

} //namespace testing
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::test_case_factory_impl

namespace abc {
namespace testing {

//! Non-template base class for test_case_factory.
class ABACLADE_TESTING_SYM test_case_factory_impl :
   public static_list<test_case_factory_list, test_case_factory_impl>::node {
public:
   /*! Constructor.

   @param pfnFactory
      Pointer to the derived class’s factory function.
   */
   test_case_factory_impl(std::unique_ptr<test_case> (* pfnFactory)(runner * prunner)) :
      factory(pfnFactory) {
   }

   /*! Factory of abc::testing::test_case instances.

   @param prunner
      Runner to be used by the test case.
   @return
      Test case instance.
   */
   std::unique_ptr<test_case> (* const factory)(runner * prunner);
};

} //namespace testing
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::test_case_factory

namespace abc {
namespace testing {

/*! Template version of abc::testing::test_case_factory_impl, able to instantiate classes derived
from abc::testing::test_case. */
template <class T>
class test_case_factory : public test_case_factory_impl {
public:
   //! Constructor.
   test_case_factory() :
      test_case_factory_impl(&static_factory) {
   }

private:
   /*! Class factory for T.

   @param prunner
      Runner to provide to the test case.
   */
   static std::unique_ptr<test_case> static_factory(runner * prunner) {
      std::unique_ptr<T> pt(new T());
      pt->init(prunner);
      return std::move(pt);
   }
};

} //namespace testing
} //namespace abc


/*! Registers an abc::testing::test_case-derived class for execution by an abc::testing::runner
instance.

@param cls
   Test case class.
*/
#define ABC_TESTING_REGISTER_TEST_CASE(cls) \
   namespace { \
   \
   ::abc::testing::test_case_factory<cls> ABC_CPP_APPEND_UID(g__test_case_factory_); \
   \
   } /*namespace*/

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_TESTING_TEST_CASE_HXX
