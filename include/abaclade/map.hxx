/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#ifndef _ABACLADE_MAP_HXX
#define _ABACLADE_MAP_HXX

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> before this file
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::map

namespace abc {

//! Key/value map using a simplified hopscotch hashing collision resolution algorithm.
template <typename TKey, typename TValue, typename THasher = std::hash<TKey>>
class map : public THasher {
public:
   //! Hash generator for TKey.
   typedef THasher hasher;

   //! Iterator type.
   class iterator {
   public:
      /*! Constructor.

      @param pmap
         Pointer to the map owning the iterated objects.
      @param iBucket
         Index of the current bucket.
      */
      iterator(map const * pmap, std::size_t iBucket) :
         mc_pmap(pmap),
         m_iBucket(iBucket) {
      }

   private:
      //! Pointer to the map to iterate over.
      map const * const mc_pmap;
      //! Current bucket index.
      std::size_t m_iBucket;
   };

public:
   /*! Constructor.

   @param m
      Source object.
   */
   map() :
      m_cBuckets(0),
      m_cUsedBuckets(0) {
   }
   map(map && m) :
      m_piHashes(std::move(m.m_piHashes)),
      m_pkeys(std::move(m.m_pkeys)),
      m_pvalues(std::move(m.m_pvalues)),
      m_cBuckets(m.m_cBuckets),
      m_cUsedBuckets(m.m_cUsedBuckets) {
      m.m_cBuckets = 0;
      m.m_cUsedBuckets = 0;
   }

   //! Destructor.
   ~map() {
      clear();
   }

   /*! Assignment operator.

   @param m
      Source object.
   */
   map & operator=(map && m) {
      m_piHashes = std::move(m.m_piHashes);
      m_pkeys = std::move(m.m_pkeys);
      m_pvalues = std::move(m.m_pvalues);
      m_cBuckets = m.m_cBuckets;
      m.m_cBuckets = 0;
      m_cUsedBuckets = m.m_cUsedBuckets;
      m.m_cUsedBuckets = 0;
      return *this;
   }

   /*! Element lookup operator.

   @param key
      Key to lookup.
   @return
      Value corresponding to key. If key is not in the map, an exception will be thrown.
   */
   TValue & operator[](TKey const & key) const {
      std::size_t iBucket = bucket_index_from_key(key);
      if (iBucket == smc_iNullIndex) {
         // TODO: throw proper exception.
         throw 0;
      }
      return m_pvalues[iBucket];
   }

   /*! Adds a key/value pair to the map, overwriting the value if key is already associated to one.

   @param key
      Key to add.
   @param value
      Value to add.
   */
   iterator add(TKey key, TValue value) {
      std::size_t iKeyHash = get_and_adjust_hash(key);
      /* This loop will continue to increase the table size until we’re able to find a free bucket
      for the new element. */
      std::size_t iBucket;
      for (;;) {
         iBucket = get_key_bucket_index(key, iKeyHash);
         if (iBucket != smc_iNullIndex) {
            break;
         }
         if (m_cBuckets) {
            // TODO: resize the hash table.
         } else {
            create_empty_buckets();
         }
      }
      /* TODO: write value at m_pvalues[iBucket]; get_key_bucket_index() took care of key and its
      hash. */
      return iterator(this, iBucket);
   }

   //! Removes all elements from the map.
   void clear() {
      std::size_t * piHash = m_piHashes, * piHashesEnd = piHash + m_cBuckets;
      TKey * pkey = &get_key(0);
      TValue * pvalue = &get_value(0);
      for (; piHash < piHashesEnd; ++piHash, ++pkey, ++pvalue) {
         if (*piHash != smc_iEmptyBucketHash) {
            *piHash = smc_iEmptyBucketHash;
            pkey->~TKey();
            pvalue->~TValue();
         }
      }
      m_cUsedBuckets = 0;
   }

   /*! Removes a key/value pair given the key, which must be in the map.

   @param key
      Key associated to the value to remove.
   */
   void remove(TKey const & key) {
      std::size_t iBucket = bucket_index_from_key(key);
      if (iBucket == smc_iNullIndex) {
         // TODO: throw proper exception.
         throw 0;
      }
      // Mark the bucket as free and destruct the corresponding key and value.
      m_piHashes[iBucket] = smc_iEmptyBucketHash;
      get_key(iBucket).~TKey();
      get_value(iBucket).~TValue();
   }

private:
   void create_empty_buckets(std::size_t cBuckets = smc_cBucketsMin) {
      std::unique_ptr<std::size_t[]> piHashes(new std::size_t[cBuckets]);
      std::unique_ptr<std::max_align_t[]> pkeys(
         new std::max_align_t[ABC_ALIGNED_SIZE(sizeof(TKey) * cBuckets)]
      );
      std::unique_ptr<std::max_align_t[]> pvalues(
         new std::max_align_t[ABC_ALIGNED_SIZE(sizeof(TValue) * cBuckets)]
      );
      // Assign the new arrays and set the remaining members in an exception-safe sequence.
      m_piHashes = std::move(piHashes);
      m_pkeys = std::move(pkeys);
      m_pvalues = std::move(pvalues);
      m_cBuckets = cBuckets;
      m_cUsedBuckets = 0;
      memory::clear(m_piHashes.get(), cBuckets);
   }

