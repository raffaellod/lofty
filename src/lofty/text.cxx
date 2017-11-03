/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/byte_order.hxx>
#include <lofty/numeric.hxx>
#include <lofty/text.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {

std::size_t get_encoding_size(encoding enc) {
   // Little helper to map lofty::text::encoding values with byte sizes (see below).
   struct enc_char_byte_size_t {
      std::uint8_t enc;
      std::uint8_t char_byte_size;
   };

   // Character size, in bytes, for each recognized encoding.
   static enc_char_byte_size_t const enc_char_byte_sizes[] = {
      { encoding::utf8,         1 },
      { encoding::utf16le,      2 },
      { encoding::utf16be,      2 },
      { encoding::utf32le,      4 },
      { encoding::utf32be,      4 },
      { encoding::iso_8859_1,   1 },
      { encoding::windows_1252, 1 }
   };
   // TODO: improve search algorithm, or maybe use a real map.
   for (std::size_t i = 0; i < LOFTY_COUNTOF(enc_char_byte_sizes); ++i) {
      if (enc == encoding::enum_type(enc_char_byte_sizes[i].enc)) {
         return enc_char_byte_sizes[i].char_byte_size;
      }
   }
   // TODO: provide more information in the exception.
   LOFTY_THROW(domain_error, ());
}

str get_line_terminator_str(line_terminator lterm) {
   if (lterm == line_terminator::any) {
      lterm = line_terminator::host;
   }
   switch (lterm.base()) {
      case lofty::text::line_terminator::cr:
         return str(LOFTY_SL("\r"));
      case lofty::text::line_terminator::lf:
         return str(LOFTY_SL("\n"));
      case lofty::text::line_terminator::cr_lf:
         return str(LOFTY_SL("\r\n"));
      default:
         // TODO: provide more information in the exception.
         LOFTY_THROW(domain_error, ());
   }
}

