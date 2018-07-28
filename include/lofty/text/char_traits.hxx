/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_TEXT_CHAR_TRAITS_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_TEXT_CHAR_TRAITS_HXX
#endif

#ifndef _LOFTY_TEXT_CHAR_TRAITS_HXX_NOPUB
#define _LOFTY_TEXT_CHAR_TRAITS_HXX_NOPUB

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {
_LOFTY_PUBNS_BEGIN

/*! UTF-8 character traits (constants and functions). Note that this class is not modeled after
std::char_traits. */
class LOFTY_SYM utf8_char_traits {
public:
   /*! Max length of a code point, in UTF-8 characters (bytes). Technically, 6 is also possible due to the way
   bits are encoded, but it’s illegal. */
   static unsigned const max_codepoint_length = 4;
   //! Highest code point that can be encoded in a single UTF-8 character.
   static char32_t const max_single_char_codepoint = 0x7f;

public:
   /*! Converts a char8_t array into a code point (UTF-32 character).

   @param src_begin
      Start of the character array to decode.
   @return
      Decoded code point.
   */
   static char32_t chars_to_codepoint(char8_t const * src_begin);

   /*! Converts a code point (UTF-32 character) into a char8_t array.

   @param cp
      Code point to be encoded.
   @param dst_begin
      Start of the character array that will receive the encoded version of cp.
   @return
      Pointer to the character beyond the last one used in *dst_begin.
   */
   static char8_t * codepoint_to_chars(char32_t cp, char8_t * dst_begin);

   /*! Return the number of characters needed to convert the specified code point into UTF-8 characters.

   @param cp
      Source code point.
   @return
      Length of the resulting character sequence.
   */
   static unsigned codepoint_size(char32_t cp);

   /*! Returns the sequence indicator bit mask suitable to precede a continuation of cont_byte_size bytes.

   @param cont_byte_size
      Length of the sequence, in bytes.
   @return
      Sequence indicator bit mask.
   */
   static /*constexpr*/ char8_t cont_length_to_seq_indicator(unsigned cont_byte_size) {
      // 0x3f00 will produce 0x00 (when >> 0), 0xc0 (2), 0xe0 (3), 0xf0 (4).
      return static_cast<char8_t>(0x3f00 >> bit_shift_masks[cont_byte_size]);
   }

   /*! Returns true if the specified character is a trail (non-lead) character.

   @param ch
      UTF-8 character.
   @return
      true if ch is a trail character, or false if it’s a lead character.
   */
   static /*constexpr*/ bool is_trail_char(char8_t ch) {
      return (ch & 0xc0) == 0x80;
   }

   /*! Returns the bits in a lead byte that are part of the encoded code point. Notice that the bits will need
   to be shifted in the right position to form a valid UTF-32 character.

   @param ch
      First byte of an UTF-8 code point.
   @param cont_byte_size
      Length of the remainder of the UTF-8 byte sequence, in bytes.
   @return
      Bits in ch that participate in the code point.
   */
   static /*constexpr*/ char32_t get_lead_char_codepoint_bits(char8_t ch, unsigned cont_byte_size) {
      return static_cast<char32_t>(ch & (0x7f >> bit_shift_masks[cont_byte_size]));
   }

   /*! Checks if a character is a valid UTF-8 lead character.

   @param ch
      Character to validate.
   @return
      true if ch is a valid UTF-8 lead character, or false otherwise.
   */
   static bool is_valid_lead_char(char8_t ch) {
      std::uint8_t i = static_cast<std::uint8_t>(ch);
      return (valid_lead_chars_mask[i >> 3] & (0x80u >> (i & 0x07u))) != 0;
   }

   /*! Returns the run length of an UTF-8 sequence, given its lead byte.

   @param ch
      First byte of an UTF-8 code point.
   @return
      Length of the code point sequence, or 1 if the character is not a lead byte, i.e. it’s a code point
      encoded as a single byte or an invalid sequence.
   */
   static /*constexpr*/ unsigned lead_char_to_codepoint_size(char8_t ch) {
      unsigned i = static_cast<std::uint8_t>(ch);
      // See comments on cp_sizes_by_lead_char in char_traits.cxx to understand this way of accessing it.
      return static_cast<unsigned>(
      // (cp_sizes_by_lead_char[byte index] >> [nibble index → 0 or 4]) & nibble mask
         (cp_sizes_by_lead_char[  i >> 2  ] >> (    (i & 2) << 1     )) & 0xf
      );
   }

private:
   //! Maps each UTF-8 lead byte to the length of its entire encoded code point.
   static std::uint8_t const cp_sizes_by_lead_char[];
   /*! Shift counts for the mask 0x7f to be applied to each lead byte to get the bits actually part of the
   code point; indexed by the number of bytes in the sequence. */
   static std::uint8_t const bit_shift_masks[];
   //! A set bit in this array means that the corresponding character is a valid UTF-8 lead character.
   static std::uint8_t const valid_lead_chars_mask[];
};

_LOFTY_PUBNS_END
}} //namespace lofty::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {
_LOFTY_PUBNS_BEGIN

