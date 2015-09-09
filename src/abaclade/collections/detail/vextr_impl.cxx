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

#include <abaclade.hxx>
#include <abaclade/collections.hxx>
#include <abaclade/numeric.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

raw_vextr_impl_base::raw_vextr_impl_base(std::size_t cbEmbeddedCapacity) {
   m_pBegin = nullptr;
   m_pEnd = nullptr;
   mc_bEmbeddedPrefixedItemArray = cbEmbeddedCapacity > 0;
   if (mc_bEmbeddedPrefixedItemArray) {
      // Assign cbEmbeddedCapacity to the embedded item array that follows *this.
      embedded_prefixed_item_array()->m_cbCapacity = cbEmbeddedCapacity;
   }
   m_bPrefixedItemArray = false;
   m_bDynamic = false;
   m_bNulT = false;
}

raw_vextr_impl_base::raw_vextr_impl_base(
   std::size_t cbEmbeddedCapacity, void const * pConstSrcBegin, void const * pConstSrcEnd,
   bool bNulT /*= false*/
) {
   m_pBegin = const_cast<void *>(pConstSrcBegin);
   m_pEnd = const_cast<void *>(pConstSrcEnd);
   mc_bEmbeddedPrefixedItemArray = cbEmbeddedCapacity > 0;
   if (mc_bEmbeddedPrefixedItemArray) {
      // Assign cbEmbeddedCapacity to the embedded item array that follows *this.
      embedded_prefixed_item_array()->m_cbCapacity = cbEmbeddedCapacity;
   }
   m_bPrefixedItemArray = false;
   m_bDynamic = false;
   m_bNulT = bNulT;
}

/*static*/ std::size_t raw_vextr_impl_base::calculate_increased_capacity(
   std::size_t cbOld, std::size_t cbNew
) {
   ABC_TRACE_FUNC(cbOld, cbNew);

   std::size_t cbNewCapacity;
   // Avoid a pointless multiplication by 0.
   if (cbOld) {
      cbNewCapacity = cbOld * smc_iGrowthRate;
      // Check for overflow.
      if (cbNewCapacity <= cbOld) {
         /* If std::size_t overflowed, the memory allocation cannot possibly succeed; just return a
         very large number instead. */
         return numeric::max<std::size_t>::value;
      }
   } else {
      cbNewCapacity = smc_cbCapacityMin;
   }
   if (cbNewCapacity < cbNew) {
      /* The item array is growing faster than our hard-coded growth rate, so just use the new size
      as the capacity. */
      cbNewCapacity = cbNew;
   }
   if (cbNewCapacity - cbOld < smc_cbCapacityMin) {
      /* Make sure we don’t increase by less than smc_cbCapacityMin bytes, so we won’t reallocate
      right on the next size change. */
      cbNewCapacity = cbOld + smc_cbCapacityMin;
   }
   return cbNewCapacity;
}

void raw_vextr_impl_base::validate_pointer(void const * p, bool bAllowEnd) const {
   auto pEnd = static_cast<std::int8_t const *>(m_pEnd);
   if (bAllowEnd) {
      ++pEnd;
   }
   if (p < m_pBegin || static_cast<std::int8_t const *>(p) >= pEnd) {
      ABC_THROW(out_of_range, (p, m_pBegin, m_pEnd));
   }
}

/*static*/ void raw_vextr_impl_base::validate_pointer(
   raw_vextr_impl_base const * prvib, void const * p, bool bAllowEnd
) {
   if (!prvib) {
      ABC_THROW(collections::out_of_range, ());
   }
   prvib->validate_pointer(p, bAllowEnd);
}

}}} //namespace abc::collections::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

raw_vextr_transaction::raw_vextr_transaction(
   raw_vextr_impl_base * prvib, bool bTrivial, std::size_t cbNew
) :
   m_prvib(prvib) {
   ABC_TRACE_FUNC(this, prvib, bTrivial, cbNew);

   _construct(bTrivial, cbNew);
}
raw_vextr_transaction::raw_vextr_transaction(
   raw_vextr_impl_base * prvib, bool bTrivial, std::size_t cbAdd, std::size_t cbRemove
) :
   m_prvib(prvib) {
   ABC_TRACE_FUNC(this, prvib, bTrivial, cbAdd, cbRemove);

   _construct(bTrivial, prvib->size<std::int8_t>() + cbAdd - cbRemove);
}

