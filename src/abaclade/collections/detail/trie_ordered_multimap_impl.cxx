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

void scalar_keyed_trie_ordered_multimap_impl::list_node::unlink_and_destruct(
   type_void_adapter const & type
) const {
   if (m_plnNext) {
      m_plnNext->m_plnPrev = m_plnPrev;
   }
   if (m_plnPrev) {
      m_plnPrev->m_plnNext = m_plnNext;
   }
   type.destruct(value_ptr(type));
   memory::_raw_free(this);
}

void * scalar_keyed_trie_ordered_multimap_impl::list_node::value_ptr(
   type_void_adapter const & type
) const {
   return type.align_pointer(&m_plnPrev + 1);
}


scalar_keyed_trie_ordered_multimap_impl::scalar_keyed_trie_ordered_multimap_impl(
   scalar_keyed_trie_ordered_multimap_impl && tommi
) :
   m_pnRoot(tommi.m_pnRoot),
   m_cValues(tommi.m_cValues),
   mc_iTreeAnchorsLevel(tommi.mc_iTreeAnchorsLevel) {
   tommi.m_pnRoot = nullptr;
   tommi.m_cValues = 0;
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
      node ** ppnChildInParent = &m_pnRoot;
      std::uintmax_t iKeyRemaining = iKey;
      std::size_t iLevel = 0;
      do {
         ptnParent = static_cast<tree_node *>(*ppnChildInParent);
         if (!ptnParent) {
            ptnParent = (iLevel == mc_iTreeAnchorsLevel ? new anchor_node() : new tree_node());
            *ppnChildInParent = ptnParent;
         }
         iBitsPermutation = static_cast<unsigned>(
            iKeyRemaining & (smc_cBitPermutationsPerLevel - 1)
         );
         iKeyRemaining >>= smc_cBitsPerLevel;
         ppnChildInParent = &ptnParent->m_apnChildren[iBitsPermutation];
      } while (++iLevel <= mc_iTreeAnchorsLevel);
   }

   // We got here, so *ptnParent is actually an anchor_node.
   anchor_node * panParent = static_cast<anchor_node *>(ptnParent);
   /* To calculate the node size, add typeValue.size() bytes to the offset of the value in a node at
   address 0. This allows packing the node optimally even if the unpadded node size is e.g. 6
   (sizeof will return 8 for that) and typeValue.size() is 2, giving 8 instead of 10 (which would
   really mean at least 12 bytes, a 50% waste of memory). */
   std::unique_ptr<list_node, memory::freeing_deleter> pln(static_cast<list_node *>(
      memory::_raw_alloc(reinterpret_cast<std::size_t>(
         static_cast<list_node *>(0)->value_ptr(typeValue)
      ) + typeValue.size())
   ));
   void * pDst = pln->value_ptr(typeValue);
   if (bMove) {
      typeValue.move_construct(pDst, const_cast<void *>(pValue));
   } else {
      typeValue.copy_construct(pDst, pValue);
   }
   list_node * plnRet = pln.get(), * plnPrev = panParent->m_aplnChildrenLasts[iBitsPermutation];
   pln->m_plnPrev = plnPrev;
   pln->m_plnNext = nullptr;
   panParent->m_aplnChildrenLasts[iBitsPermutation] = plnRet;
   if (!panParent->m_apnChildren[iBitsPermutation]) {
      panParent->m_apnChildren[iBitsPermutation] = plnRet;
   }
   if (plnPrev) {
      plnPrev->m_plnNext = plnRet;
   }
   // Transfer ownership of the list_node to the list.
   pln.release();
   ++m_cValues;
   return plnRet;
}

void scalar_keyed_trie_ordered_multimap_impl::clear(type_void_adapter const & typeValue) {
   ABC_TRACE_FUNC(this/*, typeValue*/);

   if (m_pnRoot) {
      destruct_tree_node(typeValue, static_cast<tree_node *>(m_pnRoot), 0);
      m_pnRoot = nullptr;
      m_cValues = 0;
   }
}

scalar_keyed_trie_ordered_multimap_impl::indexed_anchor
scalar_keyed_trie_ordered_multimap_impl::descend_to_anchor(std::uintmax_t iKey) const {
   ABC_TRACE_FUNC(this, iKey);

   std::uintmax_t iKeyRemaining = iKey;
   tree_node * ptnParent = static_cast<tree_node *>(m_pnRoot);
   std::size_t iLevel = 0;
   for (;;) {
      unsigned iBitsPermutation = static_cast<unsigned>(
         iKeyRemaining & (smc_cBitPermutationsPerLevel - 1)
      );
      if (iLevel == mc_iTreeAnchorsLevel) {
         // At this level, *ptnParent is an anchor.
         return indexed_anchor(static_cast<anchor_node *>(ptnParent), iBitsPermutation);
      } else if (!ptnParent) {
         return indexed_anchor(nullptr, 0);
      }
      iKeyRemaining >>= smc_cBitsPerLevel;
      ptnParent = static_cast<tree_node *>(ptnParent->m_apnChildren[iBitsPermutation]);
      ++iLevel;
   }

}

