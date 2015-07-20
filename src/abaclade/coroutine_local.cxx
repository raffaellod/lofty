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

ABC_COLLECTIONS_STATIC_LIST_DEFINE_SUBCLASS_STATIC_MEMBERS(coroutine_local_storage)
context_local_storage_impl::static_members_t coroutine_local_storage::sm_sm = { 0, 0, 0 };

coroutine_local_storage::coroutine_local_storage() :
   context_local_storage_impl(&sm_sm) {
}

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
   unsigned i = sm_sm.cVars;
   for (auto it(rbegin()), itEnd(rend()); it != itEnd; ++it) {
      if (is_var_constructed(--i)) {
         it->destruct(get_storage(&*it));
         var_destructed(i);
         bAnyDestructed = true;
      }
   }
   return bAnyDestructed;
}

}} //namespace abc::detail
