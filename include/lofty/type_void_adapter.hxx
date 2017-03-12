/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015, 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_TYPE_VOID_ADAPTER_HXX
#define _LOFTY_TYPE_VOID_ADAPTER_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Encapsulates raw constructors, destructors and assignment operators for a type.
// TODO: document rationale, design and use cases.
class LOFTY_SYM type_void_adapter {
private:
   //! Prototype of a function that copy-constructs elements from one array to another.
   typedef void (* copy_construct_impl_type)(void *, void const *, void const *);
   //! Prototype of a function that destructs a range of elements in an array.
   typedef void (* destruct_impl_type)(void const *, void const *);
   //! Prototype of a function that move-constructs elements from one array to another.
   typedef void (* move_construct_impl_type)(void *, void *, void *);

public:
   /*! Returns the alignment of a variable of this type.

   @return
      Alignment in bytes.
   */
   std::size_t alignment() const {
      return alignment_;
   }

   /*! Adjusts (increases) an offset as needed by the type’s alignment requirements.

   @param offset
      Byte offset to align.
   @return
      Aligned offset.
   */
   std::uintptr_t align_offset(std::uintptr_t offset) const {
      // TODO: deduplicate this copy of bitmanip::ceiling_to_pow2_multiple().
      std::uintptr_t step = static_cast<std::uintptr_t>(alignment_ - 1);
      return (offset + step) & ~step;
   }

   /*! Adjusts (increases) a pointer as needed by the type’s alignment requirements.

   @param p
      Pointer to align.
   @return
      Aligned pointer.
   */
   void * align_pointer(void const * p) const {
      return reinterpret_cast<void *>(align_offset(reinterpret_cast<std::uintptr_t>(p)));
   }

   /*! Copy-constructs an object from one memory location to another.

   @param dst
      Pointer to the destination memory area. The memory is supposed to be uninitialized.
   @param src
      Pointer to the object to copy.
   */
   void copy_construct(void * dst, void const * src) const {
      copy_construct(dst, src, static_cast<std::int8_t const *>(src) + size_);
   }

   /*! Copy-constructs elements from an array to another.

   @param dst_begin
      Pointer to the start of the destination array. The array is supposed to be uninitialized.
   @param src_begin
      Pointer to the first element to copy.
   @param src_end
      Pointer to the end of the source array.
   */
   void copy_construct(void * dst_begin, void const * src_begin, void const * src_end) const {
      copy_construct_fn(dst_begin, src_begin, src_end);
   }

   /*! Destructs an object.

   @param p
      Pointer to the object to destruct.
   */
   void destruct(void const * p) const {
      destruct(p, static_cast<std::int8_t const *>(p) + size_);
   }

   /*! Destructs a range of elements in an array.

   @param begin
      Pointer to the first element to destruct.
   @param end
      Pointer to beyond the last element to destruct.
   */
   void destruct(void const * begin, void const * end) const {
      destruct_fn(begin, end);
   }

   /*! Move-constructs an object from one memory location to another.

   @param dst
      Pointer to the destination memory area. The memory is supposed to be uninitialized.
   @param src
      Pointer to the object to move.
   */
   void move_construct(void * dst, void * src) const {
      move_construct(dst, src, static_cast<std::int8_t *>(src) + size_);
   }

   /*! Move-constructs elements from an array to another.

   @param dst_begin
      Pointer to the start of the destination array. The array is supposed to be uninitialized.
   @param src_begin
      Pointer to the first element to move.
   @param src_end
      Pointer to the end of the source array.
   */
   void move_construct(void * dst_begin, void * src_begin, void * src_end) const {
      move_construct_fn(dst_begin, src_begin, src_end);
   }

   //! Makes alignment() and align_pointer() available.
   template <typename T>
   void set_align() {
      alignment_ = static_cast<std::uint16_t>(alignof(T));
   }

   //! Makes copy_construct() available (trivial copy case).
   template <typename T>
   void set_copy_construct(typename _std::enable_if<
      _std::is_trivially_copy_constructible<T>::value, T *
   >::type = nullptr) {
      set_size<T>();
      copy_construct_fn = reinterpret_cast<copy_construct_impl_type>(&copy_construct_trivial_impl);
   }

   //! Makes copy_construct() available (non-trivial copy case).
   template <typename T>
   void set_copy_construct(typename _std::enable_if<
      !_std::is_trivially_copy_constructible<T>::value, T *
   >::type = nullptr) {
      set_size<T>();
      copy_construct_fn = reinterpret_cast<copy_construct_impl_type>(
         &copy_construct_impl<typename _std::remove_cv<T>::type>
      );
   }

   //! Makes destruct() available (trivial copy case).
   template <typename T>
   void set_destruct(typename _std::enable_if<
      _std::is_trivially_destructible<T>::value, T *
   >::type = nullptr) {
      set_size<T>();
      destruct_fn = reinterpret_cast<destruct_impl_type>(&destruct_trivial_impl);
   }

