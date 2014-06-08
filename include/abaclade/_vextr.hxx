/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> instead of this file
#endif



/** DOC:4019 abc::*str and abc::*vector design

abc::*str and abc::*vectors are intelligent wrappers around C arrays; they are able to dynamically
adjust the size of the underlying array, while also taking advantage of an optional fixed-size array
embedded into the string/vector object.

Data-wise, the implementation stores two pointers, one to the first item and one to beyond the last
item, instead of the more common start pointer/length pair of variables. This makes checking an
iterator against the end of the array a matter of a simple load/compare in terms of machine level
instructions, as opposed to load/load/add/compare as it’s necessary for the pointer/length pair.

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

•  _raw_vextr_impl_base: core functionality for a vector of items: a little code and all member
   variables.

   •  _raw_complex_vextr_impl: implementation of a vector of objects of non-trivial class: this is
      fully transactional and therefore exception-proof, but it’s of course slower and uses more
      memory even during simpler operations.

   •  _raw_trivial_vextr_impl: implementation of a vector of plain values (instances of trivial
      class or native type): this is a near-optimal solution, still exception-proof but also taking
      advantage of the knowledge that no copy constructors need to be called.

Making these classes non-template allows avoiding template bloat, possibly at the expense of some
execution speed.

Note: vextr is a portmanteau of vector and str(ing), because most of the above classes are used by
both.

For vectors, the two leaf classes above are wrapped by an additional template layer,
abc::_raw_vector, that eliminates any differences between the two interfaces caused by the need for
abc::_raw_trivial_vextr_impl to also double as implementation of the string classes. The complete
lower-level class hierarchy is therefore:

•  _raw_vextr_impl_base

   •  _raw_complex_vextr_impl / _raw_trivial_vextr_impl

      •  _raw_vector: consolidates the trivial and complex interfaces into a single one by having
         two distinct specializations (trivial/non-trivial).

         •  vector_base: base for the upper-level vector class hierarchy.

      •  str_base: always derives from _raw_trivial_vextr_impl, it’s the base for the upper-level
         string class hierarchy.


The upper-level class hierarchies consist in these classes:

•  vector_base/str_base

   •  ivector (TODO: if the need arises)/istr: immutable (assign-only) vector/string; uses a read-
      only (possibly shared) or dynamically-allocated item array, and as a consequence does not
      offer any modifier methods or operators beyond operator==(). Only class in its hierarchy that
      can be constructed from a C++ literal without creating a copy of the constructor argument.

   •  mvector/mstr: mutable (fully modifiable) vector/string; always uses a writable, statically- or
      dynamically-allocated item array (never read-only nor shared), and provides an abstraction for
      its derived classes.

      •  dmvector/dmstr: mutable vector/string that always uses a dynamically-allocated item array.

      •  smvector/smstr: mutable vector/string that can use an internal statically-allocated item
         array and will switch to a dynamically-allocated one only if necessary. Nearly as fast as
         the C-style direct manipulation of an array, only wasting a very small amount of space, and
         providing the ability to switch to a dynamically-allocated item array on-the-fly in case
         the client needs to store in it more items than are available.

The upper-level class hierarchy is arranged so that the compiler will prevent implicit cast of mstr
or smstr to istr &/dmstr &; allowing this would allow the istr/dmstr move constructor/assignment
operator to be invoked to transfer ownership of a static item array, which is wrong, or (if falling
back to copying the static item array into a new dynamic one) would result in memory allocation and
items being copied, which would violate the “nothrow” requirement for the move constructor/
assignment operator.

All string (TODO: and vector) types can be implicitly cast as istr const &; this makes istr const &
the lowest common denominator for string exchange, much like str is in Python. Because of this, an
istr const & can be using a static item array (because it can be a smstr), while any other istr will
always use a const/read-only item array or a dynamic one.

mvector/mstr cannot have a “nothrow” move constructor or assignment operator from itself, because
the two underlying objects might have static item arrays of different sizes. This isn’t a huge deal-
breaker because of the intended limited usage for mstr and smstr, but it’s something to watch out
for, and it means that mstr should not be used in container classes.

This table illustrates the best type of string to use for each use scenario:

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


Last but not least, let’s look at the underlying data storage in some of the possible semantic
statuses.

Key:

   ┌──────────────┬──────────┬──────────────────┬───────────────┬────────────────┬─────────────────┐
   │ Pointer to   │ Pointer  │ Capacity of item │ T if item     │ E is vextr has │ D if item array │
   │ beginning of │ to end   │ array, or 0 if   │ array is NUL- │ embedded       │ is dynamically- │
   │ array        │ of array │ it’s read-only   │ terminated    │ static array   │ allocated       │
   └──────────────┴──────────┴──────────────────┴───────────────┴────────────────┴─────────────────┘

   Additionally, an embedded item array can follow, prefixed by its length (here in items, but in
   the implementation it’s actually a byte count).


1. istr() or dmstr(): no item array.
   ┌─────────┬─────────┬───┬───┬───┬───┐
   │ nullptr │ nullptr │ 0 │ - │ - │ - │
   └─────────┴─────────┴───┴───┴───┴───┘

2. smstr<5>(): has a static embedded fixed-size buffer, but does not use it yet.
   ┌─────────┬─────────┬───┬───┬───┬───╥───┬───────────┐
   │ nullptr │ nullptr │ 0 │ - │ E │ - ║ 5 │ - - - - - │
   └─────────┴─────────┴───┴───┴───┴───╨───┴───────────┘

3. istr("abc"): points to read-only memory, which also has a NUL terminator.
   ┌─────────┬─────────┬───┬───┬───┬───┐                ┌──────────┐
   │ 0xptr   │ 0xptr   │ 0 │ T │ - │ - │                │ a b c \0 │
   └─────────┴─────────┴───┴───┴───┴───┘                └──────────┘
     │         │                                        ▲       ▲
     │         └────────────────────────────────────────│───────┘
     └──────────────────────────────────────────────────┘

4. dmstr("abc"): points to a dynamically-allocated copy of the source string literal.
   ┌─────────┬─────────┬───┬───┬───┬───┐                ┌─────────────────┐
   │ 0xptr   │ 0xptr   │ 8 │ - │ - │ D │                │ a b c - - - - - │
   └─────────┴─────────┴───┴───┴───┴───┘                └─────────────────┘
     │         │                                        ▲       ▲
     │         └────────────────────────────────────────│───────┘
     └──────────────────────────────────────────────────┘

5. smstr<4> s4("abc"): copies the source string literal to the embedded array, and points to it.
   ┌─────────┬─────────┬───┬───┬───┬───╥───┬─────────┐
   │ 0xptr   │ 0xptr   │ 4 │ - │ E │ - ║ 4 │ a b c - │
   └─────────┴─────────┴───┴───┴───┴───╨───┴─────────┘
     │         │                           ▲       ▲
     │         └───────────────────────────│───────┘
     └─────────────────────────────────────┘

7. s4 += "abc": switches to a dynamically-allocated buffer because the embedded one is too small.
   ┌─────────┬─────────┬───┬───┬───┬───╥───┬─────────┐  ┌─────────────────┐
   │ 0xptr   │ 0xptr   │ 8 │ - │ E │ D ║ 4 │ - - - - │  │ a b c - - - - - │
   └─────────┴─────────┴───┴───┴───┴───╨───┴─────────┘  └─────────────────┘
     │         │                                        ▲       ▲
     │         └────────────────────────────────────────│───────┘
     └──────────────────────────────────────────────────┘
*/

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_vextr_packed_data


