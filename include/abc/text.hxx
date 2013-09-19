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

#ifndef ABC_TEXT_HXX
#define ABC_TEXT_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/byteorder.hxx>
#include <abc/enum.hxx>
#include <memory>



////////////////////////////////////////////////////////////////////////////////////////////////////
// Declarations

namespace abc {

namespace text {

/// Recognized text encodings. Little endians should be listed before big endians; some code relies
// on this.
ABC_ENUM(encoding, \
	/** Unknown/undetermined encoding. */ \
	(unknown,          0), \
	/** Identity encoding: no transcoding is to occur. */ \
	(identity,         1), \
	/** Offset of the first charset encoding (as opposed to non-charsets, such as “unknown” and
	 * “identity”). */ \
	(_charsets_offset, 2), \
	/** UTF-8 encoding. */ \
	(utf8,             2), \
	/** UTF-16 Little Endian encoding. */ \
	(utf16le,          3), \
	/** UTF-16 Big Endian encoding. */ \
	(utf16be,          4), \
	/** UTF-32 Little Endian encoding. */ \
	(utf32le,          5), \
	/** UTF-32 Big Endian encoding. */ \
	(utf32be,          6), \
	/** ISO-8859-1 encoding. Only supported in detection and handling, but not as internal string
	 * representation. */ \
	(iso_8859_1,       7), \
	/** Windows-1252 encoding. Only supported in detection and handling, but not as internal string
	 * representation. */ \
	(windows_1252,     8), \
	/** EBCDIC encoding. Only supported in detection and handling, but not as internal string
	 * representation. */ \
	(ebcdic,           9), \
	/** UTF-16 encoding (host endianness). */ \
	(utf16_host,       (ABC_HOST_LITTLE_ENDIAN ? utf16le : utf16be)), \
	/** UTF-32 encoding (host endianness). */ \
	(utf32_host,       (ABC_HOST_LITTLE_ENDIAN ? utf32le : utf32be)), \
	/** Default host encoding. */ \
	(host,             (ABC_HOST_UTF == 8 ? utf8 : (ABC_HOST_UTF == 16 ? utf16_host : utf32_host))) \
);

/// Recognized line terminators.
ABC_ENUM(line_terminator, \
	/** Unknown/undetermined line terminator. */ \
	(unknown,       0), \
	/** Offset of the first known line terminator. */ \
	(_known_offset, 1), \
	/** Old Mac style: Carriage Return, '\r'. */ \
	(cr,            1), \
	/** Unix/POSIX style: Line Feed, '\n'. */ \
	(lf,            2), \
	/** DOS/Windows style: Carriage Return + Line Feed, '\r', '\n'. */ \
	(cr_lf,         3), \
	/** EBCDIC style: Next Line, '\x15'. */ \
	(nel,           4), \
	/** Default host line terminator. */ \
	(host,          (ABC_HOST_API_WIN32 ? cr_lf : lf)) \
);

/// Character size, in bytes, for each recognized encoding.
extern uint8_t const gc_cbEncChar[];

/// This can be used by any char32_t-returning function that needs to return a value that’s
// obviously not a char32_t value.
char32_t const invalid_char(~char32_t(0));

/// This must be used to replace any invalid char32_t value.
char32_t const replacement_char(0x00fffd);

/// Maximum run length for the encoding of a code point, in any encoding.
// Technically, 6 is an illegal UTF-8 run, but it’s possible due to the way bits are encoded, so
// it’s here.
size_t const max_codepoint_length(6);


/// Prototype of a void version of str_str().
typedef void const * (* str_str_fn)(
	void const * pchHaystackBegin, void const * pchHaystackEnd,
	void const * pchNeedleBegin, void const * pchNeedleEnd
);


/// Provides an estimate of the space, in bytes, necessary to store a string, transcoded in a
// different encoding. For example, transcoding from UTF-32 to UTF-16 will yield half the source
// size, although special cases such as surrogates might make the estimate too low.
size_t estimate_transcoded_size(encoding encSrc, void const * pSrc, size_t cbSrc, encoding encDst);

/// Returns the character size, in bytes, for the specified charset encoding, or 0 for non-charset
// encodings (e.g. identity_encoding).
/*constexpr*/ size_t get_encoding_size(encoding enc);

/// Returns a byte sequence representing a line terminator in the requested encoding.
void const * get_line_terminator_bytes(encoding enc, line_terminator lterm, size_t * pcb);

/// Tries to guess the encoding of a sequence of bytes, optionally also taking into account the
// total number of bytes in the source of which the buffer is the beginning.
//
// While this function can check for validity of some encodings, it does not guarantee that, for
// example, for a return value of utf8_encoding utf8_traits::is_valid() will return true for the
// same buffer.
encoding guess_encoding(
	void const * pBuf, size_t cbBuf, size_t cbSrcTotal = 0, size_t * pcbBom = NULL
);

/// Tries to guess the line terminators employed by a sequence of bytes, interpreted according to a
// specified encoding. The second argument is really character count, it’s not a typo; the size of
// each character is inferred via enc.
line_terminator guess_line_terminator(void const * pBuf, size_t cchBuf, encoding enc);

/// Converts from one character encoding to another. All pointed-by variables are updated to discard
// the bytes used in the conversion; the number of bytes written is returned.
// UTF validity: not necessary; invalid sequences are replaced with text::replacement_char.
size_t transcode(
	std::nothrow_t const &,
	encoding encSrc, void const ** ppSrc, size_t * pcbSrc,
	encoding encDst, void       ** ppDst, size_t * pcbDstMax
);

} //namespace text

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text globals

namespace abc {

namespace text {

inline /*constexpr*/ size_t get_encoding_size(encoding enc) {
	return gc_cbEncChar[enc.base()];
}

} //namespace text

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_TEXT_HXX

