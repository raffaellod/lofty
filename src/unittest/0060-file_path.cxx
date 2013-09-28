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

#include <abc/module.hxx>
#include <abc/trace.hxx>
using namespace abc;


class test_module :
	public module_impl<test_module> {
public:

	int main(vector<istr const> const & vsArgs) {
		abc_trace_fn((/*vsArgs*/));

		UNUSED_ARG(vsArgs);

		// Basic operations.
		{
			file_path fp(file_path::current_dir());

			// These should be normalized out.
			if (fp != fp / SL("")) {
				return 10;
			}
			if (fp != fp / SL("/")) {
				return 11;
			}
			if (fp != fp / SL("//")) {
				return 12;
			}
			if (fp != fp / SL(".")) {
				return 13;
			}
			if (fp != fp / SL("/.")) {
				return 14;
			}
			if (fp != fp / SL("./")) {
				return 15;
			}
			if (fp != fp / SL("/./")) {
				return 16;
			}
			if (fp != fp / SL("./.")) {
				return 17;
			}

			// These should NOT be normalized: three dots are just another regular path component.
			if (fp == fp / SL("...")) {
				return 20;
			}
			if (fp == fp / SL("/...")) {
				return 21;
			}
			if (fp == fp / SL(".../")) {
				return 22;
			}
			if (fp == fp / SL("/.../")) {
				return 23;
			}

			// Now with one additional trailing component.
			if (fp / SL("test") != fp / SL("/test")) {
				return 30;
			}
			if (fp / SL("test") != fp / SL("//test")) {
				return 31;
			}
			if (fp / SL("test") != fp / SL("./test")) {
				return 32;
			}
			if (fp / SL("test") != fp / SL("/./test")) {
				return 33;
			}
			if (fp / SL("test") != fp / SL("././test")) {
				return 34;
			}

			// Verify that ".." works.
			if (fp / SL("a/..") != fp) {
				return 40;
			}
			if (fp / SL("a/../b") != fp / SL("b")) {
				return 41;
			}
			if (fp / SL("a/../b/..") != fp) {
				return 42;
			}
			if (fp / SL("a/b/../..") != fp) {
				return 43;
			}
			if (fp / SL("a/b/../c") != fp / SL("a/c")) {
				return 44;
			}
			if (fp / SL("a/../b/../c") != fp / SL("c")) {
				return 45;
			}
			if (fp / SL("a/b/../../c") != fp / SL("c")) {
				return 46;
			}
		}

		return EXIT_SUCCESS;
	}
};

ABC_DECLARE_MODULE_IMPL_CLASS(test_module)

