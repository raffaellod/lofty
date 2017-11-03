/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_COLLECTIONS_TRIE_ORDERED_MULTIMAP_HXX
#define _LOFTY_COLLECTIONS_TRIE_ORDERED_MULTIMAP_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/collections/_pvt/trie_ordered_multimap_impl.hxx>
#include <lofty/type_void_adapter.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections {

/*! Key/value multimap using a trie.

Specializations are defined only for scalar key types.

This implementation focuses on insertion and removal speed, providing O(1) insertion, O(1) extraction of the
first element, and O(1) extraction of any other element given an iterator to it. In the scalar key case, this
is achieved by using a trie where each node consumes a nibble (4 bits) of the key; values are stored in
doubly-linked lists connected to the leaves of the binary tree.

For example, let’s consider a hypothetical trie using integer 4-bit keys where each node consumes one bit,
populated with the following data:

   @verbatim
   ┌────────┬───────┐
   │ Key    │ Value │
   ├────────┼───────┤
   │ 0b1000 │ a     │
   │ 0b1010 │ b     │
   │ 0b1010 │ c     │
   └────────┴───────┘
   @endverbatim

The internal data representation of the above would be:

   @verbatim
    ⎧ ┌0──────┬1──────┐
    ⎪ │nullptr│0xptr  │
    ⎪ └───────┴───────┘
    ⎪          │
    ⎪          ▼
    ⎪          ┌0──────┬1──────┐
    ⎪          │0xptr  │nullptr│
    ⎪          └───────┴───────┘
    ⎪           │
   1⎨           ▼
    ⎪           ┌0──────┬1──────┐
    ⎪           │0xptr  │0xptr  │
    ⎪           └───────┴───────┘
    ⎪    ┌───────┘       └──────────────────┐
    ⎪    ▼                                  ▼
    ⎪  ⎧ ┌First0─┬F1─────╥Last0──┬L1─────┐  ┌F0─────┬F1─────╥L0─────┬L1─────┐
    ⎪ 2⎨ │0xptr  │nullptr║0xptr  │nullptr│  │0xptr  │nullptr║0xptr  │nullptr│
    ⎩  ⎩ └───────┴───────╨───────┴───────┘  └───────┴───────╨───────┴───────┘
          ├───────────────┘                  │               └─────┐
          ▼                                  ▼                     ▼
    ⎧     ┌───────┬───────┬───┐              ┌───────┬───────┬───┐ ┌───────┬───────┬───┐
   3⎨     │nullptr│nullptr│ a │              │nullptr│0xptr  │ b │ │0xptr  │nullptr│ c │
    ⎩     └───────┴───────┴───┘              └───────┴───────┴───┘ └───────┴───────┴───┘
                                             ▲        │            ▲│
                                             │        └────────────┘│
                                             └──────────────────────┘
   @endverbatim

In the graph above, 1 is the prefix tree, where each node contains pointers to its children; 2 is the anchor
level, where each node also contains pointers the last nodes of each list of identically-keyed values; 3 is
the value level, containing doubly-linked lists of identically-keyed values. */
template <typename TKey, typename TValue, unsigned impl_type = (_std::is_scalar<TKey>::value ? 1 : 0)>
class trie_ordered_multimap;

