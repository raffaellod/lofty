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
#include <lofty/event.hxx>
#include <lofty/io/text.hxx>
#include <lofty/keyed_demux.hxx>
#include <lofty/logging.hxx>
#include <lofty/mutex.hxx>
#include <lofty/range.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/thread.hxx>
#include <lofty/to_str.hxx>
#include <lofty/try_finally.hxx>


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

   ASSERT(coro1.id() != coroutine::id_type(0));
   ASSERT(coro2.id() != coroutine::id_type(0));
   ASSERT(coro3.id() == coroutine::id_type(0));

   // Verify that the string representations are different.
   str coroutine1_str(to_str(coro1)), coroutine2_str(to_str(coro2)), coroutine3_str(to_str(coro3));
   ASSERT(coroutine1_str != coroutine2_str);
   ASSERT(coroutine1_str != coroutine3_str);
   ASSERT(coroutine2_str != coroutine3_str);
   ASSERT(coroutine3_str == LOFTY_SL("CRID:-"));

   this_thread::run_coroutines();

   ASSERT(coro1_completed);
   ASSERT(coro2_completed);

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
   auto old_stderr(io::text::stderr);
   io::text::stderr = capturing_stderr;
   LOFTY_TRY {
      this_thread::run_coroutines();
   } LOFTY_FINALLY {
      io::text::stderr = _std::move(old_stderr);
   };

   // While we’re at it, verify that something was written to stderr while *capturing_stderr was stderr.
   ASSERT(capturing_stderr->get_str() != str::empty);

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

   ASSERT( workers_completed[0]);
   ASSERT(!workers_interrupted[0]);
   ASSERT(!workers_completed[1]);
   ASSERT( workers_interrupted[1]);
   ASSERT(!workers_completed[2]);
   ASSERT( workers_interrupted[2]);
   ASSERT( workers_completed[3]);
   ASSERT(!workers_interrupted[3]);
   ASSERT( workers_completed[4]);
   ASSERT(!workers_interrupted[4]);
   ASSERT(controller_coro_completed);

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

   ASSERT(workers_awoke[0] == 3u);
   ASSERT(workers_awoke[1] == 1u);
   ASSERT(workers_awoke[2] == 2u);
   ASSERT(workers_awoke[3] == 5u);
   ASSERT(workers_awoke[4] == 4u);

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
   ASSERT(coros_completed[0] == 2u);
   ASSERT(coros_completed[1] == 3u);
   ASSERT(coros_completed[2] == 4u);
   ASSERT(coros_completed[3] == 1u);

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

      ASSERT(coro1_completed);
   });
   thread1.join();
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

#if 0
LOFTY_TESTING_TEST_CASE_FUNC(
   coroutine_on_multithreaded_scheduler,
   "lofty::coroutine – multiple threads sharing one scheduler"
) {
   LOFTY_TRACE_FUNC();

   auto coro_sched(this_thread::attach_coroutine_scheduler());

   static unsigned const threads_size = 16;
   static unsigned const coros_size = 64;
   thread threads[threads_size];
   collections::vector<bool> coros_completed;
   coros_completed.set_size(coros_size);

   // Schedule all the coroutines.
   for (unsigned i = 0; i < coros_size; ++i) {
      coroutine([&coros_completed, i] () {
         if (i > 10) {
            this_coroutine::sleep_for_ms(i);
         }
         coros_completed[i] = true;
      });
   }

   /* Detach the thread scheduler, so that thread startup synchronization will not try to act as if we’re in a
   coroutine. */
   // TODO: FIXME: this is weak.
   this_thread::detach_coroutine_scheduler();

   // Now bring up the extra threads, which will automatically begin executing coroutines.
   LOFTY_FOR_EACH(auto & thread, threads) {
      thread = lofty::thread([&coro_sched] () {
         this_thread::attach_coroutine_scheduler(coro_sched);
         this_thread::run_coroutines();
      });
   }
   // Go with the other threads.
   this_thread::attach_coroutine_scheduler(coro_sched);
   this_thread::run_coroutines();

   // At this point, all threads should be finished; join them all.
   LOFTY_FOR_EACH(auto & thread, threads) {
      thread.join();
   }

   // Test that no false values can be found in coros_completed.
   auto noncompleted_coro_itr(coros_completed.find(false));
   int noncompleted_coro_index = noncompleted_coro_itr != coros_completed.cend()
      ? static_cast<int>(noncompleted_coro_itr - coros_completed.cbegin())
      : -1;
   ASSERT(noncompleted_coro_index == -1);

   // Avoid running other tests with a coroutine scheduler, as it might change their behavior.
   this_thread::detach_coroutine_scheduler();
}
#endif

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

   ASSERT(resumed[0] == 3u);
   ASSERT(resumed[1] == 5u);
   ASSERT(resumed[2] == 1u);
   ASSERT(resumed[3] == 2u);
   ASSERT(resumed[4] == 4u);
   ASSERT(!timedout[0]);
   ASSERT(!timedout[1]);
   ASSERT(!timedout[2]);
   ASSERT( timedout[3]);
   ASSERT(!timedout[4]);

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
      ASSERT(!timedout);

      // With stateless events, now event2.wait() would time out.
      try {
         event2.wait(5);
         timedout = false;
      } catch (io::timeout const &) {
         timedout = true;
      }
      ASSERT(!timedout);

      // These, on the other hand, must time out.
      try {
         event1.wait(5);
         timedout = false;
      } catch (io::timeout const &) {
         timedout = true;
      }
      ASSERT(timedout);
      try {
         event2.wait(5);
         timedout = false;
      } catch (io::timeout const &) {
         timedout = true;
      }
      ASSERT(timedout);
   });

   this_thread::run_coroutines();

   // Avoid running other tests with a coroutine scheduler, as it might change their behavior.
   this_thread::detach_coroutine_scheduler();
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   coroutine_mutex,
   "lofty::mutex (using coroutines)"
) {
   LOFTY_TRACE_FUNC();

   this_thread::attach_coroutine_scheduler();

   // These are atomic to allow changes in one coroutine to show in the other coroutine.
   _std::atomic<int> i1(1), i2(2), i3(3);
   mutex i_mutex;

   coroutine coro1([&i_mutex, &i1, &i2, &i3] () {
      _std::unique_lock<mutex> lock(i_mutex);
      ++i1; // 2
      // This will yield to coro2, which will change i2 to 3 if not blocked by the mutex.
      this_coroutine::sleep_for_ms(1);
      i3 += i1 * i2; // 7
   });

   coroutine coro2([&i_mutex, &i1, &i2, &i3] () {
      _std::unique_lock<mutex> lock(i_mutex);
      ++i2; // 3
      // This will yield to coro1, which will change i3 to 6 if not blocked by the mutex.
      this_coroutine::sleep_for_ms(1);
      i3 += i1 * i2; // 13
   });

   coroutine coro3([this, &i_mutex, &coro1, &coro2] () {
      coro1.join();
      coro2.join();
      ASSERT(i_mutex.try_lock());
      i_mutex.unlock();
   });

   this_thread::run_coroutines();

   ASSERT(i1.load() == 2);
   ASSERT(i2.load() == 3);
   ASSERT(i3.load() == 13);

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

      ASSERT(step == 3u);
      ASSERT(get_returns[0] == 0);
      ASSERT(get_returns[1] == 2);
      ASSERT(get_returns[2] == 0);
      ASSERT(get_returns[3] == 4);
   }
   // Avoid running other tests with a coroutine scheduler, as it might change their behavior.
   this_thread::detach_coroutine_scheduler();
}

}} //namespace lofty::test
