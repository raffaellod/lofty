/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013-2015 Raffaello D. Di Napoli

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
#include <abaclade/testing/test_case.hxx>
#include <abaclade/text/char_ptr_to_str_adapter.hxx>
#include <abaclade/text.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace testing {

test_case::test_case() {
}

/*virtual*/ test_case::~test_case() {
}

void test_case::init(runner * prunner) {
   ABC_TRACE_FUNC(this, prunner);

   m_prunner = prunner;
}

void test_case::assert_does_not_throw(
   text::file_address const & tfa, _std::function<void ()> const & fnExpr, str const & sExpr
) {
   ABC_TRACE_FUNC(this, tfa, /*fnExpr, */sExpr);

   str sCaughtWhat;
   try {
      fnExpr();
   } catch (_std::exception const & x) {
      sCaughtWhat = str(ABC_SL("throws {}")).format(text::char_ptr_to_str_adapter(x.what()));
   } catch (...) {
      sCaughtWhat = ABC_SL("unknown type");
   }
   m_prunner->log_assertion(
      tfa, !sCaughtWhat, sExpr, str::empty, ABC_SL("does not throw"), sCaughtWhat
   );
}

void test_case::assert_false(text::file_address const & tfa, bool bActual, str const & sExpr) {
   ABC_TRACE_FUNC(this, tfa, bActual, sExpr);

   m_prunner->log_assertion(
      tfa, !bActual, sExpr, str::empty, !bActual ? str::empty : ABC_SL("false"), ABC_SL("true")
   );
}

void test_case::assert_true(text::file_address const & tfa, bool bActual, str const & sExpr) {
   ABC_TRACE_FUNC(this, tfa, bActual, sExpr);

   m_prunner->log_assertion(
      tfa, bActual, sExpr, str::empty, bActual ? str::empty : ABC_SL("true"), ABC_SL("false")
   );
}

void test_case::assert_throws(
   text::file_address const & tfa, _std::function<void ()> const & fnExpr, str const & sExpr,
   _std::function<bool (_std::exception const &)> const & fnMatchType, char const * pszExpectedWhat
) {
   ABC_TRACE_FUNC(this, tfa, /*fnExpr, */sExpr, /*fnMatchType, */pszExpectedWhat);

   bool bPass = false;
   str sCaughtWhat;
   try {
      fnExpr();
      sCaughtWhat = ABC_SL("does not throw");
   } catch (_std::exception const & x) {
      if (fnMatchType(x)) {
         bPass = true;
      } else {
         sCaughtWhat = str(ABC_SL("throws {}")).format(text::char_ptr_to_str_adapter(x.what()));
      }
   } catch (...) {
      sCaughtWhat = ABC_SL("unknown type");
   }
   m_prunner->log_assertion(
      tfa, bPass, sExpr, str::empty,
      str(ABC_SL("throws {}")).format(text::char_ptr_to_str_adapter(pszExpectedWhat)), sCaughtWhat
   );
}

}} //namespace abc::testing

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace testing {

test_case_factory_list::data_members test_case_factory_list::sm_dm =
   ABC_COLLECTIONS_STATIC_LIST_INITIALIZER;

}} //namespace abc::testing
