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

#ifndef ABC_STRING_HXX
#define ABC_STRING_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/_vextr.hxx>
#include <abc/utf_traits.hxx>
#include <functional>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_string_to_string_backend_base


namespace abc {

/** Base class for the specializations of to_string_backend for string types. Not using templates,
so the implementation can be in a cxx file.
*/
class _string_to_string_backend_base {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	_string_to_string_backend_base(char_range const & crFormat);


protected:

	/** Writes the contents of the string, applying the specified format.

	TODO: comment signature.
	*/
	void write(void const * p, size_t cb, text::encoding enc, ostream * posOut);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_string_to_string_backend


namespace abc {

/** Mid-class for the specializations of to_string_backend for string types. This is used by string
literal types as well (see to_string_backend.hxx).
*/
template <typename T, typename C>
class _string_to_string_backend :
	public _string_to_string_backend_base {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	_string_to_string_backend(char_range const & crFormat) :
		_string_to_string_backend_base(crFormat) {
	}
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::char_range_


namespace abc {

// Specialization of to_string_backend.
template <typename C>
class to_string_backend<char_range_<C>> :
	public _string_to_string_backend<char_range_<C>, C> {

	typedef _string_to_string_backend<char_range_<C>, C> string_to_string_backend;

public:

	/** Constructor.

	TODO: comment signature.
	*/
	to_string_backend(char_range const & crFormat = char_range()) :
		string_to_string_backend(crFormat) {
	}


	/** See to_string_backend::write().
	*/
	void write(char_range_<C> const & cr, ostream * posOut) {
		string_to_string_backend::write(
			cr.cbegin().base(), sizeof(C) * cr.get_size(), text::utf_traits<C>::host_encoding, posOut
		);
	}
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_string


namespace abc {

/** Template-independent methods of string_.
*/
class _raw_string :
	public _raw_trivial_vextr_impl {

	ABC_CLASS_PREVENT_COPYING(_raw_string)

public:

	/** Returns the current size of the string buffer, in characters, minus room for the trailing NUL
	terminator.

	TODO: comment signature.
	*/
	size_t get_capacity() const {
		return _raw_trivial_vextr_impl::get_capacity(true);
	}


	/** Returns the current length of the string, in characters, excluding the trailing NUL
	terminator.

	TODO: comment signature.
	*/
	size_t get_size() const {
		return _raw_trivial_vextr_impl::get_size(true);
	}


	/** Computes the hash value of the string.

	TODO: comment signature.
	*/
	size_t hash(size_t cbItem) const;


	/** See _raw_trivial_vextr_impl::set_capacity().

	TODO: comment signature.
	*/
	void set_capacity(size_t cbItem, size_t cchMin, bool bPreserve) {
		_raw_trivial_vextr_impl::set_capacity(cbItem, cchMin, bPreserve, true);
	}


	/** Changes the length of the string, without changing its capacity.

	TODO: comment signature.
	*/
	void set_size(size_t cbItem, size_t cch);


protected:

	/** Constructor.

	TODO: comment signature.
	*/
	_raw_string(size_t cchStaticMax) :
		_raw_trivial_vextr_impl(cchStaticMax, true) {
	}
	_raw_string(void const * pConstSrc, size_t cchSrc) :
		_raw_trivial_vextr_impl(pConstSrc, cchSrc + 1 /*NUL*/) {
	}


	/** See _raw_trivial_vextr_impl::adjust_index().

	TODO: comment signature.
	*/
	size_t adjust_index(ptrdiff_t i) const {
		return _raw_trivial_vextr_impl::adjust_index(i, true);
	}


	/** See _raw_trivial_vextr_impl::adjust_range().

	TODO: comment signature.
	*/
	void adjust_range(ptrdiff_t * piFirst, ptrdiff_t * pci) const {
		_raw_trivial_vextr_impl::adjust_range(piFirst, pci, true);
	}


	/** See _raw_trivial_vextr_impl::assign_copy().

	TODO: comment signature.
	*/
	void assign_copy(size_t cbItem, void const * p, size_t ci) {
		_raw_trivial_vextr_impl::assign_copy(cbItem, p, ci, true);
	}
	void assign_copy(size_t cbItem, void const * p1, size_t ci1, void const * p2, size_t ci2) {
		_raw_trivial_vextr_impl::assign_copy(cbItem, p1, ci1, p2, ci2, true);
	}


	/** See _raw_trivial_vextr_impl::assign_move().

	TODO: comment signature.
	*/
	void assign_move(_raw_string && rs) {
		_raw_trivial_vextr_impl::assign_move(static_cast<_raw_trivial_vextr_impl &&>(rs), true);
	}


	/** See _raw_trivial_vextr_impl::assign_move_dynamic_or_copy().

	TODO: comment signature.
	*/
	void assign_move_dynamic_or_copy(size_t cbItem, _raw_string && rs) {
		_raw_trivial_vextr_impl::assign_move_dynamic_or_copy(
			cbItem, static_cast<_raw_trivial_vextr_impl &&>(rs), true
		);
	}


	/** See _raw_trivial_vextr_impl::assign_share_ro_or_copy().

	TODO: comment signature.
	*/
	void assign_share_ro_or_copy(size_t cbItem, _raw_string const & rs) {
		_raw_trivial_vextr_impl::assign_share_ro_or_copy(cbItem, rs, true);
	}
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::string_base_


namespace abc {

// Forward declarations.
template <typename C, class TTraits = text::utf_traits<C>>
class cstring_;
template <typename C, class TTraits = text::utf_traits<C>>
class wdstring_;

/** Base class for strings. It behaves like a vector with a last NUL element hidden from clients; an
empty string always has an accessible trailing NUL character.

Methods that take an array of characters whose length is obtained by its size instead of e.g.
strlen(), will discard the last element, asserting that it’s the NUL terminator. See [HACK#0010
abc::string_ and abc::vector design] for implementation details for this and all the *string
classes.
*/
template <typename C, class TTraits>
class string_base_ :
	protected _raw_string,
	public _iterable_vector<string_base_<C, TTraits>, C>,
	public support_explicit_operator_bool<string_base_<C, TTraits>> {

	ABC_CLASS_PREVENT_COPYING(string_base_)

	typedef cstring_<C, TTraits> cstring;
	typedef wdstring_<C, TTraits> wdstring;

protected:

	/** Shortcut for the base class providing iterator-based types and methods. */
	typedef _iterable_vector<string_base_<C, TTraits>, C> itvec;


public:

	/** Character type. */
	typedef C char_t;
	/** See _iterable_vector::const_iterator. */
	typedef typename itvec::const_iterator const_iterator;


public:

	/** Character access operator.

	TODO: comment signature.
	*/
	C operator[](size_t i) const {
		if (i > get_size()) {
			abc_throw(index_error(intptr_t(i)));
		}
		return get_data()[i];
	}


	/** Returns true if the length is greater than 0.

	TODO: comment signature.
	*/
	explicit_operator_bool() const {
		return get_size() > 0;
	}


	/** Support for relational operators.

	TODO: comment signature.
	*/
	int compare_to(cstring const & s) const {
		return TTraits::str_cmp(get_data(), get_size(), s.get_data(), s.get_size());
	}
	template <size_t t_cch>
	int compare_to(C const (& ach)[t_cch]) const {
		assert(ach[t_cch - 1 /*NUL*/] == '\0');
		return TTraits::str_cmp(get_data(), get_size(), ach, t_cch - 1 /*NUL*/);
	}
	// This overload needs to be template, or it will take precedence over the one above.
	template <typename = void>
	int compare_to(C const * psz) const {
		return TTraits::str_cmp(get_data(), get_size(), psz, TTraits::str_len(psz));
	}


	/** Searches for the specified value; returns an iterator to the first matching item, or
	const_iterator() (evaluates to false) for no matches.

	TODO: comment signature.
	*/
	const_iterator find(char32_t chNeedle, const_iterator itFirst = const_iterator()) const {
		return const_iterator(TTraits::str_chr(
			(itFirst ? itFirst : itvec::cbegin()).base(), itvec::cend().base(),
			chNeedle
		));
	}
	const_iterator find(cstring const & sNeedle, const_iterator itFirst = const_iterator()) const {
		return const_iterator(TTraits::str_str(
			(itFirst ? itFirst : itvec::cbegin()).base(), itvec::cend().base(),
			sNeedle.cbegin().base(), sNeedle.cend().base()
		));
	}


	/** Searches for the specified value, starting from the end or the provided iterator; returns an
	iterator to the first matching item, or const_iterator() (evaluates to false) for no matches.

	TODO: comment signature.
	*/
	const_iterator find_last(char32_t chNeedle, const_iterator itEnd = const_iterator()) const {
		return const_iterator(TTraits::str_chr_r(
			itvec::cbegin().base(), (itEnd ? itEnd : itvec::cend()).base(),
			chNeedle
		));
	}
	const_iterator find_last(
		cstring const & sNeedle, const_iterator itEnd = const_iterator()
	) const {
		return const_iterator(TTraits::str_str_r(
			itvec::cbegin().base(), (itEnd ? itEnd : itvec::cend()).base(),
			sNeedle.cbegin().base(), sNeedle.cend().base()
		));
	}


	/** Uses the current contents of this string to generate a new one using string_ostream::print().

	TODO: comment signature.
	*/
	template <typename ... Ts>
	wdstring format(Ts const & ... ts) const;


	/** Returns the current size of the string buffer, in characters, minus room for the trailing NUL
	terminator.

	TODO: comment signature.
	*/
	size_t get_capacity() const {
		return _raw_string::get_capacity();
	}


	/** Returns a read-only pointer to the character array.

	TODO: comment signature.
	*/
	C const * get_data() const {
		return _raw_string::get_data<C>();
	}


	/** Work around the protected inheritance, forcing the raw access to be explicit.

	TODO: comment signature.
	*/
	_raw_string & get_raw() {
		return *this;
	}
	_raw_string const & get_raw() const {
		return *this;
	}


	/** Returns the count of characters in the string.

	TODO: comment signature.
	*/
	size_t get_size() const {
		return _raw_string::get_size();
	}


	/** Returns the count of code points in the string.

	TODO: comment signature.
	*/
	size_t get_size_cp() const {
		C const * pchBegin(get_data());
		return TTraits::str_cp_len(pchBegin, pchBegin + get_size());
	}


	/** Returns a portion of the string.

	ichFirst
		0-based index of the first character. If negative, it’s 1-based index from the end of the
		string.
	[cch]
		Count of characters to return. If negative, it’s the count of characters to skip, from the end
		of the string.
	*/
	wdstring substr(ptrdiff_t ichFirst) const {
		return substr(ichFirst, get_size());
	}
	wdstring substr(ptrdiff_t ichFirst, ptrdiff_t cch) const {
		adjust_range(&ichFirst, &cch);
		return wdstring(get_data() + ichFirst, size_t(cch));
	}
	wdstring substr(const_iterator itFirst) const {
		return substr(itFirst, itvec::cend());
	}
	wdstring substr(const_iterator itBegin, const_iterator itEnd) const {
		return wdstring(itBegin.base(), size_t(itEnd - itBegin));
	}


protected:

	/** Constructor.

	TODO: comment signature.
	*/
	string_base_(size_t cchStatic) :
		_raw_string(cchStatic) {
	}
	string_base_(C const * pch, size_t cch) :
		_raw_string(pch, cch) {
	}


	/** See _raw_string::assign_copy().

	TODO: comment signature.
	*/
	void assign_copy(C const * pch, size_t cch) {
		_raw_string::assign_copy(sizeof(C), pch, cch);
	}
	void assign_copy(C const * pch1, size_t cch1, C const * pch2, size_t cch2) {
		_raw_string::assign_copy(sizeof(C), pch1, cch1, pch2, cch2);
	}


	/** See _raw_string::assign_move().

	TODO: comment signature.
	*/
	void assign_move(string_base_ && sb) {
		_raw_string::assign_move(static_cast<_raw_string &&>(sb));
	}


	/** See _raw_string::assign_move_dynamic_or_copy().

	TODO: comment signature.
	*/
	void assign_move_dynamic_or_copy(string_base_ && sb) {
		_raw_string::assign_move_dynamic_or_copy(sizeof(C), static_cast<_raw_string &&>(sb));
	}


	/** See _raw_string::assign_share_ro_or_copy().

	TODO: comment signature.
	*/
	void assign_share_ro_or_copy(string_base_ const & sb) {
		_raw_string::assign_share_ro_or_copy(sizeof(C), sb);
	}
};

} //namespace abc


// Relational operators.
#define ABC_RELOP_IMPL(op) \
	template <typename C, class TTraits> \
	inline bool operator op( \
		abc::string_base_<C, TTraits> const & s1, abc::string_base_<C, TTraits> const & s2 \
	) { \
		return s1.compare_to(static_cast<abc::cstring_<C, TTraits>>(s2)) op 0; \
	} \
	template <typename C, class TTraits, size_t t_cch> \
	inline bool operator op(abc::string_base_<C, TTraits> const & s, C const (& ach)[t_cch]) { \
		return s.compare_to(ach) op 0; \
	} \
	template <typename C, class TTraits, size_t t_cch> \
	inline bool operator op(C const (& ach)[t_cch], abc::string_base_<C, TTraits> const & s) { \
		return -s.compare_to(ach) op 0; \
	} \
	template <typename C, class TTraits> \
	inline bool operator op(abc::string_base_<C, TTraits> const & s, C const * psz) { \
		return s.compare_to(psz) op 0; \
	} \
	template <typename C, class TTraits, size_t t_cch> \
	inline bool operator op(C const * psz, abc::string_base_<C, TTraits> const & s) { \
		return -s.compare_to(psz) op 0; \
	}
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
ABC_RELOP_IMPL(>)
ABC_RELOP_IMPL(>=)
ABC_RELOP_IMPL(<)
ABC_RELOP_IMPL(<=)
#undef ABC_RELOP_IMPL


namespace abc {

// Specialization of to_string_backend.
template <typename C, class TTraits>
class to_string_backend<string_base_<C, TTraits>> :
	public _string_to_string_backend<string_base_<C, TTraits>, C> {

	typedef _string_to_string_backend<string_base_<C, TTraits>, C> string_to_string_backend;

public:

	/** Constructor.

	TODO: comment signature.
	*/
	to_string_backend(char_range const & crFormat = char_range()) :
		string_to_string_backend(crFormat) {
	}


	/** See to_string_backend::write().
	*/
	void write(string_base_<C, TTraits> const & s, ostream * posOut) {
		string_to_string_backend::write(
			s.get_data(), sizeof(C) * s.get_size(), TTraits::host_encoding, posOut
		);
	}
};

} //namespace abc


namespace std {

// Specialization of std::hash.
template <typename C, class TTraits>
struct hash<abc::string_base_<C, TTraits>> {

	size_t operator()(abc::string_base_<C, TTraits> const & sb) const {
		return sb.get_raw().hash(sizeof(C));
	}
};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::cstring_


namespace abc {

// Forward declaration.
template <typename C, class TTraits = text::utf_traits<C>>
class wstring_;

/** string_base_-derived class, to be used as “the” string class in most cases. It cannot be
modified in-place, which means that it shouldn’t be used in code performing intensive string
manipulations.
*/
template <typename C, class TTraits /*= text::utf_traits<C>*/>
class cstring_ :
	public string_base_<C, TTraits> {

	typedef string_base_<C, TTraits> string_base;
	typedef wstring_<C, TTraits> wstring;
	typedef wdstring_<C, TTraits> wdstring;

public:

	/** Constructor.

	TODO: comment signature.
	*/
	cstring_() :
		string_base(0) {
	}
	cstring_(cstring_ const & cs) :
		string_base(0) {
		operator=(cs);
	}
	cstring_(cstring_ && cs) :
		string_base(0) {
		operator=(std::move(cs));
	}
	cstring_(string_base && sb) :
		string_base(0) {
		operator=(std::move(sb));
	}
	cstring_(wdstring && wds) :
		string_base(0) {
		operator=(std::move(wds));
	}
	template <size_t t_cch>
	cstring_(C const (& ach)[t_cch]) :
		string_base(ach, t_cch - 1 /*NUL*/) {
		assert(ach[t_cch - 1 /*NUL*/] == '\0');
	}
	cstring_(C const * psz, size_t cch) :
		string_base(0) {
		assign_copy(psz, cch);
	}
	cstring_(unsafe_t, C const * psz) :
		string_base(psz, TTraits::str_len(psz)) {
	}
	cstring_(unsafe_t, C const * psz, size_t cch) :
		string_base(psz, cch) {
	}


	/** Assignment operator.

	TODO: comment signature.
	*/
	cstring_ & operator=(cstring_ const & cs) {
		assign_share_ro_or_copy(cs);
		return *this;
	}
	cstring_ & operator=(cstring_ && cs) {
		// Non-const, so it can’t be anything but a real cstring_, so it owns its item array.
		assign_move(std::move(cs));
		return *this;
	}
	cstring_ & operator=(string_base && sb) {
		assign_move_dynamic_or_copy(std::move(sb));
		return *this;
	}
	cstring_ & operator=(wdstring && wds) {
		assign_move(std::move(wds));
		return *this;
	}
	template <size_t t_cch>
	cstring_ & operator=(C const (& ach)[t_cch]) {
		// This order is safe, because the constructor invoked on the next line won’t throw.
		this->~cstring_();
		::new(this) cstring_(ach);
		return *this;
	}


	/** Automatic conversion to char_range.

	TODO: comment signature.
	*/
	operator char_range_<C>() const {
		return char_range_<C>(string_base::cbegin().base(), string_base::cend().base());
	}
};

typedef cstring_<char_t> cstring;
typedef cstring_<char8_t> cstring8;
typedef cstring_<char16_t> cstring16;
typedef cstring_<char32_t> cstring32;


// Specialization of to_string_backend.
template <typename C, class TTraits>
class to_string_backend<cstring_<C, TTraits>> :
	public to_string_backend<string_base_<C, TTraits>> {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	to_string_backend(char_range const & crFormat = char_range()) :
		to_string_backend<string_base_<C, TTraits>>(crFormat) {
	}
};

} //namespace abc


namespace std {

// Specialization of std::hash.
template <typename C, class TTraits>
struct hash<abc::cstring_<C, TTraits>> :
	public hash<abc::string_base_<C, TTraits>> {
};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::wstring_


namespace abc {

/** string_base_-derived class, to be used as argument type for functions that want to modify a
string argument, since unlike cstring_, it allows in-place alterations to the string. Both wsstring
and wdstring_ are automatically converted to this.
*/
template <typename C, class TTraits /*= text::utf_traits<C>*/>
class wstring_ :
	public string_base_<C, TTraits> {

	typedef string_base_<C, TTraits> string_base;
	typedef cstring_<C, TTraits> cstring;
	typedef wdstring_<C, TTraits> wdstring;

public:

	/** Assignment operator.

	TODO: comment signature.
	*/
	wstring_ & operator=(wstring_ const & ws) {
		return operator=(static_cast<string_base const &>(ws));
	}
	// WARNING - this move-assignment operator CAN throw!
	wstring_ & operator=(wstring_ && ws) {
		return operator=(static_cast<string_base &&>(ws));
	}
	wstring_ & operator=(string_base const & sb) {
		assign_copy(sb.get_data(), sb.get_size());
		return *this;
	}
	wstring_ & operator=(string_base && sb) {
		assign_move_dynamic_or_copy(std::move(sb));
		return *this;
	}
	wstring_ & operator=(wdstring && wds) {
		assign_move(std::move(wds));
		return *this;
	}
	template <size_t t_cch>
	wstring_ & operator=(C const (& ach)[t_cch]) {
		assert(ach[t_cch - 1 /*NUL*/] == '\0');
		assign_copy(ach, t_cch - 1 /*NUL*/);
		return *this;
	}


	/** Concatenation-assignment operator.

	TODO: comment signature.
	*/
	wstring_ & operator+=(C ch) {
		append(&ch, 1);
		return *this;
	}
	template <size_t t_cch>
	wstring_ & operator+=(C const (& ach)[t_cch]) {
		assert(ach[t_cch - 1 /*NUL*/] == '\0');
		append(ach, t_cch - 1 /*NUL*/);
		return *this;
	}
	wstring_ & operator+=(cstring const & cs) {
		append(cs.get_data(), cs.get_size());
		return *this;
	}


	/** Allows automatic cross-class-hierarchy casts.

	TODO: comment signature.
	*/
	operator cstring const &() const {
		return *static_cast<cstring const *>(static_cast<string_base const *>(this));
	}


	/** Item access operator.

	TODO: comment signature.
	*/
	C & operator[](size_t i) {
		if (i > string_base::get_size()) {
			abc_throw(index_error(intptr_t(i)));
		}
		return get_data()[i];
	}
	C operator[](size_t i) const {
		return string_base::operator[](intptr_t(i));
	}


	/** Same as operator+=(), but for multi-argument overloads.

	TODO: comment signature.
	*/
	void append(C const * pchAdd, size_t cchAdd) {
		_raw_string::append(sizeof(C), pchAdd, cchAdd, true);
	}


	/** Returns a pointer to the character array.

	TODO: comment signature.
	*/
	C * get_data() {
		return _raw_string::get_data<C>();
	}
	C const * get_data() const {
		return _raw_string::get_data<C>();
	}


	/** Grows the item array until the specified callback succeeds in filling it and returns a number
	of needed characters that’s less than the size of the buffer. For example, for cchMax == 3 (NUL
	terminator included), it must return <= 2 (NUL excluded).

	This method is not transaction-safe; if an exception is thrown in the callback or elsewhere,
	*this will not be restored to its previous state.

	TODO: maybe improve exception resilience? Check typical usage to see if it’s an issue.

	Note: this method probably benefits from being inlined in spite of its size, because in most
	cases the callback will be just a lambda wrapper around some API/OS function, so when the
	compiler inlines this method, it will most likely blend its code with the (also inlined) lambda,
	probably resulting in some optimizations that would be otherwise missed.

	TODO: comment signature.
	*/
	void grow_for(std::function<size_t (C * pch, size_t cchMax)> fnRead) {
		typedef _raw_vextr_impl_base rvib;
		// The initial size avoids a couple of reallocations.
		// Also, these numbers should guarantee that set_capacity() will allocate exactly the
		// requested number of characters, eliminating the need to query back with get_capacity().
		size_t cchRet(rvib::smc_cMinSlots * rvib::smc_iGrowthRate * rvib::smc_iGrowthRate);
		for (size_t cchMax(cchRet); cchRet >= cchMax; cchMax *= rvib::smc_iGrowthRate) {
			set_capacity(cchMax - 1 /*NUL*/, false);
			cchRet = fnRead(get_data(), cchMax);
		}
		// Finalize the length.
		set_size(cchRet);
	}


	/** See _raw_string::set_capacity().

	TODO: comment signature.
	*/
	void set_capacity(size_t cchMin, bool bPreserve) {
		_raw_string::set_capacity(sizeof(C), cchMin, bPreserve);
	}


	/** See _raw_string::set_size().

	TODO: comment signature.
	*/
	void set_size(size_t cch) {
		_raw_string::set_size(sizeof(C), cch);
	}


protected:

	/** Constructor.

	TODO: comment signature.
	*/
	wstring_(size_t cchStatic) :
		string_base(cchStatic) {
	}
	wstring_(C const * pch, size_t cch) :
		string_base(pch, cch) {
	}
};

typedef wstring_<char_t> wstring;
typedef wstring_<char8_t> wstring8;
typedef wstring_<char16_t> wstring16;
typedef wstring_<char32_t> wstring32;


// Specialization of to_string_backend.
template <typename C, class TTraits>
class to_string_backend<wstring_<C, TTraits>> :
	public to_string_backend<string_base_<C, TTraits>> {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	to_string_backend(char_range const & crFormat = char_range()) :
		to_string_backend<string_base_<C, TTraits>>(crFormat) {
	}
};

} //namespace abc


namespace std {

// Specialization of std::hash.
template <typename C, class TTraits>
struct hash<abc::wstring_<C, TTraits>> :
	public hash<abc::string_base_<C, TTraits>> {
};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::wdstring_


namespace abc {

/** wstring_-derived class, good for clients that need in-place manipulation of strings whose length
is unknown at design time.
*/
template <typename C, class TTraits /*= text::utf_traits<C>*/>
class wdstring_ :
	public wstring_<C, TTraits> {

	typedef string_base_<C, TTraits> string_base;
	typedef wstring_<C, TTraits> wstring;
	typedef cstring_<C, TTraits> cstring;

public:

	/** Constructor.

	TODO: comment signature.
	*/
	wdstring_() :
		wstring(0) {
	}
	wdstring_(wdstring_ const & wds) :
		wstring(0) {
		operator=(wds);
	}
	wdstring_(wdstring_ && wds) :
		wstring(0) {
		operator=(std::move(wds));
	}
	wdstring_(string_base const & sb) :
		wstring(0) {
		operator=(sb);
	}
	wdstring_(string_base && sb) :
		wstring(0) {
		operator=(std::move(sb));
	}
	template <size_t t_cch>
	wdstring_(C const (& ach)[t_cch]) :
		wstring(0) {
		operator=(ach);
	}
	wdstring_(C const * pch, size_t cch) :
		wstring(0) {
		assign_copy(pch, cch);
	}
	wdstring_(C const * pch1, size_t cch1, C const * pch2, size_t cch2) :
		wstring(0) {
		assign_copy(pch1, cch1, pch2, cch2);
	}


	/** Assignment operator.

	TODO: comment signature.
	*/
	wdstring_ & operator=(wdstring_ const & wds) {
		wstring::operator=(wds);
		return *this;
	}
	wdstring_ & operator=(wdstring_ && wds) {
		wstring::operator=(std::move(wds));
		return *this;
	}
	wdstring_ & operator=(string_base const & sb) {
		wstring::operator=(sb);
		return *this;
	}
	wdstring_ & operator=(string_base && sb) {
		wstring::operator=(std::move(sb));
		return *this;
	}
	template <size_t t_cch>
	wdstring_ & operator=(C const (& ach)[t_cch]) {
		wstring::operator=(ach);
		return *this;
	}
};

} //namespace abc


/** Concatenation operator.

TODO: comment signature.
*/
template <typename C, class TTraits>
inline abc::wdstring_<C, TTraits> operator+(
	abc::string_base_<C, TTraits> const & sb1, abc::string_base_<C, TTraits> const & sb2
) {
	return abc::wdstring_<C, TTraits>(
		sb1.get_data(), sb1.get_size(), sb2.get_data(), sb2.get_size()
	);
}
// Overloads taking a character literal.
template <typename C, class TTraits>
inline abc::wdstring_<C, TTraits> operator+(abc::string_base_<C, TTraits> const & sb, C ch) {
	return abc::wdstring_<C, TTraits>(sb.get_data(), sb.get_size(), &ch, 1);
}
template <typename C, class TTraits>
inline abc::wdstring_<C, TTraits> operator+(C ch, abc::string_base_<C, TTraits> const & sb) {
	return abc::wdstring_<C, TTraits>(&ch, 1, sb.get_data(), sb.get_size());
}
// Overloads taking a string literal.
template <typename C, class TTraits, size_t t_cch>
inline abc::wdstring_<C, TTraits> operator+(
	abc::string_base_<C, TTraits> const & sb, C const (& ach)[t_cch]
) {
	assert(ach[t_cch - 1 /*NUL*/] == '\0');
	return abc::wdstring_<C, TTraits>(sb.get_data(), sb.get_size(), ach, t_cch - 1 /*NUL*/);
}
template <typename C, class TTraits, size_t t_cch>
inline abc::wdstring_<C, TTraits> operator+(
	C const (& ach)[t_cch], abc::string_base_<C, TTraits> const & sb
) {
	assert(ach[t_cch - 1 /*NUL*/] == '\0');
	return abc::wdstring_<C, TTraits>(ach, t_cch - 1 /*NUL*/, sb.get_data(), sb.get_size());
}
// Overloads taking a temporary wdstring as left operand; they can avoid creating an intermediate
// string.
template <typename C, class TTraits>
inline abc::wdstring_<C, TTraits> operator+(abc::wdstring_<C, TTraits> && wds, C ch) {
	wds += ch;
	return std::move(wds);
}
template <typename C, class TTraits, size_t t_cch>
inline abc::wdstring_<C, TTraits> operator+(
	abc::wdstring_<C, TTraits> && wds, C const (& ach)[t_cch]
) {
	wds += ach;
	return std::move(wds);
}


namespace abc {

// Specialization of to_string_backend.
template <typename C, class TTraits>
class to_string_backend<wdstring_<C, TTraits>> :
	public to_string_backend<string_base_<C, TTraits>> {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	to_string_backend(char_range const & crFormat = char_range()) :
		to_string_backend<string_base_<C, TTraits>>(crFormat) {
	}
};

} //namespace abc


namespace std {

// Specialization of std::hash.
template <typename C, class TTraits>
struct hash<abc::wdstring_<C, TTraits>> :
	public hash<abc::string_base_<C, TTraits>> {
};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::wsstring


namespace abc {

/** wstring_-derived class, good for clients that need in-place manipulation of strings that are
most likely to be shorter than a known small size.
*/
template <size_t t_cchStatic, typename C = char_t, class TTraits = text::utf_traits<C>>
class wsstring :
	public wstring_<C, TTraits> {

	typedef string_base_<C, TTraits> string_base;
	typedef wstring_<C, TTraits> wstring;
	typedef cstring_<C, TTraits> cstring;
	typedef wdstring_<C, TTraits> wdstring;

private:

	/** Actual static item array size. */
	static size_t const smc_cchFixed = _ABC__RAW_VEXTR_IMPL_BASE__ADJUST_ITEM_COUNT(
		t_cchStatic + 1 /*NUL*/
	);


public:

	/** Constructor.

	TODO: comment signature.
	*/
	wsstring() :
		wstring(smc_cchFixed) {
	}
	wsstring(wsstring const & wss) :
		wstring(smc_cchFixed) {
		operator=(wss);
	}
	// This won’t throw exceptions - see operator=(wsstring &&).
	wsstring(wsstring && wss) :
		wstring(smc_cchFixed) {
		operator=(std::move(wss));
	}
	wsstring(string_base const & sb) :
		wstring(smc_cchFixed) {
		operator=(sb);
	}
	wsstring(string_base && sb) :
		wstring(smc_cchFixed) {
		operator=(std::move(sb));
	}
	wsstring(wdstring && wds) :
		wstring(smc_cchFixed) {
		operator=(std::move(wds));
	}
	template <size_t t_cch>
	wsstring(C const (& ach)[t_cch]) :
		wstring(smc_cchFixed) {
		operator=(ach);
	}


	/** Assignment operator.

	TODO: comment signature.
	*/
	wsstring & operator=(wsstring const & ws) {
		wstring::operator=(ws);
		return *this;
	}
	// If the source is using its static item array, it will be copied without allocating a dynamic
	// one; if the source is dynamic, it will be moved. Either way, this won’t throw.
	wsstring & operator=(wsstring && ws) {
		wstring::operator=(std::move(ws));
		return *this;
	}
	// This also covers wsstring of different template arguments.
	wsstring & operator=(string_base const & sb) {
		wstring::operator=(sb);
		return *this;
	}
	// This also covers wsstring of different template arguments.
	wsstring & operator=(string_base && sb) {
		wstring::operator=(std::move(sb));
		return *this;
	}
	wsstring & operator=(wdstring && wds) {
		wstring::operator=(std::move(wds));
		return *this;
	}
	template <size_t t_cch>
	wsstring & operator=(C const (& ach)[t_cch]) {
		wstring::operator=(ach);
		return *this;
	}


private:

	// This section must match exactly _raw_vextr_impl_base_with_static_item_array.

	/** See _raw_vextr_impl_base_with_static_item_array::m_ciStaticMax. */
	size_t m_ciStaticMax;
	/** See _raw_vextr_impl_base_with_static_item_array::m_at. */
	std::max_align_t m_at[ABC_ALIGNED_SIZE(sizeof(C) * smc_cchFixed)];
};

typedef wdstring_<char_t> wdstring;
typedef wdstring_<char8_t> wdstring8;
typedef wdstring_<char16_t> wdstring16;
typedef wdstring_<char32_t> wdstring32;


// Specialization of to_string_backend.
template <size_t t_cchStatic, typename C, class TTraits>
class to_string_backend<wsstring<t_cchStatic, C, TTraits>> :
	public to_string_backend<string_base_<C, TTraits>> {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	to_string_backend(char_range const & crFormat = char_range()) :
		to_string_backend<string_base_<C, TTraits>>(crFormat) {
	}
};

} //namespace abc


namespace std {

// Specialization of std::hash.
template <size_t t_cchStatic, typename C, class TTraits>
struct hash<abc::wsstring<t_cchStatic, C, TTraits>> :
	public hash<abc::string_base_<C, TTraits>> {
};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_STRING_HXX

