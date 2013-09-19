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



////////////////////////////////////////////////////////////////////////////////////////////////////
// Declarations

namespace abc {

/// DESIGN_4019 abc::*string_ and abc::*vector design
//
// *string_ and *vector are implemented using the same base set of classes:
//
// •  _raw_vextr_impl_base, core functionality for a vector of items: a little code and all member
//    variables; this is then extended by three implementation classes:
//
//    •  _raw_complex_vextr_impl, implementation of a vector of objects of non-trivial class: this
//       is fully transactional and therefore exception-proof, but it's of course slower and uses
//       more memory even during simpler operations;
//
//    •  _raw_trivial_vextr_impl, implementation of a vector of plain values (instances of trivial
//       class or native type): this is a near-optimal solution, still exception-proof but also
//       taking advantage of the knowledge that no copy constructors need to be called; this class
//       also supports the presence of a last element of value 0, opening up for the implementation
//       of a string-like vector:
//
//       •  _raw_string, implementation of a string: mostly based on _raw_trivial_vector_impl, it
//          also provides means for clients of string_ to avoid having to be templates themselves,
//          by giving access to type-deleted (void *) methods.
//
// A vector/string using a static item array is nearly as fast as the C-style direct manipulation of
// an array, only wasting a very small amount of space, and providing the ability to switch to a
// dynamically-allocated item array on-the-fly in case the client needs to store in it more items
// than are available.
//
// Note: vextr is a silly portmanteau of vector and str(ing), because most of the above classes are
// used by both.
//
//
// Underlying data storage:
//
// 1. cstring() or wdstring()
//    ┌───┬───┬───────┐
//    │ p │ 1 │ 0|f|f │
//    └───┴───┴───────┘
//      │                 ┌────┐
//      ╰────────────────▶│ \0 │               Read-only memory (NUL)
//                        └────┘
//
// 2. wsstring<5>()
//    ┌───┬───┬───────╥───┬─────────┐
//    │ p │ 1 │ 0|f|t ║ 4 │ - - - - │          Static (can be stack-allocated) fixed-size buffer
//    └───┴───┴───────╨───┴─────────┘
//      │                 ┌────┐
//      └────────────────▶│ \0 │               Read-only memory (NUL)
//                        └────┘
//
// 3. cstring("abc"):
//    ┌───┬───┬───────┐
//    │ p │ 4 │ 0|f|f │
//    └───┴───┴───────┘
//      │                 ┌──────────┐
//      └────────────────▶│ a b c \0 │         Read-only memory
//                        └──────────┘
//
// 4. wdstring("abc")
//    ┌───┬───┬───────┐
//    │ p │ 4 │ 8|t|f │
//    └───┴───┴───────┘
//      │                 ┌──────────────────┐
//      └────────────────▶│ a b c \0 - - - - │ Dynamically-allocated variable-size buffer
//                        └──────────────────┘
//
// 5. wsstring<3>("abc")
//    ┌───┬───┬───────╥───┬──────────┐
//    │ p │ 4 │ 4|f|t ║ 4 │ a b c \0 │         Static (can be stack-allocated) fixed-size buffer
//    └───┴───┴───────╨───┴──────────┘
//      │                 ▲
//      └─────────────────┘
//
// 6. wsstring<2>("abc"):
//    ┌───┬───┬───────╥───┬───────┐
//    │ p │ 4 │ 8|t|t ║ 3 │ - - - │            Static (can be stack-allocated) fixed-size buffer
//    └───┴───┴───────╨───┴───────┘
//      │                 ┌──────────────────┐
//      └────────────────▶│ a b c \0 - - - - │ Dynamically-allocated variable-size buffer
//                        └──────────────────┘
//
//
// String types:
//
//    cstring (constant string)
//       Item array can be read-only (and shared) or dynamic.
//    wsstring (writable static string)
//       Item array cannot be read-only nor shared, but it can be static or dynamic.
//    wdstring (writable dynamic string)
//       Item array cannot be read-only, nor shared, nor static - always dynamic and writable.
//
//
// Argument usage scenarios:
//
//    cstring           g_cs;
//    cstring const     gc_cs;
//    wdstring          g_wds;
//    wdstring const    gc_wds;
//    wsstring<n>       g_wss;
//    wsstring<n> const gc_wss;
//    wstring           g_ws;
//    wstring const     gc_ws;
//
//
// •  No need to modify:
//
//    void f1(cstring const & csArg) {
//       // N/A - const.
//       csArg += "abc";
//
//       // Share a read-only item array, or copy it: cstring::operator=(cstring const &)
//       // Use assign_share_ro_or_copy().
//       g_cs = csArg;
//
//       // TODO: validate these!
//       // 1. Copy-construct: cstring::cstring(cstring const &)
//       //    Use assign_share_ro_or_copy(): will share a read-only item array, but will copy
//       //    anything else. It's a copy - it's expected to have a separate life.
//       // 2. Move-assign from the copy: cstring::operator=(cstring &&) (“nothrow”)
//       //    Use assign_move().
//       g_cs = std::move(csArg);
//       // 3. Destruct the now-empty copy: cstring::~cstring()
//
//       // Copy the item array: wstring::operator=(cstring const &)
//       // Use assign_copy().
//       g_ws = csArg;
//
//       // TODO: validate these!
//       // 1. Same as 1. above.
//       // 2. Move-assign from the copy: wdstring::operator=(cstring &&) (can throw)
//       //    Use assign_move_dynamic_or_copy(): will move a dynamic item array, or will copy
//       //    anything else (like assign_copy()).
//       g_ws = std::move(csArg);
//       // 3. Same as 3. above.
//
//       // Copy the item array: wdstring::operator=(cstring const &)
//       // Use assign_copy().
//       g_wds = csArg;
//
//       // TODO: validate these!
//       // 1. Same as 1. above.
//       // 2. Move-assign from the copy: wdstring::operator=(cstring &&) (can throw)
//       //    Use assign_move_dynamic_or_copy(): will move a dynamic item array, or will copy
//       //    anything else (like assign_copy()).
//       g_wds = std::move(csArg);
//       // 3. Same as 3. above.
//
//       // Copy the item array: wsstring<n>::operator=(cstring const &)
//       // Use assign_copy().
//       g_wss = csArg;
//
//       // TODO: validate these!
//       // 1. Same as 1. above.
//       // 2. Move-assign from the copy: wsstring<n>::operator=(cstring &&) (can throw)
//       //    See considerations for 2. above.
//       g_wss = std::move(csArg);
//       // 3. Same as 3. above.
//    }
//
//    // 1. Construct a temporary object: cstring::cstring(char (& ach)[t_cch])
//    f1("abc");
//    // 2. Destruct the temporary object: cstring::~cstring()
//
//    // Pass by const &.
//    f1(g_cs);
//    f1(gc_cs);
//
//    // Invoke wstring::operator cstring const &() const. Given that it's a REFERENCE, it's fine if
//    // the source goes away and you get a crash: it's like freeing a pointer after passing it
//    // around.
//    f1(g_ws);
//    f1(gc_ws);
//
//    // Invoke wdstring::operator cstring const &() const. See considerations above.
//    f1(g_wds);
//    f1(gc_wds);
//
//    // Invoke wsstring<n>::operator cstring const &() const. See considerations above.
//    f1(g_wss);
//    f1(gc_wss);
//
//
// •  Writable dynamic string:
//
//    void f2(wdstring * pwdsArg) {
//       // Modify the buffer, maybe changing it for size reasons.
//       *pwdsArg += "abc";
//
//       // Copy the item array: cstring::operator=(wdstring const &)
//       // Use assign_copy(). Can never share, because wdstring never uses a read-only buffer.
//       g_cs = *pwdsArg;
//
//       // Move the item array: cstring::operator=(wdstring &&) (“nothrow”)
//       // Use assign_move(). “nothrow” because wdstring cannot be a wsstring<n> under covers.
//       g_cs = std::move(*pwdsArg);
//
//       // Copy the item array: wstring::operator=(wdstring const &)
//       // Use assign_copy().
//       g_ws = *pwdsArg;
//
//       // Move the item array: wstring::operator=(wdstring &&) (“nothrow”)
//       // Use assign_move(). “nothrow” because wdstring cannot be a wsstring<n> under covers.
//       g_ws = std::move(*pwdsArg);
//
//       // Copy the item array: wdstring::operator=(wdstring const &)
//       // Use assign_copy().
//       g_wds = *pwdsArg;
//
//       // Move the item array: wdstring::operator=(wdstring &&) (“nothrow”)
//       // Use assign_move(). “nothrow” because wdstring cannot be a wsstring<n> under covers.
//       g_wds = std::move(*pwdsArg);
//
//       // Copy the item array: wsstring<n>::operator=(wdstring const &)
//       // Use assign_copy().
//       g_wss = *pwdsArg;
//
//       // Move the item array: wsstring<n>::operator=(wdstring &&) (“nothrow”)
//       // Use assign_move(). “nothrow” because wdstring cannot be a wsstring<n> under covers.
//       g_wss = std::move(*pwdsArg);
//    }
//
//    // N/A - no such conversion.
//    f2("abc");
//    f2(&g_cs);
//    f2(&gc_cs);
//
//    // N/A - no such conversion. This must be the case, otherwise the “nothrow” conditions
//    // described above cannot be guaranteed.
//    f2(&g_ws);
//    f2(&gc_ws);
//
//    // Pass by &.
//    f2(&g_wds);
//
//    // N/A - const.
//    f2(&gc_wds);
//
//    // N/A - no such conversion. This must be the case, otherwise the “nothrow” conditions
//    // described above cannot be guaranteed.
//    f2(&g_wss);
//    f2(&gc_wss);
//
//
// •  Writable (static or dynamic) string:
//
//    void f3(wstring * pwsArg) {
//       // Modify the buffer, maybe changing it for size reasons.
//       *pwsArg += "abc";
//
//       // Copy the item array: cstring::operator=(wstring const &)
//       // Use assign_copy(): can never share, because wstring never uses a read-only buffer.
//       g_cs = *pwsArg;
//
//       // Move the item array: cstring::operator=(wstring &&) (can throw)
//       // Use assign_move_dynamic_or_copy(). can throw because wstring can be a wsstring<n>
//       // under covers!
//       g_cs = std::move(*pwsArg);
//
//       // Copy the item array: wstring::operator=(wstring const &)
//       // Use assign_copy().
//       g_ws = *pwsArg;
//
//       // Move the item array: wstring::operator=(wstring &&) (can throw)
//       // Use assign_move_dynamic_or_copy(). See considerations above.
//       // WARNING - this class has a throwing move constructor/assignment operator!
//       g_ws = std::move(*pwsArg);
//
//       // Copy the item array: wdstring::operator=(wstring const &)
//       // Use assign_copy().
//       g_wds = *pwsArg;
//
//       // Move the item array: wdstring::operator=(wstring &&) (“nothrow”)
//       // Use assign_move(). Can throw because wstring can be a wsstring<n> under covers!
//       g_wds = std::move(*pwsArg);
//
//       // Copy the item array: wsstring<n>::operator=(wstring const &)
//       // Use assign_copy().
//       g_wss = *pwsArg;
//
//       // Move the item array: wsstring<n>::operator=(wstring &&) (can throw)
//       // Use assign_move_dynamic_or_copy(): will move a dynamic item array, or will copy anything
//       // else (like assign_copy()).
//       g_wss = std::move(*pwsArg);
//    }
//
//    // N/A - no such conversion.
//    f3("abc");
//    f3(&g_cs);
//    f3(&gc_cs);
//
//    // Pass by &.
//    f3(&g_ws);
//
//    // N/A - const.
//    f3(&gc_ws);
//
//    // Down-cast to wstring &.
//    f3(&g_wds);
//
//    // N/A - const.
//    f3(&gc_wds);
//
//    // Down-cast to wstring &.
//    f3(&g_wss);
//
//    // N/A - const.
//    f3(&gc_wss);
//
//
// From the above, it emerges that:
//
// •  wstring and wsstring<n> cannot publicly derive from cstring or wdstring, because that would
//    enable automatic down-cast to c/wdstring &, which would then expose to the c/wdstring move
//    constructor/assignment operator being invoked to move a static item array, which is wrong, or
//    (if attempting to work around the move) would result in the static item array being copied,
//    which would violate the “nothrow” requirement for the move constructor/assignment operator.
//
// •  wdstring can publicly derive from wstring, with wstring being a base class for both wdstring
//    and wsstring<n>.
//
// •  The only differences between cstring and cstring const & are:
//    1. cstring const & can be using a static item array (because it can be a wsstring<n>), while
//       any other cstring will always use a const/read-only item array or a dynamic one;
//    2. other string types can only be automatically converted to cstring const &.
//
// •  The difference between cstring and wstring (and therefore wdstring/wsstring<n>) is that the
//    former can be constructed from a static string without copying it, but only offers read-only
//    methods and operators; the latter offers the whole range of features one would expect, but
//    will create a new item array upon construction or assignment (or use the embedded static one,
//    in case of wsstring<n>).
//
// •  wstring cannot have a “nothrow” move constructor or assignment operator from itself, because
//    the underlying objects might have static item arrays of different sizes. This isn't a huge
//    deal-breaker because of the intended limited usage for wstring and wsstring<n>.
//
// The resulting class hierarchy is therefore:
//
//   string_base (pretty much the whole cstring)
//      cstring
//      wstring (pretty much the whole wdstring/wsstring<n>)
//         wdstring
//         wsstring<n>
//
//                  │                     Functional need                     │
//    ──────────────┼──────────────┬─────────────────┬──────────┬─────────────┤
//                  │ Local/member │ Method/function │ Writable │  Constant   │
//    Class         │ variable     │ argument        │          │ (read-only) │
//    ──────────────┼──────────────┼─────────────────┼──────────┼─────────────┤
//    cstring const │       x      │    x (const &)  │          │      x      │
//    wstring       │              │      x (*)      │     x    │             │
//    wdstring      │       x      │                 │     x    │             │
//    wsstring      │       x      │                 │     x    │             │
//    ──────────────┴──────────────┴─────────────────┴──────────┴─────────────┘

/// Template-independent members of _raw_*_vextr_impl that are identical for trivial and non-trivial
// types.
class _raw_vextr_impl_base;

/// Template-independent implementation of a vector for non-trivial contained types.
class _raw_complex_vextr_impl;

/// Template-independent implementation of a vector for trivial contained types. The entire class is
// NUL-termination-aware; this is the most derived common base class of both vector and string_.
class _raw_trivial_vextr_impl;

/// Provides standard methods to create iterators of type pointer_iterator from a
// _raw_vextr_impl_base-derived class.
template <class TCont, typename TVal>
struct _iterable_vector;

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_vextr_packed_data


namespace abc {

// Note: getters and setters in this class don’t follow the regular naming convention used
// everywhere else, to underline the fact this is just a group of member variables rather than a
// regular class.
class _raw_vextr_packed_data {
public:

	/// Constructor.
	//
	_raw_vextr_packed_data(size_t ciMax, bool bDynamic, bool bHasStatic) :
		m_iPackedData(
			ciMax | (bDynamic ? smc_bDynamicMask : 0) | (bHasStatic ? smc_bHasStaticMask : 0)
		) {
	}


	/// Assignment operator. Updates all components except bHasStatic.
	//
	_raw_vextr_packed_data & operator=(_raw_vextr_packed_data const & rvpd) {
		m_iPackedData = (rvpd.m_iPackedData & ~smc_bHasStaticMask)
						  | (m_iPackedData & smc_bHasStaticMask);
		return *this;
	}


	/// Assigns new values to all components except bHasStatic.
	//
	_raw_vextr_packed_data & set(size_t ciMax, bool bDynamic) {
		m_iPackedData = ciMax
						  | (bDynamic ? smc_bDynamicMask : 0)
						  | (m_iPackedData & smc_bHasStaticMask);
		return *this;
	}


	/// Returns/assigns ciMax.
	//
	size_t get_ciMax() const {
		return m_iPackedData & smc_ciMaxMask;
	}
	size_t set_ciMax(size_t ciMax) {
		m_iPackedData = (m_iPackedData & ~smc_ciMaxMask) | ciMax;
		return ciMax;
	}