void scalar_keyed_trie_ordered_multimap_impl::destruct_list(
   type_void_adapter const & type, list_node * pln
) {
   ABC_TRACE_FUNC(this/*, type*/, pln);

   while (pln) {
      list_node * plnNext = pln->m_plnNext;
      type.destruct(pln->value_ptr(type));
      memory::_raw_free(pln);
      pln = plnNext;
   }
}

void scalar_keyed_trie_ordered_multimap_impl::destruct_anchor_node(
   type_void_adapter const & typeValue, anchor_node * pan
) {
   ABC_TRACE_FUNC(this/*, typeValue*/, pan);

   unsigned iBitsPermutation = 0;
   do {
      if (list_node * pln = static_cast<list_node *>(pan->m_apnChildren[iBitsPermutation])) {
         destruct_list(typeValue, pln);
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
      if (node * pn = ptn->m_apnChildren[iBitsPermutation]) {
         if (iLevel == mc_iTreeAnchorsLevel) {
            destruct_anchor_node(typeValue, static_cast<anchor_node *>(pn));
         } else {
            destruct_tree_node(typeValue, static_cast<tree_node *>(pn), iLevel);
         }
      }
   } while (++iBitsPermutation < smc_cBitPermutationsPerLevel);
   delete ptn;
}

scalar_keyed_trie_ordered_multimap_impl::list_node * scalar_keyed_trie_ordered_multimap_impl::find(
   std::uintmax_t iKey
) {
   ABC_TRACE_FUNC(this, iKey);

   indexed_anchor ia(descend_to_anchor(iKey));
   return ia.pan ? static_cast<list_node *>(ia.pan->m_apnChildren[ia.iBitsPermutation]) : nullptr;
}

scalar_keyed_trie_ordered_multimap_impl::key_value_pair
scalar_keyed_trie_ordered_multimap_impl::front() {
   ABC_TRACE_FUNC(this);

   node * pnChild;
   std::uintmax_t iKey = 0;

   // Descend into the tree.
   tree_node * ptnParent = static_cast<tree_node *>(m_pnRoot);
   std::size_t iLevel = 0;
   unsigned cNextLevelBitsShift = 0;
   do {
      if (!ptnParent) {
         return key_value_pair(0, nullptr);
      }
      // Look for the left-most branch to descend into.
      unsigned iBitsPermutation = 0;
      do {
         pnChild = ptnParent->m_apnChildren[iBitsPermutation];
         if (pnChild) {
            // Prepend the selected bit permutation to the key.
            iKey |= static_cast<std::uintmax_t>(iBitsPermutation) << cNextLevelBitsShift;
            cNextLevelBitsShift += smc_cBitsPerLevel;
            break;
         }
      } while (++iBitsPermutation < smc_cBitPermutationsPerLevel);
      ptnParent = static_cast<tree_node *>(pnChild);
   } while (++iLevel <= mc_iTreeAnchorsLevel);

   /* We got here, so *pnChild is actually a value list’s first node, though pnChild might be
   nullptr. */
   return key_value_pair(iKey, static_cast<list_node *>(pnChild));
}

void scalar_keyed_trie_ordered_multimap_impl::remove_value(
   type_void_adapter const & typeValue, std::uintmax_t iKey, list_node * pln
) {
   ABC_TRACE_FUNC(this/*, typeValue*/, iKey, pln);

   if (!pln->m_plnNext || !pln->m_plnPrev) {
      // *pln is the first or the last node in its list, so we need to update the anchor.
      indexed_anchor ia(descend_to_anchor(iKey));
      if (!ia.pan) {
         // TODO: throw invalid_iterator.
         ABC_THROW(generic_error, ());
      }
      if (!pln->m_plnNext) {
         ia.pan->m_aplnChildrenLasts[ia.iBitsPermutation] = nullptr;
      }
      if (!pln->m_plnPrev) {
         ia.pan->m_apnChildren[ia.iBitsPermutation] = nullptr;
      }
   }
   pln->unlink_and_destruct(typeValue);
   --m_cValues;
}

}}} //namespace abc::collections::detail
