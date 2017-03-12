/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_FROM_STR_HXX
#define _LOFTY_FROM_STR_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

/*! Throws a lofty::text::syntax_error if a call to lofty::from_str() did not consume the entire source
string.

@param src
   Temporary stream used by the implementation of lofty::from_str().
*/
LOFTY_SYM void throw_on_unused_from_str_chars(io::text::str_istream const & src);

}} //namespace lofty::_pvt

namespace lofty {

/*! Returns an object constructed from its string representation, optionally with a custom format.

@param s
   String to reconstruct into an object.
@param format
   Type-specific format string.
@return
   Object reconstructed from s according to format.
*/
template <typename T>
inline T from_str(str const & s, str const & format = str::empty) {
   io::text::str_istream istream(external_buffer, &s);
   from_text_istream<T> ftis;
   ftis.set_format(format);
   T t;
   ftis.read(&t, &istream);
   _pvt::throw_on_unused_from_str_chars(istream);
   return _std::move(t);
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_FROM_STR_HXX