	/// Returns true if the parent object’s m_p points to a dynamically-allocated item array.
	//
//	bool is_item_array_dynamic() const {
	bool get_bDynamic() const {
		return (m_iPackedData & smc_bDynamicMask) != 0;
	}


	/// Returns true if the parent object is followed by a static item array.
	//
//	bool has_static_item_array() const {
	bool get_bHasStatic() const {
		return (m_iPackedData & smc_bHasStaticMask) != 0;
	}


private:

	/// Bit-field composed by the following components:
	// •  true if the parent object is followed by a static item array.
	//    bool const bHasStatic;
	// •  true if the item array was dynamically allocated.
	//    bool bDynamic;
	// •  Size of the item array.
	//    size_t ciMax;
	size_t m_iPackedData;

	/// Mask to access bHasStatic from m_iPackedData.
	static size_t const smc_bHasStaticMask = 0x01;
	/// Mask to access bDynamic from m_iPackedData.
	static size_t const smc_bDynamicMask = 0x02;


public:

	/// Mask to access ciMax from m_iPackedData.
	static size_t const smc_ciMaxMask = ~(smc_bDynamicMask | smc_bHasStaticMask);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_vextr_impl_base


namespace abc {

class _raw_vextr_impl_base {

	ABC_CLASS_PREVENT_COPYING(_raw_vextr_impl_base)

protected:

