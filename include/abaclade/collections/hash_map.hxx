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

#ifndef _ABACLADE_COLLECTIONS_HASH_MAP_HXX
#define _ABACLADE_COLLECTIONS_HASH_MAP_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/collections/detail/hash_map_impl.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections {

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
class hash_map : public detail::hash_map_impl, private THasher, private TKeyEqual {
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
      public hash_map_impl::iterator_base {
   private:
      friend class hash_map;

   public:
      //! Const key/value type.
      struct value_type {
         TKey const & key;
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
      //! Default constructor.
      const_iterator() {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current key/value pair.
      */
      value_type operator*() const {
         ABC_TRACE_FUNC(this);

         validate();
         hash_map const * phm = static_cast<hash_map const *>(m_phm);
         return value_type(phm->key_ptr(m_iBucket), phm->value_ptr(m_iBucket));
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current key/value pair.
      */
      pair_ptr<value_type> operator->() const {
         ABC_TRACE_FUNC(this);

         validate();
         hash_map const * phm = static_cast<hash_map const *>(m_phm);
         return pair_ptr<value_type>(phm->key_ptr(m_iBucket), phm->value_ptr(m_iBucket));
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
         return const_iterator(m_phm, iBucketPrev);
      }

   protected:
      //! See hash_map_impl::iterator_base::iterator_base.
      const_iterator(hash_map_impl const * phm, std::size_t iBucket) :
         hash_map_impl::iterator_base(phm, iBucket) {
      }
   };

   //! Iterator type.
   class iterator :
      public const_iterator {
   private:
      friend class hash_map;

   public:
      //! Key/value type.
      struct value_type {
         TKey const & key;
         TValue & value;

         value_type(TKey const * pkey, TValue * pvalue) :
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
         hash_map const * phm = static_cast<hash_map const *>(this->m_phm);
         return value_type(phm->key_ptr(this->m_iBucket), phm->value_ptr(this->m_iBucket));
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current key/value pair.
      */
      pair_ptr<value_type> operator->() const {
         ABC_TRACE_FUNC(this);

         this->validate();
         hash_map const * phm = static_cast<hash_map const *>(this->m_phm);
         return pair_ptr<value_type>(
            phm->key_ptr(this->m_iBucket), phm->value_ptr(this->m_iBucket)
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
      iterator(hash_map_impl const * phm, std::size_t iBucket) :
         const_iterator(phm, iBucket) {
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

   @param hm
      Source object.
   */
   hash_map(hash_map && hm) :
      detail::hash_map_impl(std::move(hm)) {
   }

   //! Destructor.
   ~hash_map() {
      clear();
   }

   /*! Move-assignment operator.

   @param hm
      Source object.
   @return
      *this.
   */
   hash_map & operator=(hash_map && hm) {
      detail::hash_map_impl::operator=(std::move(hm));
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
   requires more work to avoid the commented-out set_copy_construct() below for non-copiable types.

   @param key
      Key to add.
   @param value
      Value to add.
   @return
      Pair containing an iterator to the newly added key/value, and a bool value that is true if the
      key/value pair was just added, or false if the key already existed in the map and the
      corresponding value was overwritten.
   */
   _std::tuple<iterator, bool> add_or_assign(TKey key, TValue value) {
      ABC_TRACE_FUNC(this/*, key, value*/);

      type_void_adapter typeKey, typeValue;
//      typeKey.set_copy_construct<TKey>();
      typeKey.set_destruct<TKey>();
      typeKey.set_move_construct<TKey>();
      typeKey.set_size<TKey>();
//      typeValue.set_copy_construct<TValue>();
      typeValue.set_destruct<TValue>();
      typeValue.set_move_construct<TValue>();
      typeValue.set_size<TValue>();
      std::size_t iKeyHash = calculate_and_adjust_hash(key), iBucket;
      bool bNew;
      _std::tie(iBucket, bNew) = hash_map_impl::add_or_assign(
         typeKey, typeValue, &keys_equal, &key, iKeyHash, &value, 1 | 2
      );
      return _std::make_tuple(iterator(this, iBucket), bNew);
   }

   /*! Returns an iterator set to the first key/value pair in the map.

   @return
      Iterator to the first key/value pair.
   */
   iterator begin() {
      iterator it(this, smc_iNullIndex);
      it.increment();
      return std::move(it);
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
      ABC_TRACE_FUNC(this);

      type_void_adapter typeKey, typeValue;
      typeKey.set_destruct<TKey>();
      typeKey.set_size<TKey>();
      typeValue.set_destruct<TValue>();
      typeValue.set_size<TValue>();
      hash_map_impl::clear(typeKey, typeValue);
   }

   /*! Returns an iterator set beyond the last key/value pair in the map.

   @return
      Iterator set to beyond the last key/value pair.
   */
   iterator end() {
      return iterator(this, smc_iNullIndex);
   }

   /*! Returns a const iterator set beyond the last key/value pair in the map.

   @return
      Const iterator set to beyond the last key/value pair.
   */
   const_iterator end() const {
      return const_cast<hash_map *>(this)->begin();
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

   /*! Removes and returns a value given an iterator to it.

   @param it
      Iterator to the key/value to extract.
   */
   TValue pop(const_iterator it) {
      ABC_TRACE_FUNC(this/*, it*/);

      it.validate();
      TValue value(std::move(*value_ptr(it.m_iBucket)));
      type_void_adapter typeKey, typeValue;
      typeKey.set_destruct<TKey>();
      typeKey.set_size<TKey>();
      typeValue.set_destruct<TValue>();
      typeValue.set_size<TValue>();
      empty_bucket(typeKey, typeValue, it.m_iBucket);
      return std::move(value);
   }

   /*! Removes and returns a value given a key, which must be in the map.

   @param key
      Key associated to the value to extract.
   */
   TValue pop(TKey const & key) {
      ABC_TRACE_FUNC(this/*, key*/);

      std::size_t iBucket = lookup_key(key);
      if (iBucket == smc_iNullIndex) {
         // TODO: provide more information in the exception.
         ABC_THROW(key_error, ());
      }
      TValue value(std::move(*value_ptr(iBucket)));
      type_void_adapter typeKey, typeValue;
      typeKey.set_destruct<TKey>();
      typeKey.set_size<TKey>();
      typeValue.set_destruct<TValue>();
      typeValue.set_size<TValue>();
      empty_bucket(typeKey, typeValue, iBucket);
      return std::move(value);
   }

   /*! Removes a value given an iterator to it.

   @param it
      Iterator to the key/value to remove.
   */
   void remove(const_iterator it) {
      ABC_TRACE_FUNC(this/*, it*/);

      type_void_adapter typeKey, typeValue;
      typeKey.set_destruct<TKey>();
      typeKey.set_size<TKey>();
      typeValue.set_destruct<TValue>();
      typeValue.set_size<TValue>();
      empty_bucket(typeKey, typeValue, it);
   }

   /*! Removes a value given a key, which must be in the map.

   @param key
      Key associated to the value to remove.
   */
   void remove(TKey const & key) {
      if (!remove_if_found(key)) {
         // TODO: provide more information in the exception.
         ABC_THROW(key_error, ());
      }
   }

   /*! Removes a value given a key, if found in the map. If the key is not in the map, no removal
   occurs.

   @param key
      Key associated to the value to remove.
   */
   bool remove_if_found(TKey const & key) {
      ABC_TRACE_FUNC(this/*, key*/);

      std::size_t iBucket = lookup_key(key);
      if (iBucket != smc_iNullIndex) {
         type_void_adapter typeKey, typeValue;
         typeKey.set_destruct<TKey>();
         typeKey.set_size<TKey>();
         typeValue.set_destruct<TValue>();
         typeValue.set_size<TValue>();
         empty_bucket(typeKey, typeValue, iBucket);
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

   /*! Compares two keys for equality. Static helper used by detail::hash_map_impl.

   @param phmi
      Pointer to *this.
   @param pKey1
      Pointer to the first key to compare.
   @param pKey2
      Pointer to the second key to compare.
   @return
      true if the two keys compare as equal, or false otherwise.
   */
   static bool keys_equal(hash_map_impl const * phmi, void const * pKey1, void const * pKey2) {
      hash_map const * phm = static_cast<hash_map const *>(phmi);
      return phm->key_equal::operator()(
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
      _std::tie(iNhBegin, iNhEnd) = hash_neighborhood_range(iKeyHash);

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

}} //namespace abc::collections

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COLLECTIONS_HASH_MAP_HXX
