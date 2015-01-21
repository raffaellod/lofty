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
#include <abaclade/collections/map.hxx>

#include <climits> // CHAR_BIT


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::detail::map_impl

namespace abc {
namespace collections {
namespace detail {

std::size_t const map_impl::smc_cIdealNeighborhoodBuckets = sizeof(std::size_t) * CHAR_BIT / 8;

map_impl::map_impl() :
   m_cBuckets(0),
   m_cUsedBuckets(0),
   m_cNeighborhoodBuckets(0) {
}
map_impl::map_impl(map_impl && m) :
   m_piHashes(std::move(m.m_piHashes)),
   m_pKeys(std::move(m.m_pKeys)),
   m_pValues(std::move(m.m_pValues)),
   m_cBuckets(m.m_cBuckets),
   m_cUsedBuckets(m.m_cUsedBuckets),
   m_cNeighborhoodBuckets(m.m_cNeighborhoodBuckets) {
   m.m_cBuckets = 0;
   m.m_cUsedBuckets = 0;
   m.m_cNeighborhoodBuckets = 0;
}

map_impl::~map_impl() {
}

map_impl & map_impl::operator=(map_impl && m) {
   m_piHashes = std::move(m.m_piHashes);
   m_pKeys = std::move(m.m_pKeys);
   m_pValues = std::move(m.m_pValues);
   m_cBuckets = m.m_cBuckets;
   m.m_cBuckets = 0;
   m_cUsedBuckets = m.m_cUsedBuckets;
   m.m_cUsedBuckets = 0;
   m_cNeighborhoodBuckets = m.m_cNeighborhoodBuckets;
   m.m_cNeighborhoodBuckets = 0;
   return *this;
}

std::pair<std::size_t, bool> map_impl::add(
   std::size_t cbKey, std::size_t cbValue, keys_equal_fn pfnKeysEqual,
   move_key_value_to_bucket_fn pfnMoveKeyValueToBucket,
   destruct_key_value_fn pfnDestructKeyValue, void * pKey, std::size_t iKeyHash, void * pValue
) {
   if (!m_cBuckets) {
      grow_table(cbKey, cbValue, pfnMoveKeyValueToBucket, pfnDestructKeyValue);
   }
   /* Repeatedly resize the table until we’re able to find an empty bucket for the new element. This
   should typically loop at most once, but smc_iNeedLargerNeighborhoods may need more. */
   std::size_t iBucket;
   for (;;) {
      iBucket = get_existing_or_empty_bucket_for_key(
         cbKey, cbValue, pfnKeysEqual, pfnMoveKeyValueToBucket, pKey, iKeyHash
      );
      if (iBucket < smc_iSpecialIndex) {
         break;
      } else if (iBucket == smc_iNeedLargerNeighborhoods) {
         grow_neighborhoods();
      } else {
         grow_table(cbKey, cbValue, pfnMoveKeyValueToBucket, pfnDestructKeyValue);
      }
   }

   std::size_t * piHash = &m_piHashes[iBucket];
   bool bNew = (*piHash == smc_iEmptyBucketHash);
   if (bNew) {
      // The bucket is currently empty, so initialize it with hash/key/value.
      pfnMoveKeyValueToBucket(this, pKey, pValue, iBucket);
      *piHash = iKeyHash;
   } else {
      // The bucket already has a value, so overwrite that with the value argument.
      pfnMoveKeyValueToBucket(this, nullptr, pValue, iBucket);
   }
   ++m_cUsedBuckets;
   return std::make_pair(iBucket, bNew);
}

std::size_t map_impl::find_bucket_movable_to_empty(std::size_t iEmptyBucket) const {
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

std::size_t map_impl::find_empty_bucket(std::size_t iNhBegin, std::size_t iNhEnd) const {
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

std::size_t map_impl::get_empty_bucket_for_key(
   std::size_t cbKey, std::size_t cbValue, move_key_value_to_bucket_fn pfnMoveKeyValueToBucket,
   std::size_t iKeyHash
) {
   std::size_t iNhBegin, iNhEnd;
   std::tie(iNhBegin, iNhEnd) = hash_neighborhood_range(iKeyHash);
   // Search for an empty bucket in the neighborhood.
   std::size_t iBucket = find_empty_bucket(iNhBegin, iNhEnd);
   if (iBucket != smc_iNullIndex) {
      return iBucket;
   }
   return find_empty_bucket_outside_neighborhood(
      cbKey, cbValue, pfnMoveKeyValueToBucket, iNhBegin, iNhEnd
   );
}

std::size_t map_impl::get_existing_or_empty_bucket_for_key(
   std::size_t cbKey, std::size_t cbValue, keys_equal_fn pfnKeysEqual,
   move_key_value_to_bucket_fn pfnMoveKeyValueToBucket, void const * pKey, std::size_t iKeyHash
) {
   std::size_t iNhBegin, iNhEnd;
   std::tie(iNhBegin, iNhEnd) = hash_neighborhood_range(iKeyHash);
   // Look for the key or an empty bucket in the neighborhood.
   std::size_t iBucket = lookup_key_or_find_empty_bucket(
      cbKey, pKey, iKeyHash, pfnKeysEqual, iNhBegin, iNhEnd
   );
   if (iBucket != smc_iNullIndex) {
      return iBucket;
   }
   return find_empty_bucket_outside_neighborhood(
      cbKey, cbValue, pfnMoveKeyValueToBucket, iNhBegin, iNhEnd
   );
}

std::size_t map_impl::find_empty_bucket_outside_neighborhood(
   std::size_t cbKey, std::size_t cbValue, move_key_value_to_bucket_fn pfnMoveKeyValueToBucket,
   std::size_t iNhBegin, std::size_t iNhEnd
) {
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
      if (iMovableBucket >= smc_iSpecialIndex) {
         /* No buckets have contents that can be moved to iEmptyBucket; the hash table or the
         neighborhoods need to be resized. */
         return iMovableBucket;
      }
      // Move the contents of iMovableBucket to iEmptyBucket.
      pfnMoveKeyValueToBucket(
         this,
         reinterpret_cast<std::int8_t *>(m_pKeys  .get()) + cbKey   * iMovableBucket,
         reinterpret_cast<std::int8_t *>(m_pValues.get()) + cbValue * iMovableBucket,
         iEmptyBucket
      );
      m_piHashes[iEmptyBucket] = m_piHashes[iMovableBucket];
      iEmptyBucket = iMovableBucket;
   }
   return iEmptyBucket;
}

void map_impl::grow_table(
   std::size_t cbKey, std::size_t cbValue, move_key_value_to_bucket_fn pfnMoveKeyValueToBucket,
   destruct_key_value_fn pfnDestructKeyValue
) {
   // The “old” names of these four variables will make sense in a moment…
   std::size_t cOldBuckets = m_cBuckets ? m_cBuckets * smc_iGrowthFactor : smc_cBucketsMin;
   std::unique_ptr<std::size_t[]> piOldHashes(new std::size_t[cOldBuckets]);
   std::unique_ptr<abc::max_align_t[]> pOldKeys(
      new abc::max_align_t[ABC_ALIGNED_SIZE(cbKey * cOldBuckets)]
   );
   std::unique_ptr<abc::max_align_t[]> pOldValues(
      new abc::max_align_t[ABC_ALIGNED_SIZE(cbValue * cOldBuckets)]
   );
   // At this point we’re safe from exceptions, so we can update the member variables.
   std::swap(m_cBuckets, cOldBuckets);
   std::swap(m_piHashes, piOldHashes);
   std::swap(m_pKeys,    pOldKeys);
   std::swap(m_pValues,  pOldValues);
   // Now the names of these variables make sense :)

   /* Recalculate the neighborhood size. The (missing) “else” to this “if” is for when the actual
   neighborhood size is greater than the ideal, which can happen when dealing with a subpar hash
   function that resulted in more collisions than smc_cIdealNeighborhoodBuckets. In that scenario,
   the table size increase doesn’t change anything, since the fix has already been applied to
   m_cNeighborhoodBuckets, which we won’t change here. */
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
   std::int8_t * pbOldKey   = reinterpret_cast<std::int8_t *>(pOldKeys  .get());
   std::int8_t * pbOldValue = reinterpret_cast<std::int8_t *>(pOldValues.get());
   for (; piOldHash < piOldHashesEnd; ++piOldHash, pbOldKey += cbKey, pbOldValue += cbValue) {
      if (*piOldHash != smc_iEmptyBucketHash) {
         std::size_t iNewBucket = get_empty_bucket_for_key(
            cbKey, cbValue, pfnMoveKeyValueToBucket, *piOldHash
         );
         ABC_ASSERT(iNewBucket < smc_iSpecialIndex,
            ABC_SL("failed to find empty bucket while growing hash table; ")
            ABC_SL("if it could be found before, why not now when there are more buckets?")
         );

         // Move hash/key/value to the new bucket.
         pfnMoveKeyValueToBucket(this, pbOldKey, pbOldValue, iNewBucket);
         m_piHashes[iNewBucket] = *piOldHash;
         pfnDestructKeyValue(pbOldKey, pbOldValue);
      }
   }
}

std::size_t map_impl::lookup_key_or_find_empty_bucket(
   std::size_t cbKey, void const * pKey, std::size_t iKeyHash, keys_equal_fn pfnKeysEqual,
   std::size_t iNhBegin, std::size_t iNhEnd
) const {
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
            reinterpret_cast<std::int8_t const *>(m_pKeys.get()) +
               cbKey * static_cast<std::size_t>(piHash - m_piHashes.get()),
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

} //namespace detail
} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
