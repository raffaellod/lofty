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

ABC_TESTING_TEST_CASE_FUNC(
   to_str_int,
   "abc::to_str – int"
) {
   ABC_TRACE_FUNC(this);

   // Test zero, decimal base.
   ABC_TESTING_ASSERT_EQUAL(to_str(0, istr::empty), ABC_SL("0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(0, ABC_SL(" 1")), ABC_SL(" 0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(0, ABC_SL("01")), ABC_SL("0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(0, ABC_SL(" 2")), ABC_SL(" 0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(0, ABC_SL("02")), ABC_SL("00"));

   // Test positive values, decimal base.
   ABC_TESTING_ASSERT_EQUAL(to_str(1, istr::empty), ABC_SL("1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(1, ABC_SL(" 1")), ABC_SL(" 1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(1, ABC_SL("01")), ABC_SL("1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(1, ABC_SL(" 2")), ABC_SL(" 1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(1, ABC_SL("02")), ABC_SL("01"));

   // Test negative values, decimal base.
   ABC_TESTING_ASSERT_EQUAL(to_str(-1, istr::empty), ABC_SL("-1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(-1, ABC_SL(" 1")), ABC_SL("-1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(-1, ABC_SL("01")), ABC_SL("-1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(-1, ABC_SL(" 2")), ABC_SL("-1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(-1, ABC_SL("02")), ABC_SL("-1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(-1, ABC_SL(" 3")), ABC_SL(" -1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(-1, ABC_SL("03")), ABC_SL("-01"));
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   to_str_std_int8_t,
   "abc::to_str – std::int8_t"
) {
   ABC_TRACE_FUNC(this);

   // Test zero, hexadecimal base.
   ABC_TESTING_ASSERT_EQUAL(to_str(std::int8_t(0), ABC_SL("x")), ABC_SL("0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(std::int8_t(0), ABC_SL(" 1x")), ABC_SL("0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(std::int8_t(0), ABC_SL("01x")), ABC_SL("0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(std::int8_t(0), ABC_SL(" 2x")), ABC_SL(" 0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(std::int8_t(0), ABC_SL("02x")), ABC_SL("00"));

   // Test positive values, hexadecimal base.
   ABC_TESTING_ASSERT_EQUAL(to_str(std::int8_t(1), ABC_SL("x")), ABC_SL("1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(std::int8_t(1), ABC_SL(" 1x")), ABC_SL("1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(std::int8_t(1), ABC_SL("01x")), ABC_SL("1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(std::int8_t(1), ABC_SL(" 2x")), ABC_SL(" 1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(std::int8_t(1), ABC_SL("02x")), ABC_SL("01"));

   // Test negative values, hexadecimal base.
   ABC_TESTING_ASSERT_EQUAL(to_str(std::int8_t(-1), ABC_SL("x")), ABC_SL("ff"));
   ABC_TESTING_ASSERT_EQUAL(to_str(std::int8_t(-1), ABC_SL(" 1x")), ABC_SL("ff"));
   ABC_TESTING_ASSERT_EQUAL(to_str(std::int8_t(-1), ABC_SL("01x")), ABC_SL("ff"));
   ABC_TESTING_ASSERT_EQUAL(to_str(std::int8_t(-1), ABC_SL(" 2x")), ABC_SL("ff"));
   ABC_TESTING_ASSERT_EQUAL(to_str(std::int8_t(-1), ABC_SL("02x")), ABC_SL("ff"));
   ABC_TESTING_ASSERT_EQUAL(to_str(std::int8_t(-1), ABC_SL(" 3x")), ABC_SL(" ff"));
   ABC_TESTING_ASSERT_EQUAL(to_str(std::int8_t(-1), ABC_SL("03x")), ABC_SL("0ff"));
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   to_str_raw_ptr,
   "abc::to_str – raw pointers"
) {
   ABC_TRACE_FUNC(this);

   std::uintptr_t iBad = 0xbad;

   // Test nullptr.
   ABC_TESTING_ASSERT_EQUAL(to_str(static_cast<void *>(nullptr), istr::empty), ABC_SL("nullptr"));

   // Test void pointer.
   ABC_TESTING_ASSERT_EQUAL(to_str(reinterpret_cast<void *>(iBad), istr::empty), ABC_SL("0xbad"));

   // Test void const volatile pointer.
   ABC_TESTING_ASSERT_EQUAL(
      to_str(reinterpret_cast<void const volatile *>(iBad), istr::empty), ABC_SL("0xbad")
   );

   // Test function pointer.
   ABC_TESTING_ASSERT_EQUAL(
      to_str(reinterpret_cast<void (*)(int)>(iBad), istr::empty), ABC_SL("0xbad")
   );

   /* Test char_t const pointer. Also confirms that pointers-to-char are NOT treated as strings
   by abc::to_str(). */
   ABC_TESTING_ASSERT_EQUAL(
      to_str(reinterpret_cast<char_t const *>(iBad), istr::empty), ABC_SL("0xbad")
   );
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   to_str_smart_ptr,
   "abc::to_str – smart pointers"
) {
   ABC_TRACE_FUNC(this);

   int * pi = new int;
   istr sPtr(to_str(pi));

   {
      _std::unique_ptr<int> upi(pi);
      // Test non-nullptr _std::unique_ptr.
      ABC_TESTING_ASSERT_EQUAL(to_str(upi, istr::empty), sPtr);

      upi.release();
      // Test nullptr _std::unique_ptr.
      ABC_TESTING_ASSERT_EQUAL(to_str(upi, istr::empty), ABC_SL("nullptr"));
   }
   {
      _std::shared_ptr<int> spi(pi);
      // Test non-nullptr _std::shared_ptr.
      ABC_TESTING_ASSERT_EQUAL(to_str(spi, istr::empty), sPtr);
      _std::weak_ptr<int> wpi(spi);
      // Test non-nullptr _std::weak_ptr.
      ABC_TESTING_ASSERT_EQUAL(to_str(wpi, istr::empty), sPtr);

      spi.reset();
      // Test nullptr _std::shared_ptr.
      ABC_TESTING_ASSERT_EQUAL(to_str(spi, istr::empty), ABC_SL("nullptr"));
      // Test expired non-nullptr _std::weak_ptr.
      ABC_TESTING_ASSERT_EQUAL(to_str(wpi, istr::empty), ABC_SL("nullptr"));

      wpi.reset();
      // Test nullptr _std::weak_ptr.
      ABC_TESTING_ASSERT_EQUAL(to_str(wpi, istr::empty), ABC_SL("nullptr"));
   }
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   to_str_tuple,
   "abc::to_str – STL tuple types"
) {
   ABC_TRACE_FUNC(this);

   // Test _std::tuple.
   ABC_TESTING_ASSERT_EQUAL(to_str(_std::tuple<>()), ABC_SL("()"));
   ABC_TESTING_ASSERT_EQUAL(to_str(_std::tuple<int>(1)), ABC_SL("(1)"));
   ABC_TESTING_ASSERT_EQUAL(to_str(_std::tuple<int, int>(1, 2)), ABC_SL("(1, 2)"));
   ABC_TESTING_ASSERT_EQUAL(to_str(_std::tuple<istr, int>(ABC_SL("abc"), 42)), ABC_SL("(abc, 42)"));
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

union union_type {
   int i;
   char ch;
};

struct struct_type {
   int i;
   char ch;
};

class class_type {
   int i;
   char ch;
};

ABC_TESTING_TEST_CASE_FUNC(
   to_str_std_type_info,
   "abc::to_str – std::type_info"
) {
   ABC_TRACE_FUNC(this);

   // Test std::type_info.
   ABC_TESTING_ASSERT_EQUAL(to_str(typeid(1)), ABC_SL("int"));
   ABC_TESTING_ASSERT_EQUAL(to_str(typeid(double)), ABC_SL("double"));
   ABC_TESTING_ASSERT_EQUAL(to_str(typeid(bool)), ABC_SL("bool"));
   ABC_TESTING_ASSERT_EQUAL(to_str(typeid(union_type )), ABC_SL("abc::test::union_type" ));
   ABC_TESTING_ASSERT_EQUAL(to_str(typeid(struct_type)), ABC_SL("abc::test::struct_type"));
   ABC_TESTING_ASSERT_EQUAL(to_str(typeid(class_type )), ABC_SL("abc::test::class_type" ));
}

}} //namespace abc::test
