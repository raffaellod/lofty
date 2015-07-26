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
#include <abaclade/defer_to_scope_end.hxx>
#include <abaclade/testing/test_case.hxx>
#include <abaclade/thread.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::thread – concurrent operation") {
   ABC_TRACE_FUNC(this);

   std::atomic<bool> bThr1Completed(false), bThr2Completed(false);

   thread thr1([this, &bThr1Completed] () {
      ABC_TRACE_FUNC(this);

      bThr1Completed.store(true);
   });
   thread thr2([this, &bThr2Completed] () {
      ABC_TRACE_FUNC(this);

      bThr2Completed.store(true);
   });
   thread thr3;

   ABC_TESTING_ASSERT_TRUE(thr1.joinable());
   ABC_TESTING_ASSERT_TRUE(thr2.joinable());
   ABC_TESTING_ASSERT_FALSE(thr3.joinable());

   ABC_TESTING_ASSERT_NOT_EQUAL(thr1.id(), thread::id_type(0));
   ABC_TESTING_ASSERT_NOT_EQUAL(thr2.id(), thread::id_type(0));
   ABC_TESTING_ASSERT_EQUAL    (thr3.id(), thread::id_type(0));

   // Verify that the string representations are different.
   dmstr sThread1(to_str(thr1)), sThread2(to_str(thr2)), sThread3(to_str(thr3));
   ABC_TESTING_ASSERT_NOT_EQUAL(sThread1, sThread2);
   ABC_TESTING_ASSERT_NOT_EQUAL(sThread1, sThread3);
   ABC_TESTING_ASSERT_NOT_EQUAL(sThread2, sThread3);
   ABC_TESTING_ASSERT_EQUAL(sThread3, ABC_SL("TID:-"));

   // Wait for thr1 and thr2 to complete.
   thr1.join();
   thr2.join();
   ABC_TESTING_ASSERT_FALSE(thr1.joinable());
   ABC_TESTING_ASSERT_FALSE(thr2.joinable());

   ABC_TESTING_ASSERT_TRUE(bThr1Completed.load());
   ABC_TESTING_ASSERT_TRUE(bThr2Completed.load());
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

#if !ABC_HOST_API_FREEBSD && !ABC_HOST_API_MACH
ABC_TESTING_TEST_CASE_FUNC("abc::thread – interruption") {
   ABC_TRACE_FUNC(this);

   static std::size_t const sc_cWorkers = 5;
   std::atomic<bool> abWorkersCompleted[sc_cWorkers], abWorkersInterrupted[sc_cWorkers];
   thread thrWorkers[sc_cWorkers];
   for (std::size_t i = 0; i < sc_cWorkers; ++i) {
      std::atomic<bool> * pbWorkerCompleted = &abWorkersCompleted[i];
      std::atomic<bool> * pbWorkerInterrupted = &abWorkersInterrupted[i];
      pbWorkerCompleted->store(false);
      pbWorkerInterrupted->store(false);
      thrWorkers[i] = thread([this, pbWorkerCompleted, pbWorkerInterrupted] () {
         ABC_TRACE_FUNC(this);

         try {
            /* Expect to be interrupted by the main thread. Make this sleep long enough so as not to
            cause sporadic test failures, but avoid slowing the test down by too much. */
            this_thread::sleep_for_ms(150);
            pbWorkerCompleted->store(true);
         } catch (execution_interruption const &) {
            pbWorkerInterrupted->store(true);
         }
      });
   }

   // abc::thread guarantees that the threads are are already running at this point.
   thrWorkers[1].interrupt();
   thrWorkers[2].interrupt();

   ABC_FOR_EACH(auto & thr, thrWorkers) {
      thr.join();
   }

   ABC_TESTING_ASSERT_TRUE (abWorkersCompleted  [0].load());
   ABC_TESTING_ASSERT_FALSE(abWorkersInterrupted[0].load());
   ABC_TESTING_ASSERT_FALSE(abWorkersCompleted  [1].load());
   ABC_TESTING_ASSERT_TRUE (abWorkersInterrupted[1].load());
   ABC_TESTING_ASSERT_FALSE(abWorkersCompleted  [2].load());
   ABC_TESTING_ASSERT_TRUE (abWorkersInterrupted[2].load());
   ABC_TESTING_ASSERT_TRUE (abWorkersCompleted  [3].load());
   ABC_TESTING_ASSERT_FALSE(abWorkersInterrupted[3].load());
   ABC_TESTING_ASSERT_TRUE (abWorkersCompleted  [4].load());
   ABC_TESTING_ASSERT_FALSE(abWorkersInterrupted[4].load());
}
#endif //if !ABC_HOST_API_FREEBSD && !ABC_HOST_API_MACH

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

