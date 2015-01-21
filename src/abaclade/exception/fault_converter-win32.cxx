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

// #include <abaclade.hxx> already done in exception-fault_converter.cxx.


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::exception::fault_converter

namespace {

//! Structured Exception translator on program startup.
::_se_translator_function g_setfDefault;

//! Translates Win32 Structured Exceptions into C++ exceptions, whenever possible.
void ABC_STL_CALLCONV fault_se_translator(unsigned iCode, ::_EXCEPTION_POINTERS * pxpInfo) {
   switch (iCode) {
      case EXCEPTION_ACCESS_VIOLATION: {
         /* Attempt to read from or write to an inaccessible address.
         ExceptionInformation[0] contains a read-write flag that indicates the type of operation
         that caused the access violation. If this value is zero, the thread attempted to read the
         inaccessible data. If this value is 1, the thread attempted to write to an inaccessible
         address. If this value is 8, the thread causes a user-mode data execution prevention (DEP)
         violation.
         ExceptionInformation[1] specifies the virtual address of the inaccessible data. */
         void const * pAddr = reinterpret_cast<void const *>(
            pxpInfo->ExceptionRecord->ExceptionInformation[1]
         );
         if (pAddr == nullptr) {
            ABC_THROW(abc::null_pointer_error, ());
         } else {
            ABC_THROW(abc::memory_address_error, (pAddr));
         }
      }

//    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
         /* Attempt to access an array element that is out of bounds, and the underlying hardware
         supports bounds checking. */
//       break;

      case EXCEPTION_DATATYPE_MISALIGNMENT:
         // Attempt to read or write data that is misaligned on hardware that requires alignment.
         ABC_THROW(abc::memory_access_error, (nullptr));

      case EXCEPTION_FLT_DENORMAL_OPERAND:
         /* An operand in a floating-point operation is too small to represent as a standard
         floating-point value. */
         // Fall through.
      case EXCEPTION_FLT_DIVIDE_BY_ZERO:
         // Attempt to divide a floating-point value by a floating-point divisor of zero.
         // Fall through.
      case EXCEPTION_FLT_INEXACT_RESULT:
         /* The result of a floating-point operation cannot be represented exactly as a decimal
         fraction. */
         // Fall through.
      case EXCEPTION_FLT_INVALID_OPERATION:
         // Other floating-point exception.
         // Fall through.
      case EXCEPTION_FLT_OVERFLOW:
         /* The exponent of a floating-point operation is greater than the magnitude allowed by the
         corresponding type. */
         // Fall through.
      case EXCEPTION_FLT_STACK_CHECK:
         // The stack overflowed or underflowed as a result of a floating-point operation.
         // Fall through.
      case EXCEPTION_FLT_UNDERFLOW:
         /* The exponent of a floating-point operation is less than the magnitude allowed by the
         corresponding type. */
         ABC_THROW(abc::floating_point_error, ());

      case EXCEPTION_ILLEGAL_INSTRUCTION:
         // Attempt to execute an invalid instruction.
         break;

      case EXCEPTION_IN_PAGE_ERROR:
         /* Attempt to access a page that was not present, and the system was unable to load the
         page. For example, this exception might occur if a network connection is lost while running
         a program over the network. */
         break;

      case EXCEPTION_INT_DIVIDE_BY_ZERO:
         // The thread attempted to divide an integer value by an integer divisor of zero.
         ABC_THROW(abc::division_by_zero_error, ());

      case EXCEPTION_INT_OVERFLOW:
         /* The result of an integer operation caused a carry out of the most significant bit of the
         result. */
         ABC_THROW(abc::overflow_error, ());

      case EXCEPTION_PRIV_INSTRUCTION:
         /* Attempt to execute an instruction whose operation is not allowed in the current machine
         mode. */
         break;

      case EXCEPTION_STACK_OVERFLOW:
         // The thread used up its stack.
         break;
   }
}

} //namespace

namespace abc {

exception::fault_converter::fault_converter() {
   // Install the translator of Win32 structured exceptions into C++ exceptions.
   g_setfDefault = ::_set_se_translator(&fault_se_translator);
}

exception::fault_converter::~fault_converter() {
   ::_set_se_translator(g_setfDefault);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
