/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/collections.hxx>
#include <lofty/collections/_pvt/complex_vextr_impl.hxx>
#include <lofty/memory.hxx>
#include <lofty/numeric.hxx>
#include <lofty/_std/memory.hxx>
#include <lofty/type_void_adapter.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

vextr_impl_base::vextr_impl_base(std::size_t embedded_byte_capacity) {
   begin_ptr = nullptr;
   end_ptr = nullptr;
   has_embedded_prefixed_array = embedded_byte_capacity > 0;
   if (has_embedded_prefixed_array) {
      // Assign embedded_byte_capacity to the embedded item array that follows *this.
      embedded_prefixed_array()->capacity = embedded_byte_capacity;
   }
   array_is_prefixed = false;
   dynamic = false;
   has_nul_term = false;
}

vextr_impl_base::vextr_impl_base(
   std::size_t embedded_byte_capacity, void const * const_src_begin, void const * const_src_end,
   bool has_nul_term_ /*= false*/
) {
   begin_ptr = const_cast<void *>(const_src_begin);
   end_ptr = const_cast<void *>(const_src_end);
   has_embedded_prefixed_array = embedded_byte_capacity > 0;
   if (has_embedded_prefixed_array) {
      // Assign embedded_byte_capacity to the embedded item array that follows *this.
      embedded_prefixed_array()->capacity = embedded_byte_capacity;
   }
   array_is_prefixed = false;
   dynamic = false;
   has_nul_term = has_nul_term_;
}

/*static*/ std::size_t vextr_impl_base::calculate_increased_capacity(
   std::size_t old_size, std::size_t new_size
) {
   std::size_t new_capacity;
   // Avoid a pointless multiplication by 0.
   if (old_size) {
      new_capacity = old_size * growth_rate;
      // Check for overflow.
      if (new_capacity <= old_size) {
         /* If std::size_t overflowed, the memory allocation cannot possibly succeed; just return a very large
         number instead. */
         return numeric::max<std::size_t>::value;
      }
   } else {
      new_capacity = capacity_bytes_min;
   }
   if (new_capacity < new_size) {
      /* The item array is growing faster than our hard-coded growth rate, so just use the new size as the
      capacity. */
      new_capacity = new_size;
   }
   if (new_capacity - old_size < capacity_bytes_min) {
      /* Make sure we don’t increase by less than capacity_bytes_min bytes, so we won’t reallocate right on
      the next size change. */
      new_capacity = old_size + capacity_bytes_min;
   }
   return new_capacity;
}

void vextr_impl_base::validate_pointer(void const * p, bool allow_end) const {
   auto validity_end = static_cast<std::int8_t const *>(end_ptr);
   if (allow_end) {
      ++validity_end;
   }
   if (p < begin_ptr || static_cast<std::int8_t const *>(p) >= validity_end) {
      LOFTY_THROW(out_of_range, (p, begin_ptr, end_ptr));
   }
}

/*static*/ void vextr_impl_base::validate_pointer(
   vextr_impl_base const * this_ptr, void const * p, bool allow_end
) {
   if (!this_ptr) {
      LOFTY_THROW(collections::out_of_range, ());
   }
   this_ptr->validate_pointer(p, allow_end);
}

}}} //namespace lofty::collections::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

vextr_transaction::vextr_transaction(vextr_impl_base * target_, bool trivial, std::size_t new_size) :
   target(target_) {
   _construct(trivial, new_size);
}
vextr_transaction::vextr_transaction(
   vextr_impl_base * target_, bool trivial, std::size_t insert_size, std::size_t remove_size
) :
   target(target_) {
   _construct(trivial, target->size<std::int8_t>() + insert_size - remove_size);
}

void vextr_transaction::commit() {
   // If we are abandoning the old item array, proceed to destruct it if necessary.
   if (will_replace_array()) {
      target->~vextr_impl_base();
      // work_copy no longer owns its item array.
      work_copy_array_needs_free = false;
   }
   // Update the target object.
   target->assign_shallow(work_copy);

   // TODO: consider releasing some memory from an oversized dynamically-allocated item array.
}

