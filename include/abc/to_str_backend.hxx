﻿/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

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

#ifndef ABC_TO_STRING_BACKEND_HXX
#define ABC_TO_STRING_BACKEND_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/str.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals

namespace abc {

/** DOC:3984 abc::to_str()

abc::to_str() is a thin wrapper around abc::to_str_backend, so that any class can provide even a
partial specialization for it (partial specializations of function are stil not allowed in C++11).

The format specification is provided by beginning and end pointer, so that a caller can specifiy a
non-NUL-terminated substring of a larger string.

The interpretation of the format specification is up to the specialization of abc::to_str_backend.
*/

/** Returns the string representation of the specified value, optionally with a custom format.

Cannot be implemented here because iostream.hxx (required for the core of the implementation,
abc::str_ostream) depends on this file - circular dependency.
*/
template <typename T>
dmstr to_str(T const & t, istr const & sFormat = istr());

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_int_to_str_backend_base


namespace abc {

// Forward declaration from iostream.hxx.
class ostream;
// Methods here need to use ostream * instead of ostream &, because at this point ostream has only
// been forward-declared above, but not defined yet (a pointer to a forward-declared type is legal,
// but a reference to it is not).

/** Base class for the specializations of to_str_backend for integer types.
*/
class _int_to_str_backend_base {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	_int_to_str_backend_base(unsigned cbInt, char_range const & crFormat);


protected:

	/** Writes the provided buffer to *posOut, prefixed as necessary.

	TODO: comment signature.
	*/
	void add_prefixes_and_write(
		bool bNegative, ostream * posOut, mstr * psBuf, char_t * pchBufFirstUsed
	) const;


	/** Converts a signed integer to its string representation.

	TODO: comment signature.
	*/
	template <typename I>
	void write_impl(I i, ostream * posOut) const;


	/** Converts a 64-bit signed integer to its string representation.

	TODO: comment signature.
	*/
	void write_s64(int64_t i, ostream * posOut) const;


	/** Converts a 64-bit unsigned integer to its string representation.

	TODO: comment signature.
	*/
	void write_u64(uint64_t i, ostream * posOut) const;


	/** Converts a 32-bit signed integer to its string representation.

	TODO: comment signature.
	*/
	void write_s32(int32_t i, ostream * posOut) const;


	/** Converts a 32-bit unsigned integer to its string representation.

	TODO: comment signature.
	*/
	void write_u32(uint32_t i, ostream * posOut) const;


	/** Converts a 16-bit signed integer to its string representation.

	TODO: comment signature.
	*/
	void write_s16(int16_t i, ostream * posOut) const;


	/** Converts a 16-bit unsigned integer to its string representation.

	TODO: comment signature.
	*/
	void write_u16(uint16_t i, ostream * posOut) const;


	/** Converts an 8-bit signed integer to its string representation.

	TODO: comment signature.
	*/
	void write_s8(int8_t i, ostream * posOut) const {
		if (m_iBaseOrShift == 10) {
			write_s16(i, posOut);
		} else {
			// Avoid extending the sign, as it would generate too many digits in any notation except
			// decimal.
			write_s16(uint8_t(i), posOut);
		}
	}


	/** Converts an 8-bit unsigned integer to its string representation.

	TODO: comment signature.
	*/
	void write_u8(uint8_t i, ostream * posOut) const {
		write_u16(i, posOut);
	}


protected:

	/** Pointer to either smc_achIntToStrL or smc_achIntToStrU. */
	char_t const * m_pchIntToStr;
	/** 10 (for decimal notation) or log2(notation) (for power-of-two notations). */
	unsigned m_iBaseOrShift;
	/** Minimum number of digits to be generated. Always >= 1, to ensure the generation of at least a
	single zero. */
	unsigned m_cchWidth;
	/** Required buffer size. */
	unsigned m_cchBuf;
	/** Character to be used to pad the digits to m_cchWidth length. */
	char_t m_chPad;
	/** Character to be used as sign in case the number is not negative; NUL if none. */
	char_t m_chSign;
	/** First character of the prefix; NUL if none (which means that m_chPrefix1 is ignored). */
	char_t m_chPrefix0;
	/** Second character of the prefix; NUL if none. */
	char_t m_chPrefix1;

