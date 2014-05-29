/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abc.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_vextr_impl_base


namespace abc {

_raw_vextr_impl_base::transaction::transaction(
   size_t cbItem, _raw_vextr_impl_base * prvib, ptrdiff_t ciNew, ptrdiff_t ciDelta /*= 0*/
) :
   m_rvpd(0, false, false, false),
   m_prvib(prvib),
   // Calculate the new number of items, if expressed as a delta.
   m_bFree(false) {
   ABC_TRACE_FN((this, cbItem, prvib, ciNew, ciDelta));

   size_t ci;
   if (ciNew >= 0) {
      ci = size_t(ciNew);
   } else {
      ci = size_t(ptrdiff_t(m_prvib->size<int8_t>() / cbItem) + ciDelta);
   }
   if (ci == 0) {
      // Empty string/array: no need to use an item array.
      m_pBegin = m_pEnd = nullptr;
      // A read-only item array has no capacity. This was already set above.
      // m_rvpd.set_ciMax(0);
      return;
   }
   // This will return nullptr if there’s no static item array.
   m_pBegin = m_prvib->static_array_ptr<void>();
   if (m_pBegin) {
      size_t ciStaticMax(m_prvib->static_capacity());
      if (ci <= ciStaticMax) {
         // The static item array is large enough.
         m_rvpd.set_ciMax(ciStaticMax);
         goto set_pEnd;
      }
   }
   if (ci <= m_prvib->m_rvpd.get_ciMax()) {
      // The current item array is large enough, no need to change anything.
      m_pBegin = m_prvib->m_pBegin;
      m_rvpd = m_prvib->m_rvpd;
   } else {
      // The current item array (static or dynamic) is not large enough.

      // Calculate the total allocation size.
      // TODO: better algorithm.
      size_t ciMax(ci * _raw_vextr_impl_base::smc_iGrowthRate);
      // Apply some constraints.
      if (ciMax < _raw_vextr_impl_base::smc_cMinSlots) {
         ciMax = _raw_vextr_impl_base::smc_cMinSlots;
      } else {
         // Ensure that the lower bits are clear by rounding up.
         ciMax = _ABC__RAW_VEXTR_IMPL_BASE__ADJUST_ITEM_COUNT(ciMax);
      }
      // Check for overflow.
      // TODO: verify that the compiler doesn’t optimize this if away.
      if (ciMax <= ci) {
         // Theoretically, this could result in ciMax < ci; in practice it doesn’t matter because
         // the following memory allocation will fail for such sizes.
         ciMax = _raw_vextr_packed_data::smc_ciMaxMask;
      }

      size_t cb(cbItem * ciMax);
      if (m_prvib->m_rvpd.get_bDynamic()) {
         // Resize the current dynamically-allocated item array. Notice that the reallocation is
         // effective immediately, which means that m_prvib must be updated now – if no exceptions
         // are thrown, that is.
         m_prvib->m_pBegin = memory::_raw_realloc(m_prvib->m_pBegin, cb);
         m_prvib->m_pEnd = m_prvib->begin<int8_t>() + cbItem * ci;
         m_prvib->m_rvpd.set_ciMax(ciMax);
         m_pBegin = m_prvib->m_pBegin;
      } else {
         // Allocate a new item array.
         m_pBegin = memory::_raw_alloc(cb);
         m_bFree = true;
      }
      m_rvpd.set(ciMax, false, true);
   }
set_pEnd:
   m_pEnd = static_cast<int8_t *>(m_pBegin) + cbItem * ci;
}


void _raw_vextr_impl_base::transaction::commit() {
   ABC_TRACE_FN((this));

   // If we are abandoning the old item array, proceed to destruct it if necessary.
   if (m_pBegin != m_prvib->m_pBegin) {
      m_prvib->~_raw_vextr_impl_base();
      m_prvib->m_pBegin = m_pBegin;
   }
   // Update the target object.
   m_prvib->m_pEnd = m_pEnd;
   m_prvib->m_rvpd = m_rvpd;
   // This object no longer owns the temporary item array.
   m_bFree = false;
   // TODO: consider releasing some memory from an oversized dynamically-allocated item array.
}


_raw_vextr_impl_base::_raw_vextr_impl_base(size_t ciStaticMax) :
   m_pBegin(nullptr),
   m_pEnd(nullptr),
   m_rvpd(0, false, false, ciStaticMax > 0) {
   ABC_TRACE_FN((this, ciStaticMax));

   if (ciStaticMax) {
      // Assign ciStaticMax to the static item array that follows *this.
      _raw_vextr_impl_base_with_static_item_array * prvibwsia(
         static_cast<_raw_vextr_impl_base_with_static_item_array *>(this)
      );
      prvibwsia->m_ciStaticMax = ciStaticMax;
   }
}


void const * _raw_vextr_impl_base::translate_offset(intptr_t ib) const {
   ABC_TRACE_FN((this, ib));

   int8_t const * pb(ib >= 0 ? begin<int8_t>() : end<int8_t>());
   pb += ib;
   if (begin<int8_t>() <= pb && pb < end<int8_t>()) {
      return pb;
   }
   // TODO: use the index, not the offset.
   ABC_THROW(index_error, (ib));
}


std::pair<void const *, void const *> _raw_vextr_impl_base::translate_byte_range(
   intptr_t ibBegin, intptr_t ibEnd
) const {
   ABC_TRACE_FN((this, ibBegin, ibEnd));

   intptr_t cb(intptr_t(size<int8_t>()));
   if (ibBegin < 0) {
      ibBegin += cb;
      if (ibBegin < 0) {
         // If the start of the interval is still negative, clip it to 0.
         ibBegin = 0;
      }
   }
   if (ibEnd < 0) {
      ibEnd += cb;
      if (ibEnd < 0) {
         // If the end of the interval is still negative, clip it to 0.
         ibEnd = 0;
      }
   } else if (ibEnd > cb) {
      // If the end of the interval is beyond the end of the item array, clip it to the latter.
      ibEnd = cb;
   }
   // If the interval is empty, return [0, 0) .
   if (ibBegin >= ibEnd) {
      return std::pair<void const *, void const *>(nullptr, nullptr);
   }
   // Return the constructed interval.
   return std::pair<void const *, void const *>(begin<int8_t>() + ibBegin, begin<int8_t>() + ibEnd);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_complex_vextr_impl


namespace abc {

void _raw_complex_vextr_impl::assign_concat(
   type_void_adapter const & type,
   void const * p1Begin, void const * p1End, bool bMove1,
   void const * p2Begin, void const * p2End, bool bMove2
) {
   ABC_TRACE_FN((this, /*type, */p1Begin, p1End, bMove1, p2Begin, p2End, bMove2));

   size_t cb1(size_t(static_cast<int8_t const *>(p1End) - static_cast<int8_t const *>(p1Begin)));
   size_t cb2(size_t(static_cast<int8_t const *>(p2End) - static_cast<int8_t const *>(p2Begin)));
   transaction trn(type.cb, this, ptrdiff_t((cb1 + cb2) / type.cb));
   size_t cbOrig(size<int8_t>());
   std::unique_ptr<int8_t[]> pbBackup;
   int8_t * pbWorkCopy(trn.work_array<int8_t>());
   if (cb1 || cb2) {
      // If we’re going to overwrite the old item array, move the items to a backup array, so we can
      // restore them in case of exceptions thrown while constructing the new objects.
      if (cbOrig && !trn.will_replace_item_array()) {
         pbBackup.reset(new int8_t[cbOrig]);
         type.move_constr(pbBackup.get(), m_pBegin, m_pEnd);
         destruct_items(type);
      }
      try {
         if (cb1) {
            if (bMove1) {
               type.move_constr(pbWorkCopy, const_cast<void *>(p1Begin), const_cast<void *>(p1End));
            } else {
               type.copy_constr(pbWorkCopy, p1Begin, p1End);
            }
            pbWorkCopy += cb1;
         }
         if (cb2) {
            if (bMove2) {
               type.move_constr(pbWorkCopy, const_cast<void *>(p2Begin), const_cast<void *>(p2End));
            } else {
               type.copy_constr(pbWorkCopy, p2Begin, p2End);
            }
         }
      } catch (...) {
         // If we already constructed the copies of p1, destruct them.
         int8_t * pbWorkCopy1Begin(trn.work_array<int8_t>());
         if (pbWorkCopy > pbWorkCopy1Begin) {
            int8_t * pbWorkCopy1End(pbWorkCopy1Begin + cb1);
            if (bMove1) {
               // If we moved them from p1, don’t forget to move them back. Of course this means
               // that we first have to destruct p1’s items and re-construct them.
               type.destruct(p1Begin, p1End);
               type.move_constr(const_cast<void *>(p1Begin), pbWorkCopy1Begin, pbWorkCopy1End);
            }
            type.destruct(pbWorkCopy1Begin, pbWorkCopy1End);
         }
         // If earlier we decided to make a backup, restore it now, then destruct it.
         if (pbBackup) {
            type.move_constr(m_pBegin, pbBackup.get(), pbBackup.get() + cbOrig);
            type.destruct(pbBackup.get(), pbBackup.get() + cbOrig);
         }
         throw;
      }
   }
   if (cbOrig) {
      // If we made a backup, it also means that now this is the only copy of the original items, so
      // we must use it to destruct them, instead of m_pBegin/End.
      if (pbBackup) {
         type.destruct(pbBackup.get(), pbBackup.get() + cbOrig);
      } else {
         destruct_items(type);
      }
   }
   trn.commit();
}


void _raw_complex_vextr_impl::assign_move(
   type_void_adapter const & type, _raw_complex_vextr_impl && rcvi
) {
   ABC_TRACE_FN((this/*, type, rcvi*/));

   if (rcvi.m_pBegin == m_pBegin) {
      return;
   }
   ABC_ASSERT(rcvi.m_rvpd.get_bDynamic(), SL("cannot move a static item array"));
   // Discard the current contents.
   destruct_items(type);
   this->~_raw_complex_vextr_impl();
   // Take over the dynamic array.
   m_pBegin = rcvi.m_pBegin;
   m_pEnd = rcvi.m_pEnd;
   m_rvpd = rcvi.m_rvpd;
   // And now empty the source.
   rcvi.assign_empty();
}


void _raw_complex_vextr_impl::assign_move_dynamic_or_move_items(
   type_void_adapter const & type, _raw_complex_vextr_impl && rcvi
) {
   if (rcvi.m_pBegin == m_pBegin) {
      return;
   }
   if (rcvi.m_rvpd.get_bDynamic()) {
      assign_move(type, std::move(rcvi));
   } else {
      // Can’t move the item array, so move the items instead.
      {
         size_t ciSrc(rcvi.size<int8_t>() / type.cb);
         transaction trn(type.cb, this, ptrdiff_t(ciSrc));
         // Assume that destructing the current items first and then moving in rcvi’s items is an
         // exception-safe approach.
         if (size<int8_t>()) {
            destruct_items(type);
         }
         // Now that the current items have been destructed, move-construct the new items.
         if (ciSrc) {
            type.move_constr(trn.work_array<void>(), rcvi.m_pBegin, rcvi.m_pEnd);
         }
         trn.commit();
      }
      // And now empty the source.
      rcvi.destruct_items(type);
      rcvi.assign_empty();
   }
}


/** Safely moves a range of items to another position in the same array, carefully moving items in
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
static void overlapping_move_constr(
   type_void_adapter const & type, void * pDstBegin, void * pSrcBegin, void * pSrcEnd
) {
   if (pDstBegin == pSrcBegin) {
      return;
   }
   int8_t * pbSrcBegin(static_cast<int8_t *>(pSrcBegin));
   int8_t * pbSrcEnd  (static_cast<int8_t *>(pSrcEnd  ));
   int8_t * pbDstBegin(static_cast<int8_t *>(pDstBegin));
   int8_t * pbDstEnd  (pbDstBegin + (pbSrcEnd - pbSrcBegin));
   if (pbDstBegin < pbSrcBegin && pbSrcBegin < pbDstEnd) {
      // ┌─────────────────┐
      // │ a - - B C D e f │
      // ├─────────────────┤
      // │ a B C D - - e f │
      // └─────────────────┘
      //
      // Move the items from left to right (the block moves from right to left).
      size_t cbBeforeOverlap(size_t(pbSrcBegin - pbDstBegin));

      // Move-construct the items that have an unused destination, then destruct them, so they can
      // be overwritten by the next move if necessary.
      type.move_constr(pbDstBegin, pbSrcBegin, pbSrcBegin + cbBeforeOverlap);
      type.destruct(pbSrcBegin, pbSrcBegin + cbBeforeOverlap);
      // ┌─────────────────┐
      // │ a B C - - D e f │
      // └─────────────────┘

      // Move-assign all the remaining items (the overlapping area) to shift them.
      type.move_constr(pbSrcBegin, pbSrcBegin + cbBeforeOverlap, pbSrcEnd);
      // Destruct their former locations.
      type.destruct(pbSrcBegin + cbBeforeOverlap, pbSrcEnd);
      // ┌─────────────────┐
      // │ a B C D - - e f │
      // └─────────────────┘
   } else if (pbSrcBegin < pbDstBegin && pbDstBegin < pbSrcEnd) {
      // ┌─────────────────┐
      // │ a B C D - - e f │
      // ├─────────────────┤
      // │ a - - B C D e f │
      // └─────────────────┘
      //
      // This situation is the mirror of the above, so the move must be done backwards, copying
      // right to left (the block moves from left to right).
      size_t cbAfterOverlap(size_t(pbDstEnd - pbSrcEnd));

      // Move-construct the items that have an unused destination, then destruct them, so they can
      // be overwritten by the next move if necessary.
      type.move_constr(pbSrcEnd, pbSrcEnd - cbAfterOverlap, pbSrcEnd);
      type.destruct(pbDstEnd, pbSrcEnd);
      // ┌─────────────────┐
      // │ a B - - C D e f │
      // └─────────────────┘

      // Move-assign backwards all the remaining items (the overlapping area) to shift them.
      // This is slow, costing two function calls for each item.
      int8_t * pbSrcItem(pbSrcEnd - cbAfterOverlap);
      int8_t * pbDstItem(pbSrcEnd);
      while (pbSrcItem > pbSrcBegin) {
         int8_t * pbSrcItemEnd(pbSrcItem);
         pbSrcItem -= type.cb;
         pbDstItem -= type.cb;
         type.move_constr(pbDstItem, pbSrcItem, pbSrcItemEnd);
         type.destruct(pbSrcItem, pbSrcItemEnd);
      }
      // ┌─────────────────┐
      // │ a - - B C D e f │
      // └─────────────────┘
   } else {
      type.move_constr(pbDstBegin, pbSrcBegin, pbSrcEnd);
   }
}


void _raw_complex_vextr_impl::_insert(
   type_void_adapter const & type, size_t iOffset, void const * p, size_t ci, bool bMove
) {
   ABC_TRACE_FN((this, /*type, */iOffset, p, ci, bMove));

