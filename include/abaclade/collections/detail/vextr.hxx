/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with Abaclade. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif

#include <abaclade/collections/detail/type_void_adapter.hxx>

/*! @page vextr_design High-efficiency strings and vectors
Design of abc::text::istr, abc::collections::mvector, and related classes.

Abaclade’s string and vector classes are intelligent wrappers around C arrays; they are able to
dynamically adjust the size of the underlying array, while also taking advantage of an optional
fixed-size array embedded into the string/vector object.

This table illustrates the best type of string to use for each use scenario:

   @verbatim
   ┌───────────────────────────────────────────────────────┬──────────────────────┐
   │ Functional need                                       │ Suggested type       │
   ├───────────────────────────────────────────────────────┼──────────────────────┤
   │ Local/member constant                                 │ istr const           │
   │                                                       │                      │
   │ Local/member immutable variable                       │ istr                 │
   │ (can be assigned to, but not modified)                │                      │
   │                                                       │                      │
   │ Local/member variable                                 │ smstr<expected size> │
   │                                                       │                      │
   │ Function argument                                     │                      │
   │ •  Input-only, temporary usage                        │ istr const &         │
   │ •  Input-only, to be moved to dmstr with longer scope │ dmstr                │
   │ •  Output-only (value on input ignored)               │ mstr *               │
   │ •  Non-const input                                    │ mstr &               │
   │                                                       │                      │
   │ Function return value                                 │                      │
   │ •  From string literal                                │ istr                 │
   │ •  Read-only reference to non-local variable          │ istr const &         │
   │ •  From local temporary string                        │ dmstr                │
   │ •  Reference to non-local variable                    │ mstr &               │
   │                                                       │                      │
   │ Value in container classes                            │ any except mstr      │
   │                                                       │                      │
   │ Key in hash-based container classes                   │ istr const           │
   └───────────────────────────────────────────────────────┴──────────────────────┘
   @endverbatim

Data-wise (see abc::collections::detail::raw_vextr_impl_data), the implementation stores two
pointers, one to the first item and one to beyond the last item, instead of the more common start
pointer/length pair of variables. This makes checking an iterator against the end of the array a
matter of a simple load/compare in terms of machine level instructions, as opposed to load/load/add/
compare as it’s necessary for the pointer/length pair. The item array pointed to by the begin/end
pointers can be part of a prefixed item array
(abc::collections::detail::raw_vextr_prefixed_item_array), which includes information such as the
total capacity of the item array, which is used to find out when the item array needs to be
reallocated to make room for more items.

While there are several implementations of these classes, they can all be grouped in two clades:
immutable and mutable/modifiable. The firsts behave much like Python’s strings or tuples, in that
they only expose observers; the seconds offer the entire range of modifiers as well, making them
more like strings/vectors in most other languages, but at the cost of a restricted range of options
for the allocation of their internal arrays.

The implementation of strings and vectors revolves around two class hierarchies: the lower-level
hierarchy creates a code path split between trivial (e.g. integral) types to exploit the inherent
exception safety of these without breaking the transactional guarantee for complex types; the upper-
level hierarchies hide parts of the lower-level classes’ interface so that transitions between their
different possible semantic statuses (e.g. mutable/immutable) have to be validated via the C++ type
system.

For vectors, the leaf-most classes have two implementations: one for movable-only types, and one for
copyable types.


The lower-level hierarchy consists in these non-template classes:

•  detail::raw_vextr_impl_base: core functionality for a vector of items: a little code and all
   member variables.

   •  detail::raw_complex_vextr_impl: implementation of a vector of objects of non-trivial class:
      this is fully transactional and therefore exception-proof, but it’s of course slower and uses
      more memory even during simpler operations.

   •  detail::raw_trivial_vextr_impl: implementation of a vector of plain values (instances of
      trivial class or native type): this is a near-optimal solution, still exception-proof but also
      taking advantage of the knowledge that no copy constructors need to be called.

Making these classes non-template allows avoiding template bloat, possibly at the expense of some
execution speed.

Note: vextr is a portmanteau of vector and str(ing), because most of the above classes are used by
both.

For vectors, the two leaf classes above are wrapped by an additional template layer,
abc::collections::detail::raw_vector, that eliminates any differences between the two interfaces
caused by the need for abc::collections::detail::raw_trivial_vextr_impl to also double as
implementation of the string classes.
The complete lower-level class hierarchy is therefore:

•  detail::raw_vextr_impl_base

   •  detail::raw_complex_vextr_impl / detail::raw_trivial_vextr_impl

      •  collections::detail::raw_vector: consolidates the trivial and complex interfaces into a
         single one by having two distinct specializations (trivial/non-trivial).

         •  collections::detail::vector_base: base for the upper-level vector class hierarchy.

      •  str_base: always derives from detail::raw_trivial_vextr_impl, it’s the base for the upper-
         level string class hierarchy.


The upper-level class hierarchies consist in these classes:

•  collections::detail::vector_base/str_base

   •  collections::ivector (TODO: if the need arises)/istr: immutable (assign-only) vector/string;
      uses a read-only (possibly shared) or dynamically-allocated item array, and as a consequence
      does not offer any modifier methods or operators beyond operator==(). Only class in its
      hierarchy that can be constructed from a C++ literal without creating a copy of the
      constructor argument.

   •  collections::mvector/mstr: mutable (fully modifiable) vector/string; always uses a writable,
      embedded or dynamically-allocated prefixed item array (never a non-prefixed, read-only item
      array), and provides an abstraction for its derived classes.

      •  collections::dmvector/dmstr: mutable vector/string that always uses a dynamically-allocated
         item array.

      •  collections::smvector/smstr: mutable vector/string that can use an internal embedded
         prefixed item array and will switch to a dynamically-allocated one only if necessary.
         Nearly as fast as the C-style direct manipulation of an array, only wasting a very small
         amount of space, and providing the ability to switch to a dynamically-allocated item array
         on-the-fly in case the client needs to store in it more items than are available.

The upper-level class hierarchy is arranged so that the compiler will prevent implicit cast of mstr
or smstr to istr &/dmstr &; allowing this would allow the istr/dmstr move constructor/assignment
operator to be invoked to transfer ownership of an embedded prefixed item array, which is wrong, or
(if falling back to copying the embedded item array into a new dynamically-allocated one) would
result in memory allocation and items being copied, which would violate the “nothrow” requirement
for the move constructor/assignment operator.

All string (TODO: and vector) types can be implicitly cast as istr const &; this makes istr const &
the lowest common denominator for string exchange, much like str is in Python. Because of this, an
istr const & can be using an embedded prefixed item array (because it can be a smstr), while any
other istr will always use either a read-only, non-prefixed item array or a dynamic prefixed item
array.

collections::mvector/mstr cannot have a “nothrow” move constructor or assignment operator from
itself, because the two underlying objects might have embedded item arrays of different sizes. This
isn’t a huge deal-breaker because of the intended limited usage for mstr and smstr, but it’s
something to watch out for, and it means that mstr should not be used in container classes.

Last but not least, let’s look at the underlying data storage in some of the possible semantic
statuses.

Key:

   @verbatim
   ┌──────────────┬──────────┬───────────┬───────────────┬────────────────┬─────────────────┐
   │ Pointer to   │ Pointer  │ P if item │ T if item     │ E is vextr has │ D if item array │
   │ beginning of │ to end   │ array is  │ array is NUL- │ embedded       │ is dynamically- │
   │ array        │ of array │ prefixed  │ terminated    │ prefixed array │ allocated       │
   └──────────────┴──────────┴───────────┴───────────────┴────────────────┴─────────────────┘
   @endverbatim

   Additionally, an embedded item array can follow, prefixed by its length (here in items, but in
   the implementation it’s actually a byte count).


1. istr() or dmstr(): no item array.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┬───┐
   │ nullptr │ nullptr │ - │ - │ - │ - │
   └─────────┴─────────┴───┴───┴───┴───┘
   @endverbatim

2. smstr<4>(): has an embedded prefixed fixed-size array, but does not use it yet.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┬───╥───┬─────────┐
   │ nullptr │ nullptr │ - │ - │ E │ - ║ 4 │ - - - - │
   └─────────┴─────────┴───┴───┴───┴───╨───┴─────────┘
   @endverbatim

3. istr("abc"): points to a non-prefixed item array in read-only memory, which also has a NUL
   terminator.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┬───┐                    ┌──────────┐
   │ 0xptr   │ 0xptr   │ - │ T │ - │ - │                    │ a b c \0 │
   └─────────┴─────────┴───┴───┴───┴───┘                    └──────────┘
     │         │                                            ▲       ▲
     │         └────────────────────────────────────────────│───────┘
     └──────────────────────────────────────────────────────┘
   @endverbatim

4. dmstr("abc"): points to a dynamically-allocated prefixed copy of the source string literal.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┬───┐                ┌───┬─────────────────┐
   │ 0xptr   │ 0xptr   │ P │ - │ - │ D │                │ 8 │ a b c - - - - - │
   └─────────┴─────────┴───┴───┴───┴───┘                └───┴─────────────────┘
     │         │                                            ▲       ▲
     │         └────────────────────────────────────────────│───────┘
     └──────────────────────────────────────────────────────┘
   @endverbatim

5. smstr<4> s4("abc"): copies the source string literal to the embedded prefixed item array, and
   points to it.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┬───╥───┬─────────┐
   │ 0xptr   │ 0xptr   │ P │ - │ E │ - ║ 4 │ a b c - │
   └─────────┴─────────┴───┴───┴───┴───╨───┴─────────┘
     │         │                           ▲       ▲
     │         └───────────────────────────│───────┘
     └─────────────────────────────────────┘
   @endverbatim

7. s4 += "abc": switches to a dynamically-allocated prefixed array because the embedded one is not
   large enough.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┬───╥───┬─────────┐  ┌───┬─────────────────┐
   │ 0xptr   │ 0xptr   │ P │ - │ E │ D ║ 4 │ - - - - │  │ 8 │ a b c a b c - - │
   └─────────┴─────────┴───┴───┴───┴───╨───┴─────────┘  └───┴─────────────────┘
     │         │                                            ▲             ▲
     │         └────────────────────────────────────────────│─────────────┘
     └──────────────────────────────────────────────────────┘
   @endverbatim
*/


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::detail::raw_vextr_prefixed_item_array

