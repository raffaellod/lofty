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

#ifndef ABC_TESTING_RUNNER_HXX
#define ABC_TESTING_RUNNER_HXX

#include <abc/testing/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abc/iostream.hxx>
#include <abc/vector.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::assertion_error


namespace abc {

namespace testing {

/** Thrown to indicate that a test assertion failed, and the execution of the test case must be
halted.
*/
class ABCTESTINGAPI assertion_error :
   public virtual exception {
public:

   /** Constructor.
   */
   assertion_error();
};

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::runner


namespace abc {

namespace testing {

// Forward declarations.
class test_case;


/** Executes test cases.
*/
class ABCTESTINGAPI runner {
public:

   /** Constructor.
   */
   runner(std::shared_ptr<ostream> posOut);


   /** Destructor.
   */
   ~runner();


   /** Loads all the test cases registered with ABC_TESTING_REGISTER_TEST_CASE() and prepares to run
   them.
   */
   void load_registered_test_cases();


   /** Logs the result of a test.
   */
   void log_result(bool bSuccess, istr const & sExpr);


   /** Prints test results based on the information collected by log_result() and run_test_case().

   return
      true if all assertions were successful, or false otherwise.
   */
   bool log_summary();


   /** Executes each loaded test case.
   */
   void run();


   /** Executes a test case.
   */
   void run_test_case(test_case & tc);


private:

   /** Vector of loaded test test cases to be executed. */
   // TODO: currently abc::*vector containers don’t support move-only types; change to use
   // std::unique_ptr when that becomes supported.
   dmvector<test_case *> m_vptc;
// dmvector<std::unique_ptr<test_case>> m_vptc;
   /** Output stream. */
   std::shared_ptr<ostream> m_pos;
   /** Total count of test cases executed. */
   unsigned m_cTotalTestCases;
   /** Total count of successful test cases. */
   unsigned m_cPassedTestCases;
   /** Total count of assertions performed. */
   unsigned m_cTotalAssertions;
   /** Total count of successful assertions. */
   unsigned m_cPassedAssertions;
};

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_TESTING_RUNNER_HXX

