/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

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

/** Thrown to indicate that a test assertion failed, and the execution of the unit must be halted.
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
class unit;


/** Executes unit tests.
*/
class ABCTESTINGAPI runner {
public:

	/** Constructor.
	*/
	runner(std::shared_ptr<ostream> posOut);


	/** Destructor.
	*/
	~runner();


	/** Loads all the units registered with ABC_TESTING_UNIT_REGISTER() and prepares to run them.
	*/
	void load_registered_units();


	/** Logs the result of a test.
	*/
	void log_result(bool bSuccess, istr const & sExpr);


	/** Prints test results based on the results collected by process_logs().

	return
		true if all tests passed, or false otherwise.
	*/
	bool log_summary();


	/** Executes each loaded unit test.
	*/
	void run();


	/** Executes a unit test.
	*/
	void run_unit(unit & u);


private:

	/** Vector of loaded test units to be executed. */
	// TODO: currently abc::*vector containers don’t support move-only types; change to use
	// std::unique_ptr when that becomes supported.
	dmvector<unit *> m_vpu;
//	dmvector<std::unique_ptr<unit>> m_vpu;
	/** Output stream. */
	std::shared_ptr<ostream> m_pos;
	/** Total count of units executed. */
	unsigned m_cTotalUnits;
	/** Total count of successful units. */
	unsigned m_cPassedUnits;
	/** Total count of assertion/expectation tests executed. */
	unsigned m_cTotalTests;
	/** Total count of successful assertion/expectation tests. */
	unsigned m_cPassedTests;
};

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_TESTING_RUNNER_HXX

