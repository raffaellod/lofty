/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/coroutine.hxx>
#include <lofty/defer_to_scope_end.hxx>
#include <lofty/io/text.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/thread.hxx>
#include <lofty/to_str.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   coroutine_concurrency,
   "lofty::coroutine – concurrent operation"
) {
   LOFTY_TRACE_FUNC(this);

   bool coro1_completed = false, coro2_completed = false;

   coroutine coro1([this, &coro1_completed] () {
      coro1_completed = true;
   });
   coroutine coro2([this, &coro2_completed] () {
      coro2_completed = true;
   });
   coroutine coro3;

   LOFTY_TESTING_ASSERT_NOT_EQUAL(coro1.id(), coroutine::id_type(0));
   LOFTY_TESTING_ASSERT_NOT_EQUAL(coro2.id(), coroutine::id_type(0));
   LOFTY_TESTING_ASSERT_EQUAL    (coro3.id(), coroutine::id_type(0));

   // Verify that the string representations are different.
   str coroutine1_str(to_str(coro1)), coroutine2_str(to_str(coro2)), coroutine3_str(to_str(coro3));
   LOFTY_TESTING_ASSERT_NOT_EQUAL(coroutine1_str, coroutine2_str);
   LOFTY_TESTING_ASSERT_NOT_EQUAL(coroutine1_str, coroutine3_str);
   LOFTY_TESTING_ASSERT_NOT_EQUAL(coroutine2_str, coroutine3_str);
   LOFTY_TESTING_ASSERT_EQUAL(coroutine3_str, LOFTY_SL("CRID:-"));

   this_thread::run_coroutines();

   LOFTY_TESTING_ASSERT_TRUE(coro1_completed);
   LOFTY_TESTING_ASSERT_TRUE(coro2_completed);

   // Avoid running other tests with a coroutine scheduler, as it might change their behavior.
   this_thread::detach_coroutine_scheduler();
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   coroutine_exception_containment,
   "lofty::coroutine – exception containment"
) {
   LOFTY_TRACE_FUNC(this);

   coroutine coro1([this] () {
      LOFTY_TRACE_FUNC(this);

      // If exceptions are not properly contained by Lofty, this will kill the entire process.
      LOFTY_THROW(generic_error, ());
   });

   /* Temporarily redirect stderr to a local string stream, so the exception trace from the coroutine won’t
   show in the test output. */
   auto capturing_stderr(_std::make_shared<io::text::str_ostream>());
   {
      auto old_stderr(io::text::stderr);
      io::text::stderr = capturing_stderr;
      LOFTY_DEFER_TO_SCOPE_END(io::text::stderr = _std::move(old_stderr));

      this_thread::run_coroutines();
   }

   // While we’re at it, verify that something was written to stderr while *capturing_stderr was stderr.
   LOFTY_TESTING_ASSERT_NOT_EQUAL(capturing_stderr->get_str(), str::empty);

   // Avoid running other tests with a coroutine scheduler, as it might change their behavior.
   this_thread::detach_coroutine_scheduler();
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   coroutine_interruption,
   "lofty::coroutine – interruption"
) {
   LOFTY_TRACE_FUNC(this);

   static std::size_t const workers_size = 5;
   bool workers_completed[workers_size], workers_interrupted[workers_size];
   coroutine worker_coros[workers_size];
   for (std::size_t i = 0; i < workers_size; ++i) {
      auto worker_completed = &workers_completed[i];
      auto worker_interrupted = &workers_interrupted[i];
      *worker_completed = false;
      *worker_interrupted = false;
      worker_coros[i] = coroutine([this, worker_completed, worker_interrupted] () {
         LOFTY_TRACE_FUNC(this);

         try {
            /* Expect to be interrupted by controller_coro. Make this sleep long enough so as not to cause
            sporadic test failures, but avoid slowing the test down by too much. */
            this_coroutine::sleep_for_ms(150);
            *worker_completed = true;
         } catch (execution_interruption const &) {
            *worker_interrupted = true;
         }
      });
   }

   bool controller_coro_completed = false;
   coroutine controller_coro([this, &worker_coros, &controller_coro_completed] () {
      LOFTY_TRACE_FUNC(this);

      /* Since coroutines on a single thread are started in FIFO order, the workers are already running at
      this point. */
      worker_coros[1].interrupt();
      worker_coros[2].interrupt();
      controller_coro_completed = true;
      // When this coroutine returns, the interruptions will take effect.
   });

   this_thread::run_coroutines();

   LOFTY_TESTING_ASSERT_TRUE(workers_completed[0]);
   LOFTY_TESTING_ASSERT_FALSE(workers_interrupted[0]);
   LOFTY_TESTING_ASSERT_FALSE(workers_completed[1]);
   LOFTY_TESTING_ASSERT_TRUE(workers_interrupted[1]);
   LOFTY_TESTING_ASSERT_FALSE(workers_completed[2]);
   LOFTY_TESTING_ASSERT_TRUE(workers_interrupted[2]);
   LOFTY_TESTING_ASSERT_TRUE(workers_completed[3]);
   LOFTY_TESTING_ASSERT_FALSE(workers_interrupted[3]);
   LOFTY_TESTING_ASSERT_TRUE(workers_completed[4]);
   LOFTY_TESTING_ASSERT_FALSE(workers_interrupted[4]);
   LOFTY_TESTING_ASSERT_TRUE(controller_coro_completed);

   // Avoid running other tests with a coroutine scheduler, as it might change their behavior.
   this_thread::detach_coroutine_scheduler();
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   coroutine_sleep,
   "lofty::coroutine – sleep"
) {
   LOFTY_TRACE_FUNC(this);

   static std::size_t const workers_size = 5;
   coroutine worker_coros[workers_size];
   unsigned sleeps[workers_size] = { 20, 30, 10, 50, 40 };
   std::size_t workers_awoke[workers_size];
   memory::clear(workers_awoke);
   _std::atomic<std::size_t> next_awaking_worker_slot(0);
   for (std::size_t i = 0; i < workers_size; ++i) {
      worker_coros[i] = coroutine([this, i, &sleeps, &workers_awoke, &next_awaking_worker_slot] () {
         LOFTY_TRACE_FUNC(this);

         this_coroutine::sleep_for_ms(sleeps[i]);
         workers_awoke[next_awaking_worker_slot.fetch_add(1)] = i + 1;
      });
   }

   this_thread::run_coroutines();

   LOFTY_TESTING_ASSERT_EQUAL(workers_awoke[0], 3u);
   LOFTY_TESTING_ASSERT_EQUAL(workers_awoke[1], 1u);
   LOFTY_TESTING_ASSERT_EQUAL(workers_awoke[2], 2u);
   LOFTY_TESTING_ASSERT_EQUAL(workers_awoke[3], 5u);
   LOFTY_TESTING_ASSERT_EQUAL(workers_awoke[4], 4u);

   // Avoid running other tests with a coroutine scheduler, as it might change their behavior.
   this_thread::detach_coroutine_scheduler();
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   coroutine_on_secondary_thread,
   "lofty::coroutine – on non-main thread"
) {
   LOFTY_TRACE_FUNC(this);

   thread thread1([this] () {
      bool coro1_completed = false;

      coroutine coro1([&coro1_completed] () {
         coro1_completed = true;
      });

      this_thread::run_coroutines();

      LOFTY_TESTING_ASSERT_TRUE(coro1_completed);
   });
   thread1.join();
}

}} //namespace lofty::test
