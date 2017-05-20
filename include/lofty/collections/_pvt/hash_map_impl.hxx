/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_COLLECTIONS__PVT_HASH_MAP_IMPL_HXX
#define _LOFTY_COLLECTIONS__PVT_HASH_MAP_IMPL_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/numeric.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declaration.
namespace lofty {

class type_void_adapter;

} //namespace lofty

namespace lofty { namespace collections { namespace _pvt {

//! Non-template implementation class for lofty::collections::hash_map.
class LOFTY_SYM hash_map_impl : public support_explicit_operator_bool<hash_map_impl> {
protected:
   typedef bool (* keys_equal_fn_type)(hash_map_impl const * this_ptr, void const * key1, void const * key2);

   //! Integer type used to track changes in the map.
   typedef std::uint16_t rev_int_t;

   //! Base class for hash_map iterator implementations.
   class LOFTY_SYM iterator_base {
   private:
      friend class hash_map_impl;

   public:
      typedef _std::forward_iterator_tag iterator_category;

   public:
      //! Default constructor.
      iterator_base();

      /*! Constructor.

      @param owner_map
         Pointer to the map owning the iterated objects.
      @param bucket
         Index of the current bucket.
      */
      iterator_base(hash_map_impl const * owner_map, std::size_t bucket);

      /*! Equality relational operator.

      @param other
         Object to compare to *this.
      @return
         true if *this is an iterator to the same key/value pair as other, or false otherwise.
      */
      bool operator==(iterator_base const & other) const {
         LOFTY_TRACE_FUNC(this/*, other*/);

         return owner_map == other.owner_map && bucket == other.bucket;
      }

      /*! Inequality relational operator.

      @param other
         Object to compare to *this.
      @return
         true if *this has a different key/value pair than other, or false otherwise.
      */
      bool operator!=(iterator_base const & other) const {
         LOFTY_TRACE_FUNC(this/*, other*/);

         return !operator==(other);
      }

   protected:
      //! Moves the iterator to next used bucket.
      void increment();

      /*! Throws a collections::out_of_range exception if the iterator is at the end of the container or has
      been invalidated by a change in the container. */
      void validate() const;

   protected:
      //! Pointer to the map to iterate over.
      hash_map_impl const * owner_map;
      //! Current bucket index.
      std::size_t bucket;
      //! Last container revision number known to the iterator.
      rev_int_t rev;
   };

public:
   //! Default constructor.
   hash_map_impl();

   /*! Move constructor.

   @param src
      Source object.
   */
   hash_map_impl(hash_map_impl && src);

   //! Destructor.
   ~hash_map_impl();

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   hash_map_impl & operator=(hash_map_impl && src);

   /*! Boolean evaluation operator.

   @return
      true if the map is not empty, or false otherwise.
   */
   LOFTY_EXPLICIT_OPERATOR_BOOL() const {
      return used_buckets > 0;
   }

   /*! Returns the maximum number of key/value pairs the map can currently hold.

   @return
      Current size of the allocated storage, in elements.
   */
   std::size_t capacity() const {
      return total_buckets;
   }

   /*! Returns the current neighborhood size.

   @return
      Current neighborhood size.
   */
   std::size_t neighborhood_size() const {
      return neighborhood_size_;
   }

   /*! Returns the count of elements in the map.

   @return
      Count of elements.
   */
   std::size_t size() const {
      return used_buckets;
   }

protected:
   /*! Returns the index of the bucket matching the specified key, or locates an empty bucket and returns its
   index after moving it in the key’s neighborhood.

   @param key_type
      Adapter for the key type.
   @param value_type
      Adapter for the value type.
   @param keys_equal_fn
      Pointer to a function that returns true if two keys compare as equal.
   @param key
      Pointer to the key to add.
   @param key_hash
      Hash of *key.
   @param value
      Pointer to the value to add.
   @param move
      Bitmask; 1 (bit 0) indicates that *key should be moved, while 2 (bit 1) indicates that *value should be
      moved.
   @return
      Pair containing the index of the newly-occupied bucket and a bool value that is true if the key/value
      pair was just added, or false if the key already existed in the map and the corresponding value was
      overwritten.
   */
   _std::tuple<std::size_t, bool> add_or_assign(
      type_void_adapter const & key_type, type_void_adapter const & value_type,
      keys_equal_fn_type keys_equal_fn, void * key, std::size_t key_hash, void * value, unsigned move
   );

   /*! Removes all elements from the map.

   @param key_type
      Adapter for the key type.
   @param value_type
      Adapter for the value type.
   */
   void clear(type_void_adapter const & key_type, type_void_adapter const & value_type);