// Partial specialization for scalar key types.
template <typename TKey, typename TValue>
class trie_ordered_multimap<TKey, TValue, 1> : public _pvt::bitwise_trie_ordered_multimap_impl {
protected:
   /*! Pointer type returned by iterator::operator->() that behaves like a pointer, but in fact includes the
   object it points to.

   Needed because iterator::operator->() must return a pointer-like type to a key/value pair (::reference),
   but key/value pairs are never stored anywhere in the map. */
   template <typename TPair>
   class pair_ptr {
   public:
      //! Constructor. See TPair::TPair().
      pair_ptr(TKey key, TValue * value) :
         pair(key, value) {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current key/value pair.
      */
      TPair const & operator*() const {
         return pair;
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current key/value pair.
      */
      TPair const * operator->() const {
         return &pair;
      }

   private:
      //! Internal pair returned by operator->().
      TPair const pair;
   };

public:
   //! Key type.
   typedef TKey key_type;
   //! Mapped value type.
   typedef TValue mapped_type;

   //! Key/value pair type.
   struct value_type {
      //! Key.
      TKey key;
      //! Value.
      TValue value;

      /*! Constructor.

      @param key_
         Source key.
      @param value_
         Source value.
      */
      template <typename UKey, typename UValue>
      value_type(UKey && key_, UValue && value_) :
         key(_std::forward<UKey>(key_)),
         value(_std::forward<UValue>(value_)) {
      }
   };

   //! Key/value reference type.
   struct reference {
      TKey const key;
      TValue & value;

      /*! Constructor.

      @param key_
         Referred key. Actually copied.
      @param value_
         Pointer to the value to create a reference to.
      */
      reference(TKey key_, TValue * value_) :
         key(key_),
         value(*value_) {
      }
   };

   //! Const key/value reference type.
   struct const_reference {
      TKey const key;
      TValue const & value;

      /*! Constructor.

      @param key_
         Referred key. Actually copied.
      @param value_
         Pointer to the value to create a reference to.
      */
      const_reference(TKey key_, TValue const * value_) :
         key(key_),
         value(*value_) {
      }

      /*! Copy constructor from non-const_reference.

      @param src
         Source object.
      */
      const_reference(reference const & src) :
         key(src.key),
         value(src.value) {
      }
   };

   //! Const key/value pair iterator type.
   class const_iterator {
   private:
      friend class trie_ordered_multimap;

   public:
      typedef std::ptrdiff_t difference_type;
      typedef _std::forward_iterator_tag iterator_category;
      typedef const_reference value_type;
      typedef const_reference * pointer;
      typedef const_reference & reference;

   public:
      //! Default constructor.
      const_iterator() :
         owner_map(nullptr),
         key(),
         ln(nullptr) {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current key/value pair.
      */
      value_type operator*() const {
         trie_ordered_multimap::validate_iterator(ln);
         return value_type(key, ln->value_ptr<TValue>());
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current key/value pair.
      */
      pair_ptr<value_type> operator->() const {
         trie_ordered_multimap::validate_iterator(ln);
         return pair_ptr<value_type>(key, ln->value_ptr<TValue>());
      }

      /*! Preincrement operator.

      @return
         *this after it’s moved to the key/value pair following the one currently referred to.
      */
      const_iterator & operator++() {
         trie_ordered_multimap::validate_iterator(ln);
         if (list_node * next = ln->next()) {
            ln = next;
         } else {
            auto kvp(owner_map->find_next_key(trie_ordered_multimap::key_to_int(key)));
            key = int_to_key(kvp.key);
            ln = kvp.ln;
         }
         return *this;
      }

      /*! Postincrement operator.

      @return
         Iterator referring to the key/value pair following the one referring to by this iterator.
      */
      const_iterator operator++(int) {
         const_iterator old(*this);
         operator++();
         return _std::move(old);
      }

      /*! Equality relational operator.

      @param other
         Object to compare to *this.
      @return
         true if *this is an iterator to the same key/value pair as other, or false otherwise.
      */
      bool operator==(const_iterator const & other) const {
         return ln == other.ln;
      }

      /*! Inequality relational operator.

      @param other
         Object to compare to *this.
      @return
         true if *this has a different key/value pair than other, or false otherwise.
      */
      bool operator!=(const_iterator const & other) const {
         return !operator==(other);
      }

   protected:
      /*! Constructor.

      @param owner_map_
         Trie containing the current value.
      @param key_
         Key associated to the value referred to by the iterator.
      @param ln_
         Pointer to the value referred to by the iterator, or nullptr to create an “end” iterator.
      */
      const_iterator(trie_ordered_multimap const * owner_map_, TKey key_, list_node * ln_) :
         owner_map(owner_map_),
         key(key_),
         ln(ln_) {
      }

   protected:
      //! Trie containing the current value.
      trie_ordered_multimap const * owner_map;
      //! Key the iterator is at.
      TKey key;
      //! Pointer to the current value’s node.
      list_node * ln;
   };

   //! Non-const key/value pair iterator type.
   class iterator : public const_iterator {
   private:
      friend class trie_ordered_multimap;

   public:
      typedef typename trie_ordered_multimap::reference value_type;
      typedef typename trie_ordered_multimap::reference * pointer;
      typedef typename trie_ordered_multimap::reference & reference;

   public:
      //! Default constructor.
      iterator() {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current key/value pair.
      */
      value_type operator*() const {
         return value_type(this->key, this->ln->template value_ptr<TValue>());
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current key/value pair.
      */
      pair_ptr<value_type> operator->() const {
         return pair_ptr<value_type>(this->key, this->ln->template value_ptr<TValue>());
      }

   private:
      /*! Constructor.

      @param owner_map_
         Trie containing the current value.
      @param key_
         Key associated to the value referred to by the iterator.
      @param ln_
         Pointer to the value referred to by the iterator, or nullptr to create an “end” iterator.
      */
      iterator(trie_ordered_multimap const * owner_map_, TKey key_, list_node * ln_) :
         const_iterator(owner_map_, key_, ln_) {
      }
   };

public:
   //! Default constructor.
   trie_ordered_multimap() :
      _pvt::bitwise_trie_ordered_multimap_impl(sizeof(TKey)) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   trie_ordered_multimap(trie_ordered_multimap && src) :
      _pvt::bitwise_trie_ordered_multimap_impl(_std::move(src)) {
   }

   //! Destructor.
   ~trie_ordered_multimap() {
      clear();
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   trie_ordered_multimap & operator=(trie_ordered_multimap && src) {
      trie_ordered_multimap old(_std::move(*this));
      _pvt::bitwise_trie_ordered_multimap_impl::operator=(_std::move(src));
      return *this;
   }

   /*! Adds a key/value pair to the map.

   TODO: make two copies of this method, taking TValue const & and TValue &&; this requires more work to avoid
   the commented-out set_copy_construct() below for non-copiable types.

   @param key
      Key to add.
   @param value
      Value to add.
   @return
      Iterator to the newly added key/value.
   */
   iterator add(TKey key, TValue value) {
      type_void_adapter value_tva;
      value_tva.set_align<TValue>();
      value_tva.set_move_construct<TValue>();
      value_tva.set_size<TValue>();
      return iterator(this, key, _pvt::bitwise_trie_ordered_multimap_impl::add(
         value_tva, key_to_int(key), &value, true
      ));
   }

   /*! Returns an iterator set to the first key/value pair in the map.

   @return
      Reference to the first key/value in the map.
   */
   iterator begin() {
      auto kvp(find_first_key(false));
      return iterator(this, int_to_key(kvp.key), kvp.ln);
   }

   /*! Returns a const reference to the first key/value pair in the map.

   @return
      Const reference to the first key/value in the map.
   */
   const_iterator begin() const {
      return const_cast<trie_ordered_multimap *>(this)->begin();
   }

   /*! Returns a const reference to the first key/value pair in the map.

   @return
      Const reference to the first key/value in the map.
   */
   const_iterator cbegin() const {
      return const_cast<trie_ordered_multimap *>(this)->begin();
   }

   /*! Returns a const iterator set beyond the last key/value pair in the map.

   @return
      Const iterator set to beyond the last key/value pair.
   */
   const_iterator cend() {
      return const_cast<trie_ordered_multimap *>(this)->end();
   }

   //! Removes all elements from the map.
   void clear() {
      type_void_adapter value_tva;
      value_tva.set_align<TValue>();
      value_tva.set_destruct<TValue>();
      return _pvt::bitwise_trie_ordered_multimap_impl::clear(value_tva);
   }

   /*! Returns an iterator set beyond the last key/value pair in the map.

   @return
      Iterator set to beyond the last key/value pair.
   */
   iterator end() {
      return iterator(this, 0, nullptr);
   }

   /*! Returns a const iterator set beyond the last key/value pair in the map.

   @return
      Const iterator set to beyond the last key/value pair.
   */
   const_iterator end() const {
      return const_cast<trie_ordered_multimap *>(this)->begin();
   }

   /*! Searches the map for a specific key, returning an iterator to the first corresponding key/value pair if
   found.

   @param key
      Key to search for.
   @return
      Iterator to the first matching key/value, or end() if the key could not be found.
   */
   iterator find(TKey key) {
      return iterator(this, key, _pvt::bitwise_trie_ordered_multimap_impl::find(key_to_int(key)));
   }

   /*! Searches the map for a specific key, returning a const iterator to the first corresponding key/value
   pair if found.

   @param key
      Key to search for.
   @return
      Const iterator to the first matching key/value, or cend() if the key could not be found.
   */
   const_iterator find(TKey key) const {
      return const_cast<trie_ordered_multimap *>(this)->find(key);
   }

   /*! Returns a reference to the first key/value pair in the map.

   @return
      Reference to the first key/value in the map.
   */
   reference front() {
      auto kvp(find_first_key(true));
      return reference(int_to_key(kvp.key), kvp.ln->template value_ptr<TValue>());
   }

   /*! Returns a const reference to the first key/value pair in the map.

   @return
      Const reference to the first key/value in the map.
   */
   const_reference front() const {
      return const_cast<trie_ordered_multimap *>(this)->front();
   }

   /*! Removes and returns a key/value pair given an iterator to it.

   @param itr
      Iterator to the key/value to extract.
   @return
      Extracted key/value pair.
   */
   value_type pop(const_iterator itr) {
      validate_iterator(itr.ln);
      type_void_adapter value_tva;
      value_tva.set_align<TValue>();
      value_tva.set_destruct<TValue>();
      value_type ret(itr.key, _std::move(*static_cast<TValue *>(itr.ln->value_ptr(value_tva))));
      remove_value(value_tva, key_to_int(itr.key), itr.ln);
      return _std::move(ret);
   }

   /*! Removes and returns the key/value pair that would be returned as a reference by front().

   @return
      Extracted key/value pair.
   */
   value_type pop_front() {
      type_void_adapter value_tva;
      value_tva.set_align<TValue>();
      value_tva.set_destruct<TValue>();
      auto kvp(find_first_key(true));
      value_type ret(int_to_key(kvp.key), _std::move(*static_cast<TValue *>(kvp.ln->value_ptr(value_tva))));
      remove_value(value_tva, kvp.key, kvp.ln);
      return _std::move(ret);
   }

   /*! Removes a value given an iterator to it.

   @param itr
      Iterator to the key/value to remove.
   */
   void remove(const_iterator itr) {
      validate_iterator(itr.ln);
      type_void_adapter value_tva;
      value_tva.set_align<TValue>();
      value_tva.set_destruct<TValue>();
      remove_value(value_tva, key_to_int(itr.key), itr.ln);
   }

private:
   /*! Converts a non-template integer key to TKey.

   @param key
      Key to convert.
   @return
      Converted key.
   */
   static TKey int_to_key(std::uintmax_t key) {
      return static_cast<TKey>(key);
   }

   /*! Converts a TKey into a non-template integer key.

   @param key
      Key to convert.
   @return
      Converted key.
   */
   static std::uintmax_t key_to_int(TKey key) {
      return static_cast<std::uintmax_t>(key);
   }
};

}} //namespace lofty::collections

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_COLLECTIONS_TRIE_ORDERED_MULTIMAP_HXX
