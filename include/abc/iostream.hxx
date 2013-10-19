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

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/file.hxx>
#include <abc/to_str_backend.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::stream_base


namespace abc {

/** Base for abstract data streams.
*/
class ABCAPI stream_base {
public:

	/** Constructor.
	*/
	stream_base();


	/** Destructor.
	*/
	virtual ~stream_base();


	/** Returns the encoding of the data read from or written to the stream.

	return
		Current encoding.
	*/
	text::encoding get_encoding() const {
		return m_enc;
	}


	/** Returns the line terminator of the text read from or written to the stream.

	return
		Current line terminator.
	*/
	text::line_terminator get_line_terminator() const {
		return m_lterm;
	}


	/** Sets the encoding of the data read from or written to the stream.

	enc
		New encoding to be used for future data reads/writes.
	*/
	virtual void set_encoding(text::encoding enc);


	/** Sets the line terminator to be assumed for the text read from this stream, that to be used
	when writing to it.

	lterm
		New line terminator to be used for future data reads/writes.
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
class ABCAPI istream :
	public virtual stream_base,
	public support_explicit_operator_bool<istream> {
public:

	/** Constructor.
	*/
	istream() :
		stream_base() {
	}


	/** Destructor.
	*/
	virtual ~istream();


	/** Returns whether the stream has more data to be read.

	return
		false if at_end() would return true, or true otherwise.
	*/
	explicit_operator_bool() const {
		return !at_end();
	}


	/** Returns true if the stream has reached the end of the data.

	return
		true if the stream contains no more data to read, or false otherwise.
	*/
	virtual bool at_end() const = 0;


	/** Reads a whole line in the provided mutable string, discarding any line termination characters
	read.

	ps
		Pointer to the string that will receive the line read.
	enc
		Encoding used by the string pointed to by prs. If not the same as stream’s encoding, a
		conversion will be performed.
	return
		*this.
	*/
	template <typename C, class TTraits>
	istream & read_line(mstr_<C, TTraits> * ps, text::encoding enc = TTraits::host_encoding) {
		_read_line(
			&ps->raw(), enc,
			TTraits::max_codepoint_length, reinterpret_cast<text::str_str_fn>(TTraits::str_str)
		);
		return *this;
	}


	/** Reads at most cbMax bytes from the stream into the specified buffer.

	p
		Pointer to the buffer to read the data into.
	cbMax
		Capacity of the buffer pointed to by p, in bytes.
	enc
		Encoding used by the buffer pointed to by p; if not the same as the stream’s encoding, a
		conversion will be performed.
	*/
	virtual size_t read_raw(
		void * p, size_t cbMax, text::encoding enc = text::encoding::identity
	) = 0;


	/** Pretends to undo the reads of cbMax bytes, which must be provided in the specified buffer.

	p
		Pointer to the buffer to unread (write back to the read buffer).
	cb
		Size of the buffer pointed to by p, in bytes.
	enc
		Encoding used by the buffer pointed to by p; if not the same as the stream’s encoding, a
		conversion will be performed.
	*/
	virtual void unread_raw(
		void const * p, size_t cb, text::encoding enc = text::encoding::identity
	) = 0;


private:

