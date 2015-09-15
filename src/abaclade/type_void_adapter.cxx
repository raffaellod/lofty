/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
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
#include <abaclade/type_void_adapter.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*static*/ void type_void_adapter::copy_construct_trivial_impl(
   std::int8_t * pbDstBegin, std::int8_t * pbSrcBegin, std::int8_t * pbSrcEnd
) {
   memory::copy(pbDstBegin, pbSrcBegin, static_cast<std::size_t>(pbSrcEnd - pbSrcBegin));
}

/*static*/ void type_void_adapter::destruct_trivial_impl(void const * pBegin, void const * pEnd) {
   ABC_UNUSED_ARG(pBegin);
   ABC_UNUSED_ARG(pEnd);
   // Nothing to do.
}

} //namespace abc
