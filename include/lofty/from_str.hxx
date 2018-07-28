/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_FROM_STR_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_FROM_STR_HXX
#endif

#ifndef _LOFTY_FROM_STR_HXX_NOPUB
#define _LOFTY_FROM_STR_HXX_NOPUB

#include <lofty/from_text_istream.hxx>
#include <lofty/_std/memory.hxx>
#include <lofty/_std/utility.hxx>
#include <lofty/text/str-0.hxx>

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

   /*! Parses a string into an regex_capture_format instance.

   @param format_expr
      Type-specific format expression.
   @return
      Formatting options.
   */
   text::parsers::_LOFTY_PUBNS regex_capture_format const & parse_format_expr(
      text::_LOFTY_PUBNS str const & format_expr
   );

   /*! Runs the parser, and returns the match containing the read object, if matched, or throws an exception
   if not.

   @param src
      Source string.
   @param t_first_state
      Pointer to the first state accepting the type to parse.
   @return
      Reference to the capture containing input matched by *t_first_state.
   */
   text::parsers::_LOFTY_PUBNS dynamic_match_capture const & parse_src(
      text::_LOFTY_PUBNS str const & src, text::parsers::_LOFTY_PUBNS dynamic_state const * t_first_state
   );

private:
   //! Pointer to members of complex types that would require additional files to be #included.
   _std::_LOFTY_PUBNS unique_ptr<impl> pimpl;

public:
   //! Pointer to the parser in *pimpl. Having it here avoids one extra function call in from_str() to get it.
   text::parsers::_LOFTY_PUBNS dynamic * const parser;
};

}} //namespace lofty::_pvt

namespace lofty {
_LOFTY_PUBNS_BEGIN

/*! Returns an object constructed from its string representation, optionally with a custom format.

TODO: needs an overload that allows to specify a full regex_capture_format instance, probably in a more
convenient syntax than just taking regex_capture_format const & .

@param s
   String to reconstruct into an object.
@param format_expr
   Type-specific format expression.
@return
   Object reconstructed from s according to format.
*/
template <typename T>
inline T from_str(
   text::_LOFTY_PUBNS str const & src,
   text::_LOFTY_PUBNS str const & format_expr = text::_LOFTY_PUBNS str::empty
) {
   from_text_istream<T> ftis;
   _pvt::from_str_helper helper;
   T ret;
   ftis.convert_capture(helper.parse_src(src, ftis.format_to_parser_states(
      helper.parse_format_expr(format_expr), helper.parser
   )), &ret);
   return _std::_pub::move(ret);
}

_LOFTY_PUBNS_END
} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_FROM_STR_HXX_NOPUB

#ifdef _LOFTY_FROM_STR_HXX
   #undef _LOFTY_NOPUB

   namespace lofty {

   using _pub::from_str;

   }

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_FROM_STR_HXX
