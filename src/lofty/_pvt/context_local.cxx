/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2017 Raffaello D. Di Napoli

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
#include <lofty/bitmanip.hxx>

#include <cstdlib> // std::abort()


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

void context_local_storage_registrar_impl::add_var(
   context_local_storage_node_impl * var, std::size_t var_byte_size
) {
   var->storage_index = vars_count++;
   // Calculate the offset for *var’s storage and increase var_size accordingly.
   var->storage_byte_offset = vars_byte_size;
   vars_byte_size += bitmanip::ceiling_to_pow2_multiple(var_byte_size, sizeof(_std::max_align_t));
   if (frozen_byte_size != 0 && vars_byte_size > frozen_byte_size) {
      // TODO: can’t log/report anything since no thread locals are available! Fix me!
      std::abort();
   }
}

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

context_local_storage_impl::context_local_storage_impl(context_local_storage_registrar_impl * registrar) :
   vars_constructed(new bool[registrar->vars_count]),
   bytes(new std::int8_t[registrar->vars_byte_size]) {
   memory::clear(vars_constructed.get(), registrar->vars_count);
   memory::clear(bytes.get(), registrar->vars_byte_size);
   if (registrar->frozen_byte_size == 0) {
      // Track the size of this first block.
      registrar->frozen_byte_size = registrar->vars_byte_size;
   }
}

context_local_storage_impl::~context_local_storage_impl() {
}

bool context_local_storage_impl::destruct_vars(context_local_storage_registrar_impl const & registrar) {
   bool any_destructed = false;
   // Iterate backwards over the list to destruct TLS/CRLS for this storage.
   unsigned i = registrar.vars_count;
   for (auto itr(registrar.rbegin()), end(registrar.rend()); itr != end; ++itr) {
      auto & var = static_cast<context_local_storage_node_impl &>(*itr);
      if (vars_constructed[--i]) {
         if (var.destruct) {
            var.destruct(&bytes[var.storage_byte_offset]);
            /* Only set any_destructed if we executed a destructor: if we didn’t, it can’t have re-constructed
            any other variables. */
            any_destructed = true;
         }
         vars_constructed[i] = false;
      }
   }
   return any_destructed;
}

void * context_local_storage_impl::get_storage(context_local_storage_node_impl const & var) {
   void * ret = &bytes[var.storage_byte_offset];
   if (!vars_constructed[var.storage_index]) {
      if (var.construct) {
         var.construct(ret);
      }
      vars_constructed[var.storage_index] = true;
   }
   return ret;
}

}} //namespace lofty::_pvt