void vextr_transaction::_construct(bool trivial, std::size_t new_size) {
   work_copy_array_needs_free = false;
   if (new_size == 0) {
      // Empty string/array: no need to use an item array.
      work_copy.assign_empty();
   } else {
      /* Since we never write to non-prefixed item arrays and we’re in a transaction to prepare to write to
      one, it must be prefixed. */
      work_copy.array_is_prefixed = true;
      // Any change in size voids the NUL termination of the item array.
      work_copy.has_nul_term = false;

      auto embedded_pfx_array = target->embedded_prefixed_array();
      if (embedded_pfx_array && new_size <= embedded_pfx_array->capacity) {
         // The embedded item array is large enough; switch to using it.
         work_copy.begin_ptr = embedded_pfx_array->array;
         work_copy.dynamic = false;
      } else if (target->array_is_prefixed && new_size <= target->capacity<std::int8_t>()) {
         // The current item array is prefixed (writable) and large enough, no need to change anything.
         work_copy.begin_ptr = target->begin_ptr;
         work_copy.dynamic = target->dynamic;
      } else {
         // The current item array (embedded or dynamic) is not large enough.

         // Calculate the total allocation size.
         std::size_t orig_size = target->size<std::int8_t>();
         std::size_t new_capacity = vextr_impl_base::calculate_increased_capacity(orig_size, new_size);
         typedef vextr_impl_base::_prefixed_array prefixed_array;
         std::size_t new_array_desc_size = LOFTY_OFFSETOF(prefixed_array, array) + new_capacity;
         prefixed_array * pfx_array;
         if (trivial && target->dynamic) {
            /* Resize the current dynamically-allocated item array. Notice that the reallocation is effective
            immediately, which means that target must be updated now – if no exceptions are thrown, that
            is. */
            pfx_array = target->prefixed_array();
            memory::realloc(&pfx_array, new_array_desc_size);
            target->begin_ptr = pfx_array->array;
            target->end_ptr = target->begin<std::int8_t>() + orig_size;
         } else {
            /* Allocate a new item array. This is the only option for non-trivial types because they must be
            moved using their move constructor. */
            pfx_array = memory::alloc<prefixed_array>(new_array_desc_size);
            work_copy_array_needs_free = true;
         }
         pfx_array->capacity = new_capacity;
         work_copy.begin_ptr = pfx_array->array;
         work_copy.dynamic = true;
      }
      work_copy.end_ptr = static_cast<std::int8_t *>(work_copy.begin_ptr) + new_size;
   }
}

}}} //namespace lofty::collections::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

void complex_vextr_impl::assign_concat(
   type_void_adapter const & type, void const * src1_begin, void const * src1_end, void const * src2_begin,
   void const * src2_end, std::uint8_t move
) {
   auto src1_size = reinterpret_cast<std::size_t>(src1_end) - reinterpret_cast<std::size_t>(src1_begin);
   auto src2_size = reinterpret_cast<std::size_t>(src2_end) - reinterpret_cast<std::size_t>(src2_begin);
   vextr_transaction trn(this, false, src1_size + src2_size);
   std::size_t orig_size = size<std::int8_t>();
   _std::unique_ptr<std::int8_t[]> backup_array;
   std::int8_t * work_copy = trn.work_array<std::int8_t>();
   if (src1_size > 0 || src2_size > 0) {
      /* If we’re going to overwrite the old item array and we’re going to perform copies (exception hazard),
      move the items to a backup array so we can restore them in case of exceptions. */
      if (orig_size && move != (1 & 2) && !trn.will_replace_array()) {
         backup_array.reset(new std::int8_t[orig_size]);
         type.move_construct(backup_array.get(), begin_ptr, end_ptr);
         destruct_items(type);
      }
      try {
         if (src1_size > 0) {
            if (move & 1) {
               type.move_construct(work_copy, const_cast<void *>(src1_begin), const_cast<void *>(src1_end));
            } else {
               type.copy_construct(work_copy, src1_begin, src1_end);
            }
            work_copy += src1_size;
         }
         if (src2_size > 0) {
            if (move & 2) {
               type.move_construct(work_copy, const_cast<void *>(src2_begin), const_cast<void *>(src2_end));
            } else {
               type.copy_construct(work_copy, src2_begin, src2_end);
            }
         }
      } catch (...) {
         // If we already constructed the copies of src1, destruct them.
         std::int8_t * src1_work_copy_begin = trn.work_array<std::int8_t>();
         if (work_copy > src1_work_copy_begin) {
            std::int8_t * src1_work_copy_end = src1_work_copy_begin + src1_size;
            if (move & 1) {
               /* If we moved them from src1, don’t forget to move them back. Of course this means that we
               first have to destruct src1’s items and re-construct them. */
               type.destruct(src1_begin, src1_end);
               type.move_construct(const_cast<void *>(src1_begin), src1_work_copy_begin, src1_work_copy_end);
            }
            type.destruct(src1_work_copy_begin, src1_work_copy_end);
         }
         // If earlier we decided to make a backup, restore it now, then destruct it.
         if (backup_array) {
            type.move_construct(begin_ptr, backup_array.get(), backup_array.get() + orig_size);
            type.destruct(backup_array.get(), backup_array.get() + orig_size);
         }
         throw;
      }
   }
   if (orig_size) {
      /* If we made a backup, it also means that now that’s the only copy of the original items, so we must
      use it to destruct them, instead of destruct_items(). */
      if (backup_array) {
         type.destruct(backup_array.get(), backup_array.get() + orig_size);
      } else {
         destruct_items(type);
      }
   }
   trn.commit();
}

