/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015
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

#ifndef _ABACLADE_COLLECTIONS_TRIE_ORDERED_MULTIMAP_HXX
#define _ABACLADE_COLLECTIONS_TRIE_ORDERED_MULTIMAP_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/collections/detail/trie_ordered_multimap_impl.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections {

/*! Key/value multimap using a trie.

Specializations are defined only for scalar and string key types.

This implementation focuses on insertion and removal speed, providing O(1) insertion, O(1)
extraction of the first element, and O(1) extraction of any other element given an iterator to it.
In the scalar key case, this is achieved by using a trie where each node consumes a nibble (4 bits)
of the key; values are stored in doubly-linked lists connected to the leaves of the binary tree.

For example, let’s consider a hypothetical trie using integer 4-bit keys where each node consumes
one bit, populated with the following data:

   @verbatim
   ┌────────┬───────┐
   │ Key    │ Value │
   ├────────┼───────┤
   │ 0b0001 │ a     │
   │ 0b0101 │ b     │
   │ 0b0101 │ c     │
   └────────┴───────┘
   @endverbatim

The internal data representation of the above would be:

   @verbatim
    ⎧ ┌───────┬───────┐
    ⎪ │nullptr│0xptr  │
    ⎪ └───────┴───────┘
    ⎪          │
    ⎪          ▼
    ⎪          ┌───────┬───────┐
    ⎪          │0xptr  │nullptr│
    ⎪          └───────┴───────┘
    ⎪           │
   1⎨           ▼
    ⎪           ┌───────┬───────┐
    ⎪           │0xptr  │0xptr  │
    ⎪           └───────┴───────┘
    ⎪    ┌───────┘       └──────────────────────┐
    ⎪    ▼                                      ▼
    ⎪  ⎧ ┌───────┬───────╥───────┬───────┐      ┌───────┬───────╥───────┬───────┐
    ⎪ 2⎨ │0xptr  │nullptr║0xptr  │nullptr│      │0xptr  │nullptr║0xptr  │nullptr│
    ⎩  ⎩ └───────┴───────╨───────┴───────┘      └───────┴───────╨───────┴───────┘
          ├───────────────┘                      │               └─────┐
          ▼                                      ▼                     ▼
    ⎧     ┌───────┬───────┬───┐                  ┌───────┬───────┬───┐ ┌───────┬───────┬───┐
   3⎨     │nullptr│nullptr│ a │                  │nullptr│0xptr  │ b │ │0xptr  │nullptr│ c │
    ⎩     └───────┴───────┴───┘                  └───────┴───────┴───┘ └───────┴───────┴───┘
                                                 ▲        │            ▲│
                                                 │        └────────────┘│
                                                 └──────────────────────┘
   @endverbatim

In the graph above, 1 is the prefix tree, where each node contains pointers to its children; 2 is
the leaf level, where each node also contains pointers the last nodes of each list of identically-
keyed values; 3 is the value level, containing doubly-linked lists of identically-keyed values. */
template <typename TKey, typename TValue, unsigned t_iImplType = (
   std::is_scalar<TKey>::value ? 1 : 0
)>
class trie_ordered_multimap;

