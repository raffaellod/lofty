/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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
#include <abaclade/map.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::map_impl

namespace abc {
namespace detail {

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

std::size_t map_impl::find_bucket_movable_to_empty(std::size_t iEmptyBucket) const {
   /* Minimum number of buckets on the right of iEmptyBucket that we need in order to have a full
   neighborhood to scan. */
   std::size_t cBucketsRightOfEmpty = m_cNeighborhoodBuckets - 1;
   // Ensure that iEmptyBucket will be on the right of any of the buckets we’re going to check.
   if (iEmptyBucket < cBucketsRightOfEmpty) {
      iEmptyBucket += m_cBuckets;
   }
   // Calculate the bucket index range of the neighborhood that ends with iEmptyBucket.
   std::size_t const * piEmptyHash = m_piHashes.get() + (iEmptyBucket & (m_cBuckets - 1)),
                     * piHash      = piEmptyHash - cBucketsRightOfEmpty,
                     * piHashesEnd = m_piHashes.get() + m_cBuckets;
   // Prepare to track the count of collisions (identical hashes) in the selected neighborhood.
   std::size_t iSampleHash = *piHash, cCollisions = 0;
   /* The neighborhood may wrap, so we can only test for inequality and rely on the wrap-around
   logic at the end of the loop body. */
   while (piHash != piEmptyHash) {
      /* Get the end of the original neighborhood for the key in this bucket; if the empty bucket is
      within that index, the contents of this bucket can be moved to the empty one. */
      std::size_t iCurrNhEnd = hash_neighborhood_index(*piHash) + m_cNeighborhoodBuckets;
      /* Both indices are allowed to be >m_cBuckets (see earlier if), so this comparison is always
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
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
