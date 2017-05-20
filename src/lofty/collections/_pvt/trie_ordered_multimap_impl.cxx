/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/bitmanip.hxx>
#include <lofty/collections.hxx>
#include <lofty/collections/_pvt/trie_ordered_multimap_impl.hxx>
#include <lofty/collections/vector.hxx>
#include <lofty/type_void_adapter.hxx>

#include <climits> // CHAR_BIT


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

bitwise_trie_ordered_multimap_impl::tree_node_slot
bitwise_trie_ordered_multimap_impl::tree_node_slot::first_used_child() const {
   LOFTY_TRACE_FUNC(this);

   /* Create a fictional tree_node_slot on the selected child, with index -1, and have it find its next used
   sibling which, due to starting from -1, is really the first used sibling. */
   return tree_node_slot(tn->children[child_index].tn, unsigned(-1)).next_used_sibling();
}

bitwise_trie_ordered_multimap_impl::tree_node_slot
bitwise_trie_ordered_multimap_impl::tree_node_slot::next_used_sibling() const {
   LOFTY_TRACE_FUNC(this);

   for (unsigned i = child_index; ++i < LOFTY_COUNTOF(tn->children); ) {
      if (tn->children[i].tn) {
         return tree_node_slot(tn, i);
      }
   }
   return tree_node_slot(nullptr, 0);
}


bitwise_trie_ordered_multimap_impl::bitwise_trie_ordered_multimap_impl(std::size_t key_byte_size) :
   size_(0),
   key_padding_bits(static_cast<std::uint8_t>((sizeof(std::uintmax_t) - key_byte_size) * CHAR_BIT)),
   tree_anchors_level(static_cast<std::uint8_t>(key_byte_size * CHAR_BIT / bits_per_level - 1)) {
}
bitwise_trie_ordered_multimap_impl::bitwise_trie_ordered_multimap_impl(
   bitwise_trie_ordered_multimap_impl && src
) :
   root(src.root),
   size_(src.size_),
   key_padding_bits(src.key_padding_bits),
   tree_anchors_level(src.tree_anchors_level) {
   src.root.tn = nullptr;
   src.size_ = 0;
}

bitwise_trie_ordered_multimap_impl & bitwise_trie_ordered_multimap_impl::operator=(
   bitwise_trie_ordered_multimap_impl && src
) {
   // Assume that the subclass has already moved *this out.
   root = src.root;
   src.root.tn = nullptr;
   size_ = src.size_;
   src.size_ = 0;
   return *this;
}

bitwise_trie_ordered_multimap_impl::list_node * bitwise_trie_ordered_multimap_impl::add(
   type_void_adapter const & value_type, std::uintmax_t key, void const * value, bool move
) {
   LOFTY_TRACE_FUNC(this/*, value_type*/, key, value, move);

   tree_node * parent;
   unsigned bits_permutation;
   // Descend into the tree, creating nodes as necessary until the path for key is complete.
   {
      /* child_in_parent points to *parent’s parent’s pointer to *parent. It’s a convenient to touch the
      pointer to *parent. */
      tree_or_list_node_ptr * child_in_parent = &root;
      std::uintmax_t key_remaining = key << key_padding_bits;
      unsigned level = 0;
      do {
         parent = child_in_parent->tn;
         if (!parent) {
            parent = (level == tree_anchors_level ? new anchor_node() : new tree_node());
            child_in_parent->tn = parent;
         }
         key_remaining = bitmanip::rotate_l(key_remaining, bits_per_level);
         bits_permutation = static_cast<unsigned>(key_remaining & (bit_permutations_per_level - 1));
         child_in_parent = &parent->children[bits_permutation];
      } while (++level <= tree_anchors_level);
   }
   // We got here, so *parent is actually an anchor_node. Append a new node to its list.
   anchor_node_slot anchor_slot(static_cast<anchor_node *>(parent), bits_permutation);
   list_node * ret = anchor_slot.push_back(value_type, value, move);
   ++size_;
   return ret;
}

void bitwise_trie_ordered_multimap_impl::clear(type_void_adapter const & value_type) {
   LOFTY_TRACE_FUNC(this/*, value_type*/);

   if (root.tn) {
      if (tree_anchors_level == 0) {
         // *root is an anchor.
         destruct_anchor_node(value_type, static_cast<anchor_node *>(root.tn));
      } else {
         destruct_tree_node(value_type, root.tn, 0);
      }
      root = tree_or_list_node_ptr();
      size_ = 0;
   }
}

