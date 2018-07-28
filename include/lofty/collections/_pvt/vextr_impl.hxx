/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_COLLECTIONS__PVT_VEXTR_IMPL_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_COLLECTIONS__PVT_VEXTR_IMPL_HXX
#endif

#ifndef _LOFTY_COLLECTIONS__PVT_VEXTR_IMPL_HXX_NOPUB
#define _LOFTY_COLLECTIONS__PVT_VEXTR_IMPL_HXX_NOPUB

#include <lofty/memory.hxx>
#include <lofty/noncopyable.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

/*! Stores an item array and its capacity. Used as a real template by classes with embedded item array (see
lofty::text::sstr and lofty::collections::vector), and used with template capacity == 1 for all non-template-
driven manipulations in non-template code in the “lower-level” hierarchy, which relies on capacity instead. */
template <typename T, std::size_t embedded_capacity>
class vextr_prefixed_array {
public:
   //! Embedded item array capacity, in bytes.
   static std::size_t const embedded_byte_capacity = sizeof(T) * embedded_capacity;
   /*! Actual capacity of array, in bytes. This depends on the memory that was allocated for *this, so it can
   be greater than embedded_byte_capacity. */
   std::size_t capacity;
   /*! Fixed-size item array. This can’t be a T[] because we don’t want its items to be constructed/destructed
   automatically, and because the count may be greater than what’s declared here. */
   _std::max_align_t array[LOFTY_ALIGNED_SIZE(embedded_byte_capacity)];
};

}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

/*! Data members of vextr_impl_base, as a plain old struct. This is the most basic implementation block for
all lofty::text::str and lofty::collections::vector classes. */
struct vextr_impl_data {
   //! Pointer to the start of the item array.
   void * begin_ptr;
   //! Pointer to the end of the item array.
   void * end_ptr;
   //! true if *this includes an embedded prefixed item array.
   bool /*const*/ has_embedded_prefixed_array:1;
   //! true if the item array is part of a prefixed item array.
   bool array_is_prefixed:1;
   /*! true if the current item array is allocated dynamically, or false otherwise (embedded prefixed or non-
   prefixed). */
   bool dynamic:1;
   //! true if the item array is NUL-terminated.
   bool has_nul_term:1;
};

}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

/*! Template-independent members of *_vextr_impl that are identical for trivial and non-trivial types.

Lofty’s string and vector classes are intelligent wrappers around C arrays; they are able to dynamically
adjust the size of the underlying array, while also taking advantage of an optional fixed-size array embedded
into the string/vector object.

Since the implementation is shared between vectors and strings, the implementation class name is vextr, which
is a portmanteau of vector and str(ing).

Data-wise, vextr stores two pointers, one to the first item and one to beyond the last item (see
lofty::collections::_pvt::vextr_impl_data); this makes checking an iterator against the end of the array a
matter of a simple load/compare in terms of machine level instructions. The item array pointed to by the
begin/end pointers can be part of a prefixed item array (lofty::collections::_pvt::vextr_prefixed_array),
which includes information such as the total capacity of the item array, which is used to find out when the
item array needs to be reallocated to make room for more items.

All vextr* classes are non-template to avoid template bloat, though potentially at the expense of some
execution speed.

The class hierarchy of vextr/vector/str is composed as follows:

   @verbatim
                              ┌────────────────────────────────────┐
                              │ collections::_pvt::vextr_impl_data │
                              └────────────────────────────────────┘
                                                 △
                                                 │
                              ┌────────────────────────────────────┐
                              │ collections::_pvt::vextr_impl_base │
                              └────────────────────────────────────┘
                              △                                    △
                              │                                    │
       ┌───────────────────────────────────────┐ ┌───────────────────────────────────────┐
       │ collections::_pvt::trivial_vextr_impl │ │ collections::_pvt::complex_vextr_impl │
       └───────────────────────────────────────┘ └───────────────────────────────────────┘
         △                                △                                  △
         │                                │                                  │
   ┌─────────────────┐ ┌──────────────────────────────────┐ ┌──────────────────────────────────┐
   │  text::sstr<0>  │ │  collections::_pvt::vector_impl  │ │  collections::_pvt::vector_impl  │
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
•  lofty::collections::_pvt::vextr_impl_base – core functionality for a vector of elements: a little code and
   all member variables;
•  lofty::collections::_pvt::complex_vextr_impl – implementation of a vector of objects of non-trivial class:
   this is fully transactional and therefore exception-proof, but it’s of course slower and uses more memory
   even during simpler operations;
•  lofty::collections::_pvt::trivial_vextr_impl – implementation of a vector of plain values (instances of
   trivial class or native type): this is a near-optimal solution, still exception-proof but also taking
   advantage of the knowledge that no copy constructors need to be called.
•  lofty::collections::_pvt::vector_impl – abstracts away the differences between trivial and complex
   interfaces, exposing them as a single interface;
•  lofty::collections::vector – Lofty’s vector class;
•  lofty::text::str – Lofty’s string class.

See for lofty::text::str and lofty::collections::vector for examples of the memory layout of the above
classes. */
class LOFTY_SYM vextr_impl_base : public vextr_impl_data {
protected:
   // Allow transactions to access this class’s protected members.
   friend class vextr_transaction;

public:
   /*! Non-template prefixed item array used for the calculation of offsets that will then be applied to real
   instantiations of the prefixed item array template. */
   typedef vextr_prefixed_array<std::int8_t, 1> _prefixed_array;

public:
   //! Destructor.
   ~vextr_impl_base() {
      if (dynamic) {
         memory::_pub::free(prefixed_array());
      }
   }