	/// Allows to get a temporary item array from a pool of options, then work with it, and upon
	// destruction it ensures that the array is either adopted by the associated
	// _raw_vextr_impl_base, or properly discarded.
	//
	// A transaction will not take care of copying the item array, if switching to a different item
	// array.
	//
	// For size increases, the reallocation (if any) is performed in the constructor; for decreases,
	// it’s performed in commit().
	//
	class transaction {

		ABC_CLASS_PREVENT_COPYING(transaction)

	public:

		/// Constructor.
		transaction(
			size_t cbItem,
			_raw_vextr_impl_base * prvib, ptrdiff_t ciNew, ptrdiff_t ciDelta = 0, bool bNulT = false
		);


		/// Destructor.
		//
		~transaction() {
			if (m_bFree) {
				memory::free(m_p);
			}
		}


		/// Commits the transaction; if the item array is to be replaced, the current one will be
		// released if necessary; it’s up to the client to destruct any items in it. If this method is
		// not called before the transaction is destructed, it’s up to the client to also ensure that
		// any and all objects constructed in the work array have been properly destructed.
		void commit(size_t cbItem = 0, bool bNulT = false);


		/// Returns the work item array.
		//
		template <typename T = void>
		T * get_work_array() const {
			return static_cast<T *>(m_p);
		}


