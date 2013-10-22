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
	for (auto it(m_vpu.begin()); it != m_vpu.end(); ++it) {
		delete *it;
	}
}


void runner::load_registered_units() {
	for (
		unit_factory_impl::factory_list_item * pfli = unit_factory_impl::get_factory_list_head();
		pfli;
		pfli = pfli->pfliNext
	) {
		// Instantiate the unit test.
		auto punit(pfli->pfnFactory(this));
		// TODO: currently abc::*vector containers don’t support move-only types; change to use
		// std::unique_ptr when that becomes supported.
		m_vpu.append(punit.release());
//		m_vpu.append(std::move(punit));
	}
}


void runner::log_result(bool bSuccess, istr const & sExpr) {
	if (!bSuccess /*|| verbose*/) {
		m_pos->print(SL("{}: {}\n"), bSuccess ? SL("Pass") : SL("Fail"), sExpr);
	}
	if (bSuccess) {
		++m_cPassedTests;
	}
	++m_cTotalTests;
}


bool runner::log_summary() {
	if (m_cTotalTests == 0) {
		m_pos->write(SL("No tests performed\n"));
	} else {
		m_pos->print(
			SL("Units summary: ")
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
			SL("Tests summary: ")
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
	for (auto it(m_vpu.begin()); it != m_vpu.end(); ++it) {
		run_unit(**it);
	}
}


void runner::run_unit(unit & u) {
	m_pos->print(SL("Testing unit \"{}\" ...\n"), u.title());

	// Save the current total and passed counts, so we can compare them after running the unit.
	unsigned cPrevTotalTests(m_cTotalTests), cPrevPassedTests(m_cPassedTests);
	bool bPassed(false);
	try {
		u.run();
		// If both the total and the passed count increased, the unit passed.
		if (cPrevTotalTests - m_cTotalTests == cPrevPassedTests - m_cPassedTests) {
			bPassed = true;
			++m_cPassedUnits;
		}
	} catch (assertion_error const &) {
		// This exception type is only used to interrupt abc::testing::unit::run().
		m_pos->write(SL("Unit execution interrupted\n"));
	} catch (std::exception const & x) {
		exception::write_with_scope_trace(m_pos.get(), &x);
	} catch (...) {
		exception::write_with_scope_trace(m_pos.get());
	}
	++m_cTotalUnits;

	m_pos->print(SL("Completed unit \"{}\": {}\n"), u.title(), bPassed ? SL("pass") : SL("fail"));
}

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

