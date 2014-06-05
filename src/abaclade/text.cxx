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

ABCAPI size_t estimate_transcoded_size(
   encoding encSrc, void const * pSrc, size_t cbSrc, encoding encDst
) {
   ABC_TRACE_FUNC(encSrc, pSrc, cbSrc, encDst);

   // Little helper to map abc::text::encoding values with byte sizes (see below).
   struct enc_cb_t {
      uint8_t enc;
      uint8_t cb;
   };

   // Average size, in bytes, of 10 characters in each supported encoding.
   static enc_cb_t const sc_aecAvg10Chars[] = {
      { encoding::utf8,         15 }, // Some languages require 3 bytes per character.
      { encoding::utf16le,      20 }, // Consider surrogates extremely unlikely, as they are.
      { encoding::utf16be,      20 }, // Same as encoding::utf16le.
      { encoding::utf32le,      40 }, // Exact constant.
      { encoding::utf32be,      40 }, // Same as encoding::utf32le.
      { encoding::iso_8859_1,   10 }, // Exact constant.
      { encoding::windows_1252, 10 }, // Exact constant.
      { encoding::ebcdic,       10 }, // Exact constant.
   };

   if (encSrc == encoding::unknown) {
      ABC_THROW(argument_error, ());
   }
   if (encDst == encoding::unknown) {
      ABC_THROW(argument_error, ());
   }
   // TODO: use this to give a more accurate estimate for UTF-8, by evaluating which language block
   // seems to be dominant in the source.
   ABC_UNUSED_ARG(pSrc);

   size_t cbSrcAvg, cbDstAvg;
   // Estimate the number of code points in the source.
   // TODO: improve search algorithm, or maybe use a real map.
   for (size_t i(0); i < ABC_COUNTOF(sc_aecAvg10Chars); ++i) {
      encoding::enum_type enc(encoding::enum_type(sc_aecAvg10Chars[i].enc));
      if (encSrc == enc) {
         cbSrcAvg = sc_aecAvg10Chars[i].cb;
      }
      if (encDst == enc) {
         cbDstAvg = sc_aecAvg10Chars[i].cb;
      }
   }
   // If we were using floating-point math, this would be the return statement’s expression:
   //    ceil(cbSrc / cbSrcAvg) * cbDstAvg
   // We need to emulate ceil() on integers with:
   //    (cbSrc + cbSrcAvg - 1) / cbSrcAvg
   // Also, we have to multiply first, to avoid underflow, but this would expose to overflow, in
   // which case we’ll divide first, as in the original expression.
   size_t ccp(cbSrc * cbDstAvg);
   if (ccp >= cbSrc) {
      // No integer overflow.
      return (ccp + cbSrcAvg - 1) / cbSrcAvg;
   } else {
      // Integer overflow occurred: evaluate the expression in the original order.
      return ((cbSrc + cbSrcAvg - 1) / cbSrcAvg) * cbDstAvg;
   }
}


ABCAPI size_t get_encoding_size(encoding enc) {
   ABC_TRACE_FUNC(enc);

   // Little helper to map abc::text::encoding values with byte sizes (see below).
   struct enc_cb_t {
      uint8_t enc;
      uint8_t cb;
   };

   // Character size, in bytes, for each recognized encoding.
   static enc_cb_t const sc_aecEncChar[] = {
      { encoding::utf8,         1 },
      { encoding::utf16le,      2 },
      { encoding::utf16be,      2 },
      { encoding::utf32le,      4 },
      { encoding::utf32be,      4 },
      { encoding::iso_8859_1,   1 },
      { encoding::windows_1252, 1 },
      { encoding::ebcdic,       1 },
   };
   // TODO: improve search algorithm, or maybe use a real map.
   for (size_t i(0); i < ABC_COUNTOF(sc_aecEncChar); ++i) {
      if (enc == encoding::enum_type(sc_aecEncChar[i].enc)) {
         return sc_aecEncChar[i].cb;
      }
   }
   // TODO: provide more information in the exception.
   ABC_THROW(domain_error, ());
}


ABCAPI istr get_line_terminator_str(line_terminator lterm) {
   ABC_TRACE_FUNC(lterm);

   switch (lterm.base()) {
      case abc::text::line_terminator::cr:
         return istr(SL("\r"));
      case abc::text::line_terminator::lf:
         return istr(SL("\n"));
      case abc::text::line_terminator::cr_lf:
         return istr(SL("\r\n"));
      default:
         // TODO: provide more information in the exception.
         ABC_THROW(domain_error, ());
   }
}


