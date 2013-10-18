/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

Copyright 2010, 2011, 2012, 2013
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

#include <abc/core.hxx>
#include <abc/iostream.hxx>
#include <abc/trace.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_int_to_str_backend_base


namespace abc {

char_t const _int_to_str_backend_base::smc_achIntToStrU[16] = {
	CL('0'), CL('1'), CL('2'), CL('3'), CL('4'), CL('5'), CL('6'), CL('7'), CL('8'), CL('9'),
	CL('A'), CL('B'), CL('C'), CL('D'), CL('E'), CL('F')
};
char_t const _int_to_str_backend_base::smc_achIntToStrL[16] = {
	CL('0'), CL('1'), CL('2'), CL('3'), CL('4'), CL('5'), CL('6'), CL('7'), CL('8'), CL('9'),
	CL('a'), CL('b'), CL('c'), CL('d'), CL('e'), CL('f')
};


ABCAPI _int_to_str_backend_base::_int_to_str_backend_base(
	unsigned cbInt, char_range const & crFormat
) :
	m_pchIntToStr(smc_achIntToStrL),
	m_iBaseOrShift(10),
	// Default to generating at least a single zero.
	m_cchWidth(1),
	m_chPad(CL(' ')),
	// A sign will only be displayed if the number is negative and no prefix is applied.
	m_chSign(CL('\0')),
	m_chPrefix0(CL('\0')),
	m_chPrefix1(CL('\0')) {
	abc_trace_fn((this, cbInt, crFormat));

	bool bPrefix(false), bDefaultNotation(true);
	auto it(crFormat.cbegin());
	char_t ch;
	if (it == crFormat.cend()) {
		goto default_notation;
	}
	ch = *it++;
	// Display a plus or a space in front of non-negative numbers.
	if (ch == CL('+') || ch == CL(' ')) {
		// Force this character to be displayed for non-negative numbers.
		m_chSign = ch;
		if (it == crFormat.cend()) {
			goto default_notation;
		}
		ch = *it++;
	}
	// Prefix with 0b, 0B, 0, 0x or 0X.
	if (ch == CL('#')) {
		bPrefix = true;
		if (it == crFormat.cend()) {
			goto default_notation;
		}
		ch = *it++;
	}
	// Pad with zeroes instead of spaces.
	if (ch == CL('0')) {
		m_chPad = CL('0');
		if (it == crFormat.cend()) {
			goto default_notation;
		}
		ch = *it++;
	}
	// “Width” - minimum number of digits.
	if (ch >= CL('1') && ch <= CL('9')) {
		// Undo the default; the following loop will yield at least 1 anyway (because we don’t get
		// here for a 0 - see if above).
		m_cchWidth = 0;
		do {
			m_cchWidth = m_cchWidth * 10 + unsigned(ch) - CL('0');
			if (it == crFormat.cend()) {
				goto default_notation;
			}
			ch = *it++;
		} while (ch >= CL('0') && ch <= CL('9'));
	}

	bDefaultNotation = false;
default_notation:
	// If we skipped the assignment to false, we run out of characters, so default the notation.
	if (bDefaultNotation) {
		ch = CL('d');
	}

	// Determine which notation to use, which will also yield the approximate number of characters
	// per byte.
	unsigned cchByte;
	switch (ch) {
		case CL('b'):
		case CL('B'):
		case CL('o'):
		case CL('x'):
		case CL('X'):
			if (bPrefix) {
				m_chPrefix0 = CL('0');
			}
			// Fall through.
		case CL('d'):
			switch (ch) {
				case CL('b'): // Binary notation, lowercase prefix.
				case CL('B'): // Binary notation, uppercase prefix.
					m_chPrefix1 = ch;
					m_iBaseOrShift = 1;
					cchByte = 8;
					break;
				case CL('o'): // Octal notation.
					m_iBaseOrShift = 3;
					cchByte = 3;
					break;
				case CL('X'): // Hexadecimal notation, uppercase prefix and letters.
					m_pchIntToStr = smc_achIntToStrU;
					// Fall through.
				case CL('x'): // Hexadecimal notation, lowercase prefix and letters.
					m_chPrefix1 = ch;
					m_iBaseOrShift = 4;
					cchByte = 2;
					break;
				case CL('d'): // Decimal notation.
					m_iBaseOrShift = 10;
					cchByte = 3;
					break;
			}
			if (it == crFormat.cend()) {
				break;
			}
			// If we still have any characters, they are garbage (fall through).
		default:
			abc_throw(syntax_error, (
				SL("unexpected character"), crFormat, unsigned(it - crFormat.cbegin())
			));
	}

	// Now we know enough to calculate the required buffer size.
	m_cchBuf = 2 /*prefix or sign*/ + std::max(m_cchWidth, cchByte * cbInt);
}


ABCAPI void _int_to_str_backend_base::add_prefixes_and_write(
	bool bNegative, ostream * posOut, mstr * psBuf, char_t * pchBufFirstUsed
) const {
	abc_trace_fn((this, bNegative, posOut, psBuf/*, pchBufFirstUsed*/));

	char_t const * pchBufEnd(psBuf->cend().base());
	char_t * pch(pchBufFirstUsed);
	// Ensure that at least one digit is generated.
	if (pch == pchBufEnd) {
		*--pch = CL('0');
	}
	// Determine the sign character: only if in decimal notation, and make it a minus sign if the
	// number is negative.
	char_t chSign(m_iBaseOrShift == 10 ? bNegative ? CL('-') : m_chSign : CL('\0'));
	// Decide whether we’ll put a sign last, after the padding.
	bool bSignLast(chSign && m_chPad == CL('0'));
	// Add the sign character if there’s no prefix and the padding is not zeroes.
	if (chSign && m_chPad != CL('0')) {
		*--pch = chSign;
	}
	// Ensure that at least m_cchWidth characters are generated (but reserve a space for the sign).
	char_t const * pchFirst(pchBufEnd - (m_cchWidth - (bSignLast ? 1 : 0)));
	while (pch > pchFirst) {
		*--pch = m_chPad;
	}
	// Add prefix or sign (if padding with zeroes), if any.
	if (m_chPrefix0) {
		if (m_chPrefix1) {
			*--pch = m_chPrefix1;
		}
		*--pch = m_chPrefix0;
	} else if (bSignLast) {
		// Add the sign character.
		*--pch = chSign;
	}
	// Write the constructed string.
	posOut->write_raw(pch, size_t(pchBufEnd - pch), text::encoding::host);
}


template <typename I>
inline void _int_to_str_backend_base::write_impl(I i, ostream * posOut) const {
	abc_trace_fn((this, i, posOut));

	// Create a buffer of sufficient size for binary notation (the largest).
	smstr<2 /* prefix or sign */ + sizeof(I) * CHAR_BIT> sBuf;
	sBuf.set_size(m_cchBuf);
	char_t * pch(sBuf.end().base());

	// Generate the digits.
	I iRest(i);
	if (m_iBaseOrShift == 10) {
		// Base 10: must use % and /.
		I iDivider((I(m_iBaseOrShift)));
		while (iRest) {
			I iMod(iRest % iDivider);
			iRest /= iDivider;
			*--pch = m_pchIntToStr[numeric::is_negative<I>(iMod) ? -iMod : iMod];
		}
	} else {
		// Base 2 ^ n: can use & and >>.
		I iMask((I(1) << m_iBaseOrShift) - 1);
		while (iRest) {
			*--pch = m_pchIntToStr[iRest & iMask];
			iRest >>= m_iBaseOrShift;
		}
	}

	// Add prefix or sign, and write to the ostream.
	add_prefixes_and_write(numeric::is_negative<I>(i), posOut, &sBuf, pch);
}


ABCAPI void _int_to_str_backend_base::write_s64(int64_t i, ostream * posOut) const {
	write_impl(i, posOut);
}


ABCAPI void _int_to_str_backend_base::write_u64(uint64_t i, ostream * posOut) const {
	write_impl(i, posOut);
}


#if ABC_HOST_WORD_SIZE < 64

ABCAPI void _int_to_str_backend_base::write_s32(int32_t i, ostream * posOut) const {
	write_impl(i, posOut);
}


ABCAPI void _int_to_str_backend_base::write_u32(uint32_t i, ostream * posOut) const {
	write_impl(i, posOut);
}


#if ABC_HOST_WORD_SIZE < 32

ABCAPI void _int_to_str_backend_base::write_s16(int16_t i, ostream * posOut) const {
	write_impl(i, posOut);
}


ABCAPI void _int_to_str_backend_base::write_u16(uint16_t i, ostream * posOut) const {
	write_impl(i, posOut);
}

#endif //if ABC_HOST_WORD_SIZE < 32
#endif //if ABC_HOST_WORD_SIZE < 64


} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend<bool>


