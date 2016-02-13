/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2016 Raffaello D. Di Napoli

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
#include <abaclade/to_str.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test { namespace {

class type_with_member_ttos {
public:
   type_with_member_ttos(str s) :
      m_s(_std::move(s)) {
   }

   str const & get() const {
      return m_s;
   }

   void to_text_ostream(io::text::ostream * ptos) const {
      ptos->write(m_s);
   }

private:
   str m_s;
};

class type_with_nonmember_ttos {
public:
   type_with_nonmember_ttos(str s) :
      m_s(_std::move(s)) {
   }

   str const & get() const {
      return m_s;
   }

private:
   str m_s;
};

}}} //namespace abc::test::

namespace abc {

template <>
class to_text_ostream<test::type_with_nonmember_ttos> {
public:
   void set_format(str const & sFormat) {
      ABC_UNUSED_ARG(sFormat);
   }

   void write(test::type_with_nonmember_ttos const & twnmt, io::text::ostream * ptos) {
      ptos->write(twnmt.get());
   }
};

} //namespace abc

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   to_text_ostream_member_nonmember,
   "abc::to_text_ostream – member and non-member to_text_ostream"
) {
   ABC_TRACE_FUNC(this);

   type_with_member_ttos    twmt(ABC_SL("TWMT"));
   type_with_nonmember_ttos twnt(ABC_SL("TWNT"));

   /* These assertions are more important at compile time than at run time; if the to_str() calls
   compile, they won’t return the wrong value. */
   ABC_TESTING_ASSERT_EQUAL(to_str(twmt), twmt.get());
   ABC_TESTING_ASSERT_EQUAL(to_str(twnt), twnt.get());
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   to_text_ostream_bool,
   "abc::to_text_ostream – bool"
) {
   ABC_TRACE_FUNC(this);

   ABC_TESTING_ASSERT_EQUAL(to_str(false), ABC_SL("false"));
   ABC_TESTING_ASSERT_EQUAL(to_str(true), ABC_SL("true"));
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   to_text_ostream_int,
   "abc::to_text_ostream – int"
) {
   ABC_TRACE_FUNC(this);

   // Test zero, decimal base.
   ABC_TESTING_ASSERT_EQUAL(to_str(0, str::empty), ABC_SL("0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(0, ABC_SL(" 1")), ABC_SL(" 0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(0, ABC_SL("01")), ABC_SL("0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(0, ABC_SL(" 2")), ABC_SL(" 0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(0, ABC_SL("02")), ABC_SL("00"));

   // Test positive values, decimal base.
   ABC_TESTING_ASSERT_EQUAL(to_str(1, str::empty), ABC_SL("1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(1, ABC_SL(" 1")), ABC_SL(" 1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(1, ABC_SL("01")), ABC_SL("1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(1, ABC_SL(" 2")), ABC_SL(" 1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(1, ABC_SL("02")), ABC_SL("01"));

   // Test negative values, decimal base.
   ABC_TESTING_ASSERT_EQUAL(to_str(-1, str::empty), ABC_SL("-1"));
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
   to_text_ostream_std_int8_t,
   "abc::to_text_ostream – std::int8_t"
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
   to_text_ostream_raw_ptr,
   "abc::to_text_ostream – raw pointers"
) {
   ABC_TRACE_FUNC(this);

   std::uintptr_t iBad = 0xbad;

   // Test nullptr.
   ABC_TESTING_ASSERT_EQUAL(to_str(static_cast<void *>(nullptr), str::empty), ABC_SL("nullptr"));

   // Test void pointer.
   ABC_TESTING_ASSERT_EQUAL(to_str(reinterpret_cast<void *>(iBad), str::empty), ABC_SL("0xbad"));

   // Test void const volatile pointer.
   ABC_TESTING_ASSERT_EQUAL(
      to_str(reinterpret_cast<void const volatile *>(iBad), str::empty), ABC_SL("0xbad")
   );

   // Test function pointer.
   ABC_TESTING_ASSERT_EQUAL(
      to_str(reinterpret_cast<void (*)(int)>(iBad), str::empty), ABC_SL("0xbad")
   );

   /* Test char_t const pointer. Also confirms that pointers-to-char are NOT treated as strings
   by abc::to_text_ostream(). */
   ABC_TESTING_ASSERT_EQUAL(
      to_str(reinterpret_cast<char_t const *>(iBad), str::empty), ABC_SL("0xbad")
   );
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   to_text_ostream_smart_ptr,
   "abc::to_text_ostream – smart pointers"
) {
   ABC_TRACE_FUNC(this);

   int * pi = new int;
   str sPtr(to_str(pi));

   {
      _std::unique_ptr<int> upi(pi);
      // Test non-nullptr _std::unique_ptr.
      ABC_TESTING_ASSERT_EQUAL(to_str(upi, str::empty), sPtr);

      upi.release();
      // Test nullptr _std::unique_ptr.
      ABC_TESTING_ASSERT_EQUAL(to_str(upi, str::empty), ABC_SL("nullptr"));
   }
   {
      _std::shared_ptr<int> spi(pi);
      // Test non-nullptr _std::shared_ptr.
      ABC_TESTING_ASSERT_EQUAL(to_str(spi, str::empty), sPtr);
      _std::weak_ptr<int> wpi(spi);
      // Test non-nullptr _std::weak_ptr.
      ABC_TESTING_ASSERT_EQUAL(to_str(wpi, str::empty), sPtr);

      spi.reset();
      // Test nullptr _std::shared_ptr.
      ABC_TESTING_ASSERT_EQUAL(to_str(spi, str::empty), ABC_SL("nullptr"));
      // Test expired non-nullptr _std::weak_ptr.
      ABC_TESTING_ASSERT_EQUAL(to_str(wpi, str::empty), ABC_SL("nullptr"));

      wpi.reset();
      // Test nullptr _std::weak_ptr.
      ABC_TESTING_ASSERT_EQUAL(to_str(wpi, str::empty), ABC_SL("nullptr"));
   }
}

}} //namespace abc::test

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   to_text_ostream_tuple,
   "abc::to_text_ostream – STL tuple types"
) {
   ABC_TRACE_FUNC(this);

   // Test _std::tuple.
   ABC_TESTING_ASSERT_EQUAL(to_str(_std::tuple<>()), ABC_SL("()"));
   ABC_TESTING_ASSERT_EQUAL(to_str(_std::tuple<int>(1)), ABC_SL("(1)"));
   ABC_TESTING_ASSERT_EQUAL(to_str(_std::tuple<int, int>(1, 2)), ABC_SL("(1, 2)"));
   ABC_TESTING_ASSERT_EQUAL(to_str(_std::tuple<str, int>(ABC_SL("abc"), 42)), ABC_SL("(abc, 42)"));
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
   to_text_ostream_std_type_info,
   "abc::to_text_ostream – std::type_info"
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
