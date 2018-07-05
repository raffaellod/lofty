/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013-2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

/*! @file
Classes and macros to help write test cases and assertions. */

#ifndef _LOFTY_TESTING_TEST_CASE_HXX
#define _LOFTY_TESTING_TEST_CASE_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/testing/runner.hxx>
#include <lofty/to_str.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing {

//! Base class for test cases.
class LOFTY_TESTING_SYM test_case {
protected:
   /*! Provides a store() method for LOFTY_TESTING_ASSERT(expr) to invoke when expr is a binary operator.
   Since binary expressions are evaluated by expr_left_operand::operator??(), this class does absolutely
   nothing. */
   struct pre_stored_expr_result {
      //! Does nothing.
      void store() const {
      }
   };

   //! Holds the left operand of a binary expression, or a whole unary expression.
   template <typename Left>
   struct expr_left_operand {
      //! Pointer to the metadata to save results to.
      runner::assertion_expr * assertion_expr;
      //! Left operand of the binary expression, or unary expression operand.
      Left left;

      /*! Constructor.

      @param assertion_expr_
         Pointer to the metadata to save results to.
      @param left_
         Left operand of the binary expression, or unary expression operand.
      */
      expr_left_operand(runner::assertion_expr * assertion_expr_, Left left_) :
         assertion_expr(assertion_expr_),
         left(left_) {
      }

