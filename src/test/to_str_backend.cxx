/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013
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

#include <abc/testing/test_case.hxx>
#include <abc/testing/mock/iostream.hxx>
#include <abc/trace.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::to_str_backend_test_case_base

namespace abc {

namespace test {

class to_str_backend_test_case_base :
   public testing::test_case {
protected:

   /** Writes the first argument to an internal ostream, using the second as format specification,
   and returns the resulting stream contents.

   t
      Value to output.
   achFormatSpec
      Format specification for t.
   return
      The resulting contents of the internal stream.
   */
   template <typename T, size_t t_cchFormatSpec>
   istr const get_to_str_backend_output(
      T const & t, char_t const (& achFormatSpec)[t_cchFormatSpec]
   ) {
      ABC_TRACE_FN((t, achFormatSpec));

      to_str_backend<T> tsb(achFormatSpec);
      m_mos.reset();
      tsb.write(t, &m_mos);
      return m_mos.contents();
   }


protected:

   /** Stream to convert to string into. */
   testing::mock::ostream m_mos;
};

} //namespace test

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::to_str_backend_int

namespace abc {

namespace test {

class to_str_backend_int :
   public to_str_backend_test_case_base {
public:

   /** See to_str_backend_test_case_base::title().
   */
   virtual istr title() {
      return istr(SL("abc::to_str_backend - int"));
   }


   /** See to_str_backend_test_case_base::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      // Test zero, decimal base.
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(0, SL("")), SL("0"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(0, SL(" 1")), SL(" 0"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(0, SL("01")), SL("0"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(0, SL(" 2")), SL(" 0"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(0, SL("02")), SL("00"));

      // Test positive values, decimal base.
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(1, SL("")), SL("1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(1, SL(" 1")), SL(" 1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(1, SL("01")), SL("1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(1, SL(" 2")), SL(" 1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(1, SL("02")), SL("01"));

      // Test negative values, decimal base.
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(-1, SL("")), SL("-1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(-1, SL(" 1")), SL("-1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(-1, SL("01")), SL("-1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(-1, SL(" 2")), SL("-1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(-1, SL("02")), SL("-1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(-1, SL(" 3")), SL(" -1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(-1, SL("03")), SL("-01"));
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::to_str_backend_int)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::to_str_backend_int8

namespace abc {

namespace test {

class to_str_backend_int8 :
   public to_str_backend_test_case_base {
public:

   /** See to_str_backend_test_case_base::title().
   */
   virtual istr title() {
      return istr(SL("abc::to_str_backend - int8_t"));
   }


   /** See to_str_backend_test_case_base::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      // Test zero, hexadecimal base.
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(int8_t(0), SL("x")), SL("0"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(int8_t(0), SL(" 1x")), SL("0"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(int8_t(0), SL("01x")), SL("0"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(int8_t(0), SL(" 2x")), SL(" 0"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(int8_t(0), SL("02x")), SL("00"));

      // Test positive values, hexadecimal base.
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(int8_t(1), SL("x")), SL("1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(int8_t(1), SL(" 1x")), SL("1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(int8_t(1), SL("01x")), SL("1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(int8_t(1), SL(" 2x")), SL(" 1"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(int8_t(1), SL("02x")), SL("01"));

      // Test negative values, hexadecimal base.
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(int8_t(-1), SL("x")), SL("ff"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(int8_t(-1), SL(" 1x")), SL("ff"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(int8_t(-1), SL("01x")), SL("ff"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(int8_t(-1), SL(" 2x")), SL("ff"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(int8_t(-1), SL("02x")), SL("ff"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(int8_t(-1), SL(" 3x")), SL(" ff"));
      ABC_TESTING_ASSERT_EQUAL(get_to_str_backend_output(int8_t(-1), SL("03x")), SL("0ff"));
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::to_str_backend_int8)


////////////////////////////////////////////////////////////////////////////////////////////////////

