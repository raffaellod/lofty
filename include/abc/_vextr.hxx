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

#ifndef ABC__VEXTR_HXX
#define ABC__VEXTR_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/numeric.hxx>
#include <abc/memory.hxx>
#include <abc/type_raw_cda.hxx>



/** DOC:4019 abc::*str_ and abc::*vector design

*str_ and *vector are implemented using the same base set of classes:

•	_raw_vextr_impl_base, core functionality for a vector of items: a little code and all member
	variables; this is then extended by three implementation classes:

	•	_raw_complex_vextr_impl, implementation of a vector of objects of non-trivial class: this is
		fully transactional and therefore exception-proof, but it's of course slower and uses more
		memory even during simpler operations;

	•	_raw_trivial_vextr_impl, implementation of a vector of plain values (instances of trivial
		class or native type): this is a near-optimal solution, still exception-proof but also taking
		advantage of the knowledge that no copy constructors need to be called; this class also
		supports the presence of a last element of value 0, opening up for the implementation of a
		string-like vector:

		•	_raw_str, implementation of a string: mostly based on _raw_trivial_vector_impl, it also
			provides means for clients of str_ to avoid having to be templates themselves, by giving
			access to type-deleted (void *) methods.

A vector/string using a static item array is nearly as fast as the C-style direct manipulation of an
array, only wasting a very small amount of space, and providing the ability to switch to a
dynamically-allocated item array on-the-fly in case the client needs to store in it more items than
are available.

Note: vextr is a silly portmanteau of vector and str(ing), because most of the above classes are
used by both.


Underlying data storage:

1.	istr() or dmstr()
	┌───┬───┬───────┐
	│ p │ 1 │ 0|f|f │
	└───┴───┴───────┘
	  │                 ┌────┐
	  ╰────────────────▶│ \0 │						Read-only memory (NUL)
		                 └────┘

2.	smstr<5>()
	┌───┬───┬───────╥───┬─────────┐
	│ p │ 1 │ 0|f|t ║ 4 │ - - - - │				Static (can be stack-allocated) fixed-size buffer
	└───┴───┴───────╨───┴─────────┘
	  │                 ┌────┐
	  └────────────────▶│ \0 │						Read-only memory (NUL)
		                 └────┘

3.	istr("abc"):
	┌───┬───┬───────┐
	│ p │ 4 │ 0|f|f │
	└───┴───┴───────┘
	  │                 ┌──────────┐
	  └────────────────▶│ a b c \0 │				Read-only memory
		                 └──────────┘

4.	dmstr("abc")
	┌───┬───┬───────┐
	│ p │ 4 │ 8|t|f │
	└───┴───┴───────┘
	  │                 ┌──────────────────┐
	  └────────────────▶│ a b c \0 - - - - │	Dynamically-allocated variable-size buffer
		                 └──────────────────┘

5.	smstr<3>("abc")
	┌───┬───┬───────╥───┬──────────┐
	│ p │ 4 │ 4|f|t ║ 4 │ a b c \0 │				Static (can be stack-allocated) fixed-size buffer
	└───┴───┴───────╨───┴──────────┘
	  │                 ▲
	  └─────────────────┘

6.	smstr<2>("abc"):
	┌───┬───┬───────╥───┬───────┐
	│ p │ 4 │ 8|t|t ║ 3 │ - - - │					Static (can be stack-allocated) fixed-size buffer
	└───┴───┴───────╨───┴───────┘
	  │                 ┌──────────────────┐
	  └────────────────▶│ a b c \0 - - - - │	Dynamically-allocated variable-size buffer
		                 └──────────────────┘


String types:

	istr (immutable string)
		Item array can be read-only (and shared) or dynamic.
	smstr (statically- or dynamically-allocated mutable string)
		Item array cannot be read-only nor shared, but it can be static or dynamic.
	dmstr (dynamically-allocated mutable string)
		Item array cannot be read-only, nor shared, nor static - always dynamic and writable.


Argument usage scenarios:

	istr           g_is;
	istr const     gc_is;
	dmstr          g_dms;
	dmstr const    gc_dms;
	smstr<n>       g_sms;
	smstr<n> const gc_sms;
	mstr           g_ms;
	mstr const     gc_ms;


•	No need to modify:

	void f1(istr const & isArg) {
		// N/A - const.
		isArg += "abc";

		// Share a read-only item array, or copy it: istr::operator=(istr const &)
		// Use assign_share_ro_or_copy().
		g_is = isArg;

		// TODO: validate these!
		// 1. Copy-construct: istr::istr(istr const &)
		//    Use assign_share_ro_or_copy(): will share a read-only item array, but will copy anything
		//    else. It's a copy - it's expected to have a separate life.
		// 2. Move-assign from the copy: istr::operator=(istr &&) (“nothrow”)
		//    Use assign_move().
		g_is = std::move(isArg);
		// 3. Destruct the now-empty copy: istr::~istr()

		// Copy the item array: mstr::operator=(istr const &)
		// Use assign_copy().
		g_ms = isArg;

		// TODO: validate these!
		// 1. Same as 1. above.
		// 2. Move-assign from the copy: dmstr::operator=(istr &&) (can throw)
		//    Use assign_move_dynamic_or_copy(): will move a dynamic item array, or will copy anything
		//    else (like assign_copy()).
		g_ms = std::move(isArg);
		// 3. Same as 3. above.

		// Copy the item array: dmstr::operator=(istr const &)
		// Use assign_copy().
		g_dms = isArg;

		// TODO: validate these!
		// 1. Same as 1. above.
		// 2. Move-assign from the copy: dmstr::operator=(istr &&) (can throw)
		//    Use assign_move_dynamic_or_copy(): will move a dynamic item array, or will copy anything
		//    else (like assign_copy()).
		g_dms = std::move(isArg);
		// 3. Same as 3. above.

		// Copy the item array: smstr<n>::operator=(istr const &)
		// Use assign_copy().
		g_sms = isArg;

		// TODO: validate these!
		// 1. Same as 1. above.
		// 2. Move-assign from the copy: smstr<n>::operator=(istr &&) (can throw)
		//    See considerations for 2. above.
		g_sms = std::move(isArg);
		// 3. Same as 3. above.
	}

	// 1. Construct a temporary object: istr::istr(char (& ach)[t_cch])
	f1("abc");
	// 2. Destruct the temporary object: istr::~istr()

	// Pass by const &.
	f1(g_is);
	f1(gc_is);

	// Invoke mstr::operator istr const &() const. Given that it's a REFERENCE, it's fine if the
	// source goes away and you get a crash: it's like freeing a pointer after passing it around.
	f1(g_ms);
	f1(gc_ms);

	// Invoke dmstr::operator istr const &() const. See considerations above.
	f1(g_dms);
	f1(gc_dms);

	// Invoke smstr<n>::operator istr const &() const. See considerations above.
	f1(g_sms);
	f1(gc_sms);


•	Writable dynamic string:

	void f2(dmstr * pdmsArg) {
		// Modify the buffer, maybe changing it for size reasons.
		*pdmsArg += "abc";

		// Copy the item array: istr::operator=(dmstr const &)
		// Use assign_copy(). Can never share, because dmstr never uses a read-only buffer.
		g_is = *pdmsArg;

		// Move the item array: istr::operator=(dmstr &&) (“nothrow”)
		// Use assign_move(). “nothrow” because dmstr cannot be a smstr<n> under covers.
		g_is = std::move(*pdmsArg);

		// Copy the item array: mstr::operator=(dmstr const &)
		// Use assign_copy().
		g_ms = *pdmsArg;

		// Move the item array: mstr::operator=(dmstr &&) (“nothrow”)
		// Use assign_move(). “nothrow” because dmstr cannot be a smstr<n> under covers.
		g_ms = std::move(*pdmsArg);

		// Copy the item array: dmstr::operator=(dmstr const &)
		// Use assign_copy().
		g_dms = *pdmsArg;

		// Move the item array: dmstr::operator=(dmstr &&) (“nothrow”)
		// Use assign_move(). “nothrow” because mdstr cannot be a smstr<n> under covers.
		g_dms = std::move(*pdmsArg);

		// Copy the item array: smstr<n>::operator=(dmstr const &)
		// Use assign_copy().
		g_sms = *pdmsArg;

		// Move the item array: smstr<n>::operator=(dmstr &&) (“nothrow”)
		// Use assign_move(). “nothrow” because dmstr cannot be a smstr<n> under covers.
		g_sms = std::move(*pdmsArg);
	}

	// N/A - no such conversion.
	f2("abc");
	f2(&g_is);
	f2(&gc_is);

	// N/A - no such conversion. This must be the case, otherwise the “nothrow” conditions described
	// above cannot be guaranteed.
	f2(&g_ms);
	f2(&gc_ms);

	// Pass by &.
	f2(&g_dms);

	// N/A - const.
	f2(&gc_dms);

	// N/A - no such conversion. This must be the case, otherwise the “nothrow” conditions described
	// above cannot be guaranteed.
	f2(&g_sms);
	f2(&gc_sms);


•	Writable (static or dynamic) string:

   void f3(mstr * pmsArg) {
      // Modify the buffer, maybe changing it for size reasons.
      *pmsArg += "abc";

      // Copy the item array: istr::operator=(mstr const &)
      // Use assign_copy(): can never share, because mstr never uses a read-only buffer.
      g_is = *pmsArg;

      // Move the item array: istr::operator=(mstr &&) (can throw)
      // Use assign_move_dynamic_or_copy(). can throw because mstr can be a smstr<n> under covers!
      g_is = std::move(*pmsArg);

      // Copy the item array: mstr::operator=(mstr const &)
      // Use assign_copy().
      g_ms = *pmsArg;

      // Move the item array: mstr::operator=(mstr &&) (can throw)
      // Use assign_move_dynamic_or_copy(). See considerations above.
      // WARNING - this class has a throwing move constructor/assignment operator!
      g_ms = std::move(*pmsArg);

      // Copy the item array: dmstr::operator=(mstr const &)
      // Use assign_copy().
      g_dms = *pmsArg;

      // Move the item array: dmstr::operator=(mstr &&) (“nothrow”)
      // Use assign_move(). Can throw because mstr can be a smstr<n> under covers!
      g_dms = std::move(*pmsArg);

      // Copy the item array: smstr<n>::operator=(mstr const &)
      // Use assign_copy().
      g_sms = *pmsArg;

      // Move the item array: smstr<n>::operator=(mstr &&) (can throw)
      // Use assign_move_dynamic_or_copy(): will move a dynamic item array, or will copy anything
      // else (like assign_copy()).
      g_sms = std::move(*pmsArg);
   }

   // N/A - no such conversion.
   f3("abc");
   f3(&g_is);
   f3(&gc_is);

   // Pass by &.
   f3(&g_ms);

   // N/A - const.
   f3(&gc_ms);

   // Down-cast to mstr &.
   f3(&g_dms);

   // N/A - const.
   f3(&gc_dms);

   // Down-cast to mstr &.
   f3(&g_sms);

   // N/A - const.
   f3(&gc_sms);


From the above, it emerges that:

•	mstr and smstr<n> cannot publicly derive from istr or dmstr, because that would enable automatic
	down-cast to i/dmstr &, which would then expose to the i/dmstr move constructor/assignment
	operator being invoked to move a static item array, which is wrong, or (if attempting to work
	around the move) would result in the static item array being copied, which would violate the
	“nothrow” requirement for the move constructor/assignment operator.

•	dmstr can publicly derive from mstr, with mstr being a base class for both dmstr and smstr<n>.

•	The only differences between istr and istr const & are:
	1.	istr const & can be using a static item array (because it can be a smstr<n>), while any other
		istr will always use a const/read-only item array or a dynamic one;
	2.	other string types can only be automatically converted to istr const &.

•	The difference between istr and mstr (and therefore dmstr/smstr<n>) is that the former can be
	constructed from a static string without copying it, but only offers read-only methods and
	operators; the latter offers the whole range of features one would expect, but will create a new
	item array upon construction or assignment (or use the embedded static one, in case of smstr<n>).

•	mstr cannot have a “nothrow” move constructor or assignment operator from itself, because the
	underlying objects might have static item arrays of different sizes. This isn't a huge deal-
	breaker because of the intended limited usage for mstr and smstr<n>.

The resulting class hierarchy is therefore:

	str_base (near-complete implementation of istr)
		istr
		mstr (near-complete implementation of dmstr/smstr<n>)
			dmstr
			smstr<n>

             ┌─────────────────────────────────────────────────────────┐
             │                     Functional need                     │
┌────────────┼──────────────┬─────────────────┬──────────┬─────────────┤
│            │ Local/member │ Method/function │ Writable │  Constant   │
│ Class      │ variable     │ argument        │          │ (read-only) │
├────────────┼──────────────┼─────────────────┼──────────┼─────────────┤
│ istr const │       x      │    x (const &)  │          │      x      │
│ mstr       │              │      x (*)      │     x    │             │
│ dmstr      │       x      │                 │     x    │             │
│ smstr      │       x      │                 │     x    │             │
└────────────┴──────────────┴─────────────────┴──────────┴─────────────┘
*/

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_vextr_packed_data