   transaction trn(type.cb, this, -1, ptrdiff_t(ci));
   size_t ibOffset(type.cb * iOffset);
   int8_t * pbOffset(trn.work_array<int8_t>() + ibOffset);
   // Regardless of whether we’re switching item arrays, the items beyond the insertion point must
   // always be moved.
   size_t ciTail(size<int8_t>() / type.cb - iOffset);
   if (ciTail) {
      overlapping_move_constr(
         type, pbOffset + type.cb * ci, begin<int8_t>() + ibOffset, end<int8_t>()
      );
   }
   // Copy/move the new items over.
   if (bMove) {
      // No point in using try/catch here; we just assume that a move constructor won’t throw.
      type.move_constr(
         pbOffset,
         const_cast<void *>(p),
         static_cast<int8_t *>(const_cast<void *>(p)) + type.cb * ci
      );
   } else {
      try {
         type.copy_constr(pbOffset, p, static_cast<int8_t const *>(p) + type.cb * ci);
      } catch (...) {
         // Undo the overlapping_move_constr() above.
         if (ciTail) {
            overlapping_move_constr(
               type, begin<int8_t>() + ibOffset,
               pbOffset + type.cb * ci, pbOffset + type.cb * (ci + ciTail)
            );
         }
         throw;
      }
   }
   // Also move to the new array the items before the insertion point, otherwise we’ll lose them in
   // the switch.
   if (iOffset && trn.will_replace_item_array()) {
      type.move_constr(trn.work_array<void>(), m_pBegin, begin<int8_t>() + ibOffset);
      type.destruct(m_pBegin, begin<int8_t>() + ibOffset);
   }
   trn.commit();
}