namespace abc {

ABCAPI to_str_backend<bool>::to_str_backend(
	char_range const & crFormat /*= char_range()*/
) {
	abc_trace_fn((this, crFormat));

	auto it(crFormat.cbegin());

	// TODO: parse the format string.

	// If we still have any characters, they are garbage.
	if (it != crFormat.cend()) {
		abc_throw(syntax_error, (
			SL("unexpected character"), crFormat, unsigned(it - crFormat.cbegin())
		));
	}
}


ABCAPI void to_str_backend<bool>::write(bool b, ostream * posOut) {
	abc_trace_fn((this, b, posOut));

	// TODO: apply format options.
	if (b) {
		posOut->write(SL("true"));
	} else {
		posOut->write(SL("false"));
	}
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend<void const volatile *>


namespace abc {

char_t const to_str_backend<void const volatile *>::smc_achFormat[] = SL("#x");


ABCAPI to_str_backend<void const volatile *>::to_str_backend(
	char_range const & crFormat /*= char_range()*/
) :
	to_str_backend<uintptr_t>(char_range(smc_achFormat)) {
	abc_trace_fn((this, crFormat));

	auto it(crFormat.cbegin());

	// TODO: parse the format string.

	// If we still have any characters, they are garbage.
	if (it != crFormat.cend()) {
		abc_throw(syntax_error, (
			SL("unexpected character"), crFormat, unsigned(it - crFormat.cbegin())
		));
	}
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