encoding guess_encoding(
   void const * buf_begin, void const * buf_end, std::size_t src_total_bytes /*= 0*/,
   std::size_t * bom_byte_size /*= nullptr*/
) {
   auto buf_bytes_begin = static_cast<std::uint8_t const *>(buf_begin);
   auto buf_bytes_end   = static_cast<std::uint8_t const *>(buf_end);
   // If the total size is not specified, assume that the buffer is the wholesource.
   if (src_total_bytes == 0) {
      src_total_bytes = static_cast<std::size_t>(buf_bytes_end - buf_bytes_begin);
   }

   /* Statuses for the scanner. Each BOM status must be 1 bit to the right of its resulting encoding; LE
   variants must be 2 bits to the right of their BE counterparts. */
   enum enc_scan_status {
      ESS_UTF8_BOM     = 0x0001,
      ESS_UTF8         = 0x0002,
      ESS_UTF16LE_BOM  = 0x0004,
      ESS_UTF16LE      = 0x0008,
      ESS_UTF16BE_BOM  = 0x0010,
      ESS_UTF16BE      = 0x0020,
      ESS_UTF32LE_BOM  = 0x0040,
      ESS_UTF32LE      = 0x0080,
      ESS_UTF32BE_BOM  = 0x0100,
      ESS_UTF32BE      = 0x0200,
      ESS_ISO_8859_1   = 0x0400,
      ESS_WINDOWS_1252 = 0x0800,

      // ESS_UTF*_BOM
      ESS_MASK_BOMS    = 0x0155,
      // ESS_UTF16*
      ESS_MASK_UTF16   = 0x003c,
      // ESS_UTF32*
      ESS_MASK_UTF32   = 0x03c0,
      // Everything else.
      ESS_MASK_NONUTF  = 0x0d00,
      // Start status.
      ESS_MASK_START   = ESS_MASK_NONUTF | ESS_MASK_BOMS | ESS_UTF8
   };

   // A 1 in this bit array means that the corresponding byte value is valid in ISO-8859-1.
   static std::uint8_t const iso88591_validity[] = {
      0x80, 0x3e, 0x00, 0x08, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
      0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
   };
   // A 1 in this bit array means that the corresponding byte value is valid in Windows-1252.
   static std::uint8_t const windows1252_validity[] = {
      0x80, 0x3e, 0x00, 0x08, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
      0xfd, 0x5f, 0xfe, 0xdf, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
   };
   // BOMs for bom_scan_data.
   static std::uint8_t const
      utf8_bom   [] = { 0xef, 0xbb, 0xbf },
      utf16le_bom[] = { 0xff, 0xfe },
      utf16be_bom[] = { 0xfe, 0xff },
      utf32le_bom[] = { 0xff, 0xfe, 0x00, 0x00 },
      utf32be_bom[] = { 0x00, 0x00, 0xfe, 0xff };
   // Struct to uniformize scanning for BOMs.
   static struct bom_scan_data_t {
      std::uint8_t const * bom_bytes;
      std::uint16_t ess_flag;
      std::uint8_t bom_byte_size;
   } const bom_scan_data[] = {
      { utf8_bom,    ESS_UTF8_BOM,    sizeof(utf8_bom   ) },
      { utf16le_bom, ESS_UTF16LE_BOM, sizeof(utf16le_bom) },
      { utf16be_bom, ESS_UTF16BE_BOM, sizeof(utf16be_bom) },
      { utf32le_bom, ESS_UTF32LE_BOM, sizeof(utf32le_bom) },
      { utf32be_bom, ESS_UTF32BE_BOM, sizeof(utf32be_bom) }
   };


   // Initially, consider anything that doesn’t require a BOM.
   unsigned flags = ESS_MASK_START;

   // Initially, assume no BOM will be found.
   if (bom_byte_size) {
      *bom_byte_size = 0;
   }

   // Easy checks.
   if (src_total_bytes & (sizeof(char32_t) - 1)) {
      // UTF-32 requires a number of bytes multiple of sizeof(char32_t).
      flags &= ~static_cast<unsigned>(ESS_MASK_UTF32);
      if (src_total_bytes & (sizeof(char16_t) - 1)) {
         // UTF-16 requires an even number of bytes.
         flags &= ~static_cast<unsigned>(ESS_MASK_UTF16);
      }
   }

   /* Parse every byte, gradually excluding more and more possibilities, hopefully ending with exactly one
   guess. */
   unsigned utf8_trail_byte_size = 0;
   std::size_t byte_index = 0;
   for (auto buf_bytes = buf_bytes_begin; buf_bytes < buf_bytes_end; ++buf_bytes, ++byte_index) {
      std::uint8_t b = *buf_bytes;

      if (flags & ESS_UTF8) {
         // Check for UTF-8 validity. Checking for overlongs or invalid code points is out of scope here.
         if (utf8_trail_byte_size) {
            if (!utf8_char_traits::is_trail_char(static_cast<char8_t>(b))) {
               // This byte should be part of a sequence, but it’s not.
               flags &= ~static_cast<unsigned>(ESS_UTF8);
            } else {
               --utf8_trail_byte_size;
            }
         } else {
            if (utf8_char_traits::is_trail_char(static_cast<char8_t>(b))) {
               // This byte should be a lead byte, but it’s not.
               flags &= ~static_cast<unsigned>(ESS_UTF8);
            } else {
               utf8_trail_byte_size = utf8_char_traits::lead_char_to_codepoint_size(
                  static_cast<char8_t>(b)
               ) - 1;
               if ((b & 0x80) && utf8_trail_byte_size == 0) {
                  /* By utf8_char_traits::lead_char_to_codepoint_size(), a non-ASCII byte that doesn’t have a
                  continuation is an invalid one. */
                  flags &= ~static_cast<unsigned>(ESS_UTF8);
               }
            }
         }
      }

      if (flags & (ESS_UTF16LE | ESS_UTF16BE)) {
         /* Check for UTF-16 validity. The only check possible is proper ordering of surrogates; everything
         else is allowed. */
         for (unsigned enc_flag = ESS_UTF16LE; enc_flag <= ESS_UTF16BE; enc_flag <<= 2) {
            /* This will go ahead with the check if byte_index is indexing the most significant byte, i.e. odd
            for LE and even for BE. */
            if ((flags & enc_flag) && ((byte_index & sizeof(char16_t)) != 0) == (enc_flag != ESS_UTF16LE)) {
               switch (b & 0xfc) {
                  case 0xd8: {
                     // There must be a trail surrogate after 1 byte.
                     std::uint8_t const * next_byte = buf_bytes + sizeof(char16_t);
                     if (next_byte >= buf_bytes_end || (*next_byte & 0xfc) != 0xdc) {
                        flags &= ~enc_flag;
                     }
                     break;
                  }
                  case 0xdc: {
                     // Assume there was a lead surrogate 2 bytes before.
                     std::uint8_t const * prev_byte = buf_bytes - sizeof(char16_t);
                     if (prev_byte < buf_bytes_begin || (*prev_byte & 0xfc) != 0xd8) {
                        flags &= ~enc_flag;
                     }
                     break;
                  }
               }
            }
         }
      }

      if ((flags & (ESS_UTF32LE | ESS_UTF32BE)) && (byte_index & sizeof(char32_t)) == sizeof(char32_t) - 1) {
         /* Check for UTF-32 validity. Just ensure that each quadruplet of bytes defines a valid UTF-32
         character; this is fairly strict, as it requires one 00 byte every four bytes, as well as other
         restrictions. */
         std::uint32_t ch = *reinterpret_cast<std::uint32_t const *>(buf_bytes - (sizeof(char32_t) - 1));
         if ((flags & ESS_UTF32LE) && !is_codepoint_valid(byte_order::le_to_host(ch))) {
            flags &= ~static_cast<unsigned>(ESS_UTF32LE);
         }
         if ((flags & ESS_UTF32BE) && !is_codepoint_valid(byte_order::be_to_host(ch))) {
            flags &= ~static_cast<unsigned>(ESS_UTF32BE);
         }
      }

      if (flags & ESS_ISO_8859_1) {
         /* Check for ISO-8859-1 validity. This is more of a guess, since there’s a big many other encodings
         that would pass this check. */
         if ((iso88591_validity[b >> 3] & (1 << (b & 7))) == 0) {
            flags &= ~static_cast<unsigned>(ESS_ISO_8859_1);
         }
      }

      if (flags & ESS_WINDOWS_1252) {
         /* Check for Windows-1252 validity. Even more of a guess, since this considers valid even more
         characters. */
         if ((windows1252_validity[b >> 3] & (1 << (b & 7))) == 0) {
            flags &= ~static_cast<unsigned>(ESS_WINDOWS_1252);
         }
      }

      if (flags & ESS_MASK_BOMS) {
         /* Lastly, check for one or more BOMs. This needs to be last, so if it enables other checks, they
         don’t get performed on the last BOM byte it just analyzed, which would most likely cause them to
         fail. */
         for (std::size_t bom_i = 0; bom_i < LOFTY_COUNTOF(bom_scan_data); ++bom_i) {
            unsigned bom_flag = bom_scan_data[bom_i].ess_flag;
            if (flags & bom_flag) {
               if (b != bom_scan_data[bom_i].bom_bytes[byte_index]) {
                  // This byte doesn’t match: stop checking for this BOM.
                  flags &= ~bom_flag;
               } else if (byte_index == bom_scan_data[bom_i].bom_byte_size - 1u) {
                  /* This was the last BOM byte, which means that the whole BOM was matched: stop checking for
                  the BOM, and enable checking for the encoding itself. */
                  flags &= ~bom_flag;
                  flags |= bom_flag << 1;
                  /* Return the BOM length to the caller, if requested. This will be overwritten in case
                  another, longer BOM is found (e.g. the BOM in UTF-16LE is the start of the BOM in
                  UTF-32LE). */
                  if (bom_byte_size) {
                     *bom_byte_size = bom_scan_data[bom_i].bom_byte_size;
                  }
               }
            }
         }
      }
   }

   // Now, of all possibilities, pick the most likely.
   if (flags & ESS_UTF8) {
      return encoding::utf8;
   } else if (flags & ESS_UTF32LE) {
      return encoding::utf32le;
   } else if (flags & ESS_UTF32BE) {
      return encoding::utf32be;
   } else if (flags & ESS_UTF16LE) {
      return encoding::utf16le;
   } else if (flags & ESS_UTF16BE) {
      return encoding::utf16be;
   } else if (flags & ESS_ISO_8859_1) {
      return encoding::iso_8859_1;
   } else if (flags & ESS_WINDOWS_1252) {
      return encoding::windows_1252;
   } else {
      return encoding::unknown;
   }
}

