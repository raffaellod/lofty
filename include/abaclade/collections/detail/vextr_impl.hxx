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

/*! @page vextr-design High-efficiency strings and vectors
Design of abc::text::str and abc::collections::vector.

Abaclade’s string and vector classes are intelligent wrappers around C arrays; they are able to
dynamically adjust the size of the underlying array, while also taking advantage of an optional
fixed-size array embedded into the string/vector object.

Since the implementation is shared between vectors and strings, the implementation class name is
vextr, which is a portmanteau of vector and str(ing).

Data-wise, vextr stores two pointers, one to the first item and one to beyond the last item
(see abc::collections::detail::vextr_impl_data); this makes checking an iterator against the end of
the array a matter of a simple load/compare in terms of machine level instructions. The item array
pointed to by the begin/end pointers can be part of a prefixed item array
(abc::collections::detail::vextr_prefixed_item_array), which includes information such as the total
capacity of the item array, which is used to find out when the item array needs to be reallocated to
make room for more items.

All vextr* classes are non-template to avoid template bloat, though potentially at the expense of
some execution speed.

The class hierarchy of vextr/vector/str is composed as follows:

   @verbatim
                             ┌──────────────────────────────────────┐
                             │ collections::detail::vextr_impl_base │
                             └──────────────────────────────────────┘
                              △                                    △
                              │                                    │
      ┌─────────────────────────────────────────┐ ┌─────────────────────────────────────────┐
      │ collections::detail::trivial_vextr_impl │ │ collections::detail::complex_vextr_impl │
      └─────────────────────────────────────────┘ └─────────────────────────────────────────┘
         △                                △                                  △
         │                                │                                  │
   ┌─────────────────┐ ┌──────────────────────────────────┐ ┌──────────────────────────────────┐
   │  text::sstr<0>  │ │ collections::detail::vector_impl │ │ collections::detail::vector_impl │
   │ (aka text::str) │ │ (trivial partial specialization) │ │ (complex partial specialization) │
   └─────────────────┘ └──────────────────────────────────┘ └──────────────────────────────────┘
            △                                  △                      △
            │                                  │                      │
    ┌───────────────┐                         ┌────────────────────────┐
    │ text::sstr<n> │                         │ collections::vector<0> │
    └───────────────┘                         └────────────────────────┘
                                                           △
                                                           │
                                              ┌────────────────────────┐
                                              │ collections::vector<n> │
                                              └────────────────────────┘
   @endverbatim

Here’s an overview of each class:
•  abc::collections::detail::vextr_impl_base – core functionality for a vector of elements: a little
   code and all member variables;
•  abc::collections::detail::complex_vextr_impl – implementation of a vector of objects of non-
   trivial class: this is fully transactional and therefore exception-proof, but it’s of course
   slower and uses more memory even during simpler operations;
•  abc::collections::detail::trivial_vextr_impl – implementation of a vector of plain values
   (instances of trivial class or native type): this is a near-optimal solution, still exception-
   proof but also taking advantage of the knowledge that no copy constructors need to be called.
•  abc::collections::detail::vector_impl – abstracts away the differences between trivial and
   complex interfaces, exposing them as a single interface;
•  abc::collections::vector – Abaclade’s vector class;
•  abc::text::str – Abaclade’s string class.

Let’s look at the underlying data storage in different scenarios. Key:
   @verbatim
   ┌──────────────┬──────────┬───────────┬───────────────┬────────────────┬─────────────────┐
   │ Pointer to   │ Pointer  │ P if elem │ T if element  │ E is vextr has │ D if elem array │
   │ beginning of │ to end   │ array is  │ array is NUL- │ embedded       │ is dynamically- │
   │ array        │ of array │ prefixed  │ terminated    │ prefixed array │ allocated       │
   └──────────────┴──────────┴───────────┴───────────────┴────────────────┴─────────────────┘
   @endverbatim

Additionally, an embedded element array can follow, prefixed by its length (here shown as an element
count, but in the real implementation it’s actually a byte count).


1. str s1(): no element array.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┬───┐
   │ nullptr │ nullptr │ - │ - │ - │ - │
   └─────────┴─────────┴───┴───┴───┴───┘
   @endverbatim

2. sstr<4> s2(): has an embedded prefixed fixed-size array, but does not use it yet.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┬───╥───┬─────────┐
   │ nullptr │ nullptr │ - │ - │ E │ - ║ 4 │ - - - - │
   └─────────┴─────────┴───┴───┴───┴───╨───┴─────────┘
   @endverbatim

3. str s3("abc"): points to a non-prefixed element array in read-only memory, which also has a NUL
   terminator.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┬───┐                    ┌──────────┐
   │ 0xptr   │ 0xptr   │ - │ T │ - │ - │                    │ a b c \0 │
   └─────────┴─────────┴───┴───┴───┴───┘                    └──────────┘
     │         │                                            ▲       ▲
     │         └────────────────────────────────────────────│───────┘
     └──────────────────────────────────────────────────────┘
   @endverbatim

4. s3 += "def": switches to a dynamically-allocated prefixed array to perform the concatenation.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┬───┐                ┌───┬─────────────────┐
   │ 0xptr   │ 0xptr   │ P │ - │ - │ D │                │ 8 │ a b c d e f - - │
   └─────────┴─────────┴───┴───┴───┴───┘                └───┴─────────────────┘
     │         │                                            ▲             ▲
     │         └────────────────────────────────────────────│─────────────┘
     └──────────────────────────────────────────────────────┘
   @endverbatim

5. s2 += "abc": starts using the embedded prefixed fixed-size array to perform the concatenation.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┬───╥───┬─────────┐
   │ 0xptr   │ 0xptr   │ P │ - │ E │ - ║ 4 │ a b c - │
   └─────────┴─────────┴───┴───┴───┴───╨───┴─────────┘
     │         │                           ▲       ▲
     │         └───────────────────────────│───────┘
     └─────────────────────────────────────┘
   @endverbatim

7. s2 += "def": switches to a dynamically-allocated prefixed array because the embedded one is not
   large enough.
   @verbatim
   ┌─────────┬─────────┬───┬───┬───┬───╥───┬─────────┐  ┌───┬─────────────────┐
   │ 0xptr   │ 0xptr   │ P │ - │ E │ D ║ 4 │ - - - - │  │ 8 │ a b c d e f - - │
   └─────────┴─────────┴───┴───┴───┴───╨───┴─────────┘  └───┴─────────────────┘
     │         │                                            ▲             ▲
     │         └────────────────────────────────────────────│─────────────┘
     └──────────────────────────────────────────────────────┘
   @endverbatim
*/


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