namespace abc {
namespace collections {
namespace detail {

/*! Stores an item array and its capacity. Used as a real template by classes with embedded item
array in the “upper level” hierarchy (see @ref vextr_design), and used with template capacity == 1
for all non-template-driven manipulations in non-template code in the “lower-level” hierarchy, which
relies on m_cbCapacity instead. */
template <typename T, std::size_t t_ciEmbeddedCapacity>
class raw_vextr_prefixed_item_array {
public:
   //! Embedded item array capacity, in bytes.
   static std::size_t const smc_cbEmbeddedCapacity = sizeof(T) * t_ciEmbeddedCapacity;
   /*! Actual capacity of m_at, in bytes. This depends on the memory that was allocated for *this,
   so it can be greater than smc_cbEmbeddedCapacity. */
   std::size_t m_cbCapacity;
   /*! Fixed-size item array. This can’t be a T[] because we don’t want its items to be constructed/
   destructed automatically, and because the count may be greater than what’s declared here. */
   abc::max_align_t m_at[ABC_ALIGNED_SIZE(smc_cbEmbeddedCapacity)];
};

} //namespace detail
} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::detail::raw_vextr_impl_data

namespace abc {
namespace collections {
namespace detail {

/*! Data members of raw_vextr_impl_base, as a plain old struct. This is the most basic
implementation block for all abc::text::*str and abc::collections::*vector classes. */
struct raw_vextr_impl_data {
   //! Pointer to the start of the item array.
   void * m_pBegin;
   //! Pointer to the end of the item array.
   void * m_pEnd;
   //! true if *this includes an embedded prefixed item array.
   bool /*const*/ mc_bEmbeddedPrefixedItemArray:1;
   //! true if the item array is part of a prefixed item array.
   bool m_bPrefixedItemArray:1;
   /*! true if the current item array is allocated dynamically, or false otherwise (embedded
   prefixed or non-prefixed). */
   bool m_bDynamic:1;
   //! true if the item array is NUL-terminated.
   bool m_bNulT:1;
};

} //namespace detail
} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::detail::raw_vextr_impl_base

namespace abc {
namespace collections {
namespace detail {

/*! Template-independent members of detail::raw_*_vextr_impl that are identical for trivial and non-
trivial types. */
class ABACLADE_SYM raw_vextr_impl_base : public raw_vextr_impl_data {
protected:
   // Allow transactions to access this class’s protected members.
   friend class raw_vextr_transaction;

public:
   /*! Non-template prefixed item array used for the calculation of offsets that will then be
   applied to real instantiations of the prefixed item array template. */
   typedef raw_vextr_prefixed_item_array<std::int8_t, 1> _prefixed_item_array;

public:
   //! Destructor.
   ~raw_vextr_impl_base() {
      if (m_bDynamic) {
         memory::_raw_free(prefixed_item_array());
      }
   }

