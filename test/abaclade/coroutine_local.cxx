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
#include <abaclade/coroutine.hxx>
#include <abaclade/testing/test_case.hxx>
#include <abaclade/thread.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

/* A coroutine_local variable, being specific to a thread and a coroutine, by definition does not
need to be atomic; however this test case wants to find out if the variable is accidentally shared
among multipliple threads or coroutines, and making the value not atomic could hide the problem. So
atomic it is. */
static coroutine_local_value<std::atomic<int>> g_iCoroutineLocal /*= 0*/;

ABC_TESTING_TEST_CASE_FUNC("abc::coroutine_local_* – basic functionality") {
   ABC_TRACE_FUNC(this);

   g_iCoroutineLocal.get().store(10);
   thread thr1([this] () {
      ABC_TRACE_FUNC(this);

      g_iCoroutineLocal.get().store(11);
   });
   coroutine coro1([this] () {
      ABC_TRACE_FUNC(this);

      g_iCoroutineLocal.get().store(21);
      // Yield to another coroutine.
      this_thread::sleep_for_ms(1);
      ABC_TESTING_ASSERT_EQUAL(g_iCoroutineLocal.get().load(), 21);
   });
   coroutine coro2([this] () {
      ABC_TRACE_FUNC(this);

      g_iCoroutineLocal.get().store(22);
      // Yield to another coroutine.
      this_thread::sleep_for_ms(1);
      ABC_TESTING_ASSERT_EQUAL(g_iCoroutineLocal.get().load(), 22);
   });
   this_thread::run_coroutines();
   // Ensure the .store() in the other thread has taken place after this line.
   thr1.join();

   ABC_TESTING_ASSERT_EQUAL(g_iCoroutineLocal.get().load(), 10);

   // Avoid running other tests with a coroutine scheduler, as it might change their behavior.
   this_thread::detach_coroutine_scheduler();
}

}} //namespace abc::test
