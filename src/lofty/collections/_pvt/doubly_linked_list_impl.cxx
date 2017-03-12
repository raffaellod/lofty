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
#include <lofty/collections.hxx>
#include <lofty/collections/_pvt/doubly_linked_list_impl.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

void * doubly_linked_list_impl::node::operator new(std::size_t alloc_size, type_void_adapter const & type) {
   LOFTY_UNUSED_ARG(alloc_size);
   /* To calculate the node size, pack the value against the end of the node, potentially using
   space that alloc_size (== sizeof(node)) would reserve as padding. */
   return memory::alloc_bytes(type.align_offset(LOFTY_UNPADDED_SIZEOF(node, prev_)) + type.size());
}

doubly_linked_list_impl::node::node(
   type_void_adapter const & type, node ** first_node, node ** last_node, node * prev__, node * next__,
   void const * value_src, bool move
) :
   next_(next__),
   prev_(prev__) {
   LOFTY_TRACE_FUNC(this/*, type*/, first_node, last_node, prev__, next__, value_src, move);

   // Copy- or move-onstruct the value of the node.
   void * value_dst = value_ptr(type);
   if (move) {
      type.move_construct(value_dst, const_cast<void *>(value_src));
   } else {
      type.copy_construct(value_dst, value_src);
   }
   // If no exceptions were thrown, link the node into the list.
   if (prev_) {
      prev_->next_ = this;
   } else {
      *first_node = this;
   }
   if (next_) {
      next_->prev_ = this;
   } else {
      *last_node = this;
   }
}

void doubly_linked_list_impl::node::unlink(node ** first_node, node ** last_node) {
   if (prev_) {
      prev_->next_ = next_;
   } else if (first_node) {
      *first_node = next_;
   }
   if (next_) {
      next_->prev_ = prev_;
   } else if (last_node) {
      *last_node = prev_;
   }
}

void * doubly_linked_list_impl::node::value_ptr(type_void_adapter const & type) const {
   // Make sure that the argument is the address following the last member.
   return type.align_pointer(&prev_ + 1);
}


void doubly_linked_list_impl::iterator_base::advance(bool forward) {
   LOFTY_TRACE_FUNC(this, forward);

   validate();
   nd = forward ? nd->next() : nd->prev();
}

void doubly_linked_list_impl::iterator_base::validate() const {
   if (!nd) {
      LOFTY_THROW(out_of_range, ());
   }
}


doubly_linked_list_impl::doubly_linked_list_impl() :
   first_node(nullptr),
   last_node(nullptr),
   size_(0) {
}
doubly_linked_list_impl::doubly_linked_list_impl(doubly_linked_list_impl && src) :
   first_node(src.first_node),
   last_node(src.last_node),
   size_(src.size_) {
   src.size_ = 0;
   src.first_node = nullptr;
   src.last_node = nullptr;
}

doubly_linked_list_impl & doubly_linked_list_impl::operator=(doubly_linked_list_impl && src) {
   LOFTY_TRACE_FUNC(this);

   // Assume that the subclass has already moved *this out.
   first_node = src.first_node;
   src.first_node = nullptr;
   last_node = src.last_node;
   src.last_node = nullptr;
   size_ = src.size_;
   src.size_ = 0;
   return *this;
}

doubly_linked_list_impl::node * doubly_linked_list_impl::back() const {
   if (!last_node) {
      LOFTY_THROW(bad_access, ());
   }
   return last_node;
}

void doubly_linked_list_impl::clear(type_void_adapter const & type) {
   LOFTY_TRACE_FUNC(this/*, type*/);

   destruct_list(type, first_node);
   first_node = nullptr;
   last_node = nullptr;
   size_ = 0;
}

/*static*/ void doubly_linked_list_impl::destruct_list(type_void_adapter const & type, node * nd) {
   LOFTY_TRACE_FUNC(/*type, */nd);

   while (nd) {
      node * next = nd->next();
      type.destruct(nd->value_ptr(type));
      delete nd;
      nd = next;
   }
}

doubly_linked_list_impl::node * doubly_linked_list_impl::front() const {
   if (!first_node) {
      LOFTY_THROW(bad_access, ());
   }
   return first_node;
}

/*static*/ doubly_linked_list_impl::node * doubly_linked_list_impl::push_back(
   type_void_adapter const & type, node ** first_node, node ** last_node, void const * value, bool move
) {
   LOFTY_TRACE_FUNC(/*type, */first_node, last_node, value, move);

   return new(type) node(type, first_node, last_node, *last_node, nullptr, value, move);
}

doubly_linked_list_impl::node * doubly_linked_list_impl::push_back(
   type_void_adapter const & type, void const * value, bool move
) {
   node * ret = push_back(type, &first_node, &last_node, value, move);
   ++size_;
   return ret;
}

/*static*/ doubly_linked_list_impl::node * doubly_linked_list_impl::push_front(
   type_void_adapter const & type, node ** first_node, node ** last_node, void const * value, bool move
) {
   LOFTY_TRACE_FUNC(/*type, */first_node, last_node, value, move);

   return new(type) node(type, first_node, last_node, nullptr, *first_node, value, move);
}

doubly_linked_list_impl::node * doubly_linked_list_impl::push_front(
   type_void_adapter const & type, void const * value, bool move
) {
   node * ret = push_front(type, &first_node, &last_node, value, move);
   ++size_;
   return ret;
}

/*static*/ void doubly_linked_list_impl::remove(
   type_void_adapter const & type, node ** first_node, node ** last_node, node * nd
) {
   LOFTY_TRACE_FUNC(/*type, */first_node, last_node, nd);

   nd->unlink(first_node, last_node);
   type.destruct(nd->value_ptr(type));
   delete nd;
}

void doubly_linked_list_impl::remove(type_void_adapter const & type, node * nd) {
   remove(type, &first_node, &last_node, nd);
   --size_;
}

}}} //namespace lofty::collections::_pvt
