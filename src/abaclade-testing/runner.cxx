/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013-2016 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#include <abaclade/testing/runner.hxx>
#include <abaclade/testing/test_case.hxx>
#include <abaclade/text.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace testing {

assertion_error::assertion_error() {
}

}} //namespace abc::testing

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace testing {

runner::runner(_std::shared_ptr<io::text::ostream> ptos) :
   m_ptos(_std::move(ptos)),
   m_cFailedAssertions(0) {
}

runner::~runner() {
}

void runner::load_registered_test_cases() {
   ABC_TRACE_FUNC(this);

   ABC_FOR_EACH(auto const & tcf, test_case_factory_list::instance()) {
      // Instantiate the test case.
      m_vptc.push_back(tcf.factory(this));
   }
}

void runner::log_assertion(
   text::file_address const & tfa, bool bPass, str const & sExpr, str const & sOp,
   str const & sExpected, str const & sActual /*= str::empty*/
) {
   ABC_TRACE_FUNC(this, tfa, sExpr, sOp, sExpected, sActual);

   if (bPass) {
      m_ptos->print(
         ABC_SL("ABCMK-TEST-ASSERT-PASS {}: pass: {} {}{}\n"), tfa, sExpr, sOp, sExpected
      );
   } else {
      ++m_cFailedAssertions;
      m_ptos->print(
         ABC_SL("ABCMK-TEST-ASSERT-FAIL {}: fail: {}\n")
            ABC_SL("  expected: {}{}\n")
            ABC_SL("  actual:   {}\n"),
         tfa, sExpr, sOp, sExpected, sActual
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

   m_ptos->print(ABC_SL("ABCMK-TEST-CASE-START {}\n"), tc.title());

   try {
      tc.run();
   } catch (assertion_error const &) {
      // This exception type is only used to interrupt abc::testing::test_case::run().
      m_ptos->write(ABC_SL("test case execution interrupted\n"));
   } catch (_std::exception const & x) {
      exception::write_with_scope_trace(m_ptos.get(), &x);
      m_ptos->write(ABC_SL("ABCMK-TEST-ASSERT-FAIL unhandled exception, see stack trace above\n"));
   } catch (...) {
      exception::write_with_scope_trace(m_ptos.get());
      m_ptos->write(ABC_SL("ABCMK-TEST-ASSERT-FAIL unhandled exception, see stack trace above\n"));
   }

   m_ptos->write(ABC_SL("ABCMK-TEST-CASE-END\n"));
}

}} //namespace abc::testing
