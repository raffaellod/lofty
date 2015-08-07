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

#include <abaclade.hxx>
#include <abaclade/bitmanip.hxx>
#include <abaclade/collections/detail/trie_ordered_multimap_impl.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

bitwise_trie_ordered_multimap_impl::tree_node_slot
bitwise_trie_ordered_multimap_impl::tree_node_slot::first_used_child() const {
   ABC_TRACE_FUNC(this);

   /* Create a fictional tree_node_slot on the selected child, with index -1, and have it find its
   next used sibling which, due to starting from -1, is really the first used sibling. */
   return tree_node_slot(m_ptn->m_apnChildren[m_iChild].tn, unsigned(-1)).next_used_sibling();
}

bitwise_trie_ordered_multimap_impl::tree_node_slot
bitwise_trie_ordered_multimap_impl::tree_node_slot::next_used_sibling() const {
   ABC_TRACE_FUNC(this);

   unsigned i = m_iChild;
   while (++i < ABC_COUNTOF(m_ptn->m_apnChildren)) {
      if (m_ptn->m_apnChildren[i].tn) {
         return tree_node_slot(m_ptn->m_apnChildren[i].tn, i);
      }
   }
   return tree_node_slot(nullptr, 0);
}


bitwise_trie_ordered_multimap_impl::bitwise_trie_ordered_multimap_impl(
   bitwise_trie_ordered_multimap_impl && bwtommi
) :
   m_pnRoot(bwtommi.m_pnRoot),
   m_cValues(bwtommi.m_cValues),
   mc_iKeyPadding(bwtommi.mc_iKeyPadding),
   mc_iTreeAnchorsLevel(bwtommi.mc_iTreeAnchorsLevel) {
   bwtommi.m_pnRoot.tn = nullptr;
   bwtommi.m_cValues = 0;
}

bitwise_trie_ordered_multimap_impl & bitwise_trie_ordered_multimap_impl::operator=(
   bitwise_trie_ordered_multimap_impl && bwtommi
) {
   // Assume that the subclass has already moved *this out.
   m_pnRoot = bwtommi.m_pnRoot;
   bwtommi.m_pnRoot.tn = nullptr;
   m_cValues = bwtommi.m_cValues;
   bwtommi.m_cValues = 0;
   return *this;
}

bitwise_trie_ordered_multimap_impl::list_node * bitwise_trie_ordered_multimap_impl::add(
   type_void_adapter const & typeValue, std::uintmax_t iKey, void const * pValue, bool bMove
) {
   ABC_TRACE_FUNC(this/*, typeValue*/, iKey, pValue, bMove);

   tree_node * ptnParent;
   unsigned iBitsPermutation;
   // Descend into the tree, creating nodes as necessary until the path for iKey is complete.
   {
      // ppnChildInParent points to *ptnParent’s parent’s pointer to *ptnParent.
      tree_or_list_node_ptr * ppnChildInParent = &m_pnRoot;
      std::uintmax_t iKeyRemaining = iKey << mc_iKeyPadding;
      unsigned iLevel = 0;
      do {
         ptnParent = ppnChildInParent->tn;
         if (!ptnParent) {
            ptnParent = (iLevel == mc_iTreeAnchorsLevel ? new anchor_node() : new tree_node());
            ppnChildInParent->tn = ptnParent;
         }
         iKeyRemaining = bitmanip::rotate_l(iKeyRemaining, smc_cBitsPerLevel);
         iBitsPermutation = static_cast<unsigned>(
            iKeyRemaining & (smc_cBitPermutationsPerLevel - 1)
         );
         ppnChildInParent = &ptnParent->m_apnChildren[iBitsPermutation];
      } while (++iLevel <= mc_iTreeAnchorsLevel);
   }
   // We got here, so *ptnParent is actually an anchor_node. Append a new node to its list.
   anchor_node_slot ans(static_cast<anchor_node *>(ptnParent), iBitsPermutation);
   list_node * pln = ans.push_back(typeValue, pValue, bMove);
   ++m_cValues;
   return pln;
}

void bitwise_trie_ordered_multimap_impl::clear(type_void_adapter const & typeValue) {
   ABC_TRACE_FUNC(this/*, typeValue*/);

   if (m_pnRoot.tn) {
      if (mc_iTreeAnchorsLevel == 0) {
         // *pnRoot is an anchor.
         destruct_anchor_node(typeValue, static_cast<anchor_node *>(m_pnRoot.tn));
      } else {
         destruct_tree_node(typeValue, m_pnRoot.tn, 0);
      }
      m_pnRoot = tree_or_list_node_ptr();
      m_cValues = 0;
   }
}

