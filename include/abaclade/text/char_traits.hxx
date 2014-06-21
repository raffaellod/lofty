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
// abc::text::utf8_char_traits


namespace abc {
namespace text {

/** UTF-8 character traits (constants and functions). Note that this class is not modeled after
std::char_traits.
*/
class ABACLADE_SYM utf8_char_traits {
public:

   /** Max length of a code point, in UTF-8 characters (bytes). Technically, 6 is also possible due
   to the way bits are encoded, but it’s illegal. */
   static unsigned const max_codepoint_length = 4;


public:

   /** Converts a code point (UTF-32 character) into a char8_t array.

   cp
      Code point to be encoded.
   pchDstBegin
      Start of the character array that will receive the encoded version of cp.
   return
      Pointer to the character beyond the last one used in *pchDstBegin.
   */
   static char8_t * codepoint_to_chars(char32_t cp, char8_t * pchDstBegin);


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


   /** Returns true if the specified character is a trail (non-lead) character.

   ch
      UTF-8 character.
   return
      true if ch is a trail character, or false if it’s a lead character.
   */
   static /*constexpr*/ bool is_trail_char(char8_t ch) {
      return (ch & 0xc0) == 0x80;
   }


   /** Returns the bits in a lead byte that are part of the encoded code point. Notice that the bits
   will need to be shifted in the right position to form a valid UTF-32 character.

   ch
      First byte of an UTF-8 code point.
   cbCont
      Length of the remainder of the UTF-8 byte sequence, in bytes.
   return
      Bits in ch that participate in the code point.
   */
   static /*constexpr*/ char32_t get_lead_char_codepoint_bits(char8_t ch, unsigned cbCont) {
      return char32_t(ch & (0x7f >> smc_acbitShiftMask[cbCont]));
   }


   /** Returns the run length of an UTF-8 sequence, given its lead byte.

   ch
      First byte of an UTF-8 code point.
   return
      Length of the code point sequence, or 1 if the character is not a lead byte, i.e. it’s a code
      point encoded as a single byte or an invalid sequence.
   */
   static /*constexpr*/ unsigned lead_char_to_codepoint_size(char8_t ch) {
      unsigned i(static_cast<uint8_t>(ch));
      // See comments on smc_acbConts in char_traits.cxx to understand this way of accessing it.
      //             (smc_acbCpSizesByLeadChar[byte idx] >> [nibble idx → 0 or 4]) & nibble mask
      return unsigned(smc_acbCpSizesByLeadChar[ i >> 2 ] >> (  (i & 2) << 1     )) & 0xfu;
   }


private:

   /** Maps each UTF-8 lead byte to the length of its entire encoded code point. */
   static uint8_t const smc_acbCpSizesByLeadChar[];
   /** Shift counts for the mask 0x7f to be applied to each lead byte to get the bits actually part
   of the code point; indexed by the number of bytes in the sequence. */
   static uint8_t const smc_acbitShiftMask[];
};

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::utf16_char_traits


namespace abc {
namespace text {

/** UTF-16 character traits (constants and functions). Note that this class is not modeled after
std::char_traits.
*/
class utf16_char_traits {
public:

   /** Max length of a code point, in UTF-16 characters. */
   static unsigned const max_codepoint_length = 2;


public:

   /** Converts a code point (UTF-32 character) into a char16_t array.

   cp
      Code point to be encoded.
   pchDstBegin
      Start of the character array that will receive the encoded version of cp.
   return
      Pointer to the character beyond the last one used in *pchDstBegin.
   */
   static char16_t * codepoint_to_chars(char32_t cp, char16_t * pchDstBegin);


   /** Returns true if the specified character is a surrogate lead.

   ch
      UTF-16 character.
   return
      true if ch is a lead surrogate, or false if it’s a trail surrogate.
   */
   static /*constexpr*/ bool is_lead_surrogate(char16_t ch) {
      return (ch & 0xfc00) == 0xd800;
   }


   /** Returns true if the specified character is a surrogate (lead or trail).

   ch
      UTF-16 character.
   return
      true if ch is a surrogate, or false otherwise.
   */
   static /*constexpr*/ bool is_surrogate(char16_t ch) {
      return (ch & 0xf800) == 0xd800;
   }


   /** See utf8_char_traits::is_trail_char().
   */
   static /*constexpr*/ bool is_trail_char(char16_t ch) {
      return (ch & 0xfc00) == 0xdc00;
   }


   /** See utf8_char_traits::lead_char_to_codepoint_size().
   */
   static /*constexpr*/ unsigned lead_char_to_codepoint_size(char16_t ch) {
      return is_lead_surrogate(ch) ? 2u : 1u;
   }
};

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::host_char_traits


namespace abc {
namespace text {

/** String traits for the host character type, abc::char_t. Derives from either utf8_char_traits or
utf16_char_traits.
*/
class ABACLADE_SYM host_char_traits :
#if ABC_HOST_UTF == 8
   public utf8_char_traits {
#elif ABC_HOST_UTF == 16
   public utf16_char_traits {
#endif

#if ABC_HOST_UTF == 8
   typedef utf8_char_traits traits_base;
#elif ABC_HOST_UTF == 16
   typedef utf16_char_traits traits_base;
#endif

public:

   /** Converts a code point (UTF-32 character) into a char_t array.

   cp
      Code point to be encoded.
   achDst
      Character array that will receive the encoded version of cp.
   return
      Pointer to the character beyond the last one used in achDst.
   */
   static char_t * codepoint_to_chars(char32_t cp, char_t (& achDst)[max_codepoint_length]) {
      return traits_base::codepoint_to_chars(cp, achDst);
   }
};

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

