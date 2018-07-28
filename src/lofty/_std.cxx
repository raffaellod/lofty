﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/_std/exception.hxx>
#include <lofty/_std/memory.hxx>
#include <lofty/_std/new.hxx>
#include <lofty/_std/tuple.hxx>
#include <lofty/_std/typeinfo.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_STL_LOFTY

namespace lofty { namespace _std {

exception::exception() {
}

/*virtual*/ exception::~exception() {
}

/*virtual*/ char const * exception::what() const {
   return "lofty::_std::exception";
}

}}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_STL_LOFTY || LOFTY_HOST_STL_MSVCRT == 1600

namespace lofty { namespace _std {

bad_weak_ptr::bad_weak_ptr() {
}

/*virtual*/ bad_weak_ptr::~bad_weak_ptr() {
}

/*virtual*/ char const * bad_weak_ptr::what() const /*override*/ {
   return "lofty::_std::bad_weak_ptr";
}

}}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_STL_LOFTY || LOFTY_HOST_STL_MSVCRT == 1600

namespace lofty { namespace _std { namespace _pvt {

shared_refcount::shared_refcount(unsigned strong_refs_, unsigned weak_refs_) :
   strong_refs(strong_refs_),
   weak_refs(weak_refs_ + (strong_refs_ > 0 ? 1 : 0)) {
}

/*virtual*/ shared_refcount::~shared_refcount() {
   LOFTY_ASSERT(
      strong_refs.load() == 0, LOFTY_SL("shared_refcount being destructed with non-zero strong references!")
   );
   LOFTY_ASSERT(
      weak_refs.load() == 0, LOFTY_SL("shared_refcount being destructed with non-zero weak references!")
   );
}

void shared_refcount::add_strong_ref() {
   // Increment the count of strong references if non-zero; it it’s zero, the owned object is gone.
   unsigned old_strong_refs;
   do {
      old_strong_refs = strong_refs.load();
      if (old_strong_refs == 0) {
         throw bad_weak_ptr();
      }
   } while (!strong_refs.compare_exchange_strong(old_strong_refs, old_strong_refs + 1));
}

/*virtual*/ void * shared_refcount::get_deleter(type_info const &) const {
   return nullptr;
}

/*virtual*/ void shared_refcount::delete_this() {
   delete this;
}

}}} //namespace lofty::_std::_pvt

#endif //ifdef _LOFTY_STD_MEMORY_HXX

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_STL_LOFTY

namespace lofty { namespace _std {

nothrow_t const nothrow;

}}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_STL_LOFTY

namespace lofty { namespace _std {

bad_alloc::bad_alloc() {
}

/*virtual*/ bad_alloc::~bad_alloc() {
}

/*virtual*/ char const * bad_alloc::what() const /*override*/ {
   return "lofty::_std::bad_alloc";
}

}}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_STL_LOFTY || !defined(LOFTY_CXX_VARIADIC_TEMPLATES)

namespace lofty { namespace _std {

_pvt::ignore_t const ignore;

}}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_STL_LOFTY

namespace lofty { namespace _std {

bad_cast::bad_cast() {
}

/*virtual*/ bad_cast::~bad_cast() {
}

/*virtual*/ char const * bad_cast::what() const /*override*/ {
   return "lofty::_std::bad_cast";
}

}}

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_STL_LOFTY

namespace lofty { namespace _std {

bad_typeid::bad_typeid() {
}

/*virtual*/ bad_typeid::~bad_typeid() {
}

/*virtual*/ char const * bad_typeid::what() const /*override*/ {
   return "lofty::_std::bad_typeid";
}

}}

#endif
