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

   void add(TKey key, TValue value) {
   }

   //! Removes all elements from the map.
   void clear() {
      std::size_t * piHash = m_piHashes, * piHashesEnd = piHash + m_cBuckets;
      TKey * pkey = &get_key(0);
      TValue * pvalue = &get_value(0);
      for (; piHash < piHashesEnd; ++piHash, ++pkey, ++pvalue) {
         if (*piHash != smc_iEmptyBucket) {
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
      // Get the exact bucket in the neighborhood of the key’s hash.
      std::size_t iBucket = bucket_index_from_key(key);
      // Mark the bucket as free and destruct the corresponding key and value.
      m_piHashes[iBucket] = smc_iEmptyBucket;
      get_key(iBucket).~TKey();
      get_value(iBucket).~TValue();
   }

private:
   void create_empty_buckets(std::size_t cBuckets = smc_cBucketsMin) {
      m_piHashes.reset(new std::size_t[cBuckets]);
      m_pkeys.reset(new std::max_align_t[ABC_ALIGNED_SIZE(sizeof(TKey) * cBuckets)]);
      m_pvalues.reset(new std::max_align_t[ABC_ALIGNED_SIZE(sizeof(TValue) * cBuckets)]);
      m_cBuckets = cBuckets;
   }

   std::size_t bucket_index_from_key(TKey const & key) const {
      return bucket_index_from_key(key, get_and_adjust_hash(key));
   }
   std::size_t bucket_index_from_key(TKey const & key, std::size_t iHash) const {
      // Get a range of indices representing the neighborhood.
      std::size_t iNeighborhoodBegin = hash_to_neighborhood_index(iHash);
      std::size_t iNeighborhoodEnd = iNeighborhoodBegin + smc_cNeighborhood;
      /* Determine if we’ll need to check any buckets at the beginning of the array due to the
      neighborhood wrapping. */
      std::size_t iWrappedNeighborhoodEnd;
      if (iNeighborhoodEnd > m_cBuckets) {
         iWrappedNeighborhoodEnd = iNeighborhoodEnd - m_cBuckets;
         iNeighborhoodEnd = m_cBuckets;
      } else {
         iWrappedNeighborhoodEnd = 0;
      }
      // Scan till the end of the neighborhood (clipped to the end of the array).
      std::size_t iBucket = scan_buckets_for_key(key, iHash, iNeighborhoodBegin, iNeighborhoodEnd);
      if (iBucket != smc_iKeyNotFound && iWrappedNeighborhoodEnd) {
         return iBucket;
      }
      // Scan the remaining buckets, if this neighborhood wraps.
      iBucket = scan_buckets_for_key(key, iHash, 0, iWrappedNeighborhoodEnd);
      if (iBucket != smc_iKeyNotFound) {
         return iBucket;
      }
      // The specified key is not in the map.
      // TODO: throw proper exception.
      throw 0;
   }

   /*! Calculates, adjusts and returns the hash value for the specified key.

   @param key
      Key to calculate a hash value for.
   @return
      Hash value of key.
   */
   std::size_t get_and_adjust_hash(TKey const & key) const {
      std::size_t iHash = hasher::operator()(key);
      return iHash == smc_iEmptyBucket ? smc_iZeroHash : iHash;
   }

   TKey & get_key(std::size_t i) const {
      return reinterpret_cast<TKey *>(m_pkeys.get())[i];
   }

   TValue & get_value(std::size_t i) const {
      return reinterpret_cast<TKey *>(m_pvalues.get())[i];
   }

   std::size_t hash_to_neighborhood_index(std::size_t iHash) const {
      if (!m_cBuckets) {
         // No buckets, no index can be returned. This means that iHash cannot be in the map.
         // TODO: throw proper exception.
         throw 0;
      }
      return iHash & (m_cBuckets - 1);
   }

   bool keys_equal(TKey const & key1, TKey const & key2) const {
      return key1 == key2;
   }

   /*! Scans a range of buckets, looking for a specific key.

   @param key
      Key to scan for.
   @param iHash
      Hash of key.
   @param iBegin
      Start of the bucket range.
   @param iEnd
      End of the bucket range.
   @return
      Index of the bucket at which the key could be found, or smc_iKeyNotFound if the key was not
      found.
   */
   std::size_t scan_buckets_for_key(
      TKey const & key, std::size_t iHash, std::size_t iBegin, std::size_t iEnd
   ) const {
      std::size_t const * piHashesBegin = m_piHashes + iBegin, * piHashesEnd = m_piHashes + iEnd;
      TKey const * pkey = m_pkeys + iBegin;
      for (std::size_t const * piHash = piHashesBegin; piHash < piHashesEnd; ++piHash, ++pkey) {
         if (*piHash == iHash && keys_equal(*pkey == key)) {
            return static_cast<std::size_t>(piHash - piHashesBegin);
         }
      }
      return smc_iKeyNotFound;
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
   //! Neighborhood size.
   static std::size_t const smc_cNeighborhood = sizeof(std::size_t) * CHAR_BIT;
   //! Minimum bucket count.
   static std::size_t const smc_cBucketsMin = 8;
   //! Special hash value used to indicate that a bucket is empty.
   static std::size_t const smc_iEmptyBucket = 0;
   /*! Special index returned by scan_buckets_for_key() to indicate that the key could not be found
   in the specified range. */
   static std::size_t const smc_iKeyNotFound = numeric::max<std::size_t>::value;
   /*! Hash value substituted when the hash function returns 0; this is so we can use 0 (aliased by
   smc_iEmptyBucket) as a special value. This specific value is merely the largest prime number that
   will fit in 2^16, which is the (future, if ever) minimum word size supported by Abaclade. */
   static std::size_t const smc_iZeroHash = 65521;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_MAP_HXX

