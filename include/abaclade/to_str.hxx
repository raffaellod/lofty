/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2016 Raffaello D. Di Napoli

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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*! Returns the string representation of the specified value, optionally with a custom format.

abc::to_str() is a more advanced alternative to std::to_string() (see C++11 § 21.5 “Numeric
conversions”); here are the main differences when compared to the STL function:

•  It accepts an additional argument, controlling how the conversion to string is to be done;

•  Its default specialization relies on abc::to_text_ostream, which outputs its result to an
   abc::io::text::ostream instance; this means that the complete specialization is shared with
   abc::io::text::ostream::print();

The format specification is provided to an abc::to_text_ostream specialization by passing it an
abc::text::str const &, so a caller can specify a non-NUL-terminated substring of a larger string
without the need for temporary strings. Once an abc::to_text_ostream instance has been constructed,
it must be able to sequentially process an infinite number of conversions, i.e. instances of a
to_text_ostream specialization must be reusable.

The interpretation of the format specification is up to the specialization of abc::to_text_ostream.

When code causes the compiler to instantiate a specialization of abc::to_text_ostream that has not
been defined, GCC will generate an error:

   @verbatim
   error: ‘abc::to_text_ostream<my_type> …’ has incomplete type
   @endverbatim

The only fix for this is to provide an explicit specialization of abc::to_text_ostream for my_type.

@param t
   Object to generate a string representation for.
@param sFormat
   Type-specific format string.
@return
   String representation of t according to sFormat.
*/
template <typename T>
inline str to_str(T const & t, str const & sFormat = str::empty) {
   io::text::str_ostream sos;
   {
      to_text_ostream<T> ttos;
      ttos.set_format(sFormat);
      ttos.write(t, &sos);
   }
   return sos.release_content();
}

} //namespace abc