namespace abc {

// Note: getters and setters in this class don’t follow the regular naming convention used
// everywhere else, to underline the fact this is just a group of member variables rather than a
// regular class.
class _raw_vextr_packed_data {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	_raw_vextr_packed_data(size_t ciMax, bool bDynamic, bool bHasStatic) :
		m_iPackedData(
			ciMax | (bDynamic ? smc_bDynamicMask : 0) | (bHasStatic ? smc_bHasStaticMask : 0)
		) {
	}


	/** Assignment operator. Updates all components except bHasStatic.

	TODO: comment signature.
	*/
	_raw_vextr_packed_data & operator=(_raw_vextr_packed_data const & rvpd) {
		m_iPackedData = (rvpd.m_iPackedData & ~smc_bHasStaticMask)
						  | (m_iPackedData & smc_bHasStaticMask);
		return *this;
	}


	/** Assigns new values to all components except bHasStatic.

	TODO: comment signature.
	*/
	_raw_vextr_packed_data & set(size_t ciMax, bool bDynamic) {
		m_iPackedData = ciMax
						  | (bDynamic ? smc_bDynamicMask : 0)
						  | (m_iPackedData & smc_bHasStaticMask);
		return *this;
	}


	/** Returns/assigns ciMax.

	TODO: comment signature.
	*/
	size_t get_ciMax() const {
		return m_iPackedData & smc_ciMaxMask;
	}
	size_t set_ciMax(size_t ciMax) {
		m_iPackedData = (m_iPackedData & ~smc_ciMaxMask) | ciMax;
		return ciMax;
	}


