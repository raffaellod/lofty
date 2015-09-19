/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABACLADE_COLLECTIONS_DETAIL_HASH_MAP_IMPL_HXX
#define _ABACLADE_COLLECTIONS_DETAIL_HASH_MAP_IMPL_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/numeric.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declaration.
namespace abc {

class type_void_adapter;

} //namespace abc

namespace abc { namespace collections { namespace detail {

//! Non-template implementation class for abc::collections::hash_map.
class ABACLADE_SYM hash_map_impl :
   public support_explicit_operator_bool<hash_map_impl> {
protected:
   typedef bool (* keys_equal_fn)(
      hash_map_impl const * phmi, void const * pKey1, void const * pKey2
   );

   //! Integer type used to track changes in the map.
   typedef std::uint16_t rev_int_t;

   //! Base class for hash_map iterator implementations.
   class ABACLADE_SYM iterator_base {
   private:
      friend class hash_map_impl;

   public:
      typedef _std::forward_iterator_tag iterator_category;

   public:
      //! Default constructor.
      iterator_base();

      /*! Constructor.

      @param phm
         Pointer to the map owning the iterated objects.
      @param iBucket
         Index of the current bucket.
      */
      iterator_base(hash_map_impl const * phm, std::size_t iBucket);

      /*! Equality relational operator.

      @param it
         Object to compare to *this.
      @return
         true if *this is an iterator to the same key/value pair as it, or false otherwise.
      */
      bool operator==(iterator_base const & it) const {
         ABC_TRACE_FUNC(this/*, it*/);

         return m_phm == it.m_phm && m_iBucket == it.m_iBucket;
      }

      /*! Inequality relational operator.

      @param it
         Object to compare to *this.
      @return
         true if *this has a different key/value pair than it, or false otherwise.
      */
      bool operator!=(iterator_base const & it) const {
         ABC_TRACE_FUNC(this/*, it*/);

         return !operator==(it);
      }

   protected:
      //! Moves the iterator to next used bucket.
      void increment();

      /*! Throws a collections::out_of_range exception if the iterator is at the end of the
      container or has been invalidated by a change in the container. */
      void validate() const;

   protected:
      //! Pointer to the map to iterate over.
      hash_map_impl const * m_phm;
      //! Current bucket index.
      std::size_t m_iBucket;
      //! Last container revision number known to the iterator.
      rev_int_t m_iRev;
   };

public:
   //! Default constructor.
   hash_map_impl();

   /*! Move constructor.

   @param hmi
      Source object.
   */
   hash_map_impl(hash_map_impl && hmi);

   //! Destructor.
   ~hash_map_impl();

   /*! Move-assignment operator.

   @param hmi
      Source object.
   @return
      *this.
   */
   hash_map_impl & operator=(hash_map_impl && hmi);

   /*! Boolean evaluation operator.

   @return
      true if the map is not empty, or false otherwise.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return m_cUsedBuckets > 0;
   }

   /*! Returns the maximum number of key/value pairs the map can currently hold.

   @return
      Current size of the allocated storage, in elements.
   */
   std::size_t capacity() const {
      return m_cBuckets;
   }

   /*! Returns true if the map contains no elements.

   @return
      true if the map is empty, or false otherwise.
   */
   bool empty() const {
      return m_cUsedBuckets == 0;
   }

   /*! Returns the current neighborhood size.

   @return
      Current neighborhood size.
   */
   std::size_t neighborhood_size() const {
      return m_cNeighborhoodBuckets;
   }

   /*! Returns the count of elements in the map.

   @return
      Count of elements.
   */
   std::size_t size() const {
      return m_cUsedBuckets;
   }

protected:
   /*! Returns the index of the bucket matching the specified key, or locates an empty bucket and
   returns its index after moving it in the key’s neighborhood.

   @param typeKey
      Adapter for the key type.
   @param typeValue
      Adapter for the value type.
   @param pfnKeysEqual
      Pointer to a function that returns true if two keys compare as equal.
   @param pKey
      Pointer to the key to add.
   @param iKeyHash
      Hash of *pKey.
   @param pValue
      Pointer to the value to add.
   @param iMove
      Bitmask; 1 (bit 0) indicates that *pKey should be moved, while 2 (bit 1) indicates that
      *pValue should be moved.
   @return
      Pair containing the index of the newly-occupied bucket and a bool value that is true if the
      key/value pair was just added, or false if the key already existed in the map and the
      corresponding value was overwritten.
   */
   _std::tuple<std::size_t, bool> add_or_assign(
      type_void_adapter const & typeKey, type_void_adapter const & typeValue,
      keys_equal_fn pfnKeysEqual, void * pKey, std::size_t iKeyHash, void * pValue, unsigned iMove
   );

   /*! Removes all elements from the map.

   @param typeKey
      Adapter for the key type.
   @param typeValue
      Adapter for the value type.
   */
   void clear(type_void_adapter const & typeKey, type_void_adapter const & typeValue);

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
   _std::tuple<std::size_t, std::size_t> hash_neighborhood_range(std::size_t iHash) const {
      std::size_t iNhBegin = hash_neighborhood_index(iHash);
      std::size_t iNhEnd = iNhBegin + m_cNeighborhoodBuckets;
      // Wrap the end index back in the table.
      iNhEnd &= m_cBuckets - 1;
      return _std::make_tuple(iNhBegin, iNhEnd);
   }

   /*! Marks a bucket as empty and destructs the corresponding key and value.

   @param typeKey
      Adapter for the key type.
   @param typeValue
      Adapter for the value type.
   @param it
      Iterator to the bucket to empty.
   */
   void empty_bucket(
      type_void_adapter const & typeKey, type_void_adapter const & typeValue, iterator_base it
   ) {
      ABC_TRACE_FUNC(this/*, typeKey, typeValue, it*/);

      it.validate();
      empty_bucket(typeKey, typeValue, it.m_iBucket);
   }

   /*! Marks a bucket as empty and destructs the corresponding key and value.

   @param typeKey
      Adapter for the key type.
   @param typeValue
      Adapter for the value type.
   @param iBucket
      Index of the bucket to empty.
   */
   void empty_bucket(
      type_void_adapter const & typeKey, type_void_adapter const & typeValue, std::size_t iBucket
   );

private:
   /*! Finds the first (non-empty) bucket whose contents can be moved to the specified bucket.

   @param iEmptyBucket
      Index of the empty bucket, which is also the last bucket of the neighborhood to scan.
   @return
      Index of the first bucket whose contents can be moved, or a special value if none of the
      occupied buckets contains a key from the neighborhood ending at iEmptyBucket.
   */
   std::size_t find_bucket_movable_to_empty(std::size_t iEmptyBucket) const;

   /*! Looks for an empty bucket in the specified bucket range.

   @param iNhBegin
      Beginning of the neighborhood bucket index range.
   @param iNhEnd
      End of the neighborhood bucket index range.
   @return
      Index of the first empty bucket found, or smc_iNullIndex if no empty buckets were found.
   */
   std::size_t find_empty_bucket(std::size_t iNhBegin, std::size_t iNhEnd) const;

   /*! Looks for an empty bucket outsize the specified bucket range.

   @param typeKey
      Adapter for the key type.
   @param typeValue
      Adapter for the value type.
   @param iNhBegin
      Beginning of the neighborhood bucket index range.
   @param iNhEnd
      End of the neighborhood bucket index range.
   @return
      Index of a bucket that has been emptied by moving its contents outside the neighborhood, or
      smc_iNullIndex if no movable buckets could be found.
   */
   std::size_t find_empty_bucket_outside_neighborhood(
      type_void_adapter const & typeKey, type_void_adapter const & typeValue, std::size_t iNhBegin,
      std::size_t iNhEnd
   );

   /*! Locates an empty bucket where the specified key may be stored, and returns its index after
   moving it in the key’s neighborhood.

   @param typeKey
      Adapter for the key type.
   @param typeValue
      Adapter for the value type.
   @param iKeyHash
      Hash of *pKey.
   @return
      Index of the bucket for the specified key. If key no empty bucket can be found or moved in the
      key’s neighborhood, the returned index is smc_iNullIndex.
   */
   std::size_t get_empty_bucket_for_key(
      type_void_adapter const & typeKey, type_void_adapter const & typeValue, std::size_t iKeyHash
   );

   /*! Returns the index of the bucket matching the specified key, or locates an empty bucket and
   returns its index after moving it in the key’s neighborhood.

   @param typeKey
      Adapter for the key type.
   @param typeValue
      Adapter for the value type.
   @param pfnKeysEqual
      Pointer to a function that returns true if two keys compare as equal.
   @param pKey
      Pointer to the key to lookup.
   @param iKeyHash
      Hash of *pKey.
   @return
      Index of the bucket for the specified key. If key is not already in the map and no empty
      bucket can be moved in the key’s neighborhood, the returned index is smc_iNullIndex.
   */
   std::size_t get_existing_or_empty_bucket_for_key(
      type_void_adapter const & typeKey, type_void_adapter const & typeValue,
      keys_equal_fn pfnKeysEqual, void const * pKey, std::size_t iKeyHash
   );

   /*! Enlarges the neighborhood size by a factor of smc_iGrowthFactor. This does not require moving
   the contents of any buckets, since buckets will still be part of the correct neighborhood. */
   void grow_neighborhoods() {
      m_cNeighborhoodBuckets *= smc_iGrowthFactor;
   }

   /*! Enlarges the hash table by a factor of smc_iGrowthFactor. The contents of each bucket are
   moved from the old arrays to new temporary ones, and the two array sets are then swapped.

   The bucket contents transfer work is done by reusing functions that obtain the arrays to operate
   on via member variables. In the assumption that transferring the contents of a bucket won’t throw
   because it only involves move-constructions and destructions, we optimistically update the member
   variables as soon as all memory allocations are done; if anything were to go wrong after that,
   we’d have no guaranteed-safe way of recovering from a half-transferred scenario anyway.

   @param typeKey
      Adapter for the key type.
   @param typeValue
      Adapter for the value type.
   */
   void grow_table(type_void_adapter const & typeKey, type_void_adapter const & typeValue);

   /*! Looks for a specific key or an unused bucket in the map.

   @param typeKey
      Adapter for the key types.
   @param pfnKeysEqual
      Pointer to a function that returns true if two keys compare as equal.
   @param pKey
      Pointer to the key to lookup.
   @param iKeyHash
      Hash of *pKey.
   @param iNhBegin
      Beginning of the neighborhood bucket index range.
   @param iNhEnd
      End of the neighborhood bucket index range.
   @return
      Index of the bucket at which the key could be found, or index of the first empty bucket found,
      or smc_iNullIndex if neither could be found.
   */
   std::size_t lookup_key_or_find_empty_bucket(
      type_void_adapter const & typeKey, keys_equal_fn pfnKeysEqual, void const * pKey,
      std::size_t iKeyHash, std::size_t iNhBegin, std::size_t iNhEnd
   ) const;

   /*! Copies or moves a value, and optionally a key, to the specified bucket.

   @param typeKey
      Adapter for the key type.
   @param typeValue
      Adapter for the value type.
   @param iBucket
      Index of the destination bucket.
   @param pKey
      Pointer to the key to move to the destination bucket. May be nullptr if the key doesn’t need
      to be changed because it’s already correct.
   @param pValue
      Pointer to the value to move to the destination bucket.
   @param iMove
      Bitmask; 1 (bit 0) indicates that *pKey should be moved, while 2 (bit 1) indicates that
      *pValue should be moved.
   */
   void set_bucket_key_value(
      type_void_adapter const & typeKey, type_void_adapter const & typeValue, std::size_t iBucket,
      void * pKey, void * pValue, unsigned iMove
   );

protected:
   //! Array containing the hash of each key.
   _std::unique_ptr<std::size_t[]> m_piHashes;
   //! Array of keys.
   _std::unique_ptr<void, memory::freeing_deleter> m_pKeys;
   //! Array of buckets.
   _std::unique_ptr<void, memory::freeing_deleter> m_pValues;
   //! Count of total buckets. Always a power of two.
   std::size_t m_cBuckets;
   //! Count of elements / occupied buckets.
   std::size_t m_cUsedBuckets;
   /*! Neighborhood size. The map will try to keep this to smc_cIdealNeighborhoodBuckets, but the
   actual value may be smaller if the table is too small, or larger if the hash function results in
   too many collisions. In the worst case, this will be the same as m_cBuckets. */
   std::size_t m_cNeighborhoodBuckets;
   //! Indicates the revision number of the map contents.
   rev_int_t m_iRev;

   //! Minimum bucket count. Must be a power of 2.
   static std::size_t const smc_cBucketsMin = 8;
   //! Special hash value used to indicate that a bucket is empty.
   static std::size_t const smc_iEmptyBucketHash = 0;
   //! Hash table or neighborhood growth factor. Must be a power of 2.
   static std::size_t const smc_iGrowthFactor = 4;
   //! Default/ideal neighborhood size.
   static std::size_t const smc_cIdealNeighborhoodBuckets;
   /*! Hash value substituted when the hash function returns 0; this is so we can use 0 (aliased by
   smc_iEmptyBucketHash) as a special value. This specific value is merely the largest prime number
   that will fit in 2^16, which is the (future, if ever) minimum word size supported by Abaclade. */
   static std::size_t const smc_iZeroHash = 65521;

   //! First special index value.
   static std::size_t const smc_iFirstSpecialIndex = numeric::max<std::size_t>::value - 8;
   /*! Special value returned by find_bucket_movable_to_empty() to indicate that the neighborhood
   size needs to be increased before trying again. */
   static std::size_t const smc_iNeedLargerNeighborhoods = numeric::max<std::size_t>::value - 2;
   /*! Special value returned by find_bucket_movable_to_empty() to indicate that the hash table size
   needs to be increased before trying again. */
   static std::size_t const smc_iNeedLargerTable = numeric::max<std::size_t>::value - 1;
   /*! Special index returned by several methods to indicate a logical “null index”. Code in
   iterator_base::increment() relies on smc_iNullIndex + 1 == 0. */
   static std::size_t const smc_iNullIndex = numeric::max<std::size_t>::value;
};

}}} //namespace abc::collections::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COLLECTIONS_DETAIL_HASH_MAP_IMPL_HXX
