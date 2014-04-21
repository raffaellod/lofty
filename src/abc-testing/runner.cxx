/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013, 2014
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abc/testing/core.hxx>
#include <abc/testing/runner.hxx>
#include <abc/testing/test_case.hxx>
#include <abc/trace.hxx>



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

runner::runner(std::shared_ptr<ostream> posOut) :
   m_pos(std::move(posOut)),
   m_cFailedAssertions(0) {
}


runner::~runner() {
   // TODO: currently abc::*vector containers don’t support move-only types; remove this manual
   // cleanup code when std::unique_ptr becomes supported.
   for (auto it(m_vptc.begin()); it != m_vptc.end(); ++it) {
      delete *it;
   }
}


void runner::load_registered_test_cases() {
   ABC_TRACE_FN((this));

   for (
      test_case_factory_impl::list_item * pli(test_case_factory_impl::get_factory_list_head());
      pli;
      pli = pli->pliNext
   ) {
      // Instantiate the test case.
      auto ptc(pli->pfnFactory(this));
      // TODO: currently abc::*vector containers don’t support move-only types; change to use
      // std::unique_ptr when that becomes supported.
      m_vptc.append(ptc.release());
//    m_vptc.append(std::move(ptc));
   }
}


void runner::log_assertion(
   source_location const & srcloc, bool bPass,
   istr const & sExpr, istr const & sOp, istr const & sExpected, istr const & sActual /*= istr()*/
) {
   ABC_TRACE_FN((this, srcloc, sExpr, sOp, sExpected, sActual));

   if (bPass) {
      m_pos->print(SL("ABCMK-TEST-ASSERT-PASS {}: pass: {} {}{}\n"), srcloc, sExpr, sOp, sExpected);
   } else {
      ++m_cFailedAssertions;
      m_pos->print(
         SL("ABCMK-TEST-ASSERT-FAIL {}: fail: {}\n")
            SL("  expected: {}{}\n")
            SL("  actual:   {}\n"),
         srcloc, sExpr, sOp, sExpected, sActual
      );
   }
}


bool runner::log_summary() {
   ABC_TRACE_FN((this));

   return m_cFailedAssertions == 0;
}


void runner::run() {
   ABC_TRACE_FN((this));

   for (auto it(m_vptc.begin()); it != m_vptc.end(); ++it) {
      run_test_case(**it);
   }
}


void runner::run_test_case(test_case & tc) {
   ABC_TRACE_FN((this/*, tc*/));

   m_pos->print(SL("ABCMK-TEST-CASE-START {}\n"), tc.title());

   try {
      tc.run();
   } catch (assertion_error const &) {
      // This exception type is only used to interrupt abc::testing::test_case::run().
      m_pos->write(SL("test case execution interrupted\n"));
   } catch (std::exception const & x) {
      exception::write_with_scope_trace(m_pos.get(), &x);
   } catch (...) {
      exception::write_with_scope_trace(m_pos.get());
   }

   m_pos->print(SL("ABCMK-TEST-CASE-END\n"));
}

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