/*! Stores an item array and its capacity. Used as a real template by classes with embedded item
array in the “upper level” hierarchy (see @ref vextr-design), and used with template capacity == 1
for all non-template-driven manipulations in non-template code in the “lower-level” hierarchy, which
relies on m_cbCapacity instead. */
template <typename T, std::size_t t_ciEmbeddedCapacity>
class vextr_prefixed_item_array {
public:
   //! Embedded item array capacity, in bytes.
   static std::size_t const smc_cbEmbeddedCapacity = sizeof(T) * t_ciEmbeddedCapacity;
   /*! Actual capacity of m_at, in bytes. This depends on the memory that was allocated for *this,
   so it can be greater than smc_cbEmbeddedCapacity. */
   std::size_t m_cbCapacity;
   /*! Fixed-size item array. This can’t be a T[] because we don’t want its items to be constructed/
   destructed automatically, and because the count may be greater than what’s declared here. */
   _std::max_align_t m_at[ABC_ALIGNED_SIZE(smc_cbEmbeddedCapacity)];
};

}}} //namespace abc::collections::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

/*! Data members of vextr_impl_base, as a plain old struct. This is the most basic implementation
block for all abc::text::str and abc::collections::vector classes. */
struct vextr_impl_data {
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

}}} //namespace abc::collections::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