namespace abc {

class _raw_vextr_packed_data {
public:

   /** Constructor.

   bHasStatic
      true if the parent object is followed by a static item array, or false otherwise.
   bNulT
      true if the item array ends in a NUL terminator, or false otherwise.
   */
   _raw_vextr_packed_data(bool bHasStatic, bool bNulT) :
      m_iPackedData(
         (bHasStatic ? smc_bHasStaticMask : 0) |
         (bNulT ? smc_bNulTMask : 0)
      ) {
   }


   /** Returns true if the parent object’s m_pBegin/End point to a dynamically-allocated item array.

   return
      true if the item array is allocated dynamically, or false otherwise (static or read-only).
   */
   bool dynamic() const {
      return (m_iPackedData & smc_bDynamicMask) != 0;
   }


   /** Returns true if the parent object is followed by a static item array.

   return
      true if the object also has a static item array, or false otherwise.
   */
   bool has_static_item_array() const {
      return (m_iPackedData & smc_bHasStaticMask) != 0;
   }


   /** Returns true if the parent object’s m_pBegin/End points to a NUL-terminated item array.

   return
      true if the item array ends in a NUL terminator, or false otherwise.
   */
   bool nul_terminated() const {
      return (m_iPackedData & smc_bNulTMask) != 0;
   }