	/** Returns true if the parent object’s m_p points to a dynamically-allocated item array.

	TODO: comment signature.
	*/
//	bool is_item_array_dynamic() const {
	bool get_bDynamic() const {
		return (m_iPackedData & smc_bDynamicMask) != 0;
	}


	/** Returns true if the parent object is followed by a static item array.

	TODO: comment signature.
	*/
//	bool has_static_item_array() const {
	bool get_bHasStatic() const {
		return (m_iPackedData & smc_bHasStaticMask) != 0;
	}


private:

	/** Bit-field composed by the following components:

	bool const bHasStatic
		true if the parent object is followed by a static item array.
	bool bDynamic
		true if the item array was dynamically allocated.
	size_t ciMax;
		Size of the item array.
	*/
	size_t m_iPackedData;

	/** Mask to access bHasStatic from m_iPackedData. */
	static size_t const smc_bHasStaticMask = 0x01;
	/** Mask to access bDynamic from m_iPackedData. */
	static size_t const smc_bDynamicMask = 0x02;


public:

	/** Mask to access ciMax from m_iPackedData. */
	static size_t const smc_ciMaxMask = ~(smc_bDynamicMask | smc_bHasStaticMask);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_vextr_impl_base


namespace abc {

/** Template-independent members of _raw_*_vextr_impl that are identical for trivial and non-trivial
types.
*/
class _raw_vextr_impl_base {

