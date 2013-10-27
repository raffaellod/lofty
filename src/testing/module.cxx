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
#include <abc/testing/module.hxx>
#include <abc/testing/runner.hxx>
#include <abc/file_iostream.hxx>
#include <abc/trace.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::module


namespace abc {

namespace testing {

ABCTESTINGAPI int app_module::main(mvector<istr const> const & vsArgs) {
	ABC_TRACE_FN((this/*, vsArgs*/));

	ABC_UNUSED_ARG(vsArgs);

	runner r(file_ostream::stderr());
	r.load_registered_units();
	r.run();
	bool bAllPassed(r.log_summary());

	return bAllPassed ? 0 : 1;
}

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

