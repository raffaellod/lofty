/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2016 Raffaello D. Di Napoli

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

#ifndef _ABACLADE_TEXT_HXX
#define _ABACLADE_TEXT_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/collections/vector.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! Contains classes and functions to work with Unicode text strings and characters.
namespace text {}

} //namespace abc

namespace abc { namespace text {

//! This should be used to replace any invalid char32_t value.
static char32_t const replacement_char = 0x00fffd;

/*! Returns the character size, in bytes, for the specified charset encoding, or 0 for non-charset
encodings (e.g. identity_encoding).

@param enc
   Desired encoding.
@return
   Size of a character (not a code point, which can require more than one character) for the
   specified encoding, in bytes.
*/
ABACLADE_SYM std::size_t get_encoding_size(encoding enc);

/*! Returns a line terminator string corresponding to the specified line_terminator value, or the
host default if lterm is line_terminator::any.

@param lterm
   Desired line terminator.
@return
   String with the requested line terminator sequence.
*/
ABACLADE_SYM str get_line_terminator_str(line_terminator lterm);

/*! Tries to guess the encoding of a sequence of bytes, optionally also taking into account the
total number of bytes in the source of which the buffer is the beginning.

While this function can check for validity of some encodings, it does not guarantee that, for
example, for a return value of utf8_encoding str_traits::validate() will return true for the same
buffer.
TODO: why not guarantee validity? It would help weed out more encodings with fewer bytes.

@param pBufBegin
   Pointer to the beginning of the buffer to scan for encoding clues.
@param pBufEnd
   Pointer to the end of the buffer.
@param cbSrcTotal
   Total size, in bytes, of a larger string of which *pBuf is the beginning.
@param pcbBom
   Pointer to a variable that will receive the size of the Byte Order Mark if found at the beginning
   of the string, in bytes, or 0 otherwise.
@return
   Detected encoding of the string pointed to by pBuf.
*/
ABACLADE_SYM encoding guess_encoding(
   void const * pBufBegin, void const * pBufEnd, std::size_t cbSrcTotal = 0,
   std::size_t * pcbBom = nullptr
);

/*! Tries to guess the line terminator sequence employed in a string.

@param pchBegin
   Pointer to the first character of the string to scan for a line terminator sequence.
@param pchEnd
   Pointer to beyond the last character of the string.
@return
   Detected line terminator sequence, or line_terminator::any if the source buffer did not include
   any known line terminator sequence..
*/
ABACLADE_SYM line_terminator guess_line_terminator(char_t const * pchBegin, char_t const * pchEnd);

/*! Checks if a UTF-32 character is a valid Unicode code point, which means that its ordinal value
must be included in the interval [0, U+10FFFF] (see Unicode Standard 6.2 § 2.4 “Code Points and
Characters”).

@param cp
   Code point to validate.
@return
   true if the character is a valid code point, or false otherwise.
*/
inline /*constexpr*/ bool is_codepoint_valid(char32_t cp) {
   return cp <= 0x10ffff;
}

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

/*! Converts from one character encoding to another, validating the source as it’s processed.

Call this function omitting the last two arguments (ppDst and pcbDstMax) to have returned the
calculated size of the buffer necessary to hold the converted characters.

After allocating a buffer of the requested size, call this function again with the same arguments
(but with valid ppDst and pcbDstMax) to perform the transcoding; all the variables pointed to by the
pointer arguments will be updated to discard the bytes used in the conversion; otherwise no pointed-
to variables will be written to.

@param bThrowOnErrors
   On decoding, if true, an exception of type abc::text::decode_error will be thrown if any invalid
   characters are found; otherwise invalid characters will be silently replaced with
   abc::text::replacement_char.
   On encoding, if true, an exception of type abc::text::encode_error will be thrown if any code
   points cannot be converted to the destination encoding; otherwise characters that cannot be
   encoded will be replaced with an encoding-specific replacement character.
@param encSrc
   Encoding of the string pointed to by *ppSrc.
@param ppSrc
   Pointer to a pointer to the source string; the pointed-to pointer will be incremented as
   characters are transcoded.
@param pcbSrc
   Pointer to a variable that holds the size of the string pointed to by *ppSrc, and that will be
   decremented by the number of source bytes transcoded.
@param encDst
   Encoding of the string pointed to by *ppDst.
@param ppDst
   Pointer to a pointer to the destination buffer; the pointed-to pointer will be incremented as
   characters are stored in the buffer. Passing nullptr is safe and nothing will be written to it,
   but all the other arguments will be updated regardless.
@param pcbDstMax
   Pointer to a variable that holds the size of the buffer pointed to by *ppDst, and that will be
   decremented by the number of bytes stored in the buffer (or that would be stored, if ppDst is
   nullptr). If nullptr is passed no writes will be attempted to any of the arguments, but the
   return value will be correct.
@return
   Used destination buffer size, in bytes.
*/
ABACLADE_SYM std::size_t transcode(
   bool bThrowOnErrors,
   encoding encSrc, void const ** ppSrc, std::size_t * pcbSrc,
   encoding encDst, void       ** ppDst = nullptr, std::size_t * pcbDstMax = nullptr
);

}} //namespace abc::text

