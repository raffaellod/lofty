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


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

coroutine_local_storage_registrar::all_data_members coroutine_local_storage_registrar::sm_adm =
   ABC_DETAIL_CONTEXT_LOCAL_STORAGE_REGISTRAR_INITIALIZER;

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

coroutine_local_storage::~coroutine_local_storage() {
   unsigned iRemainingAttempts = 10;
   bool bAnyDestructed;
   do {
      bAnyDestructed = destruct_vars();
   } while (--iRemainingAttempts > 0 && bAnyDestructed);
}

bool coroutine_local_storage::destruct_vars() {
   bool bAnyDestructed = false;
   // Iterate backwards over the list to destruct CRLS for this coroutine.
   auto const & crlsr = coroutine_local_storage_registrar::instance();
   unsigned i = crlsr.m_cVars;
   for (auto it(crlsr.rbegin()), itEnd(crlsr.rend()); it != itEnd; ++it) {
      if (is_var_constructed(--i)) {
         it->destruct(get_storage(&*it));
         var_destructed(i);
         bAnyDestructed = true;
      }
   }
   return bAnyDestructed;
}

}} //namespace abc::detail
