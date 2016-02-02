/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2016 Raffaello D. Di Napoli

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

#ifndef _ABACLADE_FROM_STR_HXX
#define _ABACLADE_FROM_STR_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _pvt {

/*! Throws an abc::text::syntax_error if a call to abc::from_str() did not consume the entire source
string.

@param sis
   Temporary stream used by the implementation of abc::from_str().
@param sSrc
   Source string.
@param sFormat
   Format string.
*/
ABACLADE_SYM void throw_on_unused_from_str_chars(
   io::text::str_istream const & sis, str const & sSrc, str const & sFormat
);

}} //namespace abc::_pvt

namespace abc {

/*! Returns an object constructed from its string representation, optionally with a custom format.

@param s
   String to reconstruct into an object.
@param sFormat
   Type-specific format string.
@return
   Object reconstructed from s according to sFormat.
*/
template <typename T>
inline T from_str(str const & s, str const & sFormat = str::empty) {
   io::text::str_istream sis(external_buffer, &s);
   from_text_istream<T> ftis;
   ftis.set_format(sFormat);
   T t;
   ftis.read(&t, &sis);
   _pvt::throw_on_unused_from_str_chars(sis, s, sFormat);
   return _std::move(t);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_FROM_STR_HXX