void bitwise_trie_ordered_multimap_impl::destruct_anchor_node(
   type_void_adapter const & typeValue, anchor_node * pan
) {
   ABC_TRACE_FUNC(this/*, typeValue*/, pan);

   unsigned iBitsPermutation = 0;
   do {
      if (list_node * pln = pan->m_apnChildren[iBitsPermutation].ln) {
         doubly_linked_list_impl::destruct_list(typeValue, pln);
      }
   } while (++iBitsPermutation < smc_cBitPermutationsPerLevel);
   delete pan;
}

void bitwise_trie_ordered_multimap_impl::destruct_tree_node(
   type_void_adapter const & typeValue, tree_node * ptn, unsigned iLevel
) {
   ABC_TRACE_FUNC(this/*, typeValue*/, ptn, iLevel);

   ++iLevel;
   unsigned iBitsPermutation = 0;
   do {
      if (tree_node * ptnChild = ptn->m_apnChildren[iBitsPermutation].tn) {
         if (iLevel == mc_iTreeAnchorsLevel) {
            destruct_anchor_node(typeValue, static_cast<anchor_node *>(ptnChild));
         } else {
            destruct_tree_node(typeValue, ptnChild, iLevel);
         }
      }
   } while (++iBitsPermutation < smc_cBitPermutationsPerLevel);
   delete ptn;
}

bitwise_trie_ordered_multimap_impl::list_node * bitwise_trie_ordered_multimap_impl::find(
   std::uintmax_t iKey
) const {
   ABC_TRACE_FUNC(this, iKey);

   if (auto ans = find_anchor_node_slot(iKey)) {
      return ans.first_child();
   } else {
      return nullptr;
   }
}

bitwise_trie_ordered_multimap_impl::anchor_node_slot
bitwise_trie_ordered_multimap_impl::find_anchor_node_slot(std::uintmax_t iKey) const {
   ABC_TRACE_FUNC(this, iKey);

   tree_node * ptnParent = m_pnRoot.tn;
   std::uintmax_t iKeyRemaining = iKey << mc_iKeyPadding;
   unsigned iLevel = 0;
   for (;;) {
      iKeyRemaining = bitmanip::rotate_l(iKeyRemaining, smc_cBitsPerLevel);
      unsigned iBitsPermutation = static_cast<unsigned>(
         iKeyRemaining & (smc_cBitPermutationsPerLevel - 1)
      );
      if (iLevel == mc_iTreeAnchorsLevel) {
         // At this level, *ptnParent is an anchor.
         return anchor_node_slot(static_cast<anchor_node *>(ptnParent), iBitsPermutation);
      } else if (!ptnParent) {
         return anchor_node_slot(nullptr, 0);
      }
      ptnParent = ptnParent->m_apnChildren[iBitsPermutation].tn;
      ++iLevel;
   }
}

bitwise_trie_ordered_multimap_impl::key_value_ptr
bitwise_trie_ordered_multimap_impl::find_first_key() const {
   ABC_TRACE_FUNC(this);

   tree_or_list_node_ptr pnChild;
   std::uintmax_t iKey = 0;

   // Descend into the tree.
   tree_node * ptnParent = m_pnRoot.tn;
   unsigned iLevel = 0;
   do {
      if (!ptnParent) {
         return key_value_ptr(0, nullptr);
      }
      // Look for the left-most branch to descend into.
      unsigned iChild = 0;
      do {
         pnChild = ptnParent->m_apnChildren[iChild];
         if (pnChild.tn) {
            // Prepend the selected bit permutation to the key.
            iKey <<= smc_cBitsPerLevel;
            iKey |= static_cast<std::uintmax_t>(iChild);
            break;
         }
      } while (++iChild < smc_cBitPermutationsPerLevel);
      ptnParent = pnChild.tn;
   } while (++iLevel <= mc_iTreeAnchorsLevel);

   // We got to the leaf level, so we can return pnChild.ln, though it might be nullptr.
   return key_value_ptr(iKey, pnChild.ln);
}

