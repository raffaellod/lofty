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


typedef environment_error derived1_error;
typedef io_error derived2_error;
typedef file_not_found_error derived3_error;


class test_module :
	public module_impl<test_module> {
public:

	int main(vector<istr const> const & vsArgs) {
		abc_trace_fn((/*vsArgs*/));

		UNUSED_ARG(vsArgs);

		// Verify that exception polymorphism works.
		{
			try {
				throw_exception();
				return 10;
			} catch (derived3_error const & e) {
				return 11;
			} catch (derived2_error const & e) {
				return 12;
			} catch (derived1_error const & e) {
				return 13;
			} catch (generic_error const & e) {
				return 14;
			} catch (exception const & e) {
				// Success.
			} catch (...) {
				return 15;
			}

			try {
				throw_generic_error();
				return 20;
			} catch (derived3_error const & e) {
				return 21;
			} catch (derived2_error const & e) {
				return 22;
			} catch (derived1_error const & e) {
				return 23;
			} catch (generic_error const & e) {
				if (e != os_error_mapping<generic_error>::mapped_error) {
					return 24;
				}
				// Success.
			} catch (exception const & e) {
				return 25;
			} catch (...) {
				return 26;
			}

			try {
				throw_derived1_error();
				return 30;
			} catch (derived3_error const & e) {
				return 31;
			} catch (derived2_error const & e) {
				return 32;
			} catch (derived1_error const & e) {
				if (e != os_error_mapping<derived1_error>::mapped_error) {
					return 33;
				}
				// Success.
			} catch (generic_error const & e) {
				return 34;
			} catch (exception const & e) {
				return 35;
			} catch (...) {
				return 36;
			}

			try {
				throw_derived2_error();
				return 40;
			} catch (derived3_error const & e) {
				return 41;
			} catch (derived2_error const & e) {
				if (e != os_error_mapping<derived2_error>::mapped_error) {
					return 42;
				}
				// Success.
			} catch (derived1_error const & e) {
				return 43;
			} catch (generic_error const & e) {
				return 44;
			} catch (exception const & e) {
				return 45;
			} catch (...) {
				return 46;
			}

			try {
				throw_derived3_error(2351);
				return 50;
			} catch (derived3_error const & e) {
				if (e != os_error_mapping<derived3_error>::mapped_error) {
					return 51;
				}
				// Success.
			} catch (derived2_error const & e) {
				return 52;
			} catch (derived1_error const & e) {
				return 53;
			} catch (generic_error const & e) {
				return 54;
			} catch (exception const & e) {
				return 55;
			} catch (...) {
				return 56;
			}
		}

//		throw_derived3_error(2351);

		// Verify that hard exceptions are caught and converted into C++ exceptions.

		try {
			int * p(NULL);
			*p = 1;
			return 80;
		} catch (null_pointer_error const &) {
			// Success.
		} catch (memory_address_error const &) {
			return 81;
		} catch (...) {
			return 82;
		}

		try {
			int * p(NULL);
			// Under POSIX, this also counts as second test for SIGSEGV, checking that the handler is
			// still in place after its first activation above.
			*++p = 1;
			return 90;
		} catch (null_pointer_error const &) {
			return 91;
		} catch (memory_address_error const &) {
			// Success.
		} catch (...) {
			return 92;
		}

		// Enable alignment checking if the architecture supports it.
#ifdef __GNUC__
	#if defined(__i386__)
		__asm__(
			"pushf\n"
			"orl $0x00040000,(%esp)\n"
			"popf"
		);
		#define ABC_ALIGN_CHECK 1
	#elif defined(__x86_64__)
		__asm__(
			"pushf\n"
			"orl $0x0000000000040000,(%rsp)\n"
			"popf"
		);
		#define ABC_ALIGN_CHECK 1
	#endif
#endif
#if ABC_ALIGN_CHECK
		try {
			// Create an int (with another one following it) and a pointer to it.
			int i[2];
			void * p(&i[0]);
			// Misalign the pointer, partly entering the second int.
			p = static_cast<int8_t *>(p) + 1;
			*static_cast<int *>(p) = 1;
			return 100;
		} catch (memory_access_error const &) {
			// Success.
		} catch (memory_address_error const &) {
			return 101;
		} catch (...) {
			return 102;
		}
		// Disable alignment checking back.
#ifdef __GNUC__
	#if defined(__i386__)
		__asm__(
			"pushf\n"
			"andl $0xfffbffff,(%esp)\n"
			"popf"
		);
	#elif defined(__x86_64__)
		__asm__(
			"pushf\n"
			"andl $0xfffffffffffbffff,(%rsp)\n"
			"popf"
		);
	#endif
#endif
#endif //if ABC_ALIGN_CHECK

		try {
			// Non-obvious division by zero: this program takes no arguments, so vsArgs.size() is
			// always 1. The conditional code makes use of the quotient, so it can’t be optimized away.
			if (1 / (vsArgs.get_size() - 1)) {
				return 110;
			} else {
				return 111;
			}
		} catch (division_by_zero_error const &) {
			// Success.
		} catch (arithmetic_error const &) {
			return 112;
		} catch (...) {
			return 113;
		}

		return EXIT_SUCCESS;
	}


	void throw_exception() {
		abc_trace_fn(());

		abc_throw(exception());
	}


	void throw_generic_error() {
		abc_trace_fn(());

		abc_throw(generic_error());
	}


	void throw_derived1_error() {
		abc_trace_fn(());

		abc_throw(derived1_error());
	}


	void throw_derived2_error() {
		abc_trace_fn(());

		abc_throw(derived2_error());
	}


	void throw_derived3_error(int i) {
		abc_trace_fn((i));

		abc_throw(derived3_error());
	}
};

ABC_DECLARE_MODULE_IMPL_CLASS(test_module)

