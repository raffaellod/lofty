/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

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
#include <lofty/testing/test_case.hxx>
#include <lofty/text/char_ptr_to_str_adapter.hxx>
#include <lofty/to_str.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_char_ptr_to_str_adapter,
   "lofty::to_str – lofty::text::char_ptr_to_str_adapter"
) {
   LOFTY_TRACE_FUNC(this);

   LOFTY_TESTING_ASSERT_EQUAL(to_str(text::char_ptr_to_str_adapter(nullptr)), LOFTY_SL("<nullptr>"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(text::char_ptr_to_str_adapter("")), LOFTY_SL(""));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(text::char_ptr_to_str_adapter("a")), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(text::char_ptr_to_str_adapter("ab")), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(text::char_ptr_to_str_adapter("abc")), LOFTY_SL("abc"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(text::char_ptr_to_str_adapter("ab\0c")), LOFTY_SL("ab"));
}

}} //namespace lofty::test
