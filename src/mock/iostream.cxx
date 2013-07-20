// -*- coding: utf-8; mode: c++; tab-width: 3 -*-
//--------------------------------------------------------------------------------------------------
// Application-Building Components
// Copyright 2011-2013 Raffaello D. Di Napoli
//--------------------------------------------------------------------------------------------------
// This file is part of Application-Building Components (henceforth referred to as ABC).
//
// ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ABC. If not, see
// <http://www.gnu.org/licenses/>.
//--------------------------------------------------------------------------------------------------

#include <abc/mock/iostream.hxx>
#include <abc/trace.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::mock::istream


namespace abc {

namespace mock {

ostream::ostream() :
	::abc::ostream(),
	m_cchUsed(0) {
}


/// Returns true if the current contents of the stream match the specified string.
//
bool ostream::contents_equal(cstring const & sExpected) {
	abc_trace_fn((this, sExpected));

	cstring sActual(unsafe, m_achBuf, m_cchUsed);
	return sActual == sExpected;
}


/// See ostream::write().
//
/*virtual*/ void ostream::write(
	void const * p, size_t cb, text::encoding enc /*= text::encoding::identity*/
) {
	abc_trace_fn((this, p, cb, enc));

	UNUSED_ARG(enc);
	memory::copy<void>(m_achBuf + m_cchUsed, p, cb);
	m_cchUsed += cb / sizeof(m_achBuf[0]);
}

} //namespace mock

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

