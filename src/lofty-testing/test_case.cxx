/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013-2015, 2017 Raffaello D. Di Napoli

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
#include <lofty/testing/test_case.hxx>
#include <lofty/text/char_ptr_to_str_adapter.hxx>
#include <lofty/text.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing {

test_case::test_case() {
}

/*virtual*/ test_case::~test_case() {
}

void test_case::init(class runner * runner_) {
   LOFTY_TRACE_FUNC(this, runner_);

   runner = runner_;
}

void test_case::assert_does_not_throw(
   text::file_address const & file_addr, _std::function<void ()> expr_fn, str const & expr
) {
   LOFTY_TRACE_FUNC(this, file_addr, /*expr_fn, */expr);

   text::sstr<64> caught;
   try {
      expr_fn();
   } catch (_std::exception const & x) {
      caught.format(LOFTY_SL("throws {}"), typeid(x));
   } catch (...) {
      caught = LOFTY_SL("unknown type");
   }
   runner->log_assertion(file_addr, !caught, expr, str::empty, LOFTY_SL("does not throw"), caught.str());
}

void test_case::assert_false(text::file_address const & file_addr, bool actual, str const & expr) {
   LOFTY_TRACE_FUNC(this, file_addr, actual, expr);

   runner->log_assertion(
      file_addr, !actual, expr, str::empty, !actual ? str::empty : LOFTY_SL("false"), LOFTY_SL("true")
   );
}

void test_case::assert_true(text::file_address const & file_addr, bool actual, str const & expr) {
   LOFTY_TRACE_FUNC(this, file_addr, actual, expr);

   runner->log_assertion(
      file_addr, actual, expr, str::empty, actual ? str::empty : LOFTY_SL("true"), LOFTY_SL("false")
   );
}

void test_case::assert_throws(
   text::file_address const & file_addr, _std::function<void ()> expr_fn, str const & expr,
   _std::function<bool (_std::exception const &)> instanceof_fn, _std::type_info const & expected_type
) {
   LOFTY_TRACE_FUNC(this, file_addr, /*expr_fn, */expr, /*instanceof_fn, */expected_type);

   bool pass = false;
   text::sstr<64> caught, expected;
   expected.format(LOFTY_SL("throws {}"), expected_type);
   try {
      expr_fn();
      caught = LOFTY_SL("does not throw");
   } catch (_std::exception const & x) {
      if (instanceof_fn(x)) {
         pass = true;
      }
      caught.format(LOFTY_SL("throws {}"), typeid(x));
   } catch (...) {
      caught = LOFTY_SL("unknown type");
   }
   runner->log_assertion(file_addr, pass, expr, str::empty, expected.str(), caught.str());
}

}} //namespace lofty::testing

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing {

test_case_factory_list::data_members test_case_factory_list::data_members_ =
   LOFTY_COLLECTIONS_STATIC_LIST_INITIALIZER;

}} //namespace lofty::testing
