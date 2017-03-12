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

#ifndef _LOFTY_TEXT_HXX
#define _LOFTY_TEXT_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/collections/vector.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Contains classes and functions to work with Unicode text strings and characters.
namespace text {}

} //namespace lofty

namespace lofty { namespace text {

//! This should be used to replace any invalid char32_t value.
static char32_t const replacement_char = 0x00fffd;

/*! Returns the character size, in bytes, for the specified charset encoding, or 0 for non-charset encodings
(e.g. identity_encoding).

@param enc
   Desired encoding.
@return
   Size of a character (not a code point, which can require more than one character) for the specified
   encoding, in bytes.
*/
LOFTY_SYM std::size_t get_encoding_size(encoding enc);

/*! Returns a line terminator string corresponding to the specified line_terminator value, or the host default
if lterm is line_terminator::any.

@param lterm
   Desired line terminator.
@return
   String with the requested line terminator sequence.
*/
LOFTY_SYM str get_line_terminator_str(line_terminator lterm);

/*! Tries to guess the encoding of a sequence of bytes, optionally also taking into account the total number
of bytes in the source of which the buffer is the beginning.

While this function can check for validity of some encodings, it does not guarantee that, for example, for a
return value of utf8_encoding str_traits::validate() will return true for the same buffer.
TODO: why not guarantee validity? It would help weed out more encodings with fewer bytes.

@param buf_begin
   Pointer to the beginning of the buffer to scan for encoding clues.
@param buf_end
   Pointer to the end of the buffer.
@param src_total_bytes
   Total size, in bytes, of a larger string of which *buf_begin is the beginning.
@param bom_byte_size
   Pointer to a variable that will receive the size of the Byte Order Mark if found at the beginning of the
   string, in bytes, or 0 otherwise.
@return
   Detected encoding of the string pointed to by buf_begin.
*/
LOFTY_SYM encoding guess_encoding(
   void const * buf_begin, void const * buf_end, std::size_t src_total_bytes = 0,
   std::size_t * bom_byte_size = nullptr
);

/*! Tries to guess the line terminator sequence employed in a string.

@param chars_begin
   Pointer to the first character of the string to scan for a line terminator sequence.
@param chars_end
   Pointer to beyond the last character of the string.
@return
   Detected line terminator sequence, or line_terminator::any if the source buffer did not include any known
   line terminator sequence..
*/
LOFTY_SYM line_terminator guess_line_terminator(char_t const * chars_begin, char_t const * chars_end);

/*! Checks if a UTF-32 character is a valid Unicode code point, which means that its ordinal value must be
included in the interval [0, U+10FFFF] (see Unicode Standard 6.2 § 2.4 “Code Points and Characters”).

@param cp
   Code point to validate.
@return
   true if the character is a valid code point, or false otherwise.
*/
inline /*constexpr*/ bool is_codepoint_valid(char32_t cp) {
   return cp <= 0x10ffff;
}

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

/*! Converts from one character encoding to another, validating the source as it’s processed.

Call this function omitting the last two arguments (dst and dst_byte_size_max) to have returned the calculated
size of the buffer necessary to hold the converted characters.

After allocating a buffer of the requested size, call this function again with the same arguments (but with
valid dst and dst_byte_size_max) to perform the transcoding; all the variables pointed to by the pointer
arguments will be updated to discard the bytes used in the conversion; otherwise no pointed-to variables will
be written to.

@param throw_on_errors
   On decoding, if true, an exception of type lofty::text::decode_error will be thrown if any invalid
   characters are found; otherwise invalid characters will be silently replaced with
   lofty::text::replacement_char.
   On encoding, if true, an exception of type lofty::text::encode_error will be thrown if any code points
   cannot be converted to the destination encoding; otherwise characters that cannot be encoded will be
   replaced with an encoding-specific replacement character.
@param src_enc
   Encoding of the string pointed to by *src.
@param src
   Pointer to a pointer to the source string; the pointed-to pointer will be incremented as characters are
   transcoded.
@param src_byte_size
   Pointer to a variable that holds the size of the string pointed to by *src, and that will be decremented by
   the number of source bytes transcoded.
@param dst_enc
   Encoding of the string pointed to by *dst.
@param dst
   Pointer to a pointer to the destination buffer; the pointed-to pointer will be incremented as characters
   are stored in the buffer. Passing nullptr is safe and nothing will be written to it, but all the other
   arguments will be updated regardless.
@param dst_byte_size_max
   Pointer to a variable that holds the size of the buffer pointed to by *dst, and that will be decremented by
   the number of bytes stored in the buffer (or that would be stored, if dst is nullptr). If nullptr is passed
   no writes will be attempted to any of the arguments, but the return value will be correct.
@return
   Used destination buffer size, in bytes.
*/
LOFTY_SYM std::size_t transcode(
   bool throw_on_errors,
   encoding src_enc, void const ** src,           std::size_t * src_byte_size,
   encoding dst_enc, void       ** dst = nullptr, std::size_t * dst_byte_size_max = nullptr
);

}} //namespace lofty::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <>
class LOFTY_SYM to_text_ostream<text::file_address> {
public:
   /*! Changes the output format.

   @param format
      Formatting options.
   */
   void set_format(str const & format);