void complex_vextr_impl::assign_move_desc_or_move_items(
   type_void_adapter const & type, complex_vextr_impl && src
) {
   if (src.begin_ptr == begin_ptr) {
      return;
   }
   if (src.dynamic) {
      // Discard the current contents.
      destruct_items(type);
      this->~complex_vextr_impl();
      // Take over the item array.
      assign_shallow(src);
   } else {
      /* Can’t move the item array, so move the items instead. assign_concat() is fast enough; pass the source
      as the second argument pair, because its code path is faster. */
      assign_concat(type, nullptr, nullptr, src.begin_ptr, src.end_ptr, 2);
      // And now empty the source.
      src.destruct_items(type);
   }
   src.assign_empty();
}

void complex_vextr_impl::destruct_items(type_void_adapter const & type) {
   type.destruct(begin_ptr, end_ptr);
}

/*! Safely moves a range of items to another position in the same array, carefully moving items in case the
source and the destination ranges overlap. Note that this will also destruct the source items.

type
   Adapter for the items’ type.
dst_begin_v
   Pointer to the start of the destination array. The items are supposed to be uninitialized.
src_begin_v
   Pointer to the first item to move.
src_end_v
   Pointer to beyond the last item to move.
*/
static void overlapping_move_construct(
   type_void_adapter const & type, void * dst_begin_v, void * src_begin_v, void * src_end_v
) {
   if (dst_begin_v == src_begin_v) {
      return;
   }
   auto src_begin = static_cast<std::int8_t *>(src_begin_v);
   auto src_end   = static_cast<std::int8_t *>(src_end_v  );
   auto dst_begin = static_cast<std::int8_t *>(dst_begin_v);
   auto dst_end   = dst_begin + (src_end - src_begin);
   if (dst_begin < src_begin && src_begin < dst_end) {
      /*
      ┌─────────────────┐
      │ a - - B C D e f │
      ├─────────────────┤
      │ a B C D - - e f │
      └─────────────────┘
      */
      // Move the items from left to right (the block moves from right to left).
      auto size_before_overlap = static_cast<std::size_t>(src_begin - dst_begin);

      /* Move-construct the items that have an unused destination, then destruct them, so they can be
      overwritten by the next move if necessary. */
      type.move_construct(dst_begin, src_begin, src_begin + size_before_overlap);
      type.destruct(src_begin, src_begin + size_before_overlap);
      /*
      ┌─────────────────┐
      │ a B C - - D e f │
      └─────────────────┘
      */
      // Move-assign all the remaining items (the overlapping area) to shift them.
      type.move_construct(src_begin, src_begin + size_before_overlap, src_end);
      // Destruct their former locations.
      type.destruct(src_begin + size_before_overlap, src_end);
      /*
      ┌─────────────────┐
      │ a B C D - - e f │
      └─────────────────┘
      */
   } else if (src_begin < dst_begin && dst_begin < src_end) {
      /*
      ┌─────────────────┐
      │ a B C D - - e f │
      ├─────────────────┤
      │ a - - B C D e f │
      └─────────────────┘
      */
      /* This situation is the mirror of the above, so the move must be done backwards, copying right to left
      (the block moves from left to right). */
      std::size_t size_after_overlap = static_cast<std::size_t>(dst_end - src_end);

      /* Move-construct the items that have an unused destination, then destruct them, so they can be
      overwritten by the next move if necessary. */
      type.move_construct(src_end, src_end - size_after_overlap, src_end);
      type.destruct(dst_end, src_end);
      /*
      ┌─────────────────┐
      │ a B - - C D e f │
      └─────────────────┘
      */
      /* Move-assign backwards all the remaining items (the overlapping area) to shift them. This is slow,
      costing two function calls for each item. */
      auto src_elt = src_end - size_after_overlap, dst_elt = src_end;
      while (src_elt > src_begin) {
         auto src_elt_end = src_elt;
         src_elt -= type.size();
         dst_elt -= type.size();
         type.move_construct(dst_elt, src_elt, src_elt_end);
         type.destruct(src_elt, src_elt_end);
      }
      /*
      ┌─────────────────┐
      │ a - - B C D e f │
      └─────────────────┘
      */
   } else {
      type.move_construct(dst_begin, src_begin, src_end);
   }
}

