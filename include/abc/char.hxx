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

#ifndef ABC_CHAR_HXX
#define ABC_CHAR_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/core.hxx>
#include <iterator>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals

namespace abc {

/** UTF-8 character type. */
typedef char char8_t;
#if _GCC_VER >= 40400
	// char16_t is a native type, different than uint16_t.
	#define ABC_CXX_CHAR16
	// char32_t is a native type, different than uint32_t.
	#define ABC_CXX_CHAR32
	/** UTF-16 string literal. */
	#define U16L(s) u ## s
	/** UTF-32 string literal. */
	#define U32L(s) U ## s
	#if _GCC_VER >= 40500
		/** UTF-8 string literal. */
		#define U8L(s) u8 ## s
	#else
		/** UTF-8 string literal. */
		#define U8L(s) s
	#endif
#else
	#if _MSC_VER && (!defined(_WCHAR_T_DEFINED) || !defined(_NATIVE_WCHAR_T_DEFINED))
		#error Please compile with /Zc:wchar_t
	#endif
	#if ABC_HOST_API_WIN32
		// char16_t is a native type, different than uint16_t.
		#define ABC_CXX_CHAR16
		/** UTF-16 character type. */
		typedef wchar_t char16_t;
		/** UTF-32 character type. */
		typedef uint32_t char32_t;
	#else
		// char32_t is a native type, different than uint32_t.
		#define ABC_CXX_CHAR32
		/** UTF-16 character type. */
		typedef uint16_t char16_t;
		/** UTF-32 character type. */
		typedef wchar_t char32_t;
	#endif
	/** UTF-8 string literal. */
	#define U8L(s) s
	#if ABC_HOST_API_WIN32
		/** UTF-16 string literal. */
		#define U16L(s) L ## s
	#else
		/** UTF-32 string literal. */
		#define U32L(s) L ## s
	#endif		
#endif

/** UTF-* encoding supported by the host. */
#if ABC_HOST_API_WIN32 && defined(UNICODE)
	#define ABC_HOST_UTF 16
#else
	#define ABC_HOST_UTF 8
#endif

/** Default UTF character type for the host. */
#if ABC_HOST_UTF == 8
	typedef char8_t char_t;
#elif ABC_HOST_UTF == 16
	typedef char16_t char_t;
#elif ABC_HOST_UTF == 32
	typedef char32_t char_t;
#endif

/** Default string literal type for the host. */
#if ABC_HOST_UTF == 8
	#define SL(s) U8L(s)
#elif ABC_HOST_UTF == 16
	#define SL(s) U16L(s)
#elif ABC_HOST_UTF == 32
	#define SL(s) U32L(s)
#endif

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::pointer_iterator


namespace abc {

/** Iterator based on a plain pointer.
*/
template <typename TCont, typename TVal>
class pointer_iterator :
	public support_explicit_operator_bool<pointer_iterator<TCont, TVal>> {
public:

	typedef std::random_access_iterator_tag iterator_category;
	typedef TVal value_type;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef TVal * pointer;
	typedef TVal const * const_pointer;
	typedef TVal & reference;
	typedef TVal const & const_reference;


public:

	/** Constructor.

	TODO: comment signature.
	*/
	/*constexpr*/ pointer_iterator() :
		m_ptval(NULL) {
	}
	explicit pointer_iterator(TVal * pt) :
		m_ptval(pt) {
	}
	// This is really just to convert between const/non-const TVals.
	template <typename TVal2>
	pointer_iterator(pointer_iterator<TCont, TVal2> const & it) :
		m_ptval(const_cast<TVal *>(it.base())) {
	}


	/** Dereferencing operator.

	TODO: comment signature.
	*/
	TVal & operator*() const {
		return *m_ptval;
	}


	/** Dereferencing member access operator.

	TODO: comment signature.
	*/
	TVal * operator->() const {
		return m_ptval;
	}


	/** Element access operator.

	TODO: comment signature.
	*/
	TVal & operator[](ptrdiff_t i) const {
		return m_ptval[i];
	}


	/** Returns true if the internal pointer is not NULL.

	TODO: comment signature.
	*/
	explicit_operator_bool() const {
		return m_ptval != NULL;
	}


	/** Addition-assignment operator.

	TODO: comment signature.
	*/
	pointer_iterator & operator+=(ptrdiff_t i) {
		m_ptval += i;
		return *this;
	}


	/** Subtraction-assignment operator.

	TODO: comment signature.
	*/
	pointer_iterator & operator-=(ptrdiff_t i) {
		m_ptval -= i;
		return *this;
	}


	/** Addition operator.

	TODO: comment signature.
	*/
	pointer_iterator operator+(ptrdiff_t i) const {
		return pointer_iterator(m_ptval + i);
	}
	pointer_iterator operator+(pointer_iterator it) const {
		return pointer_iterator(m_ptval + it.m_ptval);
	}


	/** Subtraction operator.

	TODO: comment signature.
	*/
	pointer_iterator operator-(ptrdiff_t i) const {
		return pointer_iterator(m_ptval - i);
	}
	ptrdiff_t operator-(pointer_iterator it) const {
		return m_ptval - it.m_ptval;
	}


	/** Preincrement operator.

	TODO: comment signature.
	*/
	pointer_iterator & operator++() {
		++m_ptval;
		return *this;
	}


	/** Postincrement operator.

	TODO: comment signature.
	*/
	pointer_iterator operator++(int) {
		return pointer_iterator(m_ptval++);
	}


	/** Predecrement operator.

	TODO: comment signature.
	*/
	pointer_iterator & operator--() {
		--m_ptval;
		return *this;
	}


	/** Postdecrement operator.

	TODO: comment signature.
	*/
	pointer_iterator operator--(int) {
		return pointer_iterator(m_ptval--);
	}


	/** Returns the underlying iterator type.

	TODO: comment signature.
	*/
	TVal * base() const {
		return m_ptval;
	}


protected:

	TVal * m_ptval;
};

} //namespace abc


// Relational operators.
#define ABC_RELOP_IMPL(op) \
	template <class TCont, typename TVal> \
	inline bool operator op( \
		abc::pointer_iterator<TCont, TVal> const & it1, \
		abc::pointer_iterator<TCont, TVal> const & it2 \
	) { \
		return it1.base() op it2.base(); \
	} \
	template <class TCont, typename TVal1, typename TVal2> \
	inline bool operator op( \
		abc::pointer_iterator<TCont, TVal1> const & it1, \
		abc::pointer_iterator<TCont, TVal2> const & it2 \
	) { \
		return it1.base() == it2.base(); \
	}
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
ABC_RELOP_IMPL(>)
ABC_RELOP_IMPL(>=)
ABC_RELOP_IMPL(<)
ABC_RELOP_IMPL(<=)
#undef ABC_RELOP_IMPL


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::char_range_


namespace abc {

/** Read-only character range. Automatically converted to/from abc::istr, and used when the latter
is not yet defined (nor can it be, such as very early header files). Part of the implementation is
in str.hxx/cxx.
*/
template <typename C>
class char_range_ {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	char_range_() :
		m_pchBegin(NULL),
		m_pchEnd(NULL) {
	}
	template <size_t t_cch>
	char_range_(C const (& ach)[t_cch]) :
		m_pchBegin(ach),
		m_pchEnd(ach + t_cch - 1 /*NUL*/) {
		// Cannot assert in this header file.
		//assert(ach[t_cch - 1 /*NUL*/] == '\0');
	}
	char_range_(C const * pchBegin, size_t cch) :
		m_pchBegin(pchBegin),
		m_pchEnd(pchBegin + cch) {
	}
	char_range_(C const * pchBegin, C const * pchEnd) :
		m_pchBegin(pchBegin),
		m_pchEnd(pchEnd) {
	}


