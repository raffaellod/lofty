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
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/range.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::map_impl

namespace abc {
namespace detail {

//! Non-template implementation class for abc::map.
class ABACLADE_SYM map_impl {
public:
   /*! Constructor.

   @param m
      Source object.
   */
   map_impl();
   map_impl(map_impl && m);

   //! Destructor.
   ~map_impl();

   /*! Assignment operator.

   @param m
      Source object.
   */
   map_impl & operator=(map_impl && m);

   /*! Returns the maximum number of key/value pairs the map can currently hold.

   @return
      Current size of the allocated storage, in elements.
   */
   std::size_t capacity() const {
      return m_cBuckets;
   }

   /*! Returns the count of elements in the map.

   @return
      Count of elements.
   */
   std::size_t size() const {
      return m_cUsedBuckets;
   }

protected:
   /*! Finds the first (non-empty) bucket whose contents can be moved to the specified bucket.

   @param iEmptyBucket
      Index of the empty bucket, which is also the last bucket of the neighborhood to scan.
   @return
      Index of the first bucket whose contents can be moved, or smc_iNullIndex if none of the
      occupied buckets contain keys from the neighborhood ending at iEmptyBucket.
   */
   std::size_t find_bucket_movable_to_empty(std::size_t iEmptyBucket) const;

   /*! Returns the neighborhood index (index of the first bucket in a neighborhood) for the given
   hash.

   @param iHash
      Hash to get the neighborhood index for.
   @return
      Index of the first bucket in the neighborhood.
   */
   std::size_t hash_neighborhood_index(std::size_t iHash) const {
      return iHash & (m_cBuckets - 1);
   }

   /*! Returns the bucket index ranges for the neighborhood of the given hash.

   @param iHash
      Hash to return the neighborhood of.
   @return
      Calculated range for the neighborhood bucket index.
   */
   std::tuple<std::size_t, std::size_t> hash_neighborhood_range(std::size_t iHash) const;

   /*! Returns the current neighborhood size.

   @return
      Current neighborhood size, which is not necessarily the same as smc_cNeighborhoodBuckets.
   */
   std::size_t neighborhood_size() const;

protected:
   //! Array containing the hash of each key.
   std::unique_ptr<std::size_t[]> m_piHashes;
   //! Array of keys.
   std::unique_ptr<std::max_align_t[]> m_pKeys;
   //! Array of buckets.
   std::unique_ptr<std::max_align_t[]> m_pValues;
   //! Count of total buckets. Always a power of two.
   std::size_t m_cBuckets;
   //! Count of elements / occupied buckets.
   std::size_t m_cUsedBuckets;
   //! Minimum bucket count. Must be a power of 2.
   static std::size_t const smc_cBucketsMin = 8;
   //! Special hash value used to indicate that a bucket is empty.
   static std::size_t const smc_iEmptyBucketHash = 0;
   //! Hash table growth factor. Must be a power of 2.
   static std::size_t const smc_iGrowthFactor = 4;
   //! Neighborhood size.
   static std::size_t const smc_cNeighborhoodBuckets = sizeof(std::size_t) * CHAR_BIT;
   //! Special index returned by several methods to indicate a logical “null index”.
   static std::size_t const smc_iNullIndex = numeric::max<std::size_t>::value;
   /*! Hash value substituted when the hash function returns 0; this is so we can use 0 (aliased by
   smc_iEmptyBucketHash) as a special value. This specific value is merely the largest prime number
   that will fit in 2^16, which is the (future, if ever) minimum word size supported by Abaclade. */
   static std::size_t const smc_iZeroHash = 65521;
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::map

namespace abc {

//! Key/value map using a simplified hopscotch hashing collision resolution algorithm.
template <
   typename TKey,
   typename TValue,
   typename THasher = std::hash<TKey>,
   typename TKeyEqual = std::equal_to<TKey>
>
class map : public detail::map_impl, private THasher, private TKeyEqual {
public:
   //! Hash generator for TKey.
   typedef THasher hasher;
   //! Functor that can compare two TKey instances for equality.
   typedef TKeyEqual key_equal;

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
   map() {
   }
   map(map && m) :
      detail::map_impl(std::move(m)) {
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
      detail::map_impl::operator=(std::move(m));
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
         // TODO: provide more information in the exception.
         ABC_THROW(key_error, ());
      }
      return *value_ptr(iBucket);
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
      if (!m_cBuckets) {
         grow_table();
      }
      /* Repeatedly resize the table until we’re able to find a empty bucket for the new element.
      This should really only happen at most once. */
      while ((iBucket = get_existing_or_empty_bucket_for_key(key, iKeyHash)) == smc_iNullIndex) {
         grow_table();
      }