void _raw_complex_vextr_impl::_remove(
   type_void_adapter const & type, uintptr_t iOffset, size_t ciRemove
) {
   ABC_TRACE_FN((this, /*type, */iOffset, ciRemove));

   transaction trn(type.cb, this, -1, -ptrdiff_t(ciRemove));
   size_t cbOffset(type.cb * iOffset);
   // Destruct the items to be removed.
   type.destruct(begin<int8_t>() + cbOffset, begin<int8_t>() + cbOffset + type.cb * ciRemove);
   // The items beyond the last removed must be either copied to the new item array at ciRemove
   // offset, or shifted closer to the start.
   if (size_t ciTail = size<int8_t>() / type.cb - (iOffset + ciRemove)) {
      int8_t * pbWorkTail(trn.work_array<int8_t>() + cbOffset),
             * pbOrigTail(begin<int8_t>() + cbOffset + type.cb * ciRemove);
      if (trn.will_replace_item_array()) {
         type.move_constr(pbWorkTail, pbOrigTail, pbOrigTail + type.cb * ciTail);
         type.destruct(pbOrigTail, pbOrigTail + type.cb * ciTail);
      } else {
         overlapping_move_constr(type, pbWorkTail, pbOrigTail, pbOrigTail + type.cb * ciTail);
      }
   }
   // Also move to the new array the items before the first deleted one, otherwise we’ll lose them
   // in the switch.
   if (iOffset && trn.will_replace_item_array()) {
      type.move_constr(trn.work_array<void>(), m_pBegin, begin<int8_t>() + cbOffset);
      type.destruct(m_pBegin, begin<int8_t>() + cbOffset);
   }
   trn.commit();
}