void raw_vextr_transaction::commit() {
   ABC_TRACE_FUNC(this);

   bool bChangeItemArray = will_replace_item_array();
   // If we are abandoning the old item array, proceed to destruct it if necessary.
   if (bChangeItemArray) {
      m_prvib->~raw_vextr_impl_base();
      // m_rvibWork no longer owns its item array.
      m_bFree = false;
   }
   // Update the target object.
   m_prvib->assign_shallow(m_rvibWork);

   // TODO: consider releasing some memory from an oversized dynamically-allocated item array.
}

void raw_vextr_transaction::_construct(bool bTrivial, std::size_t cbNew) {
   ABC_TRACE_FUNC(this, bTrivial, cbNew);

   m_bFree = false;
   if (cbNew == 0) {
      // Empty string/array: no need to use an item array.
      m_rvibWork.assign_empty();
   } else {
      /* Since we never write to non-prefixed item arrays and we’re in a transaction to prepare to
      write to one, it must be prefixed. */
      m_rvibWork.m_bPrefixedItemArray = true;
      // Any change in size voids the NUL termination of the item array.
      m_rvibWork.m_bNulT = false;

      auto ppiaEmbedded = m_prvib->embedded_prefixed_item_array();
      if (ppiaEmbedded && cbNew <= ppiaEmbedded->m_cbCapacity) {
         // The embedded item array is large enough; switch to using it.
         m_rvibWork.m_pBegin = ppiaEmbedded->m_at;
         m_rvibWork.m_bDynamic = false;
      } else if (m_prvib->m_bPrefixedItemArray && cbNew <= m_prvib->capacity<std::int8_t>()) {
         /* The current item array is prefixed (writable) and large enough, no need to change
         anything. */
         m_rvibWork.m_pBegin = m_prvib->m_pBegin;
         m_rvibWork.m_bDynamic = m_prvib->m_bDynamic;
      } else {
         // The current item array (embedded or dynamic) is not large enough.

         // Calculate the total allocation size.
         std::size_t cbOrig = m_prvib->size<std::int8_t>();
         std::size_t cbNewCapacity = raw_vextr_impl_base::calculate_increased_capacity(
            cbOrig, cbNew
         );
         typedef raw_vextr_impl_base::_prefixed_item_array prefixed_item_array;
         std::size_t cbNewItemArrayDesc = ABC_OFFSETOF(prefixed_item_array, m_at) + cbNewCapacity;
         prefixed_item_array * ppia;
         if (bTrivial && m_prvib->m_bDynamic) {
            /* Resize the current dynamically-allocated item array. Notice that the reallocation is
            effective immediately, which means that m_prvib must be updated now – if no exceptions
            are thrown, that is. */
            ppia = m_prvib->prefixed_item_array();
            memory::realloc(&ppia, cbNewItemArrayDesc);
            m_prvib->m_pBegin = ppia->m_at;
            m_prvib->m_pEnd = m_prvib->begin<std::int8_t>() + cbOrig;
         } else {
            /* Allocate a new item array. This is the only option for non-trivial types because they
            must be moved using their move constructor. */
            ppia = memory::alloc<prefixed_item_array>(cbNewItemArrayDesc);
            m_bFree = true;
         }
         ppia->m_cbCapacity = cbNewCapacity;
         m_rvibWork.m_pBegin = ppia->m_at;
         m_rvibWork.m_bDynamic = true;
      }
      m_rvibWork.m_pEnd = static_cast<std::int8_t *>(m_rvibWork.m_pBegin) + cbNew;
   }
}

}}} //namespace abc::collections::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

