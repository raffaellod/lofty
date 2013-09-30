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

#ifndef ABC_VECTOR_HXX
#define ABC_VECTOR_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/_vextr.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_vector

namespace abc {

/** Thin templated wrapper for _raw_*_vextr_impl, so make the interface of those two classes
consistent, so vector doesn’t need specializations.
*/
template <typename T, bool t_bTrivial = std::is_trivial<T>::value>
class _raw_vector;

// Partial specialization for non-trivial types.
template <typename T>
class _raw_vector<T, false> :
	public _raw_complex_vextr_impl {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	_raw_vector(size_t ciStaticMax) :
		_raw_complex_vextr_impl(ciStaticMax) {
	}
	_raw_vector(void const * pConstSrc, size_t ciSrc) :
		_raw_complex_vextr_impl(pConstSrc, ciSrc) {
	}


	/** Destructor.
	*/
	~_raw_vector() {
		_raw_complex_vextr_impl::destruct_items(type_raw_cda<T>());
	}


	/** See vector::append().

	TODO: comment signature.
	*/
	void append(T const * pAdd, size_t ciAdd, bool bMove) {
		_raw_complex_vextr_impl::append(type_raw_cda<T>(), pAdd, ciAdd, bMove);
	}


	/** See vector::assign_copy().

	TODO: comment signature.
	*/
	void assign_copy(T const * p, size_t ci, bool bMove) {
		_raw_complex_vextr_impl::assign_copy(type_raw_cda<T>(), p, ci, bMove);
	}
	void assign_copy(T const * p1, size_t ci1, bool bMove1, T const * p2, size_t ci2, bool bMove2) {
		_raw_complex_vextr_impl::assign_copy(type_raw_cda<T>(), p1, ci1, bMove1, p2, ci2, bMove2);
	}


	/** See vector::assign_move().

	TODO: comment signature.
	*/
	void assign_move(_raw_complex_vextr_impl && rcvi) {
		_raw_complex_vextr_impl::assign_move(type_raw_cda<T>(), std::move(rcvi));
	}


	/** See vector::insert().

	TODO: comment signature.
	*/
	void insert(ptrdiff_t iOffset, T const * pAdd, size_t ciAdd, bool bMove) {
		_raw_complex_vextr_impl::insert(type_raw_cda<T>(), iOffset, pAdd, ciAdd, bMove);
	}


	/** See vector::remove().

	TODO: comment signature.
	*/
	void remove(ptrdiff_t iOffset, ptrdiff_t ciRemove) {
		_raw_complex_vextr_impl::remove(type_raw_cda<T>(), iOffset, ciRemove);
	}


#if 0
	/** See vector::remove_all().

	TODO: comment signature.
	*/
	void remove_all() {
		this->~_raw_vector();
		construct_empty();
	}
#endif


	/** See vector::set_capacity().

	TODO: comment signature.
	*/
	void set_capacity(size_t ciMin, bool bPreserve) {
		_raw_complex_vextr_impl::set_capacity(type_raw_cda<T>(), ciMin, bPreserve);
	}
};

// Partial specialization for trivial types. Methods here ignore the bMove argument for the
// individual elements, because the related semantic doesn’t apply.
template <typename T>
class _raw_vector<T, true> :
	public _raw_trivial_vextr_impl {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	_raw_vector(size_t ciStaticMax) :
		_raw_trivial_vextr_impl(ciStaticMax) {
	}
	_raw_vector(void const * pConstSrc, size_t ciSrc) :
		_raw_trivial_vextr_impl(pConstSrc, ciSrc) {
	}


	/** See vector::append(). This specialization ignores completely the bMove argument, since
	trivial types can only be copied.

	TODO: comment signature.
	*/
	void append(void const * pAdd, size_t ciAdd, bool bMove) {
		UNUSED_ARG(bMove);
		_raw_trivial_vextr_impl::append(sizeof(T), pAdd, ciAdd);
	}


	/** See _raw_trivial_vextr_impl::assign_copy().

	TODO: comment signature.
	*/
	void assign_copy(void const * p, size_t ci, bool bMove) {
		UNUSED_ARG(bMove);
		_raw_trivial_vextr_impl::assign_copy(sizeof(T), p, ci);
	}
	void assign_copy(
		void const * p1, size_t ci1, bool bMove1, void const * p2, size_t ci2, bool bMove2
	) {
		UNUSED_ARG(bMove1);
		UNUSED_ARG(bMove2);
		_raw_trivial_vextr_impl::assign_copy(sizeof(T), p1, ci1, p2, ci2);
	}


	/** See _raw_trivial_vextr_impl::assign_move().

	TODO: comment signature.
	*/
	void assign_move(_raw_trivial_vextr_impl && rtvi) {
		_raw_trivial_vextr_impl::assign_move(std::move(rtvi));
	}


	/** See vector::insert(). This specialization ignores completely the bMove argument, since
	trivial types can only be copied.

	TODO: comment signature.
	*/
	void insert(ptrdiff_t iOffset, void const * pAdd, size_t ciAdd, bool bMove) {
		UNUSED_ARG(bMove);
		_raw_trivial_vextr_impl::insert(sizeof(T), iOffset, pAdd, ciAdd);
	}


	/** See vector::remove().

	TODO: comment signature.
	*/
	void remove(ptrdiff_t iOffset, ptrdiff_t ciRemove) {
		_raw_trivial_vextr_impl::remove(sizeof(T), iOffset, ciRemove);
	}


#if 0
	/** See vector::remove_all().

	TODO: comment signature.
	*/
	void remove_all() {
		this->~_raw_vector();
		construct_empty();
	}
#endif


	/** See vector::set_capacity().

	TODO: comment signature.
	*/
	void set_capacity(size_t ciMin, bool bPreserve) {
		_raw_trivial_vextr_impl::set_capacity(sizeof(T), ciMin, bPreserve);
	}
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::buffered_vector


namespace abc {

/** TODO: description.
*/
template <typename T, size_t t_ciStatic = 0>
class vector;


/** TODO: description.
*/
template <typename T>
class buffered_vector :
	protected _raw_vector<T>,
	public _iterable_vector<buffered_vector<T>, T>,
	public support_explicit_operator_bool<buffered_vector<T>> {

	typedef _raw_vector<T> raw_vector;

protected:

	typedef _iterable_vector<buffered_vector<T>, T> iterable_vector;
	typedef vector<T, 0> vector0;
	typedef typename std::remove_const<T>::type Tnc;

public:

	/** Assignment operator.

	TODO: comment signature.
	*/
	buffered_vector & operator=(buffered_vector const & bv) {
		raw_vector::assign_copy(bv.data(), bv.size(), false);
		return *this;
	}
	buffered_vector & operator=(buffered_vector && v) {
		raw_vector::assign_move(std::move(v));
		return *this;
	}
	template <size_t t_ci>
	buffered_vector & operator=(T const (& at)[t_ci]) {
		raw_vector::assign_copy(at, t_ci, false);
		return *this;
	}


	/** Concatenation-assignment operator.

	TODO: comment signature.
	*/
	buffered_vector & operator+=(T const & t) {
		append(&t, 1, false);
		return *this;
	}
	buffered_vector & operator+=(Tnc && t) {
		append(&t, 1, true);
		return *this;
	}
	template <size_t t_ci>
	buffered_vector & operator+=(T const (& at)[t_ci]) {
		append(at, t_ci, false);
		return *this;
	}
	buffered_vector & operator+=(vector0 const & t) {
		append(t.data(), t.size(), false);
		return *this;
	}


	/** Concatenation operator.

	TODO: comment signature.
	*/
	vector0 operator+(T const & t) const {
		return vector0(data(), size(), &t, 1);
	}
	template <size_t t_ci>
	vector0 operator+(T const (& at)[t_ci]) const {
		return vector0(data(), size(), at, t_ci);
	}
	vector0 operator+(vector0 const & t) const {
		return vector0(data(), size(), t.data(), t.size());
	}


	/** Element access operator.

	TODO: comment signature.
	*/
	T & operator[](size_t i) {
		if (i >= size()) {
			abc_throw(index_error(intptr_t(i)));
		}
		return data()[i];
	}
	T const & operator[](size_t i) const {
		if (i >= size()) {
			abc_throw(index_error(intptr_t(i)));
		}
		return data()[i];
	}


	/** Returns true if the length is greater than 0.

	TODO: comment signature.
	*/
	explicit_operator_bool() const {
		return size() > 0;
	}


	/** Allows automatic cross-class-hierarchy casts. Notice the lack of a non-const overload: this
	prevents ending in a situation where client code tries to std::move() a vector that’s really a
	buffered_vector, to a vector that is not, which could result in exceptions being thrown.

	TODO: comment signature.
	*/
	operator vector0 const &() const {
		return *static_cast<vector0 const *>(this);
	}


	/** Adds elements at the end of the vector.

	TODO: comment signature.
	*/
	void append(T const & t) {
		append(const_cast<Tnc *>(&t), 1, false);
	}
	void append(Tnc && t) {
		append(&t, 1, true);
	}
	void append(T const * pt, size_t ci) {
		raw_vector::append(pt, ci, false);
	}
	void append(Tnc * pt, size_t ci, bool bMove) {
		raw_vector::append(pt, ci, bMove);
	}


	/** Returns the maximum number of elements the array can currently hold.

	TODO: comment signature.
	*/
	size_t capacity() const {
		return raw_vector::capacity();
	}


	/** Returns a pointer to the item array.

	TODO: comment signature.
	*/
	T * data() {
		// For some reason, gcc doesn’t like this:
		//    return raw_vector::data<T>();
		return _raw_vextr_impl_base::data<T>();
	}
	T const * data() const {
		// For some reason, gcc doesn’t like this:
		//    return raw_vector::data<T>();
		return _raw_vextr_impl_base::data<T>();
	}


	/** Returns the count of elements in the array.

	TODO: comment signature.
	*/
	size_t size() const {
		return raw_vector::size();
	}


	/** Looks for the specified value; returns the index of the first matching item, or -1 for no
	matches.

	TODO: comment signature.
	*/
	ptrdiff_t index_of(T const & t, ptrdiff_t iFirst = 0) const {
		T const * pt0(data()), * ptEnd(pt0 + size());
		for (T const * pt(pt0 + raw_vector::adjust_index(iFirst)); pt < ptEnd; ++pt) {
			if (*pt == t) {
				return pt - pt0;
			}
		}
		return -1;
	}


	/** Inserts elements at a specific position in the vector.

	iFirst
		0-based index of the element. If negative, it’s 1-based index from the end of the vector.
	*/
	void insert(ptrdiff_t i, T const & t) {
		insert(i, &t, 1);
	}
	void insert(ptrdiff_t i, Tnc && t) {
		insert(i, &t, 1, true);
	}
	void insert(ptrdiff_t i, T const * pt, size_t ci) {
		raw_vector::insert(i, pt, ci, false);
	}
	void insert(ptrdiff_t i, Tnc * pt, size_t ci, bool bMove) {
		raw_vector::insert(i, pt, ci, bMove);
	}
	void insert(typename iterable_vector::iterator it, T const & t) {
		insert(it - iterable_vector::cbegin(), t);
	}
	void insert(typename iterable_vector::iterator it, Tnc && t) {
		insert(it - iterable_vector::cbegin(), std::move(t));
	}
	void insert(typename iterable_vector::iterator it, T const * pt, size_t ci) {
		insert(it - iterable_vector::cbegin(), pt, ci);
	}
	void insert(typename iterable_vector::iterator it, Tnc * pt, size_t ci, bool bMove) {
		insert(it - iterable_vector::cbegin(), pt, ci, bMove);
	}


	/** Looks for the specified value; returns the index of the first matching item, or -1 for no
	matches.

	TODO: comment signature.
	*/
	ptrdiff_t last_index_of(T const & t) const {
		return last_index_of(t, size());
	}
	ptrdiff_t last_index_of(T const & t, ptrdiff_t iFirst) const {
		T const * pt0(data());
		for (T const * pt(pt0 + raw_vector::adjust_index(iFirst)); pt >= pt0; --pt) {
			if (*pt == t) {
				return pt - pt0;
			}
		}
		return -1;
	}


	/** Removes elements from the vector.

	i
		0-based index of the element to be removed. If negative, it’s 1-based index from the end of
		the vector.
	*/
	void remove(ptrdiff_t i, ptrdiff_t ciRemove = 1) {
		raw_vector::remove(i, ciRemove);
	}
	void remove(typename iterable_vector::iterator it, ptrdiff_t ciRemove = 1) {
		raw_vector::remove(it - iterable_vector::cbegin(), ciRemove);
	}


	/** Removes all the elements in the vector.

	TODO: comment signature.
	*/
	void remove_all() {
		raw_vector::remove_all();
	}


	/** Ensures that the item array has at least cchMin of actual item space, excluding the trailing
	NUL character. If this causes *this to switch to using a different item array, any data in the
	current one will be lost unless bPreserve == true.

	TODO: comment signature.
	*/
	void set_capacity(size_t ciMin, bool bPreserve) {
		raw_vector::set_capacity(ciMin, bPreserve);
	}


	/** Resizes the vector so that it only takes up as much memory as strictly necessary.

	TODO: comment signature.
	*/
	void shrink_to_fit() {
		// TODO: implement this.
	}


	/** Returns a segment of the vector.

	iFirst
		0-based index of the first element. If negative, it’s 1-based index from the end of the
		vector.
	[ci]
		Count of elements to return. If negative, it’s the count of elements to skip, from the end of
		the vector.
	*/
	vector0 slice(ptrdiff_t iFirst) const {
		return slice(iFirst, size());
	}
	vector0 slice(ptrdiff_t iFirst, ptrdiff_t ci) const {
		raw_vector::adjust_range(&iFirst, &ci);
		return vector0(data() + iFirst, size_t(ci));
	}


protected:

	/** Constructor.

	TODO: comment signature.
	*/
	buffered_vector(size_t cchStaticMax) :
		raw_vector(cchStaticMax) {
	}
	buffered_vector(size_t cchStaticMax, vector0 const & v) :
		raw_vector(cchStaticMax) {
		operator=(v);
	}
	buffered_vector(size_t cchStaticMax, vector0 && v) :
		raw_vector(cchStaticMax) {
		operator=(std::move(v));
	}
	// TODO: enable_if<is_trivial> to just adopt the array like base_str_ does, instead of copying
	// it?
	template <size_t t_ci>
	buffered_vector(size_t cchStaticMax, T const (& at)[t_ci]) :
		raw_vector(cchStaticMax) {
		operator=(at);
	}
	buffered_vector(size_t cchStaticMax, T const * pt, size_t ci) :
		raw_vector(cchStaticMax) {
		assign_copy(pt, ci, false);
	}
	buffered_vector(size_t cchStaticMax, T const * pt1, size_t ci1, T const * pt2, size_t ci2) :
		raw_vector(cchStaticMax) {
		assign_copy(pt1, ci1, false, pt2, ci2, false);
	}
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::vector


namespace abc {

// Generic specialization (no, it’s not an oxymoron), without embedded descriptor.
template <typename T>
class vector<T, 0> :
	public buffered_vector<T> {

	typedef typename buffered_vector<T>::iterable_vector iterable_vector;
	typedef typename buffered_vector<T>::Tnc Tnc;

public:

	/** Constructor.

	TODO: comment signature.
	*/
	vector() :
		buffered_vector<T>(0) {
	}
	vector(vector const & v) :
		buffered_vector<T>(0, v) {
	}
	vector(vector && v) :
		buffered_vector<T>(0, std::move(v)) {
	}
	template <size_t t_ci>
	explicit vector(T const (& at)[t_ci]) :
		buffered_vector<T>(0, at) {
	}
	vector(T const * pt, size_t ci) :
		buffered_vector<T>(0, pt, ci) {
	}
	vector(T const * pt1, size_t ci1, T const * pt2, size_t ci2) :
		buffered_vector<T>(0, pt1, ci1, pt2, ci2) {
	}


	/** Assignment operator.

	TODO: comment signature.
	*/
	vector & operator=(vector const & v) {
		buffered_vector<T>::operator=(v);
		return *this;
	}
	vector & operator=(vector && v) {
		buffered_vector<T>::operator=(std::move(v));
		return *this;
	}
	template <size_t t_ci>
	vector & operator=(T const (& at)[t_ci]) {
		buffered_vector<T>::operator=(at);
		return *this;
	}


	/** Concatenation-assignment operator.

	TODO: comment signature.
	*/
	vector & operator+=(T const & t) {
		buffered_vector<T>::operator+=(t);
		return *this;
	}
	vector & operator+=(Tnc && t) {
		buffered_vector<T>::operator+=(std::move(t));
		return *this;
	}
	template <size_t t_ci>
	vector & operator+=(T const (& at)[t_ci]) {
		buffered_vector<T>::operator+=(at);
		return *this;
	}
	vector & operator+=(vector const & t) {
		buffered_vector<T>::operator+=(t);
		return *this;
	}
};

// Version with embedded descriptor.
template <typename T, size_t t_ciStatic>
class vector :
	public buffered_vector<T> {

	typedef typename buffered_vector<T>::Tnc Tnc;
	typedef typename buffered_vector<T>::vector0 vector0;

protected:

	/** Actual static item array size. */
	static size_t const smc_ciFixed = _ABC__RAW_VEXTR_IMPL_BASE__ADJUST_ITEM_COUNT(t_ciStatic);


public:

	/** Constructor.

	TODO: comment signature.
	*/
	vector() :
		buffered_vector<T>(smc_ciFixed) {
	}
	vector(vector const & v) :
		buffered_vector<T>(smc_ciFixed, v) {
	}
	vector(vector && v) :
		buffered_vector<T>(smc_ciFixed, std::move(v)) {
	}
	vector(buffered_vector<T> const & bv) :
		buffered_vector<T>(smc_ciFixed, bv) {
	}
	vector(vector0 && v) :
		buffered_vector<T>(smc_ciFixed, std::move(v)) {
	}
	template <size_t t_ci>
	explicit vector(T const (& at)[t_ci]) :
		buffered_vector<T>(smc_ciFixed, at) {
	}
	vector(T const * pt, size_t ci) :
		buffered_vector<T>(smc_ciFixed, pt, ci) {
	}


	/** Assignment operator.

	TODO: comment signature.
	*/
	vector & operator=(vector const & v) {
		buffered_vector<T>::operator=(v);
		return *this;
	}
	vector & operator=(vector && v) {
		buffered_vector<T>::operator=(std::move(v));
		return *this;
	}
	vector & operator=(buffered_vector<T> const & bv) {
		buffered_vector<T>::operator=(bv);
		return *this;
	}
	vector & operator=(vector0 && v) {
		buffered_vector<T>::operator=(std::move(v));
		return *this;
	}
	template <size_t t_ci>
	vector & operator=(T const (& at)[t_ci]) {
		buffered_vector<T>::operator=(at);
		return *this;
	}


	/** Concatenation-assignment operator.

	TODO: comment signature.
	*/
	vector & operator+=(T const & t) {
		buffered_vector<T>::operator+=(t);
		return *this;
	}
	vector & operator+=(Tnc && t) {
		buffered_vector<T>::operator+=(std::move(t));
		return *this;
	}
	template <size_t t_ci>
	vector & operator+=(T const (& at)[t_ci]) {
		buffered_vector<T>::operator+=(at);
		return *this;
	}
	vector & operator+=(vector const & t) {
		buffered_vector<T>::operator+=(t);
		return *this;
	}


	/** Allows automatic cross-class-hierarchy casts. Notice the lack of a non-const overload: this
	prevents ending in a situation where client code tries to std::move() a string that’s really a
	smstr, to a string that is not, which could result in exceptions being thrown.

	TODO: comment signature.
	*/
	operator vector0 const &() const {
		return *static_cast<vector0 const *>(static_cast<buffered_vector<T> const *>(this));
	}


private:

	// This section must match exactly _raw_vextr_impl_base_with_static_item_array.

	/** See _raw_vextr_impl_base_with_static_item_array::m_ciStaticMax. */
	size_t m_ciStaticMax;
	/** See _raw_vextr_impl_base_with_static_item_array::m_at. */
	std::max_align_t m_at[ABC_ALIGNED_SIZE(sizeof(T) * smc_ciFixed)];
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_VECTOR_HXX