   /*! Returns a pointer to the start of the item array.

   @return
      Pointer to the first item.
   */
   template <typename T>
   T * begin() {
      return static_cast<T *>(begin_ptr);
   }

   /*! Returns a const pointer to the start of the item array.

   @return
      Const pointer to the first item.
   */
   template <typename T>
   T const * begin() const {
      return static_cast<T const *>(begin_ptr);
   }

   /*! Returns the count of item slots in the current item array.

   @return
      Size of the item array.
   */
   template <typename T>
   std::size_t capacity() const {
      auto array(prefixed_array());
      return array ? array->capacity / sizeof(T) : 0;
   }

   /*! Returns a pointer to the end of the item array.

   @return
      Pointer to beyond the last item.
   */
   template <typename T>
   T * end() {
      return static_cast<T *>(end_ptr);
   }

   /*! Returns a const pointer to the end of the item array.

   @return
      Const pointer to beyond the last item.
   */
   template <typename T>
   T const * end() const {
      return static_cast<T const *>(end_ptr);
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
   /*! Internal constructor used by transaction. Access is private, but vextr_transaction can use it.

   Note that this doesn’t really initialize the object! */
   vextr_impl_base() {
      has_embedded_prefixed_array = false;
      /* This is needed to disable the destructor, so we won’t try to release an invalid pointer in case
      anything goes wrong before the rest of the object is initialized. */
      dynamic = false;
   }

protected:
   /*! Constructor. Constructs the object as empty, setting begin_ptr/End to nullptr.

   @param embedded_byte_capacity
      Size of the embedded prefixed item array, in bytes, or 0 if no embedded item array is present.
   */
   vextr_impl_base(std::size_t embedded_byte_capacity);

   /*! Constructor. Assigns the object an item array.

   @param embedded_byte_capacity
      Size of the embedded prefixed item array, in bytes, or 0 if no embedded item array is present.
   @param const_src_begin
      Pointer to the start of an array that will be adopted by the vextr as read-only.
   @param const_src_end
      Pointer to the end of the array.
   @param has_nul_term
      true if the array pointed to by const_src_begin is a NUL-terminated string, or false otherwise.
   */
   vextr_impl_base(
      std::size_t embedded_byte_capacity, void const * const_src_begin, void const * const_src_end,
      bool has_nul_term = false
   );

   //! Resets the contents of the object to nullptr.
   void assign_empty() {
      begin_ptr = nullptr;
      end_ptr = nullptr;
      array_is_prefixed = false;
      dynamic = false;
      has_nul_term = false;
   }

   /*! Copies the data members of the source to *this.

   @param src
      Source vextr.
   */
   void assign_shallow(vextr_impl_base const & src) {
      begin_ptr         = src.begin_ptr;
      end_ptr           = src.end_ptr;
      array_is_prefixed = src.array_is_prefixed;
      dynamic           = src.dynamic;
      has_nul_term      = src.has_nul_term;
   }

   /*! Calculates the new capacity for the item array for growing from old_size to new_size bytes while
   attempting to reduce future allocations for subsequent size increases.

   @param old_size
      Previous (current) item array size, in bytes.
   @param new_size
      New (future) item array size, in bytes.
   @return
      New item array capacity, in bytes.
   */
   static std::size_t calculate_increased_capacity(std::size_t old_size, std::size_t new_size);

   /*! Returns a pointer to the current prefixed item array, or nullptr if the current item array is not
   prefixed.

   @return
      Pointer to the prefixed item array, or nullptr if not applicable.
   */
   _prefixed_array * prefixed_array() {
      if (array_is_prefixed) {
         // Subtract from begin_ptr the offset of the item array.
         return reinterpret_cast<_prefixed_array *>(
            begin<std::int8_t>() - LOFTY_OFFSETOF(_prefixed_array, array)
         );
      } else {
         return nullptr;
      }
   }

   /*! Returns a const pointer to the current prefixed item array, or nullptr if the current item array is not
   prefixed.

   @return
      Const pointer to the prefixed item array, or nullptr if not applicable.
   */
   _prefixed_array const * prefixed_array() const {
      return const_cast<vextr_impl_base *>(this)->prefixed_array();
   }

   /*! Returns a pointer to the embedded prefixed item array that follows this object, if present.

   @return
      Pointer to the embedded item array, or nullptr otherwise.
   */
   _prefixed_array * embedded_prefixed_array() {
      if (has_embedded_prefixed_array) {
         /* Allows to obtain the pointer to an embedded prefixed item array in non-template code without
         resorting to manual pointer arithmetics. */
#if LOFTY_HOST_CXX_MSC
   #pragma warning(push)
   /* “'class' : default constructor could not be generated because a base class default constructor
   is inaccessible” */
   #pragma warning(disable: 4623)
#endif
         class vextr_impl_base_with_embedded_prefixed_array :
            public vextr_impl_base,
            public vextr_impl_base::_prefixed_array {
         };
#if LOFTY_HOST_CXX_MSC
   #pragma warning(pop)
#endif

         return static_cast<vextr_impl_base_with_embedded_prefixed_array *>(this);
      } else {
         return nullptr;
      }
   }

   /*! Throws a collections::out_of_range if a pointer is not within bounds.

   @param p
      Pointer to validate.
   @param allow_end
      If true, p == end_ptr is allowed; if false, it’s not.
   */
   void validate_pointer(void const * p, bool allow_end) const;

   /*! Throws a collections::out_of_range if a pointer is not within bounds of *this_ptr.

   This overload is static so that it will validate that this (this_ptr) is not nullptr before dereferencing
   it.

   @param this_ptr
      this.
   @param p
      Pointer to validate.
   @param allow_end
      If true, p == this_ptr->end_ptr is allowed; if false, it’s not.
   */
   static void validate_pointer(vextr_impl_base const * this_ptr, void const * p, bool allow_end);

protected:
   //! The item array size must be no less than this many bytes.
   static std::size_t const capacity_bytes_min = sizeof(std::ptrdiff_t) * 8;
   /*! Size multiplier. This should take into account that we want to reallocate as rarely as possible, so
   every time we do it it should be for a rather conspicuous growth. */
   static unsigned const growth_rate = 2;
};

}}} //namespace lofty::collections::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

