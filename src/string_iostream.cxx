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

#include <abc/string_iostream.hxx>
#include <abc/trace.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::str_istream


namespace abc {

str_istream::str_istream(istr const & s) :
	istream(),
	m_sBuf(s),
	m_ibRead(0) {
}
str_istream::str_istream(istr && s) :
	istream(),
	m_sBuf(std::move(s)),
	m_ibRead(0) {
}
str_istream::str_istream(mstr && s) :
	istream(),
	m_sBuf(std::move(s)),
	m_ibRead(0) {
}
str_istream::str_istream(dmstr && s) :
	istream(),
	m_sBuf(std::move(s)),
	m_ibRead(0) {
}


/*virtual*/ str_istream::~str_istream() {
}


size_t str_istream::read(
	void * p, size_t cbMax, text::encoding enc /*= text::encoding::identity*/
) {
	UNUSED_ARG(p);
	UNUSED_ARG(cbMax);
	UNUSED_ARG(enc);
	return 0;
}


/*virtual*/ void str_istream::unread(void const * p, size_t cb, text::encoding enc) {
	UNUSED_ARG(p);
	UNUSED_ARG(cb);
	UNUSED_ARG(enc);
}


/*virtual*/ void str_istream::_read_line(
	_raw_str & rs, text::encoding enc, unsigned cchCodePointMax, text::str_str_fn pfnStrStr
) {
	UNUSED_ARG(rs);
	UNUSED_ARG(enc);
	UNUSED_ARG(cchCodePointMax);
	UNUSED_ARG(pfnStrStr);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::str_ostream


namespace abc {

str_ostream::str_ostream() :
	ostream(),
	m_ibWrite(0) {
}


/*virtual*/ str_ostream::~str_ostream() {
}


str_ostream::string_type str_ostream::get_contents() {
	return std::move(m_sBuf);
}


/*virtual*/ void str_ostream::write(
	void const * p, size_t cb, text::encoding enc /*= text::encoding::identity*/
) {
	abc_trace_fn((this, p, cb, enc));

	if (enc == text::encoding::unknown) {
		// Treat unknown as identity.
		enc = text::encoding::identity;
	}
	if (m_enc == text::encoding::unknown) {
		// This is the first output, so it decides for the whole file.
		m_enc = enc;
	}
	if (!cb) {
		// Nothing to do.
		return;
	}
	size_t cbChar(sizeof(string_type::value_type));
	if (enc == m_enc || enc == text::encoding::identity) {
		// Optimal case: no transcoding necessary.
		// Enlarge the string as necessary, then overwrite any character in the affected range.
		m_sBuf.set_capacity((m_ibWrite + cb) / cbChar, true);
		memory::copy<void>(reinterpret_cast<int8_t *>(m_sBuf.get_data()) + m_ibWrite, p, cb);
		m_ibWrite += cb;
	} else {
		do {
			// Calculate the additional size required, and enlarge the string.
			size_t cbDstEst(text::estimate_transcoded_size(enc, p, cb, m_enc));
			m_sBuf.set_capacity((m_ibWrite + cbDstEst) / cbChar, true);
			// Get the resulting buffer and its actual size.
			void * pBuf(reinterpret_cast<int8_t *>(m_sBuf.get_data()) + m_ibWrite);
			size_t cbBuf(cbChar * m_sBuf.get_capacity() - m_ibWrite);
			// Fill as much of the buffer as possible, and advance m_ibWrite accordingly.
			m_ibWrite += text::transcode(std::nothrow, enc, &p, &cb, m_enc, &pBuf, &cbBuf);
		} while (cb);
	}
	// Ensure the string knows its own length and is NUL-terminated.
	m_sBuf.set_size(m_ibWrite / cbChar);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