	/** Map from int [0-15] to its uppercase hexadecimal representation. */
	static char_t const smc_achIntToStrU[16];
	/** Map from int [0-15] to its lowercase hexadecimal representation. */
	static char_t const smc_achIntToStrL[16];
};



#if ABC_HOST_WORD_SIZE >= 32
#if ABC_HOST_WORD_SIZE >= 64

// On a machine with 64-bit word size, write_64*() will be faster.

inline void _int_to_str_backend_base::write_s32(int32_t i, ostream * posOut) const {
	if (m_iBaseOrShift == 10) {
		write_s64(i, posOut);
	} else {
		// Avoid extending the sign in any notation except decimal, as it would generate too many
		// digits.
		write_s64(uint32_t(i), posOut);
	}
}


inline void _int_to_str_backend_base::write_u32(uint32_t i, ostream * posOut) const {
	write_u64(i, posOut);
}

#endif //if ABC_HOST_WORD_SIZE >= 64


// On a machine with 32-bit word size, write_32*() will be faster. Note that the latter might in
// turn defer to write_64*() (see above).

inline void _int_to_str_backend_base::write_s16(int16_t i, ostream * posOut) const {
	if (m_iBaseOrShift == 10) {
		write_s32(i, posOut);
	} else {
		// Avoid extending the sign in any notation except decimal, as it would generate too many
		// digits.
		write_s32(uint16_t(i), posOut);
	}
}


inline void _int_to_str_backend_base::write_u16(uint16_t i, ostream * posOut) const {
	write_u32(i, posOut);
}

#endif //if ABC_HOST_WORD_SIZE >= 32

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_int_to_str_backend


namespace abc {

/** Implementation of the specializations of to_str_backend for integer types.
*/
template <typename I>
class _int_to_str_backend :
	public _int_to_str_backend_base {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	_int_to_str_backend(char_range const & crFormat) :
		_int_to_str_backend_base(sizeof(I), crFormat) {
	}


	/** See to_str_backend::write().

	This design is rather tricky in the way one implementation calls another:

	1.	_int_to_str_backend<I>::write()
		Always inlined, dispatches to step 2. based on number of bits;
	2.	_int_to_str_backend_base::write_{s,u}{8,16,32,64}()
		Inlined to a bit-bigger variant or implemented in to_str_backend.cxx, depending on the host
		architecture’s word size;
	3.	_int_to_str_backend_base::write_impl()
		Always inlined, but only used in functions defined in to_str_backend.cxx, so it only generates
		as many copies as strictly necessary to have fastest performanced for any integer size.

	The net result is that after all the inlining occurs, this will become a direct call to the
	fastest implementation for I of any given size.

	TODO: comment signature.
	*/
	void write(I i, ostream * posOut) {
		if (sizeof(i) <= sizeof(int8_t)) {
			if (std::is_signed<I>::value) {
				write_s8(int8_t(i), posOut);
			} else {
				write_u8(uint8_t(i), posOut);
			}
		} else if (sizeof(i) <= sizeof(int16_t)) {
			if (std::is_signed<I>::value) {
				write_s16(int16_t(i), posOut);
			} else {
				write_u16(uint16_t(i), posOut);
			}
		} else if (sizeof(i) <= sizeof(int32_t)) {
			if (std::is_signed<I>::value) {
				write_s32(int32_t(i), posOut);
			} else {
				write_u32(uint32_t(i), posOut);
			}
		} else {
			static_assert(sizeof(i) <= sizeof(int64_t), "unsupported integer size");
			if (std::is_signed<I>::value) {
				write_s64(int64_t(i), posOut);
			} else {
				write_u64(uint64_t(i), posOut);
			}
		}
	}


protected:

	/** Initial (static) buffer size sufficient to output the number in binary notation. */
	static size_t const smc_cchBufInitial = 2 /* prefix or sign */ + 8 * sizeof(I);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend


namespace abc {

/** Generates a string suitable for display from an object. Once constructed with the desired format
specification, an instance can convert to a string any number of T instances. */
template <typename T>
class to_str_backend;

// Specialization for bool.
template <>
class to_str_backend<bool> {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	to_str_backend(char_range const & crFormat = char_range());


	/** See to_str_backend::write().

	TODO: comment signature.
	*/
	void write(bool b, ostream * posOut);
};


// Specialization for integer types.
#define ABC_SPECIALIZE_to_str_backend_FOR_TYPE(I) \
	template <> \
	class to_str_backend<I> : \
		public _int_to_str_backend<I> { \
	public: \
	\
		/** Constructor. */ \
		to_str_backend(char_range const & crFormat = char_range()) : \
			_int_to_str_backend<I>(crFormat) { \
		} \
	};
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(  signed char)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(unsigned char)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(         short)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(unsigned short)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(     int)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(unsigned)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(         long)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(unsigned long)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(         long long)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(unsigned long long)
#undef ABC_SPECIALIZE_to_str_backend_FOR_TYPE


// Specialization for void const volatile *.
template <>
class to_str_backend<void const volatile *> :
	protected to_str_backend<uintptr_t> {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	to_str_backend(char_range const & crFormat = char_range());


	/** See to_str_backend::write().

	TODO: comment signature.
	*/
	void write(void const volatile * p, ostream * posOut) {
		to_str_backend<uintptr_t>::write(reinterpret_cast<uintptr_t>(p), posOut);
	}


protected:

	/** Format string used to display the address. */
	static char_t const smc_achFormat[];
};


// Specialization for any pointer-to-type.
template <typename T>
class to_str_backend<T *> :
	public to_str_backend<void const volatile *> {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	to_str_backend(char_range const & crFormat = char_range()) :
		to_str_backend<void const volatile *>(crFormat) {
	}
};


// Specialization for string literal types.
#define ABC_SPECIALIZE_to_str_backend_FOR_TYPE(C) \
	/** String literal. \
	*/ \
	template <size_t t_cch> \
	class to_str_backend<C const [t_cch]> : \
		public _str_to_str_backend<C const [t_cch], C> { \
	\
		typedef _str_to_str_backend<C const [t_cch], C> str_to_str_backend; \
	\
	public: \
	\
		/** Constructor. \
	\
		TODO: comment signature. \
		*/ \
		to_str_backend(char_range const & crFormat = char_range()) : \
			str_to_str_backend(crFormat) { \
		} \
	\
	\
		/** See to_str_backend::write(). \
	\
		TODO: comment signature. \
		*/ \
		void write(C const (& ach)[t_cch], ostream * posOut) { \
			assert(ach[t_cch - 1 /*NUL*/] == CL('\0')); \
			str_to_str_backend::write( \
				ach, sizeof(C) * (t_cch - 1 /*NUL*/), text::utf_traits<C>::host_encoding, posOut \
			); \
		} \
	}; \
	\
	/** Non-const string literal. \
	\
	TODO: remove the need for this. \
	*/ \
	template <size_t t_cch> \
	class to_str_backend<C [t_cch]> : \
		public to_str_backend<C const [t_cch]> { \
	public: \
	\
		/** Constructor. \
	\
		TODO: comment signature. \
		*/ \
		to_str_backend(char_range const & crFormat = char_range()) : \
			to_str_backend<C const [t_cch]>(crFormat) { \
		} \
	}; \
	\
	/** Pointer to NUL-terminated string. \
	*/ \
	template <> \
	class to_str_backend<C const *> : \
		public _str_to_str_backend<C const *, C> { \
	\
		typedef _str_to_str_backend<C const *, C> str_to_str_backend; \
	\
	public: \
	\
		/** Constructor. \
	\
		TODO: comment signature. \
		*/ \
		to_str_backend(char_range const & crFormat = char_range()) : \
			str_to_str_backend(crFormat) { \
		} \
	\
	\
		/** See to_str_backend::write(). \
	\
		TODO: comment signature. \
		*/ \
		void write(C const * psz, ostream * posOut) { \
			str_to_str_backend::write( \
				psz, sizeof(C) * text::utf_traits<C>::str_len(psz), \
				text::utf_traits<C>::host_encoding, posOut \
			); \
		} \
	};
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char8_t)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char16_t)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char32_t)
#undef ABC_SPECIALIZE_to_str_backend_FOR_TYPE

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_TO_STRING_BACKEND_HXX

