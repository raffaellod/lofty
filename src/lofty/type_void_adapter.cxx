/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015, 2017 Raffaello D. Di Napoli

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
#include <lofty/type_void_adapter.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*static*/ void type_void_adapter::copy_construct_trivial_impl(
   std::int8_t * dst_bytes_begin, std::int8_t * src_bytes_begin, std::int8_t * src_bytes_end
) {
   memory::copy(dst_bytes_begin, src_bytes_begin, static_cast<std::size_t>(src_bytes_end - src_bytes_begin));
}

/*static*/ void type_void_adapter::destruct_trivial_impl(void const * begin, void const * end) {
   LOFTY_UNUSED_ARG(begin);
   LOFTY_UNUSED_ARG(end);
   // Nothing to do.
}

} //namespace lofty