void raw_complex_vextr_impl::assign_concat(
   type_void_adapter const & type, void const * p1Begin, void const * p1End, void const * p2Begin,
   void const * p2End, std::uint8_t iMove
) {
   ABC_TRACE_FUNC(this, /*type, */p1Begin, p1End, p2Begin, p2End, iMove);

   std::size_t cb1 = reinterpret_cast<std::size_t>(p1End) - reinterpret_cast<std::size_t>(p1Begin);
   std::size_t cb2 = reinterpret_cast<std::size_t>(p2End) - reinterpret_cast<std::size_t>(p2Begin);
   raw_vextr_transaction trn(this, false, cb1 + cb2);
   std::size_t cbOrig = size<std::int8_t>();
   _std::unique_ptr<std::int8_t[]> pbBackup;
   std::int8_t * pbWorkCopy = trn.work_array<std::int8_t>();
   if (cb1 || cb2) {
      /* If we’re going to overwrite the old item array and we’re going to perform copies (exception
      hazard), move the items to a backup array so we can restore them in case of exceptions. */
      if (cbOrig && iMove != 1 + 2 && !trn.will_replace_item_array()) {
         pbBackup.reset(new std::int8_t[cbOrig]);
         type.move_construct(pbBackup.get(), m_pBegin, m_pEnd);
         destruct_items(type);
      }
      try {
         if (cb1) {
            if (iMove & 1) {
               type.move_construct(
                  pbWorkCopy, const_cast<void *>(p1Begin), const_cast<void *>(p1End)
               );
            } else {
               type.copy_construct(pbWorkCopy, p1Begin, p1End);
            }
            pbWorkCopy += cb1;
         }
         if (cb2) {
            if (iMove & 2) {
               type.move_construct(
                  pbWorkCopy, const_cast<void *>(p2Begin), const_cast<void *>(p2End)
               );
            } else {
               type.copy_construct(pbWorkCopy, p2Begin, p2End);
            }
         }
      } catch (...) {
         // If we already constructed the copies of p1, destruct them.
         std::int8_t * pbWorkCopy1Begin = trn.work_array<std::int8_t>();
         if (pbWorkCopy > pbWorkCopy1Begin) {
            std::int8_t * pbWorkCopy1End = pbWorkCopy1Begin + cb1;
            if (iMove & 1) {
               /* If we moved them from p1, don’t forget to move them back. Of course this means
               that we first have to destruct p1’s items and re-construct them. */
               type.destruct(p1Begin, p1End);
               type.move_construct(const_cast<void *>(p1Begin), pbWorkCopy1Begin, pbWorkCopy1End);
            }
            type.destruct(pbWorkCopy1Begin, pbWorkCopy1End);
         }
         // If earlier we decided to make a backup, restore it now, then destruct it.
         if (pbBackup) {
            type.move_construct(m_pBegin, pbBackup.get(), pbBackup.get() + cbOrig);
            type.destruct(pbBackup.get(), pbBackup.get() + cbOrig);
         }
         throw;
      }
   }
   if (cbOrig) {
      /* If we made a backup, it also means that now that’s the only copy of the original items, so
      we must use it to destruct them, instead of destruct_items(). */
      if (pbBackup) {
         type.destruct(pbBackup.get(), pbBackup.get() + cbOrig);
      } else {
         destruct_items(type);
      }
   }
   trn.commit();
}

void raw_complex_vextr_impl::assign_move_desc_or_move_items(
   type_void_adapter const & type, raw_complex_vextr_impl && rcvi
) {
   ABC_TRACE_FUNC(this/*, type, rcvi*/);

   if (rcvi.m_pBegin == m_pBegin) {
      return;
   }
   if (rcvi.m_bDynamic) {
      // Discard the current contents.
      destruct_items(type);
      this->~raw_complex_vextr_impl();
      // Take over the item array.
      assign_shallow(rcvi);
   } else {
      /* Can’t move the item array, so move the items instead. assign_concat() is fast enough; pass
      the source as the second argument pair, because its code path is faster. */
      assign_concat(type, nullptr, nullptr, rcvi.m_pBegin, rcvi.m_pEnd, 2);
      // And now empty the source.
      rcvi.destruct_items(type);
   }
   rcvi.assign_empty();
}

