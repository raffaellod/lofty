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

#ifndef _ABC__MAP_BASE_HXX
#define _ABC__MAP_BASE_HXX

#ifndef _ABC_CORE_HXX
   #error Please #include <abc/core.hxx> instead of this file
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abc/bitmanip.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_map_desc


namespace abc {

/** TODO: comment.

TODO maybe move to other header file?
*/
template <typename T>
union force_max_align {
public:

   /** Actual storage. */
   T t;


private:

   /** Forces the whole union to have the most generic alignment; on many architectures this will be
   2 * word size. In any case, this makes the union aligned the same way malloc() aligns the
   pointers it returns. */
   std::max_align_t aligner[ABC_ALIGNED_SIZE(sizeof(T))];
};


/** Template-independent map descriptor.

Just like _raw_vector_desc, the hashes array immediately follows the last declared member of this
struct, while the other two arrays (keys and values) might need to have padding added before them,
so we’ll store their offset relative to the beginning of the struct, to avoid having to recalculate
it on every access.

See [HACK#0012 abc::map] to understand why we have so many classes just to implement a map.
*/
struct _raw_map_desc {

   struct _s {
      /** Numer of entries in the table - 1. We store this instead of the actual number of entries,
      because this is used much more frequently. */
      size_t iMask;
      /** Number of active entries in the table. */
      size_t ceActive;
      /** Number of active and reserved entries in the table. */
      size_t ceUsed;
      /** Offset of the keys array from this, in bytes. */
      size_t ibKeysOffset;
      /** Offset of the values array from this, in bytes. */
      size_t ibValsOffset;
   };
   force_max_align<_s> m;

   enum {
      /** No less than this many map entries. Must be a power of 2, and at least 4. */
      smc_ceMin = 8
   };


   /** Returns a pointer to this object’s (undeclared) hashes array.

   TODO: comment signature.
   */
   size_t * get_phashes() {
      return reinterpret_cast<size_t *>(this + 1);
   }
   size_t const * get_phashes() const {
      return const_cast<_raw_map_desc *>(this)->get_phashes();
   }


   /** Returns a pointer to this object’s (undeclared) key array.

   TODO: comment signature.
   */
   void * get_pKeys() {
      return reinterpret_cast<int8_t *>(this) + m.t.ibKeysOffset;
   }
   void const * get_pKeys() const {
      return const_cast<_raw_map_desc *>(this)->get_pKeys();
   }


   /** Returns a oointer to this object’s (undeclared) value array.

   TODO: comment signature.
   */
   void * get_pVals() {
      return reinterpret_cast<int8_t *>(this) + m.t.ibValsOffset;
   }
   void const * get_pVals() const {
      return const_cast<_raw_map_desc *>(this)->get_pVals();
   }


   /** Returns true if the descriptor has enough entries to accomodate the specified number of
   items.

   TODO: comment signature.
   */
   bool can_fit(size_t ce) {
      return ce * 3 < (m.t.iMask + 1) * 2;
   }


   /** Returns true if the descriptor has enough entries to accomodate the specified number of
   items.

   TODO: comment signature.
   */
   size_t get_byte_size(size_t cbVal) {
      return m.t.ibValsOffset + cbVal * (m.t.iMask + 1);
   }


   /** Returns a pointer to the key at the specified index.

   TODO: comment signature.
   */
   void * get_key_at(size_t cbKey, size_t ie) {
      int8_t * pKeys(reinterpret_cast<int8_t *>(get_pKeys()));
      return pKeys + cbKey * ie;
   }
   void const * get_key_at(size_t cbKey, size_t ie) const {
      return const_cast<_raw_map_desc *>(this)->get_key_at(cbKey, ie);
   }


   /** Returns a pointer to the value at the specified index.

   TODO: comment signature.
   */
   void * get_value_at(size_t cbVal, size_t ie) {
      int8_t * pVals(reinterpret_cast<int8_t *>(get_pVals()));
      return pVals + cbVal * ie;
   }
   void const * get_value_at(size_t cbVal, size_t ie) const {
      return const_cast<_raw_map_desc *>(this)->get_value_at(cbVal, ie);
   }


