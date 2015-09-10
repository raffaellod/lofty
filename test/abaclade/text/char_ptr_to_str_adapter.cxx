/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015
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
#include <abaclade/testing/test_case.hxx>
#include <abaclade/text/char_ptr_to_str_adapter.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   text_char_ptr_to_str_adapter,
   "abc::to_str – abc::text::char_ptr_to_str_adapter"
) {
   ABC_TRACE_FUNC(this);

   ABC_TESTING_ASSERT_EQUAL(to_str(text::char_ptr_to_str_adapter(nullptr)), ABC_SL("<nullptr>"));
   ABC_TESTING_ASSERT_EQUAL(to_str(text::char_ptr_to_str_adapter("")), ABC_SL(""));
   ABC_TESTING_ASSERT_EQUAL(to_str(text::char_ptr_to_str_adapter("a")), ABC_SL("a"));
   ABC_TESTING_ASSERT_EQUAL(to_str(text::char_ptr_to_str_adapter("ab")), ABC_SL("ab"));
   ABC_TESTING_ASSERT_EQUAL(to_str(text::char_ptr_to_str_adapter("abc")), ABC_SL("abc"));
   ABC_TESTING_ASSERT_EQUAL(to_str(text::char_ptr_to_str_adapter("ab\0c")), ABC_SL("ab"));
}

}} //namespace abc::test