   /*! Returns a pointer to the start of the item array.

   @return
      Pointer to the first item.
   */
   template <typename T>
   T * begin() {
      return static_cast<T *>(m_pBegin);
   }
   template <typename T>
   T const * begin() const {
      return static_cast<T const *>(m_pBegin);
   }

   /*! Returns the count of item slots in the current item array.

   @return
      Size of the item array.
   */
   template <typename T>
   std::size_t capacity() const {
      auto ppia(prefixed_item_array());
      return ppia ? ppia->m_cbCapacity / sizeof(T) : 0;
   }

   /*! Returns a pointer to the end of the item array.

   @return
      Pointer to beyond the last item.
   */
   template <typename T>
   T * end() {
      return static_cast<T *>(m_pEnd);
   }
   template <typename T>
   T const * end() const {
      return static_cast<T const *>(m_pEnd);
   }

   /*! Returns the count of items in the item array.

   @return
      Size of the item array.
   */
   template <typename T>
   std::size_t size() const {
      return static_cast<std::size_t>(end<T>() - begin<T>());
   }

private:
   /*! Internal constructor used by transaction. Access is private, but raw_vextr_transaction can
   use it.

   Note that this doesn’t really initialize the object!
   */
   raw_vextr_impl_base() {
      mc_bEmbeddedPrefixedItemArray = false;
      /* This is needed to disable the destructor, so we won’t try to release an invalid pointer in
      case anything goes wrong before the rest of the object is initialized. */
      m_bDynamic = false;
   }

protected:
   /*! Constructor. The overload with cbEmbeddedCapacity constructs the object as empty, setting
   m_pBegin/End to nullptr; the overload with pConstSrcBegin/End constructs the object assigning an
   item array.

   @param cbEmbeddedCapacity
      Size of the embedded prefixed item array, in bytes, or 0 if no embedded item array is present.
   @param pConstSrcBegin
      Pointer to the start of an array that will be adopted by the vextr as read-only.
   @param pConstSrcEnd
      Pointer to the end of the array.
   @param bNulT
      true if the array pointed to by pConstSrc is a NUL-terminated string, or false otherwise.
   */
   raw_vextr_impl_base(std::size_t cbEmbeddedCapacity);
   raw_vextr_impl_base(void const * pConstSrcBegin, void const * pConstSrcEnd, bool bNulT = false) {
      m_pBegin = const_cast<void *>(pConstSrcBegin);
      m_pEnd = const_cast<void *>(pConstSrcEnd);
      mc_bEmbeddedPrefixedItemArray = false;
      m_bPrefixedItemArray = false;
      m_bDynamic = false;
      m_bNulT = bNulT;
   }