   /*! Searches for the specified key, returning the index of the bucket associated to it, or
   smc_iNullIndex if the key is not in the map.

   @param key
      Key to lookup.
   @param iKeyHash
      Hash of key.
   @return
      Index of the bucket at which the key could be found, or smc_iNullIndex if the key was not
      found.
   */
   std::size_t bucket_index_from_key(TKey const & key) const {
      return bucket_index_from_key(key, get_and_adjust_hash(key));
   }
   std::size_t bucket_index_from_key(TKey const & key, std::size_t iKeyHash) const {
      if (m_cBuckets == 0) {
         // The key cannot possibly be in the map.
         return smc_iNullIndex;
      }
      auto pairRanges(get_neighborhood_ranges_from_hash(iKeyHash));
      // Scan till the end of the neighborhood (clipped to the end of the array).
      std::size_t iBucket = bucket_index_from_key_and_bucket_range(
         &key, iKeyHash, pairRanges.first.first, pairRanges.first.second, false
      );
      if (iBucket == smc_iNullIndex && pairRanges.second.first < pairRanges.second.second) {
         // This neighborhood wraps, so we have a second range of buckets to scan.
         iBucket = bucket_index_from_key_and_bucket_range(
            &key, iKeyHash, pairRanges.second.first, pairRanges.second.second, false
         );
      }
      return iBucket;
   }

   /*! Calculates, adjusts and returns the hash value for the specified key.

   @param key
      Key to calculate a hash value for.
   @return
      Hash value of key.
   */
   std::size_t get_and_adjust_hash(TKey const & key) const {
      std::size_t iHash = hasher::operator()(key);
      return iHash == smc_iEmptyBucketHash ? smc_iZeroHash : iHash;
   }

   /*! Finds and returns the bucket index matching the specified key, allocating and initializing it
   anew if none could be found.

   @param pkey
      Key to lookup.
   @param iKeyHash
      Hash of key.
   @return
      Index of the bucket for the specified key.
   */
   std::size_t get_key_bucket_index(TKey key, std::size_t iKeyHash) {
      auto pairRanges(get_neighborhood_ranges_from_hash(iKeyHash));
      /* Look for the key or an empty bucket till the end of the neighborhood (clipped to the end of
      the array). */
      std::size_t iBucket = bucket_index_from_key_and_bucket_range(
         &key, iKeyHash, pairRanges.first.first, pairRanges.first.second, true
      );
      if (iBucket == smc_iNullIndex && pairRanges.second.first < pairRanges.second.second) {
         // Continue the search in the remainder of the neighborhood.
         iBucket = bucket_index_from_key_and_bucket_range(
            &key, iKeyHash, pairRanges.second.first, pairRanges.second.second, true
         );
      }
      if (iBucket == smc_iNullIndex) {
         /* Find a free bucket. If it’s not in the neighborhood, iteratively try to move it closer,
         starting from the end of the neighborhood till the end of the array. */
         iBucket = bucket_index_from_key_and_bucket_range(
            nullptr, smc_iEmptyBucketHash, pairRanges.first.second, m_cBuckets, true
         );
         if (iBucket == smc_iNullIndex && pairRanges.second.first < pairRanges.second.second) {
            /* Continue the search from the start of the array (or end of the wrapped neighborood)
            till the start of the neighborood. */
            iBucket = bucket_index_from_key_and_bucket_range(
               nullptr, smc_iEmptyBucketHash, pairRanges.second.second, pairRanges.first.first, true
            );
         }
         /* We have a free bucket, but it’s not in the neighborhood we need it in: iteratively try
         to move it closer. */
         // TODO
         /* Store the hash and move-construct m_pkey[iBucket] from the provided key. */
         // TODO
      }
      return iBucket;
   }

   TKey & get_key(std::size_t i) const {
      return reinterpret_cast<TKey *>(m_pkeys.get())[i];
   }

   /*! Returns the current neighborhood size.

   @return
      Current neighborhood size, which is not necessarily the same as smc_cNeighborhoodBuckets.
   */
   std::size_t get_neighborhood_size() const {
      // Can’t have a neighborhood larger than the total count of buckets.
      return std::min(smc_cNeighborhoodBuckets, m_cBuckets);
   }

   TValue & get_value(std::size_t i) const {
      return reinterpret_cast<TValue *>(m_pvalues.get())[i];
   }

   /*! Returns the neighborhood index (index of the first bucket in a neighborhood) for the given
   hash.

   @param iHash
      Hash to get the neighborhood index for.
   @return
      Index of the first bucket in the neighborhood.
   */
   std::size_t neighborhood_index_from_hash(std::size_t iHash) const {
      return iHash & (m_cBuckets - 1);
   }