		/// Returns true if the contents of the item array need to migrated due to the transaction
		// switching item arrays. If the array was/will be only resized, the return value is false,
		// because the reallocation did/will take care of moving the item array.
		//
		bool will_replace_item_array() const {
			return m_p != m_prvib->m_p;
		}


	private:

		/// Subject of the transaction.
		_raw_vextr_impl_base * m_prvib;
		/// Pointer to the item array to which clients must write. This may or may not be the same as
		// m_prvib->m_p, depending on whether we needed a new item array. This pointer will replace
		// m_prvib->m_p upon commit().
		void * m_p;
		/// Number of currently used items in m_p.
		size_t m_ci;
		/// See _raw_vextr_impl_base::m_rvpd.
		_raw_vextr_packed_data m_rvpd;
		/// true if m_p has been dynamically allocated for the transaction and needs to be freed in
		// the destructor, either because the transaction didn’t get committed, or because it did and
		// the item array is now owned by m_prvib.
		bool m_bFree;
	};

	// Allow transactions to access the protected members.
	friend class transaction;


public:

	/// Destructor.
	//
	~_raw_vextr_impl_base() {
		if (m_rvpd.get_bDynamic())
			memory::free(m_p);
	}


	/// Returns a pointer to the item array.
	//
	template <typename T = void>
	T * get_data() {
		return static_cast<T *>(m_p);
	}
	template <typename T = void>
	T const * get_data() const {
		return static_cast<T const *>(m_p);
	}


