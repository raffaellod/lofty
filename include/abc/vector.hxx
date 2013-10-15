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

#include <abc/core.hxx>
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

	/** Destructor.
	*/
	~_raw_vector() {
		destruct_items(type_raw_cda<T>());
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


	/** See vector::remove_at().

	TODO: comment signature.
	*/
	void remove_at(ptrdiff_t iOffset, ptrdiff_t ciRemove) {
		_raw_complex_vextr_impl::remove_at(type_raw_cda<T>(), iOffset, ciRemove);
	}


	/** See vector::clear().
	*/
	void clear() {
		this->~_raw_vector();
		assign_empty();
	}


	/** See vector::set_capacity().

	TODO: comment signature.
	*/
	void set_capacity(size_t ciMin, bool bPreserve) {
		_raw_complex_vextr_impl::set_capacity(type_raw_cda<T>(), ciMin, bPreserve);
	}


protected:

	/** Constructor.

	TODO: comment signature.
	*/
	_raw_vector(size_t ciStaticMax) :
		_raw_complex_vextr_impl(ciStaticMax) {
	}
	_raw_vector(void const * pConstSrc, size_t ciSrc) :
		_raw_complex_vextr_impl(pConstSrc, ciSrc) {
	}
};

// Partial specialization for trivial types. Methods here ignore the bMove argument for the
// individual elements, because move semantics don’t apply (trivial values are always copied).
template <typename T>
class _raw_vector<T, true> :
	public _raw_trivial_vextr_impl {
public:

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


	/** See vector::remove_at().

	TODO: comment signature.
	*/
	void remove_at(ptrdiff_t iOffset, ptrdiff_t ciRemove) {
		_raw_trivial_vextr_impl::remove_at(sizeof(T), iOffset, ciRemove);
	}


	/** See vector::clear().
	*/
	void clear() {
		this->~_raw_vector();
		assign_empty();
	}


	/** See vector::set_capacity().

	TODO: comment signature.
	*/
	void set_capacity(size_t ciMin, bool bPreserve) {
		_raw_trivial_vextr_impl::set_capacity(sizeof(T), ciMin, bPreserve);
	}


protected:

	/** Constructor.

	TODO: comment signature.
	*/
	_raw_vector(size_t ciStaticMax) :
		_raw_trivial_vextr_impl(ciStaticMax) {
	}
	_raw_vector(void const * pConstSrc, size_t ciSrc) :
		_raw_trivial_vextr_impl(pConstSrc, ciSrc) {
	}
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::vector_base


namespace abc {

// Forward declarations.
template <typename T>
class mvector;
template <typename T>
class dmvector;


/** Base class for vectors.

See [DOC:4019 abc::*str_ and abc::*vector design] for implementation details for this and all the
*vector classes.
*/
template <typename T>
class vector_base :
	protected _raw_vector<T>,
	public _iterable_vector<vector_base<T>, T>,
	public support_explicit_operator_bool<vector_base<T>> {

	ABC_CLASS_PREVENT_COPYING(vector_base)

	/** Shortcut for the base class providing iterator-based types and methods. */
	typedef _iterable_vector<vector_base<T>, T> itvec;


public:

	/** Item type. */
	typedef T item_type;
	/** See _iterable_vector::const_iterator. */
	typedef typename itvec::const_iterator const_iterator;


public:

	/** Element access operator.

	i
		Element index.
	return
		Element at index i.
	*/
	T const & operator[](intptr_t i) const {
		this->validate_index(i);
		return data()[i];
	}


	/** Returns true if the length is greater than 0.

	return
		true if the vector is not empty, or false otherwise.
	*/
	explicit_operator_bool() const {
		return size() > 0;
	}


	/** Returns the maximum number of elements the array can currently hold.

	return
		Current size of the item array storage, in elements.
	*/
	size_t capacity() const {
		return _raw_vector<T>::capacity();
	}


	/** Returns a pointer to the item array.

	return
		Pointer to the item array.
	*/
	T * data() {
		// For some reason, GCC doesn’t like this:
		//    return _raw_vector<T>::data<T>();
		return _raw_vextr_impl_base::data<T>();
	}
	T const * data() const {
		// For some reason, GCC doesn’t like this:
		//    return _raw_vector<T>::data<T>();
		return _raw_vextr_impl_base::data<T>();
	}


	/** Returns the count of elements in the array.

	return
		Count of elements.
	*/
	size_t size() const {
		return _raw_vector<T>::size();
	}


	/** Looks for the specified value; returns the index of the first matching item, or -1 for no
	matches.

	TODO: comment signature.
	*/
	ptrdiff_t index_of(T const & t, ptrdiff_t iFirst = 0) const {
		T const * pt0(data()), * ptEnd(pt0 + size());
		for (T const * pt(pt0 + this->adjust_index(iFirst)); pt < ptEnd; ++pt) {
			if (*pt == t) {
				return pt - pt0;
			}
		}
		return -1;
	}


	/** Looks for the specified value; returns the index of the first matching item, or -1 for no
	matches.

	TODO: comment signature.
	*/
	ptrdiff_t last_index_of(T const & t) const {
		return last_index_of(t, ptrdiff_t(size()));
	}
	ptrdiff_t last_index_of(T const & t, ptrdiff_t iFirst) const {
		T const * pt0(data());
		for (T const * pt(pt0 + this->adjust_index(iFirst)); pt >= pt0; --pt) {
			if (*pt == t) {
				return pt - pt0;
			}
		}
		return -1;
	}


	/** Returns a segment of the vector.

	iFirst
		0-based index of the first element. If negative, it’s 1-based index from the end of the
		vector.
	[ci]
		Count of elements to return. If negative, it’s the count of elements to skip, from the end of
		the vector.
	*/
	dmvector<T> slice(ptrdiff_t iFirst) const {
		return slice(iFirst, size());
	}
	dmvector<T> slice(ptrdiff_t iFirst, ptrdiff_t ci) const {
		this->adjust_range(&iFirst, &ci);
		return dmvector<T>(data() + iFirst, size_t(ci));
	}


protected:

	/** Constructor.

	TODO: comment signature.
	*/
	vector_base(size_t ciStatic) :
		_raw_vector<T>(ciStatic) {
	}
	vector_base(T const * pt, size_t ci) :
		_raw_vector<T>(pt, ci) {
	}


	/** See _raw_vector<T>::assign_copy().

	TODO: comment signature.
	*/
	void assign_copy(T const * pt, size_t ci, bool bMove) {
		_raw_vector<T>::assign_copy(pt, ci, bMove);
	}
	void assign_copy(
		T const * pt1, size_t ci1, bool bMove1, T const * pt2, size_t ci2, bool bMove2
	) {
		_raw_vector<T>::assign_copy(pt1, ci1, bMove1, pt2, ci2, bMove2);
	}


	/** See _raw_vector<T>::assign_move().

	v
		Source vector.
	*/
	void assign_move(vector_base && v) {
		_raw_vector<T>::assign_move(static_cast<_raw_vector<T> &&>(v));
	}


	/** See _raw_vector<T>::assign_move_dynamic_or_copy().

	v
		Source vector.
	*/
	void assign_move_dynamic_or_copy(vector_base && v) {
		_raw_vector<T>::assign_move_dynamic_or_copy(static_cast<_raw_vector<T> &&>(v));
	}
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::mvector


namespace abc {

/** vector_base-derived class, to be used as argument type for functions that want to modify a
vector argument, since this allows for in-place alterations to the vector. Both smvector and
dmvector are automatically converted to this.
*/
template <typename T>
class mvector :
	public vector_base<T> {

	/** Shortcut for the base class providing iterator-based types and methods. */
	typedef _iterable_vector<vector_base<T>, T> itvec;


public:

	/** See vector_base<T>::const_iterator. */
	typedef typename itvec::const_iterator const_iterator;


public:

	/** Assignment operator.

	v
		Source vector.
	return
		*this.
	*/
	mvector & operator=(mvector const & v) {
		this->assign_copy(v.data(), v.size(), false);
		return *this;
	}
	mvector & operator=(dmvector<T> && v) {
		this->assign_move(std::move(v));
		return *this;
	}


	/** Concatenation-assignment operator.

	v
		Vector to concatenate.
	return
		*this.
	*/
	mvector & operator+=(mvector const & v) {
		append(v.data(), v.size(), false);
		return *this;
	}
	mvector & operator+=(mvector && v) {
		append(v.data(), v.size(), true);
		return *this;
	}


	/** See vector_base::operator[]().
	*/
	T & operator[](intptr_t i) {
		return const_cast<T &>(vector_base<T>::operator[](i));
	}
	T const & operator[](intptr_t i) const {
		return vector_base<T>::operator[](i);
	}


	/** Adds elements at the end of the vector.

	TODO: comment signature.
	*/
	void append(T const & t) {
		append(const_cast<typename std::remove_const<T>::type *>(&t), 1, false);
	}
	void append(typename std::remove_const<T>::type && t) {
		append(&t, 1, true);
	}
	void append(T const * pt, size_t ci) {
		vector_base<T>::append(pt, ci, false);
	}
	void append(typename std::remove_const<T>::type * pt, size_t ci, bool bMove) {
		vector_base<T>::append(pt, ci, bMove);
	}


	/** Removes all the elements in the vector.
	*/
	void clear() {
		vector_base<T>::clear();
	}


	/** Inserts elements at a specific position in the vector.

	iFirst
		0-based index of the element. If negative, it’s 1-based index from the end of the vector.
	*/
	void insert(ptrdiff_t i, T const & t) {
		insert(i, &t, 1);
	}
	void insert(ptrdiff_t i, typename std::remove_const<T>::type && t) {
		insert(i, &t, 1, true);
	}
	void insert(ptrdiff_t i, T const * pt, size_t ci) {
		vector_base<T>::insert(i, pt, ci, false);
	}
	void insert(ptrdiff_t i, typename std::remove_const<T>::type * pt, size_t ci, bool bMove) {
		vector_base<T>::insert(i, pt, ci, bMove);
	}
	void insert(const_iterator it, T const & t) {
		insert(it - itvec::cbegin(), t);
	}
	void insert(const_iterator it, typename std::remove_const<T>::type && t) {
		insert(it - itvec::cbegin(), std::move(t));
	}
	void insert(const_iterator it, T const * pt, size_t ci) {
		insert(it - itvec::cbegin(), pt, ci);
	}
	void insert(const_iterator it, typename std::remove_const<T>::type * pt, size_t ci, bool bMove) {
		insert(it - itvec::cbegin(), pt, ci, bMove);
	}


	/** Removes elements from the vector.

	i
		0-based index of the element to be removed. If negative, it’s 1-based index from the end of
		the vector.
	*/
	void remove_at(ptrdiff_t i, ptrdiff_t ciRemove = 1) {
		vector_base<T>::remove_at(i, ciRemove);
	}
	void remove_at(const_iterator it, ptrdiff_t ciRemove = 1) {
		vector_base<T>::remove_at(it - itvec::cbegin(), ciRemove);
	}


	/** Ensures that the item array has at least cchMin of actual item space, excluding the trailing
	NUL character. If this causes *this to switch to using a different item array, any data in the
	current one will be lost unless bPreserve == true.

	TODO: comment signature.
	*/
	void set_capacity(size_t ciMin, bool bPreserve) {
		vector_base<T>::set_capacity(ciMin, bPreserve);
	}


	/** Resizes the vector so that it only takes up as much memory as strictly necessary.
	*/
	void shrink_to_fit() {
		// TODO: implement this.
	}


protected:

	/** Constructor.

	TODO: comment signature.
	*/
	mvector(size_t ciStaticMax) :
		vector_base<T>(ciStaticMax) {
	}
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::dmvector


namespace abc {

/** Dynamically-allocated mutable vector.
*/
template <typename T>
class dmvector :
	public mvector<T> {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	dmvector() :
		mvector<T>(0) {
	}
	dmvector(dmvector const & v) :
		mvector<T>(0) {
		this->assign_copy(v.data(), v.size(), false);
	}
	dmvector(dmvector && v) :
		mvector<T>(0) {
		this->assign_move(std::move(v));
	}
	dmvector(mvector<T> const & v) :
		mvector<T>(0) {
		this->assign_copy(v.data(), v.size(), false);
	}
	// This can throw exceptions, but it’s allowed to since it’s not the dmvector && overload.
	dmvector(mvector<T> && v) :
		mvector<T>(0) {
		this->assign_move_dynamic_or_copy(std::move(v));
	}
	dmvector(mvector<T> const & v1, mvector<T> const & v2) :
		mvector<T>(0) {
		this->assign_copy(v1.data(), v1.size(), false, v2.data(), v2.size(), false);
	}
	dmvector(mvector<T> && v1, mvector<T> const & v2) :
		mvector<T>(0) {
		this->assign_copy(v1.data(), v1.size(), true, v2.data(), v2.size(), false);
	}
	dmvector(mvector<T> const & v1, mvector<T> && v2) :
		mvector<T>(0) {
		this->assign_copy(v1.data(), v1.size(), false, v2.data(), v2.size(), true);
	}
	dmvector(mvector<T> && v1, mvector<T> && v2) :
		mvector<T>(0) {
		this->assign_copy(v1.data(), v1.size(), true, v2.data(), v2.size(), true);
	}
	template <size_t t_ci>
	explicit dmvector(T const (& at)[t_ci]) :
		mvector<T>(0) {
		this->assign_copy(at, t_ci, false);
	}
	dmvector(T const * pt, size_t ci) :
		mvector<T>(0) {
		this->assign_copy(pt, ci, false);
	}
	dmvector(T const * pt1, size_t ci1, T const * pt2, size_t ci2) :
		mvector<T>(0) {
		this->assign_copy(pt1, ci1, false, pt2, ci2, false);
	}


	/** Assignment operator.

	v
		Source vector.
	return
		*this;
	*/
	dmvector & operator=(dmvector const & v) {
		this->assign_copy(v.data(), v.size(), false);
		return *this;
	}
	dmvector & operator=(dmvector && v) {
		this->assign_move(std::move(v));
		return *this;
	}
	dmvector & operator=(mvector<T> const & v) {
		this->assign_copy(v.data(), v.size(), false);
		return *this;
	}
	// This can throw exceptions, but it’s allowed to since it’s not the dmvector && overload.
	dmvector & operator=(mvector<T> && v) {
		this->assign_move_dynamic_or_copy(std::move(v));
		return *this;
	}
};

} //namespace abc


/** Concatenation operator.

v1
	Left operand.
v2
	Right operand.
return
	Vector resulting from the concatenation of v1 and v2.
*/
template <typename T>
inline abc::dmvector<T> operator+(abc::vector_base<T> const & v1, abc::vector_base<T> const & v2) {
	return abc::dmvector<T>(
		static_cast<abc::mvector<T> const &>(v1), static_cast<abc::mvector<T> const &>(v2)
	);
}
// Overloads taking an mvector rvalue-reference as either or both operands; they can avoid creating
// intermediate copies of the elements from one or both source vectors.
// TODO: verify that compilers actually select these overloads whenever possible.
template <typename T>
inline abc::dmvector<T> operator+(abc::mvector<T> && v1, abc::mvector<T> const & v2) {
	return abc::dmvector<T>(std::move(v1), v2);
}
template <typename T>
inline abc::dmvector<T> operator+(abc::mvector<T> const & v1, abc::mvector<T> && v2) {
	return abc::dmvector<T>(v1, std::move(v2));
}
template <typename T>
inline abc::dmvector<T> operator+(abc::mvector<T> && v1, abc::mvector<T> && v2) {
	return abc::dmvector<T>(std::move(v1), std::move(v2));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::smvector


namespace abc {

template <typename T, size_t t_ciStatic>
class smvector :
	public mvector<T> {
protected:

	/** Actual static item array size. */
	static size_t const smc_ciFixed = _ABC__RAW_VEXTR_IMPL_BASE__ADJUST_ITEM_COUNT(t_ciStatic);


public:

	/** Constructor.

	TODO: comment signature.
	*/
	smvector() :
		mvector<T>(smc_ciFixed) {
	}
	smvector(smvector const & v) :
		mvector<T>(smc_ciFixed) {
		this->assign_copy(v.data(), v.size(), false);
	}
	// If the source is using its static item array, it will be copied without allocating a dynamic
	// one; if the source is dynamic, it will be moved. Either way, this won’t throw.
	smvector(smvector && v) :
		mvector<T>(smc_ciFixed) {
		this->assign_move_dynamic_or_copy(std::move(v));
	}
	// If the source is using its static item array, it will be copied without allocating a dynamic
	// one since it’s smaller than this object’s; if the source is dynamic, it will be moved. Either
	// way, this won’t throw.
	template <size_t t_ciStatic2>
	smvector(
		typename std::enable_if<(t_ciStatic > t_ciStatic2), smvector<T, t_ciStatic2> &&>::type v
	) :
		mvector<T>(smc_ciFixed) {
		this->assign_move_dynamic_or_copy(std::move(v));
	}
	smvector(mvector<T> const & v) :
		mvector<T>(smc_ciFixed) {
		this->assign_copy(v.data(), v.size(), false);
	}
	// This can throw exceptions, but it’s allowed to since it’s not the smvector && overload.
	// This also covers smvector of different static size > t_ciStatic.
	smvector(mvector<T> && v) :
		mvector<T>(smc_ciFixed) {
		this->assign_move_dynamic_or_copy(std::move(v));
	}
	smvector(dmvector<T> && v) :
		mvector<T>(smc_ciFixed) {
		this->assign_move(std::move(v));
	}
	template <size_t t_ci>
	explicit smvector(T const (& at)[t_ci]) :
		mvector<T>(smc_ciFixed) {
		this->assign_copy(at, t_ci, false);
	}
	smvector(T const * pt, size_t ci) :
		mvector<T>(smc_ciFixed) {
		this->assign_copy(pt, ci, false);
	}


	/** Assignment operator.

	TODO: comment signature.
	*/
	smvector & operator=(smvector const & v) {
		this->assign_copy(v.data(), v.size(), false);
		return *this;
	}
	// If the source is using its static item array, it will be copied without allocating a dynamic
	// one; if the source is dynamic, it will be moved. Either way, this won’t throw.
	smvector & operator=(smvector && v) {
		this->assign_move_dynamic_or_copy(std::move(v));
		return *this;
	}
	// If the source is using its static item array, it will be copied without allocating a dynamic
	// one since it’s smaller than this object’s; if the source is dynamic, it will be moved. Either
	// way, this won’t throw.
	template <size_t t_ciStatic2>
	smvector & operator=(
		typename std::enable_if<(t_ciStatic > t_ciStatic2), smvector<T, t_ciStatic2> &&>::type v
	) {
		this->assign_move_dynamic_or_copy(std::move(v));
		return *this;
	}
	smvector & operator=(mvector<T> const & v) {
		this->assign_copy(v.data(), v.size(), false);
		return *this;
	}
	// This can throw exceptions, but it’s allowed to since it’s not the smvector && overload.
	// This also covers smvector of different static size > t_ciStatic.
	smvector & operator=(mvector<T> && v) {
		this->assign_move_dynamic_or_copy(std::move(v));
		return *this;
	}
	smvector & operator=(dmvector<T> && v) {
		this->assign_move(std::move(v));
		return *this;
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

