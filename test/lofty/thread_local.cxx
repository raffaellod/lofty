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
#include <lofty/testing/test_case.hxx>
#include <lofty/thread.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

/* A thread_local variable, being specific to a thread, by definition does not need to be atomic; however this
test case wants to find out if the variable is accidentally shared among multiple threads, and making the
value not atomic could hide the problem. So atomic it is. */
static thread_local_value<_std::atomic<int>> thread_local_int /*= 0*/;

LOFTY_TESTING_TEST_CASE_FUNC(
   thread_local_basic,
   "lofty::thread_local_* – basic functionality"
) {
   LOFTY_TRACE_FUNC(this);

   thread_local_int.get().store(10);
   thread thread1([this] () {
      LOFTY_TRACE_FUNC(this);

      thread_local_int.get().store(11);
   });
   // Ensure the .store() in the other thread has taken place after this line.
   thread1.join();

   LOFTY_TESTING_ASSERT_EQUAL(thread_local_int.get().load(), 10);
}

}} //namespace lofty::test