   /** TODO: comment.
   */
   bool real_item_array() const {
      return (m_iPackedData & smc_bRealItemArrayMask) != 0;
   }


   /** Sets the bit tracking whether the parent object’s m_pBegin/End point to a dynamically-
   allocated item array.

   bDynamic
      true if the item array is allocated dynamically, or false otherwise (static or read-only).
   */
   void set_dynamic(bool bDynamic) {
      m_iPackedData &= ~smc_bDynamicMask;
      if (bDynamic) {
         m_iPackedData |= smc_bDynamicMask;
      }
   }


   /** Sets the bit tracking whether the parent object’s m_pBegin/End points to a NUL-terminated
   item array.

   bNulT
      true if the item array ends in a NUL terminator, or false otherwise.
   */
   void set_nul_terminated(bool bNulT) {
      m_iPackedData &= ~smc_bNulTMask;
      if (bNulT) {
         m_iPackedData |= smc_bNulTMask;
      }
   }


   /** TODO: comment.
   */
   void set_real_item_array(bool bRealItemArray) {
      m_iPackedData &= ~smc_bRealItemArrayMask;
      if (bRealItemArray) {
         m_iPackedData |= smc_bRealItemArrayMask;
      }
   }


private:

   /** Bit-field composed by the following components:

   bool bRealItemArray
      true if the item array is read-only.
   bool bDynamic
      true if the item array is allocated dynamically, or false otherwise (static or read-only).
   bool const bHasStatic
      true if the parent object is followed by a static item array.
   bool bNulT
      true if the item array is NUL-terminated.
   size_t cbCapacity
      Size of the item array, in bytes.
   */
   size_t m_iPackedData;

   /** Mask to access bRealItemArray from m_iPackedData. */
   static size_t const smc_bRealItemArrayMask = 0x08;
   /** Mask to access bDynamic from m_iPackedData. */
   static size_t const smc_bDynamicMask = 0x04;
   /** Mask to access bHasStatic from m_iPackedData. */
   static size_t const smc_bHasStaticMask = 0x02;
   /** Mask to access bNulT from m_iPackedData. */
   static size_t const smc_bNulTMask = 0x01;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_vextr_item_array


namespace abc {

/** Used to find out what the offset are for an embedded static item array.
*/
template <typename T, size_t t_ciStaticCapacity>
class _raw_vextr_item_array {
public:

   /** Static item array capacity, in bytes. */
   static size_t const smc_cbStaticCapacity = sizeof(T) * t_ciStaticCapacity;
   /** Actual capacity of m_at, in bytes. This depends on the memory that was allocated for *this,
   so it can be greater than smc_cbStaticCapacity. */
   size_t m_cbCapacity;
   /** Static item array. This can’t be a T[], because we don’t want its items to be constructed/
   destructed automatically, and because the count may be greater than what’s declared here. */
   std::max_align_t m_at[ABC_ALIGNED_SIZE(smc_cbStaticCapacity)];
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_vextr_impl_base


namespace abc {

/** Template-independent members of _raw_*_vextr_impl that are identical for trivial and non-trivial
types.
*/
class ABACLADE_SYM _raw_vextr_impl_base {
protected:

   // Allow transactions to access this class’s protected members.
   friend class _raw_vextr_transaction;


public:

   /** Dummy item array type used for the calculation of offsets that will then be applied to real
   instantiations of the item array template. */
   typedef _raw_vextr_item_array<int8_t, 1> dummy_item_array;


public:

