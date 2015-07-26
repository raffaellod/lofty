/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014, 2015
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

void context_local_storage_registrar_impl::add_var(
   context_local_storage_node_impl * pclsni, std::size_t cb
) {
   pclsni->m_iStorageIndex = m_cVars++;
   // Calculate the offset for *pclsni’s storage and increase cb accordingly.
   pclsni->m_ibStorageOffset = m_cb;
   m_cb += bitmanip::ceiling_to_pow2_multiple(cb, sizeof(abc::max_align_t));
   if (m_cbFrozen && m_cb > m_cbFrozen) {
      // TODO: can’t log/report anything since no thread locals are available! Fix me!
      std::abort();
   }
}

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

context_local_storage_impl::context_local_storage_impl(
   context_local_storage_registrar_impl * pclsri
) :
   m_pbConstructed(new bool[pclsri->m_cVars]),
   m_pb(new std::int8_t[pclsri->m_cb]) {
   memory::clear(m_pbConstructed.get(), pclsri->m_cVars);
   memory::clear(m_pb.get(), pclsri->m_cb);
   if (pclsri->m_cbFrozen == 0) {
      // Track the size of this first block.
      pclsri->m_cbFrozen = pclsri->m_cb;
   }
}

context_local_storage_impl::~context_local_storage_impl() {
}

void * context_local_storage_impl::get_storage(context_local_storage_node_impl const & clsni) {
   void * pb = &m_pb[clsni.m_ibStorageOffset];
   if (!m_pbConstructed[clsni.m_iStorageIndex]) {
      if (clsni.construct) {
         clsni.construct(pb);
      }
      m_pbConstructed[clsni.m_iStorageIndex] = true;
   }
   return pb;
}

bool context_local_storage_impl::destruct_vars(context_local_storage_registrar_impl const & clsri) {
   bool bAnyDestructed = false;
   // Iterate backwards over the list to destruct TLS/CRLS for this coroutine.
   unsigned i = clsri.m_cVars;
   for (auto it(clsri.rbegin()), itEnd(clsri.rend()); it != itEnd; ++it) {
      auto & clsni = static_cast<context_local_storage_node_impl &>(*it);
      if (m_pbConstructed[--i]) {
         if (clsni.destruct) {
            clsni.destruct(&m_pb[clsni.m_ibStorageOffset]);
            /* Only set bAnyDestructed if we executed a destructor: if we didn’t, it can’t have
            re-constructed any other variables. */
            bAnyDestructed = true;
         }
         m_pbConstructed[i] = false;
      }
   }
   return bAnyDestructed;
}

}} //namespace abc::detail
