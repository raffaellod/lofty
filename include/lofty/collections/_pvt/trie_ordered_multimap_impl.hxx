/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_COLLECTIONS__PVT_TRIE_ORDERED_MULTIMAP_IMPL_HXX
#define _LOFTY_COLLECTIONS__PVT_TRIE_ORDERED_MULTIMAP_IMPL_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/collections/_pvt/doubly_linked_list_impl.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

//! Implementation of lofty::collections::trie_ordered_multimap for scalar key types.
class LOFTY_SYM bitwise_trie_ordered_multimap_impl :
   public support_explicit_operator_bool<bitwise_trie_ordered_multimap_impl> {
private:
   /*! Determines the compactness of each level of the tree. Packing multiple bits on each level results in
   faster lookups and fewer memory allocations, at the cost of increased slack in each tree node. */
   static unsigned const bits_per_level = 4;
   //! Count of children pointers that each tree node needs.
   static unsigned const bit_permutations_per_level = 1 << bits_per_level;

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
      friend class bitwise_trie_ordered_multimap_impl;
      friend class tree_node_slot;

   private:
      //! Child node pointers; one for each permutation of the bits mapped to this node.
      tree_or_list_node_ptr children[bit_permutations_per_level];
   };

   //! Enables access to a single child slot in an anchor_node instance.
   class tree_node_slot : public support_explicit_operator_bool<tree_node_slot> {
   public:
      /*! Constructor.

      @param tn_
         Pointer to the tree_node that *this will wrap.
      @param child_index_
         Child index.
      */
      tree_node_slot(tree_node * tn_, unsigned child_index_) :
         tn(tn_),
         child_index(child_index_) {
      }

      /*! Boolean evaluation operator.

      @return
         true if the object is usable, or false otherwise.
      */
      LOFTY_EXPLICIT_OPERATOR_BOOL() const {
         return tn != nullptr;
      }

      /*! Returns a pointer to the selected child in the children list.

      @return
         Pointer to the first child.
      */
      tree_or_list_node_ptr child() const {
         return tn->children[child_index];
      }

      /*! Returns the first non-nullptr child under the selected child.

      @return
         First non-nullptr (in-use) child.
      */
      tree_node_slot first_used_child() const;

      /*! Returns the child index.

      @return
         Child index.
      */
      unsigned index() const {
         return child_index;
      }

      /*! Finds the next non-nullptr child of the same tree node.

      @return
         Next non-nullptr (in-use) sibling.
      */
      tree_node_slot next_used_sibling() const;

   private:
      //! Pointer to the wrapped tree_node instance.
      tree_node * tn;
      //! Child index.
      unsigned child_index;
   };

   //! Anchors value lists to the tree, mapping the last bits of the key.
   class anchor_node : public tree_node {
   private:
      friend class bitwise_trie_ordered_multimap_impl;

   public:
      //! Default constructor.
      anchor_node() {
         memory::clear(&child_lists_lasts);
      }

   private:
      //! Child lists’ end pointers; one for each permutation of the bits mapped to this tree node.
      list_node * child_lists_lasts[bit_permutations_per_level];
   };

   //! Enables access to a single child slot in an anchor_node instance.
   class anchor_node_slot : public support_explicit_operator_bool<anchor_node_slot> {
   public:
      /*! Constructor.

      @param anchor_
         Pointer to the anchor_node that *this will wrap.
      @param child_index_
         Child index.
      */
      anchor_node_slot(anchor_node * anchor_, unsigned child_index_) :
         anchor(anchor_),
         child_index(child_index_) {
      }

      /*! Boolean evaluation operator.

      @return
         true if the object is usable, or false otherwise.
      */
      LOFTY_EXPLICIT_OPERATOR_BOOL() const {
         return anchor != nullptr;
      }

      /*! Returns a pointer to the first node in the children list.

      @return
         Pointer to the first child.
      */
      list_node * first_child() const {
         return anchor->children[child_index].ln;
      }

      /*! Returns a pointer to the last node in the children list.

      @return
         Pointer to the last child.
      */
      list_node * last_child() const {
         return anchor->child_lists_lasts[child_index];
      }

      /*! Sets the pointer to the first child node.

      @param value_type
         Adapter for the list_node’s value type.
      @param value
         Pointer to the value to add.
      @param move
         true to move *value to the new node’s value, or false to copy it instead.
      @return
         Pointer to the newly-added list node.
      */
      list_node * push_front(type_void_adapter const & value_type, void const * value, bool move) const {
         return doubly_linked_list_impl::push_front(
            value_type, &anchor->children[child_index].ln,
            &anchor->child_lists_lasts[child_index], value, move
         );
      }

      /*! Sets the pointer to the last child node.

      @param value_type
         Adapter for the list_node’s value type.
      @param value
         Pointer to the value to add.
      @param move
         true to move *value to the new node’s value, or false to copy it instead.
      @return
         Pointer to the newly-added list node.
      */
      list_node * push_back(type_void_adapter const & value_type, void const * value, bool move) const {
         return doubly_linked_list_impl::push_back(
            value_type, &anchor->children[child_index].ln,
            &anchor->child_lists_lasts[child_index], value, move
         );
      }

      /*! Unlinks and destructs a node from the child node list.

      @param value_type
         Adapter for the list_node’s value type.
      @param ln
         Pointer to the node to unlink and destruct.
      */
      void remove(type_void_adapter const & value_type, list_node * ln) const {
         doubly_linked_list_impl::remove(
            value_type, &anchor->children[child_index].ln, &anchor->child_lists_lasts[child_index], ln
         );
      }

   private:
      //! Pointer to the wrapped anchor_node instance.
      anchor_node * anchor;
      //! Child index.
      unsigned child_index;
   };

   //! Key/pointer-to-value pair.
   struct key_value_ptr {
      //! Key.
      std::uintmax_t key;
      //! Pointer to the node containing the value.
      list_node * ln;

      /*! Constructor.

      @param key_
         Key to hold.
      @param ln_
         Pointer to the node to hold.
      */
      key_value_ptr(std::uintmax_t key_, list_node * ln_) :
         key(key_),
         ln(ln_) {
      }
   };

