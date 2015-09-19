/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABACLADE_TYPE_VOID_ADAPTER_HXX
#define _ABACLADE_TYPE_VOID_ADAPTER_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! Encapsulates raw constructors, destructors and assignment operators for a type.
// TODO: document rationale, design and use cases.
class ABACLADE_SYM type_void_adapter {
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
      return m_cbAlign;
   }

   /*! Adjusts (increases) an offset as needed by the type’s alignment requirements.

   @param ibOffset
      Byte offset to align.
   @return
      Aligned offset.
   */
   std::uintptr_t align_offset(std::uintptr_t ibOffset) const {
      // TODO: deduplicate this copy of bitmanip::ceiling_to_pow2_multiple().
      std::uintptr_t iStep = static_cast<std::uintptr_t>(m_cbAlign - 1);
      return (ibOffset + iStep) & ~iStep;
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

   @param pDst
      Pointer to the destination memory area. The memory is supposed to be uninitialized.
   @param pSrc
      Pointer to the object to copy.
   */
   void copy_construct(void * pDst, void const * pSrc) const {
      copy_construct(pDst, pSrc, static_cast<std::int8_t const *>(pSrc) + m_cb);
   }

   /*! Copy-constructs elements from an array to another.

   @param pDstBegin
      Pointer to the start of the destination array. The array is supposed to be uninitialized.
   @param pSrcBegin
      Pointer to the first element to copy.
   @param pSrcEnd
      Pointer to the end of the source array.
   */
   void copy_construct(void * pDstBegin, void const * pSrcBegin, void const * pSrcEnd) const {
      m_pfnCopyConstructImpl(pDstBegin, pSrcBegin, pSrcEnd);
   }

   /*! Destructs an object.

   @param p
      Pointer to the object to destruct.
   */
   void destruct(void const * p) const {
      destruct(p, static_cast<std::int8_t const *>(p) + m_cb);
   }

   /*! Destructs a range of elements in an array.

   @param pBegin
      Pointer to the first element to destruct.
   @param pEnd
      Pointer to beyond the last element to destruct.
   */
   void destruct(void const * pBegin, void const * pEnd) const {
      m_pfnDestructImpl(pBegin, pEnd);
   }

   /*! Move-constructs an object from one memory location to another.

   @param pDst
      Pointer to the destination memory area. The memory is supposed to be uninitialized.
   @param pSrc
      Pointer to the object to move.
   */
   void move_construct(void * pDst, void * pSrc) const {
      move_construct(pDst, pSrc, static_cast<std::int8_t *>(pSrc) + m_cb);
   }

   /*! Move-constructs elements from an array to another.

   @param pDstBegin
      Pointer to the start of the destination array. The array is supposed to be uninitialized.
   @param pSrcBegin
      Pointer to the first element to move.
   @param pSrcEnd
      Pointer to the end of the source array.
   */
   void move_construct(void * pDstBegin, void * pSrcBegin, void * pSrcEnd) const {
      m_pfnMoveConstructImpl(pDstBegin, pSrcBegin, pSrcEnd);
   }

   //! Makes alignment() and align_pointer() available.
   template <typename T>
   void set_align() {
      m_cbAlign = static_cast<std::uint16_t>(alignof(T));
   }

   //! Makes copy_construct() available (trivial copy case).
   template <typename T>
   void set_copy_construct(typename _std::enable_if<
      _std::is_trivially_copy_constructible<T>::value, T *
   >::type = nullptr) {
      set_size<T>();
      m_pfnCopyConstructImpl = reinterpret_cast<copy_construct_impl_type>(
         &copy_construct_trivial_impl
      );
   }

   //! Makes copy_construct() available (non-trivial copy case).
   template <typename T>
   void set_copy_construct(typename _std::enable_if<
      !_std::is_trivially_copy_constructible<T>::value, T *
   >::type = nullptr) {
      set_size<T>();
      m_pfnCopyConstructImpl = reinterpret_cast<copy_construct_impl_type>(
         &copy_construct_impl<typename _std::remove_cv<T>::type>
      );
   }

   //! Makes destruct() available (trivial copy case).
   template <typename T>
   void set_destruct(typename _std::enable_if<
      _std::is_trivially_destructible<T>::value, T *
   >::type = nullptr) {
      set_size<T>();
      m_pfnDestructImpl = reinterpret_cast<destruct_impl_type>(&destruct_trivial_impl);
   }

   //! Makes destruct() available (non-trivial copy case).
   template <typename T>
   void set_destruct(typename _std::enable_if<
      !_std::is_trivially_destructible<T>::value, T *
   >::type = nullptr) {
      set_size<T>();
      m_pfnDestructImpl = reinterpret_cast<destruct_impl_type>(
         &destruct_impl<typename _std::remove_cv<T>::type>
      );
   }

   //! Makes move_construct() available (trivial copy case).
   template <typename T>
   void set_move_construct(typename _std::enable_if<
      _std::is_trivially_move_constructible<T>::value, T *
   >::type = nullptr) {
      set_size<T>();
      m_pfnMoveConstructImpl = reinterpret_cast<move_construct_impl_type>(
         // A trivial copy move works just fine for a trivial move.
         &copy_construct_trivial_impl
      );
   }

   //! Makes move_construct() available (non-trivial copy case).
   template <typename T>
   void set_move_construct(typename _std::enable_if<
      !_std::is_trivially_move_constructible<T>::value, T *
   >::type = nullptr) {
      set_size<T>();
      m_pfnMoveConstructImpl = reinterpret_cast<move_construct_impl_type>(
         &move_construct_impl<typename _std::remove_cv<T>::type>
      );
   }

   //! Makes size() available.
   template <typename T>
   void set_size() {
      m_cb = static_cast<std::uint16_t>(sizeof(T));
   }

   /*! Returns the size of an object of this type, in bytes.

   @return
      Size of an object, in bytes.
   */
   std::size_t size() const {
      return m_cb;
   }

private:
   /*! Copies a range of elements from one array to another, overwriting any existing contents in
   the destination.

   @param ptDstBegin
      Pointer to the start of the destination array. The elements are supposed to be uninitialized.
   @param ptSrcBegin
      Pointer to the first element to copy.
   @param ptSrcEnd
      Pointer to beyond the last element to copy.
   */
   template <typename T>
   static void copy_construct_impl(T * ptDstBegin, T const * ptSrcBegin, T const * ptSrcEnd) {
      /* Assume that since it’s not trivial, it can throw exceptions, so perform a transactional
      copy. */
      T * ptDst = ptDstBegin;
      try {
         for (T const * ptSrc = ptSrcBegin; ptSrc < ptSrcEnd; ++ptSrc, ++ptDst) {
            ::new(ptDst) T(*ptSrc);
         }
      } catch (...) {
         // Undo (destruct) all the copies instantiated.
         while (--ptDst >= ptDstBegin) {
            ptDst->~T();
         }
         throw;
      }
   }

   /*! Copies memory from one array to another. Replaces copy_construct_impl() and/or
   move_construct_impl() in case of trivial copy constructor and/or move constructor.

   @param pbDstBegin
      Pointer to the start of the destination array.
   @param pbSrcBegin
      Pointer to the first byte to copy.
   @param pbSrcEnd
      Pointer to beyond the last byte to copy.
   */
   static void copy_construct_trivial_impl(
      std::int8_t * pbDstBegin, std::int8_t * pbSrcBegin, std::int8_t * pbSrcEnd
   );

   /*! Destructs a range of elements in an array.

   @param ptBegin
      Pointer to the first element to destruct.
   @param ptEnd
      Pointer to beyond the last element to destruct.
   */
   template <typename T>
   static void destruct_impl(T const * ptBegin, T const * ptEnd) {
      for (T const * pt = ptBegin; pt < ptEnd; ++pt) {
         pt->~T();
      }
   }

   /*! No-op to replace destruct_impl() in case of trivial destructor.

   @param pBegin
      Pointer to the start of the memory that would be destructed.
   @param pEnd
      Pointer to the end of the memory that would be destructed.
   */
   static void destruct_trivial_impl(void const * pBegin, void const * pEnd);

   /*! Moves a range of elements from one array to another, overwriting any existing contents in the
   destination.

   @param ptDstBegin
      Pointer to the start of the destination array. The elements are supposed to be uninitialized.
   @param ptSrcBegin
      Pointer to the first element to copy.
   @param ptSrcEnd
      Pointer to beyond the last element to copy.
   */
   template <typename T>
   static void move_construct_impl(T * ptDstBegin, T * ptSrcBegin, T * ptSrcEnd) {
      for (T * ptSrc = ptSrcBegin, * ptDst = ptDstBegin; ptSrc < ptSrcEnd; ++ptSrc, ++ptDst) {
         ::new(ptDst) T(_std::move(*ptSrc));
      }
   }

private:
   /*! Size of a variable of this type, in bytes. First member because it’s the most frequently
   used, and having it at offset 0 may lead to faster or more compact code. */
   std::uint16_t m_cb;
   //! Alignment of a variable of this type, in bytes.
   std::uint16_t m_cbAlign;
   //! Pointer to a function to copy elements from one array to another.
   copy_construct_impl_type m_pfnCopyConstructImpl;
   //! Pointer to a function to destruct elements in an array.
   destruct_impl_type m_pfnDestructImpl;
   //! Pointer to a function to move elements from one array to another.
   move_construct_impl_type m_pfnMoveConstructImpl;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_TYPE_VOID_ADAPTER_HXX
