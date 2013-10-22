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
#include <abc/testing/unit.hxx>
#include <abc/trace.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::unit


namespace abc {

namespace testing {

unit::unit() {
}


/*virtual*/ unit::~unit() {
}


void unit::init(runner * prunner) {
	m_prunner = prunner;
}


void unit::assert(bool bExpr, istr const & sExpr) {
	m_prunner->log_result(bExpr, sExpr);
	if (!bExpr) {
		abc_throw(abc::testing::assertion_error, ());
	}
}


void unit::expect(bool bExpr, istr const & sExpr) {
	m_prunner->log_result(bExpr, sExpr);
}

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::unit_factory_impl


namespace abc {

namespace testing {

/*static*/ unit_factory_impl::factory_list_item * unit_factory_impl::sm_pfliHead(NULL);
// MSC16 BUG: for some reason, this will be parsed as a function declaration if written as a
// constructor call.
/*static*/ unit_factory_impl::factory_list_item ** unit_factory_impl::sm_ppfliTailNext = NULL;


unit_factory_impl::unit_factory_impl(factory_list_item * pfli) {
	if (sm_pfliHead) {
		// We have a head and therefore a tail as well: add *pfli as the new tail.
		*sm_ppfliTailNext = pfli;
	} else {
		// We don’t have a head yet: set it up now.
		sm_pfliHead = pfli;
	}
	// Save the “next” pointer of *pfli for the next call.
	sm_ppfliTailNext = &pfli->pfliNext;
}

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