/*! Allows to get a temporary item array from a pool of options, then work with it, and upon destruction it
ensures that the array is either adopted by the associated _pvt::vextr_impl_base, or properly discarded.

A transaction will not take care of copying the item array, if switching to a different item array.

For size increases, the reallocation (if any) is performed in the constructor; for decreases, it’s performed
in commit(). */
class LOFTY_SYM vextr_transaction : public lofty::_LOFTY_PUBNS noncopyable {
public:
   /*! Constructor.

   @param target
      Subject of the transaction.
   @param trivial
      If true, the items are of a trivial type; if false, they’re not.
   @param new_size
      New item array size, in bytes.
   */
   vextr_transaction(vextr_impl_base * target, bool trivial, std::size_t new_size);

   /*! Constructor.

   @param target
      Subject of the transaction.
   @param trivial
      If true, the items are of a trivial type; if false, they’re not.
   @param insert_size
      Item array size increase, in bytes.
   @param remove_size
      Item array size decrease, in bytes.
   */
   vextr_transaction(
      vextr_impl_base * target, bool trivial, std::size_t insert_size, std::size_t remove_size
   );

   //! Destructor.
   ~vextr_transaction() {
      /* Only allow work_copy to release its item array if we allocated it for the transaction and
      commit() was never called. */
      work_copy.dynamic = work_copy_array_needs_free;
   }

   /*! Commits the transaction; if the item array is to be replaced, the current one will be released if
   necessary; it’s up to the client to destruct any items in it. If this method is not called before the
   transaction is destructed, it’s up to the client to also ensure that any and all objects constructed in the
   work array have been properly destructed. */
   void commit();

   /*! Returns the work item array.

   @return
      Pointer to the working item array.
   */
   template <typename T>
   T * work_array() const {
      return static_cast<T *>(work_copy.begin_ptr);
   }