      //! Updates *assertion_expr for LOFTY_TESTING_ASSERT(expr) when expr is not a binary operator.
      void store() const {
         bool pass = left ? true : false;
         if (!pass) {
            assertion_expr->left = to_str(left);
         }
         assertion_expr->set(pass, false, nullptr);
      }

#define LOFTY_RELOP_IMPL(op) \
      /*! Applies the binary operator op, and updates *assertion_expr.

      @param right
         Right operand.
      @return
         *this.
      */ \
      template <typename Right> \
      pre_stored_expr_result operator op(Right const & right) const { \
         bool pass = left op right ? true : false; \
         assertion_expr->set(pass, true, LOFTY_SL(#op)); \
         if (!pass) { \
            assertion_expr->left = to_str(left); \
            assertion_expr->right = to_str(right); \
         } \
         return pre_stored_expr_result(); \
      }

      LOFTY_RELOP_IMPL(==)
      LOFTY_RELOP_IMPL(!=)
      LOFTY_RELOP_IMPL(<=)
      LOFTY_RELOP_IMPL(>=)
      LOFTY_RELOP_IMPL(<)
      LOFTY_RELOP_IMPL(>)
#undef LOFTY_RELOP_IMPL
   };

   /*! Breaks off the left operand from an expression by associating to it, and wraps it into an
   expr_left_operand instance. */
   struct LOFTY_TESTING_SYM expr_left_breaker {
      //! Pointer to the metadata to save results to.
      runner::assertion_expr * assertion_expr;

      /*! Constructor.

      @param assertion_expr_
         Pointer to the metadata to save results to.
      */
      explicit expr_left_breaker(runner::assertion_expr * assertion_expr_) :
         assertion_expr(assertion_expr_) {
      }

      /*! Steals the expression on the right (i.e. left operand of the expression that follows) by
      associativity.

      @param left
         Stolen operand.
      @return
         Wrapper for left.
      */
      template <typename Left>
      expr_left_operand<Left const &> operator<(Left const & left) {
         return expr_left_operand<Left const &>(assertion_expr, left);
      }
   };

public:
   //! Default constructor.
   test_case();

   //! Destructor.
   virtual ~test_case();

   /*! Initializes the object. Split into a method separated from the constructor so that derived classes
   don’t need to declare a constructor just to forward its arguments.

   @param runner
      Pointer to the test runner.
   */
   void init(class runner * runner);

   //! Executes the test case.
   virtual void run() = 0;

   /*! Returns a short description for the test case.

   @return
      Test case title.
   */
   virtual str title() = 0;

protected:
   /*! Implementation of LOFTY_TESTING_ASSERT().

   @param file_addr
      Location of the expression.
   @param expr
      Source representation of the expression being evaluated.
   */
   void assert(text::file_address const & file_addr, str const & expr);

   /*! Implementation of LOFTY_TESTING_ASSERT_DOES_NOT_THROW().

   @param file_addr
      Location of the expression.
   @param expr_fn
      Functor wrapping the expression to evaluate.
   @param expr
      Source representation of the expression being evaluated.
   */
   void assert_does_not_throw(
      text::file_address const & file_addr, _std::function<void ()> expr_fn, str const & expr
   );

   /*! Implementation of LOFTY_TESTING_ASSERT_EQUAL().

   @param file_addr
      Location of the expression.
   @param actual
      Actual value of the evaluated expression.
   @param equal
      Value that the expression should evaluate to.
   @param expr
      C++ code evaluating to actual.
   @param equal_expr
      C++ code evaluating to equal.
   */
   template <typename TExpr, typename TEqual>
   void assert_equal(
      text::file_address const & file_addr,
      TExpr const & actual, TEqual const & equal, str const & expr, str const & equal_expr
   ) {
      bool pass = (actual == equal);
      runner->log_assertion(
         file_addr, pass, expr, LOFTY_SL("== "),
         pass ? equal_expr : str(to_str(equal)), pass ? str::empty : str(to_str(actual))
      );
   }

   /*! Implementation of LOFTY_TESTING_ASSERT_FALSE().

   @param file_addr
      Location of the expression.
   @param actual
      Actual value of the evaluated expression.
   @param expr
      C++ code evaluating to actual.
   */
   void assert_false(text::file_address const & file_addr, bool actual, str const & expr);

   /*! Implementation of LOFTY_TESTING_ASSERT_GREATER().

   @param file_addr
      Location of the expression.
   @param actual
      Actual value of the evaluated expression.
   @param lower_bound
      Exclusive lower bound.
   @param expr
      C++ code evaluating to actual.
   @param lower_bound_expr
      C++ code evaluating to lower_bound.
   */
   template <typename TExpr, typename TLBound>
   void assert_greater(
      text::file_address const & file_addr,
      TExpr const & actual, TLBound const & lower_bound, str const & expr, str const & lower_bound_expr
   ) {
      bool pass = (actual > lower_bound);
      runner->log_assertion(
         file_addr, pass, expr, LOFTY_SL("> "),
         pass ? lower_bound_expr : str(to_str(lower_bound)), pass ? str::empty : str(to_str(actual))
      );
   }

   /*! Implementation of LOFTY_TESTING_ASSERT_GREATER_EQUAL().

   @param file_addr
      Location of the expression.
   @param actual
      Actual value of the evaluated expression.
   @param lower_bound
      Inclusive lower bound.
   @param expr
      C++ code evaluating to actual.
   @param lower_bound_expr
      C++ code evaluating to lower_bound.
   */
   template <typename TExpr, typename TLBound>
   void assert_greater_equal(
      text::file_address const & file_addr,
      TExpr const & actual, TLBound const & lower_bound, str const & expr, str const & lower_bound_expr
   ) {
      bool pass = (actual >= lower_bound);
      runner->log_assertion(
         file_addr, pass, expr, LOFTY_SL(">= "),
         pass ? lower_bound_expr : str(to_str(lower_bound)), pass ? str::empty : str(to_str(actual))
      );
   }

   /*! Implementation of LOFTY_TESTING_ASSERT_LESS().

   @param file_addr
      Location of the expression.
   @param actual
      Actual value of the evaluated expression.
   @param upper_bound
      Exclusive upper bound.
   @param expr
      C++ code evaluating to actual.
   @param upper_bound_expr
      C++ code evaluating to upper_bound.
   */
   template <typename TExpr, typename TUBound>
   void assert_less(
      text::file_address const & file_addr,
      TExpr const & actual, TUBound const & upper_bound, str const & expr, str const & upper_bound_expr
   ) {
      bool pass = (actual < upper_bound);
      runner->log_assertion(
         file_addr, pass, expr, LOFTY_SL("<= "),
         pass ? upper_bound_expr : str(to_str(upper_bound)), pass ? str::empty : str(to_str(actual))
      );
   }

   /*! Implementation of LOFTY_TESTING_ASSERT_LESS_EQUAL().

   @param file_addr
      Location of the expression.
   @param actual
      Actual value of the evaluated expression.
   @param upper_bound
      Inclusive upper bound.
   @param expr
      C++ code evaluating to actual.
   @param upper_bound_expr
      C++ code evaluating to upper_bound.
   */
   template <typename TExpr, typename TUBound>
   void assert_less_equal(
      text::file_address const & file_addr,
      TExpr const & actual, TUBound const & upper_bound, str const & expr, str const & upper_bound_expr
   ) {
      bool pass = (actual <= upper_bound);
      runner->log_assertion(
         file_addr, pass, expr, LOFTY_SL("<= "),
         pass ? upper_bound_expr : str(to_str(upper_bound)), pass ? str::empty : str(to_str(actual))
      );
   }

   /*! Implementation of LOFTY_TESTING_ASSERT_NOT_EQUAL().

   @param file_addr
      Location of the expression.
   @param actual
      Actual value of the evaluated expression.
   @param not_equal
      Value that the expression should not evaluate to.
   @param expr
      C++ code evaluating to actual.
   @param not_equal_expr
      C++ code evaluating to not_equal.
   */
   template <typename TExpr, typename TNotEqual>
   void assert_not_equal(
      text::file_address const & file_addr,
      TExpr const & actual, TNotEqual const & not_equal, str const & expr, str const & not_equal_expr
   ) {
      bool pass = (actual != not_equal);
      runner->log_assertion(
         file_addr, pass, expr, LOFTY_SL("!= "),
         pass ? not_equal_expr : str(to_str(not_equal)), pass ? str::empty : str(to_str(actual))
      );
   }

   /*! Implementation of LOFTY_TESTING_ASSERT_THROWS().

   @param file_addr
      Location of the expression.
   @param expr_fn
      Functor wrapping the expression to evaluate.
   @param expr
      Source representation of the expression being evaluated.
   @param instanceof_fn
      Functor that checks whether an std::exception instance is of the desired derived type.
   @param expected_type
      Expected exception type.
   */
   void assert_throws(
      text::file_address const & file_addr, _std::function<void ()> expr_fn, str const & expr,
      _std::function<bool (_std::exception const &)> instanceof_fn,
      _std::type_info const & expected_type
   );

   /*! Implementation of LOFTY_TESTING_ASSERT_TRUE().

   @param file_addr
      Location of the expression.
   @param actual
      Actual value of the evaluated expression.
   @param expr
      C++ code evaluating to actual.
   */
   void assert_true(text::file_address const & file_addr, bool actual, str const & expr);

protected:
   //! Runner executing this test.
   class runner * runner;
   //! Holds assertion metadata during LOFTY_TESTING_ASSERT*().
   runner::assertion_expr assertion_expr;
};

}} //namespace lofty::testing