void complex_vextr_impl::insert(
   type_void_adapter const & type, std::size_t offset, void const * src, std::size_t src_size, bool move
) {
   vextr_transaction trn(this, false, src_size, 0);
   std::int8_t * dst = begin<std::int8_t>() + offset;
   void const * src_end = static_cast<std::int8_t const *>(src) + src_size;
   std::int8_t * work_dst_begin = trn.work_array<std::int8_t>() + offset;
   std::int8_t * work_dst_end = static_cast<std::int8_t *>(work_dst_begin) + src_size;
   /* Regardless of whether we’re switching item arrays, the items beyond the insertion point must always be
   moved. */
   std::size_t tail_size = static_cast<std::size_t>(end<std::int8_t>() - dst);
   if (tail_size) {
      overlapping_move_construct(type, work_dst_end, dst, end<std::int8_t>());
   }
   // Copy/move the new items over.
   if (move) {
      // No point in using try/catch here; we just assume that a move constructor won’t throw.
      type.move_construct(work_dst_begin, const_cast<void *>(src), const_cast<void *>(src_end));
   } else {
      try {
         type.copy_construct(work_dst_begin, src, src_end);
      } catch (...) {
         // Undo the overlapping_move_construct() above.
         if (tail_size) {
            overlapping_move_construct(type, dst, work_dst_end, work_dst_end + tail_size);
         }
         throw;
      }
   }
   /* Also move to the new array the items before the insertion point, otherwise we’ll lose them in the
   switch. */
   if (offset && trn.will_replace_array()) {
      type.move_construct(trn.work_array<void>(), begin_ptr, dst);
      type.destruct(begin_ptr, dst);
   }
   trn.commit();
}

void complex_vextr_impl::remove(type_void_adapter const & type, std::size_t offset, std::size_t remove_size) {
   vextr_transaction trn(this, false, 0, remove_size);
   std::int8_t * remove_begin = begin<std::int8_t>() + offset;
   std::int8_t * remove_end = remove_begin + remove_size;
   // Destruct the items to be removed.
   type.destruct(remove_begin, remove_end);
   /* The items beyond the last removed must be either copied to the new item array at remove_size offset, or
   shifted to remove_begin in the old item array. */
   if (remove_end < end_ptr) {
      if (trn.will_replace_array()) {
         type.move_construct(trn.work_array<std::int8_t>() + offset, remove_end, end_ptr);
         type.destruct(remove_end, end_ptr);
      } else {
         overlapping_move_construct(type, remove_begin, remove_end, end_ptr);
      }
   }
   /* Also move to the new array the items before the first deleted one, otherwise we’ll lose them in the
   switch. */
   if (offset && trn.will_replace_array()) {
      type.move_construct(trn.work_array<void>(), begin_ptr, remove_begin);
      type.destruct(begin_ptr, remove_begin);
   }
   trn.commit();
}

void complex_vextr_impl::set_capacity(
   type_void_adapter const & type, std::size_t new_capacity_min, bool preserve
) {
   vextr_transaction trn(this, false, new_capacity_min);
   std::size_t orig_size = size<std::int8_t>();
   if (trn.will_replace_array()) {
      // Destruct every item from the array we’re abandoning, but first move-construct them if told to do so.
      if (preserve) {
         type.move_construct(trn.work_array<void>(), begin_ptr, end_ptr);
      }
      destruct_items(type);
      if (!preserve) {
         // We just destructed the items.
         orig_size = 0;
      }
   }
   trn.commit();
   // The transaction changed the size to new_capacity_min, which is not necessarily correct.
   end_ptr = begin<std::int8_t>() + orig_size;
}

