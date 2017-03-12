/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

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
#include <lofty/collections/_pvt/singly_linked_list_impl.hxx>
#include <lofty/type_void_adapter.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

void * singly_linked_list_impl::node::operator new(std::size_t alloc_size, type_void_adapter const & type) {
   LOFTY_UNUSED_ARG(alloc_size);
   /* To calculate the node size, pack the value against the end of the node, potentially using space that
   alloc_size (== sizeof(node)) would reserve as padding. */
   return memory::alloc_bytes(type.align_offset(LOFTY_UNPADDED_SIZEOF(node, next_)) + type.size());
}

singly_linked_list_impl::node::node(
   type_void_adapter const & type, node ** first_node, node ** last_node, node * prev, node * next__,
   void const * value_src, bool move
) :
   next_(next__) {
   LOFTY_TRACE_FUNC(this/*, type*/, first_node, last_node, prev, next__, value_src, move);

   // Copy- or move-onstruct the value of the node.
   void * value_dst = value_ptr(type);
   if (move) {
      type.move_construct(value_dst, const_cast<void *>(value_src));
   } else {
      type.copy_construct(value_dst, value_src);
   }
   // If no exceptions were thrown, link the node into the list.
   if (prev) {
      prev->next_ = this;
   } else {
      *first_node = this;
   }
   if (!next__) {
      *last_node = this;
   }
}

void singly_linked_list_impl::node::unlink(node ** first_node, node ** last_node, node * prev) {
   if (prev) {
      prev->next_ = next_;
   } else if (first_node) {
      *first_node = next_;
   }
   if (!next_ && last_node) {
      *last_node = prev;
   }
}

void * singly_linked_list_impl::node::value_ptr(type_void_adapter const & type) const {
   // Make sure that the argument is the address following the last member.
   return type.align_pointer(&next_ + 1);
}


singly_linked_list_impl::singly_linked_list_impl(singly_linked_list_impl && src) :
   first_node(src.first_node),
   last_node(src.last_node),
   size_(src.size_) {
   src.size_ = 0;
   src.first_node = nullptr;
   src.last_node = nullptr;
}

singly_linked_list_impl & singly_linked_list_impl::operator=(singly_linked_list_impl && src) {
   /* Assume that the subclass has already made a copy of first_node to be able to release the old node list
   after calling this operator. */
   first_node = src.first_node;
   src.first_node = nullptr;
   last_node = src.last_node;
   src.last_node = nullptr;
   size_ = src.size_;
   src.size_ = 0;
   return *this;
}

void singly_linked_list_impl::clear(type_void_adapter const & type) {
   destruct_list(type, first_node);
   first_node = nullptr;
   last_node = nullptr;
   size_ = 0;
}

/*static*/ void singly_linked_list_impl::destruct_list(type_void_adapter const & type, node * nd) {
   LOFTY_TRACE_FUNC(/*type, */nd);

   while (nd) {
      node * next = nd->next();
      type.destruct(nd->value_ptr(type));
      delete nd;
      nd = next;
   }
}

singly_linked_list_impl::node * singly_linked_list_impl::push_back(
   type_void_adapter const & type, void const * value, bool move
) {
   LOFTY_TRACE_FUNC(this/*, type*/, value, move);

   node * ret = new(type) node(type, &first_node, &last_node, last_node, nullptr, value, move);
   ++size_;
   return ret;
}

void singly_linked_list_impl::pop_front(type_void_adapter const & type) {
   LOFTY_TRACE_FUNC(this/*, type*/);

   node * nd = first_node;
   nd->unlink(&first_node, &last_node, nullptr);
   type.destruct(nd->value_ptr(type));
   --size_;
   delete nd;
}

}}} //namespace lofty::collections::_pvt
