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
// abc::test::bbr_readline_test_case

namespace abc {
namespace test {

template <class T>
class bbr_readline_test_case :
   public testing::test_case {
public:

   /*! Returns the path to the test data.

   return
      Relative path to the test data file.
   */
   istr get_test_data_file_name() const;

   //! See testing::test_case::run().
   virtual void run() override {
      ABC_TRACE_FUNC(this);

      T const * t(static_cast<T const *>(this));
      file_path fp(ABC_SL("src/abaclade-test/io/text/data/") + t->get_test_data_file_name());
      auto ptrIn(io::text::open_reader(fp));
      dmstr sLine;
      unsigned i(1);
      while (ptrIn->read_line(&sLine)) {
         ABC_TESTING_ASSERT_EQUAL(sLine.size(), i++);
      }
   }

   //! See testing::test_case::title().
   virtual istr title() override {
      ABC_TRACE_FUNC(this);

      T const * t(static_cast<T const *>(this));
      return ABC_SL("abc::io::text::binbuf_reader – reading line-by-line, ") + t->title_suffix();
   }


   /*! Returns the portion of test_case::title() specific to the test case.

   return
      Title suffix.
   */
   istr title_suffix() const;
};

} //namespace test
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::binbuf_reader_read_line_utf8_lf_no_trailing_nl

namespace abc {
namespace test {

class binbuf_reader_read_line_utf8_lf_no_trailing_nl :
   public bbr_readline_test_case<binbuf_reader_read_line_utf8_lf_no_trailing_nl> {
public:

   //! See bbr_readline_test_case::title_suffix().
   istr title_suffix() const {
      return istr(ABC_SL("UTF-8, LF, no trailing LF"));
   }

   //! See bbr_readline_test_case::get_test_data_file_name().
   istr get_test_data_file_name() const {
      return istr(ABC_SL("utf8_lf_no-trailing-nl.txt"));
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
   public bbr_readline_test_case<binbuf_reader_read_line_utf8_mixed_no_trailing_nl> {
public:

   //! See bbr_readline_test_case::title_suffix().
   istr title_suffix() const {
      return istr(ABC_SL("UTF-8, CR/LF/CRLF mix, no trailing LF"));
   }

   //! See bbr_readline_test_case::get_test_data_file_name().
   istr get_test_data_file_name() const {
      return istr(ABC_SL("utf8_mixed_no-trailing-nl.txt"));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::binbuf_reader_read_line_utf8_mixed_no_trailing_nl)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::binbuf_reader_read_line_utf16be_lf_no_trailing_nl

namespace abc {
namespace test {

class binbuf_reader_read_line_utf16be_lf_no_trailing_nl :
   public bbr_readline_test_case<binbuf_reader_read_line_utf16be_lf_no_trailing_nl> {
public:

   //! See bbr_readline_test_case::title_suffix().
   istr title_suffix() const {
      return istr(ABC_SL("UTF-16BE, LF mix, no trailing LF"));
   }

   //! See bbr_readline_test_case::get_test_data_file_name().
   istr get_test_data_file_name() const {
      return istr(ABC_SL("utf16be+bom_lf_no-trailing-nl.txt"));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::binbuf_reader_read_line_utf16be_lf_no_trailing_nl)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::binbuf_reader_read_line_utf16le_lf_no_trailing_nl

namespace abc {
namespace test {

class binbuf_reader_read_line_utf16le_lf_no_trailing_nl :
   public bbr_readline_test_case<binbuf_reader_read_line_utf16le_lf_no_trailing_nl> {
public:

   //! See bbr_readline_test_case::title_suffix().
   istr title_suffix() const {
      return istr(ABC_SL("UTF-16LE, LF mix, no trailing LF"));
   }

   //! See bbr_readline_test_case::get_test_data_file_name().
   istr get_test_data_file_name() const {
      return istr(ABC_SL("utf16le+bom_lf_no-trailing-nl.txt"));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::binbuf_reader_read_line_utf16le_lf_no_trailing_nl)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::binbuf_reader_read_line_utf16le_mixed_no_trailing_nl

namespace abc {
namespace test {

class binbuf_reader_read_line_utf16le_mixed_no_trailing_nl :
   public bbr_readline_test_case<binbuf_reader_read_line_utf16le_mixed_no_trailing_nl> {
public:

   //! See bbr_readline_test_case::title_suffix().
   istr title_suffix() const {
      return istr(ABC_SL("UTF-16LE, CR/LF/CRLF mix, no trailing LF"));
   }

   //! See bbr_readline_test_case::get_test_data_file_name().
   istr get_test_data_file_name() const {
      return istr(ABC_SL("utf16le+bom_mixed_no-trailing-nl.txt"));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::binbuf_reader_read_line_utf16le_mixed_no_trailing_nl)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::binbuf_reader_read_line_utf32le_lf_no_trailing_nl

namespace abc {
namespace test {

class binbuf_reader_read_line_utf32le_lf_no_trailing_nl :
   public bbr_readline_test_case<binbuf_reader_read_line_utf32le_lf_no_trailing_nl> {
public:

   //! See bbr_readline_test_case::title_suffix().
   istr title_suffix() const {
      return istr(ABC_SL("UTF-32LE, LF, no trailing LF"));
   }

   //! See bbr_readline_test_case::get_test_data_file_name().
   istr get_test_data_file_name() const {
      return istr(ABC_SL("utf32le+bom_lf_no-trailing-nl.txt"));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::binbuf_reader_read_line_utf32le_lf_no_trailing_nl)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::binbuf_reader_read_line_utf32le_mixed_no_trailing_nl

namespace abc {
namespace test {

class binbuf_reader_read_line_utf32le_mixed_no_trailing_nl :
   public bbr_readline_test_case<binbuf_reader_read_line_utf32le_mixed_no_trailing_nl> {
public:

   //! See bbr_readline_test_case::title_suffix().
   istr title_suffix() const {
      return istr(ABC_SL("UTF-32LE, CR/LF/CRLF mix, no trailing LF"));
   }

   //! See bbr_readline_test_case::get_test_data_file_name().
   istr get_test_data_file_name() const {
      return istr(ABC_SL("utf32le+bom_mixed_no-trailing-nl.txt"));
   }
};

} //namespace test
} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::binbuf_reader_read_line_utf32le_mixed_no_trailing_nl)


////////////////////////////////////////////////////////////////////////////////////////////////////

