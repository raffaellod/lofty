/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2016 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#ifdef _ABACLADE_STD_MEMORY_HXX


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std {

bad_weak_ptr::bad_weak_ptr() {
}

/*virtual*/ bad_weak_ptr::~bad_weak_ptr() {
}

/*virtual*/ char const * bad_weak_ptr::what() const /*override*/ {
   return "abc::_std::bad_weak_ptr";
}

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std { namespace _pvt {

shared_refcount::shared_refcount(unsigned cStrongRefs, unsigned cWeakRefs) :
   m_cStrongRefs(cStrongRefs),
   m_cWeakRefs(cWeakRefs + (cStrongRefs > 0 ? 1 : 0)) {
}

/*virtual*/ shared_refcount::~shared_refcount() {
   ABC_ASSERT(
      m_cStrongRefs.load() == 0,
      ABC_SL("shared_refcount being destructed with non-zero strong references!")
   );
   ABC_ASSERT(
      m_cWeakRefs.load() == 0,
      ABC_SL("shared_refcount being destructed with non-zero weak references!")
   );
}

void shared_refcount::add_strong_ref() {
   // Increment the count of strong references if non-zero; it it’s zero, the owned object is gone.
   unsigned cStrongRefsOld;
   do {
      cStrongRefsOld = m_cStrongRefs.load();
      if (cStrongRefsOld == 0) {
         throw bad_weak_ptr();
      }
   } while (!m_cStrongRefs.compare_exchange_strong(cStrongRefsOld, cStrongRefsOld + 1));
}

/*virtual*/ void * shared_refcount::get_deleter(type_info const & ti) const {
   ABC_UNUSED_ARG(ti);
   return nullptr;
}

/*virtual*/ void shared_refcount::delete_this() {
   delete this;
}

}}} //namespace abc::_std::_pvt

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifdef _ABACLADE_STD_MEMORY_HXX
