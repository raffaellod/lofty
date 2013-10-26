/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

Copyright 2010, 2011, 2012, 2013
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abc/core.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals


namespace abc {

ABCAPI unsafe_t const unsafe;

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_explob_helper


namespace abc {

#ifndef ABC_CXX_EXPLICIT_CONVERSION_OPERATORS

ABCAPI void _explob_helper::bool_true() const {
}

#endif

} //namespace ABC


////////////////////////////////////////////////////////////////////////////////////////////////////

