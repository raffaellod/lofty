/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_COLLECTIONS_HASH_MAP_HXX
#define _LOFTY_COLLECTIONS_HASH_MAP_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/collections.hxx>
#include <lofty/collections/_pvt/hash_map_impl.hxx>
#include <lofty/type_void_adapter.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections {

/*! Key/value map using a derivative of the hopscotch hashing collision resolution algorithm.

This implementation uses a variable hash table size (number of buckets) to deal with varying item counts, as
well as a variable neighborhood size (number of buckets sharing the same logical index) in order to tolerate
high-collision hash functions. */
template <
   typename TKey,
   typename TValue,
   typename THasher = std::hash<TKey>,
   typename TKeyEqual = std::equal_to<TKey>
>
class hash_map : public _pvt::hash_map_impl, private THasher, private TKeyEqual {
public:
   //! Key type.
   typedef TKey key_type;
   //! Mapped value type.
   typedef TValue mapped_type;
   //! Hash generator for TKey.
   typedef THasher hasher;
   //! Functor that can compare two TKey instances for equality.
   typedef TKeyEqual key_equal;

   /*! Pointer type returned by iterator::operator->() that behaves like a pointer, but in fact includes the
   object it points to.

   Needed because iterator::operator->() must return a pointer-like type to a key/value pair (value_type), but
   key/value pairs are never stored anywhere in the map. */
   template <typename TPair>
   class pair_ptr {
   public:
      //! Constructor. See value_type::value_type().
      pair_ptr(TKey * key, TValue * value) :
         pair_(key, value) {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current key/value pair.
      */
      TPair const & operator*() const {
         return pair_;
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current key/value pair.
      */
      TPair const * operator->() const {
         return &pair_;
      }

   private:
      //! Internal pair returned by operator->().
      TPair const pair_;
   };

   //! Const iterator type.
   class const_iterator : public hash_map_impl::iterator_base {
   private:
      friend class hash_map;

   public:
      //! Const key/value type.
      struct value_type {
         TKey const & key;
         TValue const & value;

         //! Constructor. See value_type::value_type().
         value_type(TKey const * key_, TValue const * value_) :
            key(*key_),
            value(*value_) {
         }
      };

      typedef value_type * pointer;
      typedef value_type & reference;

   public:
      //! Default constructor.
      const_iterator() {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current key/value pair.
      */
      value_type operator*() const {
         LOFTY_TRACE_FUNC(this);

         validate();
         hash_map const * map = static_cast<hash_map const *>(owner_map);
         return value_type(map->key_ptr(bucket), map->value_ptr(bucket));
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current key/value pair.
      */
      pair_ptr<value_type> operator->() const {
         LOFTY_TRACE_FUNC(this);

         validate();
         hash_map const * map = static_cast<hash_map const *>(owner_map);
         return pair_ptr<value_type>(map->key_ptr(bucket), map->value_ptr(bucket));
      }

      /*! Preincrement operator.

      @return
         *this.
      */
      const_iterator & operator++() {
         LOFTY_TRACE_FUNC(this);

         validate();
         increment();
         return *this;
      }

      /*! Postincrement operator.

      @return
         Iterator pointing to the previous key/value pair.
      */
      const_iterator operator++(int) {
         LOFTY_TRACE_FUNC(this);

         validate();
         std::size_t old_bucket = bucket;
         increment();
         return const_iterator(owner_map, old_bucket);
      }

   protected:
      //! See hash_map_impl::iterator_base::iterator_base.
      const_iterator(hash_map_impl const * owner_map_, std::size_t bucket_) :
         hash_map_impl::iterator_base(owner_map_, bucket_) {
      }
   };

   //! Iterator type.
   class iterator : public const_iterator {
   private:
      friend class hash_map;

   public:
      //! Key/value type.
      struct value_type {
         //! Reference to the key.
         TKey const & key;
         //! Reference to the value.
         TValue & value;

         /*! Constructor.

         @param key_
            Pointer to the key to refer to.
         @param value_
            Pointer to the value to refer to.
         */
         value_type(TKey const * key_, TValue * value_) :
            key(*key_),
            value(*value_) {
         }
      };

      typedef value_type * pointer;
      typedef value_type & reference;

   public:
      //! Default constructor.
      iterator() {
      }

      //! See const_iterator::operator*().
      value_type operator*() const {
         LOFTY_TRACE_FUNC(this);

         this->validate();
         auto map = static_cast<hash_map const *>(this->owner_map);
         return value_type(map->key_ptr(this->bucket), map->value_ptr(this->bucket));
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current key/value pair.
      */
      pair_ptr<value_type> operator->() const {
         LOFTY_TRACE_FUNC(this);

         this->validate();
         auto map = static_cast<hash_map const *>(this->owner_map);
         return pair_ptr<value_type>(map->key_ptr(this->bucket), map->value_ptr(this->bucket));
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
      iterator(hash_map_impl const * owner_map_, std::size_t bucket_) :
         const_iterator(owner_map_, bucket_) {
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
   //! Default constructor.
   hash_map() {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   hash_map(hash_map && src) :
      _pvt::hash_map_impl(_std::move(src)) {
   }

   //! Destructor.
   ~hash_map() {
      clear();
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   hash_map & operator=(hash_map && src) {
      _pvt::hash_map_impl::operator=(_std::move(src));
      return *this;
   }

   /*! Element lookup operator.

   @param key
      Key to lookup.
   @return
      Value corresponding to key. If key is not in the map, an exception will be thrown.
   */
   TValue & operator[](TKey const & key) const {
      LOFTY_TRACE_FUNC(this/*, key*/);

      std::size_t bucket = lookup_key(key);
      if (bucket == null_index) {
         // TODO: provide more information in the exception.
         LOFTY_THROW(collections::bad_key, ());
      }
      return *value_ptr(bucket);
   }

   /*! Adds a key/value pair to the map, overwriting the value if key is already associated to one.

   TODO: make four copies of this method, taking const &/const&, &&/&&, const &/&&, &&/const &; this requires
   more work to avoid the commented-out set_copy_construct() below for non-copiable types.

   @param key
      Key to add.
   @param value
      Value to add.
   @return
      Pair containing an iterator to the newly added key/value, and a bool value that is true if the key/value
      pair was just added, or false if the key already existed in the map and the corresponding value was
      overwritten.
   */
   _std::tuple<iterator, bool> add_or_assign(TKey key, TValue value) {
      LOFTY_TRACE_FUNC(this/*, key, value*/);

      type_void_adapter key_type, value_type;
//      key_type.set_copy_construct<TKey>();
      key_type.set_destruct<TKey>();
      key_type.set_move_construct<TKey>();
      key_type.set_size<TKey>();
//      value_type.set_copy_construct<TValue>();
      value_type.set_destruct<TValue>();
      value_type.set_move_construct<TValue>();
      value_type.set_size<TValue>();
      std::size_t key_hash = calculate_and_adjust_hash(key), bucket;
      bool added;
      _std::tie(bucket, added) = hash_map_impl::add_or_assign(
         key_type, value_type, &keys_equal, &key, key_hash, &value, 1 | 2
      );
      return _std::make_tuple(iterator(this, bucket), added);
   }

   /*! Returns an iterator set to the first key/value pair in the map.

   @return
      Iterator to the first key/value pair.
   */
   iterator begin() {
      iterator itr(this, null_index);
      itr.increment();
      return _std::move(itr);
   }

   /*! Returns a const iterator set to the first key/value pair in the map.

   @return
      Const iterator to the first key/value pair.
   */
   const_iterator begin() const {
      return const_cast<hash_map *>(this)->begin();
   }

   /*! Returns a const iterator set to the first key/value pair in the map.

   @return
      Const iterator to the first key/value pair.
   */
   const_iterator cbegin() const {
      return const_cast<hash_map *>(this)->begin();
   }

   /*! Returns a const iterator set beyond the last key/value pair in the map.

   @return
      Const iterator set to beyond the last key/value pair.
   */
   const_iterator cend() {
      return const_cast<hash_map *>(this)->end();
   }

   //! Removes all elements from the map.
   void clear() {
      LOFTY_TRACE_FUNC(this);

      type_void_adapter key_type, value_type;
      key_type.set_destruct<TKey>();
      key_type.set_size<TKey>();
      value_type.set_destruct<TValue>();
      value_type.set_size<TValue>();
      hash_map_impl::clear(key_type, value_type);
   }

   /*! Returns an iterator set beyond the last key/value pair in the map.

   @return
      Iterator set to beyond the last key/value pair.
   */
   iterator end() {
      return iterator(this, null_index);
   }

   /*! Returns a const iterator set beyond the last key/value pair in the map.

   @return
      Const iterator set to beyond the last key/value pair.
   */
   const_iterator end() const {
      return const_cast<hash_map *>(this)->begin();
   }

   /*! Searches the map for a specific key, returning an iterator to the corresponding key/value pair if
   found.

   @param key
      Key to search for.
   @return
      Iterator to the matching key/value, or cend() if the key could not be found.
   */
   iterator find(TKey const & key) {
      LOFTY_TRACE_FUNC(this/*, key*/);

      std::size_t bucket = lookup_key(key);
      return iterator(this, bucket);
   }

   /*! Removes and returns a value given an iterator to it.

   @param itr
      Iterator to the key/value to extract.
   @return
      Value removed from the map.
   */
   TValue pop(const_iterator itr) {
      LOFTY_TRACE_FUNC(this/*, itr*/);

      itr.validate();
      TValue value(_std::move(*value_ptr(itr.bucket)));
      type_void_adapter key_type, value_type;
      key_type.set_destruct<TKey>();
      key_type.set_size<TKey>();
      value_type.set_destruct<TValue>();
      value_type.set_size<TValue>();
      empty_bucket(key_type, value_type, itr.bucket);
      return _std::move(value);
   }

   /*! Removes and returns a value given a key, which must be in the map.

   @param key
      Key associated to the value to extract.
   @return
      Value removed from the map.
   */
   TValue pop(TKey const & key) {
      LOFTY_TRACE_FUNC(this/*, key*/);

      std::size_t bucket = lookup_key(key);
      if (bucket == null_index) {
         // TODO: provide more information in the exception.
         LOFTY_THROW(collections::bad_key, ());
      }
      TValue value(_std::move(*value_ptr(bucket)));
      type_void_adapter key_type, value_type;
      key_type.set_destruct<TKey>();
      key_type.set_size<TKey>();
      value_type.set_destruct<TValue>();
      value_type.set_size<TValue>();
      empty_bucket(key_type, value_type, bucket);
      return _std::move(value);
   }

   /*! Removes a value given an iterator to it.

   @param it
      Iterator to the key/value to remove.
   */
   void remove(const_iterator it) {
      LOFTY_TRACE_FUNC(this/*, it*/);

      type_void_adapter key_type, value_type;
      key_type.set_destruct<TKey>();
      key_type.set_size<TKey>();
      value_type.set_destruct<TValue>();
      value_type.set_size<TValue>();
      empty_bucket(key_type, value_type, it);
   }

   /*! Removes a value given a key, which must be in the map.

   @param key
      Key associated to the value to remove.
   */
   void remove(TKey const & key) {
      if (!remove_if_found(key)) {
         // TODO: provide more information in the exception.
         LOFTY_THROW(collections::bad_key, ());
      }
   }

   /*! Removes a value given a key, if found in the map. If the key is not in the map, no removal occurs.

   @param key
      Key associated to the value to remove.
   @return
      true if a value matching the key was found (and removed), or false otherwise.
   */
   bool remove_if_found(TKey const & key) {
      LOFTY_TRACE_FUNC(this/*, key*/);

      std::size_t bucket = lookup_key(key);
      if (bucket != null_index) {
         type_void_adapter key_type, value_type;
         key_type.set_destruct<TKey>();
         key_type.set_size<TKey>();
         value_type.set_destruct<TValue>();
         value_type.set_size<TValue>();
         empty_bucket(key_type, value_type, bucket);
         return true;
      } else {
         return false;
      }
   }

private:
   /*! Calculates, adjusts and returns the hash value for the specified key.

   @param key
      Key to calculate a hash value for.
   @return
      Hash value of key.
   */
   std::size_t calculate_and_adjust_hash(TKey const & key) const {
      std::size_t key_hash = hasher::operator()(key);
      return key_hash == empty_bucket_hash ? zero_hash : key_hash;
   }

   /*! Returns a pointer to the key in the specified bucket index.

   @param bucket
      Bucket index.
   @return
      Pointer to the key.
   */
   TKey * key_ptr(std::size_t bucket) const {
      return static_cast<TKey *>(keys.get()) + bucket;
   }

   /*! Compares two keys for equality. Static helper used by _pvt::hash_map_impl.

   @param this_ptr
      Pointer to *this.
   @param key1
      Pointer to the first key to compare.
   @param key2
      Pointer to the second key to compare.
   @return
      true if the two keys compare as equal, or false otherwise.
   */
   static bool keys_equal(hash_map_impl const * this_ptr, void const * key1, void const * key2) {
      auto map = static_cast<hash_map const *>(this_ptr);
      return map->key_equal::operator()(*static_cast<TKey const *>(key1), *static_cast<TKey const *>(key2));
   }

   /*! Looks for a specific key in the map.

   @param key
      Key to lookup.
   @return
      Index of the bucket at which the key could be found, or null_index if the key could not be found.
   */
   std::size_t lookup_key(TKey const & key) const {
      std::size_t key_hash = calculate_and_adjust_hash(key);
      if (total_buckets == 0) {
         // The key cannot possibly be in the map.
         return null_index;
      }
      std::size_t nh_begin, nh_end;
      _std::tie(nh_begin, nh_end) = hash_neighborhood_range(key_hash);

      auto hash_ptr      = hashes.get() + nh_begin;
      auto hashes_nh_end = hashes.get() + nh_end;
      auto hashes_end    = hashes.get() + total_buckets;
      /* nh_begin - nh_end may be a wrapping range, so we can only test for inequality and rely on the wrap-
      around logic at the end of the loop body. Also, we need to iterate at least once, otherwise we won’t
      enter the loop at all if the start condition is the same as the end condition, which is the case for
      neighborhood_size_ == total_buckets. */
      do {
         /* Multiple calculations of the second condition should be rare enough (exact key match or hash
         collision) to make recalculating the offset from keys cheaper than keeping a cursor over keys running
         in parallel to hash_ptr. */
         if (*hash_ptr == key_hash) {
            std::size_t bucket = static_cast<std::size_t>(hash_ptr - hashes.get());
            if (key_equal::operator()(*key_ptr(bucket), key)) {
               return bucket;
            }
         }

         // Move on to the next bucket, wrapping around to the first one if needed.
         if (++hash_ptr == hashes_end) {
            hash_ptr = hashes.get();
         }
      } while (hash_ptr != hashes_nh_end);
      return null_index;
   }

   /*! Returns a pointer to the value in the specified bucket index.

   @param bucket
      Bucket index.
   @return
      Pointer to the value.
   */
   TValue * value_ptr(std::size_t bucket) const {
      return static_cast<TValue *>(values.get()) + bucket;
   }
};

}} //namespace lofty::collections

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_COLLECTIONS_HASH_MAP_HXX
