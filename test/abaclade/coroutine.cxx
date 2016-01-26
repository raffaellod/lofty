/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2016 Raffaello D. Di Napoli

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
#include <abaclade/coroutine.hxx>
#include <abaclade/defer_to_scope_end.hxx>
#include <abaclade/io/text.hxx>
#include <abaclade/testing/test_case.hxx>
#include <abaclade/thread.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   coroutine_concurrency,
   "abc::coroutine – concurrent operation"
) {
   ABC_TRACE_FUNC(this);

   bool bCoro1Completed = false, bCoro2Completed = false;

   coroutine coro1([this, &bCoro1Completed] () {
      bCoro1Completed = true;
   });
   coroutine coro2([this, &bCoro2Completed] () {
      bCoro2Completed = true;
   });
   coroutine coro3;

   ABC_TESTING_ASSERT_NOT_EQUAL(coro1.id(), coroutine::id_type(0));
   ABC_TESTING_ASSERT_NOT_EQUAL(coro2.id(), coroutine::id_type(0));
   ABC_TESTING_ASSERT_EQUAL    (coro3.id(), coroutine::id_type(0));

   // Verify that the string representations are different.
   str sCoroutine1(to_str(coro1)), sCoroutine2(to_str(coro2)), sCoroutine3(to_str(coro3));
   ABC_TESTING_ASSERT_NOT_EQUAL(sCoroutine1, sCoroutine2);
   ABC_TESTING_ASSERT_NOT_EQUAL(sCoroutine1, sCoroutine3);
   ABC_TESTING_ASSERT_NOT_EQUAL(sCoroutine2, sCoroutine3);
   ABC_TESTING_ASSERT_EQUAL(sCoroutine3, ABC_SL("CRID:-"));

   this_thread::run_coroutines();

   ABC_TESTING_ASSERT_TRUE(bCoro1Completed);
   ABC_TESTING_ASSERT_TRUE(bCoro2Completed);

   // Avoid running other tests with a coroutine scheduler, as it might change their behavior.
   this_thread::detach_coroutine_scheduler();
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   coroutine_exception_containment,
   "abc::coroutine – exception containment"
) {
   ABC_TRACE_FUNC(this);

   coroutine coro1([this] () {
      ABC_TRACE_FUNC(this);

      // If exceptions are not properly contained by Abaclade, this will kill the entire process.
      ABC_THROW(generic_error, ());
   });

   /* Temporarily redirect stderr to a local string stream, so the exception trace from the
   coroutine won’t show in the test output. */
   auto psosErr(_std::make_shared<io::text::str_ostream>());
   {
      auto ptosOldStdErr(io::text::stderr);
      io::text::stderr = psosErr;
      ABC_DEFER_TO_SCOPE_END(io::text::stderr = _std::move(ptosOldStdErr));

      this_thread::run_coroutines();
   }

   // While we’re at it, verify that something was written to stderr while *ptswErr was stderr.
   ABC_TESTING_ASSERT_NOT_EQUAL(psosErr->get_str(), str::empty);

   // Avoid running other tests with a coroutine scheduler, as it might change their behavior.
   this_thread::detach_coroutine_scheduler();
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   coroutine_interruption,
   "abc::coroutine – interruption"
) {
   ABC_TRACE_FUNC(this);

   static std::size_t const sc_cWorkers = 5;
   bool abWorkersCompleted[sc_cWorkers], abWorkersInterrupted[sc_cWorkers];
   coroutine coroWorkers[sc_cWorkers];
   for (std::size_t i = 0; i < sc_cWorkers; ++i) {
      bool * pbWorkerCompleted = &abWorkersCompleted[i];
      bool * pbWorkerInterrupted = &abWorkersInterrupted[i];
      *pbWorkerCompleted = false;
      *pbWorkerInterrupted = false;
      coroWorkers[i] = coroutine([this, pbWorkerCompleted, pbWorkerInterrupted] () {
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
   coroutine coroController([this, &coroWorkers, &bControllerCompleted] () {
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

   // Avoid running other tests with a coroutine scheduler, as it might change their behavior.
   this_thread::detach_coroutine_scheduler();
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   coroutine_sleep,
   "abc::coroutine – sleep"
) {
   ABC_TRACE_FUNC(this);

   static std::size_t const sc_cWorkers = 5;
   coroutine coroWorkers[sc_cWorkers];
   unsigned sc_aiSleeps[sc_cWorkers] = { 20, 30, 10, 50, 40 };
   std::size_t aiWorkersAwoke[sc_cWorkers];
   memory::clear(aiWorkersAwoke);
   _std::atomic<std::size_t> aiNextAwakingWorkerSlot(0);
   for (std::size_t i = 0; i < sc_cWorkers; ++i) {
      coroWorkers[i] = coroutine([
         this, i, &sc_aiSleeps, &aiWorkersAwoke, &aiNextAwakingWorkerSlot
      ] () {
         ABC_TRACE_FUNC(this);

         this_coroutine::sleep_for_ms(sc_aiSleeps[i]);
         aiWorkersAwoke[aiNextAwakingWorkerSlot.fetch_add(1)] = i + 1;
      });
   }

   this_thread::run_coroutines();

   ABC_TESTING_ASSERT_EQUAL(aiWorkersAwoke[0], 3u);
   ABC_TESTING_ASSERT_EQUAL(aiWorkersAwoke[1], 1u);
   ABC_TESTING_ASSERT_EQUAL(aiWorkersAwoke[2], 2u);
   ABC_TESTING_ASSERT_EQUAL(aiWorkersAwoke[3], 5u);
   ABC_TESTING_ASSERT_EQUAL(aiWorkersAwoke[4], 4u);

   // Avoid running other tests with a coroutine scheduler, as it might change their behavior.
   this_thread::detach_coroutine_scheduler();
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   coroutine_on_secondary_thread,
   "abc::coroutine – on non-main thread"
) {
   ABC_TRACE_FUNC(this);

   thread thr1([this] () {
      bool bCoro1Completed = false;

      coroutine coro1([&bCoro1Completed] () {
         bCoro1Completed = true;
      });

      this_thread::run_coroutines();

      ABC_TESTING_ASSERT_TRUE(bCoro1Completed);
   });
   thr1.join();
}

}} //namespace abc::test
