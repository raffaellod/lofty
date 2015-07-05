﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

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
#include <abaclade/collections/detail/trie_ordered_multimap_impl.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

scalar_keyed_trie_ordered_multimap_impl::scalar_keyed_trie_ordered_multimap_impl(
   scalar_keyed_trie_ordered_multimap_impl && sktommi
) :
   m_pnRoot(sktommi.m_pnRoot),
   m_cValues(sktommi.m_cValues),
   mc_iTreeAnchorsLevel(sktommi.mc_iTreeAnchorsLevel) {
   sktommi.m_pnRoot.tn = nullptr;
   sktommi.m_cValues = 0;
}

scalar_keyed_trie_ordered_multimap_impl & scalar_keyed_trie_ordered_multimap_impl::operator=(
   scalar_keyed_trie_ordered_multimap_impl && sktommi
) {
   // Assume that the subclass has already moved *this out.
   m_pnRoot = sktommi.m_pnRoot;
   sktommi.m_pnRoot.tn = nullptr;
   m_cValues = sktommi.m_cValues;
   sktommi.m_cValues = 0;
   return *this;
}

scalar_keyed_trie_ordered_multimap_impl::list_node * scalar_keyed_trie_ordered_multimap_impl::add(
   type_void_adapter const & typeValue, std::uintmax_t iKey, void const * pValue, bool bMove
) {
   ABC_TRACE_FUNC(this/*, typeValue*/, iKey, pValue, bMove);

   tree_node * ptnParent;
   unsigned iBitsPermutation;
   // Descend into the tree, creating nodes as necessary until the path for iKey is complete.
   {
      // ppnChildInParent points to *ptnParent’s parent’s pointer to *ptnParent.
      tree_or_list_node_ptr * ppnChildInParent = &m_pnRoot;
      std::uintmax_t iKeyRemaining = iKey;
      std::size_t iLevel = 0;
      do {
         ptnParent = ppnChildInParent->tn;
         if (!ptnParent) {
            ptnParent = (iLevel == mc_iTreeAnchorsLevel ? new anchor_node() : new tree_node());
            ppnChildInParent->tn = ptnParent;
         }
         iBitsPermutation = static_cast<unsigned>(
            iKeyRemaining & (smc_cBitPermutationsPerLevel - 1)
         );
         iKeyRemaining >>= smc_cBitsPerLevel;
         ppnChildInParent = &ptnParent->m_apnChildren[iBitsPermutation];
      } while (++iLevel <= mc_iTreeAnchorsLevel);
   }
   // We got here, so *ptnParent is actually an anchor_node. Append a new node to its list.
   anchor_node_slot ans(static_cast<anchor_node *>(ptnParent), iBitsPermutation);
   list_node * pln = ans.push_back(typeValue, pValue, bMove);
   ++m_cValues;
   return pln;
}

void scalar_keyed_trie_ordered_multimap_impl::clear(type_void_adapter const & typeValue) {
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

void scalar_keyed_trie_ordered_multimap_impl::destruct_anchor_node(
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

void scalar_keyed_trie_ordered_multimap_impl::destruct_tree_node(
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

scalar_keyed_trie_ordered_multimap_impl::list_node * scalar_keyed_trie_ordered_multimap_impl::find(
   std::uintmax_t iKey
) {
   ABC_TRACE_FUNC(this, iKey);

   if (anchor_node_slot ans = find_anchor_node_slot(iKey)) {
      return ans.first_child();
   } else {
      return nullptr;
   }
}

scalar_keyed_trie_ordered_multimap_impl::anchor_node_slot
scalar_keyed_trie_ordered_multimap_impl::find_anchor_node_slot(std::uintmax_t iKey) const {
   ABC_TRACE_FUNC(this, iKey);

   std::uintmax_t iKeyRemaining = iKey;
   tree_node * ptnParent = m_pnRoot.tn;
   std::size_t iLevel = 0;
   for (;;) {
      unsigned iBitsPermutation = static_cast<unsigned>(
         iKeyRemaining & (smc_cBitPermutationsPerLevel - 1)
      );
      if (iLevel == mc_iTreeAnchorsLevel) {
         // At this level, *ptnParent is an anchor.
         return anchor_node_slot(static_cast<anchor_node *>(ptnParent), iBitsPermutation);
      } else if (!ptnParent) {
         return anchor_node_slot(nullptr, 0);
      }
      iKeyRemaining >>= smc_cBitsPerLevel;
      ptnParent = ptnParent->m_apnChildren[iBitsPermutation].tn;
      ++iLevel;
   }

}

scalar_keyed_trie_ordered_multimap_impl::key_value_ptr
scalar_keyed_trie_ordered_multimap_impl::front() {
   ABC_TRACE_FUNC(this);

   tree_or_list_node_ptr pnChild;
   std::uintmax_t iKey = 0;

   // Descend into the tree.
   tree_node * ptnParent = m_pnRoot.tn;
   std::size_t iLevel = 0;
   unsigned cNextLevelBitsShift = 0;
   do {
      if (!ptnParent) {
         return key_value_ptr(0, nullptr);
      }
      // Look for the left-most branch to descend into.
      unsigned iBitsPermutation = 0;
      do {
         pnChild = ptnParent->m_apnChildren[iBitsPermutation];
         if (pnChild.tn) {
            // Prepend the selected bit permutation to the key.
            iKey |= static_cast<std::uintmax_t>(iBitsPermutation) << cNextLevelBitsShift;
            cNextLevelBitsShift += smc_cBitsPerLevel;
            break;
         }
      } while (++iBitsPermutation < smc_cBitPermutationsPerLevel);
      ptnParent = pnChild.tn;
   } while (++iLevel <= mc_iTreeAnchorsLevel);

   // We got to the leaf level, so we can return pnChild.ln, though it might be nullptr.
   return key_value_ptr(iKey, pnChild.ln);
}

void scalar_keyed_trie_ordered_multimap_impl::remove_value(
   type_void_adapter const & typeValue, std::uintmax_t iKey, list_node * pln
) {
   ABC_TRACE_FUNC(this/*, typeValue*/, iKey, pln);

   if (pln->next() && pln->prev()) {
      // *pln is in the middle of its list, so we don’t need to find and update the anchor.
      doubly_linked_list_impl::remove(typeValue, nullptr, nullptr, pln);
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
