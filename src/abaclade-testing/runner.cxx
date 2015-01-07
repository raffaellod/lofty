/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013, 2014, 2015
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
#include <abaclade/testing/runner.hxx>
#include <abaclade/testing/test_case.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::assertion_error

namespace abc {
namespace testing {

assertion_error::assertion_error() :
   exception() {
   m_pszWhat = "abc::assertion_error";
}

} //namespace testing
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::runner

namespace abc {
namespace testing {

runner::runner(std::shared_ptr<io::text::writer> ptwOut) :
   m_ptwOut(std::move(ptwOut)),
   m_cFailedAssertions(0) {
}

runner::~runner() {
}

void runner::load_registered_test_cases() {
   ABC_TRACE_FUNC(this);

   for (
      auto it(test_case_factory_list::begin()), itEnd(test_case_factory_list::end());
      it != itEnd;
      ++it
   ) {
      // Instantiate the test case.
      m_vptc.push_back(it->factory(this));
   }
}

void runner::log_assertion(
   source_location const & srcloc, bool bPass, istr const & sExpr, istr const & sOp,
   istr const & sExpected, istr const & sActual /*= istr::empty*/
) {
   ABC_TRACE_FUNC(this, srcloc, sExpr, sOp, sExpected, sActual);

   if (bPass) {
      m_ptwOut->print(
         ABC_SL("ABCMK-TEST-ASSERT-PASS {}: pass: {} {}{}\n"), srcloc, sExpr, sOp, sExpected
      );
   } else {
      ++m_cFailedAssertions;
      m_ptwOut->print(
         ABC_SL("ABCMK-TEST-ASSERT-FAIL {}: fail: {}\n")
            ABC_SL("  expected: {}{}\n")
            ABC_SL("  actual:   {}\n"),
         srcloc, sExpr, sOp, sExpected, sActual
      );
   }
}

bool runner::log_summary() {
   ABC_TRACE_FUNC(this);

   return m_cFailedAssertions == 0;
}

void runner::run() {
   ABC_TRACE_FUNC(this);

   for (auto it(m_vptc.begin()); it != m_vptc.end(); ++it) {
      run_test_case(**it);
   }
}

void runner::run_test_case(test_case & tc) {
   ABC_TRACE_FUNC(this/*, tc*/);

   m_ptwOut->print(ABC_SL("ABCMK-TEST-CASE-START {}\n"), tc.title());

   try {
      tc.run();
   } catch (assertion_error const &) {
      // This exception type is only used to interrupt abc::testing::test_case::run().
      m_ptwOut->write(ABC_SL("test case execution interrupted\n"));
   } catch (std::exception const & x) {
      exception::write_with_scope_trace(m_ptwOut.get(), &x);
      m_ptwOut->write(
         ABC_SL("ABCMK-TEST-ASSERT-FAIL unhandled exception, see stack trace above\n")
      );
   } catch (...) {
      exception::write_with_scope_trace(m_ptwOut.get());
      m_ptwOut->write(
         ABC_SL("ABCMK-TEST-ASSERT-FAIL unhandled exception, see stack trace above\n")
      );
   }

   m_ptwOut->write(ABC_SL("ABCMK-TEST-CASE-END\n"));
}

} //namespace testing
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