   //! Resets the contents of the object to nullptr.
   void assign_empty() {
      m_pBegin = nullptr;
      m_pEnd = nullptr;
      m_bPrefixedItemArray = false;
      m_bDynamic = false;
      m_bNulT = false;
   }

   /*! Copies the data members of the source to *this.

   @param rvib
      Source vextr.
   */
   void assign_shallow(raw_vextr_impl_base const & rvib) {
      m_pBegin             = rvib.m_pBegin;
      m_pEnd               = rvib.m_pEnd;
      m_bPrefixedItemArray = rvib.m_bPrefixedItemArray;
      m_bDynamic           = rvib.m_bDynamic;
      m_bNulT              = rvib.m_bNulT;
   }

   /*! Calculates the new capacity for the item array for growing from cbOld to cbNew bytes while
   attempting to reduce future allocations for subsequent size increases.

   @param cbOld
      Previous (current) item array size, in bytes.
   @param cbNew
      New (future) item array size, in bytes.
   @return
      New item array capacity, in bytes.
   */
   static std::size_t calculate_increased_capacity(std::size_t cbOld, std::size_t cbNew);

   /*! Returns a pointer to the current prefixed item array, or nullptr if the current item array is
   not prefixed.

   @return
      Pointer to the prefixed item array, or nullptr if not applicable.
   */
   _prefixed_item_array * prefixed_item_array() {
      if (m_bPrefixedItemArray) {
         // Subtract from m_pBegin the offset of the item array.
         return reinterpret_cast<_prefixed_item_array *>(
            begin<std::int8_t>() - reinterpret_cast<std::ptrdiff_t>(
               reinterpret_cast<_prefixed_item_array *>(0)->m_at
            )
         );
      } else {
         return nullptr;
      }
   }
   _prefixed_item_array const * prefixed_item_array() const {
      return const_cast<raw_vextr_impl_base *>(this)->prefixed_item_array();
   }

