/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016-2017 Raffaello D. Di Napoli

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
#include <lofty/os.hxx>
#include <lofty/testing/test_case.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_API_WIN32

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   os_registry,
   "lofty::os – accessing Windows Registry"
) {
   LOFTY_TRACE_FUNC(this);

   sstr<8> s;

   LOFTY_TESTING_ASSERT_FALSE(os::get_registry_value(
      HKEY_LOCAL_MACHINE, LOFTY_SL("non-existent key"), str::empty, s.str_ptr()
   ));
   LOFTY_TESTING_ASSERT_EQUAL(s, str::empty);

   LOFTY_TESTING_ASSERT_FALSE(os::get_registry_value(
      HKEY_LOCAL_MACHINE, LOFTY_SL("Software\\Classes\\Interface"), str::empty, s.str_ptr()
   ));
   LOFTY_TESTING_ASSERT_EQUAL(s, str::empty);

   LOFTY_TESTING_ASSERT_FALSE(os::get_registry_value(
      HKEY_LOCAL_MACHINE, LOFTY_SL("Software"), LOFTY_SL("non-existent value"), s.str_ptr()
   ));
   LOFTY_TESTING_ASSERT_EQUAL(s, str::empty);

   LOFTY_TESTING_ASSERT_TRUE(os::get_registry_value(
      HKEY_LOCAL_MACHINE,
      LOFTY_SL("Software\\Classes\\Interface\\{00000000-0000-0000-c000-000000000046}"),
      str::empty, s.str_ptr()
   ));
   LOFTY_TESTING_ASSERT_EQUAL(s, LOFTY_SL("IUnknown"));
}

}} //namespace lofty::test

#endif //if LOFTY_HOST_API_WIN32