/*! UTF-16 character traits (constants and functions). Note that this class is not modeled after
std::char_traits. */
class LOFTY_SYM utf16_char_traits {
public:
   //! Max length of a code point, in UTF-16 characters.
   static unsigned const max_codepoint_length = 2;
   //! Highest code point that can be encoded in a single UTF-16 character.
   static char32_t const max_single_char_codepoint = 0xffff;

public:
   /*! Converts a char16_t array into a code point (UTF-32 character).

   @param src_begin
      Start of the character array to decode.
   @return
      Decoded code point.
   */
   static char32_t chars_to_codepoint(char16_t const * src_begin);

   /*! Return the number of characters needed to convert the specified code point into UTF-16 characters.

   @param cp
      Source code point.
   @return
      Length of the resulting character sequence.
   */
   static unsigned codepoint_size(char32_t cp);

   /*! Converts a code point (UTF-32 character) into a char16_t array.

   @param cp
      Code point to be encoded.
   @param dst_begin
      Start of the character array that will receive the encoded version of cp.
   @return
      Pointer to the character beyond the last one used in *dst_begin.
   */
   static char16_t * codepoint_to_chars(char32_t cp, char16_t * dst_begin);

   /*! Returns true if the specified character is a surrogate lead.

   @param ch
      UTF-16 character.
   @return
      true if ch is a lead surrogate, or false if it’s a trail surrogate.
   */
   static /*constexpr*/ bool is_lead_surrogate(char16_t ch) {
      return (ch & 0xfc00) == 0xd800;
   }

   /*! Returns true if the specified character is a surrogate (lead or trail).

   @param ch
      UTF-16 character.
   @return
      true if ch is a surrogate, or false otherwise.
   */
   static /*constexpr*/ bool is_surrogate(char16_t ch) {
      return (ch & 0xf800) == 0xd800;
   }

   //! See utf8_char_traits::is_trail_char().
   static /*constexpr*/ bool is_trail_char(char16_t ch) {
      return (ch & 0xfc00) == 0xdc00;
   }

   //! See utf8_char_traits::lead_char_to_codepoint_size().
   static /*constexpr*/ unsigned lead_char_to_codepoint_size(char16_t ch) {
      return is_lead_surrogate(ch) ? 2u : 1u;
   }
};

_LOFTY_PUBNS_END
}} //namespace lofty::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {
_LOFTY_PUBNS_BEGIN

/*! String traits for the host character type, lofty::text::char_t. Derives from either utf8_char_traits or
utf16_char_traits. */
class LOFTY_SYM host_char_traits :
#if LOFTY_HOST_UTF == 8
   public utf8_char_traits {
#elif LOFTY_HOST_UTF == 16
   public utf16_char_traits {
#endif
public:

#if LOFTY_HOST_UTF == 8
   typedef utf8_char_traits traits_base;
#elif LOFTY_HOST_UTF == 16
   typedef utf16_char_traits traits_base;
#endif

public:
   /*! Converts a code point (UTF-32 character) into a char_t array.

   @param cp
      Code point to be encoded.
   @param dst
      Character array that will receive the encoded version of cp.
   @return
      Pointer to the character beyond the last one used in dst.
   */
   static char_t * codepoint_to_chars(char32_t cp, char_t (& dst)[max_codepoint_length]) {
      return traits_base::codepoint_to_chars(cp, dst);
   }
};

_LOFTY_PUBNS_END
}} //namespace lofty::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TEXT_CHAR_TRAITS_HXX_NOPUB

#ifdef _LOFTY_TEXT_CHAR_TRAITS_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace text {

   using _pub::host_char_traits;
   using _pub::utf8_char_traits;
   using _pub::utf16_char_traits;

   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_TEXT_CHAR_TRAITS_HXX
