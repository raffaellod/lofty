/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/testing/runner.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/text.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing {

assertion_error::assertion_error() {
}

}} //namespace lofty::testing

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing {

runner::runner(_std::shared_ptr<io::text::ostream> ostream_) :
   ostream(_std::move(ostream_)),
   failed_assertions(0) {
}

runner::~runner() {
}

void runner::load_registered_test_cases() {
   LOFTY_TRACE_FUNC(this);

   LOFTY_FOR_EACH(auto const & factory_list_elt, test_case_factory_list::instance()) {
      // Instantiate the test case.
      test_cases.push_back(factory_list_elt.factory(this));
   }
}

void runner::log_assertion(
   text::file_address const & file_addr, bool pass, str const & expr, str const & operand,
   str const & expected, str const & actual /*= str::empty*/
) {
   LOFTY_TRACE_FUNC(this, file_addr, expr, operand, expected, actual);

   if (pass) {
      ostream->print(
         LOFTY_SL("COMK-TEST-ASSERT-PASS {}: pass: {} {}{}\n"), file_addr, expr, operand, expected
      );
   } else {
      ++failed_assertions;
      ostream->print(
         LOFTY_SL("COMK-TEST-ASSERT-FAIL {}: fail: {}\n")
            LOFTY_SL("  expected: {}{}\n")
            LOFTY_SL("  actual:   {}\n"),
         file_addr, expr, operand, expected, actual
      );
   }
}

bool runner::log_summary() {
   LOFTY_TRACE_FUNC(this);

   return failed_assertions == 0;
}

void runner::run() {
   LOFTY_TRACE_FUNC(this);

   for (auto itr(test_cases.begin()); itr != test_cases.end(); ++itr) {
      run_test_case(**itr);
   }
}

void runner::run_test_case(class test_case & test_case) {
   LOFTY_TRACE_FUNC(this/*, test_case*/);

   ostream->print(LOFTY_SL("COMK-TEST-CASE-START {}\n"), test_case.title());

   try {
      test_case.run();
   } catch (assertion_error const &) {
      // This exception type is only used to interrupt lofty::testing::test_case::run().
      ostream->write(LOFTY_SL("test case execution interrupted\n"));
   } catch (_std::exception const & x) {
      exception::write_with_scope_trace(ostream.get(), &x);
      ostream->write(LOFTY_SL("COMK-TEST-ASSERT-FAIL unhandled exception, see stack trace above\n"));
   } catch (...) {
      exception::write_with_scope_trace(ostream.get());
      ostream->write(LOFTY_SL("COMK-TEST-ASSERT-FAIL unhandled exception, see stack trace above\n"));
   }

   ostream->write(LOFTY_SL("COMK-TEST-CASE-END\n"));
}

}} //namespace lofty::testing
