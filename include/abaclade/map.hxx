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

//! Index range.
class index_range : public support_explicit_operator_bool<index_range> {
public:
   /*! Constructor.

   @param iBegin
      Index of the first index in the range.
   @param iEnd
      Index beyond the last index in the range.
   */
   index_range() :
      m_iBegin(0),
      m_iEnd(0) {
   }
   index_range(std::size_t iBegin, std::size_t iEnd) :
      m_iBegin(iBegin),
      m_iEnd(iEnd) {
   }

   /*! Boolean evaluation operator.

   @return
      true if the range is non-empty, or false if it’s empty.
   */
   explicit_operator_bool() const {
      return !empty();
   }

   /*! Returns the interval not included in the range, defined as [ end(), begin() ).

   @return
      Inverted range.
   */
   index_range operator~() const {
      return index_range(m_iEnd, m_iBegin);
   }

   /*! Returns the start of the range.

   @return
      First index in the range.
   */
   std::size_t begin() const {
      return m_iBegin;
   }

   /*! Returns true if the range is empty.

   @return
      true if the range is empty, or false otherwise.
   */
   bool empty() const {
      return m_iBegin == m_iEnd;
   }

   /*! Returns the end of the range.

   @return
      Index beyond the last in the range.
   */
   std::size_t end() const {
      return m_iEnd;
   }

private:
   //! Index of the first index in the range.
   std::size_t m_iBegin;
   //! Index beyond the last index in the range.
   std::size_t m_iEnd;
};

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
      std::size_t iBucket = key_lookup(&key);
      if (iBucket == smc_iNullIndex) {
         // TODO: throw proper exception.
         throw 0;
      }
      return *get_value_ptr(iBucket);
   }

   /*! Adds a key/value pair to the map, overwriting the value if key is already associated to one.

   @param key
      Key to add.
   @param value
      Value to add.
   @return
      Pair containing an iterator to the newly added key/value, and a bool value that is true if the
      key/value pair was just added, or false if the key already existed in the map and the
      corresponding value was overwritten.
   */
   std::pair<iterator, bool> add(TKey key, TValue value) {
      std::size_t iKeyHash = calculate_and_adjust_hash(key), iBucket;
      /* Repeatedly resize the table until we’re able to find a empty bucket for the new element.
      This should really only happen at most once. */
      while ((iBucket = get_existing_or_empty_bucket_for_key(key, iKeyHash)) == smc_iNullIndex) {
         if (m_cBuckets) {
            // TODO: resize the hash table.
         } else {
            create_empty_buckets();
         }
      }

      std::size_t * piHash = &m_piHashes[iBucket];
      TValue * pvalue = get_value_ptr(iBucket);
      bool bNew = (*piHash == smc_iEmptyBucketHash);
      if (bNew) {
         // The bucket is currently empty, so initialize it with key, iKeyHash and value.
         new(get_key_ptr(iBucket)) TKey(std::move(key));
         *piHash = iKeyHash;
         new(pvalue) TValue(std::move(value));
      } else {
         // The bucket already has a value, so overwrite it with the value argument.
         *pvalue = std::move(value);
      }
      ++m_cUsedBuckets;
      return std::make_pair(iterator(this, iBucket), bNew);
   }

   //! Removes all elements from the map.
   void clear() {
      std::size_t * piHash = m_piHashes.get(), * piHashesEnd = piHash + m_cBuckets;
      TKey   * pkey   = get_key_ptr  (0);
      TValue * pvalue = get_value_ptr(0);
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
      std::size_t iBucket = key_lookup(&key);
      if (iBucket == smc_iNullIndex) {
         // TODO: throw proper exception.
         throw 0;
      }
      // Mark the bucket as empty and destruct the corresponding key and value.
      m_piHashes[iBucket] = smc_iEmptyBucketHash;
      get_key_ptr  (iBucket)->~TKey  ();
      get_value_ptr(iBucket)->~TValue();
      --m_cUsedBuckets;
   }

   /*! Returns the count of elements in the map.

   @return
      Count of elements.
   */
   std::size_t size() const {
      return m_cUsedBuckets;
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
      // Initialize m_piHashes[i] with smc_iEmptyBucketHash.
      memory::clear(m_piHashes.get(), cBuckets);
   }

   /*! Looks for a specific key or an unused bucket (if bAcceptEmptyBucket is true) in the map.

   @param pkey
      Pointer to the key to lookup.
   @param iKeyHash
      Hash of *pkey.
   @param irNeighborhood
      Nighborhood bucket index range.
   @param bAcceptEmptyBucket
      If true, an empty bucket will be considered a match, and its index returned.
   @return
      Index of the bucket at which the key could be found, or smc_iNullIndex if the key was not
      found.
   */
   std::size_t key_lookup(TKey const * pkey) const {
      return key_lookup(pkey, calculate_and_adjust_hash(*pkey));
   }
   std::size_t key_lookup(TKey const * pkey, std::size_t iKeyHash) const {
      if (m_cBuckets == 0) {
         // The key cannot possibly be in the map.
         return smc_iNullIndex;
      }
      return key_lookup(pkey, iKeyHash, get_hash_neighborhood_range(iKeyHash), false);
   }
   /* This overload could be split in three different methods:

   1. Search for an empty bucket while also checking for a matching key: this would be used when
      looking for an add() insertion point while we’re still in the neighborhood of the key;
   2. Search for an empty bucket: this would cover the rest of the search needed by add();
   3. Search for a matching key: this would be used by the non-ranged key_lookup() overloads.

   The reason it’s not split is that most of the code is shared among these operation modes, so its
   instruction cache footprint is reduced. */
   std::size_t key_lookup(
      TKey const * pkey, std::size_t iKeyHash, index_range irNeighborhood, bool bAcceptEmptyBucket
   ) const {
      /* Optimize away the check for bAcceptEmpty in the loop by comparing against iKeyHash (which
      the loop already does) if the caller desn’t want smc_iEmptyBucketHash. */
      std::size_t iAcceptableEmptyHash = bAcceptEmptyBucket ? smc_iEmptyBucketHash : iKeyHash;
      std::size_t const * piHash      = m_piHashes.get() + irNeighborhood.begin(),
                        * piHashNhEnd = m_piHashes.get() + irNeighborhood.end(),
                        * piHashesEnd = m_piHashes.get() + m_cBuckets;
      /* irNeighborhood may be a wrapping range, so we can only test for inequality and rely on the
      wrap-around logic at the end of the loop body. */
      while (piHash != piHashNhEnd) {
         if (
            *piHash == iAcceptableEmptyHash ||
            /* Multiple calculations of the 2nd operand of the && should be rare enough (exact key
            match or hash collision) to make recalculating the offset from m_pkeys cheaper than
            keeping a cursor over m_pkeys running in parallel to piHash. */
            (*piHash == iKeyHash && keys_equal(get_key_ptr(piHash - m_piHashes.get()), pkey))
         ) {
            return static_cast<std::size_t>(piHash - m_piHashes.get());
         }

         // Move on to the next bucket, wrapping around to the first one if needed.
         if (++piHash == piHashesEnd) {
            piHash = m_piHashes.get();
         }
      }
      return smc_iNullIndex;
   }

   /*! Calculates, adjusts and returns the hash value for the specified key.

   @param key
      Key to calculate a hash value for.
   @return
      Hash value of key.
   */
   std::size_t calculate_and_adjust_hash(TKey const & key) const {
      std::size_t iHash = hasher::operator()(key);
      return iHash == smc_iEmptyBucketHash ? smc_iZeroHash : iHash;
   }

   /*! Finds the first (non-empty) bucket whose contents can be moved to the specified bucket.

   @param iEmptyBucket
      Index of the empty bucket, which is also the last bucket of the neighborhood to scan.
   @return
      Index of the first bucket whose contents can be moved, or smc_iNullIndex if none of the
      occupied buckets contain keys from the neighborhood ending at iEmptyBucket.
   */
   std::size_t find_bucket_movable_to_empty(std::size_t iEmptyBucket) const {
      std::size_t cNeighborhoodBuckets = get_neighborhood_size();
      std::size_t cBucketsRightOfEmpty = cNeighborhoodBuckets - 1;
      // Ensure that iEmptyBucket will be on the right of any of the buckets we’re going to check.
      if (iEmptyBucket < cBucketsRightOfEmpty) {
         iEmptyBucket += m_cBuckets;
      }
      // Calculate the bucket index range of the neighborhood that ends with iEmptyBucket.
      std::size_t const * piEmptyHash = m_piHashes.get() + (iEmptyBucket & (m_cBuckets - 1)),
                        * piHash      = piEmptyHash - cBucketsRightOfEmpty,
                        * piHashesEnd = m_piHashes.get() + m_cBuckets;
      /* The neighborhood may wrap, so we can only test for inequality and rely on the wrap-around
      logic at the end of the loop body. */
      while (piHash != piEmptyHash) {
         /* Get the end of the original neighborhood for the key in this bucket; if the empty bucket
         is within that index, the contents of this bucket can be moved to the empty one. */
         std::size_t iCurrNhEnd = get_hash_neighborhood_index(*piHash) + cNeighborhoodBuckets;
         /* Both indices are allowed to be >m_cBuckets (see earlier if), so this comparison is
         always valid. */
         if (iEmptyBucket < iCurrNhEnd) {
            return static_cast<std::size_t>(piHash - m_piHashes.get());
         }

         // Move on to the next bucket, wrapping around to the first one if needed.
         if (++piHash == piHashesEnd) {
            piHash = m_piHashes.get();
         }
      }
      // No luck, the hash table needs to be resized.
      return smc_iNullIndex;
   }

   /*! Returns the index of the bucket matching the specified key, or locates an empty bucket and
   returns its index after moving it in the key’s neighborhood.

   @param key
      Key to lookup.
   @param iKeyHash
      Hash of key.
   @return
      Index of the bucket for the specified key. If key is not already in the map and no empty
      bucket can be moved in key’s neighborhood, the returned index is smc_iNullIndex.
   */
   std::size_t get_existing_or_empty_bucket_for_key(TKey const & key, std::size_t iKeyHash) {
      auto irNeighborhood(get_hash_neighborhood_range(iKeyHash));
      // Look for the key or an empty bucket in the neighborhood.
      std::size_t iBucket = key_lookup(&key, iKeyHash, irNeighborhood, true);
      if (iBucket != smc_iNullIndex) {
         return iBucket;
      }
      // Find an empty bucket, scanning every bucket outside the neighborhood.
      std::size_t iEmptyBucket = key_lookup(nullptr, smc_iEmptyBucketHash, ~irNeighborhood, true);
      if (iEmptyBucket == smc_iNullIndex) {
         // No luck, the hash table needs to be resized.
         return smc_iNullIndex;
      }
      /* This loop will enter (and maybe repeat) if we have an empty bucket, but it’s not in the
      key’s neighborhood, so we have to try and move it in the neighborhood. */
      while (iEmptyBucket < irNeighborhood.begin() || iEmptyBucket >= irNeighborhood.end()) {
         /* The empty bucket is out of the neighborhood. Find the first non-empty bucket that’s part
         of the left-most neighborhood containing iEmptyBucket, but excluding buckets occupied by
         keys belonging to other overlapping neighborhoods. */
         std::size_t iMovableBucket = find_bucket_movable_to_empty(iEmptyBucket);
         if (iMovableBucket == smc_iNullIndex) {
            /* No buckets have contents that can be moved to iEmptyBucket; the hash table needs to
            be resized. */
            return smc_iNullIndex;
         }
         // Move the contents of iMovableBucket to iEmptyBucket.
         move_bucket_contents(iMovableBucket, iEmptyBucket);
         iEmptyBucket = iMovableBucket;
      }
      return iEmptyBucket;
   }

   TKey * get_key_ptr(std::size_t i) const {
      return reinterpret_cast<TKey *>(m_pkeys.get()) + i;
   }

   /*! Returns the current neighborhood size.

   @return
      Current neighborhood size, which is not necessarily the same as smc_cNeighborhoodBuckets.
   */
   std::size_t get_neighborhood_size() const {
      // Can’t have a neighborhood larger than the total count of buckets.
      return m_cBuckets < smc_cNeighborhoodBuckets ? m_cBuckets : smc_cNeighborhoodBuckets;
   }

   TValue * get_value_ptr(std::size_t i) const {
      return reinterpret_cast<TValue *>(m_pvalues.get()) + i;
   }

   /*! Returns the neighborhood index (index of the first bucket in a neighborhood) for the given
   hash.

   @param iHash
      Hash to get the neighborhood index for.
   @return
      Index of the first bucket in the neighborhood.
   */
   std::size_t get_hash_neighborhood_index(std::size_t iHash) const {
      return iHash & (m_cBuckets - 1);
   }

   /*! Returns the bucket index ranges for the neighborhood of the given hash.

   @param iHash
      Hash to return the neighborhood of.
   @return
      Calculated range for the neighborhood bucket index.
   */
   index_range get_hash_neighborhood_range(std::size_t iHash) const {
      std::size_t iNeighborhoodBegin = get_hash_neighborhood_index(iHash);
      std::size_t iNeighborhoodEnd = iNeighborhoodBegin + get_neighborhood_size();
      // Wrap the end index back in the table.
      iNeighborhoodEnd &= m_cBuckets - 1;
      return index_range(iNeighborhoodBegin, iNeighborhoodEnd);
   }

   bool keys_equal(TKey const * key1, TKey const * key2) const {
      return *key1 == *key2;
   }

   /*! Moves the contents of one bucket to another bucket.

   @param iSrcBucket
      Index of the source bucket.
   @param iDstBucket
      Index of the destination bucket.
   */
   void move_bucket_contents(std::size_t iSrcBucket, std::size_t iDstBucket) {
      m_piHashes[iDstBucket] = m_piHashes[iSrcBucket];
      new(get_key_ptr  (iDstBucket)) TKey  (std::move(*get_key_ptr  (iSrcBucket)));
      new(get_value_ptr(iDstBucket)) TValue(std::move(*get_value_ptr(iSrcBucket)));
   }

private:
   //! Array containing the hash of each key.
   std::unique_ptr<std::size_t[]> m_piHashes;
   //! Array of keys.
   std::unique_ptr<std::max_align_t[]> m_pkeys;
   //! Array of buckets.
   std::unique_ptr<std::max_align_t[]> m_pvalues;
   //! Count of total buckets. Always a power of two.
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
