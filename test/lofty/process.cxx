/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/logging.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/process.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   this_process_env,
   "lofty::this_process – retrieving environment variables"
) {
   LOFTY_TRACE_FUNC();

   ASSERT(_std::get<0>(this_process::env_var(LOFTY_SL("PATH"))) != str::empty);
   ASSERT(_std::get<1>(this_process::env_var(LOFTY_SL("PATH"))));
   ASSERT(_std::get<0>(this_process::env_var(LOFTY_SL("?UNSET?"))) == str::empty);
   ASSERT(!_std::get<1>(this_process::env_var(LOFTY_SL("?UNSET?"))));
}

}} //namespace lofty::test