   /** Destructor.
   */
   ~_raw_vextr_impl_base() {
      if (m_rvpd.dynamic()) {
         memory::_raw_free(item_array());
      }
   }


   /** Returns a pointer to the start of the item array.

   return
      Pointer to the first element.
   */
   template <typename T>
   T * begin() {
      return static_cast<T *>(m_pBegin);
   }
   template <typename T>
   T const * begin() const {
      return static_cast<T const *>(m_pBegin);
   }


   /** Returns the count of item slots in the current item array.

   return
      Size of the item array.
   */
   template <typename T>
   size_t capacity() const {
      return m_rvpd.real_item_array() ? item_array()->m_cbCapacity / sizeof(T) : 0;
   }


   /** Returns a pointer to the end of the item array.

   return
      Pointer to beyond the last element.
   */
   template <typename T>
   T * end() {
      return static_cast<T *>(m_pEnd);
   }
   template <typename T>
   T const * end() const {
      return static_cast<T const *>(m_pEnd);
   }


   /** Returns the count of items in the item array.

   return
      Size of the item array.
   */
   template <typename T>
   size_t size() const {
      return size_t(end<T>() - begin<T>());
   }


private:

   /** Internal constructor used by transaction. Note that this leaves the object in an inconsistent
   and uninitialized state!

   rvpd
      Source packed data.
   */
   _raw_vextr_impl_base(_raw_vextr_packed_data const & rvpd) :
      m_rvpd(rvpd) {
   }


protected:

   /** Constructor. The overload with cbStaticCapacity constructs the object as empty, setting
   m_pBegin/End to nullptr; the overload with pConstSrcBegin/End constructs the object assigning an
   item array.

   cbStaticCapacity
      Size of the static item array, in bytes, or 0 if no static item array is present.
   pConstSrcBegin
      Pointer to the start of an array that will be adopted by the vextr as read-only.
   pConstSrcEnd
      Pointer to the end of the array.
   bNulT
      true if the array pointed to by pConstSrc is a NUL-terminated string, or false otherwise.
   */
   _raw_vextr_impl_base(size_t cbStaticCapacity);
   _raw_vextr_impl_base(
      void const * pConstSrcBegin, void const * pConstSrcEnd, bool bNulT = false
   ) :
      m_pBegin(const_cast<void *>(pConstSrcBegin)),
      m_pEnd(const_cast<void *>(pConstSrcEnd)),
      // This will set m_rvpd.capacity() to 0, meaning that the item array is read-only.
      m_rvpd(false, bNulT) {
   }


   /** Resets the contents of the object to nullptr.
   */
   void assign_empty() {
      m_pBegin = m_pEnd = nullptr;
      m_rvpd.set_dynamic(false);
      m_rvpd.set_nul_terminated(false);
      m_rvpd.set_real_item_array(false);
   }


   /** Calculates the new capacity for the item array for growing from cbOld to cbNew bytes while
   attempting to reduce future allocations for subsequent size increases.

   cbOld
      Previous (current) item array size, in bytes.
   cbNew
      New (future) item array size, in bytes.
   return
      New item array capacity, in bytes.
   */
   static size_t calculate_increased_capacity(size_t cbOld, size_t cbNew);


   /** Returns a pointer to the current item array structure.

   return
      Pointer to the item array.
   */
   dummy_item_array * item_array() {
      // Subtract from m_pBegin the offset of the item array.
      return reinterpret_cast<dummy_item_array *>(begin<int8_t>() - reinterpret_cast<ptrdiff_t>(
         &reinterpret_cast<dummy_item_array *>(0)->m_at[0]
      ));
   }
   dummy_item_array const * item_array() const {
      return const_cast<_raw_vextr_impl_base *>(this)->item_array();
   }


   /** Returns a pointer to the static item array that follows this object, if present, or nullptr
   otherwise.

   return
      Pointer to the static item array, or nullptr if no static item array is present.
   */
   template <typename T>
   T * static_array_ptr();


