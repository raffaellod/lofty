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

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_TESTING_TEST_CASE_HXX
#endif

#ifndef _LOFTY_TESTING_TEST_CASE_HXX_NOPUB
#define _LOFTY_TESTING_TEST_CASE_HXX_NOPUB

#include <lofty/_std/exception.hxx>
#include <lofty/_std/functional.hxx>
#include <lofty/_std/memory.hxx>
#include <lofty/_std/utility.hxx>
#include <lofty/testing/runner.hxx>
#include <lofty/to_str.hxx>

#if LOFTY_HOST_CXX_MSC
   // At some point, assert() gets defined; make sure it’s not.
   #undef assert
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing {
_LOFTY_PUBNS_BEGIN

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
            assertion_expr->left = lofty::_pub::to_str(left);
         }
         assertion_expr->set(pass, nullptr);
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
         assertion_expr->set(pass, LOFTY_SL(#op)); \
         if (!pass) { \
            assertion_expr->left = lofty::_pub::to_str(left); \
            assertion_expr->right = lofty::_pub::to_str(right); \
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
   virtual text::_LOFTY_PUBNS str title() = 0;

protected:
   /*! Implementation of LOFTY_TESTING_ASSERT().

   @param file_addr
      Location of the expression.
   @param expr
      Source representation of the expression being evaluated.
   */
   void assert(text::_LOFTY_PUBNS file_address const & file_addr, text::_LOFTY_PUBNS str const & expr);

   /*! Implementation of LOFTY_TESTING_ASSERT_DOES_NOT_THROW().

   @param file_addr
      Location of the expression.
   @param expr
      Source representation of the expression being evaluated.
   @param expr_fn
      Functor wrapping the expression to evaluate.
   */
   void assert_does_not_throw(
      text::_LOFTY_PUBNS file_address const & file_addr, text::_LOFTY_PUBNS str const & expr,
      _std::_LOFTY_PUBNS function<void ()> expr_fn
   );

   /*! Implementation of LOFTY_TESTING_ASSERT_THROWS().

   @param file_addr
      Location of the expression.
   @param expr
      Source representation of the expression being evaluated.
   @param expr_instanceof_fn
      Functor wrapping the expression to evaluate. Called with a non-nullptr, if returns whether an
      std::exception instance is of the desired derived type.
   */
   void assert_throws(
      text::_LOFTY_PUBNS file_address const & file_addr, text::_LOFTY_PUBNS str const & expr,
      _std::_LOFTY_PUBNS function<bool (_std::_LOFTY_PUBNS exception const *)> expr_instanceof_fn
   );

protected:
   //! Runner executing this test.
   class runner * runner;
   //! Holds assertion metadata during LOFTY_TESTING_ASSERT*().
   runner::assertion_expr assertion_expr;
};

_LOFTY_PUBNS_END
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
   this->assert_does_not_throw( \
      LOFTY_THIS_FILE_ADDRESS(), LOFTY_SL(#expr) LOFTY_SL(" does not throw"), \
      /* Wrap the expression to evaluate in a lambda with access to any variable in the scope. */ \
      [&] () { \
         static_cast<void>(expr); \
      } \
   )

/*! Asserts that an expression throws a specific type of exception.

@param type
   Exception class that should be caught.
@param expr
   Expression to evaluate.
*/
#define LOFTY_TESTING_ASSERT_THROWS(type, expr) \
   this->assert_throws( \
      LOFTY_THIS_FILE_ADDRESS(), LOFTY_SL(#expr) LOFTY_SL(" throws ") LOFTY_SL(#type), \
      /* Wrap the expression to evaluate in a lambda with access to any variable in the scope. Also put in the
      lambda a dynamic_cast to check the type of an exception, so assert_throws() doesn’t need to be a
      template to do that. */ \
      [&] (::lofty::_std::_pub::exception const * x) -> bool { \
         if (x) { \
            return dynamic_cast<type const *>(x) != nullptr; \
         } \
         static_cast<void>(expr); \
         return false; \
      } \
   )

#ifndef LOFTY_TESTING_NO_SHORT_ASSERTS
   #define ASSERT                LOFTY_TESTING_ASSERT
   #define ASSERT_DOES_NOT_THROW LOFTY_TESTING_ASSERT_DOES_NOT_THROW
   #define ASSERT_THROWS         LOFTY_TESTING_ASSERT_THROWS
#endif

/*! Declares and opens the definition of a simple test case, consisting in a single function with a unique
name.

@param name
   Name of the lofty::testing::test_case subclass.
@param test_title
   Title of the test, as a string literal.
*/
#define LOFTY_TESTING_TEST_CASE_FUNC(name, test_title) \
   class name : public ::lofty::testing::_pub::test_case { \
   public: \
      /*! See lofty::testing::test_case::title(). */ \
      virtual ::lofty::text::_pub::str title() override { \
         return ::lofty::text::_pub::str(LOFTY_SL(test_title)); \
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
_LOFTY_PUBNS_BEGIN

// Forward declaration.
class test_case_factory_impl;

/*! List of lofty::testing::test_case-derived classes that can be used by a lofty::testing::runner instance to
instantiate and execute each test case. */
class LOFTY_TESTING_SYM test_case_factory_list :
   public collections::_LOFTY_PUBNS static_list<test_case_factory_list, test_case_factory_impl> {
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

_LOFTY_PUBNS_END
}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing {
_LOFTY_PUBNS_BEGIN

//! Non-template base class for test_case_factory.
class LOFTY_TESTING_SYM test_case_factory_impl :
   public collections::_LOFTY_PUBNS static_list<test_case_factory_list, test_case_factory_impl>::node {
public:
   /*! Constructor.

   @param factory_
      Pointer to the derived class’s factory function.
   */
   test_case_factory_impl(_std::_LOFTY_PUBNS unique_ptr<test_case> (* factory_)(class runner * runner)) :
      factory(factory_) {
   }

   /*! Factory of lofty::testing::test_case instances.

   @param runner
      Runner to be used by the test case.
   @return
      Test case instance.
   */
   _std::_LOFTY_PUBNS unique_ptr<test_case> (* const factory)(class runner * runner);
};

_LOFTY_PUBNS_END
}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing {
_LOFTY_PUBNS_BEGIN

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
   static _std::_LOFTY_PUBNS unique_ptr<test_case> static_factory(class runner * runner) {
      _std::_pub::unique_ptr<T> t(new T());
      t->init(runner);
      return _std::_pub::move(t);
   }
};

_LOFTY_PUBNS_END
}} //namespace lofty::testing

/*! Registers a lofty::testing::test_case-derived class for execution by a lofty::testing::runner instance.

@param cls
   Test case class.
*/
#define LOFTY_TESTING_REGISTER_TEST_CASE(cls) \
   static ::lofty::testing::test_case_factory<cls> LOFTY_CPP_APPEND_UID(__test_case_factory_);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TESTING_TEST_CASE_HXX_NOPUB

#ifdef _LOFTY_TESTING_TEST_CASE_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace testing {
      using _pub::test_case;
      using _pub::test_case_factory;
      using _pub::test_case_factory_impl;
      using _pub::test_case_factory_list;
   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_TESTING_TEST_CASE_HXX
