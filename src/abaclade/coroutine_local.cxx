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

#include <cstdlib> // std::abort()


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

ABC_COLLECTIONS_STATIC_LIST_DEFINE_SUBCLASS_STATIC_MEMBERS(coroutine_local_storage)
std::size_t coroutine_local_storage::sm_cb = 0;
std::size_t coroutine_local_storage::sm_cbFrozen = 0;

/*explicit*/ coroutine_local_storage::coroutine_local_storage(bool bNewThread) {
   if (sm_cbFrozen == 0) {
      // Track the size of this first block.
      sm_cbFrozen = sm_cb;
   }

   if (!bNewThread) {
      construct_vars();
   }
}

coroutine_local_storage::~coroutine_local_storage() {
   if (m_pb) {
      destruct_vars();
   }
}

/*static*/ void coroutine_local_storage::add_var(
   coroutine_local_var_impl * pcrlvi, std::size_t cb
) {
   // Calculate the offset for *pcrlvi’s storage and increase sm_cb accordingly.
   pcrlvi->m_ibStorageOffset = sm_cb;
   sm_cb += bitmanip::ceiling_to_pow2_multiple(cb, sizeof(abc::max_align_t));
   if (sm_cbFrozen && sm_cb > sm_cbFrozen) {
      // TODO: can’t log/report anything since no thread locals are available! Fix me!
      std::abort();
   }
}

void coroutine_local_storage::construct_vars() {
   m_pb.reset(new std::int8_t[sm_cb]);
   // Iterate over the list to construct CRLS for this coroutine.
   for (auto it(begin()), itEnd(end()); it != itEnd; ++it) {
      it->construct(get_storage(it->m_ibStorageOffset));
   }
}

void coroutine_local_storage::destruct_vars() {
   // Iterate backwards over the list to destruct CRLS for this coroutine.
   for (auto it(rbegin()), itEnd(rend()); it != itEnd; ++it) {
      it->destruct(get_storage(it->m_ibStorageOffset));
   }
   m_pb.reset();
}

/*static*/ coroutine_local_storage * coroutine_local_storage::get() {
   return thread_local_storage::get()->m_pcrls;
}

/*static*/ void coroutine_local_storage::get_default_and_current_pointers(
   coroutine_local_storage ** ppcrlsDefault, coroutine_local_storage *** pppcrlsCurrent
) {
   *ppcrlsDefault = &thread_local_storage::get()->m_crls;
   *pppcrlsCurrent = &thread_local_storage::get()->m_pcrls;
}

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

coroutine_local_var_impl::coroutine_local_var_impl(std::size_t cbObject) {
   // Initializes m_ibStorageOffset.
   coroutine_local_storage::add_var(this, cbObject);
}

}} //namespace abc::detail