	ABC_CLASS_PREVENT_COPYING(_raw_vextr_impl_base)

protected:

	/** Allows to get a temporary item array from a pool of options, then work with it, and upon
	destruction it ensures that the array is either adopted by the associated _raw_vextr_impl_base,
	or properly discarded.

	A transaction will not take care of copying the item array, if switching to a different item
	array.

	For size increases, the reallocation (if any) is performed in the constructor; for decreases,
	it’s performed in commit().
	*/
	class transaction {

		ABC_CLASS_PREVENT_COPYING(transaction)

	public:

		/** Constructor.

		TODO: comment signature.
		*/
		transaction(
			size_t cbItem,
			_raw_vextr_impl_base * prvib, ptrdiff_t ciNew, ptrdiff_t ciDelta = 0, bool bNulT = false
		);


		/** Destructor.
		*/
		~transaction() {
			if (m_bFree) {
				memory::free(m_p);
			}
		}


		/** Commits the transaction; if the item array is to be replaced, the current one will be
		released if necessary; it’s up to the client to destruct any items in it. If this method is
		not called before the transaction is destructed, it’s up to the client to also ensure that any
		and all objects constructed in the work array have been properly destructed.

		TODO: comment signature.
		*/
		void commit(size_t cbItem = 0, bool bNulT = false);


