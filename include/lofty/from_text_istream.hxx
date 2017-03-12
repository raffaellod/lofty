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

namespace lofty { namespace _pvt {

/*! Defines a member named value that is true if “void T::from_text_istream(io::text::istream * istream)” is
declared, or false otherwise. */
template <typename T>
struct has_from_text_istream_member {
   template <typename U, void (U::*)(io::text::istream *)>
   struct member_test {};
   template <typename U>
   static long test(member_test<U, &U::from_text_istream> *);
   template <typename>
   static short test(...);

   static bool const value = (sizeof(test<T>(nullptr)) == sizeof(long));
};

}} //namespace lofty::_pvt

namespace lofty {

/*! Reads and parses a string representation of an object of type T, according to an optional format string.
Once constructed with the desired format specification, an instance must be able to convert into T instances
any number of strings.

The default implementation assumes that a public T member with signature 
“void T::from_text_istream(io::text::istream * istream)” is declared, and offers no support for a format
string.

This class template and its specializations are at the core of lofty::from_str() and 
lofty::io::text::istream::scan(). */
template <typename T>
class from_text_istream {
public:
   static_assert(
      _pvt::has_from_text_istream_member<T>::value,
      "specialization lofty::from_text_istream<T> must be provided, or public " \
      "“void T::from_text_istream(lofty::io::text::istream * istream)” must be declared"
   );

   /*! Changes the input format.

   @param format
      Formatting options.
   */
   void set_format(str const & format) {
      // No format expected/allowed.
      throw_on_unused_streaming_format_chars(format.cbegin(), format);
   }

   /*! Sets a T instance from its string representation.

   @param dst
      Pointer to the T instance to read into.
   @param src
      Pointer to the stream to read from.
   */
   void read(T * dst, io::text::istream * src) {
      dst->from_text_istream(src);
   }
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Specialization for bool.
template <>
class LOFTY_SYM from_text_istream<bool> {
public:
   //! Default constructor.
   from_text_istream();

   /*! Changes the input format.

   @param format
      Formatting options.
   */
   void set_format(str const & format);

   /*! Converts string into a boolean value.

   @param dst
      Boolean value to read into.
   @param src
      Pointer to the stream to read from.
   */
   void read(bool * dst, io::text::istream * src);

protected:
   //! String that will be translated to true.
   str true_str;
   //! String that will be translated to false.
   str false_str;
};

} //namespace lofty