   /*! Returns a pointer to the embedded prefixed item array that follows this object, if present.

   @return
      Pointer to the embedded item array, or nullptr otherwise.
   */
   _prefixed_item_array * embedded_prefixed_item_array() {
      if (mc_bEmbeddedPrefixedItemArray) {
         /* Allows to obtain the pointer to an embedded prefixed item array in non-template code
         without resorting to manual pointer arithmetics. */
#if ABC_HOST_CXX_MSC
   #pragma warning(push)
   /* “'class' : default constructor could not be generated because a base class default constructor
   is inaccessible” */
   #pragma warning(disable: 4623)
#endif
         class raw_vextr_impl_base_with_embedded_prefixed_item_array :
            public raw_vextr_impl_base,
            public raw_vextr_impl_base::_prefixed_item_array {
         };
#if ABC_HOST_CXX_MSC
   #pragma warning(pop)
#endif

         return static_cast<raw_vextr_impl_base_with_embedded_prefixed_item_array *>(this);
      } else {
         return nullptr;
      }
   }

   /*! Converts a possibly negative item byte offset into a pointer into the item array, throwing an
   index_error exception if the result is out of bounds for the item array.

   @param ib
      If positive, this is interpreted as a 0-based byte offset; if negative, it’s interpreted as a
      1-based byte offset from the end of the item array by adding this->size<std::int8_t>() to it.
   @return
      Pointer to the item.
   */
   void const * translate_offset(std::ptrdiff_t ib) const;

   /*! Converts a left-closed, right-open interval with possibly negative byte offsets into one
   consisting of two pointers into the item array.

   @param ibBegin
      Left endpoint of the interval, inclusive. If positive, this is interpreted as a 0-based byte
      offset; if negative, it’s interpreted as a 1-based byte offset from the end of the item array
      by adding this->size<std::int8_t>() to it.
   @param ibEnd
      Right endpoint of the interval, exclusive. If positive, this is interpreted as a 0-based byte
      offset; if negative, it’s interpreted as a 1-based byte offset from the end of the item array
      by adding this->size<std::int8_t>() to it.
   @return
      Left-closed, right-open interval such that return.first <= i < return.second, or the empty
      interval [nullptr, nullptr) if the offsets represent an empty interval after being adjusted.
   */
   std::pair<void const *, void const *> translate_byte_range(
      std::ptrdiff_t ibBegin, std::ptrdiff_t ibEnd
   ) const;

   /*! Validates that the specified pointer references an item within or at the end of the item
   array, throwing an index_error exception if it doesn’t. Similar to validate_pointer_noend(), but
   it accepts a pointer to the end of the item array.

   @param p
      Pointer to validate.
   */
   void validate_pointer(void const * p) const;

