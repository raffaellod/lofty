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

#include <abc/testing/core.hxx>
#include <abc/testing/runner.hxx>
#include <abc/testing/unit.hxx>
#include <abc/trace.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::assertion_error


namespace abc {

namespace testing {

assertion_error::assertion_error() :
	exception() {
	m_pszWhat = "abc::assertion_error";
}

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::runner


namespace abc {

namespace testing {

runner::runner(std::shared_ptr<ostream> posOut) :
	m_pos(std::move(posOut)),
	m_cTotalUnits(0),
	m_cPassedUnits(0),
	m_cTotalTests(0),
	m_cPassedTests(0) {
}


runner::~runner() {
	// TODO: currently abc::*vector containers don’t support move-only types; remove this manual
	// cleanup code when std::unique_ptr becomes supported.
	for (auto it(m_vptc.begin()); it != m_vptc.end(); ++it) {
		delete *it;
	}
}


void runner::load_registered_units() {
	ABC_TRACE_FN((this));

	for (
		test_case_factory_impl::factory_list_item * pfli =
			test_case_factory_impl::get_factory_list_head();
		pfli;
		pfli = pfli->pfliNext
	) {
		// Instantiate the test case.
		auto punit(pfli->pfnFactory(this));
		// TODO: currently abc::*vector containers don’t support move-only types; change to use
		// std::unique_ptr when that becomes supported.
		m_vptc.append(punit.release());
//		m_vptc.append(std::move(punit));
	}
}


void runner::log_result(bool bSuccess, istr const & sExpr) {
	ABC_TRACE_FN((this, bSuccess, sExpr));

	if (!bSuccess /*|| verbose*/) {
		m_pos->print(SL("{}: {}\n"), bSuccess ? SL("Pass") : SL("Fail"), sExpr);
	}
	if (bSuccess) {
		++m_cPassedTests;
	}
	++m_cTotalTests;
}


bool runner::log_summary() {
	ABC_TRACE_FN((this));

	if (m_cTotalTests == 0) {
		m_pos->write(SL("No tests performed\n"));
	} else {
		m_pos->print(
			SL("Test cases summary: ")
			SL("{} executed, ")
			SL("{} passed ({}%), ")
			SL("{} failed ({}%)\n"),

			m_cTotalUnits,
			m_cPassedUnits,
			m_cPassedUnits * 100 / m_cTotalUnits,
			m_cTotalUnits - m_cPassedUnits,
			((m_cTotalUnits - m_cPassedUnits) * 100 + 1) / m_cTotalUnits
		);
		m_pos->print(
			SL("Assertions summary: ")
			SL("{} performed, ")
			SL("{} passed ({}%), ")
			SL("{} failed ({}%)\n"),

			m_cTotalTests,
			m_cPassedTests,
			m_cPassedTests * 100 / m_cTotalTests,
			m_cTotalTests - m_cPassedTests,
			((m_cTotalTests - m_cPassedTests) * 100 + 1) / m_cTotalTests
		);
	}
	return m_cPassedTests == m_cTotalTests;
}


void runner::run() {
	ABC_TRACE_FN((this));

	for (auto it(m_vptc.begin()); it != m_vptc.end(); ++it) {
		run_test_case(**it);
	}
}


void runner::run_test_case(test_case & tc) {
	ABC_TRACE_FN((this/*, u*/));

	m_pos->print(SL("Running test case \"{}\" ...\n"), tc.title());

	// Save the current total and passed counts, so we can compare them after running the test case.
	unsigned cPrevTotalTests(m_cTotalTests), cPrevPassedTests(m_cPassedTests);
	bool bPassed(false);
	try {
		tc.run();
		// If both the total and the passed count increased, the test case passed.
		if (cPrevTotalTests - m_cTotalTests == cPrevPassedTests - m_cPassedTests) {
			bPassed = true;
			++m_cPassedUnits;
		}
	} catch (assertion_error const &) {
		// This exception type is only used to interrupt abc::testing::test_case::run().
		m_pos->write(SL("Test case execution interrupted\n"));
	} catch (std::exception const & x) {
		exception::write_with_scope_trace(m_pos.get(), &x);
	} catch (...) {
		exception::write_with_scope_trace(m_pos.get());
	}
	++m_cTotalUnits;

	m_pos->print(
		SL("Completed test case \"{}\": {}\n"), tc.title(), bPassed ? SL("pass") : SL("fail")
	);
}

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

