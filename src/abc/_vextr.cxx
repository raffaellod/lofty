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

char_t const _raw_vextr_impl_base::smc_chNUL(CL('\0'));


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
      ci = size_t(ptrdiff_t(m_prvib->size(cbItem)) + ciDelta);
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


uintptr_t _raw_vextr_impl_base::adjust_and_validate_index(size_t cbItem, intptr_t i) const {
   ABC_TRACE_FN((this, cbItem, i));

   intptr_t ci((intptr_t(size(cbItem))));
   if (i < 0) {
      i += ci;
   }
   if (0 > i || i >= ci) {
      ABC_THROW(index_error, (i));
   }
   return uintptr_t(i);
}


std::pair<uintptr_t, uintptr_t> _raw_vextr_impl_base::adjust_and_validate_range(
   size_t cbItem, intptr_t iBegin, intptr_t iEnd
) const {
   ABC_TRACE_FN((this, cbItem, iBegin, iEnd));

   intptr_t ci((intptr_t(size(cbItem))));
   if (iBegin < 0) {
      iBegin += ci;
      if (iBegin < 0) {
         // If the start of the interval is still negative, clip it to 0.
         iBegin = 0;
      }
   } else if (iBegin >= ci) {
      // If the interval begins beyond the end of the item array, return an empty interval.
      return std::pair<uintptr_t, uintptr_t>(0, 0);
   }
   if (iEnd < 0) {
      iEnd += ci;
      if (iEnd < 0) {
         // If the end of the interval is still negative, clip it to 0.
         iEnd = 0;
      }
   } else if (iEnd > ci) {
      // If the end of the interval is beyond the end of the item array, clip it to the latter.
      iEnd = ci;
   }
   // If the interval is empty, return [0, 0) .
   if (iBegin >= iEnd) {
      return std::pair<uintptr_t, uintptr_t>(0, 0);
   }
   // Return the constructed interval.
   return std::pair<uintptr_t, uintptr_t>(uintptr_t(iBegin), uintptr_t(iEnd));
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_complex_vextr_impl


namespace abc {

void _raw_complex_vextr_impl::assign_copy(
   type_void_adapter const & type, void const * p, size_t ci
) {
   ABC_TRACE_FN((this, /*type, */p, ci));

   transaction trn(type.cb, this, ptrdiff_t(ci));
   size_t cbOrig(size(1));
   size_t ciOrig(size(type.cb));
   // We’re going to overwrite the old item array, so move the items to a backup array, so we can
   // restore them in case of exceptions thrown while copy-constructing the new objects.
   std::unique_ptr<int8_t[]> pbBackup;
   if (ci) {
      if (cbOrig && !trn.will_replace_item_array()) {
         pbBackup.reset(new int8_t[cbOrig]);
         type.move_constr(pbBackup.get(), m_pBegin, ciOrig);
         destruct_items(type);
      }
      try {
         type.copy_constr(trn.work_array<void>(), p, static_cast<int8_t const *>(p) + type.cb * ci);
      } catch (...) {
         // If earlier we decided to make a backup, restore it now, then destruct it.
         if (pbBackup) {
            type.move_constr(m_pBegin, pbBackup.get(), ciOrig);
            type.destruct(pbBackup.get(), pbBackup.get() + cbOrig);
         }
         throw;
      }
   }
   if (cbOrig) {
      // If we made a backup, it also means that now this is the only copy of the original items,
      // so we must use it to destruct them, instead of m_pBegin/End.
      if (pbBackup) {
         type.destruct(pbBackup.get(), pbBackup.get() + cbOrig);
      } else {
         destruct_items(type);
      }
   }
   trn.commit();
}


void _raw_complex_vextr_impl::assign_concat(
   type_void_adapter const & type,
   void const * p1, size_t ci1, bool bMove1, void const * p2, size_t ci2, bool bMove2
) {
   ABC_TRACE_FN((this, /*type, */p1, ci1, bMove1, p2, ci2, bMove2));

   transaction trn(type.cb, this, ptrdiff_t(ci1 + ci2));
   size_t cbOrig(size(1));
   size_t ciOrig(size(type.cb));
   std::unique_ptr<int8_t[]> pbBackup;
   int8_t * pbWorkCopy(trn.work_array<int8_t>());
   if (ci1 || ci2) {
      // If we’re going to overwrite the old item array, move the items to a backup array, so we can
      // restore them in case of exceptions thrown while constructing the new objects.
      if (ciOrig && !trn.will_replace_item_array()) {
         pbBackup.reset(new int8_t[cbOrig]);
         type.move_constr(pbBackup.get(), m_pBegin, ciOrig);
         destruct_items(type);
      }
      try {
         if (ci1) {
            if (bMove1) {
               type.move_constr(pbWorkCopy, const_cast<void *>(p1), ci1);
            } else {
               type.copy_constr(pbWorkCopy, p1, static_cast<int8_t const *>(p1) + type.cb * ci1);
            }
            pbWorkCopy += type.cb * ci1;
         }
         if (ci2) {
            if (bMove2) {
               type.move_constr(pbWorkCopy, const_cast<void *>(p2), ci2);
            } else {
               type.copy_constr(pbWorkCopy, p2, static_cast<int8_t const *>(p2) + type.cb * ci2);
            }
         }
      } catch (...) {
         // If we already constructed the copies of p1, destruct them.
         int8_t * pbWorkCopyBegin(trn.work_array<int8_t>());
         if (pbWorkCopy > pbWorkCopyBegin) {
            if (bMove1) {
               // If we moved them from p1, don’t forget to move them back. Of course this means
               // that we first have to destruct p1’s items and re-construct them.
               type.destruct(p1, static_cast<int8_t const *>(p1) + type.cb * ci1);
               type.move_constr(const_cast<void *>(p1), pbWorkCopyBegin, ci1);
            }
            type.destruct(pbWorkCopyBegin, pbWorkCopyBegin + type.cb * ci1);
         }
         // If earlier we decided to make a backup, restore it now, then destruct it.
         if (pbBackup) {
            type.move_constr(m_pBegin, pbBackup.get(), ciOrig);
            type.destruct(pbBackup.get(), pbBackup.get() + cbOrig);
         }
         throw;
      }
   }
   if (ciOrig) {
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
         size_t ciSrc(rcvi.size(type.cb)), ciOrig(size(type.cb));
         transaction trn(type.cb, this, ptrdiff_t(ciSrc));
         // Assume that destructing the current items first and then moving in rcvi’s items is an
         // exception-safe approach.
         if (ciOrig) {
            destruct_items(type);
         }
         // Now that the current items have been destructed, move-construct the new items.
         if (ciSrc) {
            type.move_constr(trn.work_array<void>(), rcvi.m_pBegin, ciSrc);
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

TODO: comment signature.
*/
static void overlapping_move_constr(
   type_void_adapter const & type, void * pDst, void * pSrc, size_t ci
) {
   if (pDst == pSrc) {
      return;
   }
   int8_t * pbSrc(static_cast<int8_t *>(pSrc));
   int8_t * pbDst(static_cast<int8_t *>(pDst));
   int8_t * pbSrcEnd(pbSrc + type.cb * ci);
   int8_t * pbDstEnd(pbDst + type.cb * ci);
   if (pbDst < pbSrc && pbSrc < pbDstEnd) {
      // ┌─────────────────┐
      // │ a - - B C D e f │
      // ├─────────────────┤
      // │ a B C D - - e f │
      // └─────────────────┘
      //
      // Move the items from left to right (the block moves from right to left).

      int8_t * pbOverlapBegin(pbSrc);
      int8_t * pbOverlapEnd(pbDstEnd);
      size_t ciBeforeOverlap(size_t(pbOverlapBegin - pbDst) / type.cb);
      size_t ciOverlapping(size_t(pbOverlapEnd - pbOverlapBegin) / type.cb);
      size_t ciAfterOverlap(size_t(pbSrcEnd - pbOverlapEnd) / type.cb);

      if (ciBeforeOverlap) {
         // First, move-construct the items that don’t overlap.
         type.move_constr(pbDst, pbSrc, ciBeforeOverlap);
      }
      // ┌─────────────────┐
      // │ a B C b c D e f │ (lowercase b and c indicate the moved-out items)
      // └─────────────────┘
      if (ciOverlapping) {
         // Second, move-assign all the items in the overlapping area to shift them.
         type.destruct(pbOverlapBegin, pbOverlapEnd);
         type.move_constr(pbOverlapBegin, pbOverlapEnd, ciOverlapping);
      }
      // ┌─────────────────┐
      // │ a B C D c d e f │
      // └─────────────────┘
      if (ciAfterOverlap) {
         // Third, destruct the items that have no replacement and have just been moved out.
         type.destruct(pbOverlapEnd, pbSrcEnd);
      }
   } else if (pbSrc < pbDst && pbDst < pbSrcEnd) {
      // ┌─────────────────┐
      // │ a B C D - - e f │
      // ├─────────────────┤
      // │ a - - B C D e f │
      // └─────────────────┘
      //
      // This situation is the mirror of the above, so the move must be done backwards, copying
      // right to left (the block moves from left to right).

      int8_t * pbOverlapBegin(pbDst);
      int8_t * pbOverlapEnd(pbSrcEnd);
      size_t ciBeforeOverlap(size_t(pbOverlapBegin - pbSrc) / type.cb);
      size_t ciOverlapping(size_t(pbOverlapEnd - pbOverlapBegin) / type.cb);
      size_t ciAfterOverlap(size_t(pbDstEnd - pbOverlapEnd) / type.cb);

      if (ciAfterOverlap) {
         // First, move-construct the items on the right of the overlapping area.
         type.move_constr(pbOverlapEnd, pbSrcEnd - (pbDstEnd - pbOverlapEnd), ciAfterOverlap);
      }
      // ┌─────────────────┐
      // │ a B c d C D e f │ (lowercase c and d indicate the moved-out items)
      // └─────────────────┘
      if (ciOverlapping) {
         // Second, move-assign backwards all the items in the overlapping area to shift them.
         int8_t * pbSrcItem(pbSrcEnd - (pbDstEnd - pbOverlapEnd));
         int8_t * pbDstItem(pbOverlapEnd);
         while (pbSrcItem > pbSrc) {
            pbSrcItem -= type.cb;
            pbDstItem -= type.cb;
            type.destruct(pbDstItem, pbDstItem + type.cb);
            type.move_constr(pbDstItem, pbSrcItem, 1);
         }
      }
      // ┌─────────────────┐
      // │ a b c B C D e f │
      // └─────────────────┘
      if (ciBeforeOverlap) {
         // Third, destruct the items that have no replacement and have just been moved out.
         type.destruct(pbSrc, pbOverlapBegin);
      }
   } else {
      type.move_constr(pbDst, pbSrc, ci);
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
   size_t ciTail(size(type.cb) - iOffset);
   if (ciTail) {
      overlapping_move_constr(type, pbOffset + type.cb * ci, begin<int8_t>() + ibOffset, ciTail);
   }
   // Copy/move the new items over.
   if (bMove) {
      // No point in using try/catch here; we just assume that a move constructor won’t throw.
      type.move_constr(pbOffset, const_cast<void *>(p), ci);
   } else {
      try {
         type.copy_constr(pbOffset, p, static_cast<int8_t const *>(p) + type.cb * ci);
      } catch (...) {
         // Undo the overlapping_move_constr() above.
         if (ciTail) {
            overlapping_move_constr(
               type, begin<int8_t>() + ibOffset, pbOffset + type.cb * ci, ciTail
            );
         }
         throw;
      }
   }
   // Also move to the new array the items before the insertion point, otherwise we’ll lose them in
   // the switch.
   if (iOffset && trn.will_replace_item_array()) {
      type.move_constr(trn.work_array<void>(), m_pBegin, iOffset);
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
   if (size_t ciTail = size(type.cb) - (iOffset + ciRemove)) {
      int8_t * pbWorkTail(trn.work_array<int8_t>() + cbOffset),
             * pbOrigTail(begin<int8_t>() + cbOffset + type.cb * ciRemove);
      if (trn.will_replace_item_array()) {
         type.move_constr(pbWorkTail, pbOrigTail, ciTail);
         type.destruct(pbOrigTail, pbOrigTail + type.cb * ciTail);
      } else {
         overlapping_move_constr(type, pbWorkTail, pbOrigTail, ciTail);
      }
   }
   // Also move to the new array the items before the first deleted one, otherwise we’ll lose them
   // in the switch.
   if (iOffset && trn.will_replace_item_array()) {
      type.move_constr(trn.work_array<void>(), m_pBegin, iOffset);
      type.destruct(m_pBegin, begin<int8_t>() + cbOffset);
   }
   trn.commit();
}


void _raw_complex_vextr_impl::remove_range(
   type_void_adapter const & type, intptr_t iBegin, intptr_t iEnd
) {
   ABC_TRACE_FN((this, /*type, */iBegin, iEnd));

   auto range(adjust_and_validate_range(type.cb, iBegin, iEnd));
   size_t ciRemove(range.second - range.first);
   if (ciRemove) {
      _remove(type, range.first, ciRemove);
   }
}


void _raw_complex_vextr_impl::set_capacity(
   type_void_adapter const & type, size_t ciMin, bool bPreserve
) {
   ABC_TRACE_FN((this, /*type, */ciMin, bPreserve));

   size_t ciOrig(size(type.cb));
   transaction trn(type.cb, this, ptrdiff_t(ciMin));
   if (trn.will_replace_item_array()) {
      // Destruct every item from the array we’re abandoning, but first move-construct them if
      // told to do so.
      if (bPreserve) {
         type.move_constr(trn.work_array<void>(), m_pBegin, ciOrig);
      }
      destruct_items(type);
      if (!bPreserve) {
         // We just destructed the items.
         ciOrig = 0;
      }
   }
   trn.commit();
   // The transaction changed the size to ciMin, which is incorrect.
   m_pEnd = begin<int8_t>() + type.cb * ciOrig;
}


void _raw_complex_vextr_impl::set_size(type_void_adapter const & type, size_t ci) {
   ABC_TRACE_FN((this, /*type, */ci));

   if (ci != size(type.cb)) {
      if (ci > capacity()) {
         // Enlarge the item array.
         set_capacity(type, ci, true);
      }
      m_pEnd = begin<int8_t>() + type.cb * ci;
   }
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_trivial_vextr_impl


namespace abc {

void _raw_trivial_vextr_impl::assign_concat(
   size_t cbItem, void const * p1, size_t ci1, void const * p2, size_t ci2
) {
   ABC_TRACE_FN((this, cbItem, p1, ci1, p2, ci2));

   transaction trn(cbItem, this, ptrdiff_t(ci1 + ci2), 0);
   int8_t * pbWorkCopy(trn.work_array<int8_t>());
   if (ci1) {
      size_t cbSrc(cbItem * ci1);
      memory::copy(pbWorkCopy, static_cast<int8_t const *>(p1), cbSrc);
      pbWorkCopy += cbSrc;
   }
   if (ci2) {
      size_t cbSrc(cbItem * ci2);
      memory::copy(pbWorkCopy, static_cast<int8_t const *>(p2), cbSrc);
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
      assign_copy(cbItem, rtvi.m_pBegin, rtvi.size(cbItem));
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
   if (size_t ciTail = size(cbItem) - (iOffset + ciRemove)) {
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

   auto range(adjust_and_validate_range(cbItem, iBegin, iEnd));
   size_t ciRemove(range.second - range.first);
   if (ciRemove) {
      _insert_or_remove(cbItem, range.first, nullptr, 0, ciRemove);
   }
}


void _raw_trivial_vextr_impl::set_capacity(size_t cbItem, size_t ciMin, bool bPreserve) {
   ABC_TRACE_FN((this, cbItem, ciMin, bPreserve));

   size_t ciOrig(size(cbItem));
   transaction trn(cbItem, this, ptrdiff_t(ciMin), 0);
   if (trn.will_replace_item_array()) {
      if (bPreserve) {
         memory::copy(trn.work_array<int8_t>(), begin<int8_t>(), cbItem * ciOrig);
      } else {
         // We’ll lose the item array when the transaction is commited.
         ciOrig = 0;
      }
   }
   trn.commit();
   // The transaction changed the size to ciMin, which is incorrect.
   m_pEnd = begin<int8_t>() + cbItem * ciOrig;
}


void _raw_trivial_vextr_impl::set_size(size_t cbItem, size_t ci) {
   ABC_TRACE_FN((this, cbItem, ci));

   if (ci != size(cbItem)) {
      if (ci > capacity()) {
         // Enlarge the item array.
         set_capacity(cbItem, ci, true);
      }
      m_pEnd = begin<int8_t>() + cbItem * ci;
   }
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