	/** Implementation of read_line(): reads a whole line in the provided string, discarding the line
	terminator read (if any) and appending a NUL character.

	prs
		Pointer to the string that will receive the line read.
	enc
		Encoding used by the string pointed to by prs. If not the same as stream’s encoding, a
		conversion will be performed.
	cchCodePointMax
		Maximum size, in *prs characters, of a single Unicode code point. Used to calculate buffer
		sizes.
	pfnStrStr
		Pointer to a substring-search function suitable for the character type of *prs.
	*/
	virtual void _read_line(
		_raw_str * prs, text::encoding enc, unsigned cchCodePointMax, text::str_str_fn pfnStrStr
	) = 0;
};

} //namespace abc


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
class ABCAPI ostream :
	public virtual stream_base {
public:

	/** Constructor.
	*/
	ostream() :
		stream_base() {
	}


	/** Destructor.
	*/
	virtual ~ostream();


	/** Ensures that any write buffers are written to the stream. The default implementation is a
	no-op.
	*/
	virtual void flush();


	/** Writes multiple values combined together in the specified format.

	sFormat
		Format string to parse for replacements.
	ts
		Replacement values.
	return
		*this.
	*/
#ifdef ABC_CXX_VARIADIC_TEMPLATES

	template <typename ... Ts>
	ostream & print(istr const & sFormat, Ts const & ... ts);

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

	ostream & print(istr const & sFormat);
	template <typename T0>
	ostream & print(istr const & sFormat, T0 const & t0);
	template <typename T0, typename T1>
	ostream & print(istr const & sFormat, T0 const & t0, T1 const & t1);
	template <typename T0, typename T1, typename T2>
	ostream & print(istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2);
	template <typename T0, typename T1, typename T2, typename T3>
	ostream & print(
		istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3
	);
	template <typename T0, typename T1, typename T2, typename T3, typename T4>
	ostream & print(
		istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3,
		T4 const & t4
	);
	template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
	ostream & print(
		istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3,
		T4 const & t4, T5 const & t5
	);
	template <
		typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
	>
	ostream & print(
		istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3,
		T4 const & t4, T5 const & t5, T6 const & t6
	);
	template <
		typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
		typename T7
	>
	ostream & print(
		istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3,
		T4 const & t4, T5 const & t5, T6 const & t6, T7 const & t7
	);
	template <
		typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
		typename T7, typename T8
	>
	ostream & print(
		istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3,
		T4 const & t4, T5 const & t5, T6 const & t6, T7 const & t7, T8 const & t8
	);
	template <
		typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
		typename T7, typename T8, typename T9
	>
	ostream & print(
		istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3,
		T4 const & t4, T5 const & t5, T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
	);

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else


	/** Writes a value to the stream using the default formatting for abc::to_str_backend().

	t
		Value to write.
	return
		*this.
	*/
	template <typename T>
	ostream & write(T const & t) {
		to_str_backend<T> tsb;
		tsb.write(t, this);
		return *this;
	}


	/** Writes an array of bytes to the stream, translating them to the file’s character encoding
	first, if necessary.

	p
		Pointer to the buffer to write.
	cb
		Size of the buffer pointed to by p, in bytes.
	enc
		Encoding used by the buffer pointed to by p; if not the same as the stream’s encoding, a
		conversion will be performed.
	*/
	virtual void write_raw(
		void const * p, size_t cb, text::encoding enc = text::encoding::identity
	) = 0;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_ostream_print_helper


namespace abc {

/** Template-free implementation of abc::_ostream_print_helper.
*/
class ABCAPI _ostream_print_helper_impl {
public:

	/** Constructor.

	pos
		Stream to write to.
	sFormat
		Format string to parse for replacements.
	*/
	_ostream_print_helper_impl(ostream * pos, istr const & sFormat);


	/** Writes the provided arguments to the target ostream, performing replacements as necessary.
	*/
	void run();


protected:

	/** Throws an instance of abc::index_error(), providing the invalid replacement index found in
	the format string.
	*/
	ABC_FUNC_NORETURN void throw_index_error();


	/** Writes the portion of format string between m_itFormatToWriteBegin and the next replacement
	and returns true, or writes the remaining characters of the format string and returns false if no
	more replacement are found.

	return
		true if another replacement was found and should be printed, or false otherwise.
	*/
	bool write_format_up_to_next_repl();


private:

	/** Throws an instance of abc::syntax_error(), providing accurate context information.

	sDescription
		Error description.
	it
		Position of the offending character in m_sFormat.
	*/
	ABC_FUNC_NORETURN void throw_syntax_error(
		istr const & sDescription, istr::const_iterator it
	) const;


	/** Writes the portion of format string between the first character to be written
	(m_itFormatToWriteBegin) and the specified one, and updates m_itFormatToWriteBegin.

	itUpTo
		First character not to be written.
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


/** Helper for/implementation of abc::ostream::print().
*/
#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename ... Ts>
class _ostream_print_helper;

// Base recursion step: no arguments to replace.
template <>
class _ostream_print_helper<> :
	public _ostream_print_helper_impl {
public:

	/** Constructor.

	pos
		Stream to write to.
	sFormat
		Format string to parse for replacements.
	*/
	_ostream_print_helper(ostream * pos, istr const & sFormat) :
		_ostream_print_helper_impl(pos, sFormat) {
	}


protected:

	/** Writes T0 if iArg == 0, or fowards the call to the previous recursion level.

	iArg
		0-based index of the template argument to write.
	*/
	ABC_FUNC_NORETURN void write_repl(unsigned iArg) {
		// This is the last recursion stage, with no replacements available, so if we got here
		// ostream::print() was called with insufficient replacements for the given format string.
		UNUSED_ARG(iArg);
		_ostream_print_helper_impl::throw_index_error();
	}
};

// Recursion step: extract one argument, recurse with the rest.
template <typename T0, typename ... Ts>
class _ostream_print_helper<T0, Ts ...> :
	public _ostream_print_helper<Ts ...> {

	typedef _ostream_print_helper<Ts ...> osph_base;

public:

	/** Constructor.

	pos
		Stream to write to.
	sFormat
		Format string to parse for replacements.
	t0
		First replacement value.
	ts
		Remaining replacement values.
	*/
	_ostream_print_helper(ostream * pos, istr const & sFormat, T0 const & t0, Ts const & ... ts) :
		osph_base(pos, sFormat, ts ...),
		m_t0(t0) {
	}


	/** See _ostream_print_helper<>::run().
	*/
	void run() {
		while (osph_base::write_format_up_to_next_repl()) {
			// Perform and write the replacement.
			write_repl(osph_base::m_iSubstArg);
		}
	}


protected:

	/** See _ostream_print_helper<>::write_repl().
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

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

// Recursion step: extract one argument, recurse with the rest.
template <
	typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void,
	typename T4 = void, typename T5 = void, typename T6 = void, typename T7 = void,
	typename T8 = void, typename T9 = void
>
class _ostream_print_helper :
	public _ostream_print_helper<T1, T2, T3, T4, T5, T6, T7, T8, T9> {

	typedef _ostream_print_helper<T1, T2, T3, T4, T5, T6, T7, T8, T9> osph_base;

public:

	/** Constructor.

	pos
		Stream to write to.
	sFormat
		Format string to parse for replacements.
	t0
		First replacement value.
	t1
		Second replacement value.
	t2
		Third replacement value.
	t3
		Fourth replacement value.
	t4
		Fifth replacement value.
	t5
		Sixth replacement value.
	t6
		Seventh replacement value.
	t7
		Eighth replacement value.
	t8
		Ninth replacement value.
	t9
		Tenth replacement value.
	*/
	template <typename U0>
	_ostream_print_helper(
		typename std::enable_if<!std::is_void<U0>::value, ostream *>::type pos, istr const & sFormat,
		U0 const & t0
	) :
		osph_base(pos, sFormat),
		m_t0(t0) {
	}
	template <typename U0, typename U1>
	_ostream_print_helper(
		typename std::enable_if<!std::is_void<U0>::value, ostream *>::type pos, istr const & sFormat,
		U0 const & t0, U1 const & t1
	) :
		osph_base(pos, sFormat, t1),
		m_t0(t0) {
	}
	template <typename U0, typename U1, typename U2>
	_ostream_print_helper(
		typename std::enable_if<!std::is_void<U0>::value, ostream *>::type pos, istr const & sFormat,
		U0 const & t0, U1 const & t1, U2 const & t2
	) :
		osph_base(pos, sFormat, t1, t2),
		m_t0(t0) {
	}
	template <typename U0, typename U1, typename U2, typename U3>
	_ostream_print_helper(
		typename std::enable_if<!std::is_void<U0>::value, ostream *>::type pos, istr const & sFormat,
		U0 const & t0, U1 const & t1, U2 const & t2, U3 const & t3
	) :
		osph_base(pos, sFormat, t1, t2, t3),
		m_t0(t0) {
	}
	template <typename U0, typename U1, typename U2, typename U3, typename U4>
	_ostream_print_helper(
		typename std::enable_if<!std::is_void<U0>::value, ostream *>::type pos, istr const & sFormat,
		U0 const & t0, U1 const & t1, U2 const & t2, U3 const & t3, U4 const & t4
	) :
		osph_base(pos, sFormat, t1, t2, t3, t4),
		m_t0(t0) {
	}
	template <typename U0, typename U1, typename U2, typename U3, typename U4, typename U5>
	_ostream_print_helper(
		typename std::enable_if<!std::is_void<U0>::value, ostream *>::type pos, istr const & sFormat,
		U0 const & t0, U1 const & t1, U2 const & t2, U3 const & t3, U4 const & t4, U5 const & t5
	) :
		osph_base(pos, sFormat, t1, t2, t3, t4, t5),
		m_t0(t0) {
	}
	template <
		typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6
	>
	_ostream_print_helper(
		typename std::enable_if<!std::is_void<U0>::value, ostream *>::type pos, istr const & sFormat,
		U0 const & t0, U1 const & t1, U2 const & t2, U3 const & t3, U4 const & t4, U5 const & t5,
		U6 const & t6
	) :
		osph_base(pos, sFormat, t1, t2, t3, t4, t5, t6),
		m_t0(t0) {
	}
	template <
		typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
		typename U7
	>
	_ostream_print_helper(
		typename std::enable_if<!std::is_void<U0>::value, ostream *>::type pos, istr const & sFormat,
		U0 const & t0, U1 const & t1, U2 const & t2, U3 const & t3, U4 const & t4, U5 const & t5,
		U6 const & t6, U7 const & t7
	) :
		osph_base(pos, sFormat, t1, t2, t3, t4, t5, t6, t7),
		m_t0(t0) {
	}
	template <
		typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
		typename U7, typename U8
	>
	_ostream_print_helper(
		typename std::enable_if<!std::is_void<U0>::value, ostream *>::type pos, istr const & sFormat,
		U0 const & t0, U1 const & t1, U2 const & t2, U3 const & t3, U4 const & t4, U5 const & t5,
		U6 const & t6, U7 const & t7, U8 const & t8
	) :
		osph_base(pos, sFormat, t1, t2, t3, t4, t5, t6, t7, t8),
		m_t0(t0) {
	}
	template <
		typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
		typename U7, typename U8, typename U9
	>
	_ostream_print_helper(
		typename std::enable_if<!std::is_void<U0>::value, ostream *>::type pos, istr const & sFormat,
		U0 const & t0, U1 const & t1, U2 const & t2, U3 const & t3, U4 const & t4, U5 const & t5,
		U6 const & t6, U7 const & t7, U8 const & t8, U9 const & t9
	) :
		osph_base(pos, sFormat, t1, t2, t3, t4, t5, t6, t7, t8, t9),
		m_t0(t0) {
	}


	/** See _ostream_print_helper<>::run().
	*/
	void run() {
		while (osph_base::write_format_up_to_next_repl()) {
			// Perform and write the replacement.
			write_repl(osph_base::m_iSubstArg);
		}
	}


protected:

	/** See _ostream_print_helper<>::write_repl().
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

// Base recursion step: no arguments to replace.
template <>
class _ostream_print_helper<> :
	public _ostream_print_helper_impl {
public:

	/** Constructor.

	pos
		Stream to write to.
	sFormat
		Format string to parse for replacements.
	*/
	_ostream_print_helper(ostream * pos, istr const & sFormat) :
		_ostream_print_helper_impl(pos, sFormat) {
	}


protected:

	/** Writes T0 if iArg == 0, or fowards the call to the previous recursion level.

	iArg
		0-based index of the template argument to write.
	*/
	ABC_FUNC_NORETURN void write_repl(unsigned iArg) {
		// This is the last recursion stage, with no replacements available, so if we got here
		// ostream::print() was called with insufficient replacements for the given format string.
		UNUSED_ARG(iArg);
		_ostream_print_helper_impl::throw_index_error();
	}
};

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else


// Now it’s possible to implement this.
#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename ... Ts>
inline ostream & ostream::print(istr const & sFormat, Ts const & ... ts) {
	_ostream_print_helper<Ts ...> osph(this, sFormat, ts ...);
	osph.run();
	return *this;
}

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

inline ostream & ostream::print(istr const & sFormat) {
	_ostream_print_helper<> osph(this, sFormat);
	osph.run();
	return *this;
}
template <typename T0>
inline ostream & ostream::print(istr const & sFormat, T0 const & t0) {
	_ostream_print_helper<T0> osph(this, sFormat, t0);
	osph.run();
	return *this;
}
template <typename T0, typename T1>
inline ostream & ostream::print(istr const & sFormat, T0 const & t0, T1 const & t1) {
	_ostream_print_helper<T0, T1> osph(this, sFormat, t0, t1);
	osph.run();
	return *this;
}
template <typename T0, typename T1, typename T2>
inline ostream & ostream::print(istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2) {
	_ostream_print_helper<T0, T1, T2> osph(this, sFormat, t0, t1, t2);
	osph.run();
	return *this;
}
template <typename T0, typename T1, typename T2, typename T3>
inline ostream & ostream::print(
	istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3
) {
	_ostream_print_helper<T0, T1, T2, T3> osph(this, sFormat, t0, t1, t2, t3);
	osph.run();
	return *this;
}
template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline ostream & ostream::print(
	istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4
) {
	_ostream_print_helper<T0, T1, T2, T3, T4> osph(this, sFormat, t0, t1, t2, t3, t4);
	osph.run();
	return *this;
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline ostream & ostream::print(
	istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
	T5 const & t5
) {
	_ostream_print_helper<T0, T1, T2, T3, T4, T5> osph(this, sFormat, t0, t1, t2, t3, t4, t5);
	osph.run();
	return *this;
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline ostream & ostream::print(
	istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
	T5 const & t5, T6 const & t6
) {
	_ostream_print_helper<T0, T1, T2, T3, T4, T5, T6> osph(
		this, sFormat, t0, t1, t2, t3, t4, t5, t6
	);
	osph.run();
	return *this;
}
template <
	typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
	typename T7
>
inline ostream & ostream::print(
	istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
	T5 const & t5, T6 const & t6, T7 const & t7
) {
	_ostream_print_helper<T0, T1, T2, T3, T4, T5, T6, T7> osph(
		this, sFormat, t0, t1, t2, t3, t4, t5, t6, t7
	);
	osph.run();
	return *this;
}
template <
	typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
	typename T7, typename T8
>
inline ostream & ostream::print(
	istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
	T5 const & t5, T6 const & t6, T7 const & t7, T8 const & t8
) {
	_ostream_print_helper<T0, T1, T2, T3, T4, T5, T6, T7, T8> osph(
		this, sFormat, t0, t1, t2, t3, t4, t5, t6, t7, t8
	);
	osph.run();
	return *this;
}
template <
	typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
	typename T7, typename T8, typename T9
>
inline ostream & ostream::print(
	istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
	T5 const & t5, T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
) {
	_ostream_print_helper<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> osph(
		this, sFormat, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9
	);
	osph.run();
	return *this;
}

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::iostream


namespace abc {

/** Read/write abstract stream.
*/
class ABCAPI iostream :
	public virtual istream,
	public virtual ostream {
public:

	/** Constructor.
	*/
	iostream();


	/** Destructor.
	*/
	virtual ~iostream();
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_IOSTREAM_HXX