// Partial specialization for scalar key types.
template <typename TKey, typename TValue>
class trie_ordered_multimap<TKey, TValue, 1> :
   public detail::scalar_keyed_trie_ordered_multimap_impl {
public:
   //! Mapped value type.
   typedef TValue mapped_type;

protected:
   /*! Key/value pair type. Template-typed version of
   detail::scalar_keyed_trie_ordered_multimap_impl::key_value_pair. */
   struct key_value_pair {
      TKey key;
      TValue value;
   };

   /*! Pointer type returned by iterator::operator->() that behaves like a pointer, but in fact
   includes the object it points to.

   Needed because iterator::operator->() must return a pointer-like type to a key/value pair
   (value_type), but key/value pairs are never stored anywhere in the map. */
   template <typename TPair>
   class pair_ptr {
   public:
      //! Constructor. See TPair::TPair().
      pair_ptr(TKey key, TValue * pvalue) :
         m_pair(key, pvalue) {
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

   //! Const key/value pair iterator type.
   class const_iterator {
   private:
      friend class trie_ordered_multimap;

   public:
      //! Const key/value type.
      struct value_type {
         TKey const key;
         TValue const & value;

         //! Constructor.
         value_type(TKey _key, TValue const * pvalue) :
            key(*_key), value(*pvalue) {
         }
      };

      typedef std::ptrdiff_t difference_type;
      typedef std::forward_iterator_tag iterator_category;
      typedef value_type * pointer;
      typedef value_type & reference;

   public:
      //! Default constructor.
      const_iterator() :
         m_key(),
         m_pln(nullptr) {
      }

   private:
      /*! Constructor.

      @param key
         Key associated to the value referred to by the iterator.
      @param pln
         Pointer to the value referred to by the iterator, or nullptr to create an “end” iterator.
      */
      const_iterator(TKey key, list_node * pln) :
         m_key(key),
         m_pln(pln) {
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current key/value pair.
      */
      pair_ptr<value_type> operator->() const {
         return pair_ptr<value_type>(m_key, m_pln->value_ptr<TValue>());
      }

   protected:
      TKey m_key;
      list_node * m_pln;
   };

   //! Non-const key/value pair iterator type.
   class iterator : public const_iterator {
   private:
      friend class trie_ordered_multimap;

   public:
      //! Key/value type.
      struct value_type {
         TKey const key;
         TValue & value;

         value_type(TKey _key, TValue * pvalue) :
            key(_key), value(*pvalue) {
         }
      };

      typedef value_type * pointer;
      typedef value_type & reference;

   public:
      //! Default constructor.
      iterator() {
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current key/value pair.
      */
      pair_ptr<value_type> operator->() const {
         return pair_ptr<value_type>(this->m_key, this->m_pln->template value_ptr<TValue>());
      }

   private:
      /*! Constructor.

      @param key
         Key associated to the value referred to by the iterator.
      @param pln
         Pointer to the value referred to by the iterator, or nullptr to create an “end” iterator.
      */
      iterator(TKey key, list_node * pln) :
         const_iterator(key, pln) {
      }

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

   @param tomm
      Source object.
   */
   trie_ordered_multimap() {
   }
   trie_ordered_multimap(trie_ordered_multimap && tomm) :
      detail::scalar_keyed_trie_ordered_multimap_impl(std::move(tomm)) {
   }

   //! Destructor.
   ~trie_ordered_multimap() {
      clear();
   }

   /*! Assignment operator.

   @param tomm
      Source object.
   */
   trie_ordered_multimap & operator=(trie_ordered_multimap && tomm) {
      trie_ordered_multimap tommOld(std::move(*this));
      m_pnRoot = tomm.m_pnRoot;
      tomm.m_pnRoot = nullptr;
      m_cValues = tomm.m_cValues;
      tomm.m_cValues = 0;
      return *this;
   }

   /*! Adds a key/value pair to the map.

   TODO: make two copies of this method, taking TValue const & and TValue &&; this requires more
   work to avoid the commented-out set_copy_construct() below for non-copiable types.

   @param key
      Key to add.
   @param value
      Value to add.
   @return
      Iterator to the newly added key/value.
   */
   iterator add(TKey key, TValue value) {
      type_void_adapter typeValue;
      typeValue.set_align<TValue>();
      typeValue.set_move_construct<TValue>();
      typeValue.set_size<TValue>();
      return iterator(key, detail::scalar_keyed_trie_ordered_multimap_impl::add(
         typeValue, key_to_int(key), &value, true
      ));
   }

   //! Removes all elements from the map.
   void clear() {
      type_void_adapter typeValue;
      typeValue.set_align<TValue>();
      typeValue.set_size<TValue>();
      typeValue.set_destruct<TValue>();
      return detail::scalar_keyed_trie_ordered_multimap_impl::clear(typeValue);
   }

   /*! Searches the multimap for a specific key, returning an iterator to the first corresponding
   key/value pair if found.

   @param key
      Key to search for.
   @return
      Iterator to the first matching key/value, or cend() if the key could not be found.
   */
   iterator find(TKey key) {
      return iterator(key, detail::scalar_keyed_trie_ordered_multimap_impl::find(key_to_int(key)));
   }
   const_iterator find(TKey key) const {
      return const_cast<trie_ordered_multimap *>(this)->find(key);
   }

   value_type front() {
      auto kvp(detail::scalar_keyed_trie_ordered_multimap_impl::front());
      return value_type(int_to_key(kvp.iKey), kvp.pln->template value_ptr<TValue>());
   }
   const_value_type front() const {
      return const_cast<trie_ordered_multimap *>(this)->front();
   }

   /*! Removes and returns a key/value pair given an iterator to it.

   @param it
      Iterator to the key/value to extract.
   @return
      Extracted key/value pair.
   */
   key_value_pair pop(const_iterator it) {
      type_void_adapter typeValue;
      typeValue.set_align<TValue>();
      typeValue.set_destruct<TValue>();
      key_value_pair kvpRet(std::move(*static_cast<TValue *>(it.m_pln->value_ptr(typeValue))));
      remove_value(typeValue, key_to_int(it.m_key), it.m_pln);
      return std::move(kvpRet);
   }

   /*! Removes and returns the key/value pair that would be returned as a reference by front().

   @return
      Extracted key/value pair.
   */
   key_value_pair pop_front() {
      return pop(front());
   }

   /*! Removes a value given an iterator to it.

   @param it
      Iterator to the key/value to remove.
   */
   void remove(const_iterator it) {
      type_void_adapter typeValue;
      typeValue.set_align<TValue>();
      typeValue.set_destruct<TValue>();
      remove_value(typeValue, key_to_int(it.m_key), it.m_pln);
   }

private:
   static TKey int_to_key(std::uintmax_t iKey) {
      return static_cast<TKey>(iKey);
   }

   static std::uintmax_t key_to_int(TKey key) {
      return static_cast<std::uintmax_t>(key);
   }
};

}} //namespace abc::collections

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COLLECTIONS_TRIE_ORDERED_MULTIMAP_HXX
