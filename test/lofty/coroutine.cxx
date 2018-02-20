/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2018 Raffaello D. Di Napoli

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
#include <lofty/event.hxx>
#include <lofty/io/text.hxx>
#include <lofty/keyed_demux.hxx>
#include <lofty/logging.hxx>
#include <lofty/range.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/thread.hxx>
#include <lofty/to_str.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   coroutine_concurrency,
   "lofty::coroutine – concurrent operation"
) {
   LOFTY_TRACE_FUNC();

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
   LOFTY_TRACE_FUNC();

   coroutine coro1([this] () {
      LOFTY_TRACE_FUNC();

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
   LOFTY_TRACE_FUNC();

   static std::size_t const workers_size = 5;
   bool workers_completed[workers_size], workers_interrupted[workers_size];
   coroutine worker_coros[workers_size];
   for (std::size_t i = 0; i < workers_size; ++i) {
      auto worker_completed = &workers_completed[i];
      auto worker_interrupted = &workers_interrupted[i];
      *worker_completed = false;
      *worker_interrupted = false;
      worker_coros[i] = coroutine([this, worker_completed, worker_interrupted] () {
         LOFTY_TRACE_FUNC();

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
      LOFTY_TRACE_FUNC();

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
   LOFTY_TRACE_FUNC();

   static std::size_t const workers_size = 5;
   coroutine worker_coros[workers_size];
   unsigned sleeps[workers_size] = { 20, 30, 10, 50, 40 };
   std::size_t workers_awoke[workers_size];
   memory::clear(&workers_awoke);
   _std::atomic<std::size_t> next_awaking_worker_slot(0);
   for (std::size_t i = 0; i < workers_size; ++i) {
      worker_coros[i] = coroutine([this, i, &sleeps, &workers_awoke, &next_awaking_worker_slot] () {
         LOFTY_TRACE_FUNC();

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
   coroutine_join,
   "lofty::coroutine – joining"
) {
   LOFTY_TRACE_FUNC();

   unsigned coros_completed[] = { 0, 0, 0, 0 };
   _std::atomic<unsigned> next_completed_coro_slot(0);

   coroutine coro1, coro2, coro3, coro4;
   coro1 = coroutine([&coros_completed, &next_completed_coro_slot, &coro2] () {
      // Wait for a coroutine scheduled after this one.
      coro2.join();
      coros_completed[next_completed_coro_slot++] = 1;
   });
   coro2 = coroutine([&coros_completed, &next_completed_coro_slot] () {
      coros_completed[next_completed_coro_slot++] = 2;
   });
   coro3 = coroutine([&coros_completed, &next_completed_coro_slot] () {
      coros_completed[next_completed_coro_slot++] = 3;
   });
   coro4 = coroutine([&coros_completed, &next_completed_coro_slot, &coro3] () {
      /* Wait for a coroutine scheduled before this one. This will actually not wait because coro3 will have
      terminated by the time coro4 gets scheduled. */
      coro3.join();
      coros_completed[next_completed_coro_slot++] = 4;
   });

   this_thread::run_coroutines();

   // These assertions include assumptions about scheduling order. Relaxing them would be wise.
   LOFTY_TESTING_ASSERT_EQUAL(coros_completed[0], 2u);
   LOFTY_TESTING_ASSERT_EQUAL(coros_completed[1], 3u);
   LOFTY_TESTING_ASSERT_EQUAL(coros_completed[2], 4u);
   LOFTY_TESTING_ASSERT_EQUAL(coros_completed[3], 1u);

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
   LOFTY_TRACE_FUNC();

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   coroutine_event,
   "lofty::event (using coroutines)"
) {
   LOFTY_TRACE_FUNC();

   this_thread::attach_coroutine_scheduler();

   static unsigned const coros_size = 5;
   event events[coros_size];
   bool timedout[coros_size];
   unsigned resumed[coros_size];
   memory::clear(&resumed);
   _std::atomic<unsigned> next_resumed_index(0);
   for (unsigned i = 0; i < coros_size; ++i) {
      coroutine([i, &events, &timedout, &resumed, &next_resumed_index] () {
         LOFTY_TRACE_FUNC();

         try {
            // For i == 0 there will be no timeout.
            events[i].wait(i * 10);
            timedout[i] = false;
         } catch (io::timeout const &) {
            timedout[i] = true;
         }
         resumed[next_resumed_index.fetch_add(1)] = i + 1;
      });
   }

   coroutine([&events] () {
      LOFTY_TRACE_FUNC();

      events[2].trigger();
      events[4].trigger();
      // Process the first two events.
      this_coroutine::sleep_for_ms(1);
      events[0].trigger();
      events[1].trigger();
      // Avoid triggering events[3], which will timeout.
   });

   this_thread::run_coroutines();

   LOFTY_TESTING_ASSERT_EQUAL(resumed[0], 3u);
   LOFTY_TESTING_ASSERT_EQUAL(resumed[1], 5u);
   LOFTY_TESTING_ASSERT_EQUAL(resumed[2], 1u);
   LOFTY_TESTING_ASSERT_EQUAL(resumed[3], 2u);
   LOFTY_TESTING_ASSERT_EQUAL(resumed[4], 4u);
   LOFTY_TESTING_ASSERT_FALSE(timedout[0]);
   LOFTY_TESTING_ASSERT_FALSE(timedout[1]);
   LOFTY_TESTING_ASSERT_FALSE(timedout[2]);
   LOFTY_TESTING_ASSERT_TRUE(timedout[3]);
   LOFTY_TESTING_ASSERT_FALSE(timedout[4]);

   // Avoid running other tests with a coroutine scheduler, as it might change their behavior.
   this_thread::detach_coroutine_scheduler();
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   coroutine_event_trigger_before_wait,
   "lofty::event (using coroutines) – triggering before wait begins"
) {
   LOFTY_TRACE_FUNC();

   coroutine([this] () {
      LOFTY_TRACE_FUNC();

      bool timedout;
      event event1, event2;
      event1.trigger();
      event2.trigger();
      /* With a stateless representation of events, this will discard event2’s triggering because there’s
      nobody waiting for that, yet. With a stateful representation instead, event2 will remain in a triggered
      state until a wait() call on it. */
      try {
         event1.wait(5);
         timedout = false;
      } catch (io::timeout const &) {
         timedout = true;
      }
      LOFTY_TESTING_ASSERT_FALSE(timedout);

      // With stateless events, now event2.wait() would time out.
      try {
         event2.wait(5);
         timedout = false;
      } catch (io::timeout const &) {
         timedout = true;
      }
      LOFTY_TESTING_ASSERT_FALSE(timedout);

      // These, on the other hand, must time out.
      try {
         event1.wait(5);
         timedout = false;
      } catch (io::timeout const &) {
         timedout = true;
      }
      LOFTY_TESTING_ASSERT_TRUE(timedout);
      try {
         event2.wait(5);
         timedout = false;
      } catch (io::timeout const &) {
         timedout = true;
      }
      LOFTY_TESTING_ASSERT_TRUE(timedout);
   });

   this_thread::run_coroutines();

   // Avoid running other tests with a coroutine scheduler, as it might change their behavior.
   this_thread::detach_coroutine_scheduler();
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   coroutine_keyed_demux,
   "lofty::keyed_demux (using coroutines)"
) {
   LOFTY_TRACE_FUNC();

   this_thread::attach_coroutine_scheduler();
   {
      keyed_demux<short, long> number_demux;
      unsigned step = 0;
      number_demux.set_source([&step] (short * key) -> long {
         LOFTY_TRACE_FUNC();

         // In this test, the keys are the same as the values.

         this_coroutine::sleep_for_ms(1);
         switch (++step) {
            case 1:
               *key = 4;
               return 4;
            case 2:
               *key = 2;
               return 2;
            default:
               // Report EOF.
               return 0;
         }
      });

      static std::size_t const coros_size = 4;
      long get_returns[coros_size];
      LOFTY_FOR_EACH(short key, make_range<short>(1, static_cast<short>(coros_size + 1))) {
         coroutine([this, &number_demux, &get_returns, key] () {
            LOFTY_TRACE_FUNC();

            get_returns[key - 1] = number_demux.get(key, 10 * 1000);
         });
      }

      this_thread::run_coroutines();

      LOFTY_TESTING_ASSERT_EQUAL(step, 3u);
      LOFTY_TESTING_ASSERT_EQUAL(get_returns[0], 0);
      LOFTY_TESTING_ASSERT_EQUAL(get_returns[1], 2u);
      LOFTY_TESTING_ASSERT_EQUAL(get_returns[2], 0);
      LOFTY_TESTING_ASSERT_EQUAL(get_returns[3], 4u);
   }
   // Avoid running other tests with a coroutine scheduler, as it might change their behavior.
   this_thread::detach_coroutine_scheduler();
}

}} //namespace lofty::test
