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
#include <abaclade/coroutine.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::coroutine_concurrent

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::coroutine – concurrent operation") {
   ABC_TRACE_FUNC(this);

   auto & pcorosched = this_thread::attach_coroutine_scheduler();

   // TODO: use std::atomic for these variables.
   int volatile i1 = 1, i2 = 2;

   coroutine coro1([this, &i1] () -> void {
      i1 = 41;
   });
   pcorosched->add(coro1);
   coroutine coro2([this, &i2] () -> void {
      i2 = 42;
   });
   pcorosched->add(coro2);
   coroutine coro3;

   ABC_TESTING_ASSERT_NOT_EQUAL(coro1.id(), coroutine::id_type(0));
   ABC_TESTING_ASSERT_NOT_EQUAL(coro2.id(), coroutine::id_type(0));
   ABC_TESTING_ASSERT_EQUAL    (coro3.id(), coroutine::id_type(0));

   // Verify that the string representations are different.
   dmstr sCoroutine1(to_str(coro1)), sCoroutine2(to_str(coro2)), sCoroutine3(to_str(coro3));
   ABC_TESTING_ASSERT_NOT_EQUAL(sCoroutine1, sCoroutine2);
   ABC_TESTING_ASSERT_NOT_EQUAL(sCoroutine1, sCoroutine3);
   ABC_TESTING_ASSERT_NOT_EQUAL(sCoroutine2, sCoroutine3);
   ABC_TESTING_ASSERT_EQUAL(sCoroutine3, ABC_SL("CRID:-"));

   this_thread::run_coroutines();

   ABC_TESTING_ASSERT_EQUAL(i1, 41);
   ABC_TESTING_ASSERT_EQUAL(i2, 42);
}

} //namespace test
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::coroutine_exceptions

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::coroutine – exception containment") {
   ABC_TRACE_FUNC(this);

   auto & pcorosched = this_thread::attach_coroutine_scheduler();

   coroutine coro1([] () -> void {
      // If exceptions are not properly contained by Abaclade, this will kill the entire process.
      //ABC_THROW(generic_error, ());
   });
   pcorosched->add(coro1);

   this_thread::run_coroutines();
}

} //namespace test
} //namespace abc
