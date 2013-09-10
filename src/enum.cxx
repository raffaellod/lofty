// -*- coding: utf-8; mode: c++; tab-width: 3 -*-
//--------------------------------------------------------------------------------------------------
// Application-Building Components
// Copyright 2010-2013 Raffaello D. Di Napoli
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

#include <abc/enum.hxx>
#include <abc/utf_traits.hxx>
#include <abc/iostream.hxx>
#include <abc/trace.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::enum_member


namespace abc {

enum_member const * enum_member::find_in_map(enum_member const * pem, int i) {
	abc_trace_fn((pem, i));

	for (; pem->pszName; ++pem) {
		if (i == pem->iValue) {
			return pem;
		}
	}
	// TODO: provide more information in the exception.
	abc_throw(domain_error());
}
enum_member const * enum_member::find_in_map(enum_member const * pem, char_t const * psz) {
	abc_trace_fn((pem, psz));

	for (; pem->pszName; ++pem) {
		if (text::utf_traits<>::str_cmp(psz, pem->pszName) == 0) {
			return pem;
		}
	}
	// TODO: provide more information in the exception.
	abc_throw(domain_error());
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_enum_to_string_backend_impl


namespace abc {

_enum_to_string_backend_impl::_enum_to_string_backend_impl(char_range const & crFormat) {
	abc_trace_fn((crFormat));

	auto it(crFormat.cbegin());

	// TODO: parse the format string.

	// If we still have any characters, they are garbage.
	if (it != crFormat.cend()) {
		abc_throw(syntax_error(
			SL("unexpected character"), crFormat, unsigned(it - crFormat.cbegin())
		));
	}
}


void _enum_to_string_backend_impl::write_impl(int i, enum_member const * pem, ostream * posOut) {
	abc_trace_fn((i, pem, posOut));

	enum_member const * petvp(enum_member::find_in_map(pem, i));
	// TODO: apply format options.
	*posOut << petvp->pszName;
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

