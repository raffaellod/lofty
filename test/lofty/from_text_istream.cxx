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

class type_with_member_ftis {
public:
   str const & get() const {
      return s;
   }

   void from_text_istream(io::text::istream * istream) {
      s.clear();
      while (str peeked = istream->peek_chars(1)) {
         s += peeked;
         istream->consume_chars(peeked.size_in_chars());
      }
   }

private:
   str s;
};

class type_with_nonmember_ftis {
public:
   str & get() {
      return s;
   }

private:
   str s;
};

}}} //namespace lofty::test::

namespace lofty {

template <>
class from_text_istream<test::type_with_nonmember_ftis> {
public:
   void set_format(str const & format) {
      LOFTY_UNUSED_ARG(format);
   }

   void read(test::type_with_nonmember_ftis * ptwnmt, io::text::istream * istream) {
      str & s = ptwnmt->get();
      s.clear();
      while (str peeked = istream->peek_chars(1)) {
         s += peeked;
         istream->consume_chars(peeked.size_in_chars());
      }
   }
};

} //namespace lofty

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   from_text_istream_member_nonmember,
   "lofty::from_text_istream – member and non-member from_text_istream"
) {
   LOFTY_TRACE_FUNC(this);

   str twmf(LOFTY_SL("TWMF")), twnf(LOFTY_SL("TWNF"));

   /* These assertions are more important at compile time than at run time; if the from_str() calls compile,
   they won’t return the wrong value. */
   LOFTY_TESTING_ASSERT_EQUAL(from_str<type_with_member_ftis>(twmf).get(), twmf);
   LOFTY_TESTING_ASSERT_EQUAL(from_str<type_with_nonmember_ftis>(twnf).get(), twnf);
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
}

}} //namespace lofty::test
