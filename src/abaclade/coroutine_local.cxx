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

coroutine_local_storage_registrar::data_members coroutine_local_storage_registrar::sm_dm =
   ABC_DETAIL_CONTEXT_LOCAL_STORAGE_REGISTRAR_INITIALIZER;

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

coroutine_local_storage::coroutine_local_storage() :
   context_local_storage_impl(&coroutine_local_storage_registrar::instance()) {
}

coroutine_local_storage::~coroutine_local_storage() {
   unsigned iRemainingAttempts = 10;
   bool bAnyDestructed;
   do {
      bAnyDestructed = destruct_vars(coroutine_local_storage_registrar::instance());
   } while (--iRemainingAttempts > 0 && bAnyDestructed);
}

}} //namespace abc::detail
