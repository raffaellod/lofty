/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#ifdef _ABACLADE_STD_EXCEPTION_HXX


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std {

exception::exception() {
}

/*virtual*/ exception::~exception() {
}

/*virtual*/ char const * exception::what() const {
   return "abc::_std::exception";
}

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifdef _ABACLADE_STD_EXCEPTION_HXX
