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
      return iterator(this, 0);
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
      m_cBuckets = cBuckets;
      m_piHashes.reset(new std::size_t[m_cBuckets]);
      m_pkeys.reset(new std::max_align_t[ABC_ALIGNED_SIZE(sizeof(TKey) * m_cBuckets)]);
      m_pvalues.reset(new std::max_align_t[ABC_ALIGNED_SIZE(sizeof(TValue) * m_cBuckets)]);
   }

   std::size_t bucket_index_from_key(TKey const & key) const {
      return bucket_index_from_key(key, get_and_adjust_hash(key));
   }
   std::size_t bucket_index_from_key(TKey const & key, std::size_t iHash) const {
      // Get a range of indices representing the neighborhood.
      std::size_t iNeighborhoodBegin = neighborhood_index_from_hash(iHash);
      std::size_t iNeighborhoodEnd = iNeighborhoodBegin + get_neighborhood_size();
      /* Determine if we’ll need to check any buckets at the beginning of the array due to the
      neighborhood wrapping. */
      std::size_t iWrappedNeighborhoodEnd;
      if (iNeighborhoodEnd > m_cBuckets) {
         iWrappedNeighborhoodEnd = iNeighborhoodEnd - m_cBuckets;
         iNeighborhoodEnd = m_cBuckets;
      } else {
         // Make the wrapped interval [0, 0), i.e. empty.
         iWrappedNeighborhoodEnd = 0;
      }
      // Scan till the end of the neighborhood (clipped to the end of the array).
      std::size_t iBucket = bucket_index_from_key_and_bucket_range(
         key, iHash, iNeighborhoodBegin, iNeighborhoodEnd
      );
      if (iBucket == smc_iNullIndex && iWrappedNeighborhoodEnd) {
         /* This neighborhood wraps back to the start of the array, so we have additional buckets to
         scan. */
         iBucket = bucket_index_from_key_and_bucket_range(key, iHash, 0, iWrappedNeighborhoodEnd);
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
      if (!m_cBuckets) {
         // No buckets, so iHash cannot be in the map.
         return smc_iNullIndex;
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
      Index of the bucket at which the key could be found, or smc_iNullIndex if the key was not
      found.
   */
   std::size_t bucket_index_from_key_and_bucket_range(
      TKey const & key, std::size_t iHash, std::size_t iBegin, std::size_t iEnd
   ) const {
      std::size_t const * piHashesBegin = m_piHashes + iBegin, * piHashesEnd = m_piHashes + iEnd;
      TKey const * pkey = m_pkeys + iBegin;
      for (std::size_t const * piHash = piHashesBegin; piHash < piHashesEnd; ++piHash, ++pkey) {
         if (*piHash == iHash && keys_equal(*pkey == key)) {
            return static_cast<std::size_t>(piHash - piHashesBegin);
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

