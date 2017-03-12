/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

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

namespace lofty { namespace text {

//! Recognized text encodings. Little endians should be listed before big endians; some code relies on this.
LOFTY_ENUM(encoding,
   //! Unknown/undetermined encoding.
   (unknown,      0),
   //! UTF-8 encoding.
   (utf8,         1),
   //! UTF-16 Little Endian encoding.
   (utf16le,      2),
   //! UTF-16 Big Endian encoding.
   (utf16be,      3),
   //! UTF-32 Little Endian encoding.
   (utf32le,      4),
   //! UTF-32 Big Endian encoding.
   (utf32be,      5),
   //! ISO-8859-1 encoding.
   (iso_8859_1,   6),
   //! Windows-1252 encoding.
   (windows_1252, 7),
   //! UTF-16 encoding (host endianness).
   (utf16_host,   (LOFTY_HOST_LITTLE_ENDIAN ? utf16le : utf16be)),
   //! UTF-32 encoding (host endianness).
   (utf32_host,   (LOFTY_HOST_LITTLE_ENDIAN ? utf32le : utf32be)),
   //! Default host encoding.
   (host,         (LOFTY_HOST_UTF == 16 ? utf16_host : utf8))
);

//! Recognized line terminators.
LOFTY_ENUM(line_terminator,
   /*! In the context of a text stream, accept as line ending any line terminator read, or write LF characters
   as the host line terminator. */
   (any,               0),
   //! Old Mac style: Carriage Return, '\r'.
   (cr,                1),
   //! Unix/POSIX style: Line Feed, '\n'.
   (lf,                2),
   //! DOS/Windows style: Carriage Return + Line Feed, '\r', '\n'.
   (cr_lf,             3),
   //! Default host line terminator.
   (host,              (LOFTY_HOST_API_WIN32 ? cr_lf : lf))
);

/*! Casts a single character into a code point.

@param ch
   Character.
@return
   Equivalent code point.
*/
inline char32_t codepoint(char_t ch) {
#if LOFTY_HOST_UTF == 8
   return static_cast<char32_t>(static_cast<std::uint8_t>(ch));
#elif LOFTY_HOST_UTF == 16
   return static_cast<char32_t>(ch);
#endif
}

#if LOFTY_HOST_UTF > 8
/*! Casts a single character into a character of the largest native size for the host.

@param ch
   Character.
@return
   Equivalent host character.
*/
inline char_t host_char(char ch) {
   return static_cast<char_t>(static_cast<std::uint8_t>(ch));
}
#endif

/*! Calculates the length of a NUL-terminated string, in characters.

@param s
   Pointer to the NUL-terminated string of which to calculate the length.
@return
   Length of the string pointed to by s, in characters.
*/
LOFTY_SYM std::size_t size_in_chars(char_t const * s);
#if LOFTY_HOST_UTF > 8
LOFTY_SYM std::size_t size_in_chars(char const * s);
#endif

}} //namespace lofty::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text { namespace _pvt {

//! Data-only implementation of lofty::text::file_address.
struct file_address_data {
public:
   /*! Returns the file path.

   @return
      File path.
   */
   char_t const * file_path() const {
      return file_path_;
   }

   /*! Returns the line number.

   @return
      Line number.
   */
   unsigned line_number() const {
      return line_number_;
   }

public:
   //! Path to the source file.
   char_t const * file_path_;
   //! Line number in file_path_.
   unsigned line_number_;
};

}}} //namespace lofty::text::_pvt

namespace lofty { namespace text {

//! Address in a text file, expressed as the file path and a line number within it.
class file_address : protected _pvt::file_address_data {
public:
   //! Default constructor.
   file_address() {
      file_path_ = nullptr;
      line_number_ = 0;
   }

   /*! Constructor.

   @param file_path__
      Path to the source file.
   @param line_number__
      Line number in *file_path__.
   */
   file_address(char_t const * file_path__, unsigned line_number__) {
      file_path_ = file_path__;
      line_number_ = line_number__;
   }

   /*! Returns a pointer to the contained data-only struct.

   @return
      Pointer to the contained _pvt::file_address_data.
   */
   _pvt::file_address_data const * data() const {
      return this;
   }

   /*! Returns the file path.

   @return
      File path.
   */
   char_t const * file_path() const {
      return file_path_;
   }

   /*! Returns a pointer to an instance of this class from a pointer to the data-only struct.

   @param file_addr_data
      Pointer to a data-only struct.
   @return
      Pointer to the equivalent file_address instance.
   */
   static file_address const * from_data(_pvt::file_address_data const * file_addr_data) {
      return static_cast<file_address const *>(file_addr_data);
   }

   /*! Returns the line number.

   @return
      Line number.
   */
   unsigned line_number() const {
      return line_number_;
   }
};

}} //namespace lofty::text