void _raw_complex_vextr_impl::remove_range(
   type_void_adapter const & type, intptr_t iBegin, intptr_t iEnd
) {
   ABC_TRACE_FN((this, /*type, */iBegin, iEnd));

   auto range(translate_byte_range(ptrdiff_t(type.cb) * iBegin, ptrdiff_t(type.cb) * iEnd));
   size_t cbRemove(size_t(
      static_cast<int8_t const *>(range.second) - static_cast<int8_t const *>(range.first)
   ));
   if (cbRemove) {
      _remove(
         type, size_t(static_cast<int8_t const *>(range.first) - begin<int8_t>()) / type.cb,
         cbRemove / type.cb
      );
   }
}


void _raw_complex_vextr_impl::set_capacity(
   type_void_adapter const & type, size_t ciMin, bool bPreserve
) {
   ABC_TRACE_FN((this, /*type, */ciMin, bPreserve));

   size_t cbOrig(size<int8_t>());
   transaction trn(type.cb, this, ptrdiff_t(ciMin));
   if (trn.will_replace_item_array()) {
      // Destruct every item from the array we’re abandoning, but first move-construct them if
      // told to do so.
      if (bPreserve) {
         type.move_constr(trn.work_array<void>(), m_pBegin, m_pEnd);
      }
      destruct_items(type);
      if (!bPreserve) {
         // We just destructed the items.
         cbOrig = 0;
      }
   }
   trn.commit();
   // The transaction changed the size to ciMin, which is incorrect.
   m_pEnd = begin<int8_t>() + cbOrig;
}


