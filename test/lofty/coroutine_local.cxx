/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015, 2017 Raffaello D. Di Napoli

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
#include <lofty/coroutine.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/thread.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

/* A coroutine_local variable, being specific to a thread and a coroutine, by definition does not need to be
atomic; however this test case wants to find out if the variable is accidentally shared among multiple threads
or coroutines, and making the value not atomic could hide the problem. So atomic it is. */
static coroutine_local_value<_std::atomic<int>> coroutine_local_int /*= 0*/;

LOFTY_TESTING_TEST_CASE_FUNC(
   coroutine_local_basic,
   "lofty::coroutine_local_* – basic functionality"
) {
   LOFTY_TRACE_FUNC(this);

   coroutine_local_int.get().store(10);
   thread thread1([this] () {
      LOFTY_TRACE_FUNC(this);

      coroutine_local_int.get().store(11);
   });
   coroutine coro1([this] () {
      LOFTY_TRACE_FUNC(this);

      coroutine_local_int.get().store(21);
      // Yield to another coroutine.
      this_thread::sleep_for_ms(1);
      LOFTY_TESTING_ASSERT_EQUAL(coroutine_local_int.get().load(), 21);
   });
   coroutine coro2([this] () {
      LOFTY_TRACE_FUNC(this);

      coroutine_local_int.get().store(22);
      // Yield to another coroutine.
      this_thread::sleep_for_ms(1);
      LOFTY_TESTING_ASSERT_EQUAL(coroutine_local_int.get().load(), 22);
   });
   this_thread::run_coroutines();
   // Ensure the .store() in the other thread has taken place after this line.
   thread1.join();

   LOFTY_TESTING_ASSERT_EQUAL(coroutine_local_int.get().load(), 10);

   // Avoid running other tests with a coroutine scheduler, as it might change their behavior.
   this_thread::detach_coroutine_scheduler();
}

}} //namespace lofty::test