/*! Template-independent members of detail::*_vextr_impl that are identical for trivial and non-
trivial types. */
class ABACLADE_SYM vextr_impl_base : public vextr_impl_data {
protected:
   // Allow transactions to access this class’s protected members.
   friend class vextr_transaction;

public:
   /*! Non-template prefixed item array used for the calculation of offsets that will then be
   applied to real instantiations of the prefixed item array template. */
   typedef vextr_prefixed_item_array<std::int8_t, 1> _prefixed_item_array;

public:
   //! Destructor.
   ~vextr_impl_base() {
      if (m_bDynamic) {
         memory::free(prefixed_item_array());
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
   /*! Internal constructor used by transaction. Access is private, but vextr_transaction can use
   it.

   Note that this doesn’t really initialize the object!
   */
   vextr_impl_base() {
      mc_bEmbeddedPrefixedItemArray = false;
      /* This is needed to disable the destructor, so we won’t try to release an invalid pointer in
      case anything goes wrong before the rest of the object is initialized. */
      m_bDynamic = false;
   }

protected:
   /*! Constructor. Constructs the object as empty, setting m_pBegin/End to nullptr.

   @param cbEmbeddedCapacity
      Size of the embedded prefixed item array, in bytes, or 0 if no embedded item array is present.
   */
   vextr_impl_base(std::size_t cbEmbeddedCapacity);

   /*! Constructor. Assigns the object an item array.

   @param cbEmbeddedCapacity
      Size of the embedded prefixed item array, in bytes, or 0 if no embedded item array is present.
   @param pConstSrcBegin
      Pointer to the start of an array that will be adopted by the vextr as read-only.
   @param pConstSrcEnd
      Pointer to the end of the array.
   @param bNulT
      true if the array pointed to by pConstSrc is a NUL-terminated string, or false otherwise.
   */
   vextr_impl_base(
      std::size_t cbEmbeddedCapacity, void const * pConstSrcBegin, void const * pConstSrcEnd,
      bool bNulT = false
   );

   //! Resets the contents of the object to nullptr.
   void assign_empty() {
      m_pBegin = nullptr;
      m_pEnd = nullptr;
      m_bPrefixedItemArray = false;
      m_bDynamic = false;
      m_bNulT = false;
   }

   /*! Copies the data members of the source to *this.

   @param vib
      Source vextr.
   */
   void assign_shallow(vextr_impl_base const & vib) {
      m_pBegin             = vib.m_pBegin;
      m_pEnd               = vib.m_pEnd;
      m_bPrefixedItemArray = vib.m_bPrefixedItemArray;
      m_bDynamic           = vib.m_bDynamic;
      m_bNulT              = vib.m_bNulT;
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
            begin<std::int8_t>() - ABC_OFFSETOF(_prefixed_item_array, m_at)
         );
      } else {
         return nullptr;
      }
   }

   /*! Returns a const pointer to the current prefixed item array, or nullptr if the current item
   array is not prefixed.

   @return
      Const pointer to the prefixed item array, or nullptr if not applicable.
   */
   _prefixed_item_array const * prefixed_item_array() const {
      return const_cast<vextr_impl_base *>(this)->prefixed_item_array();
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
         class vextr_impl_base_with_embedded_prefixed_item_array :
            public vextr_impl_base,
            public vextr_impl_base::_prefixed_item_array {
         };
#if ABC_HOST_CXX_MSC
   #pragma warning(pop)
#endif

         return static_cast<vextr_impl_base_with_embedded_prefixed_item_array *>(this);
      } else {
         return nullptr;
      }
   }

   /*! Throws a collections::out_of_range if a pointer is not within bounds.

   @param p
      Pointer to validate.
   @param bAllowEnd
      If true, p == m_pEnd is allowed; if false, it’s not.
   */
   void validate_pointer(void const * p, bool bAllowEnd) const;

   /*! Throws a collections::out_of_range if a pointer is not within bounds of *pvib.

   This overload is static so that it will validate that this (pvib) is not nullptr before
   dereferencing it.

   @param pvib
      this.
   @param p
      Pointer to validate.
   @param bAllowEnd
      If true, p == pvib->m_pEnd is allowed; if false, it’s not.
   */
   static void validate_pointer(vextr_impl_base const * pvib, void const * p, bool bAllowEnd);

protected:
   //! The item array size must be no less than this many bytes.
   static std::size_t const smc_cbCapacityMin = sizeof(std::ptrdiff_t) * 8;
   /*! Size multiplier. This should take into account that we want to reallocate as rarely as
   possible, so every time we do it it should be for a rather conspicuous growth. */
   static unsigned const smc_iGrowthRate = 2;
};

}}} //namespace abc::collections::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

