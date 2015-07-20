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

context_local_storage_impl::context_local_storage_impl(static_members_t * psm) :
   m_pbConstructed(new bool[psm->cVars]),
   m_pb(new std::int8_t[psm->cb]) {
   memory::clear(m_pbConstructed.get(), psm->cVars);
   if (psm->cbFrozen == 0) {
      // Track the size of this first block.
      psm->cbFrozen = psm->cb;
   }
}

context_local_storage_impl::~context_local_storage_impl() {
}

/*static*/ void context_local_storage_impl::add_var(
   static_members_t * psm, context_local_var_impl_base * pclvi, std::size_t cb
) {
   pclvi->m_iStorageIndex = psm->cVars++;
   // Calculate the offset for *pclvi’s storage and increase cb accordingly.
   pclvi->m_ibStorageOffset = psm->cb;
   psm->cb += bitmanip::ceiling_to_pow2_multiple(cb, sizeof(abc::max_align_t));
   if (psm->cbFrozen && psm->cb > psm->cbFrozen) {
      // TODO: can’t log/report anything since no thread locals are available! Fix me!
      std::abort();
   }
}

void * context_local_storage_impl::get_storage(context_local_var_impl_base const * pclvi) {
   void * pb = &m_pb[pclvi->m_ibStorageOffset];
   if (!m_pbConstructed[pclvi->m_iStorageIndex]) {
      pclvi->construct(pb);
      m_pbConstructed[pclvi->m_iStorageIndex] = true;
   }
   return pb;
}

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

context_local_var_impl_base::context_local_var_impl_base() {
}

context_local_var_impl_base::~context_local_var_impl_base() {
}

}} //namespace abc::detail