	/** Returns true if the range comprises at least one character.

	TODO: comment signature.
	*/
	explicit_operator_bool() const {
		return m_pchEnd > m_pchBegin;
	}


	/** Returns the count of characters in the range.

	return
		Count of characters.
	*/
	size_t size() const {
		return size_t(m_pchEnd - m_pchBegin);
	}


public:

	typedef C value_type;
	typedef C const * const_pointer;
	typedef C const & const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef pointer_iterator<char_range_, C const> const_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;


public:

	/** Returns a const forward iterator set to the first element.

	TODO: comment signature.
	*/
	const_iterator cbegin() const {
		return const_iterator(m_pchBegin);
	}


	/** Returns a const reverse iterator set to the first element.

	TODO: comment signature.
	*/
	const_reverse_iterator crbegin() const {
		return const_reverse_iterator(m_pchBegin);
	}


	/** Returns a const forward iterator set beyond the last element.

	TODO: comment signature.
	*/
	const_iterator cend() const {
		return const_iterator(m_pchEnd);
	}


	/** Returns a const reverse iterator set beyond the last element.

	TODO: comment signature.
	*/
	const_reverse_iterator crend() const {
		return const_reverse_iterator(m_pchEnd);
	}


private:

	/** Pointer to the first character. */
	C const * m_pchBegin;
	/** Pointer after the last character. */
	C const * m_pchEnd;
};

typedef char_range_<char_t> char_range;
typedef char_range_<char8_t> char8_range;
typedef char_range_<char16_t> char16_range;
typedef char_range_<char32_t> char32_range;

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_CHAR_HXX

