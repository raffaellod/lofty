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
#include <abc/exception.hxx>
#include <abc/byteorder.hxx>
#include <abc/text.hxx>
#include <abc/utf_traits.hxx>
#include <abc/trace.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text globals

namespace abc {

namespace text {

ABCAPI size_t estimate_transcoded_size(
	encoding encSrc, void const * pSrc, size_t cbSrc, encoding encDst
) {
	abc_trace_fn((encSrc, pSrc, cbSrc, encDst));

	// Average size, in bytes, of 10 characters in each supported encoding.
	static uint8_t const sc_acbAvg10Chars[] = {
		22, // encoding::utf8: take into account that some languages require 3 bytes per character.
		20, // encoding::utf16le: consider surrogates extremely unlikely, as they are.
		20, // encoding::utf16be: same as encoding::utf16le.
		40, // encoding::utf32le: constant.
		40, // encoding::utf32be: same as encoding::utf32le.
		10, // encoding::iso_8859_1: constant.
		10, // encoding::windows_1252: constant.
		10, // encoding::ebcdic: constant.
	};

	if (encSrc < encoding::_charsets_offset) {
		abc_throw(argument_error, ());
	}
	if (encDst < encoding::_charsets_offset) {
		abc_throw(argument_error, ());
	}
	// TODO: use this to give a more accurate estimate for UTF-8, by evaluating which language block
	// seems to be dominant in the source.
	UNUSED_ARG(pSrc);

	// Estimate the number of code points in the source.
	size_t cbSrcAvg(sc_acbAvg10Chars[encSrc.base() - encoding::_charsets_offset]),
			 cbDstAvg(sc_acbAvg10Chars[encDst.base() - encoding::_charsets_offset]);
	// If we were using floating-point math, this would be the return statement’s expression:
	//    ceil(cbSrc / cbSrcAvg) * cbDstAvg
	// We need to emulate ceil() on integers with:
	//    (cbSrc + cbSrcAvg - 1) / cbSrcAvg
	// Also, we have to multiply first, to avoid underflow, but this would expose to overflow, in
	// which case we’ll divide first, as in the original expression.
	size_t ccp(cbSrc * cbDstAvg);
	if (ccp >= cbSrc) {
		// No integer overflow.
		return (ccp + cbSrcAvg - 1) / cbSrcAvg;
	} else {
		// Integer overflow occurred: evaluate the expression in the origianal order.
		return ((cbSrc + cbSrcAvg - 1) / cbSrcAvg) * cbDstAvg;
	}
}


ABCAPI size_t get_encoding_size(encoding enc) {
	// Character size, in bytes, for each recognized encoding.
	static uint8_t const sc_acbEncChar[] = {
		0, // encoding::unknown
		0, // encoding::identity
		1, // encoding::utf8
		2, // encoding::utf16le
		2, // encoding::utf16be
		4, // encoding::utf32le
		4, // encoding::utf32be
		1, // encoding::iso_8859_1
		1, // encoding::windows_1252
		1, // encoding::ebcdic
	};
	return sc_acbEncChar[enc.base()];
}


ABCAPI void const * get_line_terminator_bytes(
	encoding enc, line_terminator lterm, size_t * pcb
) {
	abc_trace_fn((enc, lterm, pcb));

	// Characters that compose line terminators, in every encoding. Below we cherry-pick from these
	// arrays the individual sequences; that’s why here 0x0d always comes before 0x0a.

	static uint8_t const sc_abAscii[2] = { 0x0d, 0x0a };
	static uint16_t const sc_abU16le[2] = {
		STATIC_BYTEORDER_HOSTTOLE16(0x000d), STATIC_BYTEORDER_HOSTTOLE16(0x000a)
	};
	static uint16_t const sc_abU16be[2] = {
		STATIC_BYTEORDER_HOSTTOBE16(0x000d), STATIC_BYTEORDER_HOSTTOBE16(0x000a)
	};
	static uint32_t const sc_abU32le[2] = {
		STATIC_BYTEORDER_HOSTTOLE32(0x000d), STATIC_BYTEORDER_HOSTTOLE32(0x000a)
	};
	static uint32_t const sc_abU32be[2] = {
		STATIC_BYTEORDER_HOSTTOBE32(0x00000d), STATIC_BYTEORDER_HOSTTOBE32(0x00000a)
	};
	static uint8_t const sc_bEbcdic(0x15);

	// All the possible line terminator sequences follow. Pointers to these structs are then arranged
	// in a lookup table, below.

	// Little helper to store start and size of each line terminator sequence.
	struct lts_t {
		void const * p;
		size_t cb;
	};

	static lts_t const sc_ltsAsciiCr   = { &sc_abAscii[0], sizeof(uint8_t )     };
	static lts_t const sc_ltsAsciiLf   = { &sc_abAscii[1], sizeof(uint8_t )     };
	static lts_t const sc_ltsAsciiCrLf = {  sc_abAscii,    sizeof(uint8_t ) * 2 };

	static lts_t const sc_ltsU16leCr   = { &sc_abU16le[0], sizeof(uint16_t)     };
	static lts_t const sc_ltsU16leLf   = { &sc_abU16le[1], sizeof(uint16_t)     };
	static lts_t const sc_ltsU16leCrLf = {  sc_abU16le,    sizeof(uint16_t) * 2 };

	static lts_t const sc_ltsU16beCr   = { &sc_abU16be[0], sizeof(uint16_t)     };
	static lts_t const sc_ltsU16beLf   = { &sc_abU16be[1], sizeof(uint16_t)     };
	static lts_t const sc_ltsU16beCrLf = {  sc_abU16be,    sizeof(uint16_t) * 2 };

	static lts_t const sc_ltsU32leCr   = { &sc_abU32le[0], sizeof(uint32_t)     };
	static lts_t const sc_ltsU32leLf   = { &sc_abU32le[1], sizeof(uint32_t)     };
	static lts_t const sc_ltsU32leCrLf = {  sc_abU32le,    sizeof(uint32_t) * 2 };

	static lts_t const sc_ltsU32beCr   = { &sc_abU32be[0], sizeof(uint32_t)     };
	static lts_t const sc_ltsU32beLf   = { &sc_abU32be[1], sizeof(uint32_t)     };
	static lts_t const sc_ltsU32beCrLf = {  sc_abU32be,    sizeof(uint32_t) * 2 };

	static lts_t const sc_ltsEbcdicNel = { &sc_bEbcdic,    sizeof(uint8_t )     };

	// Now we can arrange the above in a bidimensional lookup table, to quickly jump to the correct
	// lts_t by encoding first, and desired line terminator sequence then.
	// For convenience, line_terminator::nel is remapped to line_terminator::cr_lf.

	static lts_t const * const sc_lts[][3] = {
		//	line_terminator::cr, line_terminator::lf, line_terminator::cr_lf/nel
		{ &sc_ltsAsciiCr, &sc_ltsAsciiLf, &sc_ltsAsciiCrLf }, // encoding::utf8
		{ &sc_ltsU16leCr, &sc_ltsU16leLf, &sc_ltsU16leCrLf }, // encoding::utf16le
		{ &sc_ltsU16beCr, &sc_ltsU16beLf, &sc_ltsU16beCrLf }, // encoding::utf16be
		{ &sc_ltsU32leCr, &sc_ltsU32leLf, &sc_ltsU32leCrLf }, // encoding::utf32le
		{ &sc_ltsU32beCr, &sc_ltsU32beLf, &sc_ltsU32beCrLf }, // encoding::utf32be
		{ &sc_ltsAsciiCr, &sc_ltsAsciiLf, &sc_ltsAsciiCrLf }, // encoding::iso_8859_1
		{ &sc_ltsAsciiCr, &sc_ltsAsciiLf, &sc_ltsAsciiCrLf }, // encoding::windows_1252
		{           NULL,           NULL, &sc_ltsEbcdicNel }  // encoding::ebcdic
	};

	// Finally, the actual code: just a simple lookup.

	// Reject non-charset encodings, because we can’t determine what value CR or LF should have.
	if (!get_encoding_size(enc)) {
		abc_throw(argument_error, ());
	}
	// Reject line_terminator::nel for every encoding except encoding::ebcdic, for which it’s the
	// only allowed line_terminator.
	if ((enc == encoding::ebcdic) != (lterm == line_terminator::nel)) {
		abc_throw(argument_error, ());
	}
	// Do the NEL > CRLF remapping mentioned above.
	if (enc == encoding::ebcdic) {
		lterm = line_terminator::cr_lf;
	}
	lts_t const * plts(
		sc_lts[enc.base() - encoding::_charsets_offset][lterm.base() - line_terminator::_known_offset]
	);
	ABC_ASSERT(plts);
	*pcb = plts->cb;
	return plts->p;
}


ABCAPI encoding guess_encoding(
	void const * pBuf, size_t cbBuf, size_t cbSrcTotal /*= 0*/, size_t * pcbBom /*= NULL*/
) {
	abc_trace_fn((pBuf, cbBuf, cbSrcTotal, pcbBom));

	uint8_t const * pbBuf(static_cast<uint8_t const *>(pBuf));
	// If the total size is not specified, assume that the buffer is the whole source.
	if (!cbSrcTotal)
		cbSrcTotal = cbBuf;

	// Statuses for the scanner. Each BOM status must be 1 bit to the right of its resulting
	// encoding; LE variants must be 2 bits to the right of their BE counterparts.
	enum enc_scan_status {
		ESS_UTF8_BOM     = 0x0001,
		ESS_UTF8         = 0x0002,
		ESS_UTF16LE_BOM  = 0x0004,
		ESS_UTF16LE      = 0x0008,
		ESS_UTF16BE_BOM  = 0x0010,
		ESS_UTF16BE      = 0x0020,
		ESS_UTF32LE_BOM  = 0x0040,
		ESS_UTF32LE      = 0x0080,
		ESS_UTF32BE_BOM  = 0x0100,
		ESS_UTF32BE      = 0x0200,
		ESS_ISO_8859_1   = 0x0400,
		ESS_WINDOWS_1252 = 0x0800,

		// ESS_UTF*_BOM
		ESS_MASK_BOMS    = 0x0155,
		// ESS_UTF16*
		ESS_MASK_UTF16   = 0x003c,
		// ESS_UTF32*
		ESS_MASK_UTF32   = 0x03c0,
		// Everything else.
		ESS_MASK_NONUTF  = 0x0d00,
		// Start status.
		ESS_MASK_START   = ESS_MASK_NONUTF | ESS_MASK_BOMS | ESS_UTF8
	};

	// A 1 in this bit array means that the corresponding byte value is valid in ISO-8859-1.
	static uint8_t const sc_abValidISO88591[] = {
		0x80, 0x3e, 0x00, 0x08, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
		0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	};
	// A 1 in this bit array means that the corresponding byte value is valid in Windows-1252.
	static uint8_t const sc_abValidWindows1252[] = {
		0x80, 0x3e, 0x00, 0x08, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
		0xfd, 0x5f, 0xfe, 0xdf, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	};
	// BOMs for sc_absd.
	static uint8_t const
		sc_abUtf8Bom   [] = { 0xef, 0xbb, 0xbf },
		sc_abUtf16leBom[] = { 0xff, 0xfe },
		sc_abUtf16beBom[] = { 0xfe, 0xff },
		sc_abUtf32leBom[] = { 0xff, 0xfe, 0x00, 0x00 },
		sc_abUtf32beBom[] = { 0x00, 0x00, 0xfe, 0xff };
	// Struct to uniformize scanning for BOMs.
	static struct bomscandata_t {
		enc_scan_status ess;
		uint8_t const * pabBom;
		size_t cbBom;
	} const sc_absd[] = {
		{ ESS_UTF8_BOM,    sc_abUtf8Bom,    sizeof(sc_abUtf8Bom   ) },
		{ ESS_UTF16LE_BOM, sc_abUtf16leBom, sizeof(sc_abUtf16leBom) },
		{ ESS_UTF16BE_BOM, sc_abUtf16beBom, sizeof(sc_abUtf16beBom) },
		{ ESS_UTF32LE_BOM, sc_abUtf32leBom, sizeof(sc_abUtf32leBom) },
		{ ESS_UTF32BE_BOM, sc_abUtf32beBom, sizeof(sc_abUtf32beBom) }
	};


	// Initially, consider anything that doesn’t require a BOM.
	unsigned fess(ESS_MASK_START);

	// Initially, assume no BOM will be found.
	if (pcbBom) {
		*pcbBom = 0;
	}

	// Easy checks.
	if (cbSrcTotal & (sizeof(char32_t) - 1)) {
		// UTF-32 requires a number of bytes multiple of sizeof(char32_t).
		fess &= ~unsigned(ESS_MASK_UTF32);
		if (cbSrcTotal & (sizeof(char16_t) - 1)) {
			// UTF-16 requires an even number of bytes.
			fess &= ~unsigned(ESS_MASK_UTF16);
		}
	}

	// Parse every byte, gradually excluding more and more possibilities, hopefully ending with
	// exactly one guess.
	unsigned cbUtf8Cont(0);
	for (size_t ib(0); ib < cbBuf; ++ib) {
		uint8_t b(pbBuf[ib]);

		if (fess & ESS_UTF8) {
			// Check for UTF-8 validity. Checking for overlongs or invalid code points is out of scope
			// here.
			if (cbUtf8Cont) {
				if ((b & 0xc0) != 0x80) {
					// This byte should be part of a sequence, but it’s not.
					fess &= ~unsigned(ESS_UTF8);
				} else {
					--cbUtf8Cont;
				}
			} else {
				if ((b & 0xc0) == 0x80) {
					// This byte should be a leading byte, but it’s not.
					fess &= ~unsigned(ESS_UTF8);
				} else {
					cbUtf8Cont = utf8_traits::leading_to_cont_length(char8_t(b));
					if ((b & 0x80) && !cbUtf8Cont) {
						// By utf8_traits::leading_to_cont_length(), a non-ASCII byte that doesn’t have a
						// continuation is an invalid one.
						fess &= ~unsigned(ESS_UTF8);
					}
				}
			}
		}

		if (fess & (ESS_UTF16LE | ESS_UTF16BE)) {
			// Check for UTF-16 validity. The only check possible is proper ordering of surrogate
			// pairs; everything else is allowed.
			for (unsigned ess(ESS_UTF16LE); ess <= ESS_UTF16BE; ess <<= 2) {
				// This will go ahead with the check if ib is indexing the most significant byte, i.e.
				// odd for LE and even for BE.
				if ((fess & ess) && ((ib & sizeof(char16_t)) != 0) == (ess != ESS_UTF16LE)) {
					switch (b & 0xfc) {
						case 0xd8: {
							// There must be a surrogate second after 1 byte, and there have to be enough
							// bytes in the source; skip the check if the buffer doesn’t include that byte.
							size_t ibNext(ib + sizeof(char16_t));
							if (
								(cbSrcTotal && ibNext >= cbSrcTotal) ||
								(ibNext < cbBuf && (pbBuf[ibNext] & 0xfc) != 0xdc)
							) {
								fess &= ~ess;
							}
							break;
						}
						case 0xdc: {
							// Assume there was a surrogate first 2 bytes before.
							size_t ibPrev(ib - sizeof(char16_t));
							// ibPrev < ib checks for underflow of ibPrev.
							if (ibPrev < ib && (pbBuf[ibPrev] & 0xfc) != 0xd8) {
								fess &= ~ess;
							}
							break;
						}
					}
				}
			}
		}

		if ((fess & (ESS_UTF32LE | ESS_UTF32BE)) && (ib & sizeof(char32_t)) == sizeof(char32_t) - 1) {
			// Check for UTF-32 validity. Just ensure that each quadruplet of bytes defines a valid
			// UTF-32 character; this is fairly strict, as it requires one 00 byte every four bytes, as
			// well as other restrictions.
			uint32_t ch(*reinterpret_cast<uint32_t const *>(pbBuf + ib - (sizeof(char32_t) - 1)));
			if ((fess & ESS_UTF32LE) && !utf32_traits::is_valid(byteorder::le_to_host(ch))) {
				fess &= ~unsigned(ESS_UTF32LE);
			}
			if ((fess & ESS_UTF32BE) && !utf32_traits::is_valid(byteorder::be_to_host(ch))) {
				fess &= ~unsigned(ESS_UTF32BE);
			}
		}

		if (fess & ESS_ISO_8859_1) {
			// Check for ISO-8859-1 validity. This is more of a guess, since there’s a big many other
			// encodings which could pass this check.
			if ((sc_abValidISO88591[b >> 3] & (1 << (b & 7))) == 0) {
				fess &= ~unsigned(ESS_ISO_8859_1);
			}
		}

		if (fess & ESS_WINDOWS_1252) {
			// Check for Windows-1252 validity. Even more of a guess, since this considers valid more
			// characters still.
			if ((sc_abValidWindows1252[b >> 3] & (1 << (b & 7))) == 0) {
				fess &= ~unsigned(ESS_WINDOWS_1252);
			}
		}

		if (fess & ESS_MASK_BOMS) {
			// Lastly, check for one or more BOMs. This needs to be last, so if it enables other
			// checks, they don’t get performed on the last BOM byte it just analyzed, which would most
			// likely cause them to fail.
			for (size_t iBsd(0); iBsd < countof(sc_absd); ++iBsd) {
				unsigned essBom(sc_absd[iBsd].ess);
				if (fess & essBom) {
					if (b != sc_absd[iBsd].pabBom[ib]) {
						// This byte doesn’t match: stop checking for this BOM.
						fess &= ~essBom;
					} else if (ib == sc_absd[iBsd].cbBom - 1) {
						// This was the last BOM byte, which means that the whole BOM was matched: stop
						// checking for the BOM, and enable checking for the encoding itself.
						fess &= ~essBom;
						fess |= essBom << 1;
						// Return the BOM length to the caller, if requested. This will be overwritten in
						// case another, longer BOM is found (e.g. the BOM in UTF-16LE is the start of the
						// BOM in UTF-32LE).
						if (pcbBom) {
							*pcbBom = sc_absd[iBsd].cbBom;
						}
					}
				}
			}
		}
	}

	// Now, of all possibilities, pick the most likely.
	if (fess & ESS_UTF8) {
		return encoding::utf8;
	} else if (fess & ESS_UTF32LE) {
		return encoding::utf32le;
	} else if (fess & ESS_UTF32BE) {
		return encoding::utf32be;
	} else if (fess & ESS_UTF16LE) {
		return encoding::utf16le;
	} else if (fess & ESS_UTF16BE) {
		return encoding::utf16be;
	} else if (fess & ESS_ISO_8859_1) {
		return encoding::iso_8859_1;
	} else if (fess & ESS_WINDOWS_1252) {
		return encoding::windows_1252;
	} else {
		return encoding::unknown;
	}
}


ABCAPI line_terminator guess_line_terminator(void const * pBuf, size_t cchBuf, encoding enc) {
	abc_trace_fn((pBuf, cchBuf, enc));

	size_t cbChar(get_encoding_size(enc));
	// Reject non-charset encodings, because we can’t determine what value CR or LF should have.
	if (!cbChar) {
		abc_throw(argument_error, ());
	}

	line_terminator lterm(line_terminator::unknown);
	void const * pBufMax(static_cast<int8_t const *>(pBuf) + cbChar * cchBuf);
	switch (cbChar) {
		case sizeof(char8_t): {
			uint8_t const * pchBuf(static_cast<uint8_t const *>(pBuf));
			if (enc == encoding::ebcdic) {
				// Special case for EBCDIC.
				uint8_t chNel(0x15);
				for (; pchBuf < static_cast<uint8_t const *>(pBufMax); ++pchBuf) {
					if (*pchBuf == chNel) {
						lterm = line_terminator::nel;
						break;
					}
				}
			} else {
				// It’s one of the supported byte-oriented character sets.
				// A note on scanning an UTF-8 buffer: we want this to be tolerant to encoding errors,
				// so exploit the fact that no UTF-8 character can contain another, and just scan byte
				// by byte, without performing any check on lead bytes.
				uint8_t chCr(0x0d), chLf(0x0a);
				for (; pchBuf < static_cast<uint8_t const *>(pBufMax); ++pchBuf) {
					if (*pchBuf == chCr) {
						// CR can be followed by a LF to form the sequence CRLF, so check the following
						// character (if we have one). If we found a CR as the very last character in the
						// buffer, we can’t check the following one; at this point, we have to guess, so
						// we’ll consider CRLF more likely than CR.
						if (++pchBuf < pBufMax && *pchBuf != chLf) {
							lterm = line_terminator::cr;
						} else {
							lterm = line_terminator::cr_lf;
						}
						break;
					} else if (*pchBuf == chLf) {
						lterm = line_terminator::lf;
						break;
					}
				}
			}
			break;
		}

		case sizeof(char16_t): {
			if (enc != encoding::utf16le && enc != encoding::utf16be) {
				abc_throw(argument_error, ());
			}
			uint16_t chCr(uint16_t(enc == encoding::utf16le
				? STATIC_BYTEORDER_HOSTTOLE16(0x000d) : STATIC_BYTEORDER_HOSTTOBE16(0x000d)
			));
			uint16_t chLf(uint16_t(enc == encoding::utf16le
				? STATIC_BYTEORDER_HOSTTOLE16(0x000au) : STATIC_BYTEORDER_HOSTTOBE16(0x000du)
			));
			for (
				uint16_t const * pchBuf(static_cast<uint16_t const *>(pBuf));
				pchBuf < static_cast<uint16_t const *>(pBufMax);
				++pchBuf
			) {
				if (*pchBuf == chCr) {
					// See comments in the sizeof(char8_t) case.
					if (pchBuf < pBufMax && *(pchBuf + 1) != chLf) {
						lterm = line_terminator::cr;
					} else {
						lterm = line_terminator::cr_lf;
					}
					break;
				} else if (*pchBuf == chLf) {
					lterm = line_terminator::lf;
					break;
				}
			}
			break;
		}

		case sizeof(char32_t): {
			if (enc != encoding::utf32le && enc != encoding::utf32be) {
				abc_throw(argument_error, ());
			}
			uint32_t chCr(enc == encoding::utf32le
				? STATIC_BYTEORDER_HOSTTOLE32(0x00000d) : STATIC_BYTEORDER_HOSTTOBE32(0x00000d)
			);
			uint32_t chLf(enc == encoding::utf32le
				? STATIC_BYTEORDER_HOSTTOLE32(0x00000a) : STATIC_BYTEORDER_HOSTTOBE32(0x00000d)
			);
			for (
				uint32_t const * pchBuf(static_cast<uint32_t const *>(pBuf));
				pchBuf < static_cast<uint32_t const *>(pBufMax);
				++pchBuf
			) {
				if (*pchBuf == chCr) {
					// See comments in the sizeof(char8_t) case.
					if (++pchBuf < pBufMax && *pchBuf != chLf) {
						lterm = line_terminator::cr;
					} else {
						lterm = line_terminator::cr_lf;
					}
					break;
				} else if (*pchBuf == chLf) {
					lterm = line_terminator::lf;
					break;
				}
			}
			break;
		}
	}

	return lterm;
}


ABCAPI size_t transcode(
	std::nothrow_t const &,
	encoding encSrc, void const ** ppSrc, size_t * pcbSrc,
	encoding encDst, void       ** ppDst, size_t * pcbDstMax
) {
	abc_trace_fn((encSrc, ppSrc, pcbSrc, encDst, ppDst, pcbDstMax));

	uint8_t const * pbSrc(static_cast<uint8_t const *>(*ppSrc));
	uint8_t       * pbDst(static_cast<uint8_t       *>(*ppDst));
	uint8_t const * pbSrcEnd(pbSrc + *pcbSrc);
	uint8_t const * pbDstEnd(pbDst + *pcbDstMax);

	uint8_t const * pbSrcLastUsed;
	for (;;) {
		pbSrcLastUsed = pbSrc;
		char32_t ch32;

		// Decode a source code point into ch32.
		switch (encSrc.base()) {
			case encoding::utf8: {
				if (pbSrc + sizeof(char8_t) > pbSrcEnd) {
					goto break_for;
				}
				char8_t ch8Src(char8_t(*pbSrc++));
				switch (ch8Src & 0xc0) {
					default: {
						unsigned cbCont(utf8_traits::leading_to_cont_length(ch8Src));
						// Ensure that we still have enough characters.
						if (pbSrc + cbCont > pbSrcEnd) {
							goto break_for;
						}
						// Convert the first byte to an UTF-32 character.
						ch32 = utf8_traits::get_leading_cp_bits(ch8Src, cbCont);
						// Shift in any continuation bytes.
						for (; cbCont; --cbCont) {
							ch8Src = char8_t(*pbSrc++);
							if ((ch8Src & 0xc0) != 0x80) {
								// The sequence ended prematurely, and this byte is not part of it.
								--pbSrc;
								break;
							}
							ch32 = (ch32 << 6) | (ch8Src & 0x3f);
						}
						if (!cbCont && utf32_traits::is_valid(ch32)) {
							// The whole sequence was read, and the result is valid UTF-32.
							break;
						}
						// Replace this invalid code point (fall through).
					}
					case 0x80:
						// Replace this invalid byte.
						ch32 = replacement_char;
						break;
				}
				break;
			}

			case encoding::utf16le:
			case encoding::utf16be: {
				if (pbSrc + sizeof(char16_t) > pbSrcEnd) {
					goto break_for;
				}
				char16_t ch16Src(*reinterpret_cast<char16_t const *>(pbSrc));
				pbSrc += sizeof(char16_t);
				if (encSrc != encoding::utf16_host) {
					ch16Src = byteorder::swap(ch16Src);
				}
				switch (ch16Src & 0xfc00) {
					case 0xd800:
						// Surrogate first half.
						ch32 = char32_t(ch16Src & 0x03ff) << 10;
						if (pbSrc + sizeof(char16_t) > pbSrcEnd) {
							goto break_for;
						}
						ch16Src = *reinterpret_cast<char16_t const *>(pbSrc);
						pbSrc += sizeof(char16_t);
						if (encSrc != encoding::utf16_host) {
							ch16Src = byteorder::swap(ch16Src);
						}
						// This character must be a surrogate second half.
						if ((ch16Src & 0xfc00) == 0xdc00) {
							ch32 = (ch32 | (ch16Src & 0x03ff)) + 0x10000;
							if (utf32_traits::is_valid(ch32)) {
								break;
							}
							// Replace this invalid code point (fall through).
						}
						// Replace this invalid single surrogate (fall through).
					case 0xdc00:
						// Replace this invalid second half of a surrogate pair.
						ch16Src = replacement_char;
						break;
					default:
						ch32 = ch16Src;
						break;
				}
				break;
			}

			case encoding::utf32be:
			case encoding::utf32le:
				if (pbSrc + sizeof(char32_t) > pbSrcEnd) {
					goto break_for;
				}
				ch32 = *reinterpret_cast<char32_t const *>(pbSrc);
				pbSrc += sizeof(char32_t);
				if (encSrc != encoding::utf32_host) {
					ch32 = byteorder::swap(ch32);
				}
				break;

			case encoding::iso_8859_1:
				// TODO: ISO-8859-1 support.
				break;

			case encoding::windows_1252:
				// TODO: Windows-1252 support.
				break;

			default:
				// TODO: support more character sets/encodings?
				break;
		}

		// Encode the code point in ch32 into the destination.
		switch (encDst.base()) {
			case encoding::utf8: {
				// Compute the length of this sequence.
				unsigned cbSeq;
				if (ch32 <= 0x00007f) {
					cbSeq = 1;
				} else if (ch32 <= 0x0007ff) {
					cbSeq = 2;
				} else if (ch32 <= 0x00ffff) {
					cbSeq = 3;
				} else {
					cbSeq = 4;
				}
				if (pbDst + cbSeq > pbDstEnd) {
					goto break_for;
				}
				unsigned cbCont(cbSeq - 1);
				// Since each trailing byte can take 6 bits, the remaining ones (after >> 6 * cbCont)
				// make up what goes in the leading byte, which is combined with the proper sequence
				// indicator.
				*pbDst++ = uint8_t(
					utf8_traits::cont_length_to_seq_indicator(cbCont) | char8_t(ch32 >> 6 * cbCont)
				);
				while (cbCont--) {
					*pbDst++ = uint8_t(0x80 | ((ch32 >> 6 * cbCont) & 0x3f));
				}
				break;
			}

			case encoding::utf16le:
			case encoding::utf16be: {
				unsigned cbCont(unsigned(ch32 > 0x00ffff) << 1);
				if (pbDst + sizeof(char16_t) + cbCont > pbDstEnd) {
					goto break_for;
				}
				char16_t ch16Dst0, ch16Dst1;
				if (cbCont) {
					ch32 -= 0x10000;
					ch16Dst0 = char16_t(0xd800 | ((ch32 & 0x0ffc00) >> 10));
					ch16Dst1 = char16_t(0xdc00 |  (ch32 & 0x0003ff)       );
				} else {
					ch16Dst0 = char16_t(ch32);
				}
				if (encSrc != encoding::utf16_host) {
					ch16Dst0 = byteorder::swap(ch16Dst0);
					if (cbCont) {
						ch16Dst1 = byteorder::swap(ch16Dst1);
					}
				}
				*reinterpret_cast<char16_t *>(pbDst) = ch16Dst0;
				if (cbCont) {
					pbDst += sizeof(char16_t);
					*reinterpret_cast<char16_t *>(pbDst) = ch16Dst1;
				}
				pbDst += sizeof(char16_t);
				break;
			}

#if ABC_HOST_LITTLE_ENDIAN
			case encoding::utf32be:
#else
			case encoding::utf32le:
#endif
				// The destination endianness is the opposite of the host’s.
				ch32 = byteorder::swap(ch32);
				// Fall through.
			case encoding::utf32_host:
				if (pbDst + sizeof(char32_t) > pbDstEnd) {
					goto break_for;
				}
				*reinterpret_cast<char32_t *>(pbDst) = ch32;
				pbDst += sizeof(char32_t);
				break;

			case encoding::iso_8859_1:
				// TODO: ISO-8859-1 support.
				break;

			case encoding::windows_1252:
				// TODO: Windows-1252 support.
				break;

			default:
				// TODO: support more character sets/encodings?
				break;
		}
	}
break_for:
	// Undo any incomplete or unused read.
	pbSrc = pbSrcLastUsed;
	// Update the variables pointed to by the arguments.
	*pcbSrc -= size_t(pbSrc - static_cast<uint8_t const *>(*ppSrc));
	*ppSrc = pbSrc;
	size_t cbDstUsed(size_t(pbDst - static_cast<uint8_t *>(*ppDst)));
	*pcbDstMax -= cbDstUsed;
	*ppDst = pbDst;
	return cbDstUsed;
}

} //namespace text

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

