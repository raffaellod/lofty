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

#include <abaclade/collections/detail/doubly_linked_list_impl.hxx>
#include <abaclade/numeric.hxx>
#include <abaclade/type_void_adapter.hxx>

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
   //! Stores a single value, as well as the doubly-linked list’s links.
   typedef doubly_linked_list_impl::node list_node;

   // Forward declaration.
   class tree_node;

   //! Stores a pointer to a tree_node or a list_node.
   union tree_or_list_node_ptr {
      //! Pointer to a tree_node.
      tree_node * tn;
      //! Pointer to a list_node.
      list_node * ln;

      //! Default constructor.
      tree_or_list_node_ptr() :
         tn(nullptr) {
      }
   };

   //! Non-leaf node.
   class tree_node {
   private:
      friend class scalar_keyed_trie_ordered_multimap_impl;

   private:
      //! Child node pointers; one for each permutation of the bits mapped to this node.
      tree_or_list_node_ptr m_apnChildren[smc_cBitPermutationsPerLevel];
   };

   //! Anchors value lists to the tree, mapping the last bits of the key.
   class anchor_node : public tree_node {
   private:
      friend class scalar_keyed_trie_ordered_multimap_impl;

   public:
      //! Default constructor.
      anchor_node() {
         memory::clear(&m_aplnChildrenLasts);
      }

   private:
      //! Child lists’ end pointers; one for each permutation of the bits mapped to this tree node.
      list_node * m_aplnChildrenLasts[smc_cBitPermutationsPerLevel];
   };

   //! Enables access to a single child slot in an anchor_node instance.
   class anchor_node_slot : public support_explicit_operator_bool<anchor_node_slot> {
   public:
      /*! Constructor.

      @param pan
         Pointer to the anchor_node that *this will wrap.
      @param iChild
         Child index.
      */
      anchor_node_slot(anchor_node * pan, unsigned iChild) :
         m_pan(pan),
         m_iChild(iChild) {
      }

      /*! Returns true if the object is usable.

      @return
         true if *this is usable, or false otherwise.
      */
      ABC_EXPLICIT_OPERATOR_BOOL() const {
         return m_pan != nullptr;
      }

      /*! Returns a pointer to the first node in the children list.

      @return
         Pointer to the first child.
      */
      list_node * first_child() const {
         return m_pan->m_apnChildren[m_iChild].ln;
      }

      /*! Returns a pointer to the last node in the children list.

      @return
         Pointer to the last child.
      */
      list_node * last_child() const {
         return m_pan->m_aplnChildrenLasts[m_iChild];
      }

      /*! Sets the pointer to the first child node.

      @param pln
         Pointer to the new first child.
      */
      void link_front(list_node * pln) const {
         doubly_linked_list_impl::link_front(
            &m_pan->m_apnChildren[m_iChild].ln, &m_pan->m_aplnChildrenLasts[m_iChild], pln
         );
      }

      /*! Sets the pointer to the last child node.

      @param typeValue
         Adapter for the list_node’s value type.
      @param p
         Pointer to the value to add.
      @param bMove
         true to move *p to the new node’s value, or false to copy it instead.
      @return
         Pointer to the newly-added list node.
      */
      list_node * push_back(type_void_adapter const & typeValue, void const * p, bool bMove) const {
         return doubly_linked_list_impl::push_back(
            typeValue, &m_pan->m_apnChildren[m_iChild].ln, &m_pan->m_aplnChildrenLasts[m_iChild], p,
            bMove
         );
      }

      /*! Unlinks and destructs a node from the child node list.

      @param typeValue
         Adapter for the list_node’s value type.
      */
      void remove(type_void_adapter const & typeValue, list_node * pln) const {
         doubly_linked_list_impl::remove(
            typeValue,
            &m_pan->m_apnChildren[m_iChild].ln, &m_pan->m_aplnChildrenLasts[m_iChild], pln
         );
      }

   private:
      //! Pointer to the wrapped anchor_node instance.
      anchor_node * m_pan;
      //! Child index.
      unsigned m_iChild;
   };

   //! Key/pointer-to-value pair.
   struct key_value_ptr {
      std::uintmax_t iKey;
      list_node * pln;

      key_value_ptr(std::uintmax_t _iKey, list_node * _pln) :
         iKey(_iKey),
         pln(_pln) {
      }
   };

public:
   /*! Constructor.

   @param cbKey
      Size of a key, as returned by sizeof.
   */
   scalar_keyed_trie_ordered_multimap_impl(std::size_t cbKey) :
      m_cValues(0),
      mc_iTreeAnchorsLevel(static_cast<std::uint8_t>(cbKey * CHAR_BIT / smc_cBitsPerLevel - 1)) {
   }

   /*! Move-constructor.

   @param sktommi
      Source object.
   */
   scalar_keyed_trie_ordered_multimap_impl(scalar_keyed_trie_ordered_multimap_impl && sktommi);

   //! Destructor.
   ~scalar_keyed_trie_ordered_multimap_impl() {
   }

   /*! Assignment operator.

   @param sktommi
      Source object.
   */
   scalar_keyed_trie_ordered_multimap_impl & operator=(
      scalar_keyed_trie_ordered_multimap_impl && sktommi
   );

   /*! Adds a key/value pair to the map.

   @param typeValue
      Adapter for the value’s type.
   @param iKey
      Key to add.
   @param pvalue
      Pointer to the value to add.
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
   key_value_ptr front();

   /*! Returns the count of values in the map. Note that this may be higher than the count of keys
   in the map.

   @return
      Count of values.
   */
   std::size_t size() const {
      return m_cValues;
   }

protected:
   /*! Removes a value from the map. If the corresponding key if unique, the key is completely
   removed from the map.

   @param typeValue
      Adapter for the value’s type.
   @param iKey
      Key associated to the value.
   @param pln
      Pointer to the node containing the value.
   */
   void remove_value(type_void_adapter const & typeValue, std::uintmax_t iKey, list_node * pln);

private:
   /*! Recursively destructs an anchor node and all its child lists.

   @param typeValue
      Adapter for the value’s type.
   @param pan
      Pointer to the target anchor node.
   */
   void destruct_anchor_node(type_void_adapter const & typeValue, anchor_node * pan);

   /*! Recursively destructs a tree node and all its children.

   @param typeValue
      Adapter for the value’s type.
   @param ptn
      Pointer to the top-level tree node.
   @param iLevel
      0-based index level of *ptn.
   */
   void destruct_tree_node(type_void_adapter const & typeValue, tree_node * ptn, unsigned iLevel);

   /*! Finds an anchor node slot (values list pointers) corresponding to the specified key, if
   present.

   @param iKey
      Key to search for.
   @return
      Matching anchor node slot; will evaluate to false if the key was not found in the map.
   */
   anchor_node_slot find_anchor_node_slot(std::uintmax_t iKey) const;

private:
   //! Pointer to the top-level tree node or only anchor node.
   tree_or_list_node_ptr m_pnRoot;
   //! Count of values. This may be more than the count of keys.
   std::size_t m_cValues;
   //! 0-based index of the last level in the tree, where nodes are of type anchor_node.
   std::uint8_t const mc_iTreeAnchorsLevel;
};

}}} //namespace abc::collections::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COLLECTIONS_DETAIL_TRIE_ORDERED_MULTIMAP_IMPL_HXX