/*! Safely moves a range of items to another position in the same array, carefully moving items in
case the source and the destination ranges overlap. Note that this will also destruct the source
items.

type
   Adapter for the items’ type.
pDstBegin
   Pointer to the start of the destination array. The items are supposed to be uninitialized.
pSrcBegin
   Pointer to the first item to move.
pSrcEnd
   Pointer to beyond the last item to move.
*/
static void overlapping_move_construct(
   type_void_adapter const & type, void * pDstBegin, void * pSrcBegin, void * pSrcEnd
) {
   ABC_TRACE_FUNC(/*type, */pDstBegin, pSrcBegin, pSrcEnd);

   if (pDstBegin == pSrcBegin) {
      return;
   }
   std::int8_t * pbSrcBegin = static_cast<std::int8_t *>(pSrcBegin);
   std::int8_t * pbSrcEnd   = static_cast<std::int8_t *>(pSrcEnd  );
   std::int8_t * pbDstBegin = static_cast<std::int8_t *>(pDstBegin);
   std::int8_t * pbDstEnd   = pbDstBegin + (pbSrcEnd - pbSrcBegin);
   if (pbDstBegin < pbSrcBegin && pbSrcBegin < pbDstEnd) {
      /*
      ┌─────────────────┐
      │ a - - B C D e f │
      ├─────────────────┤
      │ a B C D - - e f │
      └─────────────────┘
      */
      // Move the items from left to right (the block moves from right to left).
      std::size_t cbBeforeOverlap = static_cast<std::size_t>(pbSrcBegin - pbDstBegin);

      /* Move-construct the items that have an unused destination, then destruct them, so they can
      be overwritten by the next move if necessary. */
      type.move_construct(pbDstBegin, pbSrcBegin, pbSrcBegin + cbBeforeOverlap);
      type.destruct(pbSrcBegin, pbSrcBegin + cbBeforeOverlap);
      /*
      ┌─────────────────┐
      │ a B C - - D e f │
      └─────────────────┘
      */
      // Move-assign all the remaining items (the overlapping area) to shift them.
      type.move_construct(pbSrcBegin, pbSrcBegin + cbBeforeOverlap, pbSrcEnd);
      // Destruct their former locations.
      type.destruct(pbSrcBegin + cbBeforeOverlap, pbSrcEnd);
      /*
      ┌─────────────────┐
      │ a B C D - - e f │
      └─────────────────┘
      */
   } else if (pbSrcBegin < pbDstBegin && pbDstBegin < pbSrcEnd) {
      /*
      ┌─────────────────┐
      │ a B C D - - e f │
      ├─────────────────┤
      │ a - - B C D e f │
      └─────────────────┘
      */
      /* This situation is the mirror of the above, so the move must be done backwards, copying
      right to left (the block moves from left to right). */
      std::size_t cbAfterOverlap = static_cast<std::size_t>(pbDstEnd - pbSrcEnd);

      /* Move-construct the items that have an unused destination, then destruct them, so they can
      be overwritten by the next move if necessary. */
      type.move_construct(pbSrcEnd, pbSrcEnd - cbAfterOverlap, pbSrcEnd);
      type.destruct(pbDstEnd, pbSrcEnd);
      /*
      ┌─────────────────┐
      │ a B - - C D e f │
      └─────────────────┘
      */
      /* Move-assign backwards all the remaining items (the overlapping area) to shift them. This is
      slow, costing two function calls for each item. */
      std::int8_t * pbSrcItem = pbSrcEnd - cbAfterOverlap;
      std::int8_t * pbDstItem = pbSrcEnd;
      while (pbSrcItem > pbSrcBegin) {
         std::int8_t * pbSrcItemEnd = pbSrcItem;
         pbSrcItem -= type.size();
         pbDstItem -= type.size();
         type.move_construct(pbDstItem, pbSrcItem, pbSrcItemEnd);
         type.destruct(pbSrcItem, pbSrcItemEnd);
      }
      /*
      ┌─────────────────┐
      │ a - - B C D e f │
      └─────────────────┘
      */
   } else {
      type.move_construct(pbDstBegin, pbSrcBegin, pbSrcEnd);
   }
}

void raw_complex_vextr_impl::insert(
   type_void_adapter const & type, std::size_t ibOffset, void const * pInsert, std::size_t cbInsert,
   bool bMove
) {
   ABC_TRACE_FUNC(this, /*type, */ibOffset, pInsert, cbInsert, bMove);

   raw_vextr_transaction trn(this, false, cbInsert, 0);
   std::int8_t * pbOffset = begin<std::int8_t>() + ibOffset;
   void const * pInsertEnd = static_cast<std::int8_t const *>(pInsert) + cbInsert;
   std::int8_t * pbWorkInsertBegin = trn.work_array<std::int8_t>() + ibOffset;
   std::int8_t * pbWorkInsertEnd = static_cast<std::int8_t *>(pbWorkInsertBegin) + cbInsert;
   /* Regardless of whether we’re switching item arrays, the items beyond the insertion point must
   always be moved. */
   std::size_t cbTail = static_cast<std::size_t>(end<std::int8_t>() - pbOffset);
   if (cbTail) {
      overlapping_move_construct(type, pbWorkInsertEnd, pbOffset, end<std::int8_t>());
   }
   // Copy/move the new items over.
   if (bMove) {
      // No point in using try/catch here; we just assume that a move constructor won’t throw.
      type.move_construct(
         pbWorkInsertBegin, const_cast<void *>(pInsert), const_cast<void *>(pInsertEnd)
      );
   } else {
      try {
         type.copy_construct(pbWorkInsertBegin, pInsert, pInsertEnd);
      } catch (...) {
         // Undo the overlapping_move_construct() above.
         if (cbTail) {
            overlapping_move_construct(type, pbOffset, pbWorkInsertEnd, pbWorkInsertEnd + cbTail);
         }
         throw;
      }
   }
   /* Also move to the new array the items before the insertion point, otherwise we’ll lose them in
   the switch. */
   if (ibOffset && trn.will_replace_item_array()) {
      type.move_construct(trn.work_array<void>(), m_pBegin, pbOffset);
      type.destruct(m_pBegin, pbOffset);
   }
   trn.commit();
}