   /*! Validates that the specified pointer references an item within the item array, throwing an
   index_error exception if it doesn’t. Similar to validate_pointer(), but it rejects a pointer to
   the end of the item array.

   @param p
      Pointer to validate.
   */
   void validate_pointer_noend(void const * p) const;

protected:
   //! The item array size must be no less than this many bytes.
   static std::size_t const smc_cbCapacityMin = sizeof(std::ptrdiff_t) * 8;
   /*! Size multiplier. This should take into account that we want to reallocate as rarely as
   possible, so every time we do it it should be for a rather conspicuous growth. */
   static unsigned const smc_iGrowthRate = 2;
};

} //namespace detail
} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::detail::raw_vextr_transaction

namespace abc {
namespace collections {
namespace detail {

/*! Allows to get a temporary item array from a pool of options, then work with it, and upon
destruction it ensures that the array is either adopted by the associated
detail::raw_vextr_impl_base, or properly discarded.

A transaction will not take care of copying the item array, if switching to a different item array.

For size increases, the reallocation (if any) is performed in the constructor; for decreases, it’s
performed in commit(). */
class ABACLADE_SYM raw_vextr_transaction : public noncopyable {
public:
   /*! Constructor.

   @param prvib
      Subject of the transaction.
   @param bTrivial
      If true, the items are of a trivial type; if false, they’re not.
   @param cbNew
      New item array size, in bytes.
   @param cbAdd
      Item array size increase, in bytes.
   @param cbRemove
      Item array size decrease, in bytes.
   */
   raw_vextr_transaction(raw_vextr_impl_base * prvib, bool bTrivial, std::size_t cbNew);
   raw_vextr_transaction(
      raw_vextr_impl_base * prvib, bool bTrivial, std::size_t cbAdd, std::size_t cbRemove
   );

   //! Destructor.
   ~raw_vextr_transaction() {
      /* Only allow m_rvibWork to release its item array if we allocated it for the transaction and
      commit() was never called. */
      m_rvibWork.m_bDynamic = m_bFree;
   }

   /*! Commits the transaction; if the item array is to be replaced, the current one will be 
   released if necessary; it’s up to the client to destruct any items in it. If this method is not
   called before the transaction is destructed, it’s up to the client to also ensure that any and
   all objects constructed in the work array have been properly destructed. */
   void commit();

   /*! Returns the work item array.

   @return
      Pointer to the working item array.
   */
   template <typename T>
   T * work_array() const {
      return static_cast<T *>(m_rvibWork.m_pBegin);
   }

   /*! Returns true if the contents of the item array need to migrated due to the transaction
   switching item arrays. If the array was/will be only resized, the return value is false, because
   the reallocation did/will take care of moving the item array.

   @return
      true if the pointer to the item array will be changed upon destruction, or false otherwise.
   */
   bool will_replace_item_array() const {
      return m_rvibWork.m_pBegin != m_prvib->m_pBegin;
   }

private:
   /*! Completes construction of the object.

   @param bTrivial
      If true, the items are of a trivial type; if false, they’re not.
   @param cbNew
      New item array size, in bytes.
   */
   void _construct(bool bTrivial, std::size_t cbNew);

private:
   /*! Temporary vextr that contains the new values for each vextr member, ready to be applied to
   *m_prvib when the transaction is committed. Its internal pointers may or may not be the same as
   the ones in m_prvib depending on whether we needed a new item array. */
   raw_vextr_impl_base m_rvibWork;
   //! Subject of the transaction.
   raw_vextr_impl_base * m_prvib;
   /*! true if m_rvibWork references an item array that has been dynamically allocated for the
   transaction and needs to be freed in the destructor, which can happen when an exception occurs
   before the transaction is committed. */
   bool m_bFree;
};

} //namespace detail
} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::detail::raw_complex_vextr_impl

namespace abc {
namespace collections {
namespace detail {

//! Template-independent implementation of a vector for non-trivial contained types.
class ABACLADE_SYM raw_complex_vextr_impl : public raw_vextr_impl_base {
public:
   /*! Copies or moves the contents of the two sources to *this, according to the source type. If
   bMove{1,2} == true, the source items will be moved by having their const-ness cast away – be
   careful.

   @param type
      Adapter for the items’ type.
   @param p1Begin
      Pointer to the start of the first source array.
   @param p1End
      Pointer to the end of the first source array.
   @param p2Begin
      Pointer to the start of the second source array.
   @param p2End
      Pointer to the end of the second source array.
   @param iMove
      Pass 1 to move the items from the first source array to the vextr’s item array, 2 to move the
      items from the second source array, or 3 to move both, or 0 to copy them all instead.
   */
   void assign_concat(
      type_void_adapter const & type, void const * p1Begin, void const * p1End,
      void const * p2Begin, void const * p2End, std::uint8_t iMove
   );