bitwise_trie_ordered_multimap_impl::key_value_ptr
bitwise_trie_ordered_multimap_impl::find_next_key(std::uintmax_t iPrevKey) const {
   ABC_TRACE_FUNC(this, iPrevKey);

   smvector<tree_node_slot, 64 /*>= mc_iTreeAnchorsLevel*/> vtnsPath;

   tree_node * ptnParent = m_pnRoot.tn;
   std::uintmax_t iKey = 0, iPrevKeyRemaining = iPrevKey << mc_iKeyPadding;
   unsigned iLevel = 0;
   for (;;) {
      iPrevKeyRemaining = bitmanip::rotate_l(iPrevKeyRemaining, smc_cBitsPerLevel);
      unsigned iBitsPermutation = static_cast<unsigned>(
         iPrevKeyRemaining & (smc_cBitPermutationsPerLevel - 1)
      );
      if (!ptnParent) {
         break;
      }
      vtnsPath.push_back(tree_node_slot(ptnParent, iBitsPermutation));
      // Copy the bits permutation from iPrevKey to iKey.
      iKey <<= smc_cBitsPerLevel;
      iKey |= static_cast<std::uintmax_t>(iBitsPermutation);
      if (iLevel == mc_iTreeAnchorsLevel) {
         break;
      }
      ptnParent = ptnParent->m_apnChildren[iBitsPermutation].tn;
      ++iLevel;
   }

   // This loop might pop levels from vtnsPath if they have no next sibling.
   while (vtnsPath) {
      ABC_TRACE_FUNC(this, iPrevKey, vtnsPath.size());

      // TODO: vector::back()
      if (auto tnsNextSibling = vtnsPath.rbegin()->next_used_sibling()) {
         // Replace the sibling and its bits permutation.
         iKey &= static_cast<std::uintmax_t>(smc_cBitPermutationsPerLevel - 1);
         iKey |= static_cast<std::uintmax_t>(tnsNextSibling.index());
         // If the path is not deep enough, descend the “first nexts” till the anchors level.
         for (
            iLevel = static_cast<unsigned>(vtnsPath.size()); iLevel < mc_iTreeAnchorsLevel; ++iLevel
         ) {
            tnsNextSibling = tnsNextSibling.first_used_child();
            iKey <<= smc_cBitsPerLevel;
            iKey |= static_cast<std::uintmax_t>(tnsNextSibling.index());
         }
         return key_value_ptr(iKey, tnsNextSibling.child().ln);
      }
      // This path level has no siblings to offer, try with the level above it.
      vtnsPath.pop_back();
      // Shift out the bits for the level we just dropped.
      iKey >>= smc_cBitsPerLevel;
   }
   // No next value to return.
   return key_value_ptr(0, nullptr);
}

void bitwise_trie_ordered_multimap_impl::prune_branch(std::uintmax_t iKey) {
   tree_node * ptn = m_pnRoot.tn, ** pptnTopmostNullable = &m_pnRoot.tn;
   tree_node * aptnAncestorsStack[smc_cBitPermutationsPerLevel];
   std::uintmax_t iKeyRemaining = iKey << mc_iKeyPadding;
   unsigned iLevel = 0;
   int iLastNonEmptyLevel = -1;
   do {
      iKeyRemaining = bitmanip::rotate_l(iKeyRemaining, smc_cBitsPerLevel);
      unsigned iBitsPermutation = static_cast<unsigned>(
         iKeyRemaining & (smc_cBitPermutationsPerLevel - 1)
      );
      // Check if the node has any children other than [iBitsPermutation].
      unsigned iChild = 0;
      do {
         if (iChild != iBitsPermutation && ptn->m_apnChildren[iChild].tn) {
            iLastNonEmptyLevel = static_cast<int>(iLevel);
            pptnTopmostNullable = &ptn->m_apnChildren[iBitsPermutation].tn;
            break;
         }
      } while (++iChild < smc_cBitPermutationsPerLevel);
      // Push this node on the ancestors stack.
      aptnAncestorsStack[iLevel] = ptn;
      ptn = ptn->m_apnChildren[iBitsPermutation].tn;
   } while (++iLevel <= mc_iTreeAnchorsLevel);

   // Now prune every empty level.
   while (static_cast<int>(--iLevel) > iLastNonEmptyLevel) {
      if (iLevel == mc_iTreeAnchorsLevel) {
         delete static_cast<anchor_node *>(aptnAncestorsStack[iLevel]);
      } else {
         delete aptnAncestorsStack[iLevel];
      }
   }
   // Make the last non-empty level no longer point to the branch we just pruned.
   *pptnTopmostNullable = nullptr;
}

void bitwise_trie_ordered_multimap_impl::remove_value(
   type_void_adapter const & typeValue, std::uintmax_t iKey, list_node * pln
) {
   ABC_TRACE_FUNC(this/*, typeValue*/, iKey, pln);

   if (pln->next() && pln->prev()) {
      // *pln is in the middle of its list, so we don’t need to find and update the anchor.
      doubly_linked_list_impl::remove(typeValue, nullptr, nullptr, pln);
   } else if (!pln->next() && !pln->prev()) {
      // *pln is in the only node in its list, so we can destruct it and prune the whole branch.
      typeValue.destruct(pln->value_ptr(typeValue));
      delete pln;
      prune_branch(iKey);
   } else {
      // *pln is the first or the last node in its list, so we need to update the anchor.
      if (anchor_node_slot ans = find_anchor_node_slot(iKey)) {
         ans.remove(typeValue, pln);
      } else {
         ABC_THROW(iterator_error, ());
      }
   }
   --m_cValues;
}

}}} //namespace abc::collections::detail
