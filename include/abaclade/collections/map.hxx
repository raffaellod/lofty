/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
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

#ifndef _ABACLADE_COLLECTIONS_MAP_HXX
#define _ABACLADE_COLLECTIONS_MAP_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/numeric.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::detail::map_impl

namespace abc {
namespace collections {
namespace detail {

//! Non-template implementation class for abc::collections::map.
class ABACLADE_SYM map_impl :
   public support_explicit_operator_bool<list_impl> {
protected:
   typedef bool (* keys_equal_fn)(map_impl const * pmapi, void const * pKey1, void const * pKey2);

   //! Integer type used to track changes in the map.
   typedef std::uint16_t rev_int_t;

   //! Iterator type for const key/value pairs.
   class ABACLADE_SYM iterator_base {
   private:
      friend class map_impl;

   public:
      /*! Constructor.

      @param pmap
         Pointer to the map owning the iterated objects.
      @param iBucket
         Index of the current bucket.
      */
      iterator_base();
      iterator_base(map_impl const * pmap, std::size_t iBucket);

      /*! Equality relational operator.

      @param it
         Object to compare to *this.
      @return
         true if *this is an iterator to the same key/value pair as it, or false otherwise.
      */
      bool operator==(iterator_base const & it) const {
         ABC_TRACE_FUNC(this/*, it*/);

         return m_pmap == it.m_pmap && m_iBucket == it.m_iBucket;
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

      /*! Throws an iterator_error exception if the iterator is at the end of the container or has
      been invalidated by a change in the container. */
      void validate() const;

   protected:
      //! Pointer to the map to iterate over.
      map_impl const * m_pmap;
      //! Current bucket index.
      std::size_t m_iBucket;
      //! Last container revision number known to the iterator.
      rev_int_t m_iRev;
   };

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

   /*! Returns true if the map size is greater than 0.

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
      return !m_cUsedBuckets;
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
   std::pair<std::size_t, bool> add_or_assign(
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
   std::tuple<std::size_t, std::size_t> hash_neighborhood_range(std::size_t iHash) const {
      std::size_t iNhBegin = hash_neighborhood_index(iHash);
      std::size_t iNhEnd = iNhBegin + m_cNeighborhoodBuckets;
      // Wrap the end index back in the table.
      iNhEnd &= m_cBuckets - 1;
      return std::make_tuple(iNhBegin, iNhEnd);
   }

   /*! Marks a bucket as empty and destructs the corresponding key and value.

   @param typeKey
      Adapter for the key type.
   @param typeValue
      Adapter for the value type.
   @param it
      Iterator to the bucket to empty.
   @param iBucket
      Index of the bucket to empty.
   */
   void empty_bucket(
      type_void_adapter const & typeKey, type_void_adapter const & typeValue, iterator_base it
   ) {
      ABC_TRACE_FUNC(this/*, typeKey, typeValue, it*/);

      it.validate();
      empty_bucket(typeKey, typeValue, it.m_iBucket);
   }
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
   std::unique_ptr<std::size_t[]> m_piHashes;
   //! Array of keys.
   std::unique_ptr<void, memory::freeing_deleter> m_pKeys;
   //! Array of buckets.
   std::unique_ptr<void, memory::freeing_deleter> m_pValues;
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

} //namespace detail
} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::map

namespace abc {
namespace collections {

/*! Key/value map using a derivative of the hopscotch hashing collision resolution algorithm.

This implementation uses a variable hash table size (number of buckets) to deal with varying item
counts, as well as a variable neighborhood size (number of buckets sharing the same logical index)
in order to tolerate high-collision hash functions. */
template <
   typename TKey,
   typename TValue,
   typename THasher = std::hash<TKey>,
   typename TKeyEqual = std::equal_to<TKey>
>
class map : public detail::map_impl, private THasher, private TKeyEqual {
public:
   //! Key type.
   typedef TKey key_type;
   //! Mapped value type.
   typedef TValue mapped_type;
   //! Hash generator for TKey.
   typedef THasher hasher;
   //! Functor that can compare two TKey instances for equality.
   typedef TKeyEqual key_equal;

