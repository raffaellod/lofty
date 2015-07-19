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
unsigned coroutine_local_storage::sm_cVars = 0;
std::size_t coroutine_local_storage::sm_cb = 0;
std::size_t coroutine_local_storage::sm_cbFrozen = 0;

coroutine_local_storage::coroutine_local_storage() :
   m_pbConstructed(new bool[sm_cVars]),
   m_pb(new std::int8_t[sm_cb]) {
   memory::clear(m_pbConstructed.get(), sm_cVars);
   if (sm_cbFrozen == 0) {
      // Track the size of this first block.
      sm_cbFrozen = sm_cb;
   }
}

coroutine_local_storage::~coroutine_local_storage() {
   unsigned iRemainingAttempts = 10;
   bool bAnyDestructed;
   do {
      bAnyDestructed = destruct_vars();
   } while (--iRemainingAttempts > 0 && bAnyDestructed);
}

/*static*/ void coroutine_local_storage::add_var(
   coroutine_local_var_impl * pcrlvi, std::size_t cb
) {
   pcrlvi->m_iStorageIndex = sm_cVars++;
   // Calculate the offset for *pcrlvi’s storage and increase sm_cb accordingly.
   pcrlvi->m_ibStorageOffset = sm_cb;
   sm_cb += bitmanip::ceiling_to_pow2_multiple(cb, sizeof(abc::max_align_t));
   if (sm_cbFrozen && sm_cb > sm_cbFrozen) {
      // TODO: can’t log/report anything since no thread locals are available! Fix me!
      std::abort();
   }
}

bool coroutine_local_storage::destruct_vars() {
   bool bAnyDestructed = false;
   // Iterate backwards over the list to destruct CRLS for this coroutine.
   bool * pb = m_pbConstructed.get();
   for (auto it(rbegin()), itEnd(rend()); it != itEnd; ++pb, ++it) {
      if (*pb) {
         it->destruct(get_storage(&*it));
         *pb = false;
         bAnyDestructed = true;
      }
   }
   return bAnyDestructed;
}

void * coroutine_local_storage::get_storage(coroutine_local_var_impl const * pcrlvi) {
   bool * pbConstructed = &m_pbConstructed[pcrlvi->m_iStorageIndex];
   void * pb = &m_pb[pcrlvi->m_ibStorageOffset];
   if (!*pbConstructed) {
      pcrlvi->construct(pb);
      *pbConstructed = true;
   }
   return pb;
}

}} //namespace abc::detail