////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace abc {

template <>
class ABACLADE_SYM to_text_ostream<text::file_address> {
public:
   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(str const & sFormat);

   /*! Writes a source location, applying the formatting options.

   @param tfa
      Source location to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void write(text::file_address const & tfa, io::text::ostream * ptos);
};

} //namespace abc
//! @endcond

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text {

//! A text encoding or decoding error occurred.
class ABACLADE_SYM error : public generic_error {
public:
   //! Default constructor.
   explicit error(errint_t err = 0);

   /*! Copy constructor.

   @param x
      Source object.
   */
   error(error const & x);

   //! Destructor.
   virtual ~error() ABC_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   error & operator=(error const & x);
};

}} //namespace abc::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text {

//! A text decoding error occurred.
class ABACLADE_SYM decode_error : public error {
public:
   /*! Constructor.

   @param sDescription
      Description of the encountered problem.
   @param pbInvalidBegin
      Pointer to the start of the byte sequence that caused the error.
   @param pbInvalidEnd
      Pointer to the end of the byte sequence that caused the error.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit decode_error(
      str const & sDescription = str::empty, std::uint8_t const * pbInvalidBegin = nullptr,
      std::uint8_t const * pbInvalidEnd = nullptr, errint_t err = 0
   );

   /*! Copy constructor.

   @param x
      Source object.
   */
   decode_error(decode_error const & x);

   //! Destructor.
   virtual ~decode_error() ABC_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   decode_error & operator=(decode_error const & x);

private:
   //! Description of the encountered problem.
   str m_sDescription;
   //! Bytes that caused the error.
   collections::vector<std::uint8_t, 16> m_viInvalid;
};

}} //namespace abc::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text {

//! A text encoding error occurred.
class ABACLADE_SYM encode_error : public error {
public:
   /*! Default constructor.

   @param sDescription
      Description of the encountered problem.
   @param chInvalid
      Code point that caused the error.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit encode_error(
      str const & sDescription = str::empty, char32_t chInvalid = 0xffffff, errint_t err = 0
   );

   /*! Copy constructor.

   @param x
      Source object.
   */
   encode_error(encode_error const & x);

   //! Destructor.
   virtual ~encode_error() ABC_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   encode_error & operator=(encode_error const & x);

private:
   //! Description of the encountered problem.
   str m_sDescription;
   /*! Code point that caused the error. Not a char32_t because if there’s anything wrong with it,
   we don’t want to find out when trying to print it in write_extended_info(). */
   std::uint32_t m_iInvalidCodePoint;
};

}} //namespace abc::text

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_TEXT_HXX
