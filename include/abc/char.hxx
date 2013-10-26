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

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#ifdef _MSC_VER
	// Silence warnings from system header files.
	#pragma warning(push)

	// “'function': exception specification does not match previous declaration”
	#pragma warning(disable: 4986)
#endif

#include <iterator>

#ifdef _MSC_VER
	#pragma warning(pop)
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals

namespace abc {

/** Indicates the level of UTF-8 string literals support:
•	2 - The UTF-8 string literal prefix (u8) is supported;
•	1 - The u8 prefix is not supported, but the compiler will generate valid UTF-8 string literals if
		 the source file is UTF-8+BOM-encoded;
•	0 - UTF-8 string literals are not supported in any way.
*/
#define ABC_CXX_UTF8LIT 0

/** Indicates how char16_t is defined:
	2 - char16_t is a native type, distinct from uint16_t and wchar_t;
	1 - abc::char16_t is a typedef for native 16-bit wchar_t, distinct from uint16_t;
	0 - abc::char16_t is a typedef for uint16_t.
*/
#define ABC_CXX_CHAR16 0

/** Indicates how char32_t is defined:
	2 - char32_t is a native type, distinct from uint32_t and wchar_t;
	1 - abc::char32_t is a typedef for native 32-bit wchar_t, distinct from uint32_t;
	0 - abc::char32_t is a typedef for uint32_t.
*/
#define ABC_CXX_CHAR32 0

#if defined(_GCC_VER) && _GCC_VER >= 40400
	// char16_t is a native type, different than uint16_t.
	#undef ABC_CXX_CHAR16
	#define ABC_CXX_CHAR16 2
	// char32_t is a native type, different than uint32_t.
	#undef ABC_CXX_CHAR32
	#define ABC_CXX_CHAR32 2

	#if _GCC_VER >= 40500
		// UTF-8 string literals are supported.
		#undef ABC_CXX_UTF8LIT
		#define ABC_CXX_UTF8LIT 2
	#endif
#else //if defined(_GCC_VER) && _GCC_VER >= 40400
	#if defined(_MSC_VER) && (!defined(_WCHAR_T_DEFINED) || !defined(_NATIVE_WCHAR_T_DEFINED))
		#error Please compile with /Zc:wchar_t
	#endif

	#if ABC_HOST_API_WIN32
		// char16_t is not a native type, but we can typedef it as wchar_t.
		#undef ABC_CXX_CHAR16
		#define ABC_CXX_CHAR16 1
	#else
		// char32_t is not a native type, but we can typedef it as wchar_t.
		#undef ABC_CXX_CHAR32
		#define ABC_CXX_CHAR32 1
	#endif
	#if !defined(_MSC_VER)
		// MSC16 will transcode non-wchar_t string literals into whatever single-byte encoding is
		// selected for the user running cl.exe; a solution has been provided in form of a hotfix
		// (<http://support.microsoft.com/kb/2284668/en-us>), but it no longer seems available, and it
		// was not ported to MSC17/VS2012, thought it seems it was finally built into MSC18/VS2013
		// (<http://connect.microsoft.com/VisualStudio/feedback/details/773186/pragma-execution-
		// character-set-utf-8-didnt-support-in-vc-2012>).
		//
		// Here we assume that no other compiler exhibits such a random behavior, and they will all
		// emit valid UTF-8 string literals it the source file is UTF-8+BOM-encoded.
		#undef ABC_CXX_UTF8LIT
		#define ABC_CXX_UTF8LIT 1
	#endif
#endif //if defined(_GCC_VER) && _GCC_VER >= 40400 … else
#if ABC_CXX_CHAR16 == 0 && ABC_CXX_CHAR32 == 0
	#error ABC_CXX_CHAR16 and/or ABC_CXX_CHAR32 must be > 0; please fix detection logic
#endif


/** UTF-8 character type. */
typedef char char8_t;

/** UTF-16 character type. */
#if ABC_CXX_CHAR16 == 1
	typedef wchar_t char16_t;
#elif ABC_CXX_CHAR16 == 0
	typedef uint16_t char16_t;
#endif

/** UTF-32 character type. */
#if ABC_CXX_CHAR32 == 1
	typedef wchar_t char32_t;
#elif ABC_CXX_CHAR32 == 0
	typedef uint32_t char32_t;
#endif


/** Defines a UCS-16 character literal.

ch
	Character literal.
return
	UCS-16 character literal.
*/
#if ABC_CXX_CHAR16 == 2
	#define U16CL(ch) u ## ch
#elif ABC_CXX_CHAR16 == 1
	#define U16CL(ch) L ## ch
#elif ABC_CXX_CHAR16 == 0
	// No native type for char16_t, but we can at least use 32-bit wchar_t to store any Unicode
	// character correctly, and then truncate that to our typedef’ed char16_t.
	// TODO: make the truncation explicit (compiler warning?).
	#define U16CL(ch) ::abc::char16_t(L ## ch)
#endif


/** Defines a UTF-32/UCS-32 character literal. Code points outside the Basic Multilingual Plane are
not supported on all platforms.

ch
	Character literal.
return
	UTF-32/UCS-32 character literal.
*/
#if ABC_CXX_CHAR32 == 2
	#define U32CL(ch) U ## ch
#elif ABC_CXX_CHAR32 == 1
	#define U32CL(ch) L ## ch
#elif ABC_CXX_CHAR32 == 0
	// No native type for char32_t, but we can at least use 16-bit wchar_t to store all Unicode BMP
	// characters correctly, and then cast that to our typedef’ed char32_t.
	#define U32CL(ch) ::abc::char32_t(L ## ch)
#endif


/** Defines a UTF-8 string literal. On some platforms, this relies on the source files being encoded
in UTF-8.

s
	String literal.
return
	UTF-8 string literal.
*/
#if ABC_CXX_UTF8LIT == 2
	#define U8SL(s) u8 ## s
#elif ABC_CXX_UTF8LIT == 1
	// Rely on the source files being encoded in UTF-8.
	#define U8SL(s) s
#endif


/** Defines a UTF-16 string literal. Not supported on all platforms; check with #ifdef before using.

s
	String literal.
return
	UTF-16 string literal.
*/
#if ABC_CXX_CHAR16 == 2
	#define U16SL(s) u ## s
#elif ABC_CXX_CHAR16 == 1
	#define U16SL(s) L ## s
#endif


/** Defines a UTF-32/UCS-32 string literal. Not supported on all platforms; check with #ifdef before
using.

s
	String literal.
return
	UTF-32 string literal.
*/
#if ABC_CXX_CHAR32 == 2
	#define U32SL(s) U ## s
#elif ABC_CXX_CHAR32 == 1
	#define U32SL(s) L ## s
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


/** Defines a character literal of the default host character literal type.

ch
	Character literal.
return
	UCS character literal.
*/
#if ABC_HOST_UTF == 8
	#define CL(ch) ch
#elif ABC_HOST_UTF == 16
	#define CL(ch) U16CL(ch)
#elif ABC_HOST_UTF == 32
	#define CL(ch) U32CL(ch)
#endif


/** Defines a string literal of the default host string literal type.

s
	String literal.
return
	UTF string literal.
*/
#if ABC_HOST_UTF == 8
	#define SL(s) U8SL(s)
#elif ABC_HOST_UTF == 16
	#define SL(s) U16SL(s)
#elif ABC_HOST_UTF == 32
	#define SL(s) U32SL(s)
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