line_terminator guess_line_terminator(char_t const * chars_begin, char_t const * chars_end) {
   for (auto chars = chars_begin; chars < chars_end; ++chars) {
      if (*chars == '\r') {
         /* CR can be followed by a LF to form the sequence CRLF, so check the following character (if we have
         one). If we found a CR as the very last character in the buffer, we can’t check the following one; at
         this point, we have to guess, so we’ll consider CRLF more likely than CR. */
         if (++chars < chars_end && *chars != '\n') {
            return line_terminator::cr;
         } else {
            return line_terminator::cr_lf;
         }
      } else if (*chars == '\n') {
         return line_terminator::lf;
      }
   }
   return line_terminator::any;
}

std::size_t transcode(
   bool throw_on_errors,
   encoding src_enc, void const ** src, std::size_t * src_byte_size,
   encoding dst_enc, void       ** dst, std::size_t * dst_byte_size_max
) {
   if (src_enc == encoding::unknown || dst_enc == encoding::unknown) {
      // TODO: provide more information in the exception.
      LOFTY_THROW(domain_error, ());
   }

   std::uint8_t const * src_bytes = static_cast<std::uint8_t const *>(*src);
   std::uint8_t const * src_bytes_end = src_bytes + *src_byte_size;
   std::uint8_t       * dst_bytes;
   std::uint8_t const * dst_bytes_end;
   bool write = dst != nullptr;
   if (dst_byte_size_max) {
      if (dst) {
         dst_bytes = static_cast<std::uint8_t *>(*dst);
      } else {
         dst_bytes = nullptr;
      }
      dst_bytes_end = dst_bytes + *dst_byte_size_max;
   } else {
      dst_bytes = nullptr;
      dst_bytes_end = static_cast<std::uint8_t *>(nullptr) + numeric::max<std::size_t>::value;
   }

   std::uint8_t const * last_used_src_byte_ptr;
   for (;;) {
      last_used_src_byte_ptr = src_bytes;
      char32_t ch32;

      // Decode a source code point into ch32.
      switch (src_enc.base()) {
         // unknown is here to avoid compiler warnings without using a default statement.
         case encoding::unknown:
         case encoding::utf8: {
            if (src_bytes + sizeof(char8_t) > src_bytes_end) {
               goto break_for;
            }
            std::uint8_t const * src_codepoint_begin = src_bytes;
            char8_t ch8_src = static_cast<char8_t>(*src_bytes++);
            if (!utf8_char_traits::is_trail_char(ch8_src)) {
               unsigned seq_bytes_size = utf8_char_traits::lead_char_to_codepoint_size(ch8_src);
               // Subtract 1 because we already consumed the lead character, above.
               unsigned trail_byte_size = seq_bytes_size - 1;
               // Ensure that we still have enough characters.
               if (src_bytes + trail_byte_size > src_bytes_end) {
                  goto break_for;
               }
               // Convert the first byte to an UTF-32 character.
               ch32 = utf8_char_traits::get_lead_char_codepoint_bits(ch8_src, trail_byte_size);
               // Shift in any continuation bytes.
               for (; trail_byte_size; --trail_byte_size) {
                  ch8_src = static_cast<char8_t>(*src_bytes++);
                  if (!utf8_char_traits::is_trail_char(ch8_src)) {
                     // The sequence ended prematurely, and this byte is not part of it.
                     if (throw_on_errors) {
                        LOFTY_THROW(decode_error, (
                           LOFTY_SL("unexpected end of UTF-8 sequence"),
                           src_codepoint_begin, src_codepoint_begin + seq_bytes_size
                        ));
                     }
                     // The error will be handled after this loop.
                     --src_bytes;
                     break;
                  }
                  ch32 = (ch32 << 6) | (ch8_src & 0x3f);
               }
               if (trail_byte_size == 0 && is_codepoint_valid(ch32)) {
                  // Everything went well.
                  break;
               }
               // Couldn’t read the whole code point or the result is not valid UTF-32.
               if (throw_on_errors) {
                  LOFTY_THROW(decode_error, (
                     LOFTY_SL("UTF-8 sequence decoded into invalid code point"),
                     src_codepoint_begin, src_codepoint_begin + seq_bytes_size
                  ));
               }
            } else {
               if (throw_on_errors) {
                  LOFTY_THROW(decode_error, (
                     LOFTY_SL("invalid UTF-8 lead byte"), src_codepoint_begin, src_codepoint_begin + 1
                  ));
               }
            }
            // Replace this invalid code point.
            ch32 = replacement_char;
            break;
         }

         case encoding::utf16le:
         case encoding::utf16be: {
            /* Note: this decoder could be changed to accept a single lead or trail surrogate; this however
            opens up for the possibility of not knowing, should we encounter a lead surrogate at the end of
            the buffer, whether we should consume it, or leave it alone and ask the caller to try again with
            more characters. By using the lead surrogate as a lone character, we may be corrupting the source
            by decoding lead and trail surrogates as separate characters should they be split in two separate
            reads; on the other hand, by refusing to decode a lead surrogate at the end of the buffer, we’d
            potentially cause the caller to enter an endless loop, as it may not be able to ever provide the
            trail surrogate we ask for. */

            if (src_bytes + sizeof(char16_t) > src_bytes_end) {
               goto break_for;
            }
            std::uint8_t const * src_codepoint_begin = src_bytes;
            char16_t ch16_src0(*reinterpret_cast<char16_t const *>(src_bytes));
            src_bytes += sizeof(char16_t);
            if (src_enc != encoding::utf16_host) {
               ch16_src0 = byte_order::swap(ch16_src0);
            }
            if (!utf16_char_traits::is_surrogate(ch16_src0)) {
               ch32 = ch16_src0;
               // Everything went well.
               break;
            } else if (utf16_char_traits::is_lead_surrogate(ch16_src0)) {
               // Expect to be able to read a second character, the trail surrogate.
               if (src_bytes + sizeof(char16_t) > src_bytes_end) {
                  goto break_for;
               }
               char16_t ch16_src1 = *reinterpret_cast<char16_t const *>(src_bytes);
               if (src_enc != encoding::utf16_host) {
                  ch16_src1 = byte_order::swap(ch16_src1);
               }
               if (utf16_char_traits::is_trail_char(ch16_src1)) {
                  src_bytes += sizeof(char16_t);
                  ch32 = ((static_cast<char32_t>(ch16_src0 & 0x03ff) << 10) | (ch16_src1 & 0x03ff)) + 0x10000;
                  if (is_codepoint_valid(ch32)) {
                     // Everything went well.
                     break;
                  }
                  if (throw_on_errors) {
                     LOFTY_THROW(decode_error, (
                        LOFTY_SL("UTF-16 surrogate decoded into invalid code point"),
                        src_codepoint_begin, src_codepoint_begin + sizeof(char16_t) * 2
                     ));
                  }
               } else {
                  if (throw_on_errors) {
                     LOFTY_THROW(decode_error, (
                        LOFTY_SL("invalid lone lead surrogate"),
                        src_codepoint_begin, src_codepoint_begin + sizeof(char16_t)
                     ));
                  }
               }
            } else {
               if (throw_on_errors) {
                  LOFTY_THROW(decode_error, (
                     LOFTY_SL("invalid lone trail surrogate"),
                     src_codepoint_begin, src_codepoint_begin + sizeof(char16_t)
                  ));
               }
            }
            // Replace this invalid code point.
            ch32 = replacement_char;
            break;
         }

         case encoding::utf32be:
         case encoding::utf32le:
            if (src_bytes + sizeof(char32_t) > src_bytes_end) {
               goto break_for;
            }
            ch32 = *reinterpret_cast<char32_t const *>(src_bytes);
            src_bytes += sizeof(char32_t);
            if (src_enc != encoding::utf32_host) {
               ch32 = byte_order::swap(ch32);
            }
            break;

         case encoding::iso_8859_1:
            if (src_bytes + 1 > src_bytes_end) {
               goto break_for;
            }
            ch32 = *src_bytes;
            ++src_bytes;
            break;

         case encoding::windows_1252:
            // TODO: Windows-1252 support.
            ch32 = 0;
            break;
      }

      // Encode the code point in ch32 into the destination.
      switch (dst_enc.base()) {
         // unknown is here to avoid compiler warnings without using a default statement.
         case encoding::unknown:
         case encoding::utf8: {
            /* Compute the length of this sequence. Technically this could throw if ch32 is not a valid
            Unicode code point, but we made sure above that that cannot happen. */
            std::size_t seq_bytes_size = sizeof(char8_t) * utf8_char_traits::codepoint_size(ch32);
            if (dst_bytes + seq_bytes_size > dst_bytes_end) {
               goto break_for;
            }
            if (write) {
               // We know that there’s enough room in *dst_bytes, so this is safe.
               dst_bytes = reinterpret_cast<std::uint8_t *>(utf8_char_traits::codepoint_to_chars(
                  ch32, reinterpret_cast<char8_t *>(dst_bytes)
               ));
            } else {
               dst_bytes += seq_bytes_size;
            }
            break;
         }

         case encoding::utf16le:
         case encoding::utf16be: {
            std::size_t seq_bytes_size = sizeof(char16_t) * utf16_char_traits::codepoint_size(ch32);
            if (dst_bytes + seq_bytes_size > dst_bytes_end) {
               goto break_for;
            }
            if (write) {
               bool need_surrogate = seq_bytes_size > sizeof(char16_t);
               char16_t ch16_dst0, ch16_dst1;
               if (need_surrogate) {
                  ch32 -= 0x10000;
                  ch16_dst0 = static_cast<char16_t>(0xd800 | ((ch32 & 0x0ffc00) >> 10));
                  ch16_dst1 = static_cast<char16_t>(0xdc00 |  (ch32 & 0x0003ff)       );
               } else {
                  ch16_dst0 = static_cast<char16_t>(ch32);
               }
               if (dst_enc != encoding::utf16_host) {
                  ch16_dst0 = byte_order::swap(ch16_dst0);
               }
               *reinterpret_cast<char16_t *>(dst_bytes) = ch16_dst0;
               dst_bytes += sizeof(char16_t);
               if (need_surrogate) {
                  if (dst_enc != encoding::utf16_host) {
                     ch16_dst1 = byte_order::swap(ch16_dst1);
                  }
                  *reinterpret_cast<char16_t *>(dst_bytes) = ch16_dst1;
                  dst_bytes += sizeof(char16_t);
               }
            } else {
               dst_bytes += seq_bytes_size;
            }
            break;
         }

#if LOFTY_HOST_LITTLE_ENDIAN
         case encoding::utf32be:
#else
         case encoding::utf32le:
#endif
            // The destination endianness is the opposite as the host’s.
            ch32 = byte_order::swap(ch32);
            // Fall through.
         case encoding::utf32_host:
            if (dst_bytes + sizeof(char32_t) > dst_bytes_end) {
               goto break_for;
            }
            if (write) {
               *reinterpret_cast<char32_t *>(dst_bytes) = ch32;
            }
            dst_bytes += sizeof(char32_t);
            break;

         case encoding::iso_8859_1:
            if (dst_bytes + 1 > dst_bytes_end) {
               goto break_for;
            }
            if (write) {
               // Check for code points that cannot be represented by ISO-8859-1.
               if (ch32 > 0x0000ff) {
                  if (throw_on_errors) {
                     LOFTY_THROW(encode_error, (LOFTY_SL("no transcoding available to ISO-8859-1"), ch32));
                  }
                  // Replace the code point with a question mark.
                  ch32 = 0x00003f;
               }
               *dst_bytes = static_cast<std::uint8_t>(ch32);
            }
            ++dst_bytes;
            break;

         case encoding::windows_1252:
            // TODO: Windows-1252 support.
            if (dst_bytes + 1 > dst_bytes_end) {
               goto break_for;
            }
            *dst_bytes = 0;
            break;
      }
   }
break_for:
   std::size_t dst_used_bytes;
   if (dst && dst_byte_size_max) {
      dst_used_bytes = static_cast<std::size_t>(dst_bytes - static_cast<std::uint8_t *>(*dst));
   } else {
      dst_used_bytes = reinterpret_cast<std::size_t>(dst_bytes);
   }
   if (dst_byte_size_max) {
      // Undo any incomplete or unused read.
      src_bytes = last_used_src_byte_ptr;
      // Update the variables pointed to by the arguments.
      *src_byte_size -= static_cast<std::size_t>(src_bytes - static_cast<std::uint8_t const *>(*src));
      *src = src_bytes;
      *dst_byte_size_max -= dst_used_bytes;
      if (dst) {
         *dst = dst_bytes;
      }
   }
   return dst_used_bytes;
}

}} //namespace lofty::text

