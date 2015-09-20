/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015 Raffaello D. Di Napoli

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

namespace abc { namespace text {

/*! Recognized text encodings. Little endians should be listed before big endians; some code relies
on this. */
ABC_ENUM(encoding,
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
   (utf16_host,   (ABC_HOST_LITTLE_ENDIAN ? utf16le : utf16be)),
   //! UTF-32 encoding (host endianness).
   (utf32_host,   (ABC_HOST_LITTLE_ENDIAN ? utf32le : utf32be)),
   //! Default host encoding.
   (host,         (ABC_HOST_UTF == 16 ? utf16_host : utf8))
);

//! Recognized line terminators.
ABC_ENUM(line_terminator,
   /*! In the context of a text I/O, accept as line ending any line terminator read, or write LF
   characters as the host line terminator. */
   (any,               0),
   /*! In the context of a text I/O, read any line terminator as single LF, or write LF characters
   as the host line terminator. */
   (convert_any_to_lf, 1),
   //! Old Mac style: Carriage Return, '\r'.
   (cr,                2),
   //! Unix/POSIX style: Line Feed, '\n'.
   (lf,                3),
   //! DOS/Windows style: Carriage Return + Line Feed, '\r', '\n'.
   (cr_lf,             4),
   //! Default host line terminator.
   (host,              (ABC_HOST_API_WIN32 ? cr_lf : lf))
);

/*! Casts a single character into a code point.

@param ch
   Character.
@return
   Equivalent code point.
*/
inline char32_t codepoint(char_t ch) {
#if ABC_HOST_UTF == 8
   return static_cast<char32_t>(static_cast<std::uint8_t>(ch));
#elif ABC_HOST_UTF == 16
   return static_cast<char32_t>(ch);
#endif
}

#if ABC_HOST_UTF > 8
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

@param psz
   Pointer to the NUL-terminated string of which to calculate the length.
@return
   Length of the string pointed to by psz, in characters.
*/
ABACLADE_SYM std::size_t size_in_chars(char_t const * psz);
#if ABC_HOST_UTF > 8
ABACLADE_SYM std::size_t size_in_chars(char const * psz);
#endif

}} //namespace abc::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text { namespace detail {

//! Data-only implementation of abc::text::file_address.
struct file_address_data {
public:
   /*! Returns the file path.

   @return
      File path.
   */
   char_t const * file_path() const {
      return m_pszFilePath;
   }

   /*! Returns the line number.

   @return
      Line number.
   */
   unsigned line_number() const {
      return m_iLine;
   }

public:
   //! Path to the source file.
   char_t const * m_pszFilePath;
   //! Line number in m_pszFilePath.
   unsigned m_iLine;
};

}}} //namespace abc::text::detail

namespace abc { namespace text {

//! Address in a text file, expressed as the file path and a line number within it.
class file_address : protected detail::file_address_data {
public:
   //! Default constructor.
   file_address() {
      m_pszFilePath = nullptr;
      m_iLine = 0;
   }

   /*! Constructor.

   @param pszFilePath
      Path to the source file.
   @param iLine
      Line number in *pszFilePath.
   */
   file_address(char_t const * pszFilePath, unsigned iLine) {
      m_pszFilePath = pszFilePath;
      m_iLine = iLine;
   }

   /*! Returns a pointer to the contained data-only struct.

   @return
      Pointer to the contained detail::file_address_data.
   */
   detail::file_address_data const * data() const {
      return this;
   }

   /*! Returns the file path.

   @return
      File path.
   */
   char_t const * file_path() const {
      return m_pszFilePath;
   }

   /*! Returns a pointer to an instance of this class from a pointer to the data-only struct.

   @param pfad
      Pointer to a data-only struct.
   @return
      Pointer to the equivalent file_address instance.
   */
   static file_address const * from_data(detail::file_address_data const * pfad) {
      return static_cast<file_address const *>(pfad);
   }

   /*! Returns the line number.

   @return
      Line number.
   */
   unsigned line_number() const {
      return m_iLine;
   }
};

}} //namespace abc::text
