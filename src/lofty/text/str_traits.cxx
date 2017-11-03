/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015, 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/text.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {

/*static*/ void str_traits::_build_find_failure_restart_table(
   char_t const * substr_begin, char_t const * substr_end, collections::vector<std::size_t> * failure_restarts
) {
   failure_restarts->set_size(static_cast<std::size_t>(substr_end - substr_begin));
   auto next_failure_restart_itr(failure_restarts->begin());

   /* The earliest repetition of a non-first character can only occur on the fourth character, so start by
   skipping two characters and storing two zeros for them, then the first iteration will also always store an
   additional zero and consume one more character. */
   auto substr = substr_begin + 2, substr_restart = substr_begin;
   *next_failure_restart_itr++ = 0;
   *next_failure_restart_itr++ = 0;
   std::size_t restart_index = 0;
   while (substr < substr_end) {
      /* Store the current failure restart index, or 0 if the previous character was the third or was not a
      match. */
      *next_failure_restart_itr++ = restart_index;
      if (*substr++ == *substr_restart) {
         // Another match: move the restart to the next character.
         ++restart_index;
         ++substr_restart;
      } else if (restart_index > 0) {
         // End of a match: restart self-matching from index 0.
         restart_index = 0;
         substr_restart = substr_begin;
      }
   }
}

/*static*/ int str_traits::compare(
   char_t const * left_begin, char_t const * left_end, char_t const * right_begin, char_t const * right_end
) {
   char_t const * left = left_begin, * right = right_begin;
   while (left < left_end && right < right_end) {
      char_t left_ch = *left++, right_ch = *right++;
#if LOFTY_HOST_UTF == 8
      /* Note: not only don’t sequences matter when scanning for the first differing bytes, but once a pair of
      differing bytes is found, if they are part of a sequence, its start must have been the same, so only
      their absolute value matters; if they started a sequence, the first byte of a longer encoding (greater
      code point value) if greater than that of a shorter one. */
#elif LOFTY_HOST_UTF == 16 //if LOFTY_HOST_UTF == 8
      // Surrogates mess with the ability to just compare the absolute char16_t value.
      bool left_is_surrogate = host_char_traits::is_surrogate(left_ch);
      bool right_is_surrogate = host_char_traits::is_surrogate(right_ch);
      if (left_is_surrogate != right_is_surrogate) {
         if (left_is_surrogate) {
            // If left_ch is a surrogate and right_ch is not, left_ch > right_ch.
            return +1;
         } else /*if (right_is_surrogate)*/ {
            // If right_ch is a surrogate and left_ch is not, left_ch < right_ch.
            return -1;
         }
      }
      /* The characters are both regular or surrogates. Since a difference in lead surrogate generates bias,
      we only get to compare trails if the leads were equal. */
#endif //if LOFTY_HOST_UTF == 8 … elif LOFTY_HOST_UTF == 16
      if (left_ch > right_ch) {
         return +1;
      } else if (left_ch < right_ch) {
         return -1;
      }
   }
   // If we’re still here, the string that didn’t run out of characters wins.
   if (left < left_end) {
      return +1;
   } else if (right < right_end) {
      return -1;
   } else {
      return 0;
   }
}

/*static*/ char_t const * str_traits::find_char(
   char_t const * str_begin, char_t const * str_end, char32_t cp
) {
   if (cp <= host_char_traits::max_single_char_codepoint) {
      // The code point can be encoded as a single character, so this faster search can be used.
      return find_char(str_begin, str_end, static_cast<char_t>(cp));
   } else {
      // The code point is two or more characters, so take the slower approach.
      char_t cp_chars[host_char_traits::max_codepoint_length];
      host_char_traits::codepoint_to_chars(cp, cp_chars);
      return find_char(str_begin, str_end, cp_chars);
   }
}
/*static*/ char_t const * str_traits::find_char(
   char_t const * str_begin, char_t const * str_end, char_t const * cp_chars
) {
#if LOFTY_HOST_UTF == 8
   char8_t cp_lead_ch = *cp_chars;
   for (char8_t const * s = str_begin, * str_next; s < str_end; s = str_next) {
      char8_t ch = *s;
      unsigned cp_size = host_char_traits::lead_char_to_codepoint_size(ch);
      // Make the next iteration resume from the next code point.
      str_next = s + cp_size;
      if (ch == cp_lead_ch) {
         if (--cp_size) {
            // The lead bytes match; check if the trailing ones do as well.
            auto str_cont = s, cp_cont = cp_chars;
            while (++str_cont < str_next && *str_cont == *++cp_cont) {
               ;
            }
            if (str_cont < str_next) {
               continue;
            }
            // The lead and trailing bytes of str and code point match.
         }
         return s;
      }
   }
#elif LOFTY_HOST_UTF == 16 //if LOFTY_HOST_UTF == 8
   // In UTF-16, there’s always at most two characters per code point.
   char16_t cp_char0 = cp_chars[0];
   /* We only have a second character if the first is a lead surrogate. Using NUL as a special value is safe,
   because if this is a surrogate, the tail surrogate cannot be NUL. */
   char16_t cp_char1 = host_char_traits::is_lead_surrogate(cp_char0) ? cp_chars[1] : host_char(0);
   /* The bounds of this loop are safe: since we assume that both strings are valid UTF-16, if
   str[0] == cp_char0 and cp_char1 != NUL then str[1] must be accessible. */
   for (auto s = str_begin; s < str_end; ++s) {
      if (s[0] == cp_char0 && (cp_char1 == 0 || s[1] == cp_char1)) {
         return s;
      }
   }
#endif //if LOFTY_HOST_UTF == 8 … elif LOFTY_HOST_UTF == 16
   return str_end;
}

/*static*/ char_t const * str_traits::find_char_last(
   char_t const * str_begin, char_t const * str_end, char32_t cp
) {
   if (cp <= host_char_traits::max_single_char_codepoint) {
      // The code point can be encoded as a single character, so this faster search can be used.
      return find_char_last(str_begin, str_end, static_cast<char_t>(cp));
   } else {
      /* The code point is two or more characters; this means that we can’t do the fast backwards scan above,
      so just do a regular substring reverse search. */
      char_t cp_chars[host_char_traits::max_codepoint_length];
      return find_substr_last(
         str_begin, str_end, cp_chars, host_char_traits::codepoint_to_chars(cp, cp_chars)
      );
   }
}

/*static*/ char_t const * str_traits::find_substr(
   char_t const * str_begin, char_t const * str_end, char_t const * substr_begin, char_t const * substr_end
) {
   if (substr_begin == substr_end) {
      // Empty substring, so just return the beginning of the str.
      return str_begin;
   }
   auto s = str_begin;
   auto substr = substr_begin;
   try {
      // Build the failure restart table.
      collections::vector<std::size_t, 64> failure_restarts;
      _build_find_failure_restart_table(substr_begin, substr_end, failure_restarts.vector0_ptr());

      std::size_t failure_restart_index = 0;
      while (s < str_end) {
         if (*s == *substr) {
            ++substr;
            if (substr == substr_end) {
               // The substring was exhausted, which means that all its characters were matched in the string.
               return s - failure_restart_index;
            }
            // Move to the next character and advance the index in failure_restarts.
            ++s;
            ++failure_restart_index;
         } else if (failure_restart_index > 0) {
            /* The current character ends the match sequence; use failure_restarts[failure_restart_index] to
            see how much into the substring we can retry matching characters. */
            failure_restart_index = failure_restarts[static_cast<std::ptrdiff_t>(failure_restart_index)];
            substr = substr_begin + failure_restart_index;
         } else {
            /* Not a match, and no restart point: we’re out of options to match this character, so consider it
            not-a-match and move past it. */
            ++s;
         }
      }
   } catch (_std::bad_alloc const &) {
      /* Could not allocate enough memory for the failure restart table: fall back to a plain (and potentially
      slower) substring search. */
      char_t substr0 = *substr_begin;
      for (; s < str_end; ++s) {
         if (*s == substr0) {
            auto str_match = s;
            substr = substr_begin;
            while (++substr < substr_end && *++str_match == *substr) {
               ;
            }
            if (substr >= substr_end) {
               // The substring was exhausted, which means that all its characters were matched in the str.
               return s;
            }
         }
      }
   }
   return str_end;
}

/*static*/ char_t const * str_traits::find_substr_last(
   char_t const * str_begin, char_t const * str_end, char_t const * substr_begin, char_t const * substr_end
) {
   LOFTY_UNUSED_ARG(str_begin);
   LOFTY_UNUSED_ARG(str_end);
   LOFTY_UNUSED_ARG(substr_begin);
   LOFTY_UNUSED_ARG(substr_end);
   // TODO: implement this!
   return str_end;
}

/*static*/ std::size_t str_traits::size_in_codepoints(char_t const * begin, char_t const * end) {
   std::size_t size = 0;
   for (auto s = begin; s < end; s += host_char_traits::lead_char_to_codepoint_size(*s)) {
      ++size;
   }
   return size;
}

/*static*/ bool str_traits::validate(
   char_t const * begin, char_t const * end, bool throw_on_errors /*= false*/
) {
#if LOFTY_HOST_UTF == 8
   for (auto s = begin; s < end; ) {
      std::uint8_t const * cp_begin = reinterpret_cast<std::uint8_t const *>(s);
      char8_t ch = *s++;
      // This should be a lead byte, and not the start of an overlong or an invalid lead byte.
      if (!utf8_char_traits::is_valid_lead_char(ch)) {
         if (throw_on_errors) {
            LOFTY_THROW(decode_error, (LOFTY_SL("invalid UTF-8 lead byte"), cp_begin, cp_begin + 1));
         } else {
            return false;
         }
      }

      /* If the lead byte is 111?0000, activate the detection logic for overlong encodings in the nested for
      loop; see below for more info. */
      bool validate_on_bits_in_first_trail_byte = (ch & 0xef) == 0xe0;

      /* Ensure that these bits are 0 to detect encoded code points above
      (11110)100 (10)00xxxx (10)yyyyyy (10)zzzzzz, which is the highest valid code point
      10000 xxxxyyyy yyzzzzzz. */
      char8_t first_trail_byte_off_validity_mask = ch == '\xf4' ? 0x30 : 0x00;

      for (unsigned trail_size = utf8_char_traits::lead_char_to_codepoint_size(ch); --trail_size; ) {
         if (s == end || !utf8_char_traits::is_trail_char(ch = *s++)) {
            /* The string ended prematurely when we were expecting more trail characters, or this is not a
            trail character. */
            if (throw_on_errors) {
               LOFTY_THROW(decode_error, (
                  LOFTY_SL("unexpected end of UTF-8 sequence"),
                  cp_begin, reinterpret_cast<std::uint8_t const *>(s)
               ));
            } else {
               return false;
            }
         }
         if (validate_on_bits_in_first_trail_byte) {
            /* Detect overlong encodings by detecting zeros in the lead byte and masking the first trail byte
            with an “on” mask. */
            static char8_t const overlong_detection_masks[] = {
               // 1-character sequences cannot be overlongs.
               /* 1 */ 0,
               // 2-character overlongs are filtered out by utf8_char_traits::is_valid_lead_char().
               /* 2 */ 0,
               // Detect 11100000 100xxxxx …, overlong for 110xxxxx ….
               /* 3 */ 0x20,
               // Detect 11110000 1000xxxx …, overlong for 1110xxxx ….
               /* 4 */ 0x30
               /* Longer overlongs are possible, but they require a lead byte that is filtered out by
               utf8_char_traits::is_valid_lead_char(). */
            };
            if (!(ch & overlong_detection_masks[trail_size])) {
               if (throw_on_errors) {
                  LOFTY_THROW(decode_error, (
                     LOFTY_SL("overlong UTF-8 sequence"), cp_begin, reinterpret_cast<std::uint8_t const *>(s)
                  ));
               } else {
                  return false;
               }
            }
            validate_on_bits_in_first_trail_byte = false;
         }
         if (first_trail_byte_off_validity_mask) {
            // If the “off” mask reveals a “1” bit, this trail byte is invalid.
            if (ch & first_trail_byte_off_validity_mask) {
               if (throw_on_errors) {
                  LOFTY_THROW(decode_error, (
                     LOFTY_SL("UTF-8 sequence decoded into invalid code point"),
                     cp_begin, reinterpret_cast<std::uint8_t const *>(s)
                  ));
               } else {
                  return false;
               }
            }
            first_trail_byte_off_validity_mask = 0;
         }
      }
   }
   return true;
#elif LOFTY_HOST_UTF == 16 //if LOFTY_HOST_UTF == 8
   bool expecting_trail_surrogate = false;
   for (auto s = begin; s < end; ++s) {
      std::uint8_t const * cp_begin = reinterpret_cast<std::uint8_t const *>(s);
      char16_t ch = *s;
      if (utf16_char_traits::is_surrogate(ch)) {
         bool trail_surrogate = utf16_char_traits::is_trail_char(ch);
         /* If this is a lead surrogate and we were expecting a trail, or this is a trail surrogate but we’re
         not in a surrogate, this character is invalid. */
         if (trail_surrogate != expecting_trail_surrogate) {
            if (throw_on_errors) {
               LOFTY_THROW(decode_error, (
                  LOFTY_SL("invalid lone surrogate"), cp_begin, cp_begin + sizeof(char16_t)
               ));
            } else {
               return false;
            }
         }
         expecting_trail_surrogate = !trail_surrogate;
      } else if (expecting_trail_surrogate) {
         // We were expecting a trail surrogate, but this is not a surrogate at all.
         if (throw_on_errors) {
            LOFTY_THROW(decode_error, (
               LOFTY_SL("invalid lone lead surrogate"), cp_begin, cp_begin + sizeof(char16_t)
            ));
         } else {
            return false;
         }
      }
   }
   // Cannot end in the middle of a surrogate.
   return !expecting_trail_surrogate;
#endif //if LOFTY_HOST_UTF == 8 … elif LOFTY_HOST_UTF == 16
}

}} //namespace lofty::text