   //! Makes destruct() available (non-trivial copy case).
   template <typename T>
   void set_destruct(typename _std::enable_if<
      !_std::is_trivially_destructible<T>::value, T *
   >::type = nullptr) {
      set_size<T>();
      destruct_fn = reinterpret_cast<destruct_impl_type>(&destruct_impl<typename _std::remove_cv<T>::type>);
   }

   //! Makes move_construct() available (trivial copy case).
   template <typename T>
   void set_move_construct(typename _std::enable_if<
      _std::is_trivially_move_constructible<T>::value, T *
   >::type = nullptr) {
      set_size<T>();
      // A trivial copy move works just fine for a trivial move.
      move_construct_fn = reinterpret_cast<move_construct_impl_type>(&copy_construct_trivial_impl);
   }

   //! Makes move_construct() available (non-trivial copy case).
   template <typename T>
   void set_move_construct(typename _std::enable_if<
      !_std::is_trivially_move_constructible<T>::value, T *
   >::type = nullptr) {
      set_size<T>();
      move_construct_fn = reinterpret_cast<move_construct_impl_type>(
         &move_construct_impl<typename _std::remove_cv<T>::type>
      );
   }

   //! Makes size() available.
   template <typename T>
   void set_size() {
      size_ = static_cast<std::uint16_t>(sizeof(T));
   }

   /*! Returns the size of an object of this type, in bytes.

   @return
      Size of an object, in bytes.
   */
   std::size_t size() const {
      return size_;
   }

private:
   /*! Copies a range of elements from one array to another, overwriting any existing contents in the
   destination.

   @param dst_begin
      Pointer to the start of the destination array. The elements are supposed to be uninitialized.
   @param src_begin
      Pointer to the first element to copy.
   @param src_end
      Pointer to beyond the last element to copy.
   */
   template <typename T>
   static void copy_construct_impl(T * dst_begin, T const * src_begin, T const * src_end) {
      // Assume that since it’s not trivial, it can throw exceptions, so perform a transactional copy.
      T * dst = dst_begin;
      try {
         for (T const * src = src_begin; src < src_end; ++src, ++dst) {
            ::new(dst) T(*src);
         }
      } catch (...) {
         // Undo (destruct) all the copies instantiated.
         while (--dst >= dst_begin) {
            dst->~T();
         }
         throw;
      }
   }

   /*! Copies memory from one array to another. Replaces copy_construct_impl() and/or move_construct_impl() in
   case of trivial copy constructor and/or move constructor.

   @param dst_bytes_begin
      Pointer to the start of the destination array.
   @param src_bytes_begin
      Pointer to the first byte to copy.
   @param src_bytes_end
      Pointer to beyond the last byte to copy.
   */
   static void copy_construct_trivial_impl(
      std::int8_t * dst_bytes_begin, std::int8_t * src_bytes_begin, std::int8_t * src_bytes_end
   );

   /*! Destructs a range of elements in an array.

   @param begin
      Pointer to the first element to destruct.
   @param end
      Pointer to beyond the last element to destruct.
   */
   template <typename T>
   static void destruct_impl(T const * begin, T const * end) {
      for (auto t = begin; t < end; ++t) {
         t->~T();
      }
   }

   /*! No-op to replace destruct_impl() in case of trivial destructor.

   @param begin
      Pointer to the start of the memory that would be destructed.
   @param end
      Pointer to the end of the memory that would be destructed.
   */
   static void destruct_trivial_impl(void const * begin, void const * end);

   /*! Moves a range of elements from one array to another, overwriting any existing contents in the
   destination.

   @param dst_begin
      Pointer to the start of the destination array. The elements are supposed to be uninitialized.
   @param src_begin
      Pointer to the first element to copy.
   @param src_end
      Pointer to beyond the last element to copy.
   */
   template <typename T>
   static void move_construct_impl(T * dst_begin, T * src_begin, T * src_end) {
      for (T * src = src_begin, * dst = dst_begin; src < src_end; ++src, ++dst) {
         ::new(dst) T(_std::move(*src));
      }
   }

private:
   /*! Size of a variable of this type, in bytes. First member because it’s the most frequently used, and
   having it at offset 0 may lead to faster or more compact code. */
   std::uint16_t size_;
   //! Alignment of a variable of this type, in bytes.
   std::uint16_t alignment_;
   //! Pointer to a function to copy elements from one array to another.
   copy_construct_impl_type copy_construct_fn;
   //! Pointer to a function to destruct elements in an array.
   destruct_impl_type destruct_fn;
   //! Pointer to a function to move elements from one array to another.
   move_construct_impl_type move_construct_fn;
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TYPE_VOID_ADAPTER_HXX
