/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_TESTING_RUNNER_HXX
#define _LOFTY_TESTING_RUNNER_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/collections/vector.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing {

//! Thrown to indicate that a test assertion failed, and the execution of the test case must be halted.
class LOFTY_TESTING_SYM assertion_error : public exception {
public:
   //! Default constructor.
   assertion_error();
};

}} //namespace lofty::testing

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing {

// Forward declarations.
class test_case;

//! Executes test cases.
class LOFTY_TESTING_SYM runner : public noncopyable {
public:
   /*! Groups assertion metadata, to reduce the number of arguments to log_assertion() and avoid repeated
   construction and destruction of string instances. */
   struct LOFTY_TESTING_SYM assertion_expr {
      /*! String representation of the evaluated expression (if binary_op is empty) or its left operand (if
      binary_op is not empty). */
      str left;
      //! Binary expression operator.
      str binary_op;
      //! String representation of the binary expression’s right operand.
      str right;
      //! true if the assertion was valid, or false otherwise.
      bool pass;

      /*! Assigns a new value to pass and oper.

      @param pass
         true if the assertion was valid, or false otherwise.
      @param binary_op
         Binary expression operator, or nullptr if the expression is not a binary operator.
      */
      void set(bool pass, char const * binary_op);
   };

public:
   /*! Constructor.

   @param ostream
      Pointer to the output stream that will be used to log the results of the tests.
   */
   runner(_std::shared_ptr<io::text::ostream> ostream);

   //! Destructor.
   ~runner();

   //! Loads all the test cases registered with LOFTY_TESTING_REGISTER_TEST_CASE() and prepares to run them.
   void load_registered_test_cases();

   /*! Logs an assertion.

   @param file_addr
      Location of the expression.
   @param expr
      Source representation of the expression being evaluated.
   @param assertion_expr
      Assertion metadata.
   */
   void log_assertion(
      text::file_address const & file_addr, str const & expr, assertion_expr * assertion_expr
   );

   /*! Prints test results based on the information collected by log_assertion() and run_test_case().

   @return
      true if all assertions were successful, or false otherwise.
   */
   bool log_summary();

   //! Executes each loaded test case.
   void run();

   /*! Executes a test case.

   @param test_case
      Test case to execute.
   */
   void run_test_case(class test_case & test_case);

private:
   //! Vector of loaded test test cases to be executed.
   collections::vector<_std::unique_ptr<test_case>> test_cases;
   //! Output stream.
   _std::shared_ptr<io::text::ostream> ostream;
   //! Total count of failed assertions.
   unsigned failed_assertions;
};

}} //namespace lofty::testing

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TESTING_RUNNER_HXX
