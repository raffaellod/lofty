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


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::exception::fault_converter

#if ABC_HOST_API_POSIX
   #include <cstdlib> // std::abort()

namespace {

//! Possible exception types thrown by throw_after_fault().
ABC_ENUM_AUTO_VALUES(fault_exception_types,
   arithmetic_error,
   division_by_zero_error,
   floating_point_error,
   memory_access_error,
   memory_address_error,
   null_pointer_error,
   overflow_error
);

/*! Throws an exception of the specified type.

@param fxt
   Type of exception to be throw.
@param iArg0
   Exception type-specific argument 0.
@param iArg1
   Exception type-specific argument 1.
*/
void throw_after_fault(
   fault_exception_types::enum_type fxt, std::intptr_t iArg0, std::intptr_t iArg1
) {
   ABC_UNUSED_ARG(iArg1);
   switch (fxt) {
      case fault_exception_types::arithmetic_error:
         ABC_THROW(abc::arithmetic_error, ());
      case fault_exception_types::division_by_zero_error:
         ABC_THROW(abc::division_by_zero_error, ());
      case fault_exception_types::floating_point_error:
         ABC_THROW(abc::floating_point_error, ());
      case fault_exception_types::memory_access_error:
         ABC_THROW(abc::memory_access_error, (reinterpret_cast<void const *>(iArg0)));
      case fault_exception_types::memory_address_error:
         ABC_THROW(abc::memory_address_error, (reinterpret_cast<void const *>(iArg0)));
      case fault_exception_types::null_pointer_error:
         ABC_THROW(abc::null_pointer_error, ());
      case fault_exception_types::overflow_error:
         ABC_THROW(abc::overflow_error, ());
      default:
         // Unexpected exception type. Should never happen.
         std::abort();
   }
}

} //namespace

   #if ABC_HOST_API_MACH
      #include "exception-fault_converter-mach.cxx"
   #else
      #include "exception-fault_converter-posix.cxx"
   #endif
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   #include "exception-fault_converter-win32.cxx"
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else

////////////////////////////////////////////////////////////////////////////////////////////////////
