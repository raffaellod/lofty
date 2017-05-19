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

//! Helper to reduce the amount of templated code generated for each from_str() call.
class LOFTY_SYM from_str_helper {
private:
   //! Stores members of types not defined at this point.
   struct impl;

public:
   //! Default constructor.
   from_str_helper();

   //! Destructor.
   ~from_str_helper();

   /*! Parses a string into an ere_capture_format instance.

   @param format_expr
      Type-specific format expression.
   @return
      Formatting options.
   */
   text::parsers::ere_capture_format const & parse_format_expr(str const & format_expr);

   /*! Runs the parser, and returns the match containing the read object, if matched, or throws an exception
   if not.

   @param src
      Source string.
   @param t_first_state
      Pointer to the first state accepting the type to parse.
   @return
      Reference to the capture containing input matched by *t_first_state.
   */
   text::parsers::dynamic_match_capture const & parse_src(
      str const & src, text::parsers::dynamic_state const * t_first_state
   );

private:
   //! Pointer to members of complex types that would require additional files to be #included.
   _std::unique_ptr<impl> pimpl;

public:
   //! Pointer to the parser in *pimpl. Having it here avoids one extra function call in from_str() to get it.
   text::parsers::dynamic * const parser;
};

}} //namespace lofty::_pvt

namespace lofty {

/*! Returns an object constructed from its string representation, optionally with a custom format.

TODO: needs an overload that allows to specify a full ere_capture_format instance, probably in a more
convenient syntax than just taking ere_capture_format const & .

@param s
   String to reconstruct into an object.
@param format_expr
   Type-specific format expression.
@return
   Object reconstructed from s according to format.
*/
template <typename T>
inline T from_str(str const & src, str const & format_expr = str::empty) {
   from_text_istream<T> ftis;
   _pvt::from_str_helper helper;
   T ret;
   ftis.convert_capture(helper.parse_src(src, ftis.format_to_parser_states(
      helper.parse_format_expr(format_expr), helper.parser
   )), &ret);
   return _std::move(ret);
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_FROM_STR_HXX
