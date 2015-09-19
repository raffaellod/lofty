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

#ifndef _ABACLADE_COLLECTIONS_DETAIL_COMPLEX_VEXTR_IMPL_HXX
#define _ABACLADE_COLLECTIONS_DETAIL_COMPLEX_VEXTR_IMPL_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declaration.
namespace abc {

class type_void_adapter;

} //namespace abc

namespace abc { namespace collections { namespace detail {

//! Template-independent implementation of a vector for non-trivial contained types.
class ABACLADE_SYM complex_vextr_impl : public vextr_impl_base {
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

   /*! Moves the source’s item array if dynamically-allocated or not prefixed, else copies it to
   *this, moving the items instead.

   @param type
      Adapter for the items’ type.
   @param rcvi
      Source vextr.
   */
   void assign_move_desc_or_move_items(type_void_adapter const & type, complex_vextr_impl && rcvi);

   /*! Destructs the item array. It does not deallocate the item array.

   @param type
      Adapter for the items’ type.
   */
   void destruct_items(type_void_adapter const & type);

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
   //! See vextr_impl_base::vextr_impl_base().
   complex_vextr_impl(std::size_t cbEmbeddedCapacity) :
      vextr_impl_base(cbEmbeddedCapacity) {
   }

   //! See vextr_impl_base::vextr_impl_base().
   complex_vextr_impl(
      std::size_t cbEmbeddedCapacity, void const * pConstSrcBegin, void const * pConstSrcEnd
   ) :
      vextr_impl_base(cbEmbeddedCapacity, pConstSrcBegin, pConstSrcEnd) {
   }
};

}}} //namespace abc::collections::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COLLECTIONS_DETAIL_COMPLEX_VEXTR_IMPL_HXX