   /** Returns the size of the array returned by static_array_ptr(), or 0 if no such array is
   present.

   return
      Capacity of the static item array, in bytes, or 0 if no static item array is present.
   */
   size_t static_capacity() const;


   /** Converts a possibly negative item byte offset into a pointer into the item array, throwing an
   index_error exception if the result is out of bounds for the item array.

   ib
      If positive, this is interpreted as a 0-based byte offset; if negative, it’s interpreted as a
      1-based byte offset from the end of the item array by adding this->size<int8_t>() to it.
   return
      Pointer to the item.
   */
   void const * translate_offset(intptr_t ib) const;


   /** Converts a left-closed, right-open interval with possibly negative byte offsets into one
   consisting of two pointers into the item array.

   ibBegin
      Left endpoint of the interval, inclusive. If positive, this is interpreted as a 0-based byte
      offset; if negative, it’s interpreted as a 1-based byte offset from the end of the item array
      by adding this->size<int8_t>() to it.
   ibEnd
      Right endpoint of the interval, exclusive. If positive, this is interpreted as a 0-based byte
      offset; if negative, it’s interpreted as a 1-based byte offset from the end of the item array
      by adding this->size<int8_t>() to it.
   return
      Left-closed, right-open interval such that return.first <= i < return.second, or the empty
      interval [nullptr, nullptr) if the offsets represent an empty interval after being adjusted.
   */
   std::pair<void const *, void const *> translate_byte_range(
      intptr_t ibBegin, intptr_t ibEnd
   ) const;


   /** Validates that the specified pointer references an item within or at the end of the item
   array, throwing an index_error exception if it doesn’t. Similar to validate_pointer_noend(), but
   it accepts a pointer to the end of the item array.

   p
      Pointer to validate.
   */
   void validate_pointer(void const * p) const;


   /** Validates that the specified pointer references an item within the item array, throwing an
   index_error exception if it doesn’t. Similar to validate_pointer(), but it rejects a pointer to
   the end of the item array.

   p
      Pointer to validate.
   */
   void validate_pointer_noend(void const * p) const;


protected:

   /** Pointer to the start of the item array. */
   void * m_pBegin;
   /** Pointer to the end of the item array. */
   void * m_pEnd;
   /** Size of the item array pointed to by m_pBegin, and other bits. */
   _raw_vextr_packed_data m_rvpd;

