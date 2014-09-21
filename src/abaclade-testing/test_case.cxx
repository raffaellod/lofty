/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013, 2014
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


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::test_case

namespace abc {
namespace testing {

test_case::test_case() {
}

/*virtual*/ test_case::~test_case() {
}

void test_case::init(runner * prunner) {
   ABC_TRACE_FUNC(this, prunner);

   m_prunner = prunner;
}

void test_case::assert_does_not_throw(
   source_location const & srcloc, std::function<void ()> const & fnExpr, istr const & sExpr
) {
   ABC_TRACE_FUNC(this, srcloc, /*fnExpr, */sExpr);

   istr sCaughtWhat;
   try {
      fnExpr();
   } catch (::std::exception const & x) {
      sCaughtWhat = istr(ABC_SL("throws {}")).format(x.what());
   } catch (...) {
      sCaughtWhat = ABC_SL("unknown type");
   }
   m_prunner->log_assertion(
      srcloc, !sCaughtWhat, sExpr, istr::empty, ABC_SL("does not throw"), sCaughtWhat
   );
}

void test_case::assert_false(source_location const & srcloc, bool bActual, istr const & sExpr) {
   ABC_TRACE_FUNC(this, srcloc, bActual, sExpr);

   m_prunner->log_assertion(
      srcloc, !bActual, sExpr, istr::empty, !bActual ? istr::empty : ABC_SL("false"), ABC_SL("true")
   );
}

void test_case::assert_true(source_location const & srcloc, bool bActual, istr const & sExpr) {
   ABC_TRACE_FUNC(this, srcloc, bActual, sExpr);

   m_prunner->log_assertion(
      srcloc, bActual, sExpr, istr::empty, bActual ? istr::empty : ABC_SL("true"), ABC_SL("false")
   );
}

void test_case::assert_throws(
   source_location const & srcloc, std::function<void ()> const & fnExpr, istr const & sExpr,
   std::function<bool (std::exception const &)> const & fnMatchType, char const * pszExpectedWhat
) {
   ABC_TRACE_FUNC(this, srcloc, /*fnExpr, */sExpr, /*fnMatchType, */pszExpectedWhat);

   bool bPass = false;
   istr sCaughtWhat;
   try {
      fnExpr();
      sCaughtWhat = ABC_SL("does not throw");
   } catch (::std::exception const & x) {
      if (fnMatchType(x)) {
         bPass = true;
      } else {
         sCaughtWhat = istr(ABC_SL("throws {}")).format(istr(external_buffer, x.what()));
      }
   } catch (...) {
      sCaughtWhat = ABC_SL("unknown type");
   }
   m_prunner->log_assertion(
      srcloc, bPass, sExpr, istr::empty,
      istr(ABC_SL("throws {}")).format(istr(external_buffer, pszExpectedWhat)), sCaughtWhat
   );
}

} //namespace testing
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::test_case_factory_impl

namespace abc {
namespace testing {

test_case_factory_impl::list_item * test_case_factory_impl::sm_pliHead = nullptr;
test_case_factory_impl::list_item ** test_case_factory_impl::sm_ppliTailNext = nullptr;

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