      std::size_t * piHash = &m_piHashes[iBucket];
      TValue * pvalue = value_ptr(iBucket);
      bool bNew = (*piHash == smc_iEmptyBucketHash);
      if (bNew) {
         // The bucket is currently empty, so initialize it with key, iKeyHash and value.
         *piHash = iKeyHash;
         new(key_ptr(iBucket)) TKey  (std::move(key  ));
         new(pvalue)           TValue(std::move(value));
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
      TKey   * pkey   = key_ptr  (0);
      TValue * pvalue = value_ptr(0);
      for (; piHash < piHashesEnd; ++piHash, ++pkey, ++pvalue) {
         if (*piHash != smc_iEmptyBucketHash) {
            *piHash = smc_iEmptyBucketHash;
            pkey  ->~TKey  ();
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
         // TODO: provide more information in the exception.
         ABC_THROW(key_error, ());
      }
      // Mark the bucket as empty and destruct the corresponding key and value.
      --m_cUsedBuckets;
      m_piHashes[iBucket] = smc_iEmptyBucketHash;
      key_ptr  (iBucket)->~TKey  ();
      value_ptr(iBucket)->~TValue();
   }

private:
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
      std::size_t iNhBegin, iNhEnd;
      std::tie(iNhBegin, iNhEnd) = hash_neighborhood_range(iKeyHash);
      // Look for the key or an empty bucket in the neighborhood.
      std::size_t iBucket = key_lookup(&key, iKeyHash, iNhBegin, iNhEnd, true);
      if (iBucket != smc_iNullIndex) {
         return iBucket;
      }
      // Find an empty bucket, scanning every bucket outside the neighborhood.
      std::size_t iEmptyBucket = key_lookup(nullptr, smc_iEmptyBucketHash, iNhEnd, iNhBegin, true);
      if (iEmptyBucket == smc_iNullIndex) {
         // No luck, the hash table needs to be resized.
         return smc_iNullIndex;
      }
      /* This loop will enter (and maybe repeat) if we have an empty bucket, but it’s not in the
      key’s neighborhood, so we have to try and move it in the neighborhood. The not-in-neighborhood
      check is made more complicated by the fact the range may wrap. */
      while (iNhBegin < iNhEnd
         ? iEmptyBucket >= iNhEnd || iEmptyBucket < iNhBegin // Non-wrapping: |---[begin end)---|
         : iEmptyBucket >= iNhEnd && iEmptyBucket < iNhBegin // Wrapping:     | end)-----[begin |
      ) {
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

   /*! Enlarges the hash table by a factor of smc_iGrowthFactor. The contents of each bucket are
   moved from the old arrays to new temporary ones, and the two array sets are then swapped.

   The bucket contents transfer work is done by reusing functions that obtain the arrays to
   operate on via member variables. In the assumption that transferring the contents of a bucket
   won’t throw because it only involves move-constructions and destructions, we optimistically
   update the member variables as soon as all memory allocations are done; if anything were to go
   wrong after that, we’d have no guaranteed-safe way of recovering from a half-transferred
   scenario anyway. */
   void grow_table() {
      // The “old” names of these four variables will make sense in a moment…
      std::size_t cOldBuckets = m_cBuckets ? m_cBuckets * smc_iGrowthFactor : smc_cBucketsMin;
      std::unique_ptr<std::size_t[]> piOldHashes(new std::size_t[cOldBuckets]);
      std::unique_ptr<std::max_align_t[]> pOldKeys(
         new std::max_align_t[ABC_ALIGNED_SIZE(sizeof(TKey) * cOldBuckets)]
      );
      std::unique_ptr<std::max_align_t[]> pOldValues(
         new std::max_align_t[ABC_ALIGNED_SIZE(sizeof(TValue) * cOldBuckets)]
      );
      // At this point we’re safe from exceptions, so we can update the member variables.
      std::swap(m_cBuckets, cOldBuckets);
      std::swap(m_piHashes, piOldHashes);
      std::swap(m_pKeys,    pOldKeys);
      std::swap(m_pValues,  pOldValues);
      // Now the names of these variables make sense :)

      // Initialize piNewHashes[i] with smc_iEmptyBucketHash.
      memory::clear(m_piHashes.get(), m_cBuckets);
      // Re-insert each hash/key/value triplet to move it from the old arrays to the new ones.
      std::size_t * piOldHash = piOldHashes.get(), * piOldHashesEnd = piOldHash + cOldBuckets;
      TKey   * pOldKey   = reinterpret_cast<TKey   *>(pOldKeys  .get());
      TValue * pOldValue = reinterpret_cast<TValue *>(pOldValues.get());
      for (; piOldHash < piOldHashesEnd; ++piOldHash, ++pOldKey, ++pOldValue) {
         if (*piOldHash != smc_iEmptyBucketHash) {
            std::size_t iNewBucket = get_existing_or_empty_bucket_for_key(*pOldKey, *piOldHash);
            ABC_ASSERT(
               iNewBucket != smc_iNullIndex,
               ABC_SL("failed to find empty bucket while growing hash table")
            );

            // Move hash/key/value to the new bucket.
            m_piHashes[iNewBucket] = *piOldHash;
            new(key_ptr  (iNewBucket)) TKey  (std::move(*pOldKey  ));
            new(value_ptr(iNewBucket)) TValue(std::move(*pOldValue));
            pOldKey  ->~TKey  ();
            pOldValue->~TValue();
         }
      }
   }

   /*! Looks for a specific key or an unused bucket (if bAcceptEmptyBucket is true) in the map.

   @param pkey
      Pointer to the key to lookup.
   @param iKeyHash
      Hash of *pkey.
   @param iNhBegin
      Beginning of the neighborhood bucket index range.
   @param iNhEnd
      End of the neighborhood bucket index range.
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
      std::size_t iNhBegin, iNhEnd;
      std::tie(iNhBegin, iNhEnd) = hash_neighborhood_range(iKeyHash);
      return key_lookup(pkey, iKeyHash, iNhBegin, iNhEnd, false);
   }
   /* This overload could be split in three different methods:

   1. Search for an empty bucket while also checking for a matching key: this would be used when
      looking for an add() insertion point while we’re still in the neighborhood of the key;
   2. Search for an empty bucket: this would cover the rest of the search needed by add();
   3. Search for a matching key: this would be used by the non-ranged key_lookup() overloads.

   The reason it’s not split is that most of the code is shared among these operation modes, so its
   instruction cache footprint is reduced. */
   std::size_t key_lookup(
      TKey const * pkey, std::size_t iKeyHash, std::size_t iNhBegin, std::size_t iNhEnd,
      bool bAcceptEmptyBucket
   ) const {
      /* Optimize away the check for bAcceptEmpty in the loop by comparing against iKeyHash (which
      the loop already does) if the caller desn’t want smc_iEmptyBucketHash. */
      std::size_t iAcceptableEmptyHash = bAcceptEmptyBucket ? smc_iEmptyBucketHash : iKeyHash;
      std::size_t const * piHash      = m_piHashes.get() + iNhBegin,
                        * piHashNhEnd = m_piHashes.get() + iNhEnd,
                        * piHashesEnd = m_piHashes.get() + m_cBuckets;
      /* iNhBegin - iNhEnd may be a wrapping range, so we can only test for inequality and rely on
      the wrap-around logic at the end of the loop body. Also, we need to iterate at least once,
      otherwise we won’t enter the loop at all if the start condition is the same as the end
      condition, which is the case for neighborhood_size() == m_cBuckets. */
      do {
         if (
            *piHash == iAcceptableEmptyHash ||
            /* Multiple calculations of the 2nd operand of the && should be rare enough (exact key
            match or hash collision) to make recalculating the offset from m_pKeys cheaper than
            keeping a cursor over m_pKeys running in parallel to piHash. */
            (*piHash == iKeyHash && keys_equal(key_ptr(piHash - m_piHashes.get()), pkey))
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

   /*! Returns a pointer to the key in the specified bucket index.

   @param i
      Bucket index.
   @return
      Pointer to the key.
   */
   TKey * key_ptr(std::size_t i) const {
      return reinterpret_cast<TKey *>(m_pKeys.get()) + i;
   }

   /*! Compares two keys for equality.

   @param key1
      Pointer to the first key to compare.
   @param key2
      Pointer to the second key to compare.
   @return
      true if the two keys compare equal, or false otherwise.
   */
   bool keys_equal(TKey const * key1, TKey const * key2) const {
      return key_equal::operator()(*key1, *key2);
   }

   /*! Moves the contents of one bucket to another bucket.

   @param iSrcBucket
      Index of the source bucket.
   @param iDstBucket
      Index of the destination bucket.
   */
   void move_bucket_contents(std::size_t iSrcBucket, std::size_t iDstBucket) {
      m_piHashes[iDstBucket] = m_piHashes[iSrcBucket];
      new(key_ptr  (iDstBucket)) TKey  (std::move(*key_ptr  (iSrcBucket)));
      new(value_ptr(iDstBucket)) TValue(std::move(*value_ptr(iSrcBucket)));
   }

   /*! Returns a pointer to the value in the specified bucket index.

   @param i
      Bucket index.
   @return
      Pointer to the value.
   */
   TValue * value_ptr(std::size_t i) const {
      return reinterpret_cast<TValue *>(m_pValues.get()) + i;
   }
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_MAP_HXX
