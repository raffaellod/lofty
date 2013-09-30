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
			dmstr s;
			// Initialize member variables for check_str().
			m_psCheck = &static_cast<istr const &>(s);
			m_pchCheck = s.data();

			s += SL("a");
			// true: operator+= must have created an item array (there was none).
			if (!check_str(true, 1, 7) || s[0] != 'a') {
				return 10;
			}

			s = s + 'b' + s;
			// true: a new string is created by operator+, which replaces s by operator=.
			if (!check_str(true, 3, 7) || s != SL("aba")) {
				return 11;
			}

			s = s.substr(1, 3);
			// true: s got replaced by operator=.
			if (!check_str(true, 2, 7) || s != SL("ba")) {
				return 12;
			}

			s += 'c';
			// false: there should’ve been enough space for 'c'.
			if (!check_str(false, 3, 7) || s != SL("bac")) {
				return 13;
			}

			s = s.substr(0, -1);
			// true: s got replaced by operator=.
			if (!check_str(true, 2, 7) || s[0] != 'b' || s[1] != 'a') {
				return 14;
			}

			s += s;
			// false: there should’ve been enough space for “baba”.
			if (!check_str(false, 4, 7) || s[0] != 'b' || s[1] != 'a' || s[2] != 'b' || s[3] != 'a') {
				return 15;
			}

			s = s.substr(-3, -2);
			// true: s got replaced by operator=.
			if (!check_str(true, 1, 7) || s[0] != 'a') {
				return 16;
			}

			s = dmstr(SL("ab")) + 'c';
			// true: s got replaced by operator=.
			if (!check_str(true, 3, 7) || s[0] != 'a' || s[1] != 'b' || s[2] != 'c') {
				return 17;
			}

			s += 'd';
			// false: there should’ve been enough space for “abcd”.
			if (!check_str(false, 4, 7) || s[0] != 'a' || s[1] != 'b' || s[2] != 'c' || s[3] != 'd') {
				return 18;
			}

			s += SL("efghijklmnopqrstuvwxyz");
			// false: while this will need to reallocate, the heap should be able to just resize the
			// allocated block, so the pointer won’t change.
			if (!check_str(false, 26, 55) || s != SL("abcdefghijklmnopqrstuvwxyz")) {
				return 19;
			}
		}

		// ASCII character and substring search.
		// The string “acabaabca” has the following properties:
		// •  misleading start for “ab” at index 0 (it’s “ac” instead) and for “abc” at index 2 (it’s
		//    “aba” instead), to catch incorrect skip-last comparisons;
		// •  first and last characters match 'a', but other inner ones do too;
		// •  would match “abcd” were it not for the last character;
		// •  matches the self-repeating “abaabc” but not the (also self-repeating) “abaabcd”.
#ifdef U8L
		{
			istr8 const s8(U8L("acabaabca"));
			istr8::const_iterator

			it = s8.find('b');
			if (it != s8.cbegin() + 3) {
				return 50;
			}

			it = s8.find(U8L("ab"));
			if (it != s8.cbegin() + 2) {
				return 51;
			}

			it = s8.find(U8L("abca"));
			if (it != s8.cbegin() + 5) {
				return 52;
			}

			it = s8.find(U8L("abcd"));
			if (it != s8.cend()) {
				return 53;
			}

			it = s8.find(U8L("abaabc"));
			if (it != s8.cbegin() + 2) {
				return 54;
			}

			it = s8.find(U8L("abaabcd"));
			if (it != s8.cend()) {
				return 55;
			}

			it = s8.find_last('b');
			if (it != s8.cend() - 3) {
				return 56;
			}

#if 0
			it = s8.find_last(U8L("ab"));
			if (it != s8.cend() - 4) {
				return 57;
			}

			it = s8.find_last(U8L("ac"));
			if (it != s8.cend() - 9) {
				return 58;
			}

			it = s8.find_last(U8L("ca"));
			if (it != s8.cend() - 2) {
				return 59;
			}
#endif
		}
