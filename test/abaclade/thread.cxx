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


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::thread_concurrent

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::thread – concurrent operation") {
   ABC_TRACE_FUNC(this);

   // TODO: use std::atomic for these variables.
   int volatile i1 = 1, i2 = 2;

   thread thr1([this, &i1] () -> void {
      i1 = 41;
   });
   thread thr2([this, &i2] () -> void {
      i2 = 42;
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

   ABC_TESTING_ASSERT_EQUAL(i1, 41);
   ABC_TESTING_ASSERT_EQUAL(i2, 42);
}

} //namespace test
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::thread_exceptions

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::thread – exception containment") {
   ABC_TRACE_FUNC(this);

   bool bThr1Completed = false;
   thread thr1([this, &bThr1Completed] () -> void {
      ABC_TRACE_FUNC(this);

      // If exceptions are not properly contained by Abaclade, this will kill the entire process.
      ABC_THROW(generic_error, ());
      bThr1Completed = true;
   });
   /* Temporarily redirect stderr to a local string writer, so the exception trace from the
   coroutine won’t show in the test output. */
   auto ptswErr(std::make_shared<io::text::str_writer>());
   {
      auto ptwOldStdErr(io::text::stderr);
      io::text::stderr = ptswErr;
      /* Wait for thr1 to complete. Can’t assert anything while thr1 is running, since any output
      would end up in ptswErr instead of the real stderr. */
      thr1.join();
      io::text::stderr = std::move(ptwOldStdErr);
   }

   ABC_TESTING_ASSERT_FALSE(thr1.joinable());
   ABC_TESTING_ASSERT_FALSE(bThr1Completed);
   // While we’re at it, verify that something was written to stderr while *ptswErr was stderr.
   ABC_TESTING_ASSERT_NOT_EQUAL(ptswErr->get_str(), istr::empty);
}

} //namespace test
} //namespace abc
