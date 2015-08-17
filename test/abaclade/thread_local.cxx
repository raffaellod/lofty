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
#include <abaclade/thread.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

/* A thread_local variable, being specific to a thread, by definition does not need to be atomic;
however this test case wants to find out if the variable is accidentally shared among multipliple
threads, and making the value not atomic could hide the problem. So atomic it is. */
static thread_local_value<_std::atomic<int>> g_iThreadLocal /*= 0*/;

ABC_TESTING_TEST_CASE_FUNC(
   thread_local_basic,
   "abc::thread_local_* – basic functionality"
) {
   ABC_TRACE_FUNC(this);

   g_iThreadLocal.get().store(10);
   thread thr1([this] () {
      ABC_TRACE_FUNC(this);

      g_iThreadLocal.get().store(11);
   });
   // Ensure the .store() in the other thread has taken place after this line.
   thr1.join();

   ABC_TESTING_ASSERT_EQUAL(g_iThreadLocal.get().load(), 10);
}

}} //namespace abc::test