	pt
		Pointer to set the iterator to.
	it
		Source iterator.
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

	return
		Reference to the current item.
	*/
	TVal & operator*() const {
		return *m_ptval;
	}


	/** Dereferencing member access operator.

	return
		Pointer to the current item.
	*/
	TVal * operator->() const {
		return m_ptval;
	}


	/** Element access operator.

	i	
		Index relative to *this.
	return
		Reference to the specified item.
	*/
	TVal & operator[](ptrdiff_t i) const {
		return m_ptval[i];
	}


	/** Returns true if the internal pointer is not NULL.

	return
		true if the iterator is on a valid item, or false if it’s set to NULL.
	*/
	explicit_operator_bool() const {
		return m_ptval != NULL;
	}


	/** Addition-assignment operator.

	i
		Count of positions by which to advance the iterator.
	return
		*this after it’s moved forward by i positions.
	*/
	pointer_iterator & operator+=(ptrdiff_t i) {
		m_ptval += i;
		return *this;
	}


	/** Subtraction-assignment operator.

	i
		Count of positions by which to rewind the iterator.
	return
		*this after it’s moved backwards by i positions.
	*/
	pointer_iterator & operator-=(ptrdiff_t i) {
		m_ptval -= i;
		return *this;
	}


	/** Addition operator.

	i
		Count of positions by which to advance the iterator.
	return
		Iterator that’s i items ahead of *this.
	*/
	pointer_iterator operator+(ptrdiff_t i) const {
		return pointer_iterator(m_ptval + i);
	}
	pointer_iterator operator+(pointer_iterator it) const {
		return pointer_iterator(m_ptval + it.m_ptval);
	}


	/** Subtraction operator.

	i
		Count of positions by which to rewind the iterator.
	return
		Iterator that’s i items behind *this.
	*/
	pointer_iterator operator-(ptrdiff_t i) const {
		return pointer_iterator(m_ptval - i);
	}
	ptrdiff_t operator-(pointer_iterator it) const {
		return m_ptval - it.m_ptval;
	}


	/** Preincrement operator.

	return
		*this after it’s moved to the value following the one currently pointed to by.
	*/
	pointer_iterator & operator++() {
		++m_ptval;
		return *this;
	}


	/** Postincrement operator.

	return
		Iterator pointing to the value following the one pointed to by this iterator.
	*/
	pointer_iterator operator++(int) {
		return pointer_iterator(m_ptval++);
	}


	/** Predecrement operator.

	return
		*this after it’s moved to the value preceding the one currently pointed to by.
	*/
	pointer_iterator & operator--() {
		--m_ptval;
		return *this;
	}


	/** Postdecrement operator.

	return
		Iterator pointing to the value preceding the one pointed to by this iterator.
	*/
	pointer_iterator operator--(int) {
		return pointer_iterator(m_ptval--);
	}


	/** Returns the underlying iterator type.

	return
		Pointer to the value pointed to by this iterator.
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
class char_range_ :
	public support_explicit_operator_bool<char_range_<C>> {
public:

	/** Constructor.

	ach
		Source string literal.
	pchBegin
		Pointer to the beginning of a character array.
	pchEnd
		Pointer to the end of *pchBegin.
	cch
		Count of characters in *pchBegin.
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
		//ABC_ASSERT(ach[t_cch - 1 /*NUL*/] == '\0');
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

	return
		true if the range is not empty, or false otherwise.
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

	/** Returns a const forward iterator set to the first character in the range.

	return
		Iterator.
	*/
	const_iterator cbegin() const {
		return const_iterator(m_pchBegin);
	}


	/** Returns a const reverse iterator set beyond the last character in the range.

	return
		Iterator.
	*/
	const_reverse_iterator crbegin() const {
		return const_reverse_iterator(m_pchBegin);
	}


	/** Returns a const forward iterator set beyond the last character in the range.

	return
		Iterator.
	*/
	const_iterator cend() const {
		return const_iterator(m_pchEnd);
	}


	/** Returns a const reverse iterator set to the first character in the range.

	return
		Iterator.
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