   /*! Pointer type returned by iterator::operator->() that behaves like a pointer, but in fact
   includes the object it points to.

   Needed because iterator::operator->() must return a pointer-like type to a key/value pair
   (value_type), but key/value pairs are never stored anywhere in the map. */
   template <typename TPair>
   class pair_ptr {
   public:
      //! Constructor. See value_type::value_type().
      pair_ptr(TKey * pkey, TValue * pvalue) :
         m_pair(pkey, pvalue) {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current key/value pair.
      */
      TPair const & operator*() const {
         return m_pair;
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current key/value pair.
      */
      TPair const * operator->() const {
         return &m_pair;
      }

   private:
      //! Internal pair returned by operator->().
      TPair const m_pair;
   };

   //! Const iterator type.
   class const_iterator :
      public map_impl::iterator_base {
   private:
      friend class map;

   public:
      //! Const key/value type.
      struct value_type {
         TKey   const & key;
         TValue const & value;

         //! Constructor. See value_type::value_type().
         value_type(TKey const * pkey, TValue const * pvalue) :
            key(*pkey), value(*pvalue) {
         }
      };

      typedef std::ptrdiff_t difference_type;
      typedef std::forward_iterator_tag iterator_category;
      typedef value_type * pointer;
      typedef value_type & reference;

   public:
      //! Constructor.
      const_iterator() {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current key/value pair.
      */
      value_type operator*() const {
         ABC_TRACE_FUNC(this);

         validate();
         map const * pmap = static_cast<map const *>(m_pmap);
         return value_type(pmap->key_ptr(m_iBucket), pmap->value_ptr(m_iBucket));
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current key/value pair.
      */
      pair_ptr<value_type> operator->() const {
         ABC_TRACE_FUNC(this);

         validate();
         map const * pmap = static_cast<map const *>(m_pmap);
         return pair_ptr<value_type>(pmap->key_ptr(m_iBucket), pmap->value_ptr(m_iBucket));
      }

      /*! Preincrement operator.

      @return
         *this.
      */
      const_iterator & operator++() {
         ABC_TRACE_FUNC(this);

         validate();
         increment();
         return *this;
      }

      /*! Postincrement operator.

      @return
         Iterator pointing to the previous key/value pair.
      */
      const_iterator operator++(int) {
         ABC_TRACE_FUNC(this);

         validate();
         std::size_t iBucketPrev = m_iBucket;
         increment();
         return iterator(m_pmap, iBucketPrev);
      }

   protected:
      //! See map_impl::iterator_base::iterator_base.
      const_iterator(map_impl const * pmap, std::size_t iBucket) :
         map_impl::iterator_base(pmap, iBucket) {
      }
   };

   //! Iterator type.
   class iterator :
      public const_iterator {
   private:
      friend class map;

   public:
      //! Key/value type.
      struct value_type {
         TKey   & key;
         TValue & value;

         value_type(TKey * pkey, TValue * pvalue) :
            key(*pkey), value(*pvalue) {
         }
      };

      typedef value_type * pointer;
      typedef value_type & reference;

   public:
      //! Constructor.
      iterator() {
      }

      //! See const_iterator::operator*().
      value_type operator*() const {
         ABC_TRACE_FUNC(this);

         this->validate();
         map const * pmap = static_cast<map const *>(this->m_pmap);
         return value_type(pmap->key_ptr(this->m_iBucket), pmap->value_ptr(this->m_iBucket));
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current key/value pair.
      */
      pair_ptr<value_type> operator->() const {
         ABC_TRACE_FUNC(this);

         this->validate();
         map const * pmap = static_cast<map const *>(this->m_pmap);
         return pair_ptr<value_type>(
            pmap->key_ptr(this->m_iBucket), pmap->value_ptr(this->m_iBucket)
         );
      }

      //! See const_iterator.operator++().
      iterator & operator++() {
         return static_cast<iterator &>(const_iterator::operator++());
      }

      //! See const_iterator::operator++(int).
      iterator operator++(int) {
         return iterator(const_iterator::operator++());
      }

   protected:
      //! See const_iterator::const_iterator.
      iterator(map_impl const * pmap, std::size_t iBucket) :
         const_iterator(pmap, iBucket) {
      }

   private:
      /*! Constructor used for cv-removing promotions from const_iterator to iterator.

      @param it
         Source object.
      */
      iterator(const_iterator const & it) :
         const_iterator(it) {
      }
   };

   typedef typename iterator::value_type value_type;
   typedef typename const_iterator::value_type const_value_type;

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
      ABC_TRACE_FUNC(this/*, key*/);

      std::size_t iBucket = lookup_key(key);
      if (iBucket == smc_iNullIndex) {
         // TODO: provide more information in the exception.
         ABC_THROW(key_error, ());
      }
      return *value_ptr(iBucket);
   }

   /*! Adds a key/value pair to the map, overwriting the value if key is already associated to one.

   TODO: make four copies of this method, taking const &/const&, &&/&&, const &/&&, &&/const &; this
   requires more work to avoid the commented-out set_copy_fn() below for non-copiable types.

   @param key
      Key to add.
   @param value
      Value to add.
   @return
      Pair containing an iterator to the newly added key/value, and a bool value that is true if the
      key/value pair was just added, or false if the key already existed in the map and the
      corresponding value was overwritten.
   */
   std::pair<iterator, bool> add_or_assign(TKey key, TValue value) {
      ABC_TRACE_FUNC(this/*, key, value*/);

      detail::type_void_adapter typeKey, typeValue;
//      typeKey.set_copy_fn<TKey>();
      typeKey.set_destr_fn<TKey>();
      typeKey.set_move_fn<TKey>();
      typeKey.set_size<TKey>();
//      typeValue.set_copy_fn<TValue>();
      typeValue.set_destr_fn<TValue>();
      typeValue.set_move_fn<TValue>();
      typeValue.set_size<TValue>();
      std::size_t iKeyHash = calculate_and_adjust_hash(key), iBucket;
      bool bNew;
      std::tie(iBucket, bNew) = map_impl::add_or_assign(
         typeKey, typeValue, &keys_equal, &key, iKeyHash, &value, 1 | 2
      );
      return std::make_pair(iterator(this, iBucket), bNew);
   }

   /*! Returns a forward iterator set to the first key/value pair.

   @return
      Forward iterator to the first key/value pair.
   */
   iterator begin() {
      iterator it(this, smc_iNullIndex);
      it.increment();
      return std::move(it);
   }
   const_iterator begin() const {
      return cbegin();
   }

   /*! Returns a const forward iterator set to the first key/value pair.

   @return
      Forward iterator to the first key/value pair.
   */
   const_iterator cbegin() const {
      const_iterator it(this, smc_iNullIndex);
      it.increment();
      return std::move(it);
   }

   /*! Returns a const forward iterator set beyond the last key/value pair.

   @return
      Forward iterator to the last key/value pair.
   */
   iterator cend() {
      return const_iterator(this, smc_iNullIndex);
   }

   //! Removes all elements from the map.
   void clear() {
      ABC_TRACE_FUNC(this);

      detail::type_void_adapter typeKey, typeValue;
      typeKey.set_destr_fn<TKey>();
      typeKey.set_size<TKey>();
      typeValue.set_destr_fn<TValue>();
      typeValue.set_size<TValue>();
      map_impl::clear(typeKey, typeValue);
   }

   /*! Returns a forward iterator set beyond the last key/value pair.

   @return
      Forward iterator to the last key/value pair.
   */
   iterator end() {
      return iterator(this, smc_iNullIndex);
   }
   const_iterator end() const {
      return cend();
   }

   /*! Removes and returns a value given an iterator or a key, which must be in the map.

   @param it
      Iterator to the key/value to extract.
   @param key
      Key associated to the value to extract.
   */
   TValue extract(const_iterator it) {
      ABC_TRACE_FUNC(this/*, it*/);

      it.validate();
      TValue value(std::move(*value_ptr(it.m_iBucket)));
      detail::type_void_adapter typeKey, typeValue;
      typeKey.set_destr_fn<TKey>();
      typeKey.set_size<TKey>();
      typeValue.set_destr_fn<TValue>();
      typeValue.set_size<TValue>();
      empty_bucket(typeKey, typeValue, it.m_iBucket);
      return std::move(value);
   }
   TValue extract(TKey const & key) {
      ABC_TRACE_FUNC(this/*, key*/);

      std::size_t iBucket = lookup_key(key);
      if (iBucket == smc_iNullIndex) {
         // TODO: provide more information in the exception.
         ABC_THROW(key_error, ());
      }
      TValue value(std::move(*value_ptr(iBucket)));
      detail::type_void_adapter typeKey, typeValue;
      typeKey.set_destr_fn<TKey>();
      typeKey.set_size<TKey>();
      typeValue.set_destr_fn<TValue>();
      typeValue.set_size<TValue>();
      empty_bucket(typeKey, typeValue, iBucket);
      return std::move(value);
   }

   /*! Searches the map for a specific key, returning an iterator to the corresponding key/value
   pair if found.

   @param key
      Key to search for.
   @return
      Iterator to the matching key/value, or cend() if the key could not be found.
   */
   iterator find(TKey const & key) {
      ABC_TRACE_FUNC(this/*, key*/);

      std::size_t iBucket = lookup_key(key);
      return iterator(this, iBucket);
   }

   /*! Removes a value given an iterator or a key, which must be in the map.

   @param it
      Iterator to the key/value to remove.
   @param key
      Key associated to the value to remove.
   */
   void remove(const_iterator it) {
      ABC_TRACE_FUNC(this/*, it*/);

      detail::type_void_adapter typeKey, typeValue;
      typeKey.set_destr_fn<TKey>();
      typeKey.set_size<TKey>();
      typeValue.set_destr_fn<TValue>();
      typeValue.set_size<TValue>();
      empty_bucket(typeKey, typeValue, it);
   }
   void remove(TKey const & key) {
      ABC_TRACE_FUNC(this/*, key*/);

      std::size_t iBucket = lookup_key(key);
      if (iBucket == smc_iNullIndex) {
         // TODO: provide more information in the exception.
         ABC_THROW(key_error, ());
      }
      detail::type_void_adapter typeKey, typeValue;
      typeKey.set_destr_fn<TKey>();
      typeKey.set_size<TKey>();
      typeValue.set_destr_fn<TValue>();
      typeValue.set_size<TValue>();
      empty_bucket(typeKey, typeValue, iBucket);
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

   /*! Returns a pointer to the key in the specified bucket index.

   @param i
      Bucket index.
   @return
      Pointer to the key.
   */
   TKey * key_ptr(std::size_t i) const {
      return static_cast<TKey *>(m_pKeys.get()) + i;
   }

   /*! Compares two keys for equality. Static helper used by detail::map_impl.

   @param pmapi
      Pointer to *this.
   @param pKey1
      Pointer to the first key to compare.
   @param pKey2
      Pointer to the second key to compare.
   @return
      true if the two keys compare as equal, or false otherwise.
   */
   static bool keys_equal(map_impl const * pmapi, void const * pKey1, void const * pKey2) {
      map const * pmap = static_cast<map const *>(pmapi);
      return pmap->key_equal::operator()(
         *static_cast<TKey const *>(pKey1), *static_cast<TKey const *>(pKey2)
      );
   }

   /*! Looks for a specific key in the map.

   @param key
      Key to lookup.
   @return
      Index of the bucket at which the key could be found, or smc_iNullIndex if the key could not be
      found.
   */
   std::size_t lookup_key(TKey const & key) const {
      std::size_t iKeyHash = calculate_and_adjust_hash(key);
      if (m_cBuckets == 0) {
         // The key cannot possibly be in the map.
         return smc_iNullIndex;
      }
      std::size_t iNhBegin, iNhEnd;
      std::tie(iNhBegin, iNhEnd) = hash_neighborhood_range(iKeyHash);

      std::size_t const * piHash      = m_piHashes.get() + iNhBegin,
                        * piHashNhEnd = m_piHashes.get() + iNhEnd,
                        * piHashesEnd = m_piHashes.get() + m_cBuckets;
      /* iNhBegin - iNhEnd may be a wrapping range, so we can only test for inequality and rely on
      the wrap-around logic at the end of the loop body. Also, we need to iterate at least once,
      otherwise we won’t enter the loop at all if the start condition is the same as the end
      condition, which is the case for m_cNeighborhoodBuckets == m_cBuckets. */
      do {
         /* Multiple calculations of the second condition should be rare enough (exact key match or
         hash collision) to make recalculating the offset from m_pKeys cheaper than keeping a cursor
         over m_pKeys running in parallel to piHash. */
         if (*piHash == iKeyHash) {
            std::size_t iBucket = static_cast<std::size_t>(piHash - m_piHashes.get());
            if (key_equal::operator()(*key_ptr(iBucket), key)) {
               return iBucket;
            }
         }

         // Move on to the next bucket, wrapping around to the first one if needed.
         if (++piHash == piHashesEnd) {
            piHash = m_piHashes.get();
         }
      } while (piHash != piHashNhEnd);
      return smc_iNullIndex;
   }

   /*! Returns a pointer to the value in the specified bucket index.

   @param i
      Bucket index.
   @return
      Pointer to the value.
   */
   TValue * value_ptr(std::size_t i) const {
      return static_cast<TValue *>(m_pValues.get()) + i;
   }
};

} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COLLECTIONS_MAP_HXX
