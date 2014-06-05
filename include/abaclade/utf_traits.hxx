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
// abc::text::utf_traits


namespace abc {
namespace text {

/** UTF character traits: constants and functions related to the UTF encoding based on the character
type C. Note that this class is not modeled after std::char_traits.
*/
template <typename C = char_t>
struct utf_traits;

typedef utf_traits<char8_t > utf8_traits;
typedef utf_traits<char16_t> utf16_traits;
typedef utf_traits<char32_t> utf32_traits;

// Specialization for UTF-8.
template <>
struct ABACLADE_SYM utf_traits<char8_t> {
public:

   /** Encoded form of the BOM. */
   static char8_t const bom[];
   /** Default encoding for this UTF encoding on this machine. */
   static encoding::enum_type const host_encoding = encoding::utf8;
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


   /** Returns a pointer to the first occurrence of a character in a string, or pchHaystackEnd if no
   matches are found. For the non-char32_t needle overload, the needle is a pointer because a code
   point can require more than one non-UTF-32 character to be encoded.

   UTF validity: necessary.

   pchHaystackBegin
      Pointer to the first character of the string to be searched.
   pchHaystackEnd
      Pointer to beyond the last character of the string to be searched.
   chNeedle
      Code point to search for.
   pchNeedle
      Pointer to the encoded code point (UTF character sequence) to search for; its length is
      deduced automatically.
   return
      Pointer to the beginning of the first match, in the string to be searched, of the code point
      to search for, or nullptr if no matches are found.
   */
   static char8_t const * str_chr(
      char8_t const * pchHaystackBegin, char8_t const * pchHaystackEnd, char32_t chNeedle
   );
   static char8_t const * str_chr(
      char8_t const * pchHaystackBegin, char8_t const * pchHaystackEnd, char8_t const * pchNeedle
   );


   /** Returns a pointer to the last occurrence of a character in a string, or pchHaystackBegin if
   no matches are found. For the char32_t needle overload, the needle is a pointer because a code
   point can require more than one non-UTF-32 character to be encoded.

   UTF validity: necessary.

   pchHaystackBegin
      Pointer to the first character of the string to be searched.
   pchHaystackEnd
      Pointer to beyond the last character of the string to be searched.
   chNeedle
      Code point to search for.
   pchNeedle
      Pointer to the encoded code point (UTF character sequence) to search for; its length is
      deduced automatically.
   return
      Pointer to the beginning of the last match, in the string to be searched, of the code point
      to search for, or nullptr if no matches are found.
   */
   static char8_t const * str_chr_r(
      char8_t const * pchHaystackBegin, char8_t const * pchHaystackEnd, char32_t chNeedle
   );
   static char8_t const * str_chr_r(
      char8_t const * pchHaystackBegin, char8_t const * pchHaystackEnd, char8_t const * pchNeedle
   ) {
      // We can’t do the fast forward scan that str_chr can do because the UTF-8 characters are in
      // the reverse order, so just do a regular substring search.
      return str_str_r(
         pchHaystackBegin, pchHaystackEnd,
         pchNeedle, pchNeedle + 1 + leading_to_cont_length(*pchNeedle)
      );
   }


   /** Compares two UTF strings.

   UTF validity: necessary.

   psz1
      Pointer to the first NUL-terminated string to compare.
   psz2
      Pointer to the second NUL-terminated string to compare.
   pch1Begin
      Pointer to the first character of the first string to compare.
   pch1End
      Pointer to beyond the last character of the string to compare.
   pch2Begin
      Pointer to the first character of the second string to compare.
   pch2End
      Pointer to beyond the last character of the second string to compare.
   return
      Standard comparison result integer:
      •  > 0 if string 1 > string 2;
      •    0 if string 1 == string 2;
      •  < 0 if string 1 < string 2.
   */
   static int str_cmp(char8_t const * psz1, char8_t const * psz2);
   static int str_cmp(
      char8_t const * pch1Begin, char8_t const * pch1End,
      char8_t const * pch2Begin, char8_t const * pch2End
   );


   /** Returns the length, in UTF characters, of a NUL-terminated string.

   UTF validity: necessary.

   psz
      Pointer to the NUL-terminated string of which to calculate the length.
   return
      Length of the string pointed to by psz, in characters.
   */
   static size_t str_len(char8_t const * psz);


   /** Returns the character index of the first occurrence of a string into another.

   UTF validity: necessary.

   pchHaystackBegin
      Pointer to the first character of the string to be searched.
   pchHaystackEnd
      Pointer to beyond the last character of the string to be searched.
   pchNeedleBegin
      Pointer to the first character of the string to search for.
   pchNeedleEnd
      Pointer to beyond the last character of the string to search for.
   return
      Pointer to the beginning of the first match, in the string to be searched, of the string to
      search for, or nullptr if no matches are found.
   */
   static char8_t const * str_str(
      char8_t const * pchHaystackBegin, char8_t const * pchHaystackEnd,
      char8_t const * pchNeedleBegin, char8_t const * pchNeedleEnd
   );


   /** Returns the character index of the last occurrence of a string into another.

   UTF validity: necessary.

   pchHaystackBegin
      Pointer to the first character of the string to be searched.
   pchHaystackEnd
      Pointer to beyond the last character of the string to be searched.
   pchNeedleBegin
      Pointer to the first character of the string to search for.
   pchNeedleEnd
      Pointer to beyond the last character of the string to search for.
   return
      Pointer to the beginning of the last match, in the string to be searched, of the string to
      search for, or nullptr if no matches are found.
   */
   static char8_t const * str_str_r(
      char8_t const * pchHaystackBegin, char8_t const * pchHaystackEnd,
      char8_t const * pchNeedleBegin, char8_t const * pchNeedleEnd
   );


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

// Specialization for UTF-16.
template <>
struct ABACLADE_SYM utf_traits<char16_t> {
public:

   /** See utf8_traits::bom. */
   static char16_t const bom[];
   /** See utf8_traits::host_encoding. */
   static encoding::enum_type const host_encoding = encoding::utf16_host;
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


   /** See utf8_traits::str_chr().
   */
   static char16_t const * str_chr(
      char16_t const * pchHaystackBegin, char16_t const * pchHaystackEnd, char32_t chNeedle
   );
   static char16_t const * str_chr(
      char16_t const * pchHaystackBegin, char16_t const * pchHaystackEnd, char16_t const * pchNeedle
   );


   /** See utf8_traits::str_chr_r().
   */
   static char16_t const * str_chr_r(
      char16_t const * pchHaystackBegin, char16_t const * pchHaystackEnd, char32_t chNeedle
   );
   static char16_t const * str_chr_r(
      char16_t const * pchHaystackBegin, char16_t const * pchHaystackEnd, char16_t const * pchNeedle
   );


   /** See utf8_traits::str_cmp().
   */
   static int str_cmp(char16_t const * psz1, char16_t const * psz2);
   static int str_cmp(
      char16_t const * pch1Begin, char16_t const * pch1End,
      char16_t const * pch2Begin, char16_t const * pch2End
   );


   /** See utf8_traits::str_len().
   */
   static size_t str_len(char16_t const * psz);


   /** See utf8_traits::str_str().
   */
   static char16_t const * str_str(
      char16_t const * pchHaystackBegin, char16_t const * pchHaystackEnd,
      char16_t const * pchNeedleBegin, char16_t const * pchNeedleEnd
   );


   /** See utf8_traits::str_str_r().
   */
   static char16_t const * str_str_r(
      char16_t const * pchHaystackBegin, char16_t const * pchHaystackEnd,
      char16_t const * pchNeedleBegin, char16_t const * pchNeedleEnd
   );
};

// Specialization for UTF-32.
template <>
struct ABACLADE_SYM utf_traits<char32_t> {
public:

   /** See utf8_traits::bom. */
   static char32_t const bom[];
   /** See utf8_traits::host_encoding. */
   static encoding::enum_type const host_encoding = encoding::utf32_host;
   /** See utf8_traits::max_codepoint_length. */
   static unsigned const max_codepoint_length = 1;


public:

   /** See utf8_traits::cp_len(). Trivial for UTF-32, since it’s always 1 character per code point.
   */
   static /*constexpr*/ size_t cp_len(char32_t const * pchBegin, char32_t const * pchEnd) {
      return size_t(pchEnd - pchBegin);
   }


   /** See utf8_traits::from_utf32().
   */
   static unsigned from_utf32(char32_t ch32, char32_t * pchDst) {
      *pchDst = ch32;
      return 1;
   }


   /** See utf8_traits::is_valid(). With overload for single characters.
   */
   static /*constexpr*/ bool is_valid(char32_t ch) {
      return ch < 0x00dc80 || (ch > 0x00dcff && ch <= 0x10ffff);
   }
   static bool is_valid(char32_t const * psz);
   static bool is_valid(char32_t const * pchBegin, char32_t const * pchEnd);


   /** See utf8_traits::str_chr().
   */
   static char32_t const * str_chr(
      char32_t const * pchHaystackBegin, char32_t const * pchHaystackEnd, char32_t chNeedle
   );
   static char32_t const * str_chr(
      char32_t const * pchHaystackBegin, char32_t const * pchHaystackEnd, char32_t const * pchNeedle
   ) {
      // In UTF-32, there’s always only one character per code point.
      return str_chr(pchHaystackBegin, pchHaystackEnd, *pchNeedle);
   }


   /** See utf8_traits::str_chr_r().
   */
   static char32_t const * str_chr_r(
      char32_t const * pchHaystackBegin, char32_t const * pchHaystackEnd, char32_t chNeedle
   );
   static char32_t const * str_chr_r(
      char32_t const * pchHaystackBegin, char32_t const * pchHaystackEnd, char32_t const * pchNeedle
   ) {
      // In UTF-32, there’s always only one character per code point.
      return str_chr_r(pchHaystackBegin, pchHaystackEnd, *pchNeedle);
   }


   /** See utf8_traits::str_cmp().
   */
   static int str_cmp(char32_t const * psz1, char32_t const * psz2);
   static int str_cmp(
      char32_t const * pch1Begin, char32_t const * pch1End,
      char32_t const * pch2Begin, char32_t const * pch2End
   );


   /** See utf8_traits::str_len().
   */
   static size_t str_len(char32_t const * psz);


   /** See utf8_traits::str_str().
   */
   static char32_t const * str_str(
      char32_t const * pchHaystackBegin, char32_t const * pchHaystackEnd,
      char32_t const * pchNeedleBegin, char32_t const * pchNeedleEnd
   );


   /** See utf8_traits::str_str_r().
   */
   static char32_t const * str_str_r(
      char32_t const * pchHaystackBegin, char32_t const * pchHaystackEnd,
      char32_t const * pchNeedleBegin, char32_t const * pchNeedleEnd
   );
};

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