		/** Returns the work item array.

		TODO: comment signature.
		*/
		template <typename T = void>
		T * get_work_array() const {
			return static_cast<T *>(m_p);
		}


		/** Returns true if the contents of the item array need to migrated due to the transaction
		switching item arrays. If the array was/will be only resized, the return value is false,
		because the reallocation did/will take care of moving the item array.

		TODO: comment signature.
		*/
		bool will_replace_item_array() const {
			return m_p != m_prvib->m_p;
		}


	private:

		/** Subject of the transaction. */
		_raw_vextr_impl_base * m_prvib;
		/** Pointer to the item array to which clients must write. This may or may not be the same as
		m_prvib->m_p, depending on whether we needed a new item array. This pointer will replace
		m_prvib->m_p upon commit(). */
		void * m_p;
		/** Number of currently used items in m_p. */
		size_t m_ci;
		/** See _raw_vextr_impl_base::m_rvpd. */
		_raw_vextr_packed_data m_rvpd;
		/** true if m_p has been dynamically allocated for the transaction and needs to be freed in
		the destructor, either because the transaction didn’t get committed, or because it did and the
		item array is now owned by m_prvib. */
		bool m_bFree;
	};

	// Allow transactions to access the protected members.
	friend class transaction;


public:

	/** Destructor.
	*/
	~_raw_vextr_impl_base() {
		if (m_rvpd.get_bDynamic())
			memory::free(m_p);
	}


	/** Returns a pointer to the item array.

	TODO: comment signature.
	*/
	template <typename T = void>
	T * get_data() {
		return static_cast<T *>(m_p);
	}
	template <typename T = void>
	T const * get_data() const {
		return static_cast<T const *>(m_p);
	}


	/** See buffered_vector::get_capacity() and _raw_str::get_capacity().

	TODO: comment signature.
	*/
	size_t get_capacity(bool bNulT = false) const {
		size_t ciMax(m_rvpd.get_ciMax());
		return ciMax - (ciMax > 0 && bNulT ? 1 /*NUL*/ : 0);
	}


	/** See buffered_vector::get_size() and _raw_str::get_size().

	TODO: comment signature.
	*/
	size_t get_size(bool bNulT = false) const {
		return m_ci - (bNulT ? 1 /*NUL*/ : 0);
	}


protected:

	/** Constructor. Constructs the object as empty, setting m_p to NULL or an empty string.

	TODO: comment signature.
	*/
	_raw_vextr_impl_base(size_t ciStaticMax, bool bNulT = false);
	// Constructs the object, assigning an item array.
	_raw_vextr_impl_base(void const * pConstSrc, size_t ciSrc) :
		m_p(const_cast<void *>(pConstSrc)),
		m_ci(ciSrc),
		// ciMax = 0 means that the item array is read-only.
		m_rvpd(0, false, false) {
		assert(pConstSrc);
	}


	/** Adjusts a 0-based index in the array. If negative, it’s interpreted as a 1-based index from
	the end.

	TODO: comment signature.
	*/
	size_t adjust_index(ptrdiff_t i, bool bNulT = false) const;


	/** Adjusts a 0-based index and count in the array. iFirst is treated like adjust_index() does;
	if the count of items is negative, it’s the count of elements to skip from the end of the item
	array.

	TODO: comment signature.
	*/
	void adjust_range(ptrdiff_t * piFirst, ptrdiff_t * pci, bool bNulT = false) const;


	/** Resets the contents of the object to an empty item array (a single NUL for string, no array
	at all for everything else).

	TODO: comment signature.
	*/
	void assign_empty(bool bNulT = false) {
		m_p = bNulT ? const_cast<char32_t *>(&smc_chNUL) : NULL;
		m_ci = bNulT ? 1 /*NUL*/ : 0;
		m_rvpd.set(0, false);
	}


	/** Returns a pointer to the static item array that follows this object, if present, or NULL
	otherwise.

	TODO: comment signature.
	*/
	template <typename T = void>
	T * get_static_array_ptr();


	/** Returns the size of the array returned by get_static_array_ptr(), or 0 if no such array is
	present.

	TODO: comment signature.
	*/
	size_t get_static_capacity() const;


	/** Returns true if m_p points to a read-only item array.

	TODO: comment signature.
	*/
	bool is_item_array_readonly() const {
		// No capacity means read-only item array.
		return m_rvpd.get_ciMax() == 0;
	}