/*! Allows to get a temporary item array from a pool of options, then work with it, and upon
destruction it ensures that the array is either adopted by the associated detail::vextr_impl_base,
or properly discarded.

A transaction will not take care of copying the item array, if switching to a different item array.

For size increases, the reallocation (if any) is performed in the constructor; for decreases, it’s
performed in commit(). */
class ABACLADE_SYM vextr_transaction : public noncopyable {
public:
   /*! Constructor.

   @param pvib
      Subject of the transaction.
   @param bTrivial
      If true, the items are of a trivial type; if false, they’re not.
   @param cbNew
      New item array size, in bytes.
   */
   vextr_transaction(vextr_impl_base * pvib, bool bTrivial, std::size_t cbNew);

   /*! Constructor.

   @param pvib
      Subject of the transaction.
   @param bTrivial
      If true, the items are of a trivial type; if false, they’re not.
   @param cbAdd
      Item array size increase, in bytes.
   @param cbRemove
      Item array size decrease, in bytes.
   */
   vextr_transaction(
      vextr_impl_base * pvib, bool bTrivial, std::size_t cbAdd, std::size_t cbRemove
   );

   //! Destructor.
   ~vextr_transaction() {
      /* Only allow m_vibWork to release its item array if we allocated it for the transaction and
      commit() was never called. */
      m_vibWork.m_bDynamic = m_bFree;
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
      return static_cast<T *>(m_vibWork.m_pBegin);
   }

   /*! Returns true if the contents of the item array need to migrated due to the transaction
   switching item arrays. If the array was/will be only resized, the return value is false, because
   the reallocation did/will take care of moving the item array.

   @return
      true if the pointer to the item array will be changed upon destruction, or false otherwise.
   */
   bool will_replace_item_array() const {
      return m_vibWork.m_pBegin != m_pvib->m_pBegin;
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
   *m_pvib when the transaction is committed. Its internal pointers may or may not be the same as
   the ones in m_pvib depending on whether we needed a new item array. */
   vextr_impl_base m_vibWork;
   //! Subject of the transaction.
   vextr_impl_base * m_pvib;
   /*! true if m_vibWork references an item array that has been dynamically allocated for the
   transaction and needs to be freed in the destructor, which can happen when an exception occurs
   before the transaction is committed. */
   bool m_bFree;
};

}}} //namespace abc::collections::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

/*! Template-independent implementation of a vector for trivial contained types. This is the most
derived common base class of both vector and str. */
class ABACLADE_SYM trivial_vextr_impl : public vextr_impl_base {
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
      /* Allow to continue with pBegin == m_pBegin if using a non-prefixed (read-only) item array;
      this allows to switch to using a prefixed (writable) item array. */
      if (pBegin == m_pBegin && m_bPrefixedItemArray) {
         return;
      }
      /* assign_concat() is fast enough. Pass the source as the second argument pair, because its
      code path is faster. */
      assign_concat(nullptr, nullptr, pBegin, pEnd);
   }

   /*! Moves the source’s item array if dynamically-allocated or not prefixed, else copies its items
   (not move – items are trivial) to *this.

   @param rtvi
      Source vextr.
   */
   void assign_move_desc_or_move_items(trivial_vextr_impl && rtvi);

   /*! Shares the source’s item array if not prefixed, otherwise it creates a copy of the source
   prefixed item array for *this.

   @param rtvi
      Source vextr.
   */
   void assign_share_raw_or_copy_desc(trivial_vextr_impl const & rtvi);

   /*! Inserts or removes items at a specific position in the vextr.

   @param ibOffset
      Byte index at which the items should be inserted or removed.
   @param pAdd
      Pointer to the first item to insert.
   @param cbAdd
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
   //! See vextr_impl_base::vextr_impl_base().
   trivial_vextr_impl(std::size_t cbEmbeddedCapacity) :
      vextr_impl_base(cbEmbeddedCapacity) {
   }

   //! See vextr_impl_base::vextr_impl_base().
   trivial_vextr_impl(
      std::size_t cbEmbeddedCapacity, void const * pConstSrcBegin, void const * pConstSrcEnd,
      bool bNulT = false
   ) :
      vextr_impl_base(cbEmbeddedCapacity, pConstSrcBegin, pConstSrcEnd, bNulT) {
   }

private:
   //! Implementation of insert_remove(). See insert_remove().
   void _insert_remove(
      std::size_t ibOffset, void const * pAdd, std::size_t cbAdd, std::size_t cbRemove
   );
};

}}} //namespace abc::collections::detail