public:
   /*! Constructor.

   @param key_byte_size
      Size of a key, as returned by sizeof.
   */
   bitwise_trie_ordered_multimap_impl(std::size_t key_byte_size);

   /*! Move constructor.

   @param src
      Source object.
   */
   bitwise_trie_ordered_multimap_impl(bitwise_trie_ordered_multimap_impl && src);

   //! Destructor.
   ~bitwise_trie_ordered_multimap_impl() {
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   bitwise_trie_ordered_multimap_impl & operator=(bitwise_trie_ordered_multimap_impl && src);

   /*! Boolean evaluation operator.

   @return
      true if the map is not empty, or false otherwise.
   */
   LOFTY_EXPLICIT_OPERATOR_BOOL() const {
      return size_ > 0;
   }

   /*! Adds a key/value pair to the map.

   @param value_type
      Adapter for the value’s type.
   @param key
      Key to add.
   @param value
      Pointer to the value to add.
   @param move
      true to move *value to the new node’s value, or false to copy it instead.
   @return
      Pointer to the newly-added list node.
   */
   list_node * add(type_void_adapter const & value_type, std::uintmax_t key, void const * value, bool move);

   /*! Removes all elements from the map.

   @param value_type
      Adapter for the value’s type.
   */
   void clear(type_void_adapter const & value_type);

   /*! Searches the multimap for a specific key, returning a pointer to the first corresponding list node if
   found.

   @param key
      Key to search for.
   @return
      Pointer to the to the first matching list node, or nullptr if the key could not be found.
   */
   list_node * find(std::uintmax_t key) const;

   /*! Returns the count of values in the map. Note that this may be higher than the count of keys in the map.

   @return
      Count of values.
   */
   std::size_t size() const {
      return size_;
   }

protected:
   /*! Finds the first key in the map, returning a pointer to the first corresponding value.

   @param throw_if_empty
      If true and the trie is empty, a collections::bad_access exception will be thrown.
   @return
      Pointer to the first key/value pair in the map, or a nullptr value if the map is empty.
   */
   key_value_ptr find_first_key(bool throw_if_empty) const;

   /*! Finds the next key minimally greater than the specified one, returning a pointer to the first
   corresponding value.

   @param prev_key
      Key to search the next of.
   @return
      Pointer to the first matching “next” key/value pair, or a nullptr value if no “next” key could be found
      in the map.
   */
   key_value_ptr find_next_key(std::uintmax_t prev_key) const;

   /*! Removes a value from the map. If the corresponding key if unique, the key is completely removed from
   the map.

   @param value_type
      Adapter for the value’s type.
   @param key
      Key associated to the value.
   @param ln
      Pointer to the node containing the value.
   */
   void remove_value(type_void_adapter const & value_type, std::uintmax_t key, list_node * ln);

   /*! Validates the members of an iterator so that the latter can be used safely. Throws a
   collections::out_of_range if the iterator is not referencing a value in the map.

   This method is static so that it can (it will) validate that this is not nullptr before dereferencing it.

   @param ln
      Pointer to the node the iterator is currently referencing.
   */
   static void validate_iterator(list_node const * ln);

private:
   /*! Recursively destructs an anchor node and all its child lists.

   @param value_type
      Adapter for the value’s type.
   @param anchor
      Pointer to the target anchor node.
   */
   void destruct_anchor_node(type_void_adapter const & value_type, anchor_node * anchor);

   /*! Recursively destructs a tree node and all its children.

   @param value_type
      Adapter for the value’s type.
   @param tn
      Pointer to the top-level tree node.
   @param level
      0-based index level of *tn.
   */
   void destruct_tree_node(type_void_adapter const & value_type, tree_node * tn, unsigned level);

   /*! Finds an anchor node slot (values list pointers) corresponding to the specified key, if present.

   @param key
      Key to search for.
   @return
      Matching anchor node slot; will evaluate to false if the key was not found in the map.
   */
   anchor_node_slot find_anchor_node_slot(std::uintmax_t key) const;

   /*! Removes all the tree nodes mapping to the specified key, as long as they have no other children.

   @param key
      Key designating the branch to prune.
   */
   void prune_branch(std::uintmax_t key);

private:
   //! Pointer to the top-level tree node or only anchor node.
   tree_or_list_node_ptr root;
   //! Count of values. This may be more than the count of keys.
   std::size_t size_;
   //! Number of bits added to a key to make it as large as std::uintmax_t.
   std::uint8_t const key_padding_bits;
   //! 0-based index of the last level in the tree, where nodes are of type anchor_node.
   std::uint8_t const tree_anchors_level;
};

}}} //namespace lofty::collections::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_COLLECTIONS__PVT_TRIE_ORDERED_MULTIMAP_IMPL_HXX