   /*! Returns the neighborhood bucket index ranges for the given hash.

   @param iHash
      Hash to return the neighborhood of.
   @return
      The neighborhood bucket index range is expressed by [return.first.first, return.first.second);
      if the neighborhood wraps from the end of the bucket array back to its start, the wrapped
      portion is expressed by [return.second.first, return.second.second). The second interval will
      be [0, 0) (i.e. empty) if the neighborhood doesn’t wrap.
   */
   std::pair<
      std::pair<std::size_t, std::size_t>, std::pair<std::size_t, std::size_t>
   > get_neighborhood_ranges_from_hash(std::size_t iHash) const {
      // Get a range of indices representing the neighborhood.
      std::size_t iNeighborhoodBegin = neighborhood_index_from_hash(iHash);
      std::size_t iNeighborhoodEnd = iNeighborhoodBegin + get_neighborhood_size();
      // Check if the neighborhood wraps around the end of the bucket array.
      if (iNeighborhoodEnd > m_cBuckets) {
         // Return two ranges.
         return std::make_pair(
            std::make_pair(iNeighborhoodBegin, m_cBuckets),
            std::make_pair(0, iNeighborhoodEnd - m_cBuckets)
         );
      } else {
         // Return a single range.
         return std::make_pair(
            std::make_pair(iNeighborhoodBegin, iNeighborhoodEnd),
            std::make_pair(0, 0)
         );
      }
   }

   bool keys_equal(TKey const & key1, TKey const & key2) const {
      return key1 == key2;
   }

   /*! Scans a range of buckets, looking for a specific key or an unused bucket (if bAcceptEmpty).
   This method could be split in three different ones:

   1. Search for an empty bucket while also checking for a matching key: this would be used when
      looking for an add() insertion point while we’re still in the neighborhood of the key;
   2. Search for an empty bucket: this would cover the rest of the search needed by add();
   3. Search for a matching key: this would be used by bucket_index_from_key().

   @param pkey
      Pointer to the key to scan for.
   @param iKeyHash
      Hash of key.
   @param iRangeBegin
      Start of the bucket range.
   @param iRangeEnd
      End of the bucket range.
   @param bAcceptEmpty
      If true, an empty bucket will be considered a match, and its index returned.
   @return
      Index of the bucket at which the key could be found, or smc_iNullIndex if the key was not
      found.
   */
   std::size_t bucket_index_from_key_and_bucket_range(
      TKey const * pkey, std::size_t iKeyHash, std::size_t iRangeBegin, std::size_t iRangeEnd,
      bool bAcceptEmpty
   ) const {
      /* Optimize away the check for bAcceptEmpty in the loop by comparing against iKeyHash (which
      the loop already does) if the caller desn’t want smc_iEmptyBucketHash. */
      std::size_t iAcceptableEmptyHash = bAcceptEmpty ? smc_iEmptyBucketHash : iKeyHash;
      std::size_t const;
      for (
         std::size_t const * piHash = m_piHashes + iRangeBegin, * piHashesEnd = piHash + iRangeEnd;
         piHash < piHashesEnd;
         ++piHash
      ) {
         if (
            *piHash == iAcceptableEmptyHash ||
            /* Hash collisions (calculating the 2nd operand of the &&) should be rare enough to make
            recalculating the offset from m_pkeys cheaper than keeping a cursor over m_pkeys running
            in parallel to piHash that would only dereferenced on collisions. */
            (*piHash == iKeyHash && keys_equal(m_pkeys[piHash - m_piHashes] == *pkey))
         ) {
            return static_cast<std::size_t>(piHash - m_piHashes);
         }
      }
      return smc_iNullIndex;
   }

private:
   //! Array containing the hash of each key.
   std::unique_ptr<std::size_t[]> m_piHashes;
   //! Array of keys.
   std::unique_ptr<std::max_align_t[]> m_pkeys;
   //! Array of buckets.
   std::unique_ptr<std::max_align_t[]> m_pvalues;
   //! Count of total buckets.
   std::size_t m_cBuckets;
   //! Count of elements / occupied buckets.
   std::size_t m_cUsedBuckets;
   //! Minimum bucket count.
   static std::size_t const smc_cBucketsMin = 8;
   //! Special hash value used to indicate that a bucket is empty.
   static std::size_t const smc_iEmptyBucketHash = 0;
   //! Neighborhood size.
   static std::size_t const smc_cNeighborhoodBuckets = sizeof(std::size_t) * CHAR_BIT;
   //! Special index returned by several methods to indicate a logical “null index”.
   static std::size_t const smc_iNullIndex = numeric::max<std::size_t>::value;
   /*! Hash value substituted when the hash function returns 0; this is so we can use 0 (aliased by
   smc_iEmptyBucketHash) as a special value. This specific value is merely the largest prime number
   that will fit in 2^16, which is the (future, if ever) minimum word size supported by Abaclade. */
   static std::size_t const smc_iZeroHash = 65521;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_MAP_HXX
