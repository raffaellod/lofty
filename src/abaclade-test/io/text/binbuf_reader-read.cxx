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
// abc::test::binbuf_reader_read_line_base

namespace abc {
namespace test {

class binbuf_reader_read_line_base :
   public testing::test_case {
public:

   /*! Returns the path to the test data.

   return
      Relative path to the test data file.
   */
   virtual file_path get_test_data() = 0;

   //! See testing::test_case::run().
   virtual void run() override {
      ABC_TRACE_FUNC(this);

      auto ptrIn(io::text::open_reader(get_test_data()));
      dmstr sLine;
      unsigned i(1);
      while (ptrIn->read_line(&sLine)) {
         ABC_TESTING_ASSERT_EQUAL(sLine.size(), i++);
      }
   }
};

} //namespace test
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::binbuf_reader_read_line_utf8_lf_no_trailing_nl

namespace abc {
namespace test {

class binbuf_reader_read_line_utf8_lf_no_trailing_nl :
   public binbuf_reader_read_line_base {
public:

   //! See binbuf_reader_read_line_base::title().
   virtual istr title() override {
      return istr(ABC_SL(
         "abc::io::text::binbuf_reader – reading line-by-line, UTF-8, LF, no trailing LF"
      ));
   }

   //! See binbuf_reader_read_line_base::get_test_data().
   virtual file_path get_test_data() override {
      return file_path(ABC_SL("src/abaclade-test/io/text/data/utf8_lf_no-trailing-nl.txt"));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::binbuf_reader_read_line_utf8_lf_no_trailing_nl)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::binbuf_reader_read_line_utf8_mixed_no_trailing_nl

namespace abc {
namespace test {

class binbuf_reader_read_line_utf8_mixed_no_trailing_nl :
   public binbuf_reader_read_line_base {
public:

   //! See binbuf_reader_read_line_base::title().
   virtual istr title() override {
      return istr(
         ABC_SL("abc::io::text::binbuf_reader – ")
         ABC_SL("reading line-by-line, UTF-8, CR/LF/CRLF mix, no trailing LF")
      );
   }

   //! See binbuf_reader_read_line_base::get_test_data().
   virtual file_path get_test_data() override {
      return file_path(ABC_SL("src/abaclade-test/io/text/data/utf8_mixed_no-trailing-nl.txt"));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::binbuf_reader_read_line_utf8_mixed_no_trailing_nl)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::binbuf_reader_read_line_utf16_mixed_no_trailing_nl

namespace abc {
namespace test {

class binbuf_reader_read_line_utf16_mixed_no_trailing_nl :
   public binbuf_reader_read_line_base {
public:

   //! See binbuf_reader_read_line_base::title().
   virtual istr title() override {
      return istr(
         ABC_SL("abc::io::text::binbuf_reader – ")
         ABC_SL("reading line-by-line, UTF-16, CR/LF/CRLF mix, no trailing LF")
      );
   }

   //! See binbuf_reader_read_line_base::get_test_data().
   virtual file_path get_test_data() override {
      return file_path(ABC_SL(
         "src/abaclade-test/io/text/data/utf16le+bom_mixed_no-trailing-nl.txt"
      ));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::binbuf_reader_read_line_utf16_mixed_no_trailing_nl)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::binbuf_reader_read_line_utf32_lf_no_trailing_nl

namespace abc {
namespace test {

class binbuf_reader_read_line_utf32_lf_no_trailing_nl :
   public binbuf_reader_read_line_base {
public:

   //! See binbuf_reader_read_line_base::title().
   virtual istr title() override {
      return istr(ABC_SL(
         "abc::io::text::binbuf_reader – reading line-by-line, UTF-32, LF, no trailing LF"
      ));
   }

   //! See binbuf_reader_read_line_base::get_test_data().
   virtual file_path get_test_data() override {
      return file_path(ABC_SL("src/abaclade-test/io/text/data/utf32le+bom_lf_no-trailing-nl.txt"));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::binbuf_reader_read_line_utf32_lf_no_trailing_nl)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::binbuf_reader_read_line_utf32_mixed_no_trailing_nl

namespace abc {
namespace test {

class binbuf_reader_read_line_utf32_mixed_no_trailing_nl :
   public binbuf_reader_read_line_base {
public:

   //! See binbuf_reader_read_line_base::title().
   virtual istr title() override {
      return istr(
         ABC_SL("abc::io::text::binbuf_reader – ")
         ABC_SL("reading line-by-line, UTF-32, CR/LF/CRLF mix, no trailing LF")
      );
   }

   //! See binbuf_reader_read_line_base::get_test_data().
   virtual file_path get_test_data() override {
      return file_path(ABC_SL(
         "src/abaclade-test/io/text/data/utf32le+bom_mixed_no-trailing-nl.txt"
      ));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::binbuf_reader_read_line_utf32_mixed_no_trailing_nl)


////////////////////////////////////////////////////////////////////////////////////////////////////

