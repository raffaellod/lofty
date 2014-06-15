/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with Abaclade. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::utf8_traits


namespace abc {
namespace text {

/** UTF-8 character traits (constants and functions). Note that this class is not modeled after
std::char_traits.
*/
struct ABACLADE_SYM utf8_traits {
public:

   /** Max length of a code point, in encoded characters. Technically, 6 is also possible for UTF-8,
   due to the way bits are encoded, but it’s illegal. */
   static unsigned const max_codepoint_length = 4;


public:

   /** Returns the sequence indicator bit mask suitable to precede a continuation of cbCont bytes.

   cbCont
      Length of the sequence, in bytes.
   return
      Sequence indicator bit mask.
   */
   static /*constexpr*/ char8_t cont_length_to_seq_indicator(unsigned cbCont) {
      // 0x3f00 will produce 0x00 (when >> 0), 0xc0 (2), 0xe0 (3), 0xf0 (4).
      return char8_t(0x3f00 >> smc_acbitShiftMask[cbCont]);
   }


   /** Returns count of code points in a string.

   UTF validity: necessary.

   pchBegin
      Pointer to the beginning of the string.
   pchEnd
      Pointer beyond the end of the string.
   return
      Count of code points included in the string.
   */
   static size_t cp_len(char8_t const * pchBegin, char8_t const * pchEnd);


   /** Converts a UTF-32 character in this UTF representation.

   UTF validity: necessary.

   ch32
      UTF-32 character to be transcoded.
   pchDst
      Buffer to receive the transcoded version of ch32; *pchDst is assumed to be at least
      max_codepoint_length characters.
   return
      Count of characters written to the buffer pointed to by pchDst.
   */
   static unsigned from_utf32(char32_t ch32, char8_t * pchDst);


   /** Checks if a string is valid UTF.

   UTF validity: checked.

   psz
      Pointer to the NUL-terminated character array to validate.
   pchBegin
      Pointer to the first character of the string to validate.
   pchEnd
      Pointer to beyond the last character of the string to validate.
   return
      true if the string is valid UTF, or false otherwise.
   */
   static bool is_valid(char8_t const * psz);
   static bool is_valid(char8_t const * pchBegin, char8_t const * pchEnd);


   /** Returns the bits in a leading byte that are part of the encoded code point. Notice that the
   bits will need to be shifted in the right position to form a valid UTF-32 character.

   ch
      First byte of an UTF-8 code point.
   cbCont
      Length of the remainder of the UTF-8 byte sequence, in bytes.
   return
      Bits in ch that participate in the code point.
   */
   static /*constexpr*/ char32_t get_leading_cp_bits(char8_t ch, unsigned cbCont) {
      return char32_t(ch & (0x7f >> smc_acbitShiftMask[cbCont]));
   }


   /** Returns the continuation length (run length - 1) of an UTF-8 sequence, given its leading
   byte.

   ch
      First byte of an UTF-8 code point.
   return
      Length of the sequence continuation, or 0 if the character is not a leading byte, i.e. it’s a
      code point encoded as a single byte or an invalid sequence.
   */
   static /*constexpr*/ unsigned leading_to_cont_length(char8_t ch) {
      unsigned i(static_cast<uint8_t>(ch));
      // See comments on smc_acbConts in utf_traits.cxx to understand this way of accessing it.
      //             (smc_acbConts[byte index] >> [nibble index → 0 or 4]) & nibble mask
      return unsigned(smc_acbConts[  i >> 2  ] >> (    (i & 2) << 1     )) & 0xfu;
   }


   /** Returns the length, in UTF characters, of a NUL-terminated string.

   UTF validity: necessary.

   psz
      Pointer to the NUL-terminated string of which to calculate the length.
   return
      Length of the string pointed to by psz, in characters.
   */
   static size_t str_len(char8_t const * psz);


private:

   /** Maps each UTF-8 leading byte to the length of its continuation. */
   static uint8_t const smc_acbConts[];
   /** Shift counts for the mask 0x7f to be applied to each leading byte to get the bits actually
   part of the code point; indexed by the number of bytes in the sequence. */
   static uint8_t const smc_acbitShiftMask[];
   /** Bitmasks to be applied to the first trailing byte to check if a code point is using an
   overlong encoding. For example, even though 11100000 10100000 10000000 has all zeroes in the code
   point part of the lead byte (mask 1110xxxx), it cannot be encoded with fewer bytes because the
   second byte uses 6 bits and the 2-byte-long sequence lead byte only has 5 code point bits (mask
   110xxxxx); in this case the mask 00100000, applied to the second byte (10100000) allows to find
   out if a code point could have been encoded with fewer characters.
   The first element (index 0) is for 1-byte continuations. */
   static uint8_t const smc_aiOverlongDetectionMasks[];
};

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::utf16_traits


namespace abc {
namespace text {

/** UTF-16 character traits (constants and functions). Note that this class is not modeled after
std::char_traits.
*/
struct ABACLADE_SYM utf16_traits {
public:

   /** See utf8_traits::max_codepoint_length. */
   static unsigned const max_codepoint_length = 2;


public:

   /** See utf8_traits::cp_len().
   */
   static size_t cp_len(char16_t const * pchBegin, char16_t const * pchEnd);


   /** See utf8_traits::from_utf32().
   */
   static unsigned from_utf32(char32_t ch32, char16_t * pchDst);


   /** See utf8_traits::is_valid().
   */
   static bool is_valid(char16_t const * psz);
   static bool is_valid(char16_t const * pchBegin, char16_t const * pchEnd);


   /** See utf8_traits::str_len().
   */
   static size_t str_len(char16_t const * psz);
};

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::utf32_traits


namespace abc {
namespace text {

/** UTF-32 character traits (constants and functions). Note that this class is not modeled after
std::char_traits.
*/
struct ABACLADE_SYM utf32_traits {
public:

   /** See utf8_traits::is_valid(). With overload for single characters.
   */
   static /*constexpr*/ bool is_valid(char32_t ch) {
      return ch < 0x00dc80 || (ch > 0x00dcff && ch <= 0x10ffff);
   }
};

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

