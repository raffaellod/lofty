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

#ifndef _ABACLADE_COLLECTIONS_DETAIL_TRIE_ORDERED_MULTIMAP_IMPL_HXX
#define _ABACLADE_COLLECTIONS_DETAIL_TRIE_ORDERED_MULTIMAP_IMPL_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/collections/type_void_adapter.hxx>
#include <abaclade/numeric.hxx>

#include <climits> // CHAR_BIT


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

//! Implementation of abc::collections::trie_ordered_multimap for scalar key types.
class ABACLADE_SYM scalar_keyed_trie_ordered_multimap_impl {
private:
   /*! Determines the compactness of each level of the tree. Packing multiple bits on each level
   results in faster lookups and fewer memory allocations, at the cost of increased slack in each
   tree node. */
   static unsigned const smc_cBitsPerLevel = 4;
   //! Count of children pointers that each tree node needs.
   static unsigned const smc_cBitPermutationsPerLevel = 1 << smc_cBitsPerLevel;

protected:
   /*! Abstract node. Defined to avoid using void * in code where pointers’ type change depending on
   the level in the tree. */
   class node {
   };

   //! Stores a single value, as well as the doubly-linked list’s links.
   class list_node : public node {
   public:
      //! Constructor.
      list_node() :
         m_plnNext(nullptr),
         m_plnPrev(nullptr) {
      }

      void unlink_and_destruct(type_void_adapter const & type) const;

      /*! Returns a pointer to the contained value.

      @param type
         Adapter for the value’s type.
      @return
         Pointer to the contained value.
      */
      void * value_ptr(type_void_adapter const & type) const;

      /*! Returns a typed pointer to the contained TValue.

      @return
         Pointer to the contained value.
      */
      template <typename TValue>
      TValue * value_ptr() const {
         type_void_adapter typeValue;
         typeValue.set_align<TValue>();
         return static_cast<TValue *>(value_ptr(typeValue));
      }

   public:
      //! Pointer to the next node.
      list_node * m_plnNext;
      //! Pointer to the previous node.
      list_node * m_plnPrev;
      // The contained value of type T follow immediately, taking alignment into consideration.
   };

   //! Non-leaf node.
   class tree_node : public node {
   public:
      //! Constructor.
      tree_node() {
         memory::clear(&m_apnChildren);
      }

   public:
      //! Child prefix pointers; one for each permutation of the bits mapped to this tree node.
      node * m_apnChildren[smc_cBitPermutationsPerLevel];
   };

   //! Anchors value lists to the tree, mapping the last bits of the key.
   class anchor_node : public tree_node {
   public:
      //! Constructor.
      anchor_node() {
         memory::clear(&m_aplnChildrenLasts);
      }

   public:
      //! Child lists end pointers; one for each permutation of the bits mapped to this tree node.
      list_node * m_aplnChildrenLasts[smc_cBitPermutationsPerLevel];
   };

   struct indexed_anchor {
      anchor_node * pan;
      unsigned iBitsPermutation;

      indexed_anchor(anchor_node * _pan, unsigned _iBitsPermutation) :
         pan(_pan),
         iBitsPermutation(_iBitsPermutation) {
      }
   };

   //! Key/pointer-to-value pair.
   struct key_value_pair {
      std::uintmax_t iKey;
      list_node * pln;

      key_value_pair(std::uintmax_t _iKey, list_node * _pln) :
         iKey(_iKey),
         pln(_pln) {
      }
   };

public:
   /*! Default constructor.

   @param cbKey
      Size of a key, as returned by sizeof.
   */
   scalar_keyed_trie_ordered_multimap_impl(std::size_t cbKey) :
      m_pnRoot(nullptr),
      m_cValues(0),
      mc_iTreeAnchorsLevel(static_cast<std::uint8_t>(cbKey * CHAR_BIT / smc_cBitsPerLevel - 1)) {
   }

   /*! Move-constructor.

   @param tommi
      Source object.
   */
   scalar_keyed_trie_ordered_multimap_impl(scalar_keyed_trie_ordered_multimap_impl && tommi);

   //! Destructor.
   ~scalar_keyed_trie_ordered_multimap_impl() {
   }

   /*! Adds a key/value pair to the map.

   @param typeValue
      Adapter for the value’s type.
   @param iKey
      Key to add.
   @param value
      Value to add.
   @param bMove
      true to move *pValue to the new node’s value, or false to copy it instead.
   @return
      Pointer to the newly-added list node.
   */
   list_node * add(
      type_void_adapter const & typeValue, std::uintmax_t iKey, void const * pValue, bool bMove
   );

   /*! Removes all elements from the map.

   @param typeValue
      Adapter for the value’s type.
   */
   void clear(type_void_adapter const & typeValue);

   /*! Returns true if the map contains no elements.

   @return
      true if the map is empty, or false otherwise.
   */
   bool empty() const {
      return m_cValues == 0;
   }

   /*! Searches the multimap for a specific key, returning a pointer to the first corresponding list
   node if found.

   @param iKey
      Key to search for.
   @return
      Pointer to the to the first matching list node, or nullptr if the key could not be found.
   */
   list_node * find(std::uintmax_t iKey);

   /*! Returns a pointer to the first value in the map.

   @return
      Pointer to the first value in the map.
   */
   key_value_pair front();

   /*! Returns the count of values in the map. Note that this may be higher than the count of keys
   in the map.

   @return
      Count of values.
   */
   std::size_t size() const {
      return m_cValues;
   }

protected:
   void remove_value(type_void_adapter const & typeValue, std::uintmax_t iKey, list_node * pln);

private:
   indexed_anchor descend_to_anchor(std::uintmax_t iKey) const;

   void destruct_list(type_void_adapter const & type, list_node * pln);

   void destruct_anchor_node(type_void_adapter const & typeValue, anchor_node * pan);

   void destruct_tree_node(type_void_adapter const & typeValue, tree_node * ptn, unsigned iLevel);

private:
   /*! Pointer to the top-level node. The type should be tree_node *, but some code requires this to
   be the same as tree_node::m_apnChildren[0]. */
   node * m_pnRoot;
   //! Count of values. This may be more than the count of keys.
   std::size_t m_cValues;
   //! 0-based number of the last level in the tree, where nodes are of type anchor_node.
   std::uint8_t const mc_iTreeAnchorsLevel;
};

}}} //namespace abc::collections::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COLLECTIONS_DETAIL_TRIE_ORDERED_MULTIMAP_IMPL_HXX