   /** The item array size must be no less than this many bytes. */
   static size_t const smc_cbCapacityIncrement = sizeof(intptr_t) * 32;
   /** The item array size must be no less than this many bytes. */
   static size_t const smc_cbCapacityMin = sizeof(intptr_t) * 8;
   /** Size multiplier. This should take into account that we want to reallocate as rarely as
   possible, so every time we do it it should be for a rather conspicuous growth. */
   static unsigned const smc_iGrowthRate = 2;
};


/** Used to find out what the offsets are for an embedded static item array.
*/
class _raw_vextr_impl_base_with_static_item_array :
   public _raw_vextr_impl_base,
   public _raw_vextr_impl_base::dummy_item_array {
};


// Now these can be implemented.

template <typename T>
inline T * _raw_vextr_impl_base::static_array_ptr() {
   if (!m_rvpd.has_static_item_array()) {
      return nullptr;
   }
   _raw_vextr_impl_base_with_static_item_array * prvibwsia(
      static_cast<_raw_vextr_impl_base_with_static_item_array *>(this)
   );
   return reinterpret_cast<T *>(prvibwsia->m_at);
}


inline size_t _raw_vextr_impl_base::static_capacity() const {
   if (!m_rvpd.has_static_item_array()) {
      return 0;
   }
   _raw_vextr_impl_base_with_static_item_array const * prvibwsia(
      static_cast<_raw_vextr_impl_base_with_static_item_array const *>(this)
   );
   return prvibwsia->m_cbCapacity;
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_vextr_transaction


namespace abc {

/** Allows to get a temporary item array from a pool of options, then work with it, and upon
destruction it ensures that the array is either adopted by the associated _raw_vextr_impl_base, or
properly discarded.

A transaction will not take care of copying the item array, if switching to a different item array.

For size increases, the reallocation (if any) is performed in the constructor; for decreases, it’s
performed in commit().
*/
class ABACLADE_SYM _raw_vextr_transaction :
   public noncopyable {
public:

   /** Constructor.

   prvib
      Subject of the transaction.
   cbNew
      New item array size, in bytes.
   cbAdd
      Item array size increase, in bytes.
   cbRemove
      Item array size decrease, in bytes.
   */
   _raw_vextr_transaction(_raw_vextr_impl_base * prvib, size_t cbNew);
   _raw_vextr_transaction(_raw_vextr_impl_base * prvib, size_t cbAdd, size_t cbRemove);


   /** Destructor.
   */
   ~_raw_vextr_transaction() {
      // Only release m_rvpdWork’s item array on this condition. In all other cases, the memory it’s
      // pointing to belongs to *m_prvib.
      m_rvibWork.m_rvpd.set_dynamic(m_bFree);
   }


   /** Commits the transaction; if the item array is to be replaced, the current one will be 
   released if necessary; it’s up to the client to destruct any items in it. If this method is not
   called before the transaction is destructed, it’s up to the client to also ensure that any and
   all objects constructed in the work array have been properly destructed.
   */
   void commit();


   /** Returns the work item array.

   return
      Pointer to the working item array.
   */
   template <typename T>
   T * work_array() const {
      return static_cast<T *>(m_rvibWork.m_pBegin);
   }


   /** Returns true if the contents of the item array need to migrated due to the transaction
   switching item arrays. If the array was/will be only resized, the return value is false, because
   the reallocation did/will take care of moving the item array.

   return
      true if the pointer to the item array will be changed upon destruction, or false otherwise.
   */
   bool will_replace_item_array() const {
      return m_rvibWork.m_pBegin != m_prvib->m_pBegin;
   }


private:

   /** Completes construction of the object.

   prvib
      Subject of the transaction.
   cbNew
      New item array size, in bytes.
   */
   void _construct(_raw_vextr_impl_base * prvib, size_t cbNew);


private:

   /** Temporary vextr that contains the new values for each vextr member, ready to be applied to
   *m_prvib when the transaction is committed. Its internal pointers may or may not be the same as
   the ones in m_prvib depending on whether we needed a new item array. */
   _raw_vextr_impl_base m_rvibWork;
   /** Subject of the transaction. */
   _raw_vextr_impl_base * m_prvib;
   /** true if m_rvibWork references an item array that has been dynamically allocated for the
   transaction and needs to be freed in the destructor. This can be the case because the transaction
   didn’t get committed, or because it did and m_rvibWork’s item array is now owned by m_prvib. */
   bool m_bFree;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_complex_vextr_impl


namespace abc {

/** Template-independent implementation of a vector for non-trivial contained types.
*/
class ABACLADE_SYM _raw_complex_vextr_impl :
   public _raw_vextr_impl_base {
public:

   /** Copies or moves the contents of the two sources to *this, according to the source type. If
   bMove{1,2} == true, the source items will be moved by having their const-ness cast away ‒ be
   careful.

   type
      Adapter for the items’ type.
   p1Begin
      Pointer to the start of the first source array.
   p1End
      Pointer to the end of the first source array.
   bMove1
      true to move the items from the first source array to the vextr’s item array, or false to
      copy them instead.
   p2Begin
      Pointer to the start of the second source array.
   p2End
      Pointer to the end of the second source array.
   bMove2
      true to move the items from the second source array to the vextr’s item array, or false to
      copy them instead.
   */
   void assign_concat(
      type_void_adapter const & type,
      void const * p1Begin, void const * p1End, bool bMove1,
      void const * p2Begin, void const * p2End, bool bMove2
   );


   /** Copies the contents of the source to *this.

   type
      Adapter for the items’ type.
   pBegin
      Pointer to the start of the source array.
   pEnd
      Pointer to the end of the source array.
   */
   void assign_copy(type_void_adapter const & type, void const * pBegin, void const * pEnd) {
      if (pBegin == m_pBegin) {
         return;
      }
      // assign_concat() is fast enough. Pass the source as the second argument pair, because its
      // code path is faster.
      assign_concat(type, nullptr, nullptr, false, pBegin, pEnd, false);
   }


   /** Moves the contents of the source to *this, taking ownership of the whole item array (items
   are not moved nor copied).

   type
      Adapter for the items’ type.
   rcvi
      Source vextr.
   */
   void assign_move(type_void_adapter const & type, _raw_complex_vextr_impl && rcvi);


   /** Moves the source’s item array if dynamically-allocated, else copies it to *this, moving the
   items instead.

   type
      Adapter for the items’ type.
   rcvi
      Source vextr.
   */
   void assign_move_dynamic_or_move_items(
      type_void_adapter const & type, _raw_complex_vextr_impl && rcvi
   );


   /** Destructs the item array. It does not deallocate the item array.

   type
      Adapter for the items’ type.
   */
   void destruct_items(type_void_adapter const & type) {
      type.destruct(m_pBegin, m_pEnd);
   }


   /** Inserts elements at a specific position in the vextr.

   type
      Adapter for the items’ type.
   ibOffset
      Byte index at which the items should be inserted.
   pInsert
      Pointer to the first item to insert.
   cbInsert
      Size of the array pointed to by pInsert, in bytes.
   bMove
      true to move the items from pInsert to the vextr’s item array, or false to copy them instead.
   */
   void insert(
      type_void_adapter const & type, uintptr_t ibOffset, void const * pInsert, size_t cbInsert,
      bool bMove
   );


   /** Removes items from the vextr.

   type
      Adapter for the items’ type.
   ibOffset
      Byte index at which the items should be removed.
   cbRemove
      Size of the array slice to be removed, in bytes.
   */
   void remove(type_void_adapter const & type, uintptr_t ibOffset, size_t cbRemove);


   /** Ensures that the item array has at least ciMin of actual item space. If this causes *this to
   switch to using a different item array, any data in the current one will be lost unless bPreserve
   == true.

   type
      Adapter for the items’ type.
   cbMin
      Minimum size of items requested, in bytes.
   bPreserve
      If true, the previous contents of the item array will be preserved even if the reallocation
      causes the vextr to switch to a different item array.
   */
   void set_capacity(type_void_adapter const & type, size_t cbMin, bool bPreserve);


   /** Changes the count of items in the vextr. If the new item count is greater than the current
   one, the added elements will be left uninitialized; it’s up to the caller to make sure that these
   elements are properly constructed, or problems will arise when the destructor will attempt to
   destruct these elements.

   type
      Adapter for the items’ type.
   cb
      New size of the items, in bytes.
   */
   void set_size(type_void_adapter const & type, size_t cb);


protected:

   /** Constructor. See _raw_vextr_impl_base::_raw_vextr_impl_base().
   */
   _raw_complex_vextr_impl(size_t cbStaticCapacity) :
      _raw_vextr_impl_base(cbStaticCapacity) {
   }
   _raw_complex_vextr_impl(void const * pConstSrcBegin, void const * pConstSrcEnd) :
      _raw_vextr_impl_base(pConstSrcBegin, pConstSrcEnd) {
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_trivial_vextr_impl


namespace abc {

/** Template-independent implementation of a vector for trivial contained types. This is the most
derived common base class of both vector and str.
*/
class ABACLADE_SYM _raw_trivial_vextr_impl :
   public _raw_vextr_impl_base {
public:

   /** Copies the contents of the two sources to *this. This method must never be called with p1 or
   p2 == m_pBegin.

   p1Begin
      Pointer to the start of the first source array.
   p1End
      Pointer to the end of the first source array.
   p2Begin
      Pointer to the start of the second source array.
   p2End
      Pointer to the end of the second source array.
   */
   void assign_concat(
      void const * p1Begin, void const * p1End, void const * p2Begin, void const * p2End
   );


   /** Copies the contents of the source array to *this.

   pBegin
      Pointer to the start of the source array.
   pEnd
      Pointer to the end of the source array.
   */
   void assign_copy(void const * pBegin, void const * pEnd) {
      if (pBegin == m_pBegin) {
         return;
      }
      // assign_concat() is fast enough. Pass the source as the second argument pair, because its
      // code path is faster.
      assign_concat(nullptr, nullptr, pBegin, pEnd);
   }


   /** Moves the source’s item array to *this. This must be called with rtvi being in control of a
   read-only or dynamic item array; see [DOC:4019 abc::*str and abc::*vector design] to see how str
   and vector ensure this.

   rtvi
      Source vextr.
   */
   void assign_move(_raw_trivial_vextr_impl && rtvi) {
      if (rtvi.m_pBegin == m_pBegin) {
         return;
      }
      // Share the item array.
      _assign_share(rtvi);
      // And now empty the source.
      rtvi.assign_empty();
   }


   /** Moves the source’s item array if dynamically-allocated, else copies its items (not move –
   items are trivial) to *this.

   rtvi
      Source vextr.
   */
   void assign_move_dynamic_or_move_items(_raw_trivial_vextr_impl && rtvi);


   /** Shares the source’s item array if read-only, else copies it to *this.

   rtvi
      Source vextr.
   */
   void assign_share_ro_or_copy(_raw_trivial_vextr_impl const & rtvi) {
      if (rtvi.m_pBegin == m_pBegin) {
         return;
      }
      if (m_rvpd.real_item_array()) {
         _assign_share(rtvi);
      } else {
         // Non-read-only, cannot share.
         assign_copy(rtvi.m_pBegin, rtvi.m_pEnd);
      }
   }


   /** Inserts elements at a specific position in the vextr.

   ibOffset
      Byte index at which the items should be inserted.
   pInsert
      Pointer to the first item to insert.
   cbInsert
      Size of the array pointed to by pInsert, in bytes.
   */
   void insert(uintptr_t ibOffset, void const * pInsert, size_t cbInsert) {
      if (cbInsert) {
         _insert_or_remove(ibOffset, pInsert, cbInsert, 0);
      }
   }


   /** Removes items from the vextr.

   ibOffset
      Byte index at which the items should be removed.
   cbRemove
      Size of the array slice to be removed, in bytes.
   */
   void remove(uintptr_t ibOffset, size_t cbRemove) {
      if (cbRemove) {
         _insert_or_remove(ibOffset, nullptr, 0, cbRemove);
      }
   }


   /** Ensures that the item array has at least cbMin of actual item space. If this causes *this to
   switch to using a different item array, any data in the current one will be lost unless bPreserve
   == true.

   cbMin
      Minimum size of items requested, in bytes.
   bPreserve
      If true, the previous contents of the item array will be preserved even if the reallocation
      causes the vextr to switch to a different item array.
   */
   void set_capacity(size_t cbMin, bool bPreserve);


   /** Changes the count of elements in the vextr. If the item array needs to be lengthened, the
   added elements will be left uninitialized.

   cb
      New size of the items, in bytes.
   */
   void set_size(size_t cb);


protected:

   /** Constructor. See _raw_vextr_impl_base::_raw_vextr_impl_base().
   */
   _raw_trivial_vextr_impl(size_t cbStaticCapacity) :
      _raw_vextr_impl_base(cbStaticCapacity) {
   }
   _raw_trivial_vextr_impl(
      void const * pConstSrcBegin, void const * pConstSrcEnd, bool bNulT = false
   ) :
      _raw_vextr_impl_base(pConstSrcBegin, pConstSrcEnd, bNulT) {
   }


private:

   /** Shares the source’s item array. It only allows sharing read-only or dynamically-allocated
   item arrays (the latter only as part of moving them).

   rtvi
      Source vextr.
   */
   void _assign_share(_raw_trivial_vextr_impl const & rtvi);


   /** Implementation of insert() and remove().

   ibOffset
      Byte index at which the items should be inserted or removed.
   pInsert
      Pointer to the first item to insert.
   cbInsert
      Size of the array pointed to be pInsert, in bytes.
   cbRemove
      Size of the slice of item array to remove, in bytes.
   */
   void _insert_or_remove(uintptr_t ibOffset, void const * pAdd, size_t cbAdd, size_t cbRemove);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