ABCAPI encoding guess_encoding(
   void const * pBufBegin, void const * pBufEnd, size_t cbSrcTotal /*= 0*/,
   size_t * pcbBom /*= nullptr*/
) {
   ABC_TRACE_FUNC(pBufBegin, pBufEnd, cbSrcTotal, pcbBom);

   uint8_t const * pbBufBegin(static_cast<uint8_t const *>(pBufEnd));
   uint8_t const * pbBufEnd(static_cast<uint8_t const *>(pBufEnd));
   // If the total size is not specified, assume that the buffer is the whole source.
   if (!cbSrcTotal) {
      cbSrcTotal = size_t(pbBufEnd - pbBufBegin);
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
   static uint8_t const sc_abValidISO88591[] = {
      0x80, 0x3e, 0x00, 0x08, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
      0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
   };
   // A 1 in this bit array means that the corresponding byte value is valid in Windows-1252.
   static uint8_t const sc_abValidWindows1252[] = {
      0x80, 0x3e, 0x00, 0x08, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
      0xfd, 0x5f, 0xfe, 0xdf, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
   };
   // BOMs for sc_absd.
   static uint8_t const
      sc_abUtf8Bom   [] = { 0xef, 0xbb, 0xbf },
      sc_abUtf16leBom[] = { 0xff, 0xfe },
      sc_abUtf16beBom[] = { 0xfe, 0xff },
      sc_abUtf32leBom[] = { 0xff, 0xfe, 0x00, 0x00 },
      sc_abUtf32beBom[] = { 0x00, 0x00, 0xfe, 0xff };
   // Struct to uniformize scanning for BOMs.
   static struct bomscandata_t {
      uint8_t const * pabBom;
      unsigned short ess;
      uint8_t cbBom;
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
      fess &= ~unsigned(ESS_MASK_UTF32);
      if (cbSrcTotal & (sizeof(char16_t) - 1)) {
         // UTF-16 requires an even number of bytes.
         fess &= ~unsigned(ESS_MASK_UTF16);
      }
   }

   // Parse every byte, gradually excluding more and more possibilities, hopefully ending with
   // exactly one guess.
   unsigned cbUtf8Cont(0);
   size_t ib(0);
   for (uint8_t const * pbBuf(pbBufBegin); pbBuf < pbBufEnd; ++pbBuf, ++ib) {
      uint8_t b(*pbBuf);

      if (fess & ESS_UTF8) {
         // Check for UTF-8 validity. Checking for overlongs or invalid code points is out of scope
         // here.
         if (cbUtf8Cont) {
            if ((b & 0xc0) != 0x80) {
               // This byte should be part of a sequence, but it’s not.
               fess &= ~unsigned(ESS_UTF8);
            } else {
               --cbUtf8Cont;
            }
         } else {
            if ((b & 0xc0) == 0x80) {
               // This byte should be a leading byte, but it’s not.
               fess &= ~unsigned(ESS_UTF8);
            } else {
               cbUtf8Cont = utf8_traits::leading_to_cont_length(char8_t(b));
               if ((b & 0x80) && !cbUtf8Cont) {
                  // By utf8_traits::leading_to_cont_length(), a non-ASCII byte that doesn’t have a
                  // continuation is an invalid one.
                  fess &= ~unsigned(ESS_UTF8);
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
                     uint8_t const * pbNext(pbBuf + sizeof(char16_t));
                     if (pbNext >= pbBufEnd || (*pbNext & 0xfc) != 0xdc) {
                        fess &= ~ess;
                     }
                     break;
                  }
                  case 0xdc: {
                     // Assume there was a lead surrogate 2 bytes before.
                     uint8_t const * pbPrev(pbBuf - sizeof(char16_t));
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
         uint32_t ch(*reinterpret_cast<uint32_t const *>(pbBuf - (sizeof(char32_t) - 1)));
         if ((fess & ESS_UTF32LE) && !utf32_traits::is_valid(byteorder::le_to_host(ch))) {
            fess &= ~unsigned(ESS_UTF32LE);
         }
         if ((fess & ESS_UTF32BE) && !utf32_traits::is_valid(byteorder::be_to_host(ch))) {
            fess &= ~unsigned(ESS_UTF32BE);
         }
      }

      if (fess & ESS_ISO_8859_1) {
         // Check for ISO-8859-1 validity. This is more of a guess, since there’s a big many other
         // encodings that would pass this check.
         if ((sc_abValidISO88591[b >> 3] & (1 << (b & 7))) == 0) {
            fess &= ~unsigned(ESS_ISO_8859_1);
         }
      }

      if (fess & ESS_WINDOWS_1252) {
         // Check for Windows-1252 validity. Even more of a guess, since this considers valid even
         // more characters.
         if ((sc_abValidWindows1252[b >> 3] & (1 << (b & 7))) == 0) {
            fess &= ~unsigned(ESS_WINDOWS_1252);
         }
      }

      if (fess & ESS_MASK_BOMS) {
         // Lastly, check for one or more BOMs. This needs to be last, so if it enables other
         // checks, they don’t get performed on the last BOM byte it just analyzed, which would most
         // likely cause them to fail.
         for (size_t iBsd(0); iBsd < ABC_COUNTOF(sc_absd); ++iBsd) {
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


ABCAPI line_terminator guess_line_terminator(char_t const * pchBegin, char_t const * pchEnd) {
   ABC_TRACE_FUNC(pchBegin, pchEnd);

   for (char_t const * pch(pchBegin); pch < pchEnd; ++pch) {
      char_t ch(*pch);
      if (ch == CL('\r')) {
         // CR can be followed by a LF to form the sequence CRLF, so check the following character
         // (if we have one). If we found a CR as the very last character in the buffer, we can’t
         // check the following one; at this point, we have to guess, so we’ll consider CRLF more
         // likely than CR.
         if (++pch < pchEnd && *pch != CL('\n')) {
            return line_terminator::cr;
         } else {
            return line_terminator::cr_lf;
         }
      } else if (ch == CL('\n')) {
         return line_terminator::lf;
      }
   }
   return line_terminator::unknown;
}


ABCAPI size_t transcode(
   std::nothrow_t const &,
   encoding encSrc, void const ** ppSrc, size_t * pcbSrc,
   encoding encDst, void       ** ppDst, size_t * pcbDstMax
) {
   ABC_TRACE_FUNC(encSrc, ppSrc, pcbSrc, encDst, ppDst, pcbDstMax);

   // If ppDst is nullptr, we’ll only calculate how much of **ppSrc can be transcoded to fit into
   // *pcbDstMax; otherwise we’ll also perform the actual transcoding.
   bool bWriteDst(ppDst != nullptr);
   uint8_t const * pbSrc(static_cast<uint8_t const *>(*ppSrc));
   uint8_t       * pbDst(bWriteDst ? static_cast<uint8_t       *>(*ppDst) : nullptr);
   uint8_t const * pbSrcEnd(pbSrc + *pcbSrc);
   uint8_t const * pbDstEnd(pbDst + *pcbDstMax);

   uint8_t const * pbSrcLastUsed;
   for (;;) {
      pbSrcLastUsed = pbSrc;
      char32_t ch32;

      // Decode a source code point into ch32.
      switch (encSrc.base()) {
         case encoding::utf8: {
            if (pbSrc + sizeof(char8_t) > pbSrcEnd) {
               goto break_for;
            }
            char8_t ch8Src(char8_t(*pbSrc++));
            switch (ch8Src & 0xc0) {
               default: {
                  unsigned cbCont(utf8_traits::leading_to_cont_length(ch8Src));
                  // Ensure that we still have enough characters.
                  if (pbSrc + cbCont > pbSrcEnd) {
                     goto break_for;
                  }
                  // Convert the first byte to an UTF-32 character.
                  ch32 = utf8_traits::get_leading_cp_bits(ch8Src, cbCont);
                  // Shift in any continuation bytes.
                  for (; cbCont; --cbCont) {
                     ch8Src = char8_t(*pbSrc++);
                     if ((ch8Src & 0xc0) != 0x80) {
                        // The sequence ended prematurely, and this byte is not part of it.
                        --pbSrc;
                        break;
                     }
                     ch32 = (ch32 << 6) | (ch8Src & 0x3f);
                  }
                  if (!cbCont && utf32_traits::is_valid(ch32)) {
                     // The whole sequence was read, and the result is valid UTF-32.
                     break;
                  }
                  // Replace this invalid code point (fall through).
               }
               case 0x80:
                  // Replace this invalid byte.
                  ch32 = replacement_char;
                  break;
            }
            break;
         }

         case encoding::utf16le:
         case encoding::utf16be: {
            if (pbSrc + sizeof(char16_t) > pbSrcEnd) {
               goto break_for;
            }
            char16_t ch16Src(*reinterpret_cast<char16_t const *>(pbSrc));
            pbSrc += sizeof(char16_t);
            if (encSrc != encoding::utf16_host) {
               ch16Src = byteorder::swap(ch16Src);
            }
            switch (ch16Src & 0xfc00) {
               case 0xd800:
                  // Lead surrogate.
                  ch32 = char32_t(ch16Src & 0x03ff) << 10;
                  if (pbSrc + sizeof(char16_t) > pbSrcEnd) {
                     goto break_for;
                  }
                  ch16Src = *reinterpret_cast<char16_t const *>(pbSrc);
                  pbSrc += sizeof(char16_t);
                  if (encSrc != encoding::utf16_host) {
                     ch16Src = byteorder::swap(ch16Src);
                  }
                  // This character must be a trail surrogate.
                  if ((ch16Src & 0xfc00) == 0xdc00) {
                     ch32 = (ch32 | (ch16Src & 0x03ff)) + 0x10000;
                     if (utf32_traits::is_valid(ch32)) {
                        break;
                     }
                     // Replace this invalid code point (fall through).
                  }
                  // Replace this invalid half surrogate (fall through).
               case 0xdc00:
                  // Replace this invalid trail surrogate.
                  ch16Src = replacement_char;
                  break;
               default:
                  ch32 = ch16Src;
                  break;
            }
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
            // TODO: ISO-8859-1 support.
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
            // Compute the length of this sequence.
            unsigned cbSeq;
            if (ch32 <= 0x00007f) {
               cbSeq = 1;
            } else if (ch32 <= 0x0007ff) {
               cbSeq = 2;
            } else if (ch32 <= 0x00ffff) {
               cbSeq = 3;
            } else {
               cbSeq = 4;
            }
            if (pbDst + cbSeq > pbDstEnd) {
               goto break_for;
            }
            if (bWriteDst) {
               unsigned cbCont(cbSeq - 1);
               // Since each trailing byte can take 6 bits, the remaining ones (after >> 6 * cbCont)
               // make up what goes in the leading byte, which is combined with the proper sequence
               // indicator.
               *pbDst++ = uint8_t(
                  utf8_traits::cont_length_to_seq_indicator(cbCont) | char8_t(ch32 >> 6 * cbCont)
               );
               while (cbCont--) {
                  *pbDst++ = uint8_t(0x80 | ((ch32 >> 6 * cbCont) & 0x3f));
               }
            } else {
               pbDst += cbSeq;
            }
            break;
         }

         case encoding::utf16le:
         case encoding::utf16be: {
            unsigned cbCont(unsigned(ch32 > 0x00ffff) << 1);
            if (pbDst + sizeof(char16_t) + cbCont > pbDstEnd) {
               goto break_for;
            }
            if (bWriteDst) {
               char16_t ch16Dst0, ch16Dst1;
               if (cbCont) {
                  ch32 -= 0x10000;
                  ch16Dst0 = char16_t(0xd800 | ((ch32 & 0x0ffc00) >> 10));
                  ch16Dst1 = char16_t(0xdc00 |  (ch32 & 0x0003ff)       );
               } else {
                  ch16Dst0 = char16_t(ch32);
               }
               if (encDst != encoding::utf16_host) {
                  ch16Dst0 = byteorder::swap(ch16Dst0);
                  if (cbCont) {
                     ch16Dst1 = byteorder::swap(ch16Dst1);
                  }
               }
               *reinterpret_cast<char16_t *>(pbDst) = ch16Dst0;
               if (cbCont) {
                  pbDst += sizeof(char16_t);
                  *reinterpret_cast<char16_t *>(pbDst) = ch16Dst1;
               }
               pbDst += sizeof(char16_t);
            } else {
               pbDst += sizeof(char16_t) * cbCont;
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
            if (bWriteDst) {
               *reinterpret_cast<char32_t *>(pbDst) = ch32;
            }
            pbDst += sizeof(char32_t);
            break;

         case encoding::iso_8859_1:
            // TODO: ISO-8859-1 support.
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
   // Undo any incomplete or unused read.
   pbSrc = pbSrcLastUsed;
   // Update the variables pointed to by the arguments.
   *pcbSrc -= size_t(pbSrc - static_cast<uint8_t const *>(*ppSrc));
   *ppSrc = pbSrc;
   size_t cbDstUsed(size_t(pbDst - static_cast<uint8_t *>(*ppDst)));
   *pcbDstMax -= cbDstUsed;
   *ppDst = pbDst;
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
   error() {
   m_pszWhat = "abc::text::decode_error";
}


void decode_error::init(errint_t err /*= 0*/) {
   error::init(err ? err : os_error_mapping<decode_error>::mapped_error);
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::encode_error


namespace abc {
namespace text {

encode_error::encode_error() :
   error() {
   m_pszWhat = "abc::text::encode_error";
}


void encode_error::init(errint_t err /*= 0*/) {
   error::init(err ? err : os_error_mapping<encode_error>::mapped_error);
}

} //namespace text
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