	/// See buffered_vector::get_capacity() and _raw_string::get_capacity().
	//
	size_t get_capacity(bool bNulT = false) const {
		size_t ciMax(m_rvpd.get_ciMax());
		return ciMax - (ciMax > 0 && bNulT ? 1 /*NUL*/ : 0);
	}


	/// See buffered_vector::get_size() and _raw_string::get_size().
	//
	size_t get_size(bool bNulT = false) const {
		return m_ci - (bNulT ? 1 /*NUL*/ : 0);
	}


protected:

	/// Constructor.
	//
	// Constructs the object as empty, setting m_p to NULL or an empty string.
	_raw_vextr_impl_base(size_t ciStaticMax, bool bNulT = false);
	// Constructs the object, assigning an item array.
	_raw_vextr_impl_base(void const * pConstSrc, size_t ciSrc) :
		m_p(const_cast<void *>(pConstSrc)),
		m_ci(ciSrc),
		// ciMax = 0 means that the item array is read-only.
		m_rvpd(0, false, false) {
		assert(pConstSrc);
	}


	/// Adjusts a 0-based index in the array. If negative, it’s interpreted as a 1-based index from
	// the end.
	size_t adjust_index(ptrdiff_t i, bool bNulT = false) const;

	/// Adjusts a 0-based index and count in the array. iFirst is treated like adjust_index() does;
	// if the count of items is negative, it’s the count of elements to skip from the end of the item
	// array.
	void adjust_range(ptrdiff_t * piFirst, ptrdiff_t * pci, bool bNulT = false) const;


	/// Resets the contents of the object to an empty item array (a single NUL for string, no array
	// at all for everything else).
	//
	void assign_empty(bool bNulT = false) {
		m_p = bNulT ? const_cast<char32_t *>(&smc_chNUL) : NULL;
		m_ci = bNulT ? 1 /*NUL*/ : 0;
		m_rvpd.set(0, false);
	}


	/// Returns a pointer to the static item array that follows this object, if present, or NULL
	// otherwise.
	template <typename T = void>
	T * get_static_array_ptr();

	/// Returns the size of the array returned by get_static_array_ptr(), or 0 if no such array is
	// present.
	size_t get_static_capacity() const;


	/// Returns true if m_p points to a read-only item array.
	//
	bool is_item_array_readonly() const {
		// No capacity means read-only item array.
		return m_rvpd.get_ciMax() == 0;
	}


	/// Puts a NUL terminator at the provided address.
	//
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

	/// Pointer to the item array.
	void * m_p;
	/// Number of currently used items in m_p.
	size_t m_ci;
	/// Size of the item array pointed to by m_p, and other bits.
	_raw_vextr_packed_data m_rvpd;