#endif
#ifdef U16L
		{
			istr16 const s16(U16L("acabaabca"));
			istr16::const_iterator it;

			it = s16.find('b');
			if (it != s16.cbegin() + 3) {
				return 60;
			}

			it = s16.find(U16L("ab"));
			if (it != s16.cbegin() + 2) {
				return 61;
			}

			it = s16.find(U16L("abca"));
			if (it != s16.cbegin() + 5) {
				return 62;
			}

			it = s16.find(U16L("abcd"));
			if (it != s16.cend()) {
				return 63;
			}

			it = s16.find(U16L("abaabc"));
			if (it != s16.cbegin() + 2) {
				return 64;
			}

			it = s16.find(U16L("abaabcd"));
			if (it != s16.cend()) {
				return 65;
			}

			it = s16.find_last('b');
			if (it != s16.cend() - 3) {
				return 66;
			}

#if 0
			it = s16.find_last(U16L("ab"));
			if (it != s16.cend() - 4) {
				return 67;
			}

			it = s16.find_last(U16L("ac"));
			if (it != s16.cend() - 9) {
				return 68;
			}

			it = s16.find_last(U16L("ca"));
			if (it != s16.cend() - 2) {
				return 69;
			}
#endif
		}
#endif
#ifdef U32L
		{
			istr32 const s32(U32L("acabaabca"));
			istr32::const_iterator it;

			it = s32.find('b');
			if (it != s32.cbegin() + 3) {
				return 70;
			}

			it = s32.find(U32L("ab"));
			if (it != s32.cbegin() + 2) {
				return 71;
			}

			it = s32.find(U32L("abca"));
			if (it != s32.cbegin() + 5) {
				return 72;
			}

			it = s32.find(U32L("abcd"));
			if (it != s32.cend()) {
				return 73;
			}

			it = s32.find(U32L("abaabc"));
			if (it != s32.cbegin() + 2) {
				return 74;
			}

			it = s32.find(U32L("abaabcd"));
			if (it != s32.cend()) {
				return 75;
			}

			it = s32.find_last('b');
			if (it != s32.cend() - 3) {
				return 76;
			}

#if 0
			it = s32.find_last(U32L("ab"));
			if (it != s32.cend() - 4) {
				return 77;
			}

			it = s32.find_last(U32L("ac"));
			if (it != s32.cend() - 9) {
				return 78;
			}

			it = s32.find_last(U32L("ca"));
			if (it != s32.cend() - 2) {
				return 79;
			}
#endif
		}
#endif

		// Non-ASCII character and substring search.
#ifdef U8L
		{
			istr8 const s8(U8L("àßçàŒ"));
			istr8::const_iterator it;

			it = s8.find(U32L('ß'));
			if (it != s8.cbegin() + 2) {
				return 80;
			}

			it = s8.find(U8L("àß"));
			if (it != s8.cbegin() + 0) {
				return 81;
			}

			it = s8.find(U8L("àŒ"));
			if (it != s8.cbegin() + 6) {
				return 82;
			}

			it = s8.find(U8L("àü"));
			if (it != s8.cend()) {
				return 83;
			}
		}
#endif
#ifdef U16L
		{
			istr16 const s16(U16L("àßçàŒ"));
			istr16::const_iterator it;

			it = s16.find(U32L('ß'));
			if (it != s16.cbegin() + 1) {
				return 90;
			}

			it = s16.find(U16L("àß"));
			if (it != s16.cbegin() + 0) {
				return 91;
			}

			it = s16.find(U16L("àŒ"));
			if (it != s16.cbegin() + 3) {
				return 92;
			}

			it = s16.find(U16L("àü"));
			if (it != s16.cend()) {
				return 93;
			}
		}
#endif
#ifdef U32L
		{
			istr32 const s32(U32L("àßçàŒ"));
			istr32::const_iterator it;

			it = s32.find(U32L('ß'));
			if (it != s32.cbegin() + 1) {
				return 100;
			}

			it = s32.find(U32L("àß"));
			if (it != s32.cbegin() + 0) {
				return 101;
			}

			it = s32.find(U32L("àŒ"));
			if (it != s32.cbegin() + 3) {
				return 102;
			}

			it = s32.find(U32L("àü"));
			if (it != s32.cend()) {
				return 103;
			}
		}
#endif

		return EXIT_SUCCESS;
	}


private:

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

ABC_DECLARE_MODULE_IMPL_CLASS(test_module)

