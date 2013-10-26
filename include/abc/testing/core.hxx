/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

Copyright 2013
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

#ifndef ABC_TESTING_CORE_HXX
#define ABC_TESTING_CORE_HXX

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif


/** Declares a symbol to be publicly visible (from the ABC testing shared library) or imported from
ABC’s testing shared library (into another library/executable). */
#ifdef _ABC_TESTING_LIB_BUILD
	#define ABCTESTINGAPI ABC_SYM_EXPORT
#else
	#define ABCTESTINGAPI ABC_SYM_IMPORT
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_TESTING_CORE_HXX

