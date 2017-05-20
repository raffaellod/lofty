/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_COLLECTIONS__PVT_COMPLEX_VEXTR_IMPL_HXX
#define _LOFTY_COLLECTIONS__PVT_COMPLEX_VEXTR_IMPL_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declaration.
namespace lofty {

class type_void_adapter;

} //namespace lofty

namespace lofty { namespace collections { namespace _pvt {

//! Template-independent implementation of a vector for non-trivial contained types.
class LOFTY_SYM complex_vextr_impl : public vextr_impl_base {
public:
   /*! Copies or moves the contents of the two sources to *this, according to the source type. If move != 0,
   the source items will be moved by having their const-ness cast away – be careful.

   @param type
      Adapter for the items’ type.
   @param src1_begin
      Pointer to the start of the first source array.
   @param src1_end
      Pointer to the end of the first source array.
   @param src2_begin
      Pointer to the start of the second source array.
   @param src2_end
      Pointer to the end of the second source array.
   @param move
      Pass 1 to move the items from the first source array to the vextr’s item array, 2 to move the items from
      the second source array, or 3 to move both, or 0 to copy them all instead.
   */
   void assign_concat(
      type_void_adapter const & type, void const * src1_begin, void const * src1_end, void const * src2_begin,
      void const * src2_end, std::uint8_t move
   );

   /*! Copies the contents of the source to *this.

   @param type
      Adapter for the items’ type.
   @param src_begin
      Pointer to the start of the source array.
   @param src_end
      Pointer to the end of the source array.
   */
   void assign_copy(type_void_adapter const & type, void const * src_begin, void const * src_end) {
      if (src_begin == begin_ptr) {
         return;
      }
      /* assign_concat() is fast enough; pass the source as the second argument pair, because its code path is
      faster. */
      assign_concat(type, nullptr, nullptr, src_begin, src_end, 0);
   }

   /*! Moves the source’s item array if dynamically-allocated or not prefixed, else copies it to *this, moving
   the items instead.

   @param type
      Adapter for the items’ type.
   @param src
      Source vextr.
   */
   void assign_move_desc_or_move_items(type_void_adapter const & type, complex_vextr_impl && src);

   /*! Destructs the item array. It does not deallocate the item array.

   @param type
      Adapter for the items’ type.
   */
   void destruct_items(type_void_adapter const & type);

   /*! Inserts items at a specific position in the vextr.

   @param type
      Adapter for the items’ type.
   @param offset
      Byte index at which the items should be inserted.
   @param src
      Pointer to the first item to insert.
   @param src_size
      Size of the array pointed to by src, in bytes.
   @param move
      true to move the items from src to the vextr’s item array, or false to copy them instead.
   */
   void insert(
      type_void_adapter const & type, std::size_t offset, void const * src,
      std::size_t src_size, bool move
   );

   /*! Removes items from the vextr.

   @param type
      Adapter for the items’ type.
   @param offset
      Byte index at which the items should be removed.
   @param remove_size
      Size of the array slice to remove, in bytes.
   */
   void remove(type_void_adapter const & type, std::size_t offset, std::size_t remove_size);

   /*! Ensures that the item array has at least new_capacity_min of actual item space. If this causes *this to
   switch to using a different item array, any data in the current one will be lost unless preserve == true.

   @param type
      Adapter for the items’ type.
   @param new_capacity_min
      Minimum size of items requested, in bytes.
   @param preserve
      If true, the previous contents of the item array will be preserved even if the reallocation causes the
      vextr to switch to a different item array.
   */
   void set_capacity(type_void_adapter const & type, std::size_t new_capacity_min, bool preserve);

   /*! Changes the count of items in the vextr. If the new item count is greater than the current one, the
   added items will be left uninitialized; it’s up to the caller to make sure that these items are properly
   constructed, or problems will arise when the destructor will attempt to destruct these items.

   @param type
      Adapter for the items’ type.
   @param new_size
      New size of the items, in bytes.
   */
   void set_size(type_void_adapter const & type, std::size_t new_size);

protected:
   //! See vextr_impl_base::vextr_impl_base().
   complex_vextr_impl(std::size_t embedded_byte_capacity) :
      vextr_impl_base(embedded_byte_capacity) {
   }

   //! See vextr_impl_base::vextr_impl_base().
   complex_vextr_impl(
      std::size_t embedded_byte_capacity, void const * const_src_begin, void const * const_src_end
   ) :
      vextr_impl_base(embedded_byte_capacity, const_src_begin, const_src_end) {
   }
};

}}} //namespace lofty::collections::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_COLLECTIONS__PVT_COMPLEX_VEXTR_IMPL_HXX
