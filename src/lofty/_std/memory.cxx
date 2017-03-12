/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

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
#ifdef _LOFTY_STD_MEMORY_HXX


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {

bad_weak_ptr::bad_weak_ptr() {
}

/*virtual*/ bad_weak_ptr::~bad_weak_ptr() {
}

/*virtual*/ char const * bad_weak_ptr::what() const /*override*/ {
   return "lofty::_std::bad_weak_ptr";
}

}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std { namespace _pvt {

shared_refcount::shared_refcount(unsigned strong_refs_, unsigned weak_refs_) :
   strong_refs(strong_refs_),
   weak_refs(weak_refs_ + (strong_refs > 0 ? 1 : 0)) {
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifdef _LOFTY_STD_MEMORY_HXX