   /*! Returns the neighborhood index (index of the first bucket in a neighborhood) for the given hash.

   @param key_hash
      Hash to get the neighborhood index for.
   @return
      Index of the first bucket in the neighborhood.
   */
   std::size_t hash_neighborhood_index(std::size_t key_hash) const {
      return key_hash & (total_buckets - 1);
   }

   /*! Returns the bucket index ranges for the neighborhood of the given hash.

   @param key_hash
      Hash to return the neighborhood of.
   @return
      Calculated range for the neighborhood bucket index.
   */
   _std::tuple<std::size_t, std::size_t> hash_neighborhood_range(std::size_t key_hash) const {
      std::size_t nh_begin = hash_neighborhood_index(key_hash);
      std::size_t nh_end = nh_begin + neighborhood_size_;
      // Wrap the end index back in the table.
      nh_end &= total_buckets - 1;
      return _std::make_tuple(nh_begin, nh_end);
   }

   /*! Marks a bucket as empty and destructs the corresponding key and value.

   @param key_type
      Adapter for the key type.
   @param value_type
      Adapter for the value type.
   @param itr
      Iterator to the bucket to empty.
   */
   void empty_bucket(
      type_void_adapter const & key_type, type_void_adapter const & value_type, iterator_base itr
   ) {
      LOFTY_TRACE_FUNC(this/*, key_type, value_type, itr*/);

      itr.validate();
      empty_bucket(key_type, value_type, itr.bucket);
   }

   /*! Marks a bucket as empty and destructs the corresponding key and value.

   @param key_type
      Adapter for the key type.
   @param value_type
      Adapter for the value type.
   @param bucket
      Index of the bucket to empty.
   */
   void empty_bucket(
      type_void_adapter const & key_type, type_void_adapter const & value_type, std::size_t bucket
   );

private:
   /*! Finds the first (non-empty) bucket whose contents can be moved to the specified bucket.

   @param empty_bucket_
      Index of the empty bucket, which is also the last bucket of the neighborhood to scan.
   @return
      Index of the first bucket whose contents can be moved, or a special value if none of the occupied
      buckets contains a key from the neighborhood ending at empty_bucket_.
   */
   std::size_t find_bucket_movable_to_empty(std::size_t empty_bucket_) const;

   /*! Looks for an empty bucket in the specified bucket range.

   @param nh_begin
      Beginning of the neighborhood bucket index range.
   @param nh_end
      End of the neighborhood bucket index range.
   @return
      Index of the first empty bucket found, or null_index if no empty buckets were found.
   */
   std::size_t find_empty_bucket(std::size_t nh_begin, std::size_t nh_end) const;

   /*! Looks for an empty bucket outsize the specified bucket range.

   @param key_type
      Adapter for the key type.
   @param value_type
      Adapter for the value type.
   @param nh_begin
      Beginning of the neighborhood bucket index range.
   @param nh_end
      End of the neighborhood bucket index range.
   @return
      Index of a bucket that has been emptied by moving its contents outside the neighborhood, or null_index
      if no movable buckets could be found.
   */
   std::size_t find_empty_bucket_outside_neighborhood(
      type_void_adapter const & key_type, type_void_adapter const & value_type, std::size_t nh_begin,
      std::size_t nh_end
   );

   /*! Locates an empty bucket where the specified key may be stored, and returns its index after moving it in
   the key’s neighborhood.

   @param key_type
      Adapter for the key type.
   @param value_type
      Adapter for the value type.
   @param key_hash
      Hash of *key.
   @return
      Index of the bucket for the specified key. If key no empty bucket can be found or moved in the key’s
      neighborhood, the returned index is null_index.
   */
   std::size_t get_empty_bucket_for_key(
      type_void_adapter const & key_type, type_void_adapter const & value_type, std::size_t key_hash
   );

   /*! Returns the index of the bucket matching the specified key, or locates an empty bucket and returns its
   index after moving it in the key’s neighborhood.

   @param key_type
      Adapter for the key type.
   @param value_type
      Adapter for the value type.
   @param keys_equal_fn
      Pointer to a function that returns true if two keys compare as equal.
   @param key
      Pointer to the key to lookup.
   @param key_hash
      Hash of *key.
   @return
      Index of the bucket for the specified key. If key is not already in the map and no empty bucket can be
      moved in the key’s neighborhood, the returned index is null_index.
   */
   std::size_t get_existing_or_empty_bucket_for_key(
      type_void_adapter const & key_type, type_void_adapter const & value_type,
      keys_equal_fn_type keys_equal_fn, void const * key, std::size_t key_hash
   );

   /*! Enlarges the neighborhood size by a factor of growth_factor. This does not require moving the contents
   of any buckets, since buckets will still be part of the correct neighborhood. */
   void grow_neighborhoods() {
      neighborhood_size_ *= growth_factor;
   }

