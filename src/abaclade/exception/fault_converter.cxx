/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014, 2015
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


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::exception::fault_converter

#if ABC_HOST_API_POSIX
   #include <cstdlib> // std::abort()

   #if ABC_HOST_API_MACH
      #include "fault_converter-mach.cxx"
   #else
      #include "fault_converter-posix.cxx"
   #endif
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   #include "fault_converter-win32.cxx"
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else

////////////////////////////////////////////////////////////////////////////////////////////////////
