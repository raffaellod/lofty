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

#ifndef ABC_IOSTREAM_HXX
#define ABC_IOSTREAM_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/file.hxx>
#include <abc/to_string_backend.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::stream_base


namespace abc {

/** Base for abstract data streams.
*/
class stream_base {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	stream_base();


	/** Destructor.
	*/
	virtual ~stream_base();


	/** Returns the encoding of the data read from or written to this stream.

	TODO: comment signature.
	*/
	text::encoding get_encoding() const {
		return m_enc;
	}


	/** Returns the line terminator of the text read from or written to this stream.

	TODO: comment signature.
	*/
	text::line_terminator get_line_terminator() const {
		return m_lterm;
	}


	/** Sets the encoding of the data read from or written to this stream.

	TODO: comment signature.
	*/
	virtual void set_encoding(text::encoding enc);


	/** Sets the line terminator to be assumed for the text read from this stream, that to be used
	when writing to it.

	TODO: comment signature.
	*/
	virtual void set_line_terminator(text::line_terminator lterm);


protected:

	/** Encoding of the data read from or written to this stream. If not explicitly set, it will be
	automatically determined as soon as enough bytes are read or written. */
	text::encoding m_enc;
	/** Line terminator used for line-oriented reads from or writes to this stream. If not explicitly
	set, it will be automatically determined as soon as enough bytes are read or written. */
	text::line_terminator m_lterm;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::istream


namespace abc {

/** Read-only abstract stream.
*/
class istream :
	public virtual stream_base,
	public support_explicit_operator_bool<istream> {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	istream() :
		stream_base() {
	}


	/** Destructor.

	TODO: comment signature.
	*/
	virtual ~istream();


	/** An istream at EOF evaulates to false; true otherwise.

	TODO: comment signature.
	*/
	explicit_operator_bool() const {
		return !is_at_end();
	}


	/** Returns true if the istream has reached the end of the data, or false otherwise.

	TODO: comment signature.
	*/
	virtual bool is_at_end() const = 0;


	/** Reads at most cbMax bytes from the stream into the specified buffer.

	TODO: comment signature.
	*/
	virtual size_t read(void * p, size_t cbMax, text::encoding enc = text::encoding::identity) = 0;


	/** Reads a whole line in the provided mstr_, discarding any line termination characters read.

	TODO: comment signature.
	*/
	template <typename C, class TTraits>
	istream & read_line(mstr_<C, TTraits> * ps, text::encoding enc = TTraits::host_encoding) {
		_read_line(
			ps->get_raw(), enc,
			TTraits::max_codepoint_length, reinterpret_cast<text::str_str_fn>(TTraits::str_str)
		);
		return *this;
	}


	/** Pretends to undo the reads of cbMax bytes, which must be provided in the specified buffer.

	TODO: comment signature.
	*/
	virtual void unread(
		void const * p, size_t cb, text::encoding enc = text::encoding::identity
	) = 0;


private:

	/** Implementation of read_line(): reads a whole line in the provided string, discarding the line
	terminator read (if any) and appending a NUL character.

	TODO: comment signature.
	*/
	virtual void _read_line(
		_raw_str & rs, text::encoding enc, unsigned cchCodePointMax, text::str_str_fn pfnStrStr
	) = 0;
};

} //namespace abc


/** Extraction operator for abc::istream.

TODO: comment signature.
*/
template <typename C, class TTraits>
inline abc::istream & operator>>(abc::istream & is, abc::mstr_<C, TTraits> & s) {
	return is.read_line(&s);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::ostream


namespace abc {

/** DOC:7103 abc::ostream::print()

Designed after Python’s str.format(), abc::ostream::print() allows to combine objects together as
strings using a format string.

The implementation of print() is entirely contained in abc::_ostream_print_helper, which accesses
the individual arguments in a recursive way, from the most-derived class down to the base class,
which also contains most of the implementation. Combined with the usage of [DOC:3984
abc::to_str()], this enables a type-safe variadic alternative to C’s printf, and voids the
requirement for explicit specification of the argumment types (such as %d, %s), much like Python’s
str.format().

Because of its type-safety, print() is also the core of [DOC:8503 Stack tracing], because it allows
to print a variable by automatically deducing its type.

The format string passed as first argument to abc::ostream::print() can contain “replacement fields”
delimited by curly braces (‘{’ and ‘}’). Anything not contained in curly braces is considered
literal text and emitted as-is; the only exceptions are the substrings “{{” and “}}”, which allow to
print “{” and “}” respectively.

A replacement field can specify an argument index; if omitted, the argument used will be the one
following the last used one, or the first if no arguments have been used up to that point. After the
optional argument index, a conversion might be requested (TODO), and an optional type-dependent
format specification can be indicated; this will be passed as-is to the specialization of
abc::to_str_backend for the selected argument.

Grammar for a replacement field:

	replacement_field	: “{” index? ( “!” conversion )? ( “:” format_spec )? “}”
	index					: [0-9]+
	conversion			: [ars]
	format_spec			: <type-specific format specification>

Basic usage examples for index:

	"Welcome to {0}"						Use argument 0
	"Please see items {}, {3}, {}"	Use argument 0, skip 1 and 2, use 3 and 4

Reference for Python’s str.format(): <http://docs.python.org/3/library/string.html#format-string-
syntax>
*/

/** Write-only abstract stream.
*/
class ostream :
	public virtual stream_base {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	ostream() :
		stream_base() {
	}


	/** Destructor.
	*/
	virtual ~ostream();


	/** Ensures that any write buffers are written to the stream. The default implementation is a
	no-op.

	TODO: comment signature.
	*/
	virtual void flush();


	/** Writes multiple values combined together in the specified format.

	TODO: comment signature.
	*/
	template <typename ... Ts>
	void print(istr const & sFormat, Ts const & ... ts);


	/** Writes an array of bytes to the stream, translating them to the file’s character encoding
	first, if necessary.

	TODO: comment signature.
	*/
	virtual void write(void const * p, size_t cb, text::encoding enc = text::encoding::identity) = 0;
};

} //namespace abc


/** Insertion operator for abc::ostream.

TODO: comment signature.
*/
template <typename T>
inline abc::ostream & operator<<(abc::ostream & os, T const & t) {
	abc::to_str_backend<T> tsb;
	tsb.write(t, &os);
	return os;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_ostream_print_helper


namespace abc {

/** Helper for ostream::print().
*/
template <typename ... Ts>
class _ostream_print_helper;

// Base recursion step: no arguments to replace.
template <>
class _ostream_print_helper<> {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	_ostream_print_helper(ostream * pos, istr const & sFormat);


	/** Writes the provided arguments to the target ostream, performing replacements as necessary.

	TODO: comment signature.
	*/
	void run();


protected:

	/** Writes the portion of format string between m_itFormatToWriteBegin and the next replacement
	and returns true, or writes the remaining characters of the format string and returns false if no
	more replacement are found.

	TODO: comment signature.
	*/
	bool write_format_up_to_next_repl();


	/** Writes T0 if iArg == 0, or fowards the call to the previous recursion level.

	TODO: comment signature.
	*/
	ABC_FUNC_NORETURN void write_repl(unsigned iArg);


private:

	/** Throws an instance of abc::syntax_error(), providing accurate context information.

	TODO: comment signature.
	*/
	ABC_FUNC_NORETURN void throw_syntax_error(
		istr const & sDescription, istr::const_iterator it
	) const;


	/** Writes the portion of format string between the first character to be written
	(m_itFormatToWriteBegin) and the specified one, and updates m_itFormatToWriteBegin.

	TODO: comment signature.
	*/
	void write_format_up_to(istr::const_iterator itUpTo);


protected:

	/** Target ostream. Needs to be a pointer because to_str_backend::write() requires a pointer. */
	ostream * m_pos;
	/** Start of the format specification of the current replacement. */
	char_t const * m_pchReplFormatSpecBegin;
	/** End of the format specification of the current replacement. */
	char_t const * m_pchReplFormatSpecEnd;
	/** 0-based index of the argument to replace the next replacement. */
	unsigned m_iSubstArg;


private:

	/** Format string. */
	istr const & m_sFormat;
	/** First format string character to be written yet. */
	istr::const_iterator m_itFormatToWriteBegin;
};

// Recursion step: extract one argument, recurse with the rest.
template <typename T0, typename ... Ts>
class _ostream_print_helper<T0, Ts ...> :
	public _ostream_print_helper<Ts ...> {

	typedef _ostream_print_helper<Ts ...> osph_base;

public:

	/** Constructor.

	TODO: comment signature.
	*/
	_ostream_print_helper(ostream * pos, istr const & sFormat, T0 const & t0, Ts const & ... ts) :
		osph_base(pos, sFormat, ts ...),
		m_t0(t0) {
	}


	/** See _ostream_print_helper<>::run().

	TODO: comment signature.
	*/
	void run() {
		while (osph_base::write_format_up_to_next_repl()) {
			// Perform and write the replacement.
			write_repl(osph_base::m_iSubstArg);
		}
	}


protected:

	/** Writes T0 if iArg == 0, or fowards the call to the previous recursion level.

	TODO: comment signature.
	*/
	void write_repl(unsigned iArg) {
		if (iArg == 0) {
			to_str_backend<T0> tsb(char_range(
				osph_base::m_pchReplFormatSpecBegin, osph_base::m_pchReplFormatSpecEnd
			));
			tsb.write(m_t0, osph_base::m_pos);
		} else {
			// Recurse to the previous level.
			osph_base::write_repl(iArg - 1);
		}
	}


private:

	/** Nth replacement. */
	T0 const & m_t0;
};


// Now it’s possible to implement this.
template <typename ... Ts>
inline void ostream::print(istr const & sFormat, Ts const & ... ts) {
	_ostream_print_helper<Ts ...> osph(this, sFormat, ts ...);
	osph.run();
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::iostream


namespace abc {

/** Read/write abstract stream.
*/
class iostream :
	public virtual istream,
	public virtual ostream {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	iostream();


	/** Destructor.
	*/
	virtual ~iostream();
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_IOSTREAM_HXX

