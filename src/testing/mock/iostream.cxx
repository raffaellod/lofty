/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

Copyright 2011, 2012, 2013
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
#include <abc/testing/mock/iostream.hxx>
#include <abc/trace.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::mock::istream


namespace abc {

namespace testing {

namespace mock {

ostream::ostream() :
	::abc::ostream(),
	m_cchUsed(0) {
}


bool ostream::contents_equal(istr const & sExpected) {
	ABC_TRACE_FN((this, sExpected));

	istr sActual(unsafe, m_achBuf, m_cchUsed);
	return sActual == sExpected;
}


/*virtual*/ void ostream::write_raw(
	void const * p, size_t cb, text::encoding enc /*= text::encoding::identity*/
) {
	ABC_TRACE_FN((this, p, cb, enc));

	ABC_UNUSED_ARG(enc);
	memory::copy<void>(m_achBuf + m_cchUsed, p, cb);
	m_cchUsed += cb / sizeof(m_achBuf[0]);
}

} //namespace mock

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

