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
#include <abaclade/bitmanip.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::coroutine_local_storage

namespace abc {
namespace detail {

ABC_COLLECTIONS_STATIC_LIST_DEFINE_SUBCLASS_STATIC_MEMBERS(coroutine_local_storage)
std::size_t coroutine_local_storage::sm_cb = 0;
/* Important: sm_pcrls MUST be defined before sm_crls to guarantee that their per-thread copies are
constructed in this same order, which is necessary since constructing a copy of sm_crls assigns a
new value to the copy of sm_pcrls; constructing sm_pcrls after sm_crls would cause its value to be
lost. */
thread_local_value<coroutine_local_storage *> coroutine_local_storage::sm_pcrls /*= nullptr*/;
thread_local_value<coroutine_local_storage> coroutine_local_storage::sm_crls;

coroutine_local_storage::coroutine_local_storage() :
   m_pb(new std::int8_t[sm_cb]) {

   // Iterate over the list to construct CRLS for this coroutine.
   for (auto it(begin()), itEnd(end()); it != itEnd; ++it) {
      it->construct(get_storage(it->m_ibStorageOffset));
   }

   // This will only change if the current thread is set to schedule coroutines.
   sm_pcrls = this;
}

coroutine_local_storage::~coroutine_local_storage() {
   /* Don’t bother to clear sm_pcrls: if this is a coroutine’s storage, then sm_pcrls != this, so it
   should’t be cleared; if it’s a thread’s storage, then the thread is terminating, so sm_pcrls is
   about to go away as well, so clearing it is unnecessary. */
   if (sm_pcrls == this) {
      sm_pcrls = nullptr;
   }

   // Iterate backwards over the list to destruct CRLS for this coroutine.
   for (auto it(rbegin()), itEnd(rend()); it != itEnd; ++it) {
      it->destruct(get_storage(it->m_ibStorageOffset));
   }
}

/*static*/ void coroutine_local_storage::add_var(
   coroutine_local_var_impl * pcrlvi, std::size_t cb
) {
   // Calculate the offset for *pcrlvi’s storage and increase sm_cb accordingly.
   pcrlvi->m_ibStorageOffset = sm_cb;
   sm_cb += bitmanip::ceiling_to_pow2_multiple(cb, sizeof(abc::max_align_t));
}

/*static*/ coroutine_local_storage * coroutine_local_storage::get() {
   return sm_pcrls;
}

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::coroutine_local_var_impl

namespace abc {
namespace detail {

coroutine_local_var_impl::coroutine_local_var_impl(std::size_t cbObject) {
   // Initializes m_ibStorageOffset.
   coroutine_local_storage::add_var(this, cbObject);
}

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
