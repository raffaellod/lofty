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
// abc::test::coroutine_concurrent

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::coroutine – concurrent operation") {
   ABC_TRACE_FUNC(this);

   // TODO: use std::atomic for these variables.
   int volatile i1 = 1, i2 = 2;

   coroutine coro1([this, &i1] () -> void {
      i1 = 41;
   });
   coroutine coro2([this, &i2] () -> void {
      i2 = 42;
   });
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

   bool bCoro1Completed = false;
   coroutine coro1([this, &bCoro1Completed] () -> void {
      ABC_TRACE_FUNC(this);

      // If exceptions are not properly contained by Abaclade, this will kill the entire process.
      ABC_THROW(generic_error, ());
      bCoro1Completed = true;
   });

   /* Temporarily redirect stderr to a local string writer, so the exception trace from the
   coroutine won’t show in the test output. */
   auto ptswErr(std::make_shared<io::text::str_writer>());
   {
      auto ptwOldStdErr(io::text::stderr);
      io::text::stderr = ptswErr;
      this_thread::run_coroutines();
      io::text::stderr = std::move(ptwOldStdErr);
   }

   ABC_TESTING_ASSERT_FALSE(bCoro1Completed);
   // While we’re at it, verify that something was written to stderr while *ptswErr was stderr.
   ABC_TESTING_ASSERT_NOT_EQUAL(ptswErr->get_str(), istr::empty);
}

} //namespace test
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::coroutine_interruption

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::coroutine – interruption") {
   ABC_TRACE_FUNC(this);

   static std::size_t const sc_cWorkers = 5;
   bool abWorkersCompleted[sc_cWorkers], abWorkersInterrupted[sc_cWorkers];
   coroutine coroWorkers[sc_cWorkers];
   for (std::size_t i = 0; i < sc_cWorkers; ++i) {
      bool * pbWorkerCompleted = &abWorkersCompleted[i];
      bool * pbWorkerInterrupted = &abWorkersInterrupted[i];
      coroWorkers[i] = coroutine([this, pbWorkerCompleted, pbWorkerInterrupted] () -> void {
         ABC_TRACE_FUNC(this);

         try {
            /* Expect to be interrupted by coroController. Make this sleep long enough so as not to
            cause sporadic test failures, but avoid slowing the test down by too much. */
            this_coroutine::sleep_for_ms(150);
            *pbWorkerCompleted = true;
         } catch (execution_interruption const &) {
            *pbWorkerInterrupted = true;
         }
      });
   }

   bool bControllerCompleted = false;
   coroutine coroController([this, &coroWorkers, &bControllerCompleted] () -> void {
      ABC_TRACE_FUNC(this);

      /* Since coroutines on a single thread are started in FIFO order, the workers are already
      running at this point. */
      coroWorkers[1].interrupt();
      coroWorkers[2].interrupt();
      bControllerCompleted = true;
      // When this coroutine returns, the interruptions will take effect.
   });

   this_thread::run_coroutines();
   ABC_TESTING_ASSERT_TRUE(abWorkersCompleted[0]);
   ABC_TESTING_ASSERT_FALSE(abWorkersInterrupted[0]);
   ABC_TESTING_ASSERT_FALSE(abWorkersCompleted[1]);
   ABC_TESTING_ASSERT_TRUE(abWorkersInterrupted[1]);
   ABC_TESTING_ASSERT_FALSE(abWorkersCompleted[2]);
   ABC_TESTING_ASSERT_TRUE(abWorkersInterrupted[2]);
   ABC_TESTING_ASSERT_TRUE(abWorkersCompleted[3]);
   ABC_TESTING_ASSERT_FALSE(abWorkersInterrupted[3]);
   ABC_TESTING_ASSERT_TRUE(abWorkersCompleted[4]);
   ABC_TESTING_ASSERT_FALSE(abWorkersInterrupted[4]);
   ABC_TESTING_ASSERT_TRUE(bControllerCompleted);
}

} //namespace test
} //namespace abc