//! @cond
#if LOFTY_HOST_CXX_GCC
   #define _LOFTY_TESTING_ASSERT_IGNORE_WARNINGS_BEGIN() \
      _Pragma("GCC diagnostic push") \
      _Pragma("GCC diagnostic ignored \"-Wparentheses\"")
   #define _LOFTY_TESTING_ASSERT_IGNORE_WARNINGS_END() \
      _Pragma("GCC diagnostic pop")
#else
   #define _LOFTY_TESTING_ASSERT_IGNORE_WARNINGS_BEGIN()
   #define _LOFTY_TESTING_ASSERT_IGNORE_WARNINGS_END()
#endif
//! @endcond

/*! Asserts that an expression is true.

@param expr
   Expression to evaluate.
*/
#define LOFTY_TESTING_ASSERT(expr) \
   do { \
      _LOFTY_TESTING_ASSERT_IGNORE_WARNINGS_BEGIN() \
      (test_case::expr_left_breaker(&this->assertion_expr) < expr).store(); \
      _LOFTY_TESTING_ASSERT_IGNORE_WARNINGS_END() \
      this->assert(LOFTY_THIS_FILE_ADDRESS(), LOFTY_SL(#expr)); \
   } while (false)

/*! Asserts that an expression does not throw.

@param expr
   Expression to evaluate.
*/
#define LOFTY_TESTING_ASSERT_DOES_NOT_THROW(expr) \
   /* Wrap the expression to evaluate in a lambda with access to any variable in the scope. */ \
   this->assert_does_not_throw(LOFTY_THIS_FILE_ADDRESS(), [&] () { \
      static_cast<void>(expr); \
   }, LOFTY_SL(#expr))

/*! Asserts that the value of an expression equals a specific value.

@param expr
   Expression to evaluate.
@param value
   Value that expr should evaluate to.
*/
#define LOFTY_TESTING_ASSERT_EQUAL(expr, value) \
   this->assert_equal(LOFTY_THIS_FILE_ADDRESS(), (expr), value, LOFTY_SL(#expr), LOFTY_SL(#value))

/*! Asserts that an expression evaluates to false.

@param expr
   Expression to evaulate.
*/
#define LOFTY_TESTING_ASSERT_FALSE(expr) \
   this->assert_false(LOFTY_THIS_FILE_ADDRESS(), (expr) ? true : false, LOFTY_SL(#expr))

/*! Asserts that the value of an expression is strictly greater than a specific lower bound.

@param expr
   Expression to evaluate.
@param lower_bound
   Exclusive lower bound.
*/
#define LOFTY_TESTING_ASSERT_GREATER(expr, lower_bound) \
   this->assert_greater( \
      LOFTY_THIS_FILE_ADDRESS(), (expr), lower_bound, LOFTY_SL(#expr), LOFTY_SL(#lower_bound) \
   )

/*! Asserts that the value of an expression is greater-than or equal-to a specific lower bound.

@param expr
   Expression to evaluate.
@param lower_bound
   Inclusive lower bound.
*/
#define LOFTY_TESTING_ASSERT_GREATER_EQUAL(expr, lower_bound) \
   this->assert_greater_equal( \
      LOFTY_THIS_FILE_ADDRESS(), (expr), lower_bound, LOFTY_SL(#expr), LOFTY_SL(#lower_bound) \
   )

/*! Asserts that the value of an expression is strictly less than a specific upper bound.

@param expr
   Expression to evaluate.
@param upper_bound
   Exclusive upper bound.
*/
#define LOFTY_TESTING_ASSERT_LESS(expr, upper_bound) \
   this->assert_less_equal( \
      LOFTY_THIS_FILE_ADDRESS(), (expr), expected, LOFTY_SL(#expr), LOFTY_SL(#upper_bound) \
   )

/*! Asserts that the value of an expression is less-than or equal-to a specific upper bound.

@param expr
   Expression to evaluate.
@param upper_bound
   Inclusive upper bound.
*/
#define LOFTY_TESTING_ASSERT_LESS_EQUAL(expr, upper_bound) \
   this->assert_less_equal( \
      LOFTY_THIS_FILE_ADDRESS(), (expr), upper_bound, LOFTY_SL(#expr), LOFTY_SL(#upper_bound) \
   )

/*! Asserts that the value of an expression differs from a specific value.

@param expr
   Expression to evaluate.
@param value
   Value that expr should not evaluate to.
*/
#define LOFTY_TESTING_ASSERT_NOT_EQUAL(expr, value) \
   this->assert_not_equal(LOFTY_THIS_FILE_ADDRESS(), (expr), value, LOFTY_SL(#expr), LOFTY_SL(#value))

/*! Asserts that an expression throws a specific type of exception.

@param type
   Exception class that should be caught.
@param expr
   Expression to evaluate.
*/
#define LOFTY_TESTING_ASSERT_THROWS(type, expr) \
   /* Wrap the expression to evaluate in a lambda with access to any variable in the scope; also wrap the
   dynamic_cast in a lambda, so assert_throws() doesn’t need to be a template to catch the desired type of
   exception. */ \
   this->assert_throws(LOFTY_THIS_FILE_ADDRESS(), [&] () { \
      static_cast<void>(expr); \
   }, LOFTY_SL(#expr), [] (::lofty::_std::exception const & x) -> bool { \
      return dynamic_cast<type const *>(&x) != nullptr; \
   }, typeid(type))

/*! Asserts that an expression evaluates to true.

@param expr
   Expression to evaulate.
*/
#define LOFTY_TESTING_ASSERT_TRUE(expr) \
   this->assert_true(LOFTY_THIS_FILE_ADDRESS(), (expr) ? true : false, LOFTY_SL(#expr))

/*! Declares and opens the definition of a simple test case, consisting in a single function with a unique
name.

@param name
   Name of the lofty::testing::test_case subclass.
@param test_title
   Title of the test, as a string literal.
*/
#define LOFTY_TESTING_TEST_CASE_FUNC(name, test_title) \
   class name : public ::lofty::testing::test_case { \
   public: \
      /*! See lofty::testing::test_case::title(). */ \
      virtual ::lofty::str title() override { \
         return ::lofty::str(LOFTY_SL(test_title)); \
      } \
   \
      /*! See lofty::testing::test_case::run(). */ \
      virtual void run() override; \
   }; \
   \
   LOFTY_TESTING_REGISTER_TEST_CASE(name) \
   \
   /*virtual*/ void name::run() /*override*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing {

// Forward declaration.
class test_case_factory_impl;

/*! List of lofty::testing::test_case-derived classes that can be used by a lofty::testing::runner instance to
instantiate and execute each test case. */
class LOFTY_TESTING_SYM test_case_factory_list :
   public collections::static_list<test_case_factory_list, test_case_factory_impl> {
public:
   /*! Returns the one and only instance of this class.

   @return
      *this.
   */
   static test_case_factory_list & instance() {
      return static_cast<test_case_factory_list &>(data_members_);
   }

private:
   //! Only instance of this class’ data.
   static data_members data_members_;
};

}} //namespace lofty::testing

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing {

//! Non-template base class for test_case_factory.
class LOFTY_TESTING_SYM test_case_factory_impl :
   public collections::static_list<test_case_factory_list, test_case_factory_impl>::node {
public:
   /*! Constructor.

   @param factory_
      Pointer to the derived class’s factory function.
   */
   test_case_factory_impl(_std::unique_ptr<test_case> (* factory_)(class runner * runner)) :
      factory(factory_) {
   }

   /*! Factory of lofty::testing::test_case instances.

   @param runner
      Runner to be used by the test case.
   @return
      Test case instance.
   */
   _std::unique_ptr<test_case> (* const factory)(class runner * runner);
};

}} //namespace lofty::testing

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing {

/*! Template version of lofty::testing::test_case_factory_impl, able to instantiate classes derived from
lofty::testing::test_case. */
template <class T>
class test_case_factory : public test_case_factory_impl {
public:
   //! Default constructor.
   test_case_factory() :
      test_case_factory_impl(&static_factory) {
   }

private:
   /*! Class factory for T.

   @param runner
      Runner to provide to the test case.
   */
   static _std::unique_ptr<test_case> static_factory(class runner * runner) {
      _std::unique_ptr<T> t(new T());
      t->init(runner);
      return _std::move(t);
   }
};

}} //namespace lofty::testing


/*! Registers a lofty::testing::test_case-derived class for execution by a lofty::testing::runner instance.

@param cls
   Test case class.
*/
#define LOFTY_TESTING_REGISTER_TEST_CASE(cls) \
   static ::lofty::testing::test_case_factory<cls> LOFTY_CPP_APPEND_UID(__test_case_factory_);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TESTING_TEST_CASE_HXX