   /*! Copies the contents of the source to *this.

   @param type
      Adapter for the items’ type.
   @param pBegin
      Pointer to the start of the source array.
   @param pEnd
      Pointer to the end of the source array.
   */
   void assign_copy(type_void_adapter const & type, void const * pBegin, void const * pEnd) {
      if (pBegin == m_pBegin) {
         return;
      }
      /* assign_concat() is fast enough; pass the source as the second argument pair, because its
      code path is faster. */
      assign_concat(type, nullptr, nullptr, pBegin, pEnd, 0);
   }

   /*! Moves the contents of the source to *this, taking ownership of the whole item array (items
   are not moved nor copied).

   @param type
      Adapter for the items’ type.
   @param rcvi
      Source vextr.
   */
   void assign_move(type_void_adapter const & type, raw_complex_vextr_impl && rcvi);

   /*! Moves the source’s item array if dynamically-allocated, else copies it to *this, moving the
   items instead.

   @param type
      Adapter for the items’ type.
   @param rcvi
      Source vextr.
   */
   void assign_move_dynamic_or_move_items(
      type_void_adapter const & type, raw_complex_vextr_impl && rcvi
   );

   /*! Destructs the item array. It does not deallocate the item array.

   @param type
      Adapter for the items’ type.
   */
   void destruct_items(type_void_adapter const & type) {
      type.destruct(m_pBegin, m_pEnd);
   }

   /*! Inserts items at a specific position in the vextr.

   @param type
      Adapter for the items’ type.
   @param ibOffset
      Byte index at which the items should be inserted.
   @param pInsert
      Pointer to the first item to insert.
   @param cbInsert
      Size of the array pointed to by pInsert, in bytes.
   @param bMove
      true to move the items from pInsert to the vextr’s item array, or false to copy them instead.
   */
   void insert(
      type_void_adapter const & type, std::size_t ibOffset, void const * pInsert,
      std::size_t cbInsert, bool bMove
   );

   /*! Removes items from the vextr.

   @param type
      Adapter for the items’ type.
   @param ibOffset
      Byte index at which the items should be removed.
   @param cbRemove
      Size of the array slice to remove, in bytes.
   */
   void remove(type_void_adapter const & type, std::size_t ibOffset, std::size_t cbRemove);

   /*! Ensures that the item array has at least ciMin of actual item space. If this causes *this to
   switch to using a different item array, any data in the current one will be lost unless bPreserve
   == true.

   @param type
      Adapter for the items’ type.
   @param cbMin
      Minimum size of items requested, in bytes.
   @param bPreserve
      If true, the previous contents of the item array will be preserved even if the reallocation
      causes the vextr to switch to a different item array.
   */
   void set_capacity(type_void_adapter const & type, std::size_t cbMin, bool bPreserve);

