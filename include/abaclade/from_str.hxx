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

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> instead of this file
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::from_str()

namespace abc {

/*! Returns an object constructed from its string representation, optionally with a custom format.

s
   String to reconstruct into an object.
sFormat
   Type-specific format string.
return
   Object reconstructed from s according to sFormat.
*/
template <typename T>
T from_str(istr const & s, istr const & sFormat = istr::empty);

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::from_str_backend

namespace abc {

/*! Parses a string into an object. Once constructed with the desired format specification, an
instance can convert into T instances any number of strings. */
template <typename T>
class from_str_backend;

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

