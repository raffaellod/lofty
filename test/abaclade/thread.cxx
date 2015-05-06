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

#include <atomic>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::thread_concurrent

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::thread – concurrent operation") {
   ABC_TRACE_FUNC(this);

   std::atomic<bool> bThr1Completed(false), bThr2Completed(false);

   thread thr1([this, &bThr1Completed] () -> void {
      ABC_TRACE_FUNC(this);

      bThr1Completed = true;
   });
   thread thr2([this, &bThr2Completed] () -> void {
      ABC_TRACE_FUNC(this);

      bThr2Completed = true;
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

   ABC_TESTING_ASSERT_TRUE(bThr1Completed);
   ABC_TESTING_ASSERT_TRUE(bThr2Completed);
}

} //namespace test
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::thread_interruption

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::thread – interruption") {
   ABC_TRACE_FUNC(this);

   static std::size_t const sc_cWorkers = 5;
   bool abWorkersCompleted[sc_cWorkers], abWorkersInterrupted[sc_cWorkers];
   thread thrWorkers[sc_cWorkers];
   for (std::size_t i = 0; i < sc_cWorkers; ++i) {
      bool * pbWorkerCompleted = &abWorkersCompleted[i];
      bool * pbWorkerInterrupted = &abWorkersInterrupted[i];
      thrWorkers[i] = thread([this, pbWorkerCompleted, pbWorkerInterrupted] () -> void {
         ABC_TRACE_FUNC(this);

         try {
            /* Expect to be interrupted by coroController. Make this sleep long enough so as not to
            cause sporadic test failures, but avoid slowing the test down by too much. */
            this_thread::sleep_for_ms(150);
            *pbWorkerCompleted = true;
         } catch (execution_interruption const &) {
            *pbWorkerInterrupted = true;
         }
      });
   }

   // abc::thread guarantees that the threads are are already running at this point.
   thrWorkers[1].interrupt();
   thrWorkers[2].interrupt();

   ABC_FOR_EACH(auto & thr, thrWorkers) {
      thr.join();
   }

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
}

} //namespace test
} //namespace abc