   /*! Changes the count of items in the vextr. If the new item count is greater than the current
   one, the added items will be left uninitialized; it’s up to the caller to make sure that these
   items are properly constructed, or problems will arise when the destructor will attempt to
   destruct these items.

   @param type
      Adapter for the items’ type.
   @param cb
      New size of the items, in bytes.
   */
   void set_size(type_void_adapter const & type, std::size_t cb);

protected:
   //! See raw_vextr_impl_base::raw_vextr_impl_base().
   raw_complex_vextr_impl(std::size_t cbEmbeddedCapacity) :
      raw_vextr_impl_base(cbEmbeddedCapacity) {
   }
   raw_complex_vextr_impl(void const * pConstSrcBegin, void const * pConstSrcEnd) :
      raw_vextr_impl_base(pConstSrcBegin, pConstSrcEnd) {
   }
};

} //namespace detail
} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::detail::raw_trivial_vextr_impl

namespace abc {
namespace collections {
namespace detail {

/*! Template-independent implementation of a vector for trivial contained types. This is the most
derived common base class of both vector and str. */
class ABACLADE_SYM raw_trivial_vextr_impl : public raw_vextr_impl_base {
public:
   /*! Copies the contents of the two sources to *this. This method must never be called with p1 or
   p2 == m_pBegin.

   @param p1Begin
      Pointer to the start of the first source array.
   @param p1End
      Pointer to the end of the first source array.
   @param p2Begin
      Pointer to the start of the second source array.
   @param p2End
      Pointer to the end of the second source array.
   */
   void assign_concat(
      void const * p1Begin, void const * p1End, void const * p2Begin, void const * p2End
   );

   /*! Copies the contents of the source array to *this.

   @param pBegin
      Pointer to the start of the source array.
   @param pEnd
      Pointer to the end of the source array.
   */
   void assign_copy(void const * pBegin, void const * pEnd) {
      if (pBegin == m_pBegin) {
         return;
      }
      /* assign_concat() is fast enough. Pass the source as the second argument pair, because its
      code path is faster. */
      assign_concat(nullptr, nullptr, pBegin, pEnd);
   }

   /*! Moves the source’s item array to *this. This must be called with rtvi being in control of a
   non-prefixed item array, or a dynamic prefixed item array; see @ref vextr_design to see how *str
   and *vector ensure this.

   @param rtvi
      Source vextr.
   */
   void assign_move(raw_trivial_vextr_impl && rtvi);

   /*! Moves the source’s item array if dynamically-allocated, else copies its items (not move –
   items are trivial) to *this.

   @param rtvi
      Source vextr.
   */
   void assign_move_dynamic_or_move_items(raw_trivial_vextr_impl && rtvi);

   /*! Shares the source’s item array if not prefixed, otherwise it creates a copy of the source
   prefixed item array for *this.

   @param rtvi
      Source vextr.
   */
   void assign_share_raw_or_copy_desc(raw_trivial_vextr_impl const & rtvi);

   /*! Inserts or removes items at a specific position in the vextr.

   @param ibOffset
      Byte index at which the items should be inserted or removed.
   @param pInsert
      Pointer to the first item to insert.
   @param cbInsert
      Size of the array pointed to be pInsert, in bytes.
   @param cbRemove
      Size of the slice of item array to remove, in bytes.
   */
   void insert_remove(
      std::size_t ibOffset, void const * pAdd, std::size_t cbAdd, std::size_t cbRemove
   ) {
      if (cbAdd != cbRemove) {
         _insert_remove(ibOffset, pAdd, cbAdd, cbRemove);
      }
   }

   /*! Ensures that the item array has at least cbMin of actual item space. If this causes *this to
   switch to using a different item array, any data in the current one will be lost unless bPreserve
   == true.

   @param cbMin
      Minimum size of items requested, in bytes.
   @param bPreserve
      If true, the previous contents of the item array will be preserved even if the reallocation
      causes the vextr to switch to a different item array.
   */
   void set_capacity(std::size_t cbMin, bool bPreserve);

   /*! Changes the count of items in the vextr. If the item array needs to be lengthened, the added
   items will be left uninitialized.

   @param cb
      New size of the items, in bytes.
   */
   void set_size(std::size_t cb);

protected:
   //! See raw_vextr_impl_base::raw_vextr_impl_base().
   raw_trivial_vextr_impl(std::size_t cbEmbeddedCapacity) :
      raw_vextr_impl_base(cbEmbeddedCapacity) {
   }
   raw_trivial_vextr_impl(
      void const * pConstSrcBegin, void const * pConstSrcEnd, bool bNulT = false
   ) :
      raw_vextr_impl_base(pConstSrcBegin, pConstSrcEnd, bNulT) {
   }

private:
   //! Implementation of insert_remove(). See insert_remove().
   void _insert_remove(
      std::size_t ibOffset, void const * pAdd, std::size_t cbAdd, std::size_t cbRemove
   );
};

} //namespace detail
} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
