/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013
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
#include <abc/testing/test_case.hxx>
#include <abc/trace.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::test_case


namespace abc {

namespace testing {

test_case::test_case() {
}


/*virtual*/ test_case::~test_case() {
}


void test_case::init(runner * prunner) {
   ABC_TRACE_FN((prunner));

   m_prunner = prunner;
}


void test_case::assert_impl(
   char const * pszFileName, unsigned iLine, bool bSuccess,
   istr const & sExpr, istr const & sOp, istr const & sExpected, istr const & sActual
) {
   ABC_TRACE_FN((this, pszFileName, iLine, bSuccess, sExpr, sOp, sExpected, sActual));

   if (bSuccess) {
      m_prunner->log_assertion_pass(pszFileName, iLine, sExpr, sOp, sExpected);
   } else {
      m_prunner->log_assertion_fail(pszFileName, iLine, sExpr, sOp, sExpected, sActual);
   }
}


void test_case::assert_false(
   char const * pszFileName, unsigned iLine, bool bActual, istr const & sExpr
) {
   ABC_TRACE_FN((this, pszFileName, iLine, bActual, sExpr));

   assert_impl(
      pszFileName, iLine, !bActual, sExpr, istr(), !bActual ? istr() : SL("false"), SL("true")
   );
}


void test_case::assert_true(
   char const * pszFileName, unsigned iLine, bool bActual, istr const & sExpr
) {
   ABC_TRACE_FN((this, pszFileName, iLine, bActual, sExpr));

   assert_impl(
      pszFileName, iLine, bActual, sExpr, istr(), bActual ? istr() : SL("true"), SL("false")
   );
}

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::test_case_factory_impl


namespace abc {

namespace testing {

/*static*/ test_case_factory_impl::list_item * test_case_factory_impl::sm_pliHead(nullptr);
// MSC16 BUG: for some reason, this will be parsed as a function declaration if written as a
// constructor call.
/*static*/ test_case_factory_impl::list_item ** test_case_factory_impl::sm_ppliTailNext = nullptr;


test_case_factory_impl::test_case_factory_impl(list_item * pli) {
   if (sm_pliHead) {
      // We have a head and therefore a tail as well: add *pli as the new tail.
      *sm_ppliTailNext = pli;
   } else {
      // We don’t have a head yet: set it up now.
      sm_pliHead = pli;
   }
   // Save the “next” pointer of *pli for the next call.
   sm_ppliTailNext = &pli->pliNext;
}

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

