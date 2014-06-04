/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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
#ifdef ABC_STLIMPL



////////////////////////////////////////////////////////////////////////////////////////////////////
// std::bad_weak_ptr


namespace std {

bad_weak_ptr::bad_weak_ptr() {
}


/*virtual*/ bad_weak_ptr::~bad_weak_ptr() {
}


/*virtual*/ char const * bad_weak_ptr::what() const {
   return "std::bad_weak_ptr";
}

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// std::_shared_refcount


namespace std {

_shared_refcount::_shared_refcount(
   ::abc::atomic::int_t cStrongRefs, ::abc::atomic::int_t cWeakRefs
) :
   m_cStrongRefs(cStrongRefs),
   m_cWeakRefs(cWeakRefs + (cStrongRefs > 0 ? 1 : 0)) {
}


/*virtual*/ _shared_refcount::~_shared_refcount() {
   ABC_ASSERT(m_cStrongRefs == 0);
   ABC_ASSERT(m_cWeakRefs == 0);
}


void _shared_refcount::add_strong_ref() {
   // Increment the count of strong references if non-zero; it it’s zero, the owned object is gone.
   ::abc::atomic::int_t cStrongRefsOld;
   do {
      cStrongRefsOld = m_cStrongRefs;
      if (cStrongRefsOld <= 0) {
         throw bad_weak_ptr();
      }
   } while (::abc::atomic::compare_and_swap(
      &m_cStrongRefs, cStrongRefsOld + 1, cStrongRefsOld
   ) != cStrongRefsOld);
}


/*virtual*/ void * _shared_refcount::get_deleter(type_info const & ti) const {
   ABC_UNUSED_ARG(ti);
   return nullptr;
}


/*virtual*/ void _shared_refcount::delete_this() {
   delete this;
}

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifdef ABC_STLIMPL

