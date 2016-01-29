/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016 Raffaello D. Di Napoli

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
#include <abaclade/from_str.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
namespace abc { namespace test { namespace {

class type_with_member_ftis {
public:
   type_with_member_ftis(str s) :
      m_s(_std::move(s)) {
   }

   str const & get() const {
      return m_s;
   }

   void from_text_istream(io::text::istream * ptis) {
      ptis->read(m_s);
   }

private:
   str m_s;
};

class type_with_nonmember_ftis {
public:
   type_with_nonmember_ftis(str s) :
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
class from_text_istream<test::type_with_nonmember_ftis> {
public:
   void set_format(str const & sFormat) {
      ABC_UNUSED_ARG(sFormat);
   }

   void read(test::type_with_nonmember_ftis * ptwnmt, io::text::istream * ptis) {
      ptis->read(ptwnmt->get());
   }
};

} //namespace abc

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   from_text_istream_member_nonmember,
   "abc::from_text_istream – member and non-member from_text_istream"
) {
   ABC_TRACE_FUNC(this);

   str sTwmf(ABC_SL("TWMF")), sTwnf(ABC_SL("TWNF"));

   /* These assertions are more important at compile time than at run time; if the from_str() calls
   compile, they won’t return the wrong value. */
   ABC_TESTING_ASSERT_EQUAL(from_str<type_with_member_ftis   >(twmf).get(), sTwmf);
   ABC_TESTING_ASSERT_EQUAL(from_str<type_with_nonmember_ftis>(twnf).get(), sTwnf);
}

}} //namespace abc::test
#endif
