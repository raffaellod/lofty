// -*- coding: utf-8; mode: c++; tab-width: 3 -*-
//--------------------------------------------------------------------------------------------------
// Application-Building Components
// Copyright 2010-2013 Raffaello D. Di Napoli
//--------------------------------------------------------------------------------------------------
// This file is part of Application-Building Components (henceforth referred to as ABC).
//
// ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ABC. If not, see
// <http://www.gnu.org/licenses/>.
//--------------------------------------------------------------------------------------------------

#include <abc/iostream.hxx>
#include <abc/trace.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::stream_base


namespace abc {

stream_base::stream_base() :
	m_enc(text::encoding::unknown),
	m_lterm(text::line_terminator::unknown) {
}


/*virtual*/ stream_base::~stream_base() {
}


/*virtual*/ void stream_base::set_encoding(text::encoding enc) {
	m_enc = enc;
}


/*virtual*/ void stream_base::set_line_terminator(text::line_terminator lterm) {
	m_lterm = lterm;
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::istream


namespace abc {

/*virtual*/ istream::~istream() {
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::ostream


namespace abc {

/*virtual*/ ostream::~ostream() {
}


/*virtual*/ void ostream::flush() {
}


_ostream_print_helper<>::_ostream_print_helper(ostream * pos, cstring const & sFormat) :
	m_pos(pos),
	// write_format_up_to_next_repl() will increment this to 0 or set it to a non-negative number.
	m_iSubstArg(unsigned(-1)),
	m_sFormat(sFormat),
	m_itFormatToWriteBegin(sFormat.cbegin()) {
}


void _ostream_print_helper<>::run() {
	// Since this specialization has no replacements, verify that the format string doesn’t specify
	// any either.
	if (write_format_up_to_next_repl())
		abc_throw(index_error(m_iSubstArg));
}


void _ostream_print_helper<>::throw_syntax_error(
	cstring const & sDescription, cstring::const_iterator it
) const {
	// +1 because the first character is 1, to human beings.
	abc_throw(syntax_error(sDescription, m_sFormat, unsigned(it - m_sFormat.cbegin() + 1)));
}


void _ostream_print_helper<>::write_format_up_to(cstring::const_iterator itUpTo) {
	abc_trace_fn((this/*, itUpTo*/));

	if (itUpTo > m_itFormatToWriteBegin) {
		m_pos->write(
			m_itFormatToWriteBegin.base(),
			sizeof(char_t) * size_t(itUpTo - m_itFormatToWriteBegin),
			text::utf_traits<>::host_encoding
		);
		m_itFormatToWriteBegin = itUpTo;
	}
}


bool _ostream_print_helper<>::write_format_up_to_next_repl() {
	abc_trace_fn((this));

	// Search for the next replacement, if any.
	cstring::const_iterator it(m_itFormatToWriteBegin), itReplFieldBegin, itEnd(m_sFormat.cend());
	char_t ch;
	for (;;) {
		if (it >= itEnd) {
			// The format string is over; write any characters not yet written.
			write_format_up_to(itEnd);
			// Report that no more replacement fields were found.
			return false;
		}
		ch = *it++;
		if (ch == '{' || ch == '}') {
			if (ch == '{') {
				// Mark the beginning of the replacement field.
				itReplFieldBegin = it - 1;
				if (it >= itEnd)
					throw_syntax_error(SL("unmatched '{' in format string"), itReplFieldBegin);
				ch = *it;
				if (ch != '{')
					// We found the beginning of a replacement field.
					break;
			} else if (ch == '}') {
				if (it >= itEnd || *it != '}')
					throw_syntax_error(SL("single '}' encountered in format string"), it - 1);
			}
			// Convert “{{” into “{” or “}}” into “}”.
			// Write up to and including the first brace.
			write_format_up_to(it);
			// The next call to write_format_up_to() will skip the second brace.
			m_itFormatToWriteBegin = ++it;
		}
	}

	// Check if we have an argument index.
	if (ch >= '0' && ch <= '9') {
		// Consume as many digits as there are, and convert them into the argument index.
		unsigned iArg(0);
		do {
			iArg *= 10;
			iArg += unsigned(ch - '0');
		} while (++it < itEnd && (ch = *it, ch >= '0' && ch <= '9'));
		if (it >= itEnd)
			throw_syntax_error(SL("unmatched '{' in format string"), itReplFieldBegin);
		// Save this index as the last used one.
		m_iSubstArg = iArg;
	} else
		// The argument index is missing, so just use the next one.
		++m_iSubstArg;

	// Check for a conversion specifier; defaults to string.
	char_t chConversion('s');
	if (ch == '!') {
		if (++it >= itEnd)
			throw_syntax_error(SL("expected conversion specifier"), it);
		ch = *it;
		switch (ch) {
			case 's':
// TODO	case 'r':
// TODO	case 'a':
				chConversion = ch;
				UNUSED_ARG(chConversion);
				break;
			default:
				throw_syntax_error(SL("unknown conversion specifier"), it);
		}
		if (++it >= itEnd)
			throw_syntax_error(SL("unmatched '{' in format string"), itReplFieldBegin);
		ch = *it;
	}

	// Check for a format specification.
	if (ch == ':') {
		if (++it >= itEnd)
			throw_syntax_error(SL("expected format specification"), it);
		m_pchReplFormatSpecBegin = it.base();
		// Find the end of the replacement field.
		it = m_sFormat.find('}', it);
		if (it == m_sFormat.cend())
			throw_syntax_error(SL("unmatched '{' in format string"), itReplFieldBegin);
		m_pchReplFormatSpecEnd = it.base();
	} else {
		// If there’s no format specification, it must be the end of the replacement field.
		if (ch != '}')
			throw_syntax_error(SL("unmatched '{' in format string"), itReplFieldBegin);
		// Set the format specification to nothing.
		m_pchReplFormatSpecBegin = NULL;
		m_pchReplFormatSpecEnd = NULL;
	}

	// Write the format string characters up to the beginning of the replacement.
	write_format_up_to(itReplFieldBegin);
	// Update this, so the next call to write_format_up_to() will skip over this replacement field.
	m_itFormatToWriteBegin = it + 1 /*'}'*/;
	// Report that a substitution must be written.
	return true;
}


void _ostream_print_helper<>::write_repl(unsigned iArg) {
	// This is the last recursion stage, with no replacements available, so if we got here
	// ostream::print() was called with insufficient replacements for the given format string.
	UNUSED_ARG(iArg);
	abc_throw(index_error(m_iSubstArg));
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::iostream


namespace abc {

iostream::iostream() :
	stream_base(),
	istream(),
	ostream() {
}


/*virtual*/ iostream::~iostream() {
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