void bitwise_trie_ordered_multimap_impl::destruct_anchor_node(
   type_void_adapter const & value_type, anchor_node * anchor
) {
   LOFTY_TRACE_FUNC(this/*, value_type*/, anchor);

   unsigned bits_permutation = 0;
   do {
      if (auto ln = anchor->children[bits_permutation].ln) {
         doubly_linked_list_impl::destruct_list(value_type, ln);
      }
   } while (++bits_permutation < bit_permutations_per_level);
   delete anchor;
}

void bitwise_trie_ordered_multimap_impl::destruct_tree_node(
   type_void_adapter const & value_type, tree_node * tn, unsigned level
) {
   LOFTY_TRACE_FUNC(this/*, value_type*/, tn, level);

   ++level;
   unsigned bits_permutation = 0;
   do {
      if (auto child = tn->children[bits_permutation].tn) {
         if (level == tree_anchors_level) {
            destruct_anchor_node(value_type, static_cast<anchor_node *>(child));
         } else {
            destruct_tree_node(value_type, child, level);
         }
      }
   } while (++bits_permutation < bit_permutations_per_level);
   delete tn;
}

auto bitwise_trie_ordered_multimap_impl::find(std::uintmax_t key) const -> list_node * {
   LOFTY_TRACE_FUNC(this, key);

   if (auto anchor_slot = find_anchor_node_slot(key)) {
      return anchor_slot.first_child();
   } else {
      return nullptr;
   }
}

bitwise_trie_ordered_multimap_impl::anchor_node_slot
bitwise_trie_ordered_multimap_impl::find_anchor_node_slot(std::uintmax_t key) const {
   LOFTY_TRACE_FUNC(this, key);

   tree_node * parent = root.tn;
   std::uintmax_t key_remaining = key << key_padding_bits;
   unsigned level = 0;
   for (;;) {
      key_remaining = bitmanip::rotate_l(key_remaining, bits_per_level);
      auto bits_permutation = static_cast<unsigned>(key_remaining & (bit_permutations_per_level - 1));
      if (level == tree_anchors_level) {
         // At this level, *parent is an anchor.
         return anchor_node_slot(static_cast<anchor_node *>(parent), bits_permutation);
      } else if (!parent) {
         return anchor_node_slot(nullptr, 0);
      }
      parent = parent->children[bits_permutation].tn;
      ++level;
   }
}

bitwise_trie_ordered_multimap_impl::key_value_ptr bitwise_trie_ordered_multimap_impl::find_first_key(
   bool throw_if_empty
) const {
   LOFTY_TRACE_FUNC(this);

   tree_or_list_node_ptr child;
   std::uintmax_t key = 0;

   // Descend into the tree.
   tree_node * parent = root.tn;
   unsigned level = 0;
   do {
      if (!parent) {
         break;
      }
      // Look for the left-most branch to descend into.
      unsigned child_index = 0;
      do {
         child = parent->children[child_index];
         if (child.tn) {
            // Prepend the selected bit permutation to the key.
            key <<= bits_per_level;
            key |= static_cast<std::uintmax_t>(child_index);
            break;
         }
      } while (++child_index < bit_permutations_per_level);
      parent = child.tn;
   } while (++level <= tree_anchors_level);

   // We got to the leaf level, so we can return child.ln, though it might be nullptr.
   if (!child.ln && throw_if_empty) {
      LOFTY_THROW(collections::bad_access, ());
   }
   return key_value_ptr(key, child.ln);
}

