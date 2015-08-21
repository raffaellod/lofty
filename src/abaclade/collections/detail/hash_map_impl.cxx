/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014, 2015
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
#include <abaclade/collections/detail/hash_map_impl.hxx>

#include <climits> // CHAR_BIT


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

std::size_t const hash_map_impl::smc_cIdealNeighborhoodBuckets = sizeof(std::size_t) * CHAR_BIT / 8;

hash_map_impl::hash_map_impl() :
   m_cBuckets(0),
   m_cUsedBuckets(0),
   m_cNeighborhoodBuckets(0),
   m_iRev(0) {
}
hash_map_impl::hash_map_impl(hash_map_impl && hmi) :
   m_piHashes(_std::move(hmi.m_piHashes)),
   m_pKeys(_std::move(hmi.m_pKeys)),
   m_pValues(_std::move(hmi.m_pValues)),
   m_cBuckets(hmi.m_cBuckets),
   m_cUsedBuckets(hmi.m_cUsedBuckets),
   m_cNeighborhoodBuckets(hmi.m_cNeighborhoodBuckets),
   m_iRev(0) {
   ABC_TRACE_FUNC(this);

   hmi.m_cBuckets = 0;
   hmi.m_cUsedBuckets = 0;
   hmi.m_cNeighborhoodBuckets = 0;
   // Invalidate all iterators for hmi.
   ++hmi.m_iRev;
}

hash_map_impl::~hash_map_impl() {
}

hash_map_impl & hash_map_impl::operator=(hash_map_impl && hmi) {
   ABC_TRACE_FUNC(this);

   m_piHashes = _std::move(hmi.m_piHashes);
   m_pKeys = _std::move(hmi.m_pKeys);
   m_pValues = _std::move(hmi.m_pValues);
   m_cBuckets = hmi.m_cBuckets;
   hmi.m_cBuckets = 0;
   m_cUsedBuckets = hmi.m_cUsedBuckets;
   hmi.m_cUsedBuckets = 0;
   m_cNeighborhoodBuckets = hmi.m_cNeighborhoodBuckets;
   hmi.m_cNeighborhoodBuckets = 0;
   // Invalidate all iterators for *this and for hmi.
   ++m_iRev;
   ++hmi.m_iRev;
   return *this;
}

_std::tuple<std::size_t, bool> hash_map_impl::add_or_assign(
   type_void_adapter const & typeKey, type_void_adapter const & typeValue,
   keys_equal_fn pfnKeysEqual, void * pKey, std::size_t iKeyHash, void * pValue, unsigned iMove
) {
   ABC_TRACE_FUNC(this/*, typeKey, typeValue*/, pfnKeysEqual, pKey, iKeyHash, pValue, iMove);

   if (m_cBuckets == 0) {
      grow_table(typeKey, typeValue);
   }
   /* Repeatedly resize the table until we’re able to find a bucket for the key. This should
   typically loop at most once, but smc_iNeedLargerNeighborhoods may need more. */
   std::size_t iBucket;
   while ((iBucket = get_existing_or_empty_bucket_for_key(
      typeKey, typeValue, pfnKeysEqual, pKey, iKeyHash
   )) >= smc_iFirstSpecialIndex) {
      if (iBucket == smc_iNeedLargerNeighborhoods) {
         grow_neighborhoods();
      } else {
         grow_table(typeKey, typeValue);
      }
   }

   std::size_t * piHash = &m_piHashes[iBucket];
   bool bNew = (*piHash == smc_iEmptyBucketHash);
   if (bNew) {
      // The bucket is currently empty, so initialize it with hash/key/value.
      set_bucket_key_value(typeKey, typeValue, iBucket, pKey, pValue, iMove);
      *piHash = iKeyHash;
   } else {
      // The bucket already has a value, so overwrite that with the value argument.
      set_bucket_key_value(typeKey, typeValue, iBucket, nullptr, pValue, iMove);
   }
   ++m_cUsedBuckets;
   ++m_iRev;
   return _std::make_tuple(iBucket, bNew);
}

