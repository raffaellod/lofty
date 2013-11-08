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

#include <abc/testing/unit.hxx>
#include <abc/trace.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::exception_polymorphism

namespace abc {

namespace test {

class exception_polymorphism :
	public testing::test_case {

	typedef environment_error    derived1_error;
	typedef io_error             derived2_error;
	typedef file_not_found_error derived3_error;

public:

	/** See testing::test_case::title().
	*/
	virtual istr title() {
		return istr(SL("abc::exception - polymorphism"));
	}


	/** See testing::test_case::run().
	*/
	virtual void run() {
		ABC_TRACE_FN((this));

		bool bCaughtCorrect;
		istr sResult;

		bCaughtCorrect = false;
		try {
			throw_exception();
			sResult = SL("threw abc::exception, but exception not thrown");
		} catch (derived3_error const &) {
			sResult = SL("threw abc::exception, but caught derived3_error");
		} catch (derived2_error const &) {
			sResult = SL("threw abc::exception, but caught derived2_error");
		} catch (derived1_error const &) {
			sResult = SL("threw abc::exception, but caught derived1_error");
		} catch (generic_error const &) {
			sResult = SL("threw abc::exception, but caught abc::generic_error");
		} catch (exception const &) {
			sResult = SL("threw and caught abc::exception");
			bCaughtCorrect = true;
		} catch (std::exception const &) {
			sResult = SL("threw abc::exception, but caught std::exception");
		} catch (...) {
			sResult = SL("threw abc::exception, but caught unknown exception");
		}
		expect(bCaughtCorrect, sResult);

		bCaughtCorrect = false;
		try {
			throw_generic_error();
			sResult = SL("threw abc::generic_error, but exception not thrown");
		} catch (derived3_error const &) {
			sResult = SL("threw abc::generic_error, but caught derived3_error");
		} catch (derived2_error const &) {
			sResult = SL("threw abc::generic_error, but caught derived2_error");
		} catch (derived1_error const &) {
			sResult = SL("threw abc::generic_error, but caught derived1_error");
		} catch (generic_error const &) {
			sResult = SL("threw and caught abc::generic_error");
			bCaughtCorrect = true;
		} catch (exception const &) {
			sResult = SL("threw abc::generic_error, but caught abc::exception");
		} catch (std::exception const &) {
			sResult = SL("threw abc::generic_error, but caught std::exception");
		} catch (...) {
			sResult = SL("threw abc::generic_error, but caught unknown exception");
		}
		expect(bCaughtCorrect, sResult);

		bCaughtCorrect = false;
		try {
			throw_derived1_error();
			sResult = SL("threw derived1_error, but exception not thrown");
		} catch (derived3_error const &) {
			sResult = SL("threw derived1_error, but caught derived3_error");
		} catch (derived2_error const &) {
			sResult = SL("threw derived1_error, but caught derived2_error");
		} catch (derived1_error const &) {
			sResult = SL("threw and caught derived1_error");
			bCaughtCorrect = true;
		} catch (generic_error const &) {
			sResult = SL("threw derived1_error, but caught abc::generic_error");
		} catch (exception const &) {
			sResult = SL("threw derived1_error, but caught abc::exception");
		} catch (std::exception const &) {
			sResult = SL("threw derived1_error, but caught std::exception");
		} catch (...) {
			sResult = SL("threw derived1_error, but caught unknown exception");
		}
		expect(bCaughtCorrect, sResult);

		bCaughtCorrect = false;
		try {
			throw_derived2_error();
			sResult = SL("threw derived2_error, but exception not thrown");
		} catch (derived3_error const &) {
			sResult = SL("threw derived2_error, but caught derived3_error");
		} catch (derived2_error const &) {
			sResult = SL("threw and caught derived2_error");
			bCaughtCorrect = true;
		} catch (derived1_error const &) {
			sResult = SL("threw derived2_error, but caught derived1_error");
		} catch (generic_error const &) {
			sResult = SL("threw derived2_error, but caught abc::generic_error");
		} catch (exception const &) {
			sResult = SL("threw derived2_error, but caught abc::exception");
		} catch (std::exception const &) {
			sResult = SL("threw derived2_error, but caught std::exception");
		} catch (...) {
			sResult = SL("threw derived2_error, but caught unknown exception");
		}
		expect(bCaughtCorrect, sResult);

		bCaughtCorrect = false;
		try {
			throw_derived3_error(2351);
			sResult = SL("threw derived3_error, but exception not thrown");
		} catch (derived3_error const &) {
			sResult = SL("threw and caught derived3_error");
			bCaughtCorrect = true;
		} catch (derived2_error const &) {
			sResult = SL("threw derived3_error, but caught derived2_error");
		} catch (derived1_error const &) {
			sResult = SL("threw derived3_error, but caught derived1_error");
		} catch (generic_error const &) {
			sResult = SL("threw derived3_error, but caught abc::generic_error");
		} catch (exception const &) {
			sResult = SL("threw derived3_error, but caught abc::exception");
		} catch (std::exception const &) {
			sResult = SL("threw derived3_error, but caught std::exception");
		} catch (...) {
			sResult = SL("threw derived3_error, but caught unknown exception");
		}
		expect(bCaughtCorrect, sResult);
	}


	void throw_exception() {
		ABC_TRACE_FN((this));

		ABC_THROW(exception, ());
	}


	void throw_generic_error() {
		ABC_TRACE_FN((this));

		ABC_THROW(generic_error, ());
	}


	void throw_derived1_error() {
		ABC_TRACE_FN((this));

		ABC_THROW(derived1_error, ());
	}


	void throw_derived2_error() {
		ABC_TRACE_FN((this));

		ABC_THROW(derived2_error, ());
	}


	void throw_derived3_error(int i) {
		ABC_TRACE_FN((this, i));

		ABC_THROW(derived3_error, ());
	}
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::exception_polymorphism)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::exception_from_os_hard_error

namespace abc {

namespace test {

class exception_from_os_hard_error :
	public testing::test_case {
public:

	/** See testing::test_case::title().
	*/
	virtual istr title() {
		return istr(SL("abc::exception - conversion of hard OS errors into C++ exceptions"));
	}


	/** See testing::test_case::run().
	*/
	virtual void run() {
		ABC_TRACE_FN((this));

		bool bCaughtCorrect;
		istr sResult;

		bCaughtCorrect = false;
		try {
			int * p(NULL);
			*p = 1;
			sResult = SL("dereferenced NULL, but no exception thrown");
		} catch (null_pointer_error const &) {
			sResult = SL("dereferenced NULL and caught abc::null_pointer_error");
			bCaughtCorrect = true;
		} catch (memory_address_error const &) {
			sResult = SL("dereferenced NULL, but caught abc::memory_address_error");
		} catch (...) {
			sResult = SL("dereferenced NULL, but caught unknown exception");
		}
		expect(bCaughtCorrect, sResult);

		bCaughtCorrect = false;
		try {
			int * p(NULL);
			// Under POSIX, this also counts as second test for SIGSEGV, checking that the handler is
			// still in place after its first activation above.
			*++p = 1;
			sResult = SL("dereferenced invalid pointer, but no exception thrown");
		} catch (null_pointer_error const &) {
			sResult = SL("dereferenced invalid pointer, but caught abc::null_pointer_error");
		} catch (memory_address_error const &) {
			sResult = SL("dereferenced invalid pointer and caught abc::memory_address_error");
			bCaughtCorrect = true;
		} catch (...) {
			sResult = SL("dereferenced invalid pointer, but caught unknown exception");
		}
		expect(bCaughtCorrect, sResult);

		// Enable alignment checking if the architecture supports it.
#ifdef __GNUC__
	#if defined(__i386__)
		__asm__(
			"pushf\n"
			"orl $0x00040000,(%esp)\n"
			"popf"
		);
		#define ABC_ALIGN_CHECK
	#elif defined(__x86_64__)
		__asm__(
			"pushf\n"
			"orl $0x0000000000040000,(%rsp)\n"
			"popf"
		);
		#define ABC_ALIGN_CHECK
	#endif
#endif

#ifdef ABC_ALIGN_CHECK
		bCaughtCorrect = false;
		try {
			// Create an int (with another one following it) and a pointer to it.
			int i[2];
			void * p(&i[0]);
			// Misalign the pointer, partly entering the second int.
			p = static_cast<int8_t *>(p) + 1;
			*static_cast<int *>(p) = 1;
			sResult = SL("unaligned memory access, but no exception thrown");
		} catch (memory_access_error const &) {
			sResult = SL("unaligned memory access and caught abc::memory_access_error");
			bCaughtCorrect = true;
		} catch (memory_address_error const &) {
			sResult = SL("unaligned memory access, but caught abc::memory_address_error");
		} catch (...) {
			sResult = SL("unaligned memory access, but caught unknown exception");
		}
		expect(bCaughtCorrect, sResult);

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
#endif //ifdef ABC_ALIGN_CHECK

		bCaughtCorrect = false;
		try {
			// Non-obvious division by zero that can’t be detected at compile time: sResult always has
			// a NUL terminator at its end, so use that as a zero divider.
			int iQuot(1 / int(sResult[intptr_t(sResult.size())]));
			// The call to istr::format() makes use of the result of the quotient, so it shouldn’t be
			// optimized away.
			istr(SL("{}")).format(iQuot);
			sResult = SL("divided by zero, but no exception thrown");
		} catch (division_by_zero_error const &) {
			sResult = SL("divided by zero and caught abc::division_by_zero_error");
			bCaughtCorrect = true;
		} catch (arithmetic_error const &) {
			sResult = SL("divided by zero, but caught abc::arithmetic_error");
		} catch (...) {
			sResult = SL("divided by zero, but caught unknown exception");
		}
		expect(bCaughtCorrect, sResult);
	}
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::exception_from_os_hard_error)


////////////////////////////////////////////////////////////////////////////////////////////////////