namespace lofty { namespace text { namespace _pvt {

/*! Template implementation of lofty::text::size_in_chars().

@param s
   Pointer to the NUL-terminated string of which to calculate the length.
@return
   Length of the string pointed to by s, in characters.
*/
template <typename C>
std::size_t size_in_chars(C const * s) {
   auto ch = s;
   while (*ch) {
      ++ch;
   }
   return static_cast<std::size_t>(ch - s);
}

}}} //namespace lofty::text::_pvt

namespace lofty { namespace text {

std::size_t size_in_chars(char_t const * s) {
   return _pvt::size_in_chars(s);
}
#if LOFTY_HOST_UTF > 8
std::size_t size_in_chars(char const * s) {
   return _pvt::size_in_chars(s);
}
#endif

}} //namespace lofty::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

void to_text_ostream<text::file_address>::set_format(str const & format) {
   auto itr(format.cbegin());

   // Add parsing of the format string here.

   throw_on_unused_streaming_format_chars(itr, format);
}

void to_text_ostream<text::file_address>::write(text::file_address const & src, io::text::ostream * dst) {
   dst->write(str(external_buffer, src.file_path()));
   dst->write(LOFTY_SL(":"));
   dst->print(LOFTY_SL("{}"), src.line_number());
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {

/*explicit*/ error::error(errint_t err_ /*= 0*/) :
   generic_error(err_) {
}

error::error(error const & src) :
   generic_error(src) {
}

/*virtual*/ error::~error() LOFTY_STL_NOEXCEPT_TRUE() {
}

error & error::operator=(error const & src) {
   generic_error::operator=(src);
   return *this;
}

}} //namespace lofty::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {

/*explicit*/ decode_error::decode_error(
   str const & description_ /*= str::empty*/, std::uint8_t const * invalid_bytes_begin /*= nullptr*/,
   std::uint8_t const * invalid_bytes_end /*= nullptr*/, errint_t err_ /*= 0*/
) :
   error(err_),
   description(description_) {
   invalid_bytes.push_back(
      invalid_bytes_begin, static_cast<std::size_t>(invalid_bytes_end - invalid_bytes_begin)
   );

   str format;
   if (description) {
      if (invalid_bytes) {
         format = LOFTY_SL("{0}; bytes={1}");
      } else {
         format = LOFTY_SL("{0}");
      }
   } else {
      if (invalid_bytes) {
         format = LOFTY_SL("bytes={1}");
      }
   }
   if (format) {
      what_ostream().print(format, description, invalid_bytes);
   }
}

decode_error::decode_error(decode_error const & src) :
   error(src),
   description(src.description),
   invalid_bytes(src.invalid_bytes) {
}

/*virtual*/ decode_error::~decode_error() LOFTY_STL_NOEXCEPT_TRUE() {
}

decode_error & decode_error::operator=(decode_error const & src) {
   error::operator=(src);
   description = src.description;
   invalid_bytes = src.invalid_bytes;
   return *this;
}

}} //namespace lofty::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {

/*explicit*/ encode_error::encode_error(
   str const & description_ /*= str::empty*/, char32_t invalid_cp_ /*= 0xffffff*/, errint_t err_ /*= 0*/
) :
   error(err_),
   description(description_),
   invalid_cp(static_cast<std::uint32_t>(invalid_cp_)) {

   str format;
   if (description) {
      if (invalid_cp != 0xffffff) {
         format = LOFTY_SL("{0}; code point={1}");
      } else {
         format = LOFTY_SL("{0}");
      }
   } else {
      if (invalid_cp != 0xffffff) {
         format = LOFTY_SL("code point={1}");
      }
   }
   if (format) {
      what_ostream().print(format, description, invalid_cp);
   }
}

encode_error::encode_error(encode_error const & src) :
   error(src),
   description(src.description),
   invalid_cp(src.invalid_cp) {
}

/*virtual*/ encode_error::~encode_error() LOFTY_STL_NOEXCEPT_TRUE() {
}

encode_error & encode_error::operator=(encode_error const & src) {
   error::operator=(src);
   description = src.description;
   invalid_cp = src.invalid_cp;
   return *this;
}

}} //namespace lofty::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {

/*explicit*/ syntax_error::syntax_error(
   str const & description_, str const & source_ /*= str::empty*/, unsigned char_index_ /*= 0*/,
   unsigned line_number_ /*= 0*/, errint_t err_ /*= 0*/
) :
   generic_error(err_),
   description(description_),
   source(source_),
   char_index(char_index_),
   line_number(line_number_) {

   str format;
   if (source) {
      if (char_index) {
         if (line_number) {
            format = LOFTY_SL("{0} in {1}:{2}:{3}");
         } else {
            format = LOFTY_SL("{0} in expression \"{1}\", character {3}");
         }
      } else {
         if (line_number) {
            format = LOFTY_SL("{0} in {1}:{2}");
         } else {
            format = LOFTY_SL("{0} in expression \"{1}\"");
         }
      }
   } else {
      if (char_index) {
         if (line_number) {
            format = LOFTY_SL("{0} in <input>:{2}:{3}");
         } else {
            format = LOFTY_SL("{0} in <expression>, character {3}");
         }
      } else {
         if (line_number) {
            format = LOFTY_SL("{0} in <input>:{2}");
         } else {
            format = LOFTY_SL("{0}");
         }
      }
   }
   what_ostream().print(format, description, source, line_number, char_index);
}

syntax_error::syntax_error(syntax_error const & src) :
   generic_error(src),
   description(src.description),
   source(src.source),
   char_index(src.char_index),
   line_number(src.line_number) {
}

/*virtual*/ syntax_error::~syntax_error() LOFTY_STL_NOEXCEPT_TRUE() {
}

syntax_error & syntax_error::operator=(syntax_error const & src) {
   generic_error::operator=(src);
   description = src.description;
   source = src.source;
   char_index = src.char_index;
   line_number = src.line_number;
   return *this;
}

}} //namespace lofty::text