void raw_complex_vextr_impl::remove(
   type_void_adapter const & type, std::size_t ibOffset, std::size_t cbRemove
) {
   ABC_TRACE_FUNC(this, /*type, */ibOffset, cbRemove);

   raw_vextr_transaction trn(this, false, 0, cbRemove);
   std::int8_t * pbRemoveBegin = begin<std::int8_t>() + ibOffset;
   std::int8_t * pbRemoveEnd = pbRemoveBegin + cbRemove;
   // Destruct the items to be removed.
   type.destruct(pbRemoveBegin, pbRemoveEnd);
   /* The items beyond the last removed must be either copied to the new item array at cbRemove
   offset, or shifted to pbRemoveBegin in the old item array. */
   if (pbRemoveEnd < m_pEnd) {
      if (trn.will_replace_item_array()) {
         type.move_construct(trn.work_array<std::int8_t>() + ibOffset, pbRemoveEnd, m_pEnd);
         type.destruct(pbRemoveEnd, m_pEnd);
      } else {
         overlapping_move_construct(type, pbRemoveBegin, pbRemoveEnd, m_pEnd);
      }
   }
   /* Also move to the new array the items before the first deleted one, otherwise we’ll lose them
   in the switch. */
   if (ibOffset && trn.will_replace_item_array()) {
      type.move_construct(trn.work_array<void>(), m_pBegin, pbRemoveBegin);
      type.destruct(m_pBegin, pbRemoveBegin);
   }
   trn.commit();
}

void raw_complex_vextr_impl::set_capacity(
   type_void_adapter const & type, std::size_t cbMin, bool bPreserve
) {
   ABC_TRACE_FUNC(this, /*type, */cbMin, bPreserve);

   raw_vextr_transaction trn(this, false, cbMin);
   std::size_t cbOrig = size<std::int8_t>();
   if (trn.will_replace_item_array()) {
      /* Destruct every item from the array we’re abandoning, but first move-construct them if told
      to do so. */
      if (bPreserve) {
         type.move_construct(trn.work_array<void>(), m_pBegin, m_pEnd);
      }
      destruct_items(type);
      if (!bPreserve) {
         // We just destructed the items.
         cbOrig = 0;
      }
   }
   trn.commit();
   // The transaction changed the size to ciMin, which is incorrect.
   m_pEnd = begin<std::int8_t>() + cbOrig;
}

void raw_complex_vextr_impl::set_size(type_void_adapter const & type, std::size_t cb) {
   ABC_TRACE_FUNC(this, /*type, */cb);

   ABC_UNUSED_ARG(type);
   // TODO: implement this.
}

}}} //namespace abc::collections::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

void raw_trivial_vextr_impl::assign_concat(
   void const * p1Begin, void const * p1End, void const * p2Begin, void const * p2End
) {
   ABC_TRACE_FUNC(this, p1Begin, p1End, p2Begin, p2End);

   std::size_t cb1 = reinterpret_cast<std::size_t>(p1End) - reinterpret_cast<std::size_t>(p1Begin);
   std::size_t cb2 = reinterpret_cast<std::size_t>(p2End) - reinterpret_cast<std::size_t>(p2Begin);
   raw_vextr_transaction trn(this, true, cb1 + cb2);
   std::int8_t * pbWorkCopy = trn.work_array<std::int8_t>();
   if (cb1) {
      memory::copy(pbWorkCopy, static_cast<std::int8_t const *>(p1Begin), cb1);
      pbWorkCopy += cb1;
   }
   if (cb2) {
      memory::copy(pbWorkCopy, static_cast<std::int8_t const *>(p2Begin), cb2);
   }

   trn.commit();
}

