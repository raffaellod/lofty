/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013, 2014, 2015
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

#ifndef _ABACLADE_TESTING_RUNNER_HXX
#define _ABACLADE_TESTING_RUNNER_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace testing {

/*! Thrown to indicate that a test assertion failed, and the execution of the test case must be
halted. */
class ABACLADE_TESTING_SYM assertion_error : public virtual exception {
public:
   //! Default constructor.
   assertion_error();
};

}} //namespace abc::testing

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace testing {

// Forward declarations.
class test_case;

//! Executes test cases.
class ABACLADE_TESTING_SYM runner {
public:
   /*! Constructor.

   @param posOut
      Pointer to the writer that will be used to log the results of the tests.
   */
   runner(_std::shared_ptr<io::text::writer> ptwOut);

   //! Destructor.
   ~runner();

   /*! Loads all the test cases registered with ABC_TESTING_REGISTER_TEST_CASE() and prepares to run
   them. */
   void load_registered_test_cases();

   /*! Logs an assertion.

   @param srcloc
      Location of the expression.
   @param bPass
      true if the assertion was valid, or false otherwise.
   @param sExpr
      Source representation of the expression being evaluated.
   @param sOp
      Applied relational operator.
   @param sExpected
      If bPass, expression generating the expected value (i.e. the C++ expression, as a string); if
      !bPass, computed expected value (i.e. the actual value returned by the C++ expression, as a
      string).
   @param sActual
      Only used if !bPass, this is the computed actual value (i.e. return value of sExpr), as a
      string.
   */
   void log_assertion(
      source_location const & srcloc, bool bPass, istr const & sExpr, istr const & sOp,
      istr const & sExpected, istr const & sActual = istr::empty
   );

   /*! Prints test results based on the information collected by log_assertion() and
   run_test_case().

   @return
      true if all assertions were successful, or false otherwise.
   */
   bool log_summary();

   //! Executes each loaded test case.
   void run();

   /*! Executes a test case.

   @param tc
      Test case to execute.
   */
   void run_test_case(test_case & tc);

private:
   //! Vector of loaded test test cases to be executed.
   collections::dmvector<_std::unique_ptr<test_case>> m_vptc;
   //! Output writer.
   _std::shared_ptr<io::text::writer> m_ptwOut;
   //! Total count of failed assertions.
   unsigned m_cFailedAssertions;
};

}} //namespace abc::testing

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_TESTING_RUNNER_HXX
