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
#include <abaclade/os.hxx>
#include <abaclade/testing/test_case.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

#if ABC_HOST_API_WIN32

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   os_registry,
   "abc::os – accessing Windows Registry"
) {
   ABC_TRACE_FUNC(this);

   sstr<8> s;

   ABC_TESTING_ASSERT_FALSE(os::get_registry_value(
      HKEY_LOCAL_MACHINE, ABC_SL("non-existent key"), str::empty, s.str_ptr()
   ));
   ABC_TESTING_ASSERT_EQUAL(s, str::empty);

   ABC_TESTING_ASSERT_FALSE(os::get_registry_value(
      HKEY_LOCAL_MACHINE, ABC_SL("Software\\Classes\\Interface"), str::empty, s.str_ptr()
   ));
   ABC_TESTING_ASSERT_EQUAL(s, str::empty);

   ABC_TESTING_ASSERT_FALSE(os::get_registry_value(
      HKEY_LOCAL_MACHINE, ABC_SL("Software"), ABC_SL("non-existent value"), s.str_ptr()
   ));
   ABC_TESTING_ASSERT_EQUAL(s, str::empty);

   ABC_TESTING_ASSERT_TRUE(os::get_registry_value(
      HKEY_LOCAL_MACHINE,
      ABC_SL("Software\\Classes\\Interface\\{00000000-0000-0000-c000-000000000046}"),
      str::empty, s.str_ptr()
   ));
   ABC_TESTING_ASSERT_EQUAL(s, ABC_SL("IUnknown"));
}

}} //namespace abc::test

#endif //if ABC_HOST_API_WIN32
