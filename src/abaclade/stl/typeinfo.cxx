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
// std::bad_alloc


namespace std {

bad_cast::bad_cast() {
}


/*virtual*/ bad_cast::~bad_cast() {
}


/*virtual*/ char const * bad_cast::what() const {
   return "std::bad_cast";
}

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// std::bad_typeid


namespace std {

bad_typeid::bad_typeid() {
}


/*virtual*/ bad_typeid::~bad_typeid() {
}


/*virtual*/ char const * bad_typeid::what() const {
   return "std::bad_typeid";
}

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifdef ABC_STLIMPL

