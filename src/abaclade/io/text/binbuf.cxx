/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
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
#include <abaclade/numeric.hxx>

#include <algorithm>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::binbuf_base

namespace abc {
namespace io {
namespace text {

binbuf_base::binbuf_base(abc::text::encoding enc) :
   base(),
   m_enc(enc) {
}

/*virtual*/ binbuf_base::~binbuf_base() {
}

/*virtual*/ abc::text::encoding binbuf_base::get_encoding() const /*override*/ {
   ABC_TRACE_FUNC(this);

   return m_enc;
}

} //namespace text
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::binbuf_reader

namespace abc {
namespace io {
namespace text {

binbuf_reader::binbuf_reader(
   std::shared_ptr<binary::buffered_reader> pbbr,
   abc::text::encoding enc /*= abc::text::encoding::unknown*/
) :
   base(),
   binbuf_base(enc),
   reader(),
   m_pbbr(std::move(pbbr)),
   m_bDiscardNextLF(false) {
}

/*virtual*/ binbuf_reader::~binbuf_reader() {
}

/*virtual*/ std::shared_ptr<binary::buffered_base> binbuf_reader::_binary_buffered_base(
) const /*override*/ {
   ABC_TRACE_FUNC(this);

   return m_pbbr;
}

std::size_t binbuf_reader::detect_encoding(std::int8_t const * pb, std::size_t cb) {
   ABC_TRACE_FUNC(this, pb, cb);

   std::size_t cbFile, cbBom;
   if (auto psb = std::dynamic_pointer_cast<binary::sized>(m_pbbr->unbuffered())) {
      // This special value prevents guess_encoding() from dismissing char16/32_t as impossible
      // just because the need to clip cbFile to a std::size_t resulted in an odd count of bytes.
      static std::size_t const sc_cbAlignedMax =
         numeric::max<std::size_t>::value & ~sizeof(char32_t);
      cbFile = static_cast<std::size_t>(
         std::min(psb->size(), static_cast<full_size_t>(sc_cbAlignedMax))
      );
   } else {
      cbFile = 0;
   }
   m_enc = abc::text::guess_encoding(pb, pb + cb, cbFile, &cbBom);
   // Cannot continue if the encoding is still unknown.
   if (m_enc == abc::text::encoding::unknown) {
      // TODO: provide more information in the exception.
      ABC_THROW(abc::text::error, ());
   }
   // If a BOM was read, consume and discard it.
   return cbBom;
}

/*virtual*/ bool binbuf_reader::read_while(mstr * psDst, bool bOneLine) /*override*/ {
   ABC_TRACE_FUNC(this, psDst, bOneLine);

   /* Start with trying to read enough bytes to have the certainty we can decode even the longest
   code point. This doesn’t necessarily mean that we’ll read as many, and this is fine because we
   just want to make sure that the following loops don’t get stuck, never being able to consume a
   whole code point; this also doesn’t mean that we’ll only read as few, because the buffered reader
   will probably read many more than this. */
   std::int8_t const * pbSrc;
   std::size_t cbSrc;
   std::tie(pbSrc, cbSrc) = m_pbbr->peek<std::int8_t>(abc::text::max_codepoint_length);
   if (!cbSrc) {
      // If nothing was read, this is the end of the data.
      return false;
   }

   // If the encoding is still undefined, try to guess it now.
   if (m_enc == abc::text::encoding::unknown) {
      std::size_t cbBom = detect_encoding(pbSrc, cbSrc);
      // If a BOM was read, consume and discard it.
      if (cbBom) {
         m_pbbr->consume_bytes(cbBom);
         pbSrc += cbBom;
         cbSrc -= cbBom;
         // If we already run out of bytes, get some more.
         if (!cbSrc) {
            std::tie(pbSrc, cbSrc) = m_pbbr->peek<std::int8_t>(abc::text::max_codepoint_length);
         }
      }
   }

   std::size_t cchReadTotal = 0;
   if (m_enc == abc::text::encoding::host) {
      // Optimal case: no transcoding necessary.
      cchReadTotal = read_while_with_host_encoding(pbSrc, &cbSrc, psDst, bOneLine);
   } else {
      // Sub-optimal case: transcoding is needed.
      cchReadTotal = read_while_with_transcode(pbSrc, &cbSrc, psDst, bOneLine);
   }

   // Truncate the string.
   psDst->set_size_in_chars(cchReadTotal);
   // If the loop terminated because it run out of data and read no data at all, we reached the end
   // of the data; otherwise, return true.
   return cbSrc || cchReadTotal;
}

std::size_t binbuf_reader::read_while_with_host_encoding(
   std::int8_t const * pbSrc, std::size_t * pcbSrc, mstr * psDst, bool bOneLine
) {
   ABC_TRACE_FUNC(this, pbSrc, pcbSrc, psDst, bOneLine);

   bool bLineEndsOnCROrAny =
      m_lterm == abc::text::line_terminator::cr ||
      m_lterm == abc::text::line_terminator::any ||
      m_lterm == abc::text::line_terminator::convert_any_to_lf;
   bool bLineEndsOnLFOrAny =
      m_lterm == abc::text::line_terminator::lf ||
      m_lterm == abc::text::line_terminator::any ||
      m_lterm == abc::text::line_terminator::convert_any_to_lf;
   // This loop consumes one peek buffer at a time; it may end prematurely if bOneLine == true.
   std::size_t cchReadTotal = 0;
   for (
      ;
      *pcbSrc;
      std::tie(pbSrc, *pcbSrc) = m_pbbr->peek<std::int8_t>(abc::text::max_codepoint_length)
   ) {
      // Enlarge the destination string and append the read buffer contents to it.
      char_t const * pchSrcBegin = reinterpret_cast<char_t const *>(pbSrc);
      char_t const * pchSrcEnd = reinterpret_cast<char_t const *>(pbSrc + *pcbSrc);
      std::size_t cchSrc = *pcbSrc / sizeof(char_t);
      psDst->set_capacity(cchReadTotal + cchSrc, true);
      char_t * pchDstBegin = psDst->chars_begin();
      char_t * pchDstOffset = pchDstBegin + cchReadTotal;
      // Validate the characters in the source buffer before appending them to *psDst.
      // TODO: intercept exceptions if the “error mode” (TODO) mandates that errors be converted
      // into a special character, in which case we switch to using read_while_with_transcode()
      // (abc::text:transcode can fix errors if told so).
      abc::text::str_traits::validate(pchSrcBegin, pchSrcEnd, true);

      /* This loop copies one line at a time from the peek buffer into the destination string, at an
      increasing offset from its start (pchDstOffset). Each iteration ends on the first line
      terminator matching m_lterm, and the line terminator is also translated on the fly if
      appropriate. If not asked for just a single line, this continues until we exhaust the current
      peek buffer. */
      char_t const * pchSrc = pchSrcBegin;
      char_t * pchDst = pchDstOffset;
      do {
         // If the first character is a CR that’s part of a CR+LF terminator we already presented as
         // a LF, make it disappear.
         if (m_bDiscardNextLF) {
            if (*pchSrc == '\n') {
               ++pchSrc;
            }
            m_bDiscardNextLF = false;
         }
         bool bLineEndsOnCRLFAndFoundCR = false;
         while (pchSrc < pchSrcEnd) {
            char_t ch = *pchSrc++;
            *pchDst++ = ch;
            if (ch == '\r') {
               if (bLineEndsOnCROrAny) {
                  if (m_lterm != abc::text::line_terminator::cr) {
                     // Make sure we discard a possible following LF.
                     m_bDiscardNextLF = true;
                     if (m_lterm == abc::text::line_terminator::convert_any_to_lf) {
                        // Convert this CR (possibly followed by LF) into a LF.
                        *(pchDst - 1) = '\n';
                     }
                  }
                  break;
               } else if (m_lterm == abc::text::line_terminator::cr_lf) {
                  // Only consider this CR if followed by a LF.
                  bLineEndsOnCRLFAndFoundCR = true;
               }
            } else if (ch == '\n') {
               if (bLineEndsOnLFOrAny || bLineEndsOnCRLFAndFoundCR) {
                  break;
               }
            }
         }
      } while (!bOneLine && pchSrc < pchSrcEnd);
      // Count the copied bytes and consume the used bytes.
      cchReadTotal += static_cast<std::size_t>(pchDst - pchDstOffset);
      m_pbbr->consume<char_t>(static_cast<std::size_t>(pchSrc - pchSrcBegin));
      if (pchSrc < pchSrcEnd) {
         // We broke out of the line-consuming loop due to bOneLine, so stop here.
         break;
      }
   }
   return cchReadTotal;
}

std::size_t binbuf_reader::read_while_with_transcode(
   std::int8_t const * pbSrc, std::size_t * pcbSrc, mstr * psDst, bool bOneLine
) {
   ABC_TRACE_FUNC(this, pbSrc, pcbSrc, psDst, bOneLine);

   bool bLineEndsOnCROrAny =
      m_lterm == abc::text::line_terminator::cr ||
      m_lterm == abc::text::line_terminator::any ||
      m_lterm == abc::text::line_terminator::convert_any_to_lf;
   bool bLineEndsOnLFOrAny =
      m_lterm == abc::text::line_terminator::lf ||
      m_lterm == abc::text::line_terminator::any ||
      m_lterm == abc::text::line_terminator::convert_any_to_lf;
   /* If bOneLine == true, we may need to reject part of a string we just transcoded (expensive:
   we’d need to calculate the buffer offset back from the string offset, and the only way to do so
   is to re-transcode the buffer capping the destination size – see below), se we’ll only transcode
   relatively small portions (of size cbSrcMax) of the peek buffer at a time.

   TODO: tune the initial value for cbSrcMax: smaller causes more repeated function calls, larger
   causes more work in abc::text::transcode() when we need to move characters back to the source. */
   //std::size_t cbSrcMax(32);
   std::size_t cchReadTotal = 0, cbSrc = *pcbSrc;
   bool bLineEndsOnCRLFAndFoundCR = false;
   for (
      ;
      cbSrc;
      std::tie(pbSrc, cbSrc) = m_pbbr->peek<std::int8_t>(abc::text::max_codepoint_length)
   ) {
      /*if (bOneLine && cbSrc > cbSrcMax) {
         cbSrc = cbSrcMax;
         cbSrcMax *= 2;
      }*/
      void const * pSrc = pbSrc;
      std::size_t cbSrcRemaining = cbSrc;
      // Calculate the additional size required.
      std::size_t cbDst = abc::text::transcode(
         true, m_enc, &pSrc, &cbSrcRemaining, abc::text::encoding::host
      );
      // Enlarge the destination string and get its begin/end pointers.
      psDst->set_capacity(cchReadTotal + cbDst / sizeof(char_t), true);
      char_t const * pchDstBegin = psDst->chars_begin();
      char_t * pchDstOffset = const_cast<char_t *>(pchDstBegin) + cchReadTotal;
      char_t * pchDstEnd = pchDstOffset;
      // Transcode the buffer chunk and advance pchDstEnd accordingly.
      abc::text::transcode(
         true, m_enc, &pSrc, &cbSrcRemaining,
         abc::text::encoding::host, reinterpret_cast<void **>(&pchDstEnd), &cbDst
      );

      /* This loop scans the destination string (buffer) one line at a time, at an increasing offset
      from its start (pchDstOffset). Each iteration ends on the first line terminator matching
      m_lterm, and the line terminator is also translated on the fly if appropriate. If not asked
      for just a single line, this continues until we exhaust the current peek buffer. */
      char_t const * pchDstUntranslated = pchDstOffset;
      char_t * pchDstTranslated = pchDstOffset;
      do {
         // If the first character is a CR that’s part of a CR+LF terminator we already presented as
         // a LF, make it disappear.
         if (m_bDiscardNextLF) {
            if (*pchDstUntranslated == '\n') {
               ++pchDstUntranslated;
            }
            m_bDiscardNextLF = false;
         }
         while (pchDstUntranslated < pchDstEnd) {
            char_t ch = *pchDstUntranslated++;
            // TODO: avoid writing ch if source and destination pointers are in sync.
            *pchDstTranslated++ = ch;
            if (ch == '\r') {
               if (bLineEndsOnCROrAny) {
                  if (m_lterm != abc::text::line_terminator::cr) {
                     // Make sure we discard a possible following LF.
                     m_bDiscardNextLF = true;
                     if (m_lterm == abc::text::line_terminator::convert_any_to_lf) {
                        // Convert this CR (possibly followed by LF) into a LF.
                        *(pchDstTranslated - 1) = '\n';
                     }
                  }
                  break;
               } else if (m_lterm == abc::text::line_terminator::cr_lf) {
                  // Only consider this CR if followed by a LF.
                  bLineEndsOnCRLFAndFoundCR = true;
               }
            } else if (ch == '\n') {
               if (bLineEndsOnLFOrAny || bLineEndsOnCRLFAndFoundCR) {
                  bLineEndsOnCRLFAndFoundCR = false;
                  break;
               }
            }
         }

         if (bOneLine && pchDstUntranslated < pchDstEnd) {
            /* If we didn’t consume all the bytes we transcoded, repeat the transcoding capping the
            destination size to the consumed range of characters; this will yield the count of bytes
            to consume. */

            // Restore the arguments for transcode(), keeping in mind we don’t want back the LF we
            // discarded earlier due to m_bDiscardNextLF.
            pSrc = static_cast<std::int8_t const *>(pbSrc);
            cbSrcRemaining = cbSrc;
            cbDst = reinterpret_cast<std::size_t>(pchDstUntranslated) -
               reinterpret_cast<std::size_t>(pchDstOffset);
            abc::text::transcode(
               true, m_enc, &pSrc, &cbSrcRemaining, abc::text::encoding::host, nullptr, &cbDst
            );
            ABC_ASSERT(cbDst == 0, ABC_SL(
               "abc::text::transcode() didn’t transcode the expected count of characters"
            ));
         }
      } while (!bOneLine && pchDstUntranslated < pchDstEnd);
      // Count the transcoded bytes and consume the used bytes.
      cchReadTotal += static_cast<std::size_t>(pchDstTranslated - pchDstOffset);
      m_pbbr->consume_bytes(cbSrc - cbSrcRemaining);
      if (cbSrcRemaining) {
         // We broke out of the line-consuming loop due to bOneLine, so stop here.
         break;
      }
   }
   *pcbSrc = cbSrc;
   return cchReadTotal;
}

} //namespace text
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::binbuf_writer

namespace abc {
namespace io {
namespace text {

binbuf_writer::binbuf_writer(
   std::shared_ptr<binary::buffered_writer> pbbw,
   abc::text::encoding enc /*= abc::text::encoding::unknown*/
) :
   base(),
   binbuf_base(enc),
   writer(),
   m_pbbw(std::move(pbbw)) {
}

/*virtual*/ binbuf_writer::~binbuf_writer() {
}

/*virtual*/ std::shared_ptr<binary::buffered_base> binbuf_writer::_binary_buffered_base(
) const /*override*/ {
   ABC_TRACE_FUNC(this);

   return m_pbbw;
}

/*virtual*/ void binbuf_writer::write_binary(
   void const * pSrc, std::size_t cbSrc, abc::text::encoding enc
) /*override*/ {
   ABC_TRACE_FUNC(this, pSrc, cbSrc, enc);

   ABC_ASSERT(
      enc != abc::text::encoding::unknown, ABC_SL("cannot write data with unknown encoding")
   );

   // If no encoding has been set yet, default to UTF-8.
   if (m_enc == abc::text::encoding::unknown) {
      m_enc = abc::text::encoding::utf8;
   }

   // Trivial case.
   if (!cbSrc) {
      return;
   }
   std::int8_t * pbDst;
   std::size_t cbDst;
   std::tie(pbDst, cbDst) = m_pbbw->get_buffer<std::int8_t>(cbSrc);
   if (enc == m_enc) {
      // Optimal case: no transcoding necessary.
      memory::copy(pbDst, static_cast<std::int8_t const *>(pSrc), cbSrc);
      cbDst = cbSrc;
   } else {
      // Sub-optimal case: transcoding is needed.
      cbDst = abc::text::transcode(
         true, enc, &pSrc, &cbSrc, m_enc, reinterpret_cast<void **>(&pbDst), &cbDst
      );
   }
   m_pbbw->commit_bytes(cbDst);
}

} //namespace text
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
