/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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
#ifdef ABC_STLIMPL


////////////////////////////////////////////////////////////////////////////////////////////////////
// std::nothrow_t and std::nothrow

namespace std {

nothrow_t const nothrow;

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////
// std::bad_alloc

namespace std {

bad_alloc::bad_alloc() {
}

/*virtual*/ bad_alloc::~bad_alloc() {
}

/*virtual*/ char const * bad_alloc::what() const /*override*/ {
   return "std::bad_alloc";
}

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifdef ABC_STLIMPL