void hash_map_impl::clear(type_void_adapter const & typeKey, type_void_adapter const & typeValue) {
   ABC_TRACE_FUNC(this/*, typeKey, typeValue*/);

   std::size_t * piHash = m_piHashes.get(), * piHashesEnd = piHash + m_cBuckets;
   std::int8_t * pbKey   = static_cast<std::int8_t *>(m_pKeys  .get());
   std::int8_t * pbValue = static_cast<std::int8_t *>(m_pValues.get());
   for (; piHash < piHashesEnd; ++piHash, pbKey += typeKey.size(), pbValue += typeValue.size()) {
      if (*piHash != smc_iEmptyBucketHash) {
         *piHash = smc_iEmptyBucketHash;
         typeKey  .destruct(pbKey);
         typeValue.destruct(pbValue);
      }
   }
   m_cUsedBuckets = 0;
   ++m_iRev;
}

void hash_map_impl::empty_bucket(
   type_void_adapter const & typeKey, type_void_adapter const & typeValue, std::size_t iBucket
) {
   ABC_TRACE_FUNC(this/*, typeKey, typeValue*/, iBucket);

   m_piHashes[iBucket] = smc_iEmptyBucketHash;
   typeKey  .destruct(static_cast<std::int8_t *>(m_pKeys  .get()) + typeKey  .size() * iBucket);
   typeValue.destruct(static_cast<std::int8_t *>(m_pValues.get()) + typeValue.size() * iBucket);
   --m_cUsedBuckets;
   /* We could avoid incrementing m_iRev and invalidating every iterator, since nothing other bucket
   was affected, but that would mean that an iterator to the removed pair could still be
   dereferenced. */
   ++m_iRev;
}

std::size_t hash_map_impl::find_bucket_movable_to_empty(std::size_t iEmptyBucket) const {
   ABC_TRACE_FUNC(this, iEmptyBucket);

   std::size_t const * piEmptyHash = m_piHashes.get() + iEmptyBucket;
   /* Minimum number of buckets on the right of iEmptyBucket that we need in order to have a full
   neighborhood to scan. */
   std::size_t cBucketsRightOfEmpty = m_cNeighborhoodBuckets - 1;
   /* Ensure that the heighborhood ending with iEmptyBucket doesn’t wrap. Always having iEmptyBucket
   on the right of any of the buckets we’re going to check simplifies the calculation of piHash and
   the range checks in the loop. */
   if (iEmptyBucket < cBucketsRightOfEmpty) {
      iEmptyBucket += m_cBuckets;
   }
   // Calculate the bucket index range of the neighborhood that ends with iEmptyBucket.
   std::size_t const * piHash      = m_piHashes.get() + iEmptyBucket - cBucketsRightOfEmpty,
                     * piHashesEnd = m_piHashes.get() + m_cBuckets;
   // Prepare to track the count of collisions (identical hashes) in the selected neighborhood.
   std::size_t iSampleHash = *piHash, cCollisions = 0;
   /* The neighborhood may wrap, so we can only test for inequality and rely on the wrap-around
   logic at the end of the loop body. */
   while (piHash != piEmptyHash) {
      /* Get the end of the original neighborhood for the key in this bucket; if the empty bucket is
      within that index, the contents of this bucket can be moved to the empty one. */
      std::size_t iCurrNhEnd = hash_neighborhood_index(*piHash) + m_cNeighborhoodBuckets;
      /* Both indices are allowed to be >m_cBuckets (see above if), so this comparison is always
      valid. */
      if (iEmptyBucket < iCurrNhEnd) {
         return static_cast<std::size_t>(piHash - m_piHashes.get());
      }

      if (iSampleHash == *piHash) {
         ++cCollisions;
      }

      // Move on to the next bucket, wrapping around to the first one if needed.
      if (++piHash == piHashesEnd) {
         piHash = m_piHashes.get();
      }
   }
   // No luck.
   if (cCollisions < cBucketsRightOfEmpty) {
      /* Resizing the hash table will redistribute the hashes in the scanned neighborhood into
      multiple neighborhoods, so repeating this algorithm will find a movable bucket. */
      return smc_iNeedLargerTable;
   } else {
      return smc_iNeedLargerNeighborhoods;
   }
}

std::size_t hash_map_impl::find_empty_bucket(std::size_t iNhBegin, std::size_t iNhEnd) const {
   ABC_TRACE_FUNC(this, iNhBegin, iNhEnd);

   std::size_t const * piHash      = m_piHashes.get() + iNhBegin,
                     * piHashNhEnd = m_piHashes.get() + iNhEnd,
                     * piHashesEnd = m_piHashes.get() + m_cBuckets;
   /* iNhBegin - iNhEnd may be a wrapping range, so we can only test for inequality and rely on the
   wrap-around logic at the end of the loop body. Also, we need to iterate at least once, otherwise
   we won’t enter the loop at all if the start condition is the same as the end condition, which is
   the case for m_cNeighborhoodBuckets == m_cBuckets. */
   do {
      if (*piHash == smc_iEmptyBucketHash) {
         return static_cast<std::size_t>(piHash - m_piHashes.get());
      }

      // Move on to the next bucket, wrapping around to the first one if needed.
      if (++piHash == piHashesEnd) {
         piHash = m_piHashes.get();
      }
   } while (piHash != piHashNhEnd);
   return smc_iNullIndex;
}

std::size_t hash_map_impl::find_empty_bucket_outside_neighborhood(
   type_void_adapter const & typeKey, type_void_adapter const & typeValue, std::size_t iNhBegin,
   std::size_t iNhEnd
) {
   ABC_TRACE_FUNC(this/*, typeKey, typeValue*/, iNhBegin, iNhEnd);

   // Find an empty bucket, scanning every bucket outside the neighborhood.
   std::size_t iEmptyBucket = find_empty_bucket(iNhEnd, iNhBegin);
   if (iEmptyBucket == smc_iNullIndex) {
      // No luck, the hash table needs to be resized.
      return smc_iNullIndex;
   }
   /* This loop will enter (and maybe repeat) if we have an empty bucket, but it’s not in the key’s
   neighborhood, so we have to try and move it in the neighborhood. The not-in-neighborhood check is
   made more complicated by the fact the range may wrap. */
   while (iNhBegin < iNhEnd
      ? iEmptyBucket >= iNhEnd || iEmptyBucket < iNhBegin // Non-wrapping: |---[begin end)---|
      : iEmptyBucket >= iNhEnd && iEmptyBucket < iNhBegin // Wrapping:     | end)-----[begin |
   ) {
      /* The empty bucket is out of the neighborhood. Find the first non-empty bucket that’s part of
      the left-most neighborhood containing iEmptyBucket, but excluding buckets occupied by keys
      belonging to other overlapping neighborhoods. */
      std::size_t iMovableBucket = find_bucket_movable_to_empty(iEmptyBucket);
      if (iMovableBucket >= smc_iFirstSpecialIndex) {
         /* No buckets have contents that can be moved to iEmptyBucket; the hash table or the
         neighborhoods need to be resized. */
         return iMovableBucket;
      }
      // Move the contents of iMovableBucket to iEmptyBucket.
      set_bucket_key_value(
         typeKey, typeValue, iEmptyBucket,
         static_cast<std::int8_t *>(m_pKeys  .get()) + typeKey  .size() * iMovableBucket,
         static_cast<std::int8_t *>(m_pValues.get()) + typeValue.size() * iMovableBucket,
         1 | 2
      );
      m_piHashes[iEmptyBucket] = m_piHashes[iMovableBucket];
      iEmptyBucket = iMovableBucket;
   }
   return iEmptyBucket;
}

std::size_t hash_map_impl::get_empty_bucket_for_key(
   type_void_adapter const & typeKey, type_void_adapter const & typeValue, std::size_t iKeyHash
) {
   ABC_TRACE_FUNC(this/*, typeKey, typeValue*/, iKeyHash);

   std::size_t iNhBegin, iNhEnd;
   _std::tie(iNhBegin, iNhEnd) = hash_neighborhood_range(iKeyHash);
   // Search for an empty bucket in the neighborhood.
   std::size_t iBucket = find_empty_bucket(iNhBegin, iNhEnd);
   if (iBucket != smc_iNullIndex) {
      return iBucket;
   }
   return find_empty_bucket_outside_neighborhood(typeKey, typeValue, iNhBegin, iNhEnd);
}

std::size_t hash_map_impl::get_existing_or_empty_bucket_for_key(
   type_void_adapter const & typeKey, type_void_adapter const & typeValue,
   keys_equal_fn pfnKeysEqual, void const * pKey, std::size_t iKeyHash
) {
   ABC_TRACE_FUNC(this/*, typeKey, typeValue*/, pfnKeysEqual, pKey, iKeyHash);

   std::size_t iNhBegin, iNhEnd;
   _std::tie(iNhBegin, iNhEnd) = hash_neighborhood_range(iKeyHash);
   // Look for the key or an empty bucket in the neighborhood.
   std::size_t iBucket = lookup_key_or_find_empty_bucket(
      typeKey, pfnKeysEqual, pKey, iKeyHash, iNhBegin, iNhEnd
   );
   if (iBucket != smc_iNullIndex) {
      return iBucket;
   }
   return find_empty_bucket_outside_neighborhood(typeKey, typeValue, iNhBegin, iNhEnd);
}

void hash_map_impl::grow_table(
   type_void_adapter const & typeKey, type_void_adapter const & typeValue
) {
   ABC_TRACE_FUNC(this/*, typeKey, typeValue*/);

   // The “old” names of these four variables will make sense in a moment…
   std::size_t cOldBuckets = m_cBuckets ? m_cBuckets * smc_iGrowthFactor : smc_cBucketsMin;
   _std::unique_ptr<std::size_t[]> piOldHashes(new std::size_t[cOldBuckets]);
   auto pOldKeys  (memory::alloc<void>(typeKey  .size() * cOldBuckets));
   auto pOldValues(memory::alloc<void>(typeValue.size() * cOldBuckets));
   // At this point we’re safe from exceptions, so we can update the member variables.
   _std::swap(m_cBuckets, cOldBuckets);
   _std::swap(m_piHashes, piOldHashes);
   _std::swap(m_pKeys,    pOldKeys);
   _std::swap(m_pValues,  pOldValues);
   // Now the names of these variables make sense :)

   /* Recalculate the neighborhood size. The (missing) “else” to this “if” is for when the actual
   neighborhood size is greater than the ideal, which can happen when dealing with a subpar hash
   function that resulted in more collisions than smc_cIdealNeighborhoodBuckets. In that scenario,
   the table size increase doesn’t change anything, since the fix has already been applied with a
   change in m_cNeighborhoodBuckets which happened before this method was called. */
   if (m_cNeighborhoodBuckets < smc_cIdealNeighborhoodBuckets) {
      if (m_cBuckets < smc_cIdealNeighborhoodBuckets) {
         /* m_cNeighborhoodBuckets has not yet reached smc_cIdealNeighborhoodBuckets, but it can’t
         exceed m_cBuckets, so set it to the latter. */
         m_cNeighborhoodBuckets = m_cBuckets;
      } else {
         // Fix m_cNeighborhoodBuckets to its ideal value.
         m_cNeighborhoodBuckets = smc_cIdealNeighborhoodBuckets;
      }
   }

   // Initialize piNewHashes[i] with smc_iEmptyBucketHash.
   memory::clear(m_piHashes.get(), m_cBuckets);
   // Re-insert each hash/key/value triplet to move it from the old arrays to the new ones.
   std::size_t * piOldHash = piOldHashes.get(), * piOldHashesEnd = piOldHash + cOldBuckets;
   std::int8_t * pbOldKey   = static_cast<std::int8_t *>(pOldKeys  .get());
   std::int8_t * pbOldValue = static_cast<std::int8_t *>(pOldValues.get());
   for (
      ;
      piOldHash < piOldHashesEnd;
      ++piOldHash, pbOldKey += typeKey.size(), pbOldValue += typeValue.size()
   ) {
      if (*piOldHash != smc_iEmptyBucketHash) {
         std::size_t iNewBucket = get_empty_bucket_for_key(typeKey, typeValue, *piOldHash);
         ABC_ASSERT(iNewBucket < smc_iFirstSpecialIndex,
            ABC_SL("failed to find empty bucket while growing hash table; ")
            ABC_SL("if it could be found before, why not now when there are more buckets?")
         );

         // Move hash/key/value to the new bucket.
         set_bucket_key_value(typeKey, typeValue, iNewBucket, pbOldKey, pbOldValue, 1 | 2);
         m_piHashes[iNewBucket] = *piOldHash;
         typeKey  .destruct(pbOldKey);
         typeValue.destruct(pbOldValue);
      }
   }
}

std::size_t hash_map_impl::lookup_key_or_find_empty_bucket(
   type_void_adapter const & typeKey, keys_equal_fn pfnKeysEqual, void const * pKey,
   std::size_t iKeyHash, std::size_t iNhBegin, std::size_t iNhEnd
) const {
   ABC_TRACE_FUNC(this/*, typeKey*/, pfnKeysEqual, pKey, iKeyHash, iNhBegin, iNhEnd);

   std::size_t const * piHash      = m_piHashes.get() + iNhBegin,
                     * piHashNhEnd = m_piHashes.get() + iNhEnd,
                     * piHashesEnd = m_piHashes.get() + m_cBuckets;
   /* iNhBegin - iNhEnd may be a wrapping range, so we can only test for inequality and rely on the
   wrap-around logic at the end of the loop body. Also, we need to iterate at least once, otherwise
   we won’t enter the loop at all if the start condition is the same as the end condition, which is
   the case for m_cNeighborhoodBuckets == m_cBuckets. */
   do {
      if (
         *piHash == smc_iEmptyBucketHash ||
         /* Multiple calculations of the second half of the && should be rare enough (exact key
         match or hash collision) to make recalculating the offset from m_pKeys cheaper than keeping
         a cursor over m_pKeys running in parallel to piHash. */
         (*piHash == iKeyHash && pfnKeysEqual(
            this,
            static_cast<std::int8_t const *>(m_pKeys.get()) +
               typeKey.size() * static_cast<std::size_t>(piHash - m_piHashes.get()),
            pKey
         ))
      ) {
         return static_cast<std::size_t>(piHash - m_piHashes.get());
      }

      // Move on to the next bucket, wrapping around to the first one if needed.
      if (++piHash == piHashesEnd) {
         piHash = m_piHashes.get();
      }
   } while (piHash != piHashNhEnd);
   return smc_iNullIndex;
}

void hash_map_impl::set_bucket_key_value(
   type_void_adapter const & typeKey, type_void_adapter const & typeValue, std::size_t iBucket,
   void * pKey, void * pValue, unsigned iMove
) {
   ABC_TRACE_FUNC(this/*, typeKey, typeValue*/, iBucket, pKey, pValue, iMove);

   if (pKey) {
      void * pDst = static_cast<std::int8_t *>(m_pKeys.get()) + typeKey.size() * iBucket;
      if (iMove & 1) {
         typeKey.move_construct(pDst, pKey);
      } else {
         typeKey.copy_construct(pDst, pKey);
      }
   }
   {
      void * pDst = static_cast<std::int8_t *>(m_pValues.get()) + typeValue.size() * iBucket;
      if (iMove & 2) {
         typeValue.move_construct(pDst, pValue);
      } else {
         typeValue.copy_construct(pDst, pValue);
      }
   }
}


hash_map_impl::iterator_base::iterator_base() :
   m_phm(nullptr),
   m_iBucket(smc_iNullIndex) {
}
hash_map_impl::iterator_base::iterator_base(hash_map_impl const * phm, std::size_t iBucket) :
   m_phm(phm),
   m_iBucket(iBucket),
   m_iRev(phm->m_iRev) {
}

void hash_map_impl::iterator_base::increment() {
   ABC_TRACE_FUNC(this);

   for (;;) {
      ++m_iBucket;
      if (m_iBucket >= m_phm->m_cBuckets) {
         m_iBucket = smc_iNullIndex;
         return;
      }
      if (m_phm->m_piHashes[m_iBucket] != smc_iEmptyBucketHash) {
         return;
      }
   }
}

void hash_map_impl::iterator_base::validate() const {
   ABC_TRACE_FUNC(this);

   if (m_iBucket == smc_iNullIndex || m_iRev != m_phm->m_iRev) {
      ABC_THROW(iterator_error, ());
   }
}

}}} //namespace abc::collections::detail