void _raw_complex_vextr_impl::set_size(type_void_adapter const & type, size_t ci) {
   ABC_TRACE_FN((this, /*type, */ci));

   size_t cb(type.cb * ci);
   if (cb != size<int8_t>()) {
      if (ci > capacity()) {
         // Enlarge the item array.
         set_capacity(type, ci, true);
      }
      m_pEnd = begin<int8_t>() + cb;
   }
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_trivial_vextr_impl


namespace abc {

void _raw_trivial_vextr_impl::assign_concat(
   size_t cbItem, void const * p1Begin, void const * p1End, void const * p2Begin, void const * p2End
) {
   ABC_TRACE_FN((this, cbItem, p1Begin, p1End, p2Begin, p2End));

   size_t cb1(size_t(static_cast<int8_t const *>(p1End) - static_cast<int8_t const *>(p1Begin)));
   size_t cb2(size_t(static_cast<int8_t const *>(p2End) - static_cast<int8_t const *>(p2Begin)));
   transaction trn(cbItem, this, ptrdiff_t((cb1 + cb2) / cbItem), 0);
   int8_t * pbWorkCopy(trn.work_array<int8_t>());
   if (cb1) {
      memory::copy(pbWorkCopy, static_cast<int8_t const *>(p1Begin), cb1);
      pbWorkCopy += cb1;
   }
   if (cb2) {
      memory::copy(pbWorkCopy, static_cast<int8_t const *>(p2Begin), cb2);
   }

   trn.commit();
}


void _raw_trivial_vextr_impl::assign_move_dynamic_or_move_items(
   size_t cbItem, _raw_trivial_vextr_impl && rtvi
) {
   if (rtvi.m_pBegin == m_pBegin) {
      return;
   }
   if (rtvi.m_rvpd.get_bDynamic()) {
      assign_move(std::move(rtvi));
   } else {
      // Can’t move, so copy instead.
      assign_copy(cbItem, rtvi.m_pBegin, rtvi.m_pEnd);
      // And now empty the source.
      rtvi.assign_empty();
   }
}


void _raw_trivial_vextr_impl::_assign_share(_raw_trivial_vextr_impl const & rtvi) {
   ABC_TRACE_FN((this/*, rtvi*/));

   ABC_ASSERT(rtvi.m_pBegin != m_pBegin, SL("cannot assign from self"));
   ABC_ASSERT(
      rtvi.is_item_array_readonly() || rtvi.m_rvpd.get_bDynamic(),
      SL("can only share read-only or dynamic item arrays (the latter only as part of a move)")
   );
   // Discard the current contents.
   this->~_raw_trivial_vextr_impl();
   // Take over the dynamic array.
   m_pBegin = rtvi.m_pBegin;
   m_pEnd = rtvi.m_pEnd;
   m_rvpd = rtvi.m_rvpd;
}


void _raw_trivial_vextr_impl::_insert_or_remove(
   size_t cbItem, uintptr_t iOffset, void const * pAdd, size_t ciAdd, size_t ciRemove
) {
   ABC_TRACE_FN((this, cbItem, iOffset, pAdd, ciAdd, ciRemove));

   ABC_ASSERT(ciAdd || ciRemove, SL("must have items being added or removed"));
   transaction trn(cbItem, this, -1, ptrdiff_t(ciAdd) - ptrdiff_t(ciRemove));
   size_t cbOffset(cbItem * iOffset);
   // Regardless of an item array switch, the items beyond the insertion point (when adding) or the
   // last removed (when removing) must always be moved/copied.
   if (size_t ciTail = size<int8_t>() / cbItem - (iOffset + ciRemove)) {
      memory::move(
         trn.work_array<int8_t>() + cbOffset + cbItem * ciAdd,
         begin<int8_t>() + cbOffset + cbItem * ciRemove,
         cbItem * ciTail
      );
   }
   if (ciAdd) {
      // Copy the new items over.
      memory::copy(
         trn.work_array<int8_t>() + cbOffset, static_cast<int8_t const *>(pAdd), cbItem * ciAdd
      );
   }
   // Also copy to the new array the items before iOffset, otherwise we’ll lose them in the switch.
   if (cbOffset && trn.will_replace_item_array()) {
      memory::copy(trn.work_array<int8_t>(), begin<int8_t>(), cbOffset);
   }

   trn.commit();
}


void _raw_trivial_vextr_impl::remove_range(size_t cbItem, intptr_t iBegin, intptr_t iEnd) {
   ABC_TRACE_FN((this, cbItem, iBegin, iEnd));

   auto range(translate_byte_range(ptrdiff_t(cbItem) * iBegin, ptrdiff_t(cbItem) * iEnd));
   size_t cbRemove(size_t(
      static_cast<int8_t const *>(range.second) - static_cast<int8_t const *>(range.first)
   ));
   if (cbRemove) {
      _insert_or_remove(
         cbItem, size_t(static_cast<int8_t const *>(range.first) - begin<int8_t>()) / cbItem,
         nullptr, 0, cbRemove / cbItem
      );
   }
}


void _raw_trivial_vextr_impl::set_capacity(size_t cbItem, size_t ciMin, bool bPreserve) {
   ABC_TRACE_FN((this, cbItem, ciMin, bPreserve));

   size_t cbOrig(size<int8_t>());
   transaction trn(cbItem, this, ptrdiff_t(ciMin), 0);
   if (trn.will_replace_item_array()) {
      if (bPreserve) {
         memory::copy(trn.work_array<int8_t>(), begin<int8_t>(), cbOrig);
      } else {
         // We’ll lose the item array when the transaction is commited.
         cbOrig = 0;
      }
   }
   trn.commit();
   // The transaction changed the size to ciMin, which is incorrect.
   m_pEnd = begin<int8_t>() + cbOrig;
}


void _raw_trivial_vextr_impl::set_size(size_t cbItem, size_t ci) {
   ABC_TRACE_FN((this, cbItem, ci));

   size_t cb(cbItem * ci);
   if (cb != size<int8_t>()) {
      if (ci > capacity()) {
         // Enlarge the item array.
         set_capacity(cbItem, ci, true);
      }
      m_pEnd = begin<int8_t>() + cb;
   }
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

