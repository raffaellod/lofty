/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013-2018 Raffaello D. Di Napoli

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
#include <lofty/testing/app.hxx>
#include <lofty/testing/runner.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/testing/utility.hxx>

#include <lofty/io/text.hxx>
#include <lofty/logging.hxx>
#include <lofty/text.hxx>
#include <lofty/text/char_ptr_to_str_adapter.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing {

/*virtual*/ int app::main(collections::vector<str> & args) /*override*/ {
   LOFTY_TRACE_METHOD();

   LOFTY_UNUSED_ARG(args);

   runner r(io::text::stderr);
   r.load_registered_test_cases();
   r.run();
   bool all_passed = r.log_summary();

   return all_passed ? 0 : 1;
}

}} //namespace lofty::testing

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
   LOFTY_FOR_EACH(auto const & factory_list_elt, test_case_factory_list::instance()) {
      // Instantiate the test case.
      test_cases.push_back(factory_list_elt.factory(this));
   }
}

void runner::log_assertion(
   text::file_address const & file_addr, str const & expr, assertion_expr * assertion_expr_
) {
   str format;
   if (assertion_expr_->pass) {
      format = LOFTY_SL("COMK-TEST-ASSERT-PASS {}: pass: {}\n");
   } else {
      format = LOFTY_SL("COMK-TEST-ASSERT-FAIL {}: fail: {}\n");
   }
   ostream->print(format, file_addr, expr);
   if (!assertion_expr_->pass) {
      ++failed_assertions;
      if (assertion_expr_->binary_op) {
         ostream->print(
            LOFTY_SL("  actual: {} {} {}\n"),
            assertion_expr_->left, assertion_expr_->binary_op, assertion_expr_->right
         );
      } else {
         ostream->print(LOFTY_SL("  actual: {}\n"), assertion_expr_->left);
      }
   }
}

bool runner::log_summary() {
   return failed_assertions == 0;
}

void runner::run() {
   LOFTY_TRACE_METHOD();

   for (auto itr(test_cases.begin()); itr != test_cases.end(); ++itr) {
      run_test_case(**itr);
   }
}

void runner::run_test_case(class test_case & test_case) {
   LOFTY_TRACE_METHOD();

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

void runner::assertion_expr::set(bool pass_, char const * binary_op_) {
   pass = pass_;
   if (binary_op_) {
      binary_op = str(external_buffer, binary_op_);
   } else {
      binary_op.clear();
   }
}

}} //namespace lofty::testing

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing {

test_case::test_case() {
}

/*virtual*/ test_case::~test_case() {
}

void test_case::init(class runner * runner_) {
   runner = runner_;
}

void test_case::assert(text::file_address const & file_addr, str const & expr) {
   runner->log_assertion(file_addr, expr, &assertion_expr);
}

void test_case::assert_does_not_throw(
   text::file_address const & file_addr, str const & expr, _std::function<void ()> expr_fn
) {
   assertion_expr.pass = false;
   assertion_expr.binary_op.clear();
   try {
      expr_fn();
      assertion_expr.pass = true;
   } catch (_std::exception const & x) {
      assertion_expr.left.format(
         LOFTY_SL("throws {}: {}"), typeid(x), text::char_ptr_to_str_adapter(x.what())
      );
   } catch (...) {
      assertion_expr.left = LOFTY_SL("throws an exception of unknown type");
   }
   assert(file_addr, expr);
}

void test_case::assert_throws(
   text::file_address const & file_addr, str const & expr,
   _std::function<bool (_std::exception const *)> expr_instanceof_fn
) {
   assertion_expr.pass = false;
   assertion_expr.binary_op.clear();
   try {
      expr_instanceof_fn(nullptr);
      assertion_expr.left = LOFTY_SL("does not throw");
   } catch (_std::exception const & x) {
      if (expr_instanceof_fn(&x)) {
         assertion_expr.pass = true;
      } else {
         assertion_expr.left.format(
            LOFTY_SL("throws {}: {}"), typeid(x), text::char_ptr_to_str_adapter(x.what())
         );
      }
   } catch (...) {
      assertion_expr.left = LOFTY_SL("throws an exception of unknown type");
   }
   assert(file_addr, expr);
}


test_case_factory_list::data_members test_case_factory_list::data_members_ =
   LOFTY_COLLECTIONS_STATIC_LIST_INITIALIZER;

}} //namespace lofty::testing

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing { namespace utility {

std::size_t instances_counter::copies_ = 0;
std::size_t instances_counter::moves_ = 0;
std::size_t instances_counter::new_ = 0;
int instances_counter::next_unique = 0;

}}} //namespace lofty::testing::utility
