/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

coroutine_local_storage_registrar::data_members coroutine_local_storage_registrar::data_members_ =
   LOFTY__PVT_CONTEXT_LOCAL_STORAGE_REGISTRAR_INITIALIZER;

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

coroutine_local_storage::coroutine_local_storage() :
   context_local_storage_impl(&coroutine_local_storage_registrar::instance()) {
}

coroutine_local_storage::~coroutine_local_storage() {
   unsigned remaining_attempts = 10;
   bool any_destructed;
   do {
      any_destructed = destruct_vars(coroutine_local_storage_registrar::instance());
   } while (--remaining_attempts > 0 && any_destructed);
}

}} //namespace lofty::_pvt
