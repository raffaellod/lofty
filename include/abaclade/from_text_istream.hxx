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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*! Throws an abc::text::syntax_error if the first argument, referencing the end of a format string
parsed by an abc::from_text_istream or abc::to_text_ostream specialization, does not equal the end
of the format string.

@param itFormatConsumedEnd
   Iterator to the end of the consumed/parsed portion of the format string.
@param sFormat
   Format string.
*/
ABACLADE_SYM void throw_on_unused_streaming_format_chars(
   str::const_iterator const & itFormatConsumedEnd, str const & sFormat
);

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _pvt {

/*! Defines a member named value that is true if
“void T::from_text_istream(io::text::istream * ptis)” is declared, or false otherwise. */
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

}} //namespace abc::_pvt

namespace abc {

/*! Reads and parses a string representation of an object of type T, according to an optional format
string. Once constructed with the desired format specification, an instance must be able to convert
into T instances any number of strings.

The default implementation assumes that a public T member with signature
“void T::from_text_istream(io::text::istream * ptis)” is declared, and offers no support for a
format string.

This class template and its specializations are at the core of abc::from_str() and
abc::io::text::istream::scan(). */
template <typename T>
class from_text_istream {
public:
   static_assert(
      _pvt::has_from_text_istream_member<T>::value,
      "specialization abc::from_text_istream<T> must be provided, " \
      "or public “void T::from_text_istream(abc::io::text::istream * ptis)” must be declared"
   );

   /*! Changes the input format.

   @param sFormat
      Formatting options.
   */
   void set_format(str const & sFormat) {
      // No format expected/allowed.
      throw_on_unused_streaming_format_chars(sFormat.cbegin(), sFormat);
   }

   /*! Sets a T instance from its string representation.

   @param t
      T instance to read.
   @param ptis
      Pointer to the stream to read from.
   */
   void read(T * t, io::text::istream * ptis) {
      t->from_text_istream(ptis);
   }
};

} //namespace abc
