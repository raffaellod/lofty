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

#ifndef ABC_MAP_HXX
#define ABC_MAP_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/_map_base.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// Declarations

namespace abc {

/// Thin templated wrapper for _raw_*_map_impl, so make the interface of those two classes
// consistent, so _map_impl doesn’t need specializations.
template <
	typename TKey, typename TVal, bool t_bTrivial = false /*std::is_trivial<T>::value*/
>
struct _raw_map_impl;

/// Implementation of map.
template <typename TKey, typename TVal, size_t t_ceStatic>
class _map_impl;

/// Map with fast lookup. Implements commit-or-rollback semantics.
template <typename TKey, typename TVal, size_t t_ceStatic = 0>
class map;

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_map_impl


namespace abc {

// Partial specialization for non-trivial types.
template <typename TKey, typename TVal>
struct _raw_map_impl<TKey, TVal, false> :
	public _raw_complex_map_impl {


	/// Adds a key/value pair to the map. Adding an item with a key that already exists in the map
	// (thus just replacing the value) is guaranteed not to invalidate any iterator.
	//
	void add(TKey const * pkey, size_t hash, TVal const * pval, bool bMoveKey, bool bMoveVal) {
		_raw_complex_map_impl::add(
			type_raw_cda<TKey>(), type_raw_cda<TVal>(), pkey, hash, pval, bMoveKey, bMoveVal
		);
	}


	/// Copies or moves the contents of the source to *this according to the source type:
	// - _raw_map_root const &: copy descriptor
	// - _raw_map_root &&: move descriptor or (move items + empty source map)
	//
	void assign(_raw_map_root const & rmrSrc, bool bMove) {
		_raw_complex_map_impl::assign(type_raw_cda<TKey>(), type_raw_cda<TVal>(), rmrSrc, bMove);
	}


	/// Returns a pointer to the value associated to the specified key. If the key could not be
	// found, an exception is thrown.
	//
	TVal * get_value(TKey const * pkey, size_t hash) {
		void_cda const & typeKey(type_raw_cda<TKey>());
		return static_cast<TVal *>(_raw_complex_map_impl::get_value(
			typeKey.cb, sizeof(TVal), typeKey.equal, pkey, hash
		));
	}
	TVal const * get_value(TKey const * pkey, size_t hash) const {
		return const_cast<_raw_map_impl *>(this)->get_value(pkey, hash);
	}


	/// Destructs every key and value in the descriptor, then releases it.
	//
	void release_desc() {
		_raw_complex_map_impl::release_desc(type_raw_cda<TKey>(), type_raw_cda<TVal>());
	}


	/// Deletes a key/value pair.
	//
	void remove(TKey const * pkey, size_t hash) {
		_raw_complex_map_impl::remove(type_raw_cda<TKey>(), type_raw_cda<TVal>(), pkey, hash);
	}


	/// Removes all items from the map.
	//
	void remove_all() {
		_raw_complex_map_impl::remove_all(type_raw_cda<TKey>(), type_raw_cda<TVal>());
	}


	/// Inserts a new key/value pair into the map. If the key already exist, the corresponding value
	// is replaced.
	//
	size_t set_item(
		TKey const * pkey, size_t hash, TVal const * pval, bool bMoveKey, bool bMoveVal
	) {
		return _raw_complex_map_impl::set_item(
			type_raw_cda<TKey>(), type_raw_cda<TVal>(), pkey, hash, pval, bMoveKey, bMoveVal
		);
	}
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_map_impl


namespace abc {

template <typename TKey, typename TVal, size_t t_ceStatic>
class _map_impl :
	public _raw_map_data,
	public support_explicit_operator_bool<_map_impl<TKey, TVal, t_ceStatic>> {

	typedef map<TKey, TVal, 0> map0;
	typedef map<TKey, TVal, t_ceStatic> TMap;

protected:

	typedef typename std::remove_const<TKey>::type TKnc;
	typedef typename std::remove_const<TVal>::type TVnc;

public:

	/// Destructor.
	//
	~_map_impl() {
		_raw_map_cast()->release_desc();
	}


	/// Assignment operator.
	//
	TMap & operator=(map0 const & m) {
		assign(m);
		return *static_cast<TMap *>(this);
	}
	TMap & operator=(map0 && m) {
		assign(std::move(m));
		return *static_cast<TMap *>(this);
	}
	template <size_t t_ceStatic2>
	TMap & operator=(map<TKey, TVal, t_ceStatic2> && m) {
		assign(std::move(m));
		return *static_cast<TMap *>(this);
	}


	/// Provides access to the individual items making up the array.
	//
	TVnc & operator[](TKey const & key) {
		TVal * pval(_raw_map_cast()->get_value(&key, key_hash(key)));
		return *pval;
	}
	TVal const & operator[](TKey const & key) const {
		return const_cast<_map_impl *>(this)->operator[](key);
	}


	/// Returns true if the map contains at least one item.
	//
	explicit_operator_bool() const {
		return get_size() > 0;
	}


	/// Adds a key/value pair to the map. Adding an item with a key that already exists in the map
	// (thus just replacing the value) is guaranteed not to invalidate any iterator.
	//
	TMap & add(TKey const & key, TVal const & val) {
		_raw_map_cast()->add(&key, key_hash(key), &val, false, false);
		return *static_cast<TMap *>(this);
	}
	TMap & add(TKey const & key, TVnc && val) {
		_raw_map_cast()->add(&key, key_hash(key), &val, false, true);
		return *static_cast<TMap *>(this);
	}
	TMap & add(TKnc && key, TVal const & val) {
		_raw_map_cast()->add(&key, key_hash(key), &val, true, false);
		return *static_cast<TMap *>(this);
	}
	TMap & add(TKnc && key, TVnc && val) {
		_raw_map_cast()->add(&key, key_hash(key), &val, true, true);
		return *static_cast<TMap *>(this);
	}


	/// Returns the number of items in the map.
	//
	size_t get_size() const {
		return _raw_map_cast()->get_size();
	}


	/// Returns a _raw_map wrapper for the _raw_map_data wrapped by this map.
	//
	_raw_map_impl<TKey, TVal> * _raw_map_cast() {
		return static_cast<_raw_map_impl<TKey, TVal> *>(static_cast<_raw_map_data *>(this));
	}
	_raw_map_impl<TKey, TVal> const * _raw_map_cast() const {
		return static_cast<_raw_map_impl<TKey, TVal> const *>(
			static_cast<_raw_map_data const *>(this)
		);
	}


	/// Removes an item from the map.
	//
	TMap & remove(TKey const & key) {
		_raw_map_cast()->remove(key, key_hash(key));
		return *static_cast<TMap *>(this);
	}


	/// Removes all the items in the map.
	//
	TMap & remove_all() {
		_raw_map_cast()->remove_all();
		return *static_cast<TMap *>(this);
	}


protected:


	/// Constructor.
	//
	_map_impl(_raw_map_desc * prmd) {
		m_prmd = prmd;
		m_prmd->reset();
	}
	_map_impl(_raw_map_desc * prmd, map0 const & m) {
		UNUSED_ARG(prmd);
		m_prmd = NULL;
		assign(m);
	}
	_map_impl(_raw_map_desc * prmd, map0 && m) {
		UNUSED_ARG(prmd);
		m_prmd = NULL;
		assign(std::move(m));
	}
	template <size_t t_ceStatic2>
	_map_impl(_raw_map_desc * prmd, map<TKey, TVal, t_ceStatic2> && m) {
		UNUSED_ARG(prmd);
		m_prmd = NULL;
		assign(std::move(m));
	}


	/// Copies or moves the contents of the source to *this according to the source type:
	// - _raw_map_root const &: copy descriptor
	// - _raw_map_root &&: move descriptor or (move items + empty source map)
	//
	void assign(_raw_map_root const & rmrSrc) {
		_raw_map_cast()->assign(rmrSrc, false);
	}
	void assign(_raw_map_root && rmrSrc) {
		_raw_map_cast()->assign(rmrSrc, true);
	}


	/// Computes the hash value of a key.
	//
	size_t key_hash(TKey const & key) {
		size_t hash(std::hash<typename std::remove_cv<TKey>::type>()(key));
		return _raw_map_root::adjust_hash(hash);
	}


	/// Inserts a new key/value pair into the map. If the key already exist, the corresponding value
	// is replaced.
	//
	void set_item(TKey const & key, TVal const & val) {
		_raw_map_cast()->set_item(&key, key_hash(key), &val, false, false);
	}
	void set_item(TKey const & key, TVnc && val) {
		_raw_map_cast()->set_item(&key, key_hash(key), &val, false, true);
	}
	void set_item(TKnc && key, TVal const & val) {
		_raw_map_cast()->set_item(&key, key_hash(key), &val, true, false);
	}
	void set_item(TKnc && key, TVnc && val) {
		_raw_map_cast()->set_item(&key, key_hash(key), &val, true, true);
	}
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::map


namespace abc {

template <typename TKey, typename TVal>
class map<TKey, TVal, 0> :
	public _map_impl<TKey, TVal, 0>,
	protected _embedded_map_desc<TKey, TVal, _raw_map_desc::smc_ceMin> {

	typedef _map_impl<TKey, TVal, 0> map_impl;
	typedef _embedded_map_desc<TKey, TVal, _raw_map_desc::smc_ceMin> embedded_map_desc;

public:

	/// Constructor.
	//
	map() :
		// This might break in the future, because we’re passing a pointer to the _raw_map_desc base,
		// which at this point (before _map_impl::_map_impl()) has not been constructed yet.
		map_impl(get_embedded_desc()) {
	}
	map(map const & m) :
		map_impl(get_embedded_desc(), m) {
	}
	map(map && m) :
		map_impl(get_embedded_desc(), std::move(m)) {
	}
	template <size_t t_ceStatic>
	map(map<TKey, TVal, t_ceStatic> && m) :
		map_impl(get_embedded_desc(), std::move(m)) {
	}


	/// Destructor.
	//
	~map() {
	}


	/// Assignment operator.
	//
	map & operator=(map const & m) {
		return map_impl::operator=(m);
	}
	map & operator=(map && m) {
		return map_impl::operator=(std::move(m));
	}
	template <size_t t_ceStatic>
	map & operator=(map<TKey, TVal, t_ceStatic> && m) {
		return map_impl::operator=(std::move(m));
	}


protected:

	/// Initializes the embedded descriptor, and returns a pointer to it.
	//
	_raw_map_desc * get_embedded_desc() {
		return embedded_map_desc::init_and_get_desc();
	}
};

template <typename TKey, typename TVal, size_t t_ceStatic>
class map :
	public _map_impl<TKey, TVal, t_ceStatic>,
	protected _embedded_map_desc<
		TKey, TVal, (t_ceStatic > _raw_map_desc::smc_ceMin
			? t_ceStatic
			: size_t(_raw_map_desc::smc_ceMin))
	> {

	typedef map<TKey, TVal, 0> map0;
	typedef _map_impl<TKey, TVal, t_ceStatic> map_impl;
	typedef _embedded_map_desc<
		TKey, TVal, (t_ceStatic > _raw_map_desc::smc_ceMin
			? t_ceStatic
			: size_t(_raw_map_desc::smc_ceMin))
	> embedded_map_desc;

public:

	/// Constructor.
	//
	map() :
		map_impl(get_embedded_desc()) {
	}
	map(map const & m)  :
		map_impl(get_embedded_desc(), m) {
	}
	map(map && m) :
		map_impl(get_embedded_desc(), std::move(m)) {
	}
	map(map0 const & m) :
		map_impl(get_embedded_desc(), m) {
	}
	map(map0 && m) :
		map_impl(get_embedded_desc(), std::move(m)) {
	}
	template <size_t t_ceStatic2>
	map(map<TKey, TVal, t_ceStatic2> && m) :
		map_impl(get_embedded_desc(), std::move(m)) {
	}


	/// Destructor.
	//
	~map() {
	}


	/// Assignment operator.
	//
	map & operator=(map const & m) {
		return map_impl::operator=(m);
	}
	map & operator=(map && m) {
		return map_impl::operator=(std::move(m));
	}
	map & operator=(map0 const & m) {
		return map_impl::operator=(m);
	}
	map & operator=(map0 && m) {
		return map_impl::operator=(std::move(m));
	}
	template <size_t t_ceStatic2>
	map & operator=(map<TKey, TVal, t_ceStatic2> && m) {
		return map_impl::operator=(std::move(m));
	}


	/// Implicit cast as map<TKey, TVal, 0> reference. It only allows read-only access; any attempt
	// to cast a non-const reference will either result in a move to be implicitly performed
	// (r-value) or a compiler error (l-value).
	//
	operator map0 const &() const {
		return *static_cast<map0 const *>(static_cast<_raw_map_data const *>(this));
	}


protected:

	/// Initializes the embedded descriptor, and returns a pointer to it.
	//
	_raw_map_desc * get_embedded_desc() {
		return embedded_map_desc::init_and_get_desc();
	}
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_MAP_HXX

#if 0

/// Dumps the hash table using an external function.
//
void hashtable_statdump(struct hashtable const * pht, struct fwriter * pfw) {
	fwriter_printf(pfw,
		T("Hash table: seed %#08x, %d max, %d used, %d entries, load factor %d%%\n"),
		pht->hashSeed,
		pht->ceMax,
		pht->cUsedSlots,
		pht->cEntries,
		(pht->cEntries * 100 + (pht->ceMax >> 1)) / pht->ceMax
	);
	for (size_t i(0); i < pht->ceMax; ++i) {
		fwriter_printf(pfw, T("  Entry %d:\n"), i);
		for (struct hashentry * phe(pht->apheHeads[i]); phe; phe = phe->pheNext)
			fwriter_printf(pfw, T("    %#08x \"%s\" data: %p\n"), phe->hash, phe->aszKey, phe->pData);
	}
}

#endif

