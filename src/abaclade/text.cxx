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

#include <abaclade.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text globals

namespace abc {
namespace text {

std::size_t get_encoding_size(encoding enc) {
   ABC_TRACE_FUNC(enc);

   // Little helper to map abc::text::encoding values with byte sizes (see below).
   struct enc_cb_t {
      std::uint8_t enc;
      std::uint8_t cb;
   };

   // Character size, in bytes, for each recognized encoding.
   static enc_cb_t const sc_aecEncChar[] = {
      { encoding::utf8,         1 },
      { encoding::utf16le,      2 },
      { encoding::utf16be,      2 },
      { encoding::utf32le,      4 },
      { encoding::utf32be,      4 },
      { encoding::iso_8859_1,   1 },
      { encoding::windows_1252, 1 }
   };
   // TODO: improve search algorithm, or maybe use a real map.
   for (std::size_t i(0); i < ABC_COUNTOF(sc_aecEncChar); ++i) {
      if (enc == encoding::enum_type(sc_aecEncChar[i].enc)) {
         return sc_aecEncChar[i].cb;
      }
   }
   // TODO: provide more information in the exception.
   ABC_THROW(domain_error, ());
}


istr get_line_terminator_str(line_terminator lterm) {
   ABC_TRACE_FUNC(lterm);

   if (lterm == line_terminator::any || lterm == line_terminator::convert_any_to_lf) {
      lterm = line_terminator::host;
   }
   switch (lterm.base()) {
      case abc::text::line_terminator::cr:
         return istr(ABC_SL("\r"));
      case abc::text::line_terminator::lf:
         return istr(ABC_SL("\n"));
      case abc::text::line_terminator::cr_lf:
         return istr(ABC_SL("\r\n"));
      default:
         // TODO: provide more information in the exception.
         ABC_THROW(domain_error, ());
   }
}


encoding guess_encoding(
   void const * pBufBegin, void const * pBufEnd, std::size_t cbSrcTotal /*= 0*/,
   std::size_t * pcbBom /*= nullptr*/
) {
   ABC_TRACE_FUNC(pBufBegin, pBufEnd, cbSrcTotal, pcbBom);

   std::uint8_t const * pbBufBegin(static_cast<std::uint8_t const *>(pBufBegin));
   std::uint8_t const * pbBufEnd(static_cast<std::uint8_t const *>(pBufEnd));
   // If the total size is not specified, assume that the buffer is the whole source.
   if (!cbSrcTotal) {
      cbSrcTotal = static_cast<std::size_t>(pbBufEnd - pbBufBegin);
   }

   // Statuses for the scanner. Each BOM status must be 1 bit to the right of its resulting
   // encoding; LE variants must be 2 bits to the right of their BE counterparts.
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
   static std::uint8_t const sc_abValidISO88591[] = {
      0x80, 0x3e, 0x00, 0x08, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
      0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
   };
   // A 1 in this bit array means that the corresponding byte value is valid in Windows-1252.
   static std::uint8_t const sc_abValidWindows1252[] = {
      0x80, 0x3e, 0x00, 0x08, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
      0xfd, 0x5f, 0xfe, 0xdf, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
   };
   // BOMs for sc_absd.
   static std::uint8_t const
      sc_abUtf8Bom   [] = { 0xef, 0xbb, 0xbf },
      sc_abUtf16leBom[] = { 0xff, 0xfe },
      sc_abUtf16beBom[] = { 0xfe, 0xff },
      sc_abUtf32leBom[] = { 0xff, 0xfe, 0x00, 0x00 },
      sc_abUtf32beBom[] = { 0x00, 0x00, 0xfe, 0xff };
   // Struct to uniformize scanning for BOMs.
   static struct bomscandata_t {
      std::uint8_t const * pabBom;
      std::uint16_t ess;
      std::uint8_t cbBom;
   } const sc_absd[] = {
      { sc_abUtf8Bom,    ESS_UTF8_BOM,    sizeof(sc_abUtf8Bom   ) },
      { sc_abUtf16leBom, ESS_UTF16LE_BOM, sizeof(sc_abUtf16leBom) },
      { sc_abUtf16beBom, ESS_UTF16BE_BOM, sizeof(sc_abUtf16beBom) },
      { sc_abUtf32leBom, ESS_UTF32LE_BOM, sizeof(sc_abUtf32leBom) },
      { sc_abUtf32beBom, ESS_UTF32BE_BOM, sizeof(sc_abUtf32beBom) }
   };


   // Initially, consider anything that doesn’t require a BOM.
   unsigned fess(ESS_MASK_START);

   // Initially, assume no BOM will be found.
   if (pcbBom) {
      *pcbBom = 0;
   }

   // Easy checks.
   if (cbSrcTotal & (sizeof(char32_t) - 1)) {
      // UTF-32 requires a number of bytes multiple of sizeof(char32_t).
      fess &= ~static_cast<unsigned>(ESS_MASK_UTF32);
      if (cbSrcTotal & (sizeof(char16_t) - 1)) {
         // UTF-16 requires an even number of bytes.
         fess &= ~static_cast<unsigned>(ESS_MASK_UTF16);
      }
   }

   // Parse every byte, gradually excluding more and more possibilities, hopefully ending with
   // exactly one guess.
   unsigned cbUtf8Cont(0);
   std::size_t ib(0);
   for (std::uint8_t const * pbBuf(pbBufBegin); pbBuf < pbBufEnd; ++pbBuf, ++ib) {
      std::uint8_t b(*pbBuf);

      if (fess & ESS_UTF8) {
         // Check for UTF-8 validity. Checking for overlongs or invalid code points is out of scope
         // here.
         if (cbUtf8Cont) {
            if (!utf8_char_traits::is_trail_char(static_cast<char8_t>(b))) {
               // This byte should be part of a sequence, but it’s not.
               fess &= ~static_cast<unsigned>(ESS_UTF8);
            } else {
               --cbUtf8Cont;
            }
         } else {
            if (utf8_char_traits::is_trail_char(static_cast<char8_t>(b))) {
               // This byte should be a lead byte, but it’s not.
               fess &= ~static_cast<unsigned>(ESS_UTF8);
            } else {
               cbUtf8Cont = utf8_char_traits::lead_char_to_codepoint_size(
                  static_cast<char8_t>(b)
               ) - 1;
               if ((b & 0x80) && cbUtf8Cont == 0) {
                  // By utf8_char_traits::lead_char_to_codepoint_size(), a non-ASCII byte that
                  // doesn’t have a continuation is an invalid one.
                  fess &= ~static_cast<unsigned>(ESS_UTF8);
               }
            }
         }
      }

      if (fess & (ESS_UTF16LE | ESS_UTF16BE)) {
         // Check for UTF-16 validity. The only check possible is proper ordering of surrogates;
         // everything else is allowed.
         for (unsigned ess(ESS_UTF16LE); ess <= ESS_UTF16BE; ess <<= 2) {
            // This will go ahead with the check if ib is indexing the most significant byte, i.e.
            // odd for LE and even for BE.
            if ((fess & ess) && ((ib & sizeof(char16_t)) != 0) == (ess != ESS_UTF16LE)) {
               switch (b & 0xfc) {
                  case 0xd8: {
                     // There must be a trail surrogate after 1 byte.
                     std::uint8_t const * pbNext(pbBuf + sizeof(char16_t));
                     if (pbNext >= pbBufEnd || (*pbNext & 0xfc) != 0xdc) {
                        fess &= ~ess;
                     }
                     break;
                  }
                  case 0xdc: {
                     // Assume there was a lead surrogate 2 bytes before.
                     std::uint8_t const * pbPrev(pbBuf - sizeof(char16_t));
                     if (pbPrev < pbBufBegin || (*pbPrev & 0xfc) != 0xd8) {
                        fess &= ~ess;
                     }
                     break;
                  }
               }
            }
         }
      }

      if ((fess & (ESS_UTF32LE | ESS_UTF32BE)) && (ib & sizeof(char32_t)) == sizeof(char32_t) - 1) {
         // Check for UTF-32 validity. Just ensure that each quadruplet of bytes defines a valid
         // UTF-32 character; this is fairly strict, as it requires one 00 byte every four bytes, as
         // well as other restrictions.
         std::uint32_t ch(*reinterpret_cast<std::uint32_t const *>(pbBuf - (sizeof(char32_t) - 1)));
         if ((fess & ESS_UTF32LE) && !is_codepoint_valid(byteorder::le_to_host(ch))) {
            fess &= ~static_cast<unsigned>(ESS_UTF32LE);
         }
         if ((fess & ESS_UTF32BE) && !is_codepoint_valid(byteorder::be_to_host(ch))) {
            fess &= ~static_cast<unsigned>(ESS_UTF32BE);
         }
      }

      if (fess & ESS_ISO_8859_1) {
         // Check for ISO-8859-1 validity. This is more of a guess, since there’s a big many other
         // encodings that would pass this check.
         if ((sc_abValidISO88591[b >> 3] & (1 << (b & 7))) == 0) {
            fess &= ~static_cast<unsigned>(ESS_ISO_8859_1);
         }
      }

      if (fess & ESS_WINDOWS_1252) {
         // Check for Windows-1252 validity. Even more of a guess, since this considers valid even
         // more characters.
         if ((sc_abValidWindows1252[b >> 3] & (1 << (b & 7))) == 0) {
            fess &= ~static_cast<unsigned>(ESS_WINDOWS_1252);
         }
      }

      if (fess & ESS_MASK_BOMS) {
         // Lastly, check for one or more BOMs. This needs to be last, so if it enables other
         // checks, they don’t get performed on the last BOM byte it just analyzed, which would most
         // likely cause them to fail.
         for (std::size_t iBsd(0); iBsd < ABC_COUNTOF(sc_absd); ++iBsd) {
            unsigned essBom(sc_absd[iBsd].ess);
            if (fess & essBom) {
               if (b != sc_absd[iBsd].pabBom[ib]) {
                  // This byte doesn’t match: stop checking for this BOM.
                  fess &= ~essBom;
               } else if (ib == sc_absd[iBsd].cbBom - 1u) {
                  // This was the last BOM byte, which means that the whole BOM was matched: stop
                  // checking for the BOM, and enable checking for the encoding itself.
                  fess &= ~essBom;
                  fess |= essBom << 1;
                  // Return the BOM length to the caller, if requested. This will be overwritten in
                  // case another, longer BOM is found (e.g. the BOM in UTF-16LE is the start of the
                  // BOM in UTF-32LE).
                  if (pcbBom) {
                     *pcbBom = sc_absd[iBsd].cbBom;
                  }
               }
            }
         }
      }
   }

   // Now, of all possibilities, pick the most likely.
   if (fess & ESS_UTF8) {
      return encoding::utf8;
   } else if (fess & ESS_UTF32LE) {
      return encoding::utf32le;
   } else if (fess & ESS_UTF32BE) {
      return encoding::utf32be;
   } else if (fess & ESS_UTF16LE) {
      return encoding::utf16le;
   } else if (fess & ESS_UTF16BE) {
      return encoding::utf16be;
   } else if (fess & ESS_ISO_8859_1) {
      return encoding::iso_8859_1;
   } else if (fess & ESS_WINDOWS_1252) {
      return encoding::windows_1252;
   } else {
      return encoding::unknown;
   }
}


line_terminator guess_line_terminator(char_t const * pchBegin, char_t const * pchEnd) {
   ABC_TRACE_FUNC(pchBegin, pchEnd);

   for (char_t const * pch(pchBegin); pch < pchEnd; ++pch) {
      char_t ch(*pch);
      if (ch == '\r') {
         // CR can be followed by a LF to form the sequence CRLF, so check the following character
         // (if we have one). If we found a CR as the very last character in the buffer, we can’t
         // check the following one; at this point, we have to guess, so we’ll consider CRLF more
         // likely than CR.
         if (++pch < pchEnd && *pch != '\n') {
            return line_terminator::cr;
         } else {
            return line_terminator::cr_lf;
         }
      } else if (ch == '\n') {
         return line_terminator::lf;
      }
   }
   return line_terminator::any;
}


std::size_t transcode(
   bool bThrowOnErrors,
   encoding encSrc, void const ** ppSrc, std::size_t * pcbSrc,
   encoding encDst, void       ** ppDst, std::size_t * pcbDstMax
) {
   ABC_TRACE_FUNC(bThrowOnErrors, encSrc, ppSrc, pcbSrc, encDst, ppDst, pcbDstMax);

   std::uint8_t const * pbSrc(static_cast<std::uint8_t const *>(*ppSrc));
   std::uint8_t const * pbSrcEnd(pbSrc + *pcbSrc);
   std::uint8_t       * pbDst;
   std::uint8_t const * pbDstEnd;
   bool bWrite(ppDst);
   if (pcbDstMax) {
      if (ppDst) {
         pbDst = static_cast<std::uint8_t *>(*ppDst);
      } else {
         pbDst = nullptr;
      }
      pbDstEnd = pbDst + *pcbDstMax;
   } else {
      pbDst = nullptr;
      pbDstEnd = static_cast<std::uint8_t *>(nullptr) + numeric::max<std::size_t>::value;
   }

   std::uint8_t const * pbSrcLastUsed;
   for (;;) {
      pbSrcLastUsed = pbSrc;
      char32_t ch32;

      // Decode a source code point into ch32.
      switch (encSrc.base()) {
         case encoding::utf8: {
            if (pbSrc + sizeof(char8_t) > pbSrcEnd) {
               goto break_for;
            }
            std::uint8_t const * pbSrcCpBegin(pbSrc);
            char8_t ch8Src(static_cast<char8_t>(*pbSrc++));
            if (!utf8_char_traits::is_trail_char(ch8Src)) {
               unsigned cbSeq(utf8_char_traits::lead_char_to_codepoint_size(ch8Src));
               // Subtract 1 because we already consumed the lead character, above.
               unsigned cbTrail(cbSeq - 1);
               // Ensure that we still have enough characters.
               if (pbSrc + cbTrail > pbSrcEnd) {
                  goto break_for;
               }
               // Convert the first byte to an UTF-32 character.
               ch32 = utf8_char_traits::get_lead_char_codepoint_bits(ch8Src, cbTrail);
               // Shift in any continuation bytes.
               for (; cbTrail; --cbTrail) {
                  ch8Src = static_cast<char8_t>(*pbSrc++);
                  if (!utf8_char_traits::is_trail_char(ch8Src)) {
                     // The sequence ended prematurely, and this byte is not part of it.
                     if (bThrowOnErrors) {
                        ABC_THROW(decode_error, (
                           ABC_SL("unexpected end of UTF-8 sequence"),
                           pbSrcCpBegin, pbSrcCpBegin + cbSeq
                        ));
                     }
                     // The error will be handled after this loop.
                     --pbSrc;
                     break;
                  }
                  ch32 = (ch32 << 6) | (ch8Src & 0x3f);
               }
               if (!cbTrail && is_codepoint_valid(ch32)) {
                  // Everything went well.
                  break;
               }
               // Couldn’t read the whole code point or the result is not valid UTF-32.
               if (bThrowOnErrors) {
                  ABC_THROW(decode_error, (
                     ABC_SL("UTF-8 sequence decoded into invalid code point"),
                     pbSrcCpBegin, pbSrcCpBegin + cbSeq
                  ));
               }
            } else {
               if (bThrowOnErrors) {
                  ABC_THROW(decode_error, (
                     ABC_SL("invalid UTF-8 lead byte"), pbSrcCpBegin, pbSrcCpBegin + 1
                  ));
               }
            }
            // Replace this invalid code point.
            ch32 = replacement_char;
            break;
         }

         case encoding::utf16le:
         case encoding::utf16be: {
            // Note: this decoder could be changed to accept a single lead or trail surrogate; this
            // however opens up for the possibility of not knowing, should we encounter a lead
            // surrogate at the end of the buffer, whether we should consume it, or leave it alone
            // and ask the caller to try again with more characters. By using the lead surrogate as
            // a lone character, we may be corrupting the source by decoding lead and trail
            // surrogates as separate characters should they be split in two separate reads; on the
            // other hand, by refusing to decode a lead surrogate at the end of the buffer, we’d
            // potentially cause the caller to enter an endless loop, as it may not be able to ever
            // provide the trail surrogate we ask for.

            if (pbSrc + sizeof(char16_t) > pbSrcEnd) {
               goto break_for;
            }
            std::uint8_t const * pbSrcCpBegin(pbSrc);
            char16_t ch16Src0(*reinterpret_cast<char16_t const *>(pbSrc));
            pbSrc += sizeof(char16_t);
            if (encSrc != encoding::utf16_host) {
               ch16Src0 = byteorder::swap(ch16Src0);
            }
            if (!utf16_char_traits::is_surrogate(ch16Src0)) {
               ch32 = ch16Src0;
               // Everything went well.
               break;
            } else if (utf16_char_traits::is_lead_surrogate(ch16Src0)) {
               // Expect to be able to read a second character, the trail surrogate.
               if (pbSrc + sizeof(char16_t) > pbSrcEnd) {
                  goto break_for;
               }
               char16_t ch16Src1(*reinterpret_cast<char16_t const *>(pbSrc));
               if (encSrc != encoding::utf16_host) {
                  ch16Src1 = byteorder::swap(ch16Src1);
               }
               if (utf16_char_traits::is_trail_char(ch16Src1)) {
                  pbSrc += sizeof(char16_t);
                  ch32 = (
                     (static_cast<char32_t>(ch16Src0 & 0x03ff) << 10) | (ch16Src1 & 0x03ff)
                  ) + 0x10000;
                  if (is_codepoint_valid(ch32)) {
                     // Everything went well.
                     break;
                  }
                  if (bThrowOnErrors) {
                     ABC_THROW(decode_error, (
                        ABC_SL("UTF-16 surrogate decoded into invalid code point"),
                        pbSrcCpBegin, pbSrcCpBegin + sizeof(char16_t) * 2
                     ));
                  }
               } else {
                  if (bThrowOnErrors) {
                     ABC_THROW(decode_error, (
                        ABC_SL("invalid lone lead surrogate"),
                        pbSrcCpBegin, pbSrcCpBegin + sizeof(char16_t)
                     ));
                  }
               }
            } else {
               if (bThrowOnErrors) {
                  ABC_THROW(decode_error, (
                     ABC_SL("invalid lone trail surrogate"),
                     pbSrcCpBegin, pbSrcCpBegin + sizeof(char16_t)
                  ));
               }
            }
            // Replace this invalid code point.
            ch32 = replacement_char;
            break;
         }

         case encoding::utf32be:
         case encoding::utf32le:
            if (pbSrc + sizeof(char32_t) > pbSrcEnd) {
               goto break_for;
            }
            ch32 = *reinterpret_cast<char32_t const *>(pbSrc);
            pbSrc += sizeof(char32_t);
            if (encSrc != encoding::utf32_host) {
               ch32 = byteorder::swap(ch32);
            }
            break;

         case encoding::iso_8859_1:
            if (pbSrc + 1 > pbSrcEnd) {
               goto break_for;
            }
            ch32 = *pbSrc;
            ++pbSrc;
            break;

         case encoding::windows_1252:
            // TODO: Windows-1252 support.
            break;

         default:
            // TODO: support more character sets/encodings?
            break;
      }

      // Encode the code point in ch32 into the destination.
      switch (encDst.base()) {
         case encoding::utf8: {
            // Compute the length of this sequence. Technically this could throw if ch32 is not a
            // valid Unicode code point, but we made sure above that that cannot happen.
            std::size_t cbSeq(sizeof(char8_t) * utf8_char_traits::codepoint_size(ch32));
            if (pbDst + cbSeq > pbDstEnd) {
               goto break_for;
            }
            if (bWrite) {
               // We know that there’s enough room in *pbDst, so this is safe.
               pbDst = reinterpret_cast<std::uint8_t *>(utf8_char_traits::codepoint_to_chars(
                  ch32, reinterpret_cast<char8_t *>(pbDst)
               ));
            } else {
               pbDst += cbSeq;
            }
            break;
         }

         case encoding::utf16le:
         case encoding::utf16be: {
            std::size_t cbSeq(sizeof(char16_t) * utf16_char_traits::codepoint_size(ch32));
            if (pbDst + cbSeq > pbDstEnd) {
               goto break_for;
            }
            if (bWrite) {
               bool bNeedSurrogate(cbSeq > sizeof(char16_t));
               char16_t ch16Dst0, ch16Dst1;
               if (bNeedSurrogate) {
                  ch32 -= 0x10000;
                  ch16Dst0 = static_cast<char16_t>(0xd800 | ((ch32 & 0x0ffc00) >> 10));
                  ch16Dst1 = static_cast<char16_t>(0xdc00 |  (ch32 & 0x0003ff)       );
               } else {
                  ch16Dst0 = static_cast<char16_t>(ch32);
               }
               if (encDst != encoding::utf16_host) {
                  ch16Dst0 = byteorder::swap(ch16Dst0);
               }
               *reinterpret_cast<char16_t *>(pbDst) = ch16Dst0;
               pbDst += sizeof(char16_t);
               if (bNeedSurrogate) {
                  if (encDst != encoding::utf16_host) {
                     ch16Dst1 = byteorder::swap(ch16Dst1);
                  }
                  *reinterpret_cast<char16_t *>(pbDst) = ch16Dst1;
                  pbDst += sizeof(char16_t);
               }
            } else {
               pbDst += cbSeq;
            }
            break;
         }

#if ABC_HOST_LITTLE_ENDIAN
         case encoding::utf32be:
#else
         case encoding::utf32le:
#endif
            // The destination endianness is the opposite of the host’s.
            ch32 = byteorder::swap(ch32);
            // Fall through.
         case encoding::utf32_host:
            if (pbDst + sizeof(char32_t) > pbDstEnd) {
               goto break_for;
            }
            if (bWrite) {
               *reinterpret_cast<char32_t *>(pbDst) = ch32;
            }
            pbDst += sizeof(char32_t);
            break;

         case encoding::iso_8859_1:
            if (pbDst + 1 > pbDstEnd) {
               goto break_for;
            }
            if (bWrite) {
               // Check for code points that cannot be represented by ISO-8859-1.
               if (ch32 > 0x0000ff) {
                  if (bThrowOnErrors) {
                     ABC_THROW(
                        encode_error, (ABC_SL("no transcoding available to ISO-8859-1"), ch32)
                     );
                  }
                  // Replace the code point with a question mark.
                  ch32 = 0x00003f;
               }
               *pbDst = static_cast<std::uint8_t>(ch32);
            }
            ++pbDst;
            break;

         case encoding::windows_1252:
            // TODO: Windows-1252 support.
            break;

         default:
            // TODO: support more character sets/encodings?
            break;
      }
   }
break_for:
   std::size_t cbDstUsed;
   if (ppDst && pcbDstMax) {
      cbDstUsed = static_cast<std::size_t>(pbDst - static_cast<std::uint8_t *>(*ppDst));
   } else {
      cbDstUsed = reinterpret_cast<std::size_t>(pbDst);
   }
   if (pcbDstMax) {
      // Undo any incomplete or unused read.
      pbSrc = pbSrcLastUsed;
      // Update the variables pointed to by the arguments.
      *pcbSrc -= static_cast<std::size_t>(pbSrc - static_cast<std::uint8_t const *>(*ppSrc));
      *ppSrc = pbSrc;
      *pcbDstMax -= cbDstUsed;
      if (ppDst) {
         *ppDst = pbDst;
      }
   }
   return cbDstUsed;
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::error


namespace abc {
namespace text {

error::error() :
   generic_error() {
   m_pszWhat = "abc::text::error";
}


void error::init(errint_t err /*= 0*/) {
   generic_error::init(err ? err : os_error_mapping<error>::mapped_error);
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::decode_error


namespace abc {
namespace text {

decode_error::decode_error() :
   generic_error(),
   error() {
   m_pszWhat = "abc::decode_error";
}
decode_error::decode_error(decode_error const & x) :
   generic_error(x),
   error(x),
   m_sDescription(x.m_sDescription),
   m_viInvalid(x.m_viInvalid) {
}


decode_error & decode_error::operator=(decode_error const & x) {
   ABC_TRACE_FUNC(this/*, x*/);

   error::operator=(x);
   m_sDescription = x.m_sDescription;
   m_viInvalid = x.m_viInvalid;
   return *this;
}


void decode_error::init(
   istr const & sDescription /*= istr()*/, std::uint8_t const * pbInvalidBegin /*= nullptr*/,
   std::uint8_t const * pbInvalidEnd /*= nullptr*/, errint_t err /*= 0*/
) {
   error::init(err ? err : os_error_mapping<decode_error>::mapped_error);
   m_sDescription = sDescription;
   m_viInvalid.append(pbInvalidBegin, static_cast<std::size_t>(pbInvalidEnd - pbInvalidBegin));
}


void decode_error::_print_extended_info(io::text::writer * ptwOut) const {
   istr sFormat;
   if (m_sDescription) {
      if (m_viInvalid) {
         sFormat = ABC_SL("{0}: byte dump: {1}\n");
      } else {
         sFormat = ABC_SL("{0}\n");
      }
   } else {
      if (m_viInvalid) {
         sFormat = ABC_SL("byte dump: {1}\n");
      }
   }

   if (sFormat) {
      ptwOut->print(sFormat, m_sDescription, m_viInvalid);
   }
   error::_print_extended_info(ptwOut);
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::encode_error


namespace abc {
namespace text {

encode_error::encode_error() :
   generic_error(),
   error() {
   m_pszWhat = "abc::encode_error";
}
encode_error::encode_error(encode_error const & x) :
   generic_error(x),
   error(x),
   m_sDescription(x.m_sDescription),
   m_iInvalidCodePoint(x.m_iInvalidCodePoint) {
}


encode_error & encode_error::operator=(encode_error const & x) {
   ABC_TRACE_FUNC(this/*, x*/);

   error::operator=(x);
   m_sDescription = x.m_sDescription;
   m_iInvalidCodePoint = x.m_iInvalidCodePoint;
   return *this;
}


void encode_error::init(
   istr const & sDescription /*= istr()*/, char32_t chInvalid /*= 0xffffff*/, errint_t err /*= 0*/
) {
   error::init(err ? err : os_error_mapping<encode_error>::mapped_error);
   m_sDescription = sDescription;
   m_iInvalidCodePoint = static_cast<std::uint32_t>(chInvalid);
}


void encode_error::_print_extended_info(io::text::writer * ptwOut) const {
   istr sFormat;
   if (m_sDescription) {
      if (m_iInvalidCodePoint != 0xffffff) {
         sFormat = ABC_SL("{0}: code point: {1}\n");
      } else {
         sFormat = ABC_SL("{0}\n");
      }
   } else {
      if (m_iInvalidCodePoint != 0xffffff) {
         sFormat = ABC_SL("code point: {1}\n");
      }
   }

   if (sFormat) {
      ptwOut->print(sFormat, m_sDescription, m_iInvalidCodePoint);
   }
   error::_print_extended_info(ptwOut);
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

