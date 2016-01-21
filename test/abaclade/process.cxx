/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#include <abaclade/testing/test_case.hxx>
#include <abaclade/process.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   this_process_env,
   "abc::this_process – retrieving environment variables"
) {
   ABC_TRACE_FUNC(this);

   ABC_TESTING_ASSERT_NOT_EQUAL(_std::get<0>(this_process::env_var(ABC_SL("PATH"))), str::empty);
   ABC_TESTING_ASSERT_TRUE(_std::get<1>(this_process::env_var(ABC_SL("PATH"))));
   ABC_TESTING_ASSERT_EQUAL(_std::get<0>(this_process::env_var(ABC_SL("?UNSET?"))), str::empty);
   ABC_TESTING_ASSERT_FALSE(_std::get<1>(this_process::env_var(ABC_SL("?UNSET?"))));
}

}} //namespace abc::test
