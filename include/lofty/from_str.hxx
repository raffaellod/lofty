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

#include <lofty/text/parsers/dynamic.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

/*! Throws a lofty::text::syntax_error after a call to lofty::from_str() failed to perform the requested
conversion.

@param src
   Source string that could not be converted.
*/
LOFTY_SYM void LOFTY_FUNC_NORETURN throw_after_from_str_parse_failed(str const & src);

}} //namespace lofty::_pvt

namespace lofty {

/*! Returns an object constructed from its string representation, optionally with a custom format.

TODO: needs an overload that allows to specify a full from_text_istream_format instance, probably in a more
convenient syntax than just taking from_text_istream_format const & .

@param s
   String to reconstruct into an object.
@param format_expr
   Type-specific format string expression.
@return
   Object reconstructed from s according to format.
*/
template <typename T>
inline T from_str(str const & src, str format_expr = str::empty) {
   // TODO: way too much templated code in here; please de-template most of it!
   text::parsers::dynamic parser;
   from_text_istream<T> ftis;
   from_text_istream_format format;
   format.expr = _std::move(format_expr);
   auto format_first = ftis.format_to_parser_states(format, &parser);
   auto end = parser.create_end_state();
   auto capture1 = parser.create_capture_group(format_first);
   capture1->set_next(end);
   auto begin = parser.create_begin_state();
   begin->set_next(capture1);
   parser.set_initial_state(begin);

   auto match(parser.run(src));
   if (!match) {
      _pvt::throw_after_from_str_parse_failed(src);
   }
   T ret;
   ftis.convert_capture(match.capture_group(0), &ret);
   return _std::move(ret);
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_FROM_STR_HXX
