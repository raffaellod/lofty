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
#include <lofty/defer_to_scope_end.hxx>
#include <lofty/io/text.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/thread.hxx>
#include <lofty/to_str.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   thread_concurrency,
   "lofty::thread – concurrent operation"
) {
   LOFTY_TRACE_FUNC(this);

   _std::atomic<bool> thread1_completed(false), thread2_completed(false);

   thread thread1([this, &thread1_completed] () {
      LOFTY_TRACE_FUNC(this);

      thread1_completed.store(true);
   });
   thread thread2([this, &thread2_completed] () {
      LOFTY_TRACE_FUNC(this);

      thread2_completed.store(true);
   });
   thread thread3;

   LOFTY_TESTING_ASSERT_TRUE(thread1.joinable());
   LOFTY_TESTING_ASSERT_TRUE(thread2.joinable());
   LOFTY_TESTING_ASSERT_FALSE(thread3.joinable());

   LOFTY_TESTING_ASSERT_NOT_EQUAL(thread1.id(), thread::id_type(0));
   LOFTY_TESTING_ASSERT_NOT_EQUAL(thread2.id(), thread::id_type(0));
   LOFTY_TESTING_ASSERT_EQUAL    (thread3.id(), thread::id_type(0));

   // Verify that the string representations are different.
   str thread1_str(to_str(thread1)), thread2_str(to_str(thread2)), thread3_str(to_str(thread3));
   LOFTY_TESTING_ASSERT_NOT_EQUAL(thread1_str, thread2_str);
   LOFTY_TESTING_ASSERT_NOT_EQUAL(thread1_str, thread3_str);
   LOFTY_TESTING_ASSERT_NOT_EQUAL(thread2_str, thread3_str);
   LOFTY_TESTING_ASSERT_EQUAL(thread3_str, LOFTY_SL("TID:-"));

   // Wait for thread1 and thread2 to complete.
   thread1.join();
   thread2.join();
   LOFTY_TESTING_ASSERT_FALSE(thread1.joinable());
   LOFTY_TESTING_ASSERT_FALSE(thread2.joinable());

   LOFTY_TESTING_ASSERT_TRUE(thread1_completed.load());
   LOFTY_TESTING_ASSERT_TRUE(thread2_completed.load());
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   thread_interruption,
   "lofty::thread – interruption"
) {
   LOFTY_TRACE_FUNC(this);

   static std::size_t const workers_size = 5;
   _std::atomic<bool> workers_completed[workers_size], workers_interrupted[workers_size];
   thread worker_threads[workers_size];
   for (std::size_t i = 0; i < workers_size; ++i) {
      auto worker_completed = &workers_completed[i];
      auto worker_interrupted = &workers_interrupted[i];
      worker_completed->store(false);
      worker_interrupted->store(false);
      worker_threads[i] = thread([this, worker_completed, worker_interrupted] () {
         LOFTY_TRACE_FUNC(this);

         try {
            /* Expect to be interrupted by the main thread. Make this sleep long enough so as not to cause
            sporadic test failures, but avoid slowing the test down by too much. */
            this_thread::sleep_for_ms(150);
            worker_completed->store(true);
         } catch (execution_interruption const &) {
            worker_interrupted->store(true);
         }
      });
   }

   // lofty::thread guarantees that the threads are are already running at this point.
   worker_threads[1].interrupt();
   worker_threads[2].interrupt();

   LOFTY_FOR_EACH(auto & thread, worker_threads) {
      thread.join();
   }

   LOFTY_TESTING_ASSERT_TRUE (workers_completed  [0].load());
   LOFTY_TESTING_ASSERT_FALSE(workers_interrupted[0].load());
   LOFTY_TESTING_ASSERT_FALSE(workers_completed  [1].load());
   LOFTY_TESTING_ASSERT_TRUE (workers_interrupted[1].load());
   LOFTY_TESTING_ASSERT_FALSE(workers_completed  [2].load());
   LOFTY_TESTING_ASSERT_TRUE (workers_interrupted[2].load());
   LOFTY_TESTING_ASSERT_TRUE (workers_completed  [3].load());
   LOFTY_TESTING_ASSERT_FALSE(workers_interrupted[3].load());
   LOFTY_TESTING_ASSERT_TRUE (workers_completed  [4].load());
   LOFTY_TESTING_ASSERT_FALSE(workers_interrupted[4].load());
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   thread_exception_propagation,
   "lofty::thread – exception propagation"
) {
   LOFTY_TRACE_FUNC(this);

   bool exception_caught = false;
   /* Temporarily redirect stderr to a local string stream, so the exception trace from the thread won’t show
   in the test output. */
   auto capturing_stderr(_std::make_shared<io::text::str_ostream>());
   {
      auto old_stderr(io::text::stderr);
      io::text::stderr = capturing_stderr;
      LOFTY_DEFER_TO_SCOPE_END(io::text::stderr = _std::move(old_stderr));

      /* Expect to be interrupted by an exception in thread1 any time from its creation to the sleep, which
      should be longer than the time it takes for thread1 to throw its exception. Can’t make any test
      assertions in this scope, since their output would end up in capturing_stderr instead of the real
      stderr. */
      try {
         thread thread1([this] () {
            LOFTY_TRACE_FUNC(this);

            LOFTY_THROW(execution_interruption, ());
         });
         /* Wait for the termination of thread1. Since thread1 will terminate with an exception, the current
         thread will be interrupted as well, right after thread1’s termination. */
         thread1.join();
      } catch (execution_interruption const &) {
         /* TODO: use a more specific exception subclass of execution_interruption, such as
         “other_thread_execution_interrupted”. */
         exception_caught = true;
      }
   }
   LOFTY_TESTING_ASSERT_TRUE(exception_caught);
   // While we’re at it, verify that something was written to stderr while *capturing_stderr was stderr.
   LOFTY_TESTING_ASSERT_NOT_EQUAL(capturing_stderr->get_str(), str::empty);
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   thread_interruption_exception_propagation,
   "lofty::thread – interruption exception propagation"
) {
   LOFTY_TRACE_FUNC(this);

   bool exception_caught = false;
   _std::atomic<bool> thread1_completed(false);
   thread thread1([this, &thread1_completed] () {
      LOFTY_TRACE_FUNC(this);

      /* Make the sleep long enough so as not to cause sporadic test failures, but avoid slowing the test down
      by too much. */
      this_thread::sleep_for_ms(150);
      thread1_completed.store(true);
   });

   /* Temporarily redirect stderr to a local string stream, so the exception trace from the thread won’t show
   in the test output. */
   auto capturing_stderr(_std::make_shared<io::text::str_ostream>());
   {
      auto old_stderr(io::text::stderr);
      io::text::stderr = capturing_stderr;
      LOFTY_DEFER_TO_SCOPE_END(io::text::stderr = _std::move(old_stderr));

      /* Expect to be interrupted by an exception in thread1 any time from its creation to the sleep, which
      should be longer than the time it takes for thread1 to throw its exception. Can’t make any test
      assertions in this scope, since their output would end up in capturing_stderr instead of the real
      stderr. */
      try {
         thread1.interrupt();
         /* Wait for the termination of thread1. Since we’re interrupting it, the current thread will be
         interrupted as well, right after thread1’s termination. */
         thread1.join();
      } catch (execution_interruption const &) {
         /* TODO: use a more specific exception subclass of execution_interruption, such as
         “other_thread_execution_interrupted”. */
         exception_caught = true;
      }
   }

   LOFTY_TESTING_ASSERT_TRUE(exception_caught);
   LOFTY_TESTING_ASSERT_FALSE(thread1_completed.load());
   // While we’re at it, verify that something was written to stderr while *capturing_stderr was stderr.
   LOFTY_TESTING_ASSERT_NOT_EQUAL(capturing_stderr->get_str(), str::empty);
}

}} //namespace lofty::test
