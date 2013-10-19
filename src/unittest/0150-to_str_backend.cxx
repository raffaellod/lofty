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



class test_app_module :
	public app_module_impl<test_app_module> {
public:

	int main(mvector<istr const> const & vsArgs) {
		abc_trace_fn((/*vsArgs*/));

		UNUSED_ARG(vsArgs);

		// Test zero, decimal base.
		if (!test_to_str_backend_output(0, SL(""), SL("0"))) {
			return 10;
		}
		if (!test_to_str_backend_output(0, SL(" 1"), SL(" 0"))) {
			return 11;
		}
		if (!test_to_str_backend_output(0, SL("01"), SL("0"))) {
			return 12;
		}
		if (!test_to_str_backend_output(0, SL(" 2"), SL(" 0"))) {
			return 13;
		}
		if (!test_to_str_backend_output(0, SL("02"), SL("00"))) {
			return 14;
		}

		// Test positive values, decimal base.
		if (!test_to_str_backend_output(1, SL(""), SL("1"))) {
			return 20;
		}
		if (!test_to_str_backend_output(1, SL(" 1"), SL(" 1"))) {
			return 21;
		}
		if (!test_to_str_backend_output(1, SL("01"), SL("1"))) {
			return 22;
		}
		if (!test_to_str_backend_output(1, SL(" 2"), SL(" 1"))) {
			return 23;
		}
		if (!test_to_str_backend_output(1, SL("02"), SL("01"))) {
			return 24;
		}

		// Test negative values, decimal base.
		if (!test_to_str_backend_output(-1, SL(""), SL("-1"))) {
			return 30;
		}
		if (!test_to_str_backend_output(-1, SL(" 1"), SL("-1"))) {
			return 31;
		}
		if (!test_to_str_backend_output(-1, SL("01"), SL("-1"))) {
			return 32;
		}
		if (!test_to_str_backend_output(-1, SL(" 2"), SL("-1"))) {
			return 33;
		}
		if (!test_to_str_backend_output(-1, SL("02"), SL("-1"))) {
			return 34;
		}
		if (!test_to_str_backend_output(-1, SL(" 3"), SL(" -1"))) {
			return 35;
		}
		if (!test_to_str_backend_output(-1, SL("03"), SL("-01"))) {
			return 36;
		}

		// Test zero, hexadecimal base.
		if (!test_to_str_backend_output(int8_t(0), SL("x"), SL("0"))) {
			return 40;
		}
		if (!test_to_str_backend_output(int8_t(0), SL(" 1x"), SL("0"))) {
			return 41;
		}
		if (!test_to_str_backend_output(int8_t(0), SL("01x"), SL("0"))) {
			return 42;
		}
		if (!test_to_str_backend_output(int8_t(0), SL(" 2x"), SL(" 0"))) {
			return 43;
		}
		if (!test_to_str_backend_output(int8_t(0), SL("02x"), SL("00"))) {
			return 44;
		}

		// Test positive values, hexadecimal base.
		if (!test_to_str_backend_output(int8_t(1), SL("x"), SL("1"))) {
			return 50;
		}
		if (!test_to_str_backend_output(int8_t(1), SL(" 1x"), SL("1"))) {
			return 51;
		}
		if (!test_to_str_backend_output(int8_t(1), SL("01x"), SL("1"))) {
			return 52;
		}
		if (!test_to_str_backend_output(int8_t(1), SL(" 2x"), SL(" 1"))) {
			return 53;
		}
		if (!test_to_str_backend_output(int8_t(1), SL("02x"), SL("01"))) {
			return 54;
		}

		// Test negative values, hexadecimal base.
		if (!test_to_str_backend_output(int8_t(-1), SL("x"), SL("ff"))) {
			return 60;
		}
		if (!test_to_str_backend_output(int8_t(-1), SL(" 1x"), SL("ff"))) {
			return 61;
		}
		if (!test_to_str_backend_output(int8_t(-1), SL("01x"), SL("ff"))) {
			return 62;
		}
		if (!test_to_str_backend_output(int8_t(-1), SL(" 2x"), SL("ff"))) {
			return 63;
		}
		if (!test_to_str_backend_output(int8_t(-1), SL("02x"), SL("ff"))) {
			return 64;
		}
		if (!test_to_str_backend_output(int8_t(-1), SL(" 3x"), SL(" ff"))) {
			return 65;
		}
		if (!test_to_str_backend_output(int8_t(-1), SL("03x"), SL("0ff"))) {
			return 66;
		}

		return EXIT_SUCCESS;
	}


	/** Writes the first argument using the second as format specification, and compares the result
	with the provided expectation, returning true if they match.

	TODO: comment signature.
	*/
	template <typename T, size_t t_cchFormatSpec, size_t t_cchExpected>
	static bool test_to_str_backend_output(
		T const & t,
		char_t const (& achFormatSpec)[t_cchFormatSpec],
		char_t const (& achExpected)[t_cchExpected]
	) {
		abc_trace_fn((t, achFormatSpec, achExpected));

		mock::ostream mos;
		to_str_backend<T> tsb(achFormatSpec);
		tsb.write(t, &mos);
		return mos.contents_equal(achExpected);
	}
};

ABC_MAIN_APP_MODULE(test_app_module)