   /*! Returns true if the contents of the item array need to migrated due to the transaction switching item
   arrays. If the array was/will be only resized, the return value is false, because the reallocation did/will
   take care of moving the item array.

   @return
      true if the pointer to the item array will be changed upon destruction, or false otherwise.
   */
   bool will_replace_array() const {
      return work_copy.begin_ptr != target->begin_ptr;
   }

private:
   /*! Completes construction of the object.

   @param trivial
      If true, the items are of a trivial type; if false, they’re not.
   @param new_size
      New item array size, in bytes.
   */
   void _construct(bool trivial, std::size_t new_size);

private:
   /*! Temporary vextr that contains the new values for each vextr member, ready to be applied to *target when
   the transaction is committed. Its internal pointers may or may not be the same as the ones in target
   depending on whether we needed a new item array. */
   vextr_impl_base work_copy;
   //! Subject of the transaction.
   vextr_impl_base * target;
   /*! true if work_copy references an item array that has been dynamically allocated for the transaction and
   needs to be freed in the destructor, which can happen when an exception occurs before the transaction is
   committed. */
   bool work_copy_array_needs_free;
};

}}} //namespace lofty::collections::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

/*! Template-independent implementation of a vector for trivial contained types. This is the most derived
common base class of both vector and str. */
class LOFTY_SYM trivial_vextr_impl : public vextr_impl_base {
public:
   /*! Copies the contents of the two sources to *this. This method must not be called with src1 or
   src2 == begin_ptr.

   @param src1_begin
      Pointer to the start of the first source array.
   @param src1_end
      Pointer to the end of the first source array.
   @param src2_begin
      Pointer to the start of the second source array.
   @param src2_end
      Pointer to the end of the second source array.
   */
   void assign_concat(
      void const * src1_begin, void const * src1_end, void const * src2_begin, void const * src2_end
   );

   /*! Copies the contents of the source array to *this.

   @param src_begin
      Pointer to the start of the source array.
   @param src_end
      Pointer to the end of the source array.
   */
   void assign_copy(void const * src_begin, void const * src_end) {
      /* Allow to continue with src_begin == begin_ptr if using a non-prefixed (read-only) item array; this
      allows to switch to using a prefixed (writable) item array. */
      if (src_begin == begin_ptr && array_is_prefixed) {
         return;
      }
      /* assign_concat() is fast enough. Pass the source as the second argument pair, because its code path is
      faster. */
      assign_concat(nullptr, nullptr, src_begin, src_end);
   }

   /*! Moves the source’s item array if dynamically-allocated or not prefixed, else copies its items (not move
   – items are trivial) to *this.

   @param src
      Source vextr.
   */
   void assign_move_desc_or_move_items(trivial_vextr_impl && src);

   /*! Shares the source’s item array if not prefixed, otherwise it creates a copy of the source prefixed item
   array for *this.

   @param src
      Source vextr.
   */
   void assign_share_raw_or_copy_desc(trivial_vextr_impl const & src);

   /*! Inserts or removes items at a specific position in the vextr.

   @param offset
      Byte index at which the items should be inserted or removed.
   @param insert_src
      Pointer to the first item to insert.
   @param insert_size
      Size of the array pointed to be insert_src, in bytes.
   @param remove_size
      Size of the slice of item array to remove, in bytes.
   */
   void insert_remove(
      std::size_t offset, void const * insert_src, std::size_t insert_size, std::size_t remove_size
   ) {
      if (insert_size != remove_size) {
         _insert_remove(offset, insert_src, insert_size, remove_size);
      }
   }

   /*! Ensures that the item array has at least new_capacity_min of actual item space. If this causes *this to
   switch to using a different item array, any data in the current one will be lost unless preserve == true.

   @param new_capacity_min
      Minimum size of items requested, in bytes.
   @param preserve
      If true, the previous contents of the item array will be preserved even if the reallocation causes the
      vextr to switch to a different item array.
   */
   void set_capacity(std::size_t new_capacity_min, bool preserve);

   /*! Changes the count of items in the vextr. If the item array needs to be lengthened, the added items will
   be left uninitialized.

   @param new_size
      New size of the items, in bytes.
   */
   void set_size(std::size_t new_size);

protected:
   //! See vextr_impl_base::vextr_impl_base().
   trivial_vextr_impl(std::size_t embedded_byte_capacity) :
      vextr_impl_base(embedded_byte_capacity) {
   }

   //! See vextr_impl_base::vextr_impl_base().
   trivial_vextr_impl(
      std::size_t embedded_byte_capacity, void const * const_src_begin, void const * const_src_end,
      bool has_nul_term_ = false
   ) :
      vextr_impl_base(embedded_byte_capacity, const_src_begin, const_src_end, has_nul_term_) {
   }

private:
   //! Implementation of insert_remove(). See insert_remove().
   void _insert_remove(
      std::size_t offset, void const * insert_src, std::size_t insert_size, std::size_t remove_size
   );
};

}}} //namespace lofty::collections::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_COLLECTIONS__PVT_VEXTR_IMPL_HXX_NOPUB

#ifdef _LOFTY_COLLECTIONS__PVT_VEXTR_IMPL_HXX
   #undef _LOFTY_NOPUB

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_COLLECTIONS__PVT_VEXTR_IMPL_HXX