void raw_trivial_vextr_impl::assign_move_desc_or_move_items(raw_trivial_vextr_impl && rtvi) {
   ABC_TRACE_FUNC(this/*, rtvi*/);

   if (rtvi.m_pBegin == m_pBegin) {
      return;
   }
   if (rtvi.m_bDynamic || !rtvi.m_bPrefixedItemArray) {
      // A dynamic or non-prefixed item array can be moved; just transfer its ownership.
      this->~raw_trivial_vextr_impl();
      assign_shallow(rtvi);
   } else {
      /* A static (prefixed) item array can’t be moved, so copy (same as move, for trivial items)
      its items instead. */
      assign_copy(rtvi.m_pBegin, rtvi.m_pEnd);
   }
   rtvi.assign_empty();
}

void raw_trivial_vextr_impl::assign_share_raw_or_copy_desc(raw_trivial_vextr_impl const & rtvi) {
   ABC_TRACE_FUNC(this/*, rtvi*/);

   /* This also checks that the source pointer (&rtvi) is safe to dereference, so the following code
   can proceed safely. */
   if (rtvi.m_pBegin == m_pBegin) {
      return;
   }
   if (rtvi.m_bPrefixedItemArray) {
      // Cannot share a prefixed item array.
      assign_copy(rtvi.m_pBegin, rtvi.m_pEnd);
   } else {
      // Discard the current contents.
      this->~raw_trivial_vextr_impl();
      // Share the source non-prefixed item array.
      assign_shallow(rtvi);
   }
}

void raw_trivial_vextr_impl::_insert_remove(
   std::size_t ibOffset, void const * pAdd, std::size_t cbAdd, std::size_t cbRemove
) {
   ABC_TRACE_FUNC(this, ibOffset, pAdd, cbAdd, cbRemove);

   raw_vextr_transaction trn(this, true, cbAdd, cbRemove);
   std::int8_t const * pbRemoveEnd = begin<std::int8_t>() + ibOffset + cbRemove;
   std::int8_t * pbWorkOffset = trn.work_array<std::int8_t>() + ibOffset;
   /* Regardless of an item array switch, the items beyond the insertion point (when adding) or the
   last removed (when removing) must always be moved/copied. */
   if (std::size_t cbTail = static_cast<std::size_t>(end<std::int8_t>() - pbRemoveEnd)) {
      memory::move(pbWorkOffset + cbAdd, pbRemoveEnd, cbTail);
   }
   if (pAdd) {
      // Copy the new items over.
      memory::copy(pbWorkOffset, static_cast<std::int8_t const *>(pAdd), cbAdd);
   }
   // Also copy to the new array the items before iOffset, otherwise we’ll lose them in the switch.
   if (ibOffset && trn.will_replace_item_array()) {
      memory::copy(trn.work_array<std::int8_t>(), begin<std::int8_t>(), ibOffset);
   }

   trn.commit();
}

void raw_trivial_vextr_impl::set_capacity(std::size_t cbMin, bool bPreserve) {
   ABC_TRACE_FUNC(this, cbMin, bPreserve);

   raw_vextr_transaction trn(this, true, cbMin);
   std::size_t cbOrig = size<std::int8_t>();
   if (trn.will_replace_item_array()) {
      if (bPreserve) {
         memory::copy(trn.work_array<std::int8_t>(), begin<std::int8_t>(), cbOrig);
      } else {
         // We’ll lose the item array when the transaction is commited.
         cbOrig = 0;
      }
   }
   trn.commit();
   // The transaction changed the size to ciMin, which is incorrect.
   m_pEnd = begin<std::int8_t>() + cbOrig;
}

void raw_trivial_vextr_impl::set_size(std::size_t cb) {
   ABC_TRACE_FUNC(this, cb);

   if (cb != size<std::int8_t>()) {
      if (cb > capacity<std::int8_t>()) {
         // Enlarge the item array.
         set_capacity(cb, true);
      }
      m_pEnd = begin<std::int8_t>() + cb;
   }
}

}}} //namespace abc::collections::detail
