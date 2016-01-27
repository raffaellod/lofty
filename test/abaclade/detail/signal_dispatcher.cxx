/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2015 Raffaello D. Di Napoli

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
#include <abaclade/math.hxx>
#include <abaclade/testing/test_case.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   detail_signal_dispatcher_os_errors_to_cxx_exceptions,
   "abc::detail::signal_dispatcher – conversion of synchronous OS errors into C++ exceptions"
) {
   ABC_TRACE_FUNC(this);

   // Validate generation of invalid pointer dereference errors.
   {
      int * p = nullptr;
      ABC_TESTING_ASSERT_THROWS(memory::bad_pointer, *p = 1);
      // Check that the handler is still in place after its first activation above.
      ABC_TESTING_ASSERT_THROWS(memory::bad_pointer, *p = 2);

      ABC_TESTING_ASSERT_THROWS(memory::bad_pointer, *++p = 1);
   }

   // Validate generation of other pointer dereference errors.
   {
#if 0 // ABC_HOST_ARCH_???
      // Enable alignment checking if the architecture supports it.

      // Create an int (with another one following it) and a pointer to it.
      int i[2];
      void * p = &i[0];
      // Misalign the pointer, partly entering the second int.
      p = static_cast<std::int8_t *>(p) + 1;
      ABC_TESTING_ASSERT_THROWS(memory::bad_pointer_alignment, *static_cast<int *>(p) = 1);
#endif
   }

   // Validate generation of arithmetic errors.
   {
      // Non-obvious division by zero that can’t be detected at compile time.
      str sEmpty;
      int iZero = static_cast<int>(sEmpty.size_in_chars()), iOne = 1;
      ABC_TESTING_ASSERT_THROWS(math::division_by_zero, iOne /= iZero);
      // Use the quotient, so it won’t be optimized away.
      to_str(iOne);
   }
}

}} //namespace abc::test
