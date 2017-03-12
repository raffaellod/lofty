/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013-2015, 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/io/text.hxx>
#include <lofty/testing/app.hxx>
#include <lofty/testing/runner.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing {

/*virtual*/ int app::main(collections::vector<str> & args) /*override*/ {
   LOFTY_TRACE_FUNC(this, args);

   LOFTY_UNUSED_ARG(args);

   runner r(io::text::stderr);
   r.load_registered_test_cases();
   r.run();
   bool all_passed = r.log_summary();

   return all_passed ? 0 : 1;
}

}} //namespace lofty::testing