bitwise_trie_ordered_multimap_impl::key_value_ptr bitwise_trie_ordered_multimap_impl::find_next_key(
   std::uintmax_t prev_key
) const {
   LOFTY_TRACE_FUNC(this, prev_key);

   vector<tree_node_slot, sizeof(std::uintmax_t) * CHAR_BIT / bits_per_level> path_nodes;

   tree_node * parent = root.tn;
   std::uintmax_t key = 0, prev_key_remaining = prev_key << key_padding_bits;
   unsigned level = 0;
   for (;;) {
      prev_key_remaining = bitmanip::rotate_l(prev_key_remaining, bits_per_level);
      auto bits_permutation = static_cast<unsigned>(prev_key_remaining & (bit_permutations_per_level - 1));
      if (!parent) {
         break;
      }
      path_nodes.push_back(tree_node_slot(parent, bits_permutation));
      // Copy the bits permutation from prev_key to key.
      key <<= bits_per_level;
      key |= static_cast<std::uintmax_t>(bits_permutation);
      if (level == tree_anchors_level) {
         break;
      }
      parent = parent->children[bits_permutation].tn;
      ++level;
   }

   // This loop might pop levels from path_nodes if they have no next sibling.
   while (path_nodes) {
      if (auto next_sibling_node = path_nodes.back().next_used_sibling()) {
         // Replace the sibling and its bits permutation.
         key &= ~static_cast<std::uintmax_t>(bit_permutations_per_level - 1);
         key |= static_cast<std::uintmax_t>(next_sibling_node.index());
         // If the path is not deep enough, descend the “first nexts” till the anchors level.
         for (; level < tree_anchors_level; ++level) {
            next_sibling_node = next_sibling_node.first_used_child();
            key <<= bits_per_level;
            key |= static_cast<std::uintmax_t>(next_sibling_node.index());
         }
         return key_value_ptr(key, next_sibling_node.child().ln);
      }
      // This path level has no siblings to offer, try with the level above it.
      path_nodes.pop_back();
      --level;
      // Shift out the bits for the level we just dropped.
      key >>= bits_per_level;
   }
   // No next value to return.
   return key_value_ptr(0, nullptr);
}

void bitwise_trie_ordered_multimap_impl::prune_branch(std::uintmax_t key) {
   tree_node * tn = root.tn, ** topmost_nullable_node = &root.tn;
   tree_node * ancestors_stack[bit_permutations_per_level];
   std::uintmax_t key_remaining = key << key_padding_bits;
   unsigned level = 0;
   int last_non_empty_level = -1;
   do {
      key_remaining = bitmanip::rotate_l(key_remaining, bits_per_level);
      auto bits_permutation = static_cast<unsigned>(key_remaining & (bit_permutations_per_level - 1));
      // Check if the node has any children other than [bits_permutation].
      unsigned child_index = 0;
      do {
         if (child_index != bits_permutation && tn->children[child_index].tn) {
            last_non_empty_level = static_cast<int>(level);
            topmost_nullable_node = &tn->children[bits_permutation].tn;
            break;
         }
      } while (++child_index < bit_permutations_per_level);
      // Push this node on the ancestors stack.
      ancestors_stack[level] = tn;
      tn = tn->children[bits_permutation].tn;
   } while (++level <= tree_anchors_level);

   // Now prune every empty level.
   while (static_cast<int>(--level) > last_non_empty_level) {
      if (level == tree_anchors_level) {
         delete static_cast<anchor_node *>(ancestors_stack[level]);
      } else {
         delete ancestors_stack[level];
      }
   }
   // Make the last non-empty level no longer point to the branch we just pruned.
   *topmost_nullable_node = nullptr;
}

void bitwise_trie_ordered_multimap_impl::remove_value(
   type_void_adapter const & value_type, std::uintmax_t key, list_node * ln
) {
   LOFTY_TRACE_FUNC(this/*, value_type*/, key, ln);

   if (ln->next() && ln->prev()) {
      // *ln is in the middle of its list, so we don’t need to find and update the anchor.
      doubly_linked_list_impl::remove(value_type, nullptr, nullptr, ln);
   } else if (!ln->next() && !ln->prev()) {
      // *ln is in the only node in its list, so we can destruct it and prune the whole branch.
      value_type.destruct(ln->value_ptr(value_type));
      delete ln;
      prune_branch(key);
   } else {
      // *ln is the first or the last node in its list, so we need to update the anchor.
      if (auto anchor_slot = find_anchor_node_slot(key)) {
         anchor_slot.remove(value_type, ln);
      } else {
         LOFTY_THROW(out_of_range, ());
      }
   }
   --size_;
}

/*static*/ void bitwise_trie_ordered_multimap_impl::validate_iterator(list_node const * ln) {
   if (!ln) {
      LOFTY_THROW(out_of_range, ());
   }
}

}}} //namespace lofty::collections::_pvt
