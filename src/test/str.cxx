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
// abc::test::str_unit_base

namespace abc {

namespace test {

class str_unit_base :
	public testing::unit {
protected:

	/** Initializes the private data members used by check_str().

	s
		String to initialize with.
	*/
	void init_check_str(istr const & s) {
		abc_trace_fn((s));

		m_psCheck = &s;
		m_pchCheck = s.data();
	}


	/** Checks if a string’s item array has been reallocated, and verifies its character count.

	TODO: comment signature.
	*/
	bool check_str(bool bPtrChanged, size_t cch, size_t cchCapacity = 0) {
		abc_trace_fn((bPtrChanged, cch, cchCapacity));

		// Check if the item array has changed in accordance to the expectation.
		if ((m_pchCheck != m_psCheck->data()) != bPtrChanged) {
			return false;
		}
		// Check if the character count matches the expectation.
		if (m_psCheck->size() != cch) {
			return false;
		}
		// Check if the capacity matches the expectation.
		if (m_psCheck->capacity() != cchCapacity) {
			return false;
		}
		// Update the item array pointer for the next call.
		m_pchCheck = m_psCheck->data();
		return true;
	}


private:

	/** Pointer to the local string variable to be checked. */
	istr const * m_psCheck;
	/** Pointer to m_psCheck’s item array. */
	char_t const * m_pchCheck;
};

} //namespace test

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str_basic

namespace abc {

namespace test {

class str_basic :
	public str_unit_base {
public:

	/** See str_unit_base::title().
	*/
	virtual istr title() {
		return istr(SL("str classes - basic operations"));
	}


	/** See str_unit_base::run().
	*/
	virtual void run() {
		abc_trace_fn(());

		dmstr s;
		init_check_str(s);

		s += SL("a");
		// true: operator+= must have created an item array (there was none).
		ABC_TESTING_EXPECT(check_str(true, 1, 7));
		ABC_TESTING_ASSERT(s[0] == CL('a'));

		s = s + CL('b') + s;
		// true: a new string is created by operator+, which replaces s by operator=.
		ABC_TESTING_EXPECT(check_str(true, 3, 7));
		ABC_TESTING_EXPECT(s == SL("aba"));

		s = s.substr(1, 3);
		// true: s got replaced by operator=.
		ABC_TESTING_EXPECT(check_str(true, 2, 7));
		ABC_TESTING_EXPECT(s == SL("ba"));

		s += CL('c');
		// false: there should’ve been enough space for 'c'.
		ABC_TESTING_EXPECT(check_str(false, 3, 7));
		ABC_TESTING_EXPECT(s == SL("bac"));

		s = s.substr(0, -1);
		// true: s got replaced by operator=.
		ABC_TESTING_EXPECT(check_str(true, 2, 7));
		ABC_TESTING_EXPECT(s[0] == CL('b') && s[1] == CL('a'));

		s += s;
		// false: there should’ve been enough space for “baba”.
		ABC_TESTING_EXPECT(check_str(false, 4, 7));
		ABC_TESTING_EXPECT(s[0] == CL('b') && s[1] == CL('a') && s[2] == CL('b') && s[3] == CL('a'));

		s = s.substr(-3, -2);
		// true: s got replaced by operator=.
		ABC_TESTING_EXPECT(check_str(true, 1, 7));
		ABC_TESTING_EXPECT(s[0] == CL('a'));

		s = dmstr(SL("ab")) + CL('c');
		// true: s got replaced by operator=.
		ABC_TESTING_EXPECT(check_str(true, 3, 7));
		ABC_TESTING_EXPECT(s[0] == CL('a') && s[1] == CL('b') && s[2] == CL('c'));

		s += CL('d');
		// false: there should’ve been enough space for “abcd”.
		ABC_TESTING_EXPECT(check_str(false, 4, 7));
		ABC_TESTING_EXPECT(s[0] == CL('a') && s[1] == CL('b') && s[2] == CL('c') && s[3] == CL('d'));

		s += SL("efghijklmnopqrstuvwxyz");
		// false: while this will need to reallocate, the heap should be able to just resize the
		// allocated block, so the pointer won’t change.
		// TODO: FIXME: can result in sporadic failures depending on heap reallocation strategy.
		ABC_TESTING_EXPECT(check_str(false, 26, 55));
		ABC_TESTING_EXPECT(s == SL("abcdefghijklmnopqrstuvwxyz"));

		s = SL("a\0b");
		s += SL("\0c");
		// false: there should have been plenty of storage allocated.
		ABC_TESTING_EXPECT(check_str(false, 5, 55));
		ABC_TESTING_EXPECT(s == SL("a\0b\0c") && SL("a\0b\0c") == s);
	}
};

} //namespace test

} //namespace abc

ABC_TESTING_UNIT_REGISTER(abc::test::str_basic)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str8_substr_ascii

namespace abc {

namespace test {

class str8_substr_ascii :
	public testing::unit {
public:

	/** See abc::testing::unit::title().
	*/
	virtual istr title() {
		return istr(SL("str classes - ASCII character and substring search - UTF-8 strings"));
	}


	/** See abc::testing::unit::run().
	*/
	virtual void run() {
		abc_trace_fn(());

		// ASCII character and substring search.
		// The string “acabaabca” has the following properties:
		// •  misleading start for “ab” at index 0 (it’s “ac” instead) and for “abc” at index 2 (it’s
		//    “aba” instead), to catch incorrect skip-last comparisons;
		// •  first and last characters match 'a', but other inner ones do too;
		// •  would match “abcd” were it not for the last character;
		// •  matches the self-repeating “abaabc” but not the (also self-repeating) “abaabcd”.
#ifdef U8SL
		istr8 const s8(U8SL("acabaabca"));
		istr8::const_iterator

		it = s8.find(U32CL('b'));
		ABC_TESTING_EXPECT(it == s8.cbegin() + 3);

		it = s8.find(U8SL("ab"));
		ABC_TESTING_EXPECT(it == s8.cbegin() + 2);

		it = s8.find(U8SL("abca"));
		ABC_TESTING_EXPECT(it == s8.cbegin() + 5);

		it = s8.find(U8SL("abcd"));
		ABC_TESTING_EXPECT(it == s8.cend());

		it = s8.find(U8SL("abaabc"));
		ABC_TESTING_EXPECT(it == s8.cbegin() + 2);

		it = s8.find(U8SL("abaabcd"));
		ABC_TESTING_EXPECT(it == s8.cend());

		it = s8.find_last(U32CL('b'));
		ABC_TESTING_EXPECT(it == s8.cend() - 3);

#if 0
		it = s8.find_last(U8SL("ab"));
		ABC_TESTING_EXPECT(it == s8.cend() - 4);

		it = s8.find_last(U8SL("ac"));
		ABC_TESTING_EXPECT(it == s8.cend() - 9);

		it = s8.find_last(U8SL("ca"));
		ABC_TESTING_EXPECT(it == s8.cend() - 2);
#endif
#endif
	}
};

} //namespace test

} //namespace abc

ABC_TESTING_UNIT_REGISTER(abc::test::str8_substr_ascii)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str16_substr_ascii

namespace abc {

namespace test {

class str16_substr_ascii :
	public testing::unit {
public:

	/** See abc::testing::unit::title().
	*/
	virtual istr title() {
		return istr(SL("str classes - ASCII character and substring search - UTF-16 strings"));
	}


	/** See abc::testing::unit::run().
	*/
	virtual void run() {
		abc_trace_fn(());

		// ASCII character and substring search.
		// The string “acabaabca” has the following properties:
		// •  misleading start for “ab” at index 0 (it’s “ac” instead) and for “abc” at index 2 (it’s
		//    “aba” instead), to catch incorrect skip-last comparisons;
		// •  first and last characters match 'a', but other inner ones do too;
		// •  would match “abcd” were it not for the last character;
		// •  matches the self-repeating “abaabc” but not the (also self-repeating) “abaabcd”.
#ifdef U16SL
		istr16 const s16(U16SL("acabaabca"));
		istr16::const_iterator it;

		it = s16.find(U32CL('b'));
		ABC_TESTING_EXPECT(it == s16.cbegin() + 3);

		it = s16.find(U16SL("ab"));
		ABC_TESTING_EXPECT(it == s16.cbegin() + 2);

		it = s16.find(U16SL("abca"));
		ABC_TESTING_EXPECT(it == s16.cbegin() + 5);

		it = s16.find(U16SL("abcd"));
		ABC_TESTING_EXPECT(it == s16.cend());

		it = s16.find(U16SL("abaabc"));
		ABC_TESTING_EXPECT(it == s16.cbegin() + 2);

		it = s16.find(U16SL("abaabcd"));
		ABC_TESTING_EXPECT(it == s16.cend());

		it = s16.find_last(U32CL('b'));
		ABC_TESTING_EXPECT(it == s16.cend() - 3);

#if 0
		it = s16.find_last(U16SL("ab"));
		ABC_TESTING_EXPECT(it == s16.cend() - 4);

		it = s16.find_last(U16SL("ac"));
		ABC_TESTING_EXPECT(it == s16.cend() - 9);

		it = s16.find_last(U16SL("ca"));
		ABC_TESTING_EXPECT(it == s16.cend() - 2);
#endif
#endif
	}
};

} //namespace test

} //namespace abc

ABC_TESTING_UNIT_REGISTER(abc::test::str16_substr_ascii)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str32_substr_ascii

namespace abc {

namespace test {

class str32_substr_ascii :
	public testing::unit {
public:

	/** See abc::testing::unit::title().
	*/
	virtual istr title() {
		return istr(SL("str classes - ASCII character and substring search - UTF-32 strings"));
	}


	/** See abc::testing::unit::run().
	*/
	virtual void run() {
		abc_trace_fn(());

		// ASCII character and substring search.
		// The string “acabaabca” has the following properties:
		// •  misleading start for “ab” at index 0 (it’s “ac” instead) and for “abc” at index 2 (it’s
		//    “aba” instead), to catch incorrect skip-last comparisons;
		// •  first and last characters match 'a', but other inner ones do too;
		// •  would match “abcd” were it not for the last character;
		// •  matches the self-repeating “abaabc” but not the (also self-repeating) “abaabcd”.
#ifdef U32SL
		istr32 const s32(U32SL("acabaabca"));
		istr32::const_iterator it;

		it = s32.find(U32CL('b'));
		ABC_TESTING_EXPECT(it == s32.cbegin() + 3);

		it = s32.find(U32SL("ab"));
		ABC_TESTING_EXPECT(it == s32.cbegin() + 2);

		it = s32.find(U32SL("abca"));
		ABC_TESTING_EXPECT(it == s32.cbegin() + 5);

		it = s32.find(U32SL("abcd"));
		ABC_TESTING_EXPECT(it == s32.cend());

		it = s32.find(U32SL("abaabc"));
		ABC_TESTING_EXPECT(it == s32.cbegin() + 2);

		it = s32.find(U32SL("abaabcd"));
		ABC_TESTING_EXPECT(it == s32.cend());

		it = s32.find_last(U32CL('b'));
		ABC_TESTING_EXPECT(it == s32.cend() - 3);

#if 0
		it = s32.find_last(U32SL("ab"));
		ABC_TESTING_EXPECT(it == s32.cend() - 4);

		it = s32.find_last(U32SL("ac"));
		ABC_TESTING_EXPECT(it == s32.cend() - 9);

		it = s32.find_last(U32SL("ca"));
		ABC_TESTING_EXPECT(it == s32.cend() - 2);
#endif
#endif
	}
};

} //namespace test

} //namespace abc

ABC_TESTING_UNIT_REGISTER(abc::test::str32_substr_ascii)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str8_substr_nonascii

namespace abc {

namespace test {

class str8_substr_nonascii :
	public testing::unit {
public:

	/** See abc::testing::unit::title().
	*/
	virtual istr title() {
		return istr(SL("str classes - non-ASCII character and substring search - UTF-8 strings"));
	}


	/** See abc::testing::unit::run().
	*/
	virtual void run() {
		abc_trace_fn(());

		// Non-ASCII character and substring search.
#ifdef U8SL
		istr8 const s8(U8SL("àßçàŒ"));
		istr8::const_iterator it;

		it = s8.find(U32CL('ß'));
		ABC_TESTING_EXPECT(it == s8.cbegin() + 2);

		it = s8.find(U8SL("àß"));
		ABC_TESTING_EXPECT(it == s8.cbegin() + 0);

		it = s8.find(U8SL("àŒ"));
		ABC_TESTING_EXPECT(it == s8.cbegin() + 6);

		it = s8.find(U8SL("àü"));
		ABC_TESTING_EXPECT(it == s8.cend());
#endif
	}
};

} //namespace test

} //namespace abc

ABC_TESTING_UNIT_REGISTER(abc::test::str8_substr_nonascii)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str16_substr_nonascii

namespace abc {

namespace test {

class str16_substr_nonascii :
	public testing::unit {
public:

	/** See abc::testing::unit::title().
	*/
	virtual istr title() {
		return istr(SL("str classes - non-ASCII character and substring search - UTF-16 strings"));
	}


	/** See abc::testing::unit::run().
	*/
	virtual void run() {
		abc_trace_fn(());

		// Non-ASCII character and substring search.
#ifdef U16SL
		istr16 const s16(U16SL("àßçàŒ"));
		istr16::const_iterator it;

		it = s16.find(U32CL('ß'));
		ABC_TESTING_EXPECT(it == s16.cbegin() + 1);

		it = s16.find(U16SL("àß"));
		ABC_TESTING_EXPECT(it == s16.cbegin() + 0);

		it = s16.find(U16SL("àŒ"));
		ABC_TESTING_EXPECT(it == s16.cbegin() + 3);

		it = s16.find(U16SL("àü"));
		ABC_TESTING_EXPECT(it == s16.cend());
#endif
	}
};

} //namespace test

} //namespace abc

ABC_TESTING_UNIT_REGISTER(abc::test::str16_substr_nonascii)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str32_substr_nonascii

namespace abc {

namespace test {

class str32_substr_nonascii :
	public testing::unit {
public:

	/** See abc::testing::unit::title().
	*/
	virtual istr title() {
		return istr(SL("str classes - non-ASCII character and substring search - UTF-32 strings"));
	}


	/** See abc::testing::unit::run().
	*/
	virtual void run() {
		abc_trace_fn(());

		// Non-ASCII character and substring search.
#ifdef U32SL
		istr32 const s32(U32SL("àßçàŒ"));
		istr32::const_iterator it;

		it = s32.find(U32CL('ß'));
		ABC_TESTING_EXPECT(it == s32.cbegin() + 1);

		it = s32.find(U32SL("àß"));
		ABC_TESTING_EXPECT(it == s32.cbegin() + 0);

		it = s32.find(U32SL("àŒ"));
		ABC_TESTING_EXPECT(it == s32.cbegin() + 3);

		it = s32.find(U32SL("àü"));
		ABC_TESTING_EXPECT(it == s32.cend());
#endif
	}
};

} //namespace test

} //namespace abc

ABC_TESTING_UNIT_REGISTER(abc::test::str32_substr_nonascii)


////////////////////////////////////////////////////////////////////////////////////////////////////