void complex_vextr_impl::set_size(type_void_adapter const & type, std::size_t new_size) {
   LOFTY_UNUSED_ARG(type);
   LOFTY_UNUSED_ARG(new_size);
   // TODO: implement this.
}

}}} //namespace lofty::collections::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

void trivial_vextr_impl::assign_concat(
   void const * src1_begin, void const * src1_end, void const * src2_begin, void const * src2_end
) {
   auto src1_size = reinterpret_cast<std::size_t>(src1_end) - reinterpret_cast<std::size_t>(src1_begin);
   auto src2_size = reinterpret_cast<std::size_t>(src2_end) - reinterpret_cast<std::size_t>(src2_begin);
   vextr_transaction trn(this, true, src1_size + src2_size);
   std::int8_t * work_copy = trn.work_array<std::int8_t>();
   if (src1_size > 0) {
      memory::copy(work_copy, static_cast<std::int8_t const *>(src1_begin), src1_size);
      work_copy += src1_size;
   }
   if (src2_size > 0) {
      memory::copy(work_copy, static_cast<std::int8_t const *>(src2_begin), src2_size);
   }

   trn.commit();
}

void trivial_vextr_impl::assign_move_desc_or_move_items(trivial_vextr_impl && src) {
   if (src.begin_ptr == begin_ptr) {
      return;
   }
   if (src.dynamic || !src.array_is_prefixed) {
      // A dynamic or non-prefixed item array can be moved; just transfer its ownership.
      this->~trivial_vextr_impl();
      assign_shallow(src);
   } else {
      /* A static (prefixed) item array can’t be moved, so copy (same as move, for trivial items) its items
      instead. */
      assign_copy(src.begin_ptr, src.end_ptr);
   }
   src.assign_empty();
}

void trivial_vextr_impl::assign_share_raw_or_copy_desc(trivial_vextr_impl const & src) {
   /* This also checks that the source pointer (&src) is safe to dereference, so the following code can
   proceed safely. */
   if (src.begin_ptr == begin_ptr) {
      return;
   }
   if (src.array_is_prefixed) {
      // Cannot share a prefixed item array.
      assign_copy(src.begin_ptr, src.end_ptr);
   } else {
      // Discard the current contents.
      this->~trivial_vextr_impl();
      // Share the source non-prefixed item array.
      assign_shallow(src);
   }
}

void trivial_vextr_impl::_insert_remove(
   std::size_t offset, void const * insert_src, std::size_t insert_size, std::size_t remove_size
) {
   vextr_transaction trn(this, true, insert_size, remove_size);
   std::int8_t const * remove_end = begin<std::int8_t>() + offset + remove_size;
   std::int8_t * work_offset = trn.work_array<std::int8_t>() + offset;
   /* Regardless of an item array switch, the items beyond the insertion point (when adding) or the last
   removed (when removing) must always be moved/copied. */
   if (std::size_t tail_size = static_cast<std::size_t>(end<std::int8_t>() - remove_end)) {
      memory::move(work_offset + insert_size, remove_end, tail_size);
   }
   if (insert_src) {
      // Copy the new items over.
      memory::copy(work_offset, static_cast<std::int8_t const *>(insert_src), insert_size);
   }
   // Also copy to the new array the items before offset, otherwise we’ll lose them in the switch.
   if (offset && trn.will_replace_array()) {
      memory::copy(trn.work_array<std::int8_t>(), begin<std::int8_t>(), offset);
   }

   trn.commit();
}

void trivial_vextr_impl::set_capacity(std::size_t new_capacity_min, bool preserve) {
   vextr_transaction trn(this, true, new_capacity_min);
   std::size_t orig_size = size<std::int8_t>();
   if (trn.will_replace_array()) {
      if (preserve) {
         memory::copy(trn.work_array<std::int8_t>(), begin<std::int8_t>(), orig_size);
      } else {
         // We’ll lose the item array when the transaction is commited.
         orig_size = 0;
      }
   }
   trn.commit();
   // The transaction changed the size to new_capacity_min, which is incorrect.
   end_ptr = begin<std::int8_t>() + orig_size;
}

void trivial_vextr_impl::set_size(std::size_t new_size) {
   if (new_size != size<std::int8_t>()) {
      if (new_size > capacity<std::int8_t>()) {
         // Enlarge the item array.
         set_capacity(new_size, true);
      }
      end_ptr = begin<std::int8_t>() + new_size;
   }
}

}}} //namespace lofty::collections::_pvt
