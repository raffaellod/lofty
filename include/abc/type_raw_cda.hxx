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

#ifndef ABC_TYPE_RAW_CDA_HXX
#define ABC_TYPE_RAW_CDA_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/core.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::type_raw_cda_base


namespace abc {

/** Encapsulates raw constructors, destructors and assignment operators for a type. To be
instantiated via type_raw_cda.
*/
struct void_cda {

	/** Prototype of a function that copies items from one array to another.

	TODO: comment signature.
	*/
	typedef void (* copy_fn)(void * pDst, void const * pSrc, size_t ci);


	/** Prototype of a function that compares two values for equality.

	TODO: comment signature.
	*/
	typedef bool (* equal_fn)(void const * p1, void const * p2);


	/** Prototype of a function that moves items from one array to another.

	TODO: comment signature.
	*/
	typedef void (* move_fn)(void * pDst, void * pSrc, size_t ci);


	/** Prototype of a function that destructs items in an array.

	TODO: comment signature.
	*/
	typedef void (* destr_fn)(void * p, size_t ci);


	/** Size of a variable of this type, in bytes. */
	size_t cb;
	/** Alignment of a variable of this type, in bytes. */
	size_t cbAlign;
	/** Function to copy items from one array to another. */
	copy_fn copy_constr;
	/** Function to move items from one array to another. */
	move_fn move_constr;
	/** Function to move items within an array. */
	move_fn overlapping_move_constr;
	/** Function to destruct items in an array. */
	destr_fn destruct;
	/** Function to compare two items for equality. */
	equal_fn equal;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::typed_raw_cda


namespace abc {

/** DOC:3395 Move constructors and exceptions

In this section, “move constructor” will strictly refer to class::class(class &&).

All classes must provide move constructors and assignment operators if the copy constructor would
result in execution of exception-prone code (e.g. resource allocation).

Because move constructors are employed widely in container classes that need to provide strong
exception guarantee (fully transacted operation) even in case of moves, move constructors must not
throw exceptions. This requirement is relaxed for moves that involve two different classes, since
these will not be used by container classes.
*/

/** Defines a generic data type.
*/
template <typename T>
struct typed_raw_cda {

	/** Copies a range of items from one array to another, overwriting any existing contents in the
	destination.

	TODO: comment signature.
	*/
	static void copy_constr(T * ptDst, T const * ptSrc, size_t ci) {
		if (std::has_trivial_copy_constructor<T>::value) {
			// No constructor, fastest copy possible.
			memory::copy(ptDst, ptSrc, ci);
		} else if (std::has_nothrow_copy_constructor<T>::value) {
			// Not trivial, but it won’t throw either.
			for (T const * ptSrcEnd(ptSrc + ci); ptSrc < ptSrcEnd; ++ptSrc, ++ptDst) {
				::new(ptDst) T(*ptSrc);
			}
		} else {
			// Exceptions can occur, so implement an all-or-nothing copy.
			T const * ptDstBegin(ptDst);
			try {
				for (T const * ptSrcEnd(ptSrc + ci); ptSrc < ptSrcEnd; ++ptSrc, ++ptDst) {
					::new(ptDst) T(*ptSrc);
				}
			} catch (...) {
				// Undo (destruct) all the copies instantiated.
				while (--ptSrc >= ptDstBegin) {
					ptDst->~T();
				}
				throw;
			}
		}
	}


	/** Destroys a range of items in an array.

	TODO: comment signature.
	*/
	static void destruct(T * pt, size_t ci) {
		if (!std::has_trivial_destructor<T>::value) {
			// The destructor is not a no-op.
			for (T * ptEnd(pt + ci); pt < ptEnd; ++pt) {
				pt->~T();
			}
		}
	}


	/** Compares two values for equality.

	TODO: comment signature.
	*/
	static bool equal(void const * pt1, void const * pt2) {
		return *static_cast<T const *>(pt1) == *static_cast<T const *>(pt2);
	}


	/** Moves a range of items from one array to another, overwriting any existing contents in the
	destination.

	TODO: comment signature.
	*/
	static void move_constr(T * ptDst, T * ptSrc, size_t ci) {
		for (T * ptSrcEnd(ptSrc + ci); ptSrc < ptSrcEnd; ++ptSrc, ++ptDst) {
			::new(ptDst) T(std::move(*ptSrc));
		}
	}


