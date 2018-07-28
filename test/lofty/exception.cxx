/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/exception.hxx>
#include <lofty/io/text/str.hxx>
#include <lofty/logging.hxx>
#include <lofty/_std/exception.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/text/str.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

class exception_polymorphism : public testing::test_case {
protected:
   //! First-level lofty::generic_error subclass.
   class derived1_error : public generic_error {
   public:
      //! Default constructor.
      derived1_error() {
      }
   };

   //! Second-level lofty::generic_error subclass.
   class derived2_error : public derived1_error {
   public:
      //! Default constructor.
      derived2_error() {
      }
   };

public:
   //! See testing::test_case::title().
   virtual text::str title() override {
      return text::str(LOFTY_SL("lofty::exception – polymorphism"));
   }

   //! See testing::test_case::run().
   virtual void run() override {
      LOFTY_TRACE_METHOD();

      ASSERT_THROWS(exception, throw_exception());
      ASSERT_THROWS(generic_error, throw_generic_error());
      ASSERT_THROWS(derived1_error, throw_derived1_error());
      ASSERT_THROWS(derived1_error, throw_derived2_error());
      ASSERT_THROWS(derived2_error, throw_derived2_error());
   }

   void throw_exception() {
      LOFTY_TRACE_FUNC();

      LOFTY_THROW(exception, ());
   }

   void throw_generic_error() {
      LOFTY_TRACE_FUNC();

      LOFTY_THROW(generic_error, ());
   }

   void throw_derived1_error() {
      LOFTY_TRACE_FUNC();

      LOFTY_THROW(derived1_error, ());
   }

   void throw_derived2_error() {
      LOFTY_TRACE_FUNC();

      LOFTY_THROW(derived2_error, ());
   }
};

}} //namespace lofty::test

LOFTY_TESTING_REGISTER_TEST_CASE(lofty::test::exception_polymorphism)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

class exception_scope_trace : public testing::test_case {
public:
   //! See testing::test_case::title().
   virtual text::str title() override {
      return text::str(LOFTY_SL("lofty::exception – scope/stack trace generation"));
   }

   //! See testing::test_case::run().
   virtual void run() override {
      LOFTY_TRACE_METHOD();

      text::str this_str;
      this_str.format(LOFTY_SL("this={}"), this);

      text::str scope_trace;

      // Verify that the current scope trace contains this function.

      scope_trace = get_scope_trace();
      ASSERT(scope_trace.find(this_str) != scope_trace.cend());

      // Verify that an exception in run_sub_*() generates a scope trace with run_sub_*().

      try {
         run_sub_1();
      } catch (_std::exception const & x) {
         scope_trace = get_scope_trace(&x);
      }
      ASSERT(scope_trace.find(LOFTY_SL("exception_scope_trace::run_sub_2")) != scope_trace.cend());
      ASSERT(scope_trace.find(LOFTY_SL("exception_scope_trace::run_sub_1")) != scope_trace.cend());
      // This method is invoked via the polymorphic lofty::testing::runner class.
      ASSERT(scope_trace.find(LOFTY_SL("runner::run")) != scope_trace.cend());
      ASSERT(scope_trace.find(this_str) != scope_trace.cend());

      // Verify that now the scope trace does not contain run_sub_*().

      scope_trace = get_scope_trace();
      ASSERT(scope_trace.find(LOFTY_SL("exception_scope_trace::run_sub_2")) == scope_trace.cend());
      ASSERT(scope_trace.find(LOFTY_SL("exception_scope_trace::run_sub_1")) == scope_trace.cend());
      // This method is invoked via the polymorphic lofty::testing::runner class.
      ASSERT(scope_trace.find(LOFTY_SL("runner::run")) != scope_trace.cend());
      ASSERT(scope_trace.find(this_str) != scope_trace.cend());
   }

   static text::str get_scope_trace(_std::exception const * x = nullptr) {
      io::text::str_ostream ostream;
      exception::write_with_scope_trace(&ostream, x);
      return ostream.release_content();
   }

   void run_sub_1() {
      LOFTY_TRACE_FUNC();

      run_sub_2();
   }

   void run_sub_2() {
      LOFTY_TRACE_FUNC();

      throw_exception();
   }

   void throw_exception() {
      LOFTY_TRACE_FUNC();

      LOFTY_THROW(exception, ());
   }
};

}} //namespace lofty::test

LOFTY_TESTING_REGISTER_TEST_CASE(lofty::test::exception_scope_trace)
