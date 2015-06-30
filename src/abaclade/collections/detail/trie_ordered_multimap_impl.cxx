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
#include <abaclade/collections/detail/trie_ordered_multimap_impl.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections { namespace detail {

scalar_keyed_trie_ordered_multimap_impl::scalar_keyed_trie_ordered_multimap_impl(
   scalar_keyed_trie_ordered_multimap_impl && sktommi
) :
   m_pRoot(sktommi.m_pRoot),
   m_cValues(sktommi.m_cValues),
   mc_iTreeAnchorsLevel(sktommi.mc_iTreeAnchorsLevel) {
   sktommi.m_pRoot = nullptr;
   sktommi.m_cValues = 0;
}

scalar_keyed_trie_ordered_multimap_impl & scalar_keyed_trie_ordered_multimap_impl::operator=(
   scalar_keyed_trie_ordered_multimap_impl && sktommi
) {
   // Assume that the subclass has already moved *this out.
   m_pRoot = sktommi.m_pRoot;
   sktommi.m_pRoot = nullptr;
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
      // ppChildInParent points to *ptnParent’s parent’s pointer to *ptnParent.
      void ** ppChildInParent = &m_pRoot;
      std::uintmax_t iKeyRemaining = iKey;
      std::size_t iLevel = 0;
      do {
         ptnParent = static_cast<tree_node *>(*ppChildInParent);
         if (!ptnParent) {
            ptnParent = (iLevel == mc_iTreeAnchorsLevel ? new anchor_node() : new tree_node());
            *ppChildInParent = ptnParent;
         }
         iBitsPermutation = static_cast<unsigned>(
            iKeyRemaining & (smc_cBitPermutationsPerLevel - 1)
         );
         iKeyRemaining >>= smc_cBitsPerLevel;
         ppChildInParent = &ptnParent->m_apChildren[iBitsPermutation];
      } while (++iLevel <= mc_iTreeAnchorsLevel);
   }
   // We got here, so *ptnParent is actually an anchor_node.
   anchor_node_slot ans(static_cast<anchor_node *>(ptnParent), iBitsPermutation);

   list_node * plnPrev = ans.get_last_child();
   std::unique_ptr<list_node> pln(new(typeValue) list_node(nullptr, plnPrev));
   // Append the node to the values list.
   ans.set_last_child(pln.get());
   if (!ans.get_first_child()) {
      ans.set_first_child(pln.get());
   }
   // Construct the node’s value.
   void * pDst = pln->value_ptr(typeValue);
   if (bMove) {
      typeValue.move_construct(pDst, const_cast<void *>(pValue));
   } else {
      typeValue.copy_construct(pDst, pValue);
   }
   ++m_cValues;
   // Transfer ownership of *pln to the list.
   return pln.release();
}

void scalar_keyed_trie_ordered_multimap_impl::clear(type_void_adapter const & typeValue) {
   ABC_TRACE_FUNC(this/*, typeValue*/);

   if (m_pRoot) {
      destruct_tree_node(typeValue, static_cast<tree_node *>(m_pRoot), 0);
      m_pRoot = nullptr;
      m_cValues = 0;
   }
}

void scalar_keyed_trie_ordered_multimap_impl::destruct_anchor_node(
   type_void_adapter const & typeValue, anchor_node * pan
) {
   ABC_TRACE_FUNC(this/*, typeValue*/, pan);

   unsigned iBitsPermutation = 0;
   do {
      if (list_node * pln = static_cast<list_node *>(pan->m_apChildren[iBitsPermutation])) {
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
      if (tree_node * ptnChild = static_cast<tree_node *>(ptn->m_apChildren[iBitsPermutation])) {
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
      return ans.get_first_child();
   } else {
      return nullptr;
   }
}

scalar_keyed_trie_ordered_multimap_impl::anchor_node_slot
scalar_keyed_trie_ordered_multimap_impl::find_anchor_node_slot(std::uintmax_t iKey) const {
   ABC_TRACE_FUNC(this, iKey);

   std::uintmax_t iKeyRemaining = iKey;
   tree_node * ptnParent = static_cast<tree_node *>(m_pRoot);
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
      ptnParent = static_cast<tree_node *>(ptnParent->m_apChildren[iBitsPermutation]);
      ++iLevel;
   }

}

scalar_keyed_trie_ordered_multimap_impl::key_value_ptr
scalar_keyed_trie_ordered_multimap_impl::front() {
   ABC_TRACE_FUNC(this);

   void * pChild;
   std::uintmax_t iKey = 0;

   // Descend into the tree.
   tree_node * ptnParent = static_cast<tree_node *>(m_pRoot);
   std::size_t iLevel = 0;
   unsigned cNextLevelBitsShift = 0;
   do {
      if (!ptnParent) {
         return key_value_ptr(0, nullptr);
      }
      // Look for the left-most branch to descend into.
      unsigned iBitsPermutation = 0;
      do {
         pChild = ptnParent->m_apChildren[iBitsPermutation];
         if (pChild) {
            // Prepend the selected bit permutation to the key.
            iKey |= static_cast<std::uintmax_t>(iBitsPermutation) << cNextLevelBitsShift;
            cNextLevelBitsShift += smc_cBitsPerLevel;
            break;
         }
      } while (++iBitsPermutation < smc_cBitPermutationsPerLevel);
      ptnParent = static_cast<tree_node *>(pChild);
   } while (++iLevel <= mc_iTreeAnchorsLevel);

   // We got here, so *pChild is actually a value list’s first node, though pChild might be nullptr.
   return key_value_ptr(iKey, static_cast<list_node *>(pChild));
}

void scalar_keyed_trie_ordered_multimap_impl::remove_value(
   type_void_adapter const & typeValue, std::uintmax_t iKey, list_node * pln
) {
   ABC_TRACE_FUNC(this/*, typeValue*/, iKey, pln);

   if (!pln->next() || !pln->prev()) {
      // *pln is the first or the last node in its list, so we need to update the anchor.
      if (anchor_node_slot ans = find_anchor_node_slot(iKey)) {
         if (!pln->next()) {
            ans.set_last_child(pln->prev());
         }
         if (!pln->prev()) {
            ans.set_first_child(pln->next());
         }
      } else {
         // TODO: throw invalid_iterator.
         ABC_THROW(generic_error, ());
      }
   }
   typeValue.destruct(pln->value_ptr(typeValue));
   delete pln;
   --m_cValues;
}

}}} //namespace abc::collections::detail
