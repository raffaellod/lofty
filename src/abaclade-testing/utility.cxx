/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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
#include <abaclade/testing/utility.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::utility::instances_counter

namespace abc {
namespace testing {
namespace utility {

std::size_t instances_counter::m_cCopies = 0;
std::size_t instances_counter::m_cMoves = 0;
std::size_t instances_counter::m_cNew = 0;
int instances_counter::m_iNextUnique = 0;

} //namespace utility
} //namespace testing
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

