﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

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
#ifdef _LOFTY_STD_NEW_HXX


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {

nothrow_t const nothrow;

}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {

bad_alloc::bad_alloc() {
}

/*virtual*/ bad_alloc::~bad_alloc() {
}

/*virtual*/ char const * bad_alloc::what() const /*override*/ {
   return "lofty::_std::bad_alloc";
}

}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifdef _LOFTY_STD_NEW_HXX