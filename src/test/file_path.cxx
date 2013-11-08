/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

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
#include <abc/trace.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::file_path_normalization

namespace abc {

namespace test {

class file_path_normalization :
	public testing::test_case {
public:

	/** See testing::test_case::title().
	*/
	virtual istr title() {
		return istr(SL("abc::file_path - normalizations"));
	}


	/** See testing::test_case::run().
	*/
	virtual void run() {
		ABC_TRACE_FN((this));

		file_path fp(file_path::current_dir());

		// These should be normalized out.
		ABC_TESTING_ASSERT_EQUAL(fp, fp / SL(""));
		ABC_TESTING_ASSERT_EQUAL(fp, fp / SL("/"));
		ABC_TESTING_ASSERT_EQUAL(fp, fp / SL("//"));
		ABC_TESTING_ASSERT_EQUAL(fp, fp / SL("."));
		ABC_TESTING_ASSERT_EQUAL(fp, fp / SL("/."));
		ABC_TESTING_ASSERT_EQUAL(fp, fp / SL("./"));
		ABC_TESTING_ASSERT_EQUAL(fp, fp / SL("/./"));
		ABC_TESTING_ASSERT_EQUAL(fp, fp / SL("./."));

		// These should NOT be normalized: three dots are just another regular path component.
		ABC_TESTING_ASSERT_NOT_EQUAL(fp, fp / SL("..."));
		ABC_TESTING_ASSERT_NOT_EQUAL(fp, fp / SL("/..."));
		ABC_TESTING_ASSERT_NOT_EQUAL(fp, fp / SL(".../"));
		ABC_TESTING_ASSERT_NOT_EQUAL(fp, fp / SL("/.../"));

		// Now with one additional trailing component.
		ABC_TESTING_ASSERT_EQUAL(fp / SL("test"), fp / SL("/test"));
		ABC_TESTING_ASSERT_EQUAL(fp / SL("test"), fp / SL("//test"));
		ABC_TESTING_ASSERT_EQUAL(fp / SL("test"), fp / SL("./test"));
		ABC_TESTING_ASSERT_EQUAL(fp / SL("test"), fp / SL("/./test"));
		ABC_TESTING_ASSERT_EQUAL(fp / SL("test"), fp / SL("././test"));

		// Verify that ".." works.
		ABC_TESTING_ASSERT_EQUAL(fp / SL("a/.."), fp);
		ABC_TESTING_ASSERT_EQUAL(fp / SL("a/../b"), fp / SL("b"));
		ABC_TESTING_ASSERT_EQUAL(fp / SL("a/../b/.."), fp);
		ABC_TESTING_ASSERT_EQUAL(fp / SL("a/b/../.."), fp);
		ABC_TESTING_ASSERT_EQUAL(fp / SL("a/b/../c"), fp / SL("a/c"));
		ABC_TESTING_ASSERT_EQUAL(fp / SL("a/../b/../c"), fp / SL("c"));
		ABC_TESTING_ASSERT_EQUAL(fp / SL("a/b/../../c"), fp / SL("c"));
	}
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::file_path_normalization)


////////////////////////////////////////////////////////////////////////////////////////////////////

