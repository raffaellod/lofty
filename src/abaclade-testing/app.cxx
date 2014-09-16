/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013, 2014
Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with Abaclade. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#include <abaclade/io/text/file.hxx>
#include <abaclade/testing/app.hxx>
#include <abaclade/testing/runner.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::app


namespace abc {
namespace testing {

/*virtual*/ int app::main(mvector<istr const> const & vsArgs) /*override*/ {
   ABC_TRACE_FUNC(this, vsArgs);

   ABC_UNUSED_ARG(vsArgs);

   runner r(io::text::stderr());
   r.load_registered_test_cases();
   r.run();
   bool bAllPassed(r.log_summary());

   return bAllPassed ? 0 : 1;
}

} //namespace testing
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