	/** Safely moves a range of items to another position in the same array, carefully moving items
	in case the source and the destination ranges overlap. Note that this will also destruct the
	source items.

	TODO: comment signature.
	*/
	static void overlapping_move_constr(T * ptDst, T * ptSrc, size_t ci) {
		if (ptDst == ptSrc) {
			return;
		}
		T const * ptSrcEnd(ptSrc + ci);
		if (ptDst < ptSrc && ptSrc < ptDst + ci) {
			// ┌─────────────────┐ 
			// │ a - - B C D e f │
			// ├─────────────────┤
			// │ a B C D - - e f │
			// └─────────────────┘
			//
			// Move the items from left to right (the block moves from right to left).

			T const * ptSrcBegin(ptSrc), * ptDstEnd(ptDst + ci);
			// First, move-construct the items that don’t overlap.
			for (; ptDst < ptSrcBegin; ++ptSrc, ++ptDst) {
				::new(ptDst) T(std::move(*ptSrc));
			}
			// ┌─────────────────┐ 
			// │ a B C b c D e f │ (lowercase b and c indicate the moved-out items)
			// └─────────────────┘
			// Second, move-assign all the items in the overlapping area to shift them.
			for (; ptDst < ptDstEnd; ++ptSrc, ++ptDst) {
				*ptDst = std::move(*ptSrc);
			}
			// ┌─────────────────┐ 
			// │ a B C D c d e f │
			// └─────────────────┘
			// Third, destruct the items that have no replacement and have just been moved out.
			for (ptSrc = ptDst; ptSrc < ptSrcEnd; ++ptSrc) {
				ptSrc->~T();
			}
		} else if (ptSrc < ptDst && ptDst < ptSrc + ci) {
			// ┌─────────────────┐
			// │ a B C D - - e f │
			// ├─────────────────┤
			// │ a - - B C D e f │
			// └─────────────────┘
			//
			// This situation is the mirror of the above, so the move must be done using decrementing
			// pointers, sweeping right to left (the block moves from left to right).

			T const * ptSrcBegin(ptSrc), * ptDstBegin(ptDst), * ptDstEnd(ptDst + ci);
			// First, move-construct the items that don’t overlap.
			for (
				ptSrc = const_cast<T *>(ptSrcEnd),
				ptDst = const_cast<T *>(ptDstEnd);
				--ptDst >= --ptSrcEnd;
			) {
				::new(ptDst) T(std::move(*ptSrc));
			}
			// ┌─────────────────┐
			// │ a B c d C D e f │ (lowercase c and d indicate the moved-out items)
			// └─────────────────┘
			// Second, move-assign all the items in the overlapping area to shift them.
			for (; ptSrc >= ptSrcBegin; --ptSrc, --ptDst) {
				*ptDst = std::move(*ptSrc);
			}
			// ┌─────────────────┐
			// │ a b c B C D e f │
			// └─────────────────┘
			// Third, destruct the items that have no replacement and have just been moved out.
			for (ptSrc = const_cast<T *>(ptDstBegin); ptSrc >= ptSrcBegin; --ptSrc) {
				ptSrc->~T();
			}
		} else {
			for (; ptSrc < ptSrcEnd; ++ptSrc, ++ptDst) {
				::new(ptDst) T(std::move(*ptSrc));
			}
		}
	}
};

// Remove const, but not volatile. Or maybe remove both?
template <typename T>
struct typed_raw_cda<T const> :
	public typed_raw_cda<T> {
};
template <typename T>
struct typed_raw_cda<T const volatile> :
	public typed_raw_cda<T volatile> {
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::type_raw_cda


namespace abc {

/** Returns a void_cda populated with the static methods from a typed_raw_cda.

TODO: comment signature.
*/
template <class T>
/*constexpr*/ void_cda const & type_raw_cda() {
	static void_cda const sc_vrcda = {
		sizeof(T),
		alignof(T),
		reinterpret_cast<void_cda:: copy_fn>(typed_raw_cda<T>::copy_constr),
		reinterpret_cast<void_cda:: move_fn>(typed_raw_cda<T>::move_constr),
		reinterpret_cast<void_cda:: move_fn>(typed_raw_cda<T>::overlapping_move_constr),
		reinterpret_cast<void_cda::destr_fn>(typed_raw_cda<T>::destruct),
		reinterpret_cast<void_cda::equal_fn>(typed_raw_cda<T>::equal)
	};
	return sc_vrcda;
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_TYPE_RAW_CDA_HXX