	/// NUL terminator of the largest character type.
	static char32_t const smc_chNUL;
	/// No less than this many items. Must be greater than, and not overlap any bits with,
	// _raw_vextr_impl_base::smc_ciMaxMask.
	static size_t const smc_cMinSlots = 8;
	/// Size multiplier. This should take into account that we want to reallocate as rarely as
	// possible, so every time we do it it should be for a rather conspicuous growth.
	static unsigned const smc_iGrowthRate = 2;
};


/// Used to find out what the offset are for an embedded static item array.
//
class _raw_vextr_impl_base_with_static_item_array :
	public _raw_vextr_impl_base {
public:

	/// Static size.
	size_t m_ciStaticMax;
	/// First item of the static array. This can’t be a T[], because we don’t want its items to be
	// constructed/destructed automatically, and because this class doesn’t know its size.
	std::max_align_t m_tFirst;
};


/// Rounds up an array size to avoid interfering with the bits outside of
// _raw_vextr_packed_data::smc_ciMaxMask. Should be a constexpr function, but for now it’s just a
// macro.
//
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

class _raw_complex_vextr_impl :
	public _raw_vextr_impl_base {

	ABC_CLASS_PREVENT_COPYING(_raw_complex_vextr_impl)

public:

	/// See _raw_vector::append().
	//
	void append(void_cda const & type, void const * pAdd, size_t ciAdd, bool bMove) {
		if (ciAdd) {
			_insert(type, get_size(), pAdd, ciAdd, bMove);
		}
	}


	/// Copies or moves the contents of the source to *this, according to the source type. If bMove
	// == true, the source items will be moved by having their const-ness cast away - be careful.
	void assign_copy(void_cda const & type, void const * p, size_t ci, bool bMove);
	void assign_copy(
		void_cda const & type,
		void const * p1, size_t ci1, bool bMove1, void const * p2, size_t ci2, bool bMove2
	);


	/// Moves the contents of the source to *this, taking ownership of the whole item array (items
	// are not moved nor copied).
	void assign_move(void_cda const & type, _raw_complex_vextr_impl && rcvi);


	/// Destructs a range of items, or the whole item array. It does not deallocate the item array.
	//
	void destruct_items(void_cda const & type) {
		type.destruct(m_p, m_ci);
	}
	void destruct_items(void_cda const & type, size_t ci) {
		type.destruct(m_p, ci);
	}


	/// See _raw_vector::insert().
	//
	void insert(
		void_cda const & type, ptrdiff_t iOffset, void const * pAdd, size_t ciAdd, bool bMove
	) {
		if (ciAdd) {
			_insert(type, adjust_index(iOffset), pAdd, ciAdd, bMove);
		}
	}


	/// See _raw_vector::remove().
	void remove(void_cda const & type, ptrdiff_t iOffset, ptrdiff_t ciRemove);

	/// See _raw_vector::set_capacity().
	void set_capacity(void_cda const & type, size_t ciMin, bool bPreserve);


protected:

	/// Constructor.
	//
	_raw_complex_vextr_impl(size_t ciStaticMax, bool bNulT = false) :
		_raw_vextr_impl_base(ciStaticMax, bNulT) {
	}
	_raw_complex_vextr_impl(void const * pConstSrc, size_t ciSrc) :
		_raw_vextr_impl_base(pConstSrc, ciSrc) {
	}


private:

	/// Actual implementation of append() and insert(). Does not validate iOffset or ciAdd.
	void _insert(void_cda const & type, size_t iOffset, void const * pAdd, size_t ciAdd, bool bMove);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_trivial_vextr_impl


namespace abc {

class _raw_trivial_vextr_impl :
	public _raw_vextr_impl_base {

	ABC_CLASS_PREVENT_COPYING(_raw_trivial_vextr_impl)

public:

	/// See _raw_vector::append() and _raw_string::append().
	//
	void append(size_t cbItem, void const * pAdd, size_t ciAdd, bool bNulT = false) {
		if (ciAdd) {
			_insert_or_remove(cbItem, get_size(bNulT), pAdd, ciAdd, 0, bNulT);
		}
	}


	/// Copies the contents of the source array to *this.
	//
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


	/// Moves the source’s item array to *this. This must be called with rtvi being in control of a
	// read-only or dynamic item array; see [DESIGN_4019 abc::*string_ and abc::*vector design] to
	// see how string and vector ensure this.
	//
	void assign_move(_raw_trivial_vextr_impl && rtvi, bool bNulT = false) {
		if (rtvi.m_p == m_p) {
			return;
		}
		// Share the item array.
		_assign_share(rtvi);
		// And now empty the source.
		rtvi.assign_empty(bNulT);
	}


	/// Moves the source’s item array if dynamically-allocated, else copies it to *this.
	//
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


	/// Shares the source’s item array if read-only, else copies it to *this.
	//
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


	/// See _raw_vector::insert().
	//
	void insert(
		size_t cbItem, ptrdiff_t iOffset, void const * pAdd, size_t ciAdd, bool bNulT = false
	) {
		if (ciAdd) {
			size_t iRealOffset(adjust_index(iOffset, bNulT));
			_insert_or_remove(cbItem, iRealOffset, pAdd, ciAdd, 0, bNulT);
		}
	}


	/// See _raw_vector::remove().
	//
	void remove(size_t cbItem, ptrdiff_t iOffset, ptrdiff_t ciRemove, bool bNulT = false) {
		adjust_range(&iOffset, &ciRemove, bNulT);
		if (ciRemove) {
			_insert_or_remove(cbItem, size_t(iOffset), NULL, 0, size_t(ciRemove), bNulT);
		}
	}


	/// See _raw_vector::set_capacity().
	void set_capacity(size_t cbItem, size_t ciMin, bool bPreserve, bool bNulT = false);


protected:

	/// Constructor.
	//
	_raw_trivial_vextr_impl(size_t ciStaticMax, bool bNulT = false) :
		_raw_vextr_impl_base(ciStaticMax, bNulT) {
	}
	_raw_trivial_vextr_impl(void const * pConstSrc, size_t ciSrc) :
		_raw_vextr_impl_base(pConstSrc, ciSrc) {
	}


private:

	/// Shares the source’s item array. It only allows sharing read-only or dynamically-allocated
	// item arrays (the latter only as part of moving them).
	void _assign_share(_raw_trivial_vextr_impl const & rtvi);

	/// Actual implementation append(), insert() and remove(). It only validates pAdd.
	void _insert_or_remove(
		size_t cbItem,
		size_t iOffset, void const * pAdd, size_t ciAdd, size_t ciRemove, bool bNulT = false
	);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_iterable_vector


namespace abc {

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

	/// Returns a forward iterator set to the first element.
	//
	iterator begin() {
		// const_cast is required because base_string_::get_data() returns const only.
		return iterator(const_cast<TVal *>(static_cast<TCont *>(this)->get_data()));
	}
	const_iterator begin() const {
		return cbegin();
	}


	/// Returns a const forward iterator set to the first element.
	//
	const_iterator cbegin() const {
		return const_iterator(static_cast<TCont const *>(this)->get_data());
	}


	/// Returns a const reverse iterator set to the first element.
	//
	const_reverse_iterator crbegin() const {
		return const_reverse_iterator(cbegin());
	}


	/// Returns a const forward iterator set beyond the last element.
	//
	const_iterator cend() const {
		return const_iterator(cbegin() + ptrdiff_t(static_cast<TCont const *>(this)->get_size()));
	}


	/// Returns a const reverse iterator set beyond the last element.
	//
	const_reverse_iterator crend() const {
		return const_reverse_iterator(cend());
	}


	/// Returns a forward iterator set beyond the last element.
	//
	iterator end() {
		return iterator(begin() + ptrdiff_t(static_cast<TCont *>(this)->get_size()));
	}
	const_iterator end() const {
		return cend();
	}


	/// Returns a reverse iterator set to the first element of the vector.
	//
	reverse_iterator rbegin() {
		return reverse_iterator(begin());
	}
	const_reverse_iterator rbegin() const {
		return crbegin();
	}


	/// Returns a reverse iterator set beyond the last element.
	//
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

