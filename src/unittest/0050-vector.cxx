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
#include <algorithm>



/** Class with a pointer.

The purpose is to have some dynamically-allocated memory that’s unique to any instance, to detect
whether a copy (possible in all forms) has been made; for the same purpose, it also counts the
number of copy-constructions or -assignments.
*/
class test_with_ptr {
public:

	test_with_ptr() :
		m_pi(new int(++m_iNext)) {
	}
	test_with_ptr(test_with_ptr const & twp) :
		m_pi(new int(*twp.m_pi)) {
		++m_iNext;
	}
	test_with_ptr(test_with_ptr && twp) :
		m_pi(std::move(twp.m_pi)) {
	}


	test_with_ptr & operator=(test_with_ptr const & twp) {
		*m_pi = *twp.m_pi;
		++m_iNext;
		return *this;
	}
	test_with_ptr & operator=(test_with_ptr && twp) {
		*m_pi = *twp.m_pi;
		return *this;
	}


	int const * get_ptr() const {
		return m_pi.get();
	}


	static int get_count() {
		return m_iNext;
	}


private:

	std::unique_ptr<int> m_pi;
	static int m_iNext;
};

bool operator==(test_with_ptr const &, test_with_ptr const &);

bool operator==(test_with_ptr const &, test_with_ptr const &) {
	return false;
}


int test_with_ptr::m_iNext = 0;



class test_app_module :
	public app_module_impl<test_app_module> {
public:

	int main(mvector<istr const> const & vsArgs) {
		ABC_TRACE_FN((/*vsArgs*/));

		ABC_UNUSED_ARG(vsArgs);
		int const * pi;

		// Simple manipulation tests.
		{
			dmvector<int> v;

			v.append(1);
			if (v.size() != 1 || v[0] != 1) {
				return 1;
			}

			v = v + v;
			v.insert(1, 2);
			if (v.size() != 3 || v[0] != 1 || v[1] != 2 || v[2] != 1) {
				return 2;
			}

			v = v.slice(1, 3);
			if (v.size() != 2 || v[0] != 2 || v[1] != 1) {
				return 3;
			}

			v.append(3);
			if (v.size() != 3 || v[0] != 2 || v[1] != 1 || v[2] != 3) {
				return 4;
			}

			auto i1(v.index_of(1));
			if (i1 != 1) {
				return 5;
			}

			auto i2(v.last_index_of(1));
			if (i2 != 1) {
				return 6;
			}

			auto it1(std::find(v.cbegin(), v.cend(), 1));
			if (it1 - v.cbegin() != 1) {
				return 7;
			}

			v.remove_at(it1);
			if (v.size() != 2 || v[0] != 2 || v[1] != 3) {
				return 8;
			}
		}

		// Try mix’n’matching vectors of different sizes, and check that vectors using static
		// descriptors only switch to dynamic descriptors if necessary.
		{
			dmvector<int> v0;
			pi = v0.data();
			v0.append(0);
			if (v0.data() == pi) {
				return 50;
			}

			smvector<int, 3> v1;
			pi = v1.data();
			v1.append(1);
			if (v1.data() == pi) {
				return 51;
			}
			pi = v1.data();
			v1.append(2);
			if (v1.data() != pi) {
				return 52;
			}

			smvector<int, 1> v2;
			pi = v2.data();
			v2.append(3);
			if (v2.data() == pi) {
				return 53;
			}

			pi = v0.data();
			v0 = v1 + v2;
			if (v0.data() == pi || v0.size() != 3 || v0[0] != 1 || v0[1] != 2 || v0[2] != 3) {
				return 54;
			}

			pi = v1.data();
			v1 = v2 + v0;
			if (
				v1.data() == pi || v1.size() != 4 ||
				v1[0] != 3 || v1[1] != 1 || v1[2] != 2 || v1[3] != 3
			) {
				return 55;
			}

			pi = v2.data();
			v2 = v0 + v1;
			if (
				v2.data() == pi || v2.size() != 7 ||
				v2[0] != 1 || v2[1] != 2 || v2[2] != 3 ||
				v2[3] != 3 || v2[4] != 1 || v2[5] != 2 || v2[6] != 3
			) {
				return 56;
			}
		}

		// Check that returning a vector with a dynamically allocated descriptor does not cause a new
		// descriptor to be allocated, nor copies the items.
		{
			dmvector<test_with_ptr> v(move_constr_test(&pi));
			if (v[0].get_ptr() != pi) {
				return 100;
			}
			if (test_with_ptr::get_count() != 1) {
				return 110 + test_with_ptr::get_count();
			}
			// Also check that add(T &&) doesn’t make extra copies.
			v.append(test_with_ptr());
			if (test_with_ptr::get_count() != 2) {
				return 120 + test_with_ptr::get_count();
			}
		}

		// Check that returning a vector with a dynamically allocated descriptor into a vector with a
		// statically allocated descriptor causes the items to be moved to the static descriptor.
		{
			smvector<test_with_ptr, 2> v(move_constr_test(&pi));
			if (v[0].get_ptr() != pi) {
				return 130;
			}
			if (test_with_ptr::get_count() != 3) {
				return 140 + test_with_ptr::get_count();
			}
		}

		return EXIT_SUCCESS;
	}


	/** Creates a local vector<test_with_ptr> that’s modified in place, and adds to it a temporary
	item (which should cause no item copies to be made) whose internal pointer is stored in *ppi; the
	vector is then returned (which, again, should cause no item copies to be made).

	TODO: comment signature.
	*/
	dmvector<test_with_ptr> move_constr_test(int const ** ppi) {
		// vector::vector();
		dmvector<test_with_ptr> v;
		// test_with_ptr::test_with_ptr();
		// vector::add(T && t);
		v.append(test_with_ptr());
		// vector::operator[]();
		// test_with_ptr::get_ptr();
		*ppi = v[0].get_ptr();
		// vector::vector(vector && v);
		return std::move(v);
		// vector::~vector();
	}
};

ABC_MAIN_APP_MODULE(test_app_module)

