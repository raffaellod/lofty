﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/coroutine.hxx>
#include <lofty/event.hxx>
#include <lofty/exception.hxx>
#include <lofty/io.hxx>
#include <lofty/io/text.hxx>
#include <lofty/io/text/str.hxx>
#include <lofty/keyed_demux.hxx>
#include <lofty/logging.hxx>
#include <lofty/memory.hxx>
#include <lofty/mutex.hxx>
#include <lofty/range.hxx>
#include <lofty/_std/atomic.hxx>
#include <lofty/_std/memory.hxx>
#include <lofty/_std/mutex.hxx>
#include <lofty/_std/utility.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/text/str.hxx>
#include <lofty/thread.hxx>
#include <lofty/to_str.hxx>
#include <lofty/try_finally.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   thread_concurrency,
   "lofty::thread – concurrent operation"
) {
   LOFTY_TRACE_FUNC();

   _std::atomic<bool> thread1_completed(false), thread2_completed(false), thread3_completed(false);
   event thread3_terminated;

   thread thread1([this, &thread1_completed] () {
      LOFTY_TRACE_FUNC();

      thread1_completed.store(true);
   });
   thread thread2([this, &thread2_completed] () {
      LOFTY_TRACE_FUNC();

      thread2_completed.store(true);
   });
   thread thread3([this, &thread3_completed, &thread3_terminated] () {
      LOFTY_TRACE_FUNC();

      thread3_completed.store(true);
      thread3_terminated.trigger();
   });
   thread3.detach();
   thread thread4;

   ASSERT(thread1.joinable());
   ASSERT(thread2.joinable());
   ASSERT(!thread3.joinable());
   ASSERT(!thread4.joinable());

   ASSERT(thread1.id() != thread::id_type(0));
   ASSERT(thread2.id() != thread::id_type(0));
   ASSERT(thread3.id() == thread::id_type(0));
   ASSERT(thread4.id() == thread::id_type(0));

   /* Verify that the string representations are different for joinable thread, and identical for non-joinable
   ones. */
   text::str thread1_str(to_str(thread1)), thread2_str(to_str(thread2));
   text::str thread3_str(to_str(thread3)), thread4_str(to_str(thread4));
   ASSERT(thread1_str != thread2_str);
   ASSERT(thread1_str != thread3_str);
   ASSERT(thread2_str != thread3_str);
   ASSERT(thread3_str == thread4_str);
   ASSERT(thread4_str == LOFTY_SL("TID:-"));

   // Wait for thread1 and thread2 to complete.
   thread1.join();
   thread2.join();
   ASSERT(!thread1.joinable());
   ASSERT(!thread2.joinable());
   // Wait for thread3 to complete.
   thread3_terminated.wait();

   ASSERT(thread1_completed.load());
   ASSERT(thread2_completed.load());
   ASSERT(thread3_completed.load());
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   thread_interruption,
   "lofty::thread – interruption"
) {
   LOFTY_TRACE_FUNC();

   static std::size_t const workers_size = 5;
   _std::atomic<bool> workers_completed[workers_size], workers_interrupted[workers_size];
   thread worker_threads[workers_size];
   for (std::size_t i = 0; i < workers_size; ++i) {
      auto worker_completed = &workers_completed[i];
      auto worker_interrupted = &workers_interrupted[i];
      worker_completed->store(false);
      worker_interrupted->store(false);
      worker_threads[i] = thread([this, worker_completed, worker_interrupted] () {
         LOFTY_TRACE_FUNC();

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

   ASSERT( workers_completed  [0].load());
   ASSERT(!workers_interrupted[0].load());
   ASSERT(!workers_completed  [1].load());
   ASSERT( workers_interrupted[1].load());
   ASSERT(!workers_completed  [2].load());
   ASSERT( workers_interrupted[2].load());
   ASSERT( workers_completed  [3].load());
   ASSERT(!workers_interrupted[3].load());
   ASSERT( workers_completed  [4].load());
   ASSERT(!workers_interrupted[4].load());
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   thread_exception_propagation,
   "lofty::thread – exception propagation"
) {
   LOFTY_TRACE_FUNC();

   bool exception_caught = false;
   /* Temporarily redirect stderr to a local string stream, so the exception trace from the thread won’t show
   in the test output. */
   auto capturing_stderr(_std::make_shared<io::text::str_ostream>());
   auto old_stderr(io::text::stderr);
   io::text::stderr = capturing_stderr;
   LOFTY_TRY {
      /* Expect to be interrupted by an exception in thread1 any time from its creation to the sleep, which
      should be longer than the time it takes for thread1 to throw its exception. Can’t make any test
      assertions in this scope, since their output would end up in capturing_stderr instead of the real
      stderr. */
      try {
         thread thread1([this] () {
            LOFTY_TRACE_FUNC();

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
   } LOFTY_FINALLY {
      io::text::stderr = _std::move(old_stderr);
   };
   ASSERT(exception_caught);
   // While we’re at it, verify that something was written to stderr while *capturing_stderr was stderr.
   ASSERT(capturing_stderr->get_str() != text::str::empty);
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   thread_interruption_exception_propagation,
   "lofty::thread – interruption exception propagation"
) {
   LOFTY_TRACE_FUNC();

   bool exception_caught = false;
   _std::atomic<bool> thread1_completed(false);
   thread thread1([this, &thread1_completed] () {
      LOFTY_TRACE_FUNC();

      /* Make the sleep long enough so as not to cause sporadic test failures, but avoid slowing the test down
      by too much. */
      this_thread::sleep_for_ms(150);
      thread1_completed.store(true);
   });

   /* Temporarily redirect stderr to a local string stream, so the exception trace from the thread won’t show
   in the test output. */
   auto capturing_stderr(_std::make_shared<io::text::str_ostream>());
   auto old_stderr(io::text::stderr);
   io::text::stderr = capturing_stderr;
   LOFTY_TRY {
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
   } LOFTY_FINALLY {
      io::text::stderr = _std::move(old_stderr);
   };

   ASSERT(exception_caught);
   ASSERT(!thread1_completed.load());
   // While we’re at it, verify that something was written to stderr while *capturing_stderr was stderr.
   ASSERT(capturing_stderr->get_str() != text::str::empty);
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   thread_event,
   "lofty::event (using threads)"
) {
   LOFTY_TRACE_FUNC();

   static unsigned const threads_size = 4;
   thread threads[threads_size];
   event events[threads_size];
   unsigned resumed[threads_size], timedout[threads_size];
   memory::clear(&resumed);
   _std::atomic<unsigned> next_resumed_index(0);
   for (unsigned i = 0; i < threads_size; ++i) {
      threads[i] = thread([i, &events, &timedout, &resumed, &next_resumed_index] () {
         LOFTY_TRACE_FUNC();

         try {
            // For i == 0 there will be no timeout.
            events[i].wait(i * 50);
            timedout[i] = false;
         } catch (io::timeout const &) {
            timedout[i] = true;
         }
         resumed[next_resumed_index.fetch_add(1)] = i + 1;
      });
   }

   events[2].trigger();
   // Process the first event.
   this_thread::sleep_for_ms(1);
   events[0].trigger();
   // Avoid triggering events[1], which will timeout.

   for (unsigned i = 0; i < threads_size; ++i) {
      threads[i].join();
   }

   ASSERT(resumed[0] == 3u);
   ASSERT(resumed[1] == 1u);
   ASSERT(resumed[2] == 2u);
   ASSERT(!timedout[0]);
   ASSERT( timedout[1]);
   ASSERT(!timedout[2]);
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   thread_mutex,
   "lofty::mutex (using threads)"
) {
   LOFTY_TRACE_FUNC();

   _std::atomic<int> i1(1), i2(2), i3(3);
   mutex i_mutex;

   thread thread1([&i_mutex, &i1, &i2, &i3] () {
      _std::unique_lock<mutex> lock(i_mutex);
      ++i1; // 2
      // This will yield to the only other coroutine, which will change i2 to 3 if not blocked by the mutex.
      this_coroutine::sleep_for_ms(1);
      i3 += i1 * i2; // 7
   });

   thread thread2([&i_mutex, &i1, &i2, &i3] () {
      _std::unique_lock<mutex> lock(i_mutex);
      ++i2; // 3
      // This will yield to the only other coroutine, which will change i3 to 6 if not blocked by the mutex.
      this_coroutine::sleep_for_ms(1);
      i3 += i1 * i2; // 13
   });

   thread1.join();
   thread2.join();
   ASSERT(i_mutex.try_lock());
   i_mutex.unlock();

   ASSERT(i1.load() == 2);
   ASSERT(i2.load() == 3);
   ASSERT(i3.load() == 13);
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   thread_keyed_demux,
   "lofty::keyed_demux (using threads)"
) {
   LOFTY_TRACE_FUNC();

   keyed_demux<short, long> number_demux;
   unsigned step = 0;
   number_demux.set_source([&step] (short * key) -> long {
      LOFTY_TRACE_FUNC();

      // In this test, the keys are the same as the values.

      this_thread::sleep_for_ms(
#ifdef COMPLEMAKE_USING_VALGRIND
         100
#else
         3
#endif
      );
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

   static std::size_t const threads_size = 4;
   thread threads[threads_size];
   long get_returns[threads_size];
   LOFTY_FOR_EACH(short key, make_range<short>(1, static_cast<short>(threads_size + 1))) {
      threads[key - 1] = thread([this, &number_demux, &get_returns, key] () {
         LOFTY_TRACE_FUNC();

         get_returns[key - 1] = number_demux.get(key, 10 * 1000);
      });
   }

   for (unsigned i = 0; i < threads_size; ++i) {
      threads[i].join();
   }

   ASSERT(step == 3u);
   ASSERT(get_returns[0] == 0);
   ASSERT(get_returns[1] == 2);
   ASSERT(get_returns[2] == 0);
   ASSERT(get_returns[3] == 4);
}

}} //namespace lofty::test