   /*! Enlarges the hash table by a factor of growth_factor. The contents of each bucket are moved from the
   old arrays to new temporary ones, and the two array sets are then swapped.

   The bucket contents transfer work is done by reusing functions that obtain the arrays to operate on via
   member variables. In the assumption that transferring the contents of a bucket won’t throw because it only
   involves move-constructions and destructions, we optimistically update the member variables as soon as all
   memory allocations are done; if anything were to go wrong after that, we’d have no guaranteed-safe way of
   recovering from a half-transferred scenario anyway.

   @param key_type
      Adapter for the key type.
   @param value_type
      Adapter for the value type.
   */
   void grow_table(type_void_adapter const & key_type, type_void_adapter const & value_type);

   /*! Looks for a specific key or an unused bucket in the map.

   @param key_type
      Adapter for the key types.
   @param keys_equal_fn
      Pointer to a function that returns true if two keys compare as equal.
   @param key
      Pointer to the key to lookup.
   @param key_hash
      Hash of *key.
   @param nh_begin
      Beginning of the neighborhood bucket index range.
   @param nh_end
      End of the neighborhood bucket index range.
   @return
      Index of the bucket at which the key could be found, or index of the first empty bucket found, or
      null_index if neither could be found.
   */
   std::size_t lookup_key_or_find_empty_bucket(
      type_void_adapter const & key_type, keys_equal_fn_type keys_equal_fn, void const * key,
      std::size_t key_hash, std::size_t nh_begin, std::size_t nh_end
   ) const;

   /*! Copies or moves a value, and optionally a key, to the specified bucket.

   @param key_type
      Adapter for the key type.
   @param value_type
      Adapter for the value type.
   @param bucket
      Index of the destination bucket.
   @param key
      Pointer to the key to move to the destination bucket. May be nullptr if the key doesn’t need to be
      changed because it’s already correct.
   @param value
      Pointer to the value to move to the destination bucket.
   @param move
      Bitmask; 1 (bit 0) indicates that *key should be moved, while 2 (bit 1) indicates that *value should be
      moved.
   */
   void set_bucket_key_value(
      type_void_adapter const & key_type, type_void_adapter const & value_type, std::size_t bucket,
      void * key, void * value, unsigned move
   );

protected:
   //! Array containing the hash of each key.
   _std::unique_ptr<std::size_t[]> hashes;
   //! Array of keys.
   _std::unique_ptr<void, memory::freeing_deleter> keys;
   //! Array of buckets.
   _std::unique_ptr<void, memory::freeing_deleter> values;
   //! Count of total buckets. Always a power of two.
   std::size_t total_buckets;
   //! Count of elements / occupied buckets.
   std::size_t used_buckets;
   /*! Neighborhood size. The map will try to keep this to ideal_neighborhood_size, but the actual value may
   be smaller if the table is too small, or larger if the hash function results in too many collisions. In the
   worst case, this will be the same as total_buckets. */
   std::size_t neighborhood_size_;
   //! Indicates the revision number of the map contents.
   rev_int_t rev;

   //! Minimum bucket count. Must be a power of 2.
   static std::size_t const min_buckets = 8;
   //! Special hash value used to indicate that a bucket is empty.
   static std::size_t const empty_bucket_hash = 0;
   //! Hash table or neighborhood growth factor. Must be a power of 2.
   static std::size_t const growth_factor = 4;
   //! Default/ideal neighborhood size.
   static std::size_t const ideal_neighborhood_size;
   /*! Hash value substituted when the hash function returns 0; this is so we can use 0 (aliased by
   empty_bucket_hash) as a special value. This specific value is merely the largest prime number that will fit
   in 2^16, which is the (future, if ever) minimum word size supported by Lofty. */
   static std::size_t const zero_hash = 65521;

   //! First special index value.
   static std::size_t const first_special_index = numeric::max<std::size_t>::value - 8;
   /*! Special value returned by find_bucket_movable_to_empty() to indicate that the neighborhood size needs
   to be increased before trying again. */
   static std::size_t const need_larger_neighborhoods = numeric::max<std::size_t>::value - 2;
   /*! Special value returned by find_bucket_movable_to_empty() to indicate that the hash table size needs to
   be increased before trying again. */
   static std::size_t const need_larger_table = numeric::max<std::size_t>::value - 1;
   /*! Special index returned by several methods to indicate a logical “null index”. Code in
   iterator_base::increment() relies on null_index + 1 == 0. */
   static std::size_t const null_index = numeric::max<std::size_t>::value;
};

}}} //namespace lofty::collections::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_COLLECTIONS__PVT_HASH_MAP_IMPL_HXX