   /*! Writes a source location, applying the formatting options.

   @param src
      Source location to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(text::file_address const & src, io::text::ostream * dst);
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {

//! A text encoding or decoding error occurred.
class LOFTY_SYM error : public generic_error {
public:
   //! Default constructor.
   explicit error(errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   error(error const & src);

   //! Destructor.
   virtual ~error() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   error & operator=(error const & src);
};

}} //namespace lofty::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {

//! A text decoding error occurred.
class LOFTY_SYM decode_error : public error {
public:
   /*! Constructor.

   @param description
      Description of the encountered problem.
   @param invalid_bytes_begin
      Pointer to the start of the byte sequence that caused the error.
   @param invalid_bytes_end
      Pointer to the end of the byte sequence that caused the error.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit decode_error(
      str const & description = str::empty, std::uint8_t const * invalid_bytes_begin = nullptr,
      std::uint8_t const * invalid_bytes_end = nullptr, errint_t err = 0
   );

   /*! Copy constructor.

   @param src
      Source object.
   */
   decode_error(decode_error const & src);

   //! Destructor.
   virtual ~decode_error() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   decode_error & operator=(decode_error const & src);

private:
   //! Description of the encountered problem.
   str description;
   //! Bytes that caused the error.
   collections::vector<std::uint8_t, 16> invalid_bytes;
};

}} //namespace lofty::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {

//! A text encoding error occurred.
class LOFTY_SYM encode_error : public error {
public:
   /*! Default constructor.

   @param description
      Description of the encountered problem.
   @param invalid_cp
      Code point that caused the error.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit encode_error(
      str const & description = str::empty, char32_t invalid_cp = 0xffffff, errint_t err = 0
   );

   /*! Copy constructor.

   @param src
      Source object.
   */
   encode_error(encode_error const & src);

   //! Destructor.
   virtual ~encode_error() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   encode_error & operator=(encode_error const & src);

private:
   //! Description of the encountered problem.
   str description;
   /*! Code point that caused the error. Not a char32_t because if there’s anything wrong with it, we don’t
   want to find out when trying to print it in write_extended_info(). */
   std::uint32_t invalid_cp;
};

}} //namespace lofty::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {

//! The syntax for the specified expression is invalid.
class LOFTY_SYM syntax_error : public generic_error {
public:
   /*! Constructor.

   Most arguments are optional, and can be specified leaving defaulted gaps in between; the resulting
   exception message will not contain omitted arguments.

   The order of line and character is inverted, so that this single overload can be used to differentiate
   between cases in which source is the single line containing the failing expression (the thrower would not
   pass line_number) and cases where source is the source file containing the error (the thrower would pass
   the non-zero line number).

   Examples:

      syntax_error(LOFTY_SL("expression cannot be empty"))
      syntax_error(LOFTY_SL("unmatched '{'"), expr, char_index)
      syntax_error(LOFTY_SL("expected expression"), str::empty, char_index, line_number)
      syntax_error(LOFTY_SL("unexpected end of file"), source_path, char_index, line_number)

   @param description
      Description of the syntax error.
   @param source
      Source of the syntax error (whole or individual line).
   @param char_index
      Character at which the error is located.
   @param line_number
      Line where the error is located.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit syntax_error(
      str const & description, str const & source = str::empty, unsigned char_index = 0,
      unsigned line_number = 0, errint_t err = 0
   );

   /*! Copy constructor.

   @param src
      Source object.
   */
   syntax_error(syntax_error const & src);

   //! Destructor.
   virtual ~syntax_error() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   syntax_error & operator=(syntax_error const & src);

private:
   //! Description of the syntax error.
   str description;
   //! Source of the syntax error (whole or individual line).
   str source;
   //! Character at which the error is located.
   unsigned char_index;
   //! Line where the error is located.
   unsigned line_number;
};

}} //namespace lofty::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TEXT_HXX
