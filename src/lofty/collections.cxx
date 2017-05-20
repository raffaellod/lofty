/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/collections.hxx>
#include <lofty/collections/_pvt/doubly_linked_list_impl.hxx>
#include <lofty/collections/_pvt/singly_linked_list_impl.hxx>
#include <lofty/collections/vector.hxx>
#include <lofty/type_void_adapter.hxx>
#include <lofty/text/parsers/dynamic.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections {

/*explicit*/ bad_access::bad_access(errint_t err_ /*= 0*/) :
   generic_error(err_) {
}

bad_access::bad_access(bad_access const & src) :
   generic_error(src) {
}

/*virtual*/ bad_access::~bad_access() LOFTY_STL_NOEXCEPT_TRUE() {
}

bad_access & bad_access::operator=(bad_access const & src) {
   generic_error::operator=(src);
   return *this;
}

}} //namespace lofty::collections

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections {

/*explicit*/ bad_key::bad_key(errint_t err_ /*= 0*/) :
   bad_access(err_) {
}

bad_key::bad_key(bad_key const & src) :
   bad_access(src) {
}

/*virtual*/ bad_key::~bad_key() LOFTY_STL_NOEXCEPT_TRUE() {
}

bad_key & bad_key::operator=(bad_key const & src) {
   bad_access::operator=(src);
   return *this;
}

}} //namespace lofty::collections

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections {

/*explicit*/ out_of_range::out_of_range(errint_t err_ /*= 0*/) :
   bad_access(err_) {
}

out_of_range::out_of_range(
   std::ptrdiff_t invalid, std::ptrdiff_t min, std::ptrdiff_t max, errint_t err_ /*= 0*/
) :
   bad_access(err_) {
   what_ostream().print(LOFTY_SL("invalid value={} valid range=[{}, {}]"), invalid, min, max);
}

out_of_range::out_of_range(void const * invalid, void const * min, void const * max, errint_t err_ /*= 0*/) :
   bad_access(err_) {
   what_ostream().print(LOFTY_SL("invalid value={} valid range=[{}, {}]"), invalid, min, max);
}

out_of_range::out_of_range(out_of_range const & src) :
   bad_access(src) {
}

/*virtual*/ out_of_range::~out_of_range() LOFTY_STL_NOEXCEPT_TRUE() {
}

out_of_range & out_of_range::operator=(out_of_range const & src) {
   bad_access::operator=(src);
   return *this;
}

}} //namespace lofty::collections

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

void * doubly_linked_list_impl::node::operator new(std::size_t alloc_size, type_void_adapter const & type) {
   LOFTY_UNUSED_ARG(alloc_size);
   /* To calculate the node size, pack the value against the end of the node, potentially using
   space that alloc_size (== sizeof(node)) would reserve as padding. */
   return memory::alloc_bytes(type.align_offset(LOFTY_UNPADDED_SIZEOF(node, prev_)) + type.size());
}

doubly_linked_list_impl::node::node(
   type_void_adapter const & type, node ** first_node_, node ** last_node_, node * prev__, node * next__,
   void const * value_src, bool move
) :
   next_(next__),
   prev_(prev__) {
   LOFTY_TRACE_FUNC(this/*, type*/, first_node_, last_node_, prev__, next__, value_src, move);

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
      *first_node_ = this;
   }
   if (next_) {
      next_->prev_ = this;
   } else {
      *last_node_ = this;
   }
}

void doubly_linked_list_impl::node::unlink(node ** first_node_, node ** last_node_) {
   if (prev_) {
      prev_->next_ = next_;
   } else if (first_node_) {
      *first_node_ = next_;
   }
   if (next_) {
      next_->prev_ = prev_;
   } else if (last_node_) {
      *last_node_ = prev_;
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

void * singly_linked_list_impl::node::operator new(std::size_t alloc_size, type_void_adapter const & type) {
   LOFTY_UNUSED_ARG(alloc_size);
   /* To calculate the node size, pack the value against the end of the node, potentially using space that
   alloc_size (== sizeof(node)) would reserve as padding. */
   return memory::alloc_bytes(type.align_offset(LOFTY_UNPADDED_SIZEOF(node, next_)) + type.size());
}

singly_linked_list_impl::node::node(
   type_void_adapter const & type, node ** first_node_, node ** last_node_, node * prev, node * next__,
   void const * value_src, bool move
) :
   next_(next__) {
   LOFTY_TRACE_FUNC(this/*, type*/, first_node_, last_node_, prev, next__, value_src, move);

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
      *first_node_ = this;
   }
   if (!next__) {
      *last_node_ = this;
   }
}

void singly_linked_list_impl::node::unlink(node ** first_node_, node ** last_node_, node * prev) {
   if (prev) {
      prev->next_ = next_;
   } else if (first_node_) {
      *first_node_ = next_;
   }
   if (!next_ && last_node_) {
      *last_node_ = prev;
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

vector_from_text_istream::vector_from_text_istream() :
   lofty::_pvt::sequence_from_text_istream(LOFTY_SL("\\{"), LOFTY_SL("\\}")) {
}

vector_from_text_istream::~vector_from_text_istream() {
}

}}} //namespace lofty::collections::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

vector_to_text_ostream::vector_to_text_ostream() :
   lofty::_pvt::sequence_to_text_ostream(LOFTY_SL("{"), LOFTY_SL("}")) {
}

vector_to_text_ostream::~vector_to_text_ostream() {
}

str vector_to_text_ostream::set_format(str const & format) {
   LOFTY_TRACE_FUNC(this, format);

   auto itr(format.cbegin());

   // Add parsing of the format string here.
   // TODO: parse format and store the appropriate element format in elt_format.
   str elt_format;

   throw_on_unused_streaming_format_chars(itr, format);

   return _std::move(elt_format);
}

}}} //namespace lofty::collections::_pvt
