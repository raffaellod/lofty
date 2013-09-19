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
#include <abc/mock/iostream.hxx>
#include <abc/trace.hxx>
using namespace abc;



class test_module :
	public module_impl<test_module> {
public:

	int main(vector<cstring const> const & vsArgs) {
		abc_trace_fn((/*vsArgs*/));

		UNUSED_ARG(vsArgs);

		mock::ostream mos;

		// Syntax errors.

		try {
			mos.reset();
			mos.print(SL("{"));
			return 10;
		} catch (syntax_error const &) {
		}

		try {
			mos.reset();
			mos.print(SL("{{{"));
			return 11;
		} catch (syntax_error const &) {
		}

		try {
			mos.reset();
			mos.print(SL("}"));
			return 12;
		} catch (syntax_error const &) {
		}

		try {
			mos.reset();
			mos.print(SL("}}}"));
			return 13;
		} catch (syntax_error const &) {
		}


		// No replacements.

		mos.reset();
		mos.print(SL(""));
		if (!mos.contents_equal(SL(""))) {
			return 20;
		}

		mos.reset();
		mos.print(SL("x"));
		if (!mos.contents_equal(SL("x"))) {
			return 21;
		}

		mos.reset();
		mos.print(SL("x"), SL("a"));
		if (!mos.contents_equal(SL("x"))) {
			return 22;
		}

		mos.reset();
		mos.print(SL("{{"));
		if (!mos.contents_equal(SL("{"))) {
			return 23;
		}

		mos.reset();
		mos.print(SL("}}"));
		if (!mos.contents_equal(SL("}"))) {
			return 24;
		}

		mos.reset();
		mos.print(SL("{{}}"));
		if (!mos.contents_equal(SL("{}"))) {
			return 25;
		}

		// Single string replacement, deducted argument index.

		mos.reset();
		mos.print(SL("{}"), SL("a"));
		if (!mos.contents_equal(SL("a"))) {
			return 30;
		}

		mos.reset();
		mos.print(SL("x{}"), SL("a"));
		if (!mos.contents_equal(SL("xa"))) {
			return 31;
		}

		mos.reset();
		mos.print(SL("{}x"), SL("a"));
		if (!mos.contents_equal(SL("ax"))) {
			return 32;
		}

		mos.reset();
		mos.print(SL("x{}x"), SL("a"));
		if (!mos.contents_equal(SL("xax"))) {
			return 33;
		}

		mos.reset();
		mos.print(SL("{{{}}}"), SL("a"));
		if (!mos.contents_equal(SL("{a}"))) {
			return 34;
		}


		// Single string replacement, explicit index.

		mos.reset();
		mos.print(SL("{0}"), SL("a"));
		if (!mos.contents_equal(SL("a"))) {
			return 40;
		}

		mos.reset();
		mos.print(SL("x{0}"), SL("a"));
		if (!mos.contents_equal(SL("xa"))) {
			return 41;
		}

		mos.reset();
		mos.print(SL("{0}x"), SL("a"));
		if (!mos.contents_equal(SL("ax"))) {
			return 42;
		}

		mos.reset();
		mos.print(SL("x{0}x"), SL("a"));
		if (!mos.contents_equal(SL("xax"))) {
			return 43;
		}


		// Single string replacement, referenced twice.

		mos.reset();
		mos.print(SL("{0}{0}"), SL("a"));
		if (!mos.contents_equal(SL("aa"))) {
			return 50;
		}

		mos.reset();
		mos.print(SL("{0}x{0}"), SL("a"));
		if (!mos.contents_equal(SL("axa"))) {
			return 51;
		}

		mos.reset();
		mos.print(SL("x{0}x{0}"), SL("a"));
		if (!mos.contents_equal(SL("xaxa"))) {
			return 52;
		}

		mos.reset();
		mos.print(SL("{0}x{0}x"), SL("a"));
		if (!mos.contents_equal(SL("axax"))) {
			return 53;
		}

		mos.reset();
		mos.print(SL("x{0}x{0}x"), SL("a"));
		if (!mos.contents_equal(SL("xaxax"))) {
			return 54;
		}


		// Two string replacements, various ways of reference.

		mos.reset();
		mos.print(SL("{}{}"), SL("a"), SL("b"));
		if (!mos.contents_equal(SL("ab"))) {
			return 60;
		}

		mos.reset();
		mos.print(SL("{0}{1}"), SL("a"), SL("b"));
		if (!mos.contents_equal(SL("ab"))) {
			return 61;
		}

		mos.reset();
		mos.print(SL("{1}{0}"), SL("a"), SL("b"));
		if (!mos.contents_equal(SL("ba"))) {
			return 62;
		}

		mos.reset();
		mos.print(SL("{1}{1}"), SL("a"), SL("b"));
		if (!mos.contents_equal(SL("bb"))) {
			return 63;
		}


		// Single integer replacement, various ways of reference, various format options.

		mos.reset();
		mos.print(SL("{}"), 34);
		if (!mos.contents_equal(SL("34"))) {
			return 70;
		}

		mos.reset();
		mos.print(SL("{:x}"), 34);
		if (!mos.contents_equal(SL("22"))) {
			return 71;
		}

		mos.reset();
		mos.print(SL("{:#x}"), 34);
		if (!mos.contents_equal(SL("0x22"))) {
			return 72;
		}

		return EXIT_SUCCESS;
	}
};

ABC_DECLARE_MODULE_IMPL_CLASS(test_module)

