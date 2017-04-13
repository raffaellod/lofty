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

#ifndef _LOFTY_HXX_INTERNAL
   #error "Please #include <lofty.hxx> instead of this file"
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! Throws a lofty::text::syntax_error if the first argument, referencing the end of a format string parsed by
a lofty::from_text_istream or lofty::to_text_ostream specialization, does not equal the end of the format
string.

@param format_consumed_end
   Iterator to the end of the consumed/parsed portion of the format string.
@param format
   Format string.
*/
LOFTY_SYM void throw_on_unused_streaming_format_chars(
   str::const_iterator const & format_consumed_end, str const & format
);

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! Reads and parses a string representation of an object of type T, according to an optional format string.
Once constructed with the desired format specification, an instance must be able to convert into T instances
any number of strings.

This class template and its specializations are at the core of lofty::from_str() and 
lofty::io::text::istream::scan(). */
template <typename T>
class from_text_istream;

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Specialization for bool.
template <>
class LOFTY_SYM from_text_istream<bool> {
public:
   //! Default constructor.
   from_text_istream();

   /*! Converts string into a boolean value.

   @param dst
      Boolean value to read into.
   @param capture0
      Pointer to the captured string.
   */
   void convert_capture(text::parsers::dynamic_match_capture const & capture0, bool * dst);

   /*! Changes the input format.

   @param format
      Formatting options.
   */
   text::parsers::dynamic_state const * format_to_parser_states(
      str const & format, text::parsers::dynamic * parser
   );

protected:
   //! String that will be translated to true.
   str true_str;
   //! String that will be translated to false.
   str false_str;
};

} //namespace lofty
