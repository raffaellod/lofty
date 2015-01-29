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
// abc::test::exception_polymorphism

namespace abc {
namespace test {

class exception_polymorphism : public testing::test_case {
protected:
   //! First-level abc::generic_error subclass.
   class derived1_error : public virtual generic_error {
   public:
      //! Constructor.
      derived1_error() :
         generic_error() {
         m_pszWhat = "abc::test::exception_polymorphism::derived1_error";
      }
   };

   //! Second-level abc::generic_error subclass.
   class derived2_error : public virtual derived1_error {
   public:
      //! Constructor.
      derived2_error() :
         derived1_error() {
         m_pszWhat = "abc::test::exception_polymorphism::derived2_error";
      }
   };

   //! Diamond-inheritance abc::generic_error subclass.
   class derived3_error : public virtual derived1_error, public virtual derived2_error {
   public:
      //! Constructor.
      derived3_error() :
         derived1_error(),
         derived2_error() {
         m_pszWhat = "abc::test::exception_polymorphism::derived3_error";
      }
   };

public:
   //! See testing::test_case::title().
   virtual istr title() override {
      return istr(ABC_SL("abc::exception – polymorphism"));
   }

   //! See testing::test_case::run().
   virtual void run() override {
      ABC_TRACE_FUNC(this);

      ABC_TESTING_ASSERT_THROWS(exception, throw_exception());
      ABC_TESTING_ASSERT_THROWS(generic_error, throw_generic_error());
      ABC_TESTING_ASSERT_THROWS(derived1_error, throw_derived1_error());
      ABC_TESTING_ASSERT_THROWS(derived1_error, throw_derived2_error());
      ABC_TESTING_ASSERT_THROWS(derived2_error, throw_derived2_error());
      ABC_TESTING_ASSERT_THROWS(derived1_error, throw_derived3_error(2351));
      ABC_TESTING_ASSERT_THROWS(derived2_error, throw_derived3_error(3512));
      ABC_TESTING_ASSERT_THROWS(derived3_error, throw_derived3_error(5123));
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

   void throw_derived3_error(int i) {
      ABC_TRACE_FUNC(this, i);

      ABC_THROW(derived3_error, ());
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::exception_polymorphism)

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {
namespace test {

ABC_TESTING_TEST_CASE_FUNC("abc::exception – conversion of hard OS errors into C++ exceptions") {
   ABC_TRACE_FUNC(this);

   {
      int * p = nullptr;
      ABC_TESTING_ASSERT_THROWS(null_pointer_error, *p = 1);
      // Check that the handler is still in place after its first activation above.
      ABC_TESTING_ASSERT_THROWS(null_pointer_error, *p = 2);

      ABC_TESTING_ASSERT_THROWS(memory_address_error, *++p = 1);
   }

   // Enable alignment checking if the architecture supports it.
#if 0 // ABC_HOST_ARCH_???
   {
      // Create an int (with another one following it) and a pointer to it.
      int i[2];
      void * p = &i[0];
      // Misalign the pointer, partly entering the second int.
      p = static_cast<std::int8_t *>(p) + 1;
      ABC_TESTING_ASSERT_THROWS(memory_access_error, *static_cast<int *>(p) = 1);
   }
#endif

   {
      // Non-obvious division by zero that can’t be detected at compile time.
      istr sEmpty;
      int iZero = static_cast<int>(sEmpty.size_in_chars()), iOne = 1;
      ABC_TESTING_ASSERT_THROWS(division_by_zero_error, iOne /= iZero);
      // The call to istr::format() makes use of the quotient, so it shouldn’t be optimized away.
      istr(ABC_SL("{}")).format(iOne);
   }
}

} //namespace test
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::exception_scope_trace

namespace abc {
namespace test {

class exception_scope_trace : public testing::test_case {
public:
   //! See testing::test_case::title().
   virtual istr title() override {
      return istr(ABC_SL("abc::exception – scope/stack trace generation"));
   }

   //! See testing::test_case::run().
   virtual void run() override {
      std::uint32_t iTestLocal = 3141592654;

      ABC_TRACE_FUNC(this, iTestLocal);

      dmstr sScopeTrace;

      // Verify that the current scope trace contains this function.

      sScopeTrace = get_scope_trace();
      ABC_TESTING_ASSERT_NOT_EQUAL(sScopeTrace.find(ABC_SL("3141592654")), sScopeTrace.cend());

      // Verify that an exception in run_sub_*() generates a scope trace with run_sub_*().

      try {
         run_sub_1(12345678u);
      } catch (std::exception const & x) {
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

   static dmstr get_scope_trace(std::exception const * px = nullptr) {
      ABC_TRACE_FUNC(px);

      io::text::str_writer tsw;
      exception::write_with_scope_trace(&tsw, px);
      return tsw.release_content();
   }

   void run_sub_1(std::uint32_t iArg) {
      ABC_TRACE_FUNC(this, iArg);

      run_sub_2(ABC_SL("spam and eggs"));
   }

   void run_sub_2(istr const & sArg) {
      ABC_TRACE_FUNC(this, sArg);

      throw_exception();
   }

   void throw_exception() {
      ABC_TRACE_FUNC(this);

      ABC_THROW(exception, ());
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::exception_scope_trace)

////////////////////////////////////////////////////////////////////////////////////////////////////