#if !ABC_HOST_API_FREEBSD && !ABC_HOST_API_MACH
ABC_TESTING_TEST_CASE_FUNC("abc::thread – exception propagation") {
   ABC_TRACE_FUNC(this);

   bool bExceptionCaught = false;
   std::atomic<bool> bThr1Completed(false);
   /* Temporarily redirect stderr to a local string writer, so the exception trace from the thread
   won’t show in the test output. */
   auto ptswErr(std::make_shared<io::text::str_writer>());
   {
      auto ptwOldStdErr(io::text::stderr);
      io::text::stderr = ptswErr;
      auto deferred1(defer_to_scope_end([&ptwOldStdErr] () {
         io::text::stderr = std::move(ptwOldStdErr);
      }));

      /* Expect to be interrupted by an exception in thr1 any time from its creation to the sleep,
      which should be longer than the time it takes for thr1 to throw its exception. Can’t make any
      test assertions in this scope, since their output would end up in ptswErr instead of the real
      stderr. */
      try {
         thread thr1([this, &bThr1Completed] () {
            ABC_TRACE_FUNC(this);

            ABC_THROW(execution_interruption, ());
            bThr1Completed.store(true);
         });
         /* Wait for the termination of thr1. Since thr1 will terminate with an exception, the
         current thread will be interrupted as well, right after thr1’s termination. */
         thr1.join();
      } catch (execution_interruption const &) {
         /* TODO: use a more specific exception subclass of execution_interruption, such as
         “other_thread_execution_interrupted”. */
         bExceptionCaught = true;
      }

      // deferred1 will restore io::text::stderr.
   }
   ABC_TESTING_ASSERT_TRUE(bExceptionCaught);
   ABC_TESTING_ASSERT_FALSE(bThr1Completed.load());
   // While we’re at it, verify that something was written to stderr while *ptswErr was stderr.
   ABC_TESTING_ASSERT_NOT_EQUAL(ptswErr->get_str(), istr::empty);
}
#endif //if !ABC_HOST_API_FREEBSD && !ABC_HOST_API_MACH

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

#if !ABC_HOST_API_FREEBSD && !ABC_HOST_API_MACH
ABC_TESTING_TEST_CASE_FUNC("abc::thread – interruption exception propagation") {
   ABC_TRACE_FUNC(this);

   bool bExceptionCaught = false;
   std::atomic<bool> bThr1Completed(false);
   thread thr1([this, &bThr1Completed] () {
      ABC_TRACE_FUNC(this);

      /* Make the sleep long enough so as not to cause sporadic test failures, but avoid slowing the
      test down by too much. */
      this_thread::sleep_for_ms(150);
      bThr1Completed.store(true);
   });

   /* Temporarily redirect stderr to a local string writer, so the exception trace from the thread
   won’t show in the test output. */
   auto ptswErr(std::make_shared<io::text::str_writer>());
   {
      auto ptwOldStdErr(io::text::stderr);
      io::text::stderr = ptswErr;
      auto deferred1(defer_to_scope_end([&ptwOldStdErr] () {
         io::text::stderr = std::move(ptwOldStdErr);
      }));

      /* Expect to be interrupted by an exception in thr1 any time from its creation to the sleep,
      which should be longer than the time it takes for thr1 to throw its exception. Can’t make any
      test assertions in this scope, since their output would end up in ptswErr instead of the real
      stderr. */
      try {
         thr1.interrupt();
         /* Wait for the termination of thr1. Since we’re interrupting it, the current thread will
         be interrupted as well, right after thr1’s termination. */
         thr1.join();
      } catch (execution_interruption const &) {
         /* TODO: use a more specific exception subclass of execution_interruption, such as
         “other_thread_execution_interrupted”. */
         bExceptionCaught = true;
      }

      // deferred1 will restore io::text::stderr.
   }

   ABC_TESTING_ASSERT_TRUE(bExceptionCaught);
   ABC_TESTING_ASSERT_FALSE(bThr1Completed.load());
   // While we’re at it, verify that something was written to stderr while *ptswErr was stderr.
   ABC_TESTING_ASSERT_NOT_EQUAL(ptswErr->get_str(), istr::empty);
}
#endif //if !ABC_HOST_API_FREEBSD && !ABC_HOST_API_MACH

}} //namespace abc::test
