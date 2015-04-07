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

class thread_concurrent : public testing::test_case {
public:
   //! Constructor.
   thread_concurrent() :
      m_i1(1),
      m_i2(2) {
   }

   //! See testing::test_case::title().
   virtual istr title() override {
      return istr(ABC_SL("abc::thread – concurrent operation"));
   }

   //! See testing::test_case::run().
   virtual void run() override {
      ABC_TRACE_FUNC(this);

      thread thr1([this] () -> void {
         m_i1 = 41;
      });
      thread thr2([this] () -> void {
         m_i2 = 42;
      });

      // TODO: use a text::str_writer to check that these come out non-empty and different.
      //io::text::stderr()->print(ABC_SL("thr1={} thr2={}\n"), thr1, thr2);

      // Wait for both threads to complete.
      thr1.join();
      thr2.join();

      ABC_TESTING_ASSERT_EQUAL(m_i1, 41);
      ABC_TESTING_ASSERT_EQUAL(m_i2, 42);
   }

private:
   //! Holds a value set by thread #1.
   // TODO: use std::atomic.
   int volatile m_i1;
   //! Holds a value set by thread #2.
   // TODO: use std::atomic.
   int volatile m_i2;
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::thread_concurrent)