   /** Clears the descriptor. It assumes that *this has no contents that need to be destructed.

   TODO: comment signature.
   */
   void reset() {
      memory::clear(get_phashes(), m.t.iMask + 1);
      m.t.ceActive = 0;
      m.t.ceUsed = 0;
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_dynamic_map_desc


namespace abc {

/** Template-independent dynamically-allocated descriptor.
*/
class _dynamic_map_desc :
   public _raw_map_desc {
public:

   /** Allocates enough memory to contain the descriptor and a specified number of hashes, keys and
   values. The values pointed to by the last arguments are to be passed back to the constructor.

   TODO: comment signature.
   */
   static void * operator new(
      size_t cbDesc, size_t cbKey, size_t cbVal,
      size_t ce, size_t * pibKeysOffset, size_t * pibValsOffset
   ) {
      // Avoid allocating too few entries, because that might cause a descriptor switch sooner than
      // later.
      if (ce < smc_ceMin) {
         ce = smc_ceMin;
      }
      // Calculate the offsets of the embedded arrays for which we need to allocate extra memory.
      get_offsets(cbKey, ce, pibKeysOffset, pibValsOffset);
      // At this point, ibValsOffset + cbVal * ce is the size of the whole object, including the 3
      // ce-sized arrays (hashes, keys, vaues). Since cbDesc can be greater than
      // sizeof(_raw_map_desc), we must also account for that difference.
      return memory::_raw_alloc((cbDesc - sizeof(_raw_map_desc)) + *pibValsOffset + cbVal * ce);
   }


   /** Constructor.

   TODO: comment signature.
   */
   _dynamic_map_desc(size_t ce, size_t const & ibKeysOffset, size_t const & ibValsOffset) {
      _raw_map_desc::m.t.iMask = ce - 1;
      _raw_map_desc::m.t.ibKeysOffset = ibKeysOffset;
      _raw_map_desc::m.t.ibValsOffset = ibValsOffset;
      reset();
   }


protected:


   /** Computes the padding to be added before the keys and values arrays, and calculates and
   returns the resulting offsets.

   TODO: comment signature.
   */
   static void get_offsets(
      size_t cbKey, size_t ce, size_t * pibKeysOffset, size_t * pibValsOffset
   ) {
      // Calculate the size of this with a size_t[ce] hashes array appended.
      size_t cbThisWithArrays(sizeof(_raw_map_desc) + sizeof(size_t) * ce);
      // Align the keys array to the closest std::max_align_t boundary, which will fit any type.
      *pibKeysOffset = bitmanip::ceiling_to_pow2_multiple(
         cbThisWithArrays, sizeof(std::max_align_t)
      );
      // Add the keys array, and repeat for the values array.
      cbThisWithArrays += cbKey * ce;
      *pibValsOffset = bitmanip::ceiling_to_pow2_multiple(
         cbThisWithArrays, sizeof(std::max_align_t)
      );
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_embedded_map_desc


namespace abc {

/** Embeddable static descriptor.

Note that, since instances of this class will follow the object that uses them (e.g. map contains
_raw_map_data first, then _embedded_map_desc), the _raw_map_desc members of this class cannot be
initialized in a constructor, because that would be called too late.
*/
template <typename TKey, typename TVal, size_t t_ceFixed>
class _embedded_map_desc :
   public _raw_map_desc {
public:

   /** Constructor.

   TODO: comment signature.
   */
   _embedded_map_desc() {
   }


   /** Returns a pointer to the embedded static descriptor, after initializing it to zero length.

   TODO: comment signature.
   */
   _raw_map_desc * init_and_get_desc() {
      _raw_map_desc::m.t.iMask = t_ceFixed - 1;
      _raw_map_desc::m.t.ibKeysOffset = reinterpret_cast<intptr_t>(&atkeys) -
         reinterpret_cast<intptr_t>(this);
      _raw_map_desc::m.t.ibValsOffset = reinterpret_cast<intptr_t>(&atvals) -
         reinterpret_cast<intptr_t>(this);
      // Purposefully avoid calling reset(), so we don’t slow down map::map() when the embedded
      // descriptor isn’t going to be used.
      return this;
   }


private:

   /** Static hashes array. */
   size_t ahashes[t_ceFixed];
   /** Static keys array. This can’t be a TKey[], because we don’t want the keys to be constructed/
   destructed automatically. */
   std::max_align_t atkeys[ABC_ALIGNED_SIZE(sizeof(TKey) * t_ceFixed)];
   /** Static values array. This can’t be a TVal[], because we don’t want the values to be
   constructed/destructed automatically. */
   std::max_align_t atvals[ABC_ALIGNED_SIZE(sizeof(TVal) * t_ceFixed)];
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_map_data


namespace abc {

/** Template-independent data members of _raw_*_map_impl.
*/
struct _raw_map_data {

   /** Pointer to the map descriptor. */
   _raw_map_desc * m_prmd;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_map_root


namespace abc {

/** Template-independent methods of _raw_*_map_impl that are identical for trivial and non-trivial
types. */
struct _raw_map_root :
   public _raw_map_data {
public:

   /** Adjusts a hash code to avoid reserved values.

   TODO: comment signature.
   */
   static size_t adjust_hash(size_t hash) {
      if (hash == smc_hashUnused) {
         return 36471;
      } else if (hash == smc_hashReserved) {
         return 19047;
      } else {
         return hash;
      }
   }


   /** See the template version _map_impl::get_size().

   TODO: comment signature.
   */
   size_t get_size() const {
      return m_prmd->m.t.ceActive;
   }


   /** Returns a pointer to the value associated to the specified key. If the key could not be
   found, an exception is thrown. See also the template version (returning & instead of *)
   _map_impl::operator[]().

   TODO: comment signature.
   */
   void * get_value(
      size_t cbKey, size_t cbVal, type_void_adapter::equal_fn pfnKeyEqual,
      void const * pKey, size_t hash
   ) {
      size_t ie(lookup(cbKey, pfnKeyEqual, pKey, hash));
      size_t hashEntry(m_prmd->get_phashes()[ie]);
      if (!is_entry_active(hashEntry)) {
         ABC_THROW(key_error, ());
      }
      return m_prmd->get_value_at(cbVal, ie);
   }
   void const * get_value(
      size_t cbKey, size_t cbVal, type_void_adapter::equal_fn pfnKeyEqual,
      void const * pKey, size_t hash
   ) const {
      return const_cast<_raw_map_root *>(this)->get_value(cbKey, cbVal, pfnKeyEqual, pKey, hash);
   }


   /** Returns true if the provided hash value identifies an active entry.

   TODO: comment signature.
   */
   static bool is_entry_active(size_t hash) {
      return hash != smc_hashUnused && hash != smc_hashReserved;
   }


   /** Returns an the index of the entry associated to the specified key (and its hash). Based on
   Algorithm D from Knuth Vol. 3, Sec. 6.4.

   TODO: comment signature.
   */
   size_t lookup(
      size_t cbKey, type_void_adapter::equal_fn pfnKeyEqual, void const * pKey, size_t hash
   ) const {
      size_t const * phashes(m_prmd->get_phashes());
      size_t hashFull(hash), i(hash);
      size_t ieRet(i & m_prmd->m.t.iMask);
      size_t hashEntry(phashes[ieRet]);
      // If the entry is unused or it’s active and the key matches, use it.
      if (hashEntry == smc_hashUnused || (
         hashEntry == hashFull && pfnKeyEqual(m_prmd->get_key_at(cbKey, ieRet), pKey)
      )) {
         return ieRet;
      }

      // If the entry is only reserved, keep track of it as it will be used if there are no other
      // active entries with this same colliding hash. Otherwise it’s a collision, and this slot
      // can’t be used at all.
      size_t ieNone(~size_t(0));
      size_t ieFirstRes(hashEntry == smc_hashReserved ? ieRet : ieNone);
      // Either way, now check for all the other entries in the same relocation chain, and at the
      // end, either the first empty or the first reserved entries will have been picked.
      for (;; hash >>= smc_cbitPerturb) {
         i = i * 5 + hash + 1;
         size_t ie(i & m_prmd->m.t.iMask);
         hashEntry = phashes[ie];
         if (hashEntry == smc_hashUnused) {
            // Unused entry: return it, unless we previously found a reserved entry, in which case
            // we’ll return that.
            return ieFirstRes != ieNone ? ieFirstRes : ie;
         }
         if (hashEntry == hashFull && pfnKeyEqual(m_prmd->get_key_at(cbKey, ie), pKey)) {
            // Active entry: return it.
            return ie;
         }
         if (hashEntry == smc_hashReserved && ieFirstRes == ieNone) {
            // Another reserved: we’ll use this only if we didn’t already have a reserved entry
            // (i.e. they were all active, so far). This is also the least likely situation.
            ieFirstRes = ie;
         }
      }
   }


protected:

   /** Returns a pointer to the _embedded_map_desc that’s assumed to follow *this.

   TODO: comment signature.
   */
   _raw_map_desc * get_embedded_desc() {
      // This works under the assumption that the alignment of the _raw_map_desc-derived object
      // forces the containing _raw_map_data-derived object to have the same alignment.
      int8_t * p(reinterpret_cast<int8_t *>(this));
      p += bitmanip::ceiling_to_pow2_multiple(sizeof(*this), sizeof(std::max_align_t));
      return reinterpret_cast<_raw_map_desc *>(p);
   }
   _raw_map_desc const * get_embedded_desc() const {
      return const_cast<_raw_map_root *>(this)->get_embedded_desc();
   }


protected:

   /** See large comment block below. This must be >= 1. */
   static int const smc_cbitPerturb = 5;
   /** Hash value used to mark unused entries. This is zero, so that we can quickly wipe the hashes
   array of a descriptor. */
   static size_t const smc_hashUnused = 0;
   /** Hash value used to mark reserved entries (i.e. formerly used). */
   static size_t const smc_hashReserved = smc_hashUnused - 1;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_complex_map_impl


namespace abc {

/** Template-independent implementation of a map for non-trivial contained types.
*/
struct _raw_complex_map_impl :
   public _raw_map_root {

   /** See the template version _map_impl::add().

   TODO: comment signature.
   */
   void add(
      type_void_adapter const & typeKey, type_void_adapter const & typeVal,
      void const * pKey, size_t hash, void const * pVal, bool bMoveKey, bool bMoveVal
   ) {
      // The only way of knowing if set_item() took up one more entry is to count the number of used
      // entries; if that goes up, we might need to resize the map.
      size_t ceUsedBefore(m_prmd->m.t.ceUsed);
      set_item(typeKey, typeVal, pKey, hash, pVal, bMoveKey, bMoveVal);
      if (m_prmd->m.t.ceUsed > ceUsedBefore && !m_prmd->can_fit(m_prmd->m.t.ceActive)) {
         try {
            // Resizes are very expensive, so enlarge the descriptor by a lot.
            resize(typeKey, typeVal, m_prmd->m.t.ceActive * 3);
         } catch (...) {
            // TODO: un-move the value, and remove().
            throw;
         }
      }
   }


   /** See the template version _map_impl::assign(). If bMove == true, the source will be modified
   by having its const-ness cast away - be careful.

   TODO: comment signature.
   */
   void assign(
      type_void_adapter const & typeKey, type_void_adapter const & typeVal,
      _raw_map_root const & rmrSrc, bool bMove
   ) {
      if (&rmrSrc == this) {
         return;
      }
      _raw_map_desc const * pemdSrc(
         static_cast<_raw_complex_map_impl const *>(&rmrSrc)->get_embedded_desc()
      );
      if (bMove && rmrSrc.m_prmd != pemdSrc) {
         // The source is using a dynamic descriptor, and we are performing a move: just take
         // ownership of the source descriptor.
         if (m_prmd) {
            release_desc(typeKey, typeVal);
         }
         m_prmd = rmrSrc.m_prmd;
      } else {
         new_desc_from(typeKey, typeVal, rmrSrc.m_prmd, rmrSrc.m_prmd->m.t.ceActive, bMove);
      }
      if (bMove) {
         // If moving, clear the source by assigning it its emptied embedded descriptor.
         _raw_map_desc * pemdSrcW(const_cast<_raw_map_desc *>(pemdSrc));
         pemdSrcW->reset();
         const_cast<_raw_map_root &>(rmrSrc).m_prmd = pemdSrcW;
      }
   }


   /** Destructs every key and value in the descriptor, then releases it.

   TODO: comment signature.
   */
   void release_desc(type_void_adapter const & typeKey, type_void_adapter const & typeVal) {
      size_t * phash(m_prmd->get_phashes());
      int8_t * pbKey(static_cast<int8_t *>(m_prmd->get_pKeys())),
             * pbVal(static_cast<int8_t *>(m_prmd->get_pVals()));
      // This loop will stop as soon as all the active entries have been destructed, potentially
      // saving quite a few comparisons.
      for (size_t ce(m_prmd->m.t.ceActive); ce; ++phash, pbKey += typeKey.cb, pbVal += typeVal.cb) {
         if (is_entry_active(*phash)) {
            typeKey.destruct(pbKey, 1);
            typeVal.destruct(pbVal, 1);
            --ce;
         }
      }
      // If the descriptor in use is not the embedded one, release it.
      if (m_prmd != get_embedded_desc()) {
         delete m_prmd;
      }
   }


   /** See the template version _map_impl::remove().

   TODO: comment signature.
   */
   void remove(
      type_void_adapter const & typeKey, type_void_adapter const & typeVal,
      void const * pKey, size_t hash
   ) {
      size_t ie(lookup(typeKey.cb, typeKey.equal, pKey, hash));
      size_t * phashEntry(&m_prmd->get_phashes()[ie]);
      if (!is_entry_active(*phashEntry)) {
         ABC_THROW(key_error, ());
      }

      void * pEntryKey(m_prmd->get_key_at(typeKey.cb, ie));
      void * pEntryVal(m_prmd->get_value_at(typeVal.cb, ie));
      typeKey.destruct(pEntryKey, 1);
      typeVal.destruct(pEntryVal, 1);
      // Mark this entry as reserved, because we might have other collided hashes. Notice that this
      // means we can’t shrink the descriptor.
      *phashEntry = smc_hashReserved;
      --m_prmd->m.t.ceActive;
   }


   /** See the template version _map_impl::clear().

   TODO: comment signature.
   */
   void clear(type_void_adapter const & typeKey, type_void_adapter const & typeVal) {
      release_desc(typeKey, typeVal);
      // Switch to the embedded descriptor and empty it.
      m_prmd = get_embedded_desc();
      m_prmd->reset();
   }


   /** See the template version _map_impl::set_item().

   TODO: comment signature.
   */
   size_t set_item(
      type_void_adapter const & typeKey, type_void_adapter const & typeVal,
      void const * pKey, size_t hash, void const * pVal, bool bMoveKey, bool bMoveVal
   ) {
      size_t ie(lookup(typeKey.cb, typeKey.equal, pKey, hash));
      size_t * phashEntry(&m_prmd->get_phashes()[ie]);
      void * pEntryKey;
      void * pEntryVal(m_prmd->get_value_at(typeVal.cb, ie));
      std::unique_ptr<int8_t[]> pbValBackup;
      bool bActive(is_entry_active(*phashEntry));
      if (bActive) {
         pbValBackup.reset(new int8_t[typeVal.cb]);
         // Move the current value to the backup, and destruct the emptied value of the entry.
         typeVal.move_constr(pbValBackup.get(), pEntryVal, 1);
         typeVal.destruct(pEntryVal, 1);
      } else {
         pEntryKey = m_prmd->get_key_at(typeKey.cb, ie);
         if (bMoveKey) {
            typeKey.move_constr(pEntryKey, const_cast<void *>(pKey), 1);
         } else {
            typeKey.copy_constr(pEntryKey, pKey, 1);
         }
      }
      // Copy or move the new value to the entry.
      if (bMoveVal) {
         typeVal.move_constr(pEntryVal, const_cast<void *>(pVal), 1);
      } else {
         try {
            typeVal.copy_constr(pEntryVal, pVal, 1);
         } catch (...) {
            if (bActive) {
               // Put the old value back in place, then destruct the emptied backup.
               typeVal.move_constr(pEntryVal, pbValBackup.get(), 1);
               typeVal.destruct(pbValBackup.get(), 1);
            } else {
               if (bMoveKey) {
                  // Move the key back.
                  void * pKeyW(const_cast<void *>(pKey));
                  typeKey.destruct(pKeyW, 1);
                  typeVal.move_constr(pKeyW, pEntryKey, 1);
               }
               // Undo the key copy or the formerly-moved-now-emptied key.
               typeKey.destruct(pEntryKey, 1);
            }
            throw;
         }
      }
      // If we’re still here, key and value have been inserted.
      if (bActive) {
         // Destruct the old value.
         typeVal.destruct(pbValBackup.get(), 1);
      } else {
         if (*phashEntry == smc_hashUnused) {
            // This entry has never been active, so now count it as used.
            ++m_prmd->m.t.ceUsed;
         }
         *phashEntry = hash;
         ++m_prmd->m.t.ceActive;
      }
      return ie;
   }


protected:

   /** Copies or moves the contents of the source descriptor to a newly allocated descriptor or to
   the embedded one if not in use, based on the requested size.

   TODO: comment signature.
   */
   void new_desc_from(
      type_void_adapter const & typeKey, type_void_adapter const & typeVal,
      _raw_map_desc const * prmdSrc, size_t ceNew, bool bMove
   ) {
      _raw_map_desc * prmdDst, * pemdDst(get_embedded_desc());
      std::unique_ptr<_dynamic_map_desc> pdmdDstNew;
      bool bUseEmbedded(pemdDst->can_fit(ceNew) && m_prmd != pemdDst);
      if (bUseEmbedded) {
         // The embedded descriptor is large enough, so we can make prmdDst point directly to it.
         pemdDst->reset();
         prmdDst = pemdDst;
      } else {
         // We need a new descriptor.
         ceNew = bitmanip::ceiling_to_pow2(ceNew);
         size_t ibKeysOffset, ibValsOffset;
         pdmdDstNew.reset(
            new(typeKey.cb, typeVal.cb, ceNew, &ibKeysOffset, &ibValsOffset)
               _dynamic_map_desc(ceNew, ibKeysOffset, ibValsOffset)
         );
         prmdDst = pdmdDstNew.get();
      }

      size_t const * phashSrc  (prmdSrc->get_phashes());
      size_t       * phashesDst(prmdDst->get_phashes());
      int8_t const * pbKeySrc (static_cast<int8_t const *>(prmdSrc->get_pKeys()));
      int8_t       * pbKeysDst(static_cast<int8_t       *>(prmdDst->get_pKeys()));
      int8_t const * pbValSrc (static_cast<int8_t const *>(prmdSrc->get_pVals()));
      int8_t       * pbValsDst(static_cast<int8_t       *>(prmdDst->get_pVals()));
      // Copy the active entries in random order. This loop will stop as soon as all the active
      // entries have been copied, potentially saving quite a few comparisons.
      for (
         size_t ceSrc(prmdSrc->m.t.ceActive);
         ceSrc;
         ++phashSrc, pbKeySrc += typeKey.cb, pbValSrc += typeVal.cb
      ) {
         if (is_entry_active(*phashSrc)) {
            // This entry is active: find its insertion point in *this.
            size_t hash(*phashSrc), i(hash);
            size_t ieDst(i & prmdDst->m.t.iMask);
            for (; phashesDst[ieDst] != smc_hashUnused; hash >>= smc_cbitPerturb) {
               i = i * 5 + hash + 1;
               ieDst = i & prmdDst->m.t.iMask;
            }
            size_t * phashDst(&phashesDst[ieDst]);
            int8_t * pbKeyDst(pbKeysDst + typeKey.cb * ieDst);
            int8_t * pbValDst(pbValsDst + typeVal.cb * ieDst);
            if (bMove) {
               typeKey.move_constr(pbKeyDst, const_cast<int8_t *>(pbKeySrc), 1);
               typeVal.move_constr(pbValDst, const_cast<int8_t *>(pbValSrc), 1);
            } else {
               // This will tell us if we need to also destruct the key we just copied, in case of
               // exceptions.
               bool bKeyConstructed(false);
               try {
                  typeKey.copy_constr(pbKeyDst, pbKeySrc, 1);
                  bKeyConstructed = true;
                  typeVal.copy_constr(pbValDst, pbValSrc, 1);
               } catch (...) {
                  // If the exception was thrown while constructing the value, destruct the key.
                  if (bKeyConstructed) {
                     typeKey.destruct(pbKeyDst, 1);
                  }
                  // Iterate backwards, destructing every active entry.
                  for (; --phashDst >= phashesDst; pbKeyDst -= typeKey.cb, pbValDst -= typeVal.cb) {
                     if (is_entry_active(*phashDst)) {
                        typeKey.destruct(pbKeyDst, 1);
                        typeVal.destruct(pbValDst, 1);
                     }
                  }
                  throw;
               }
            }
            *phashDst = hash;
            --ceSrc;
         }
      }
      prmdDst->m.t.ceUsed = prmdDst->m.t.ceActive = prmdSrc->m.t.ceActive;

      // Now that all items have been moved, we can discard the current descriptor and switch to the
      // new one.
      if (m_prmd) {
         release_desc(typeKey, typeVal);
      }
      m_prmd = prmdDst;
      if (!bUseEmbedded) {
         // Don’t deallocate the temporary descriptor, we’re using it.
         pdmdDstNew.release();
      }
   }


   /** Resizes the map by allocating a larger descriptor and moving all the entries to it. The
   number of used entries might decrease, because copying to a new descriptor will discard any
   reserved entries.

   TODO: comment signature.
   */
   void resize(type_void_adapter const & typeKey, type_void_adapter const & typeVal, size_t ceNew) {
      _raw_map_desc * prmd(get_embedded_desc());
      if (m_prmd == prmd && prmd->can_fit(ceNew)) {
         // If the embedded descriptor can still fit the items, don’t do anything at all.
         return;
      }
      new_desc_from(typeKey, typeVal, m_prmd, ceNew, true);
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_trivial_map_impl


namespace abc {

/** Template-independent implementation of a map for trivial contained types.
*/
struct _raw_trivial_map_impl :
   public _raw_map_root {
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef _ABC__MAP_BASE_HXX

