/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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
#include <abaclade/io/text/file.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::binbuf_reader_read_line_utf8_lf_no_trailing_nl

namespace abc {
namespace test {

class binbuf_reader_read_line_utf8_lf_no_trailing_nl :
   public testing::test_case {
public:

   //! See testing::test_case::title().
   virtual istr title() {
      return istr(ABC_SL(
         "abc::io::text::binbuf_reader – reading line-by-line, UTF-8, LF, no trailing LF"
      ));
   }

   //! See testing::test_case::run().
   virtual void run() {
      ABC_TRACE_FUNC(this);

      auto ptrIn(io::text::open_reader(
         file_path(ABC_SL("src/abaclade-test/io/text/data/utf8_lf_no-trailing-nl.txt"))
      ));
      dmstr s;
      unsigned i(1);
      while (ptrIn->read_line(&s)) {
         ABC_TESTING_ASSERT_EQUAL(i++, s.size());
      }
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::binbuf_reader_read_line_utf8_lf_no_trailing_nl)


////////////////////////////////////////////////////////////////////////////////////////////////////

