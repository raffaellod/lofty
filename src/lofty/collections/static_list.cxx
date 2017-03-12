/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015, 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/collections.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections {

void static_list_impl_base::link_back(node * nd) {
   // TODO: enable use LOFTY_TRACE_FUNC(this, nd) by handling reentrancy.

   nd->set_siblings(nullptr, last);
   if (!first) {
      first = nd;
   } else if (last) {
      last->set_siblings(nd, last->get_other_sibling(nullptr));
   }
   last = nd;
}

void static_list_impl_base::link_front(node * nd) {
   // TODO: enable use LOFTY_TRACE_FUNC(this, nd) by handling reentrancy.

   nd->set_siblings(first, nullptr);
   if (!last) {
      last = nd;
   } else if (first) {
      first->set_siblings(first->get_other_sibling(nullptr), nd);
   }
   first = nd;
}

std::size_t static_list_impl_base::size() const {
   // TODO: enable use LOFTY_TRACE_FUNC(this) by handling reentrancy.

   std::size_t ret = 0;
   for (
      node * next, * prev = nullptr, * curr = first;
      curr;
      next = curr->get_other_sibling(prev), prev = curr, curr = next
   ) {
      ++ret;
   }
   return ret;
}

void static_list_impl_base::unlink(node * nd) {
   // TODO: enable use LOFTY_TRACE_FUNC(this, nd) by handling reentrancy.

   /* Find nd in the list, scanning from the back to the front of the list. If nodes are added by link_back()
   in their order of construction and the order of removal is the order of their destruction, last will be nd.
   This won’t be the case if shared libraries are not unloaded in the same order in which they are loaded. */
   for (node * prev, * next = nullptr, * curr = last; curr; next = curr, curr = prev) {
      prev = curr->get_other_sibling(next);
      if (curr == nd) {
         unlink(nd, prev, next);
         break;
      }
   }
}

void static_list_impl_base::unlink(node * nd, node * prev, node * next) {
   // TODO: enable use LOFTY_TRACE_FUNC(this, nd, prev, next) by handling reentrancy.

   if (prev) {
      prev->set_siblings(prev->get_other_sibling(nd), next);
   } else if (first == nd) {
      first = next;
   }
   if (next) {
      next->set_siblings(prev, next->get_other_sibling(nd));
   } else if (last == nd) {
      last = prev;
   }
}


void static_list_impl_base::iterator::increment() {
   // TODO: enable use LOFTY_TRACE_FUNC(this) by handling reentrancy.

   /* Detect attempts to increment past the end() of the container, or increment a default-constructed
   iterator. */
   validate();

   node const * prev = curr;
   curr = next;
   next = curr ? curr->get_other_sibling(prev) : nullptr;
}

void static_list_impl_base::iterator::validate() const {
   // TODO: enable use LOFTY_TRACE_FUNC(this) by handling reentrancy.

   if (!curr) {
      LOFTY_THROW(out_of_range, ());
   }
}

}} //namespace lofty::collections
