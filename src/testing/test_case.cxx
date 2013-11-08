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
#include <abc/testing/test_case.hxx>
#include <abc/trace.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::test_case


namespace abc {

namespace testing {

test_case::test_case() {
}


/*virtual*/ test_case::~test_case() {
}


void test_case::init(runner * prunner) {
	ABC_TRACE_FN((prunner));

	m_prunner = prunner;
}


bool test_case::assert(bool bExpr, istr const & sExpr) {
	ABC_TRACE_FN((this, bExpr, sExpr));

	m_prunner->log_result(bExpr, sExpr);
	/*if (!bExpr) {
		ABC_THROW(abc::testing::assertion_error, ());
	}*/
	return bExpr;
}

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::test_case_factory_impl


namespace abc {

namespace testing {

/*static*/ test_case_factory_impl::factory_list_item * test_case_factory_impl::sm_pfliHead(NULL);
// MSC16 BUG: for some reason, this will be parsed as a function declaration if written as a
// constructor call.
/*static*/ test_case_factory_impl::factory_list_item **
	test_case_factory_impl::sm_ppfliTailNext = NULL;


test_case_factory_impl::test_case_factory_impl(factory_list_item * pfli) {
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