	/** Puts a NUL terminator at the provided address.

	TODO: comment signature.
	*/
	static void terminate(size_t cbItem, void * p) {
		switch (cbItem) {
			case sizeof(int8_t):
				*static_cast<int8_t *>(p) = 0;
				break;
			case sizeof(int16_t):
				*static_cast<int16_t *>(p) = 0;
				break;
			case sizeof(int32_t):
				*static_cast<int32_t *>(p) = 0;
				break;
		}
	}


protected:

	/** Pointer to the item array. */
	void * m_p;
	/** Number of currently used items in m_p. */
	size_t m_ci;
	/** Size of the item array pointed to by m_p, and other bits. */
	_raw_vextr_packed_data m_rvpd;

	/** NUL terminator of the largest character type. */
	static char32_t const smc_chNUL;
	/** No less than this many items. Must be greater than, and not overlap any bits with,
	_raw_vextr_impl_base::smc_ciMaxMask. */
	static size_t const smc_cMinSlots = 8;
	/** Size multiplier. This should take into account that we want to reallocate as rarely as
	possible, so every time we do it it should be for a rather conspicuous growth. */
	static unsigned const smc_iGrowthRate = 2;
};


/** Used to find out what the offset are for an embedded static item array.
*/
class _raw_vextr_impl_base_with_static_item_array :
	public _raw_vextr_impl_base {
public:

	/** Static size. */
	size_t m_ciStaticMax;
	/** First item of the static array. This can’t be a T[], because we don’t want its items to be
	constructed/destructed automatically, and because this class doesn’t know its size. */
	std::max_align_t m_tFirst;
};


/** Rounds up an array size to avoid interfering with the bits outside of
_raw_vextr_packed_data::smc_ciMaxMask. Should be a constexpr function, but for now it’s just a
macro.

TODO: comment signature.
*/
#define _ABC__RAW_VEXTR_IMPL_BASE__ADJUST_ITEM_COUNT(ci) \
	((size_t(ci) + ~_raw_vextr_packed_data::smc_ciMaxMask) & _raw_vextr_packed_data::smc_ciMaxMask)


// Now these can be implemented.

template <typename T /*= void*/>
inline T * _raw_vextr_impl_base::get_static_array_ptr() {
	if (!m_rvpd.get_bHasStatic()) {
		return NULL;
	}
	_raw_vextr_impl_base_with_static_item_array * prvibwsia(
		static_cast<_raw_vextr_impl_base_with_static_item_array *>(this)
	);
	return reinterpret_cast<T *>(&prvibwsia->m_tFirst);
}


inline size_t _raw_vextr_impl_base::get_static_capacity() const {
	if (!m_rvpd.get_bHasStatic()) {
		return 0;
	}
	_raw_vextr_impl_base_with_static_item_array const * prvibwsia(
		static_cast<_raw_vextr_impl_base_with_static_item_array const *>(this)
	);
	return prvibwsia->m_ciStaticMax;
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_complex_vextr_impl


namespace abc {

/** Template-independent implementation of a vector for non-trivial contained types.
*/
class _raw_complex_vextr_impl :
	public _raw_vextr_impl_base {

	ABC_CLASS_PREVENT_COPYING(_raw_complex_vextr_impl)

public:

	/** See _raw_vector::append().

	TODO: comment signature.
	*/
	void append(void_cda const & type, void const * pAdd, size_t ciAdd, bool bMove) {
		if (ciAdd) {
			_insert(type, get_size(), pAdd, ciAdd, bMove);
		}
	}


	/** Copies or moves the contents of the source to *this, according to the source type. If bMove
	== true, the source items will be moved by having their const-ness cast away - be careful.

	TODO: comment signature.
	*/
	void assign_copy(void_cda const & type, void const * p, size_t ci, bool bMove);
	void assign_copy(
		void_cda const & type,
		void const * p1, size_t ci1, bool bMove1, void const * p2, size_t ci2, bool bMove2
	);


	/** Moves the contents of the source to *this, taking ownership of the whole item array (items
	are not moved nor copied).

	TODO: comment signature.
	*/
	void assign_move(void_cda const & type, _raw_complex_vextr_impl && rcvi);


	/** Destructs a range of items, or the whole item array. It does not deallocate the item array.

	TODO: comment signature.
	*/
	void destruct_items(void_cda const & type) {
		type.destruct(m_p, m_ci);
	}
	void destruct_items(void_cda const & type, size_t ci) {
		type.destruct(m_p, ci);
	}


	/** See _raw_vector::insert().

	TODO: comment signature.
	*/
	void insert(
		void_cda const & type, ptrdiff_t iOffset, void const * pAdd, size_t ciAdd, bool bMove
	) {
		if (ciAdd) {
			_insert(type, adjust_index(iOffset), pAdd, ciAdd, bMove);
		}
	}


	/** See _raw_vector::remove().

	TODO: comment signature.
	*/
	void remove(void_cda const & type, ptrdiff_t iOffset, ptrdiff_t ciRemove);


	/** See _raw_vector::set_capacity().

	TODO: comment signature.
	*/
	void set_capacity(void_cda const & type, size_t ciMin, bool bPreserve);


protected:

	/** Constructor.

	TODO: comment signature.
	*/
	_raw_complex_vextr_impl(size_t ciStaticMax, bool bNulT = false) :
		_raw_vextr_impl_base(ciStaticMax, bNulT) {
	}
	_raw_complex_vextr_impl(void const * pConstSrc, size_t ciSrc) :
		_raw_vextr_impl_base(pConstSrc, ciSrc) {
	}


private:

	/** Actual implementation of append() and insert(). Does not validate iOffset or ciAdd.

	TODO: comment signature.
	*/
	void _insert(void_cda const & type, size_t iOffset, void const * pAdd, size_t ciAdd, bool bMove);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_trivial_vextr_impl


namespace abc {

/** Template-independent implementation of a vector for trivial contained types. The entire class is
NUL-termination-aware; this is the most derived common base class of both vector and str_.
*/
class _raw_trivial_vextr_impl :
	public _raw_vextr_impl_base {

	ABC_CLASS_PREVENT_COPYING(_raw_trivial_vextr_impl)

public:

	/** See _raw_vector::append() and _raw_str::append().

	TODO: comment signature.
	*/
	void append(size_t cbItem, void const * pAdd, size_t ciAdd, bool bNulT = false) {
		if (ciAdd) {
			_insert_or_remove(cbItem, get_size(bNulT), pAdd, ciAdd, 0, bNulT);
		}
	}


	/** Copies the contents of the source array to *this.

	TODO: comment signature.
	*/
	void assign_copy(size_t cbItem, void const * p, size_t ci, bool bNulT = false) {
		if (p == m_p) {
			return;
		}
		// The two-source overload is fast enough. Pass it as the second source, because its code path
		// is faster.
		assign_copy(cbItem, NULL, 0, p, ci, bNulT);
	}
	// This overload must never be called with p1 or p2 == m_p.
	void assign_copy(
		size_t cbItem, void const * p1, size_t ci1, void const * p2, size_t ci2, bool bNulT = false
	);


	/** Moves the source’s item array to *this. This must be called with rtvi being in control of a
	read-only or dynamic item array; see [DOC:4019 abc::*str_ and abc::*vector design] to see how
	str_ and vector ensure this.

	TODO: comment signature.
	*/
	void assign_move(_raw_trivial_vextr_impl && rtvi, bool bNulT = false) {
		if (rtvi.m_p == m_p) {
			return;
		}
		// Share the item array.
		_assign_share(rtvi);
		// And now empty the source.
		rtvi.assign_empty(bNulT);
	}


	/** Moves the source’s item array if dynamically-allocated, else copies it to *this.

	TODO: comment signature.
	*/
	void assign_move_dynamic_or_copy(
		size_t cbItem, _raw_trivial_vextr_impl && rtvi, bool bNulT = false
	) {
		if (rtvi.m_p == m_p) {
			return;
		}
		if (rtvi.m_rvpd.get_bDynamic()) {
			assign_move(std::move(rtvi), bNulT);
		} else {
			// Can’t move, so copy instead.
			assign_copy(cbItem, rtvi.m_p, rtvi.get_size(bNulT), bNulT);
			// And now empty the source.
			rtvi.assign_empty(bNulT);
		}
	}


	/** Shares the source’s item array if read-only, else copies it to *this.

	TODO: comment signature.
	*/
	void assign_share_ro_or_copy(
		size_t cbItem, _raw_trivial_vextr_impl const & rtvi, bool bNulT = false
	) {
		if (rtvi.m_p == m_p) {
			return;
		}
		if (rtvi.is_item_array_readonly()) {
			_assign_share(rtvi);
		} else {
			// Non-read-only, cannot share.
			assign_copy(cbItem, rtvi.m_p, rtvi.get_size(bNulT), bNulT);
		}
	}


	/** See _raw_vector::insert().

	TODO: comment signature.
	*/
	void insert(
		size_t cbItem, ptrdiff_t iOffset, void const * pAdd, size_t ciAdd, bool bNulT = false
	) {
		if (ciAdd) {
			size_t iRealOffset(adjust_index(iOffset, bNulT));
			_insert_or_remove(cbItem, iRealOffset, pAdd, ciAdd, 0, bNulT);
		}
	}


	/** See _raw_vector::remove().

	TODO: comment signature.
	*/
	void remove(size_t cbItem, ptrdiff_t iOffset, ptrdiff_t ciRemove, bool bNulT = false) {
		adjust_range(&iOffset, &ciRemove, bNulT);
		if (ciRemove) {
			_insert_or_remove(cbItem, size_t(iOffset), NULL, 0, size_t(ciRemove), bNulT);
		}
	}


	/** See _raw_vector::set_capacity().

	TODO: comment signature.
	*/
	void set_capacity(size_t cbItem, size_t ciMin, bool bPreserve, bool bNulT = false);


protected:

	/** Constructor.

	TODO: comment signature.
	*/
	_raw_trivial_vextr_impl(size_t ciStaticMax, bool bNulT = false) :
		_raw_vextr_impl_base(ciStaticMax, bNulT) {
	}
	_raw_trivial_vextr_impl(void const * pConstSrc, size_t ciSrc) :
		_raw_vextr_impl_base(pConstSrc, ciSrc) {
	}


private:

	/** Shares the source’s item array. It only allows sharing read-only or dynamically-allocated
	item arrays (the latter only as part of moving them).

	TODO: comment signature.
	*/
	void _assign_share(_raw_trivial_vextr_impl const & rtvi);


	/** Actual implementation append(), insert() and remove(). It only validates pAdd.

	TODO: comment signature.
	*/
	void _insert_or_remove(
		size_t cbItem,
		size_t iOffset, void const * pAdd, size_t ciAdd, size_t ciRemove, bool bNulT = false
	);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_iterable_vector


namespace abc {

/** Provides standard methods to create iterators of type pointer_iterator from a
_raw_vextr_impl_base-derived class.
*/
template <class TCont, typename TVal>
struct _iterable_vector {
public:

	typedef TVal value_type;
	typedef TVal * pointer;
	typedef TVal const * const_pointer;
	typedef TVal & reference;
	typedef TVal const & const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef pointer_iterator<TCont, TVal> iterator;
	typedef pointer_iterator<TCont, TVal const> const_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;


public:

	/** Returns a forward iterator set to the first element.

	TODO: comment signature.
	*/
	iterator begin() {
		// const_cast is required because base_str_::get_data() returns const only.
		return iterator(const_cast<TVal *>(static_cast<TCont *>(this)->get_data()));
	}
	const_iterator begin() const {
		return cbegin();
	}


	/** Returns a const forward iterator set to the first element.

	TODO: comment signature.
	*/
	const_iterator cbegin() const {
		return const_iterator(static_cast<TCont const *>(this)->get_data());
	}


	/** Returns a const reverse iterator set to the first element.

	TODO: comment signature.
	*/
	const_reverse_iterator crbegin() const {
		return const_reverse_iterator(cbegin());
	}


	/** Returns a const forward iterator set beyond the last element.

	TODO: comment signature.
	*/
	const_iterator cend() const {
		return const_iterator(cbegin() + ptrdiff_t(static_cast<TCont const *>(this)->get_size()));
	}


	/** Returns a const reverse iterator set beyond the last element.

	TODO: comment signature.
	*/
	const_reverse_iterator crend() const {
		return const_reverse_iterator(cend());
	}


	/** Returns a forward iterator set beyond the last element.

	TODO: comment signature.
	*/
	iterator end() {
		return iterator(begin() + ptrdiff_t(static_cast<TCont *>(this)->get_size()));
	}
	const_iterator end() const {
		return cend();
	}


	/** Returns a reverse iterator set to the first element of the vector.

	TODO: comment signature.
	*/
	reverse_iterator rbegin() {
		return reverse_iterator(begin());
	}
	const_reverse_iterator rbegin() const {
		return crbegin();
	}


	/** Returns a reverse iterator set beyond the last element.

	TODO: comment signature.
	*/
	reverse_iterator rend() {
		return reverse_iterator(end());
	}
	const_reverse_iterator rend() const {
		return crend();
	}
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC__VEXTR_HXX

