/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013, 2014, 2015
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

namespace abc { namespace test {

class exception_polymorphism : public testing::test_case {
protected:
   //! First-level abc::generic_error subclass.
   class derived1_error : public generic_error {
   public:
      //! Constructor.
      derived1_error() {
         m_pszWhat = "abc::test::exception_polymorphism::derived1_error";
      }
   };

   //! Second-level abc::generic_error subclass.
   class derived2_error : public derived1_error {
   public:
      //! Constructor.
      derived2_error() {
         m_pszWhat = "abc::test::exception_polymorphism::derived2_error";
      }
   };

public:
   //! See testing::test_case::title().
   virtual str title() override {
      return str(ABC_SL("abc::exception – polymorphism"));
   }

   //! See testing::test_case::run().
   virtual void run() override {
      ABC_TRACE_FUNC(this);

      ABC_TESTING_ASSERT_THROWS(exception, throw_exception());
      ABC_TESTING_ASSERT_THROWS(generic_error, throw_generic_error());
      ABC_TESTING_ASSERT_THROWS(derived1_error, throw_derived1_error());
      ABC_TESTING_ASSERT_THROWS(derived1_error, throw_derived2_error());
      ABC_TESTING_ASSERT_THROWS(derived2_error, throw_derived2_error());
   }

   void throw_exception() {
      ABC_TRACE_FUNC(this);

      ABC_THROW(exception, ());
   }

   void throw_generic_error() {
      ABC_TRACE_FUNC(this);

      ABC_THROW(generic_error, ());
   }

   void throw_derived1_error() {
      ABC_TRACE_FUNC(this);

      ABC_THROW(derived1_error, ());
   }

   void throw_derived2_error() {
      ABC_TRACE_FUNC(this);

      ABC_THROW(derived2_error, ());
   }
};

}} //namespace abc::test

ABC_TESTING_REGISTER_TEST_CASE(abc::test::exception_polymorphism)

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

class exception_scope_trace : public testing::test_case {
public:
   //! See testing::test_case::title().
   virtual str title() override {
      return str(ABC_SL("abc::exception – scope/stack trace generation"));
   }

   //! See testing::test_case::run().
   virtual void run() override {
      std::uint32_t iTestLocal = 3141592654;

      ABC_TRACE_FUNC(this, iTestLocal);

      str sScopeTrace;

      // Verify that the current scope trace contains this function.

      sScopeTrace = get_scope_trace();
      ABC_TESTING_ASSERT_NOT_EQUAL(sScopeTrace.find(ABC_SL("3141592654")), sScopeTrace.cend());

      // Verify that an exception in run_sub_*() generates a scope trace with run_sub_*().

      try {
         run_sub_1(12345678u);
      } catch (_std::exception const & x) {
         sScopeTrace = get_scope_trace(&x);
      }
      ABC_TESTING_ASSERT_NOT_EQUAL(
         sScopeTrace.find(ABC_SL("exception_scope_trace::run_sub_2")), sScopeTrace.cend()
      );
      ABC_TESTING_ASSERT_NOT_EQUAL(sScopeTrace.find(ABC_SL("spam and eggs")), sScopeTrace.cend());
      ABC_TESTING_ASSERT_NOT_EQUAL(
         sScopeTrace.find(ABC_SL("exception_scope_trace::run_sub_1")), sScopeTrace.cend()
      );
      ABC_TESTING_ASSERT_NOT_EQUAL(sScopeTrace.find(ABC_SL("12345678")), sScopeTrace.cend());
      // This method is invoked via the polymorphic abc::testing::runner class.
      ABC_TESTING_ASSERT_NOT_EQUAL(sScopeTrace.find(ABC_SL("runner::run")), sScopeTrace.cend());
      ABC_TESTING_ASSERT_NOT_EQUAL(sScopeTrace.find(ABC_SL("3141592654")), sScopeTrace.cend());

      // Verify that now the scope trace does not contain run_sub_*().

      sScopeTrace = get_scope_trace();
      ABC_TESTING_ASSERT_EQUAL(
         sScopeTrace.find(ABC_SL("exception_scope_trace::run_sub_2")), sScopeTrace.cend()
      );
      ABC_TESTING_ASSERT_EQUAL(sScopeTrace.find(ABC_SL("spam and eggs")), sScopeTrace.cend());
      ABC_TESTING_ASSERT_EQUAL(
         sScopeTrace.find(ABC_SL("exception_scope_trace::run_sub_1")), sScopeTrace.cend()
      );
      ABC_TESTING_ASSERT_EQUAL(sScopeTrace.find(ABC_SL("12345678")), sScopeTrace.cend());
      // This method is invoked via the polymorphic abc::testing::runner class.
      ABC_TESTING_ASSERT_NOT_EQUAL(sScopeTrace.find(ABC_SL("runner::run")), sScopeTrace.cend());
      ABC_TESTING_ASSERT_NOT_EQUAL(sScopeTrace.find(ABC_SL("3141592654")), sScopeTrace.cend());
   }

   static str get_scope_trace(_std::exception const * px = nullptr) {
      ABC_TRACE_FUNC(px);

      io::text::str_writer tsw;
      exception::write_with_scope_trace(&tsw, px);
      return tsw.release_content();
   }

   void run_sub_1(std::uint32_t iArg) {
      ABC_TRACE_FUNC(this, iArg);

      run_sub_2(ABC_SL("spam and eggs"));
   }

   void run_sub_2(str const & sArg) {
      ABC_TRACE_FUNC(this, sArg);

      throw_exception();
   }

   void throw_exception() {
      ABC_TRACE_FUNC(this);

      ABC_THROW(exception, ());
   }
};

}} //namespace abc::test

ABC_TESTING_REGISTER_TEST_CASE(abc::test::exception_scope_trace)
