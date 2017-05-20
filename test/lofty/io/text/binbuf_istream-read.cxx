/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/io/text.hxx>
#include <lofty/os/path.hxx>
#include <lofty/testing/test_case.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

class bbis_readline_test_case : public testing::test_case {
public:
   /*! Returns the path to the test data.

   @return
      Relative path to the test data file.
   */
   virtual str get_test_data_file_name() const = 0;

   //! See testing::test_case::run().
   virtual void run() override {
      LOFTY_TRACE_FUNC(this);

      os::path path(LOFTY_SL("test/lofty/io/text/data/") + get_test_data_file_name());
      auto istream(io::text::open_istream(path));
      // TODO: use enumerate().
      unsigned i = 1;
      LOFTY_FOR_EACH(auto & line, istream->lines()) {
         LOFTY_TESTING_ASSERT_EQUAL(line.size(), i++);
      }
   }

   //! See testing::test_case::title().
   virtual str title() override {
      LOFTY_TRACE_FUNC(this);

      return LOFTY_SL("lofty::io::text::binbuf_istream – reading line-by-line, ") + title_suffix();
   }

   /*! Returns the portion of test_case::title() specific to the test case.

   @return
      Title suffix.
   */
   virtual str title_suffix() const = 0;
};

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

class binbuf_istream_read_line_utf8_lf_no_trailing_nl : public bbis_readline_test_case {
public:
   //! See bbis_readline_test_case::title_suffix().
   virtual str title_suffix() const override {
      return str(LOFTY_SL("UTF-8, LF, no trailing LF"));
   }

   //! See bbis_readline_test_case::get_test_data_file_name().
   virtual str get_test_data_file_name() const override {
      return str(LOFTY_SL("utf8_lf_no-trailing-nl.txt"));
   }
};

}} //namespace lofty::test

LOFTY_TESTING_REGISTER_TEST_CASE(lofty::test::binbuf_istream_read_line_utf8_lf_no_trailing_nl)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

class binbuf_istream_read_line_utf8_mixed_no_trailing_nl : public bbis_readline_test_case {
public:
   //! See bbis_readline_test_case::title_suffix().
   virtual str title_suffix() const override {
      return str(LOFTY_SL("UTF-8, CR/LF/CRLF mix, no trailing LF"));
   }

   //! See bbis_readline_test_case::get_test_data_file_name().
   virtual str get_test_data_file_name() const override {
      return str(LOFTY_SL("utf8_mixed_no-trailing-nl.txt"));
   }
};

}} //namespace lofty::test

LOFTY_TESTING_REGISTER_TEST_CASE(lofty::test::binbuf_istream_read_line_utf8_mixed_no_trailing_nl)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

class binbuf_istream_read_line_utf16be_lf_no_trailing_nl : public bbis_readline_test_case {
public:
   //! See bbis_readline_test_case::title_suffix().
   virtual str title_suffix() const override {
      return str(LOFTY_SL("UTF-16BE, LF mix, no trailing LF"));
   }

   //! See bbis_readline_test_case::get_test_data_file_name().
   virtual str get_test_data_file_name() const override {
      return str(LOFTY_SL("utf16be+bom_lf_no-trailing-nl.txt"));
   }
};

}} //namespace lofty::test

LOFTY_TESTING_REGISTER_TEST_CASE(lofty::test::binbuf_istream_read_line_utf16be_lf_no_trailing_nl)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

class binbuf_istream_read_line_utf16le_lf_no_trailing_nl : public bbis_readline_test_case {
public:
   //! See bbis_readline_test_case::title_suffix().
   virtual str title_suffix() const override {
      return str(LOFTY_SL("UTF-16LE, LF mix, no trailing LF"));
   }

   //! See bbis_readline_test_case::get_test_data_file_name().
   virtual str get_test_data_file_name() const override {
      return str(LOFTY_SL("utf16le+bom_lf_no-trailing-nl.txt"));
   }
};

}} //namespace lofty::test

LOFTY_TESTING_REGISTER_TEST_CASE(lofty::test::binbuf_istream_read_line_utf16le_lf_no_trailing_nl)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

class binbuf_istream_read_line_utf16le_mixed_no_trailing_nl : public bbis_readline_test_case {
public:
   //! See bbis_readline_test_case::title_suffix().
   virtual str title_suffix() const override {
      return str(LOFTY_SL("UTF-16LE, CR/LF/CRLF mix, no trailing LF"));
   }

   //! See bbis_readline_test_case::get_test_data_file_name().
   virtual str get_test_data_file_name() const override {
      return str(LOFTY_SL("utf16le+bom_mixed_no-trailing-nl.txt"));
   }
};

}} //namespace lofty::test

LOFTY_TESTING_REGISTER_TEST_CASE(lofty::test::binbuf_istream_read_line_utf16le_mixed_no_trailing_nl)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

class binbuf_istream_read_line_utf32le_lf_no_trailing_nl : public bbis_readline_test_case {
public:
   //! See bbis_readline_test_case::title_suffix().
   virtual str title_suffix() const override {
      return str(LOFTY_SL("UTF-32LE, LF, no trailing LF"));
   }

   //! See bbis_readline_test_case::get_test_data_file_name().
   virtual str get_test_data_file_name() const override {
      return str(LOFTY_SL("utf32le+bom_lf_no-trailing-nl.txt"));
   }
};

}} //namespace lofty::test

LOFTY_TESTING_REGISTER_TEST_CASE(lofty::test::binbuf_istream_read_line_utf32le_lf_no_trailing_nl)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

class binbuf_istream_read_line_utf32le_mixed_no_trailing_nl : public bbis_readline_test_case {
public:
   //! See bbis_readline_test_case::title_suffix().
   virtual str title_suffix() const override {
      return str(LOFTY_SL("UTF-32LE, CR/LF/CRLF mix, no trailing LF"));
   }

   //! See bbis_readline_test_case::get_test_data_file_name().
   virtual str get_test_data_file_name() const override {
      return str(LOFTY_SL("utf32le+bom_mixed_no-trailing-nl.txt"));
   }
};

}} //namespace lofty::test

LOFTY_TESTING_REGISTER_TEST_CASE(lofty::test::binbuf_istream_read_line_utf32le_mixed_no_trailing_nl)
