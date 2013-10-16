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

#include <abc/testing/runner.hxx>
#include <abc/testing/unit.hxx>
#include <abc/trace.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::runner


namespace abc {

namespace testing {

runner::runner() {
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
}


void runner::run() {
	// TODO: implementation.
}

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

