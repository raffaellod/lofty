/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016-2017 Raffaello D. Di Napoli

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
#include <lofty/from_str.hxx>
#include <lofty/text.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test { namespace {

class type_with_ftis {
public:
   static str const twf;

public:
   str & get() {
      return s;
   }

private:
   str s;

};

str const type_with_ftis::twf(LOFTY_SL("TWF"));

}}} //namespace lofty::test::

namespace lofty {

template <>
class from_text_istream<test::type_with_ftis> {
public:
   void convert_capture(text::parsers::dynamic_match_capture const & capture0, test::type_with_ftis * dst) {
      dst->get() = capture0.str_copy();
   }

   text::parsers::dynamic_state const * format_to_parser_states(
      str const & format, text::parsers::dynamic * parser
   ) {
      LOFTY_UNUSED_ARG(format);

      return parser->create_string_state(&test::type_with_ftis::twf);
   }
};

} //namespace lofty

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   from_text_istream_basic,
   "lofty::from_text_istream – basic"
) {
   LOFTY_TRACE_FUNC(this);

   /* This assertion is more important at compile time than at run time; if the from_str() call compiles, it
   will return the correct value. */
   LOFTY_TESTING_ASSERT_EQUAL(from_str<type_with_ftis>(type_with_ftis::twf).get(), type_with_ftis::twf);
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   from_text_istream_bool,
   "lofty::from_text_istream – bool"
) {
   LOFTY_TRACE_FUNC(this);

   LOFTY_TESTING_ASSERT_EQUAL(from_str<bool>(LOFTY_SL("false")), false);
   LOFTY_TESTING_ASSERT_EQUAL(from_str<bool>(LOFTY_SL("true")), true);
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<bool>(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<bool>(LOFTY_SL("a")));
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<bool>(LOFTY_SL("atrue")));
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<bool>(LOFTY_SL("falseb")));
}

}} //namespace lofty::test
