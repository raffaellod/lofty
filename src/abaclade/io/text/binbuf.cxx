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
#include <abaclade/text-after-str.hxx>

#include <algorithm>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

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

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text { namespace detail {

/* TODO: tune smc_cbTranscodeMax.
In the non-transcoding case, smaller values cause more calls to replenish_transcoded_buffer(),
while larger values cause larger memory allocations that might be wasteful if reading a single
line and lines are much shorter than the size of the peek buffer.
In the transcoding case, smaller values causes more calls to replenish_*_buffer(), while larger
values causes more repeated iterations in abc::text::transcode() when consume_used_bytes() needs
to calculate how many source bytes have been transcoded wihtout being consumed. */
std::size_t const reader_read_helper::smc_cbTranscodeMax = 0x1000;

reader_read_helper::reader_read_helper(
   binbuf_reader * ptbbr, std::uint8_t const * pbSrc, std::size_t cbSrc, str * psDst, bool bOneLine
) :
   m_ptbbr(ptbbr),

   m_pbSrc(pbSrc),
   m_cbSrc(cbSrc),
   m_psDst(psDst),
   m_bOneLine(bOneLine),

   mc_enc(m_ptbbr->m_enc),
   m_bEOF(m_ptbbr->m_bEOF),
   m_bDiscardNextLF(m_ptbbr->m_bDiscardNextLF),

   m_bLineEndsOnCROrAny(
      m_ptbbr->m_lterm == abc::text::line_terminator::cr ||
      m_ptbbr->m_lterm == abc::text::line_terminator::any ||
      m_ptbbr->m_lterm == abc::text::line_terminator::convert_any_to_lf
   ),
   m_bLineEndsOnLFOrAny(
      m_ptbbr->m_lterm == abc::text::line_terminator::lf ||
      m_ptbbr->m_lterm == abc::text::line_terminator::any ||
      m_ptbbr->m_lterm == abc::text::line_terminator::convert_any_to_lf
   ),
   m_cchReadTotal(0) {
   if (mc_enc == abc::text::encoding::host) {
      // Cap m_cbSrc and round it down to the previous char_t.
      m_cbSrcTranscoded = std::min(m_cbSrc, smc_cbTranscodeMax) & ~(sizeof(char_t) - 1);
      m_pchTranscodedBegin = reinterpret_cast<char_t const *>(pbSrc);
      m_pchTranscodedEnd = reinterpret_cast<char_t const *>(pbSrc + m_cbSrcTranscoded);
      // Validate the characters in the peek buffer before we start appending them to *psDst.
      /* TODO: FIXME: this is not forgiving of partially-read code points, but it should be. Maybe
      only do the validation later, at the end of each line? */
      /* TODO: intercept exceptions if the reader’s “error mode” (TODO) mandates that errors be
      converted into a special character, in which case we switch to forcing a transcoding read
      mode, since abc::text:transcode can fix errors if told so (this will need a separate variable
      to track the switch, instead of always comparing mc_enc == abc::text::encoding::host). */
      /* TODO: improve this so we don’t re-validate the entire string on each read; validate only
      the portion that was just read. */
      abc::text::str_traits::validate(m_pchTranscodedBegin, m_pchTranscodedEnd, true);
      m_psDst->set_capacity(m_cbSrcTranscoded / sizeof(char_t), false);
   } else {
      std::size_t cbTranscoded = abc::text::transcode(
         true, mc_enc, reinterpret_cast<void const **>(&pbSrc), &cbSrc, abc::text::encoding::host
      );
      m_psDst->set_capacity(cbTranscoded / sizeof(char_t), false);
      m_pchTranscodedBegin = m_psDst->data();
      // Transcode *pbSrc. This will move m_pchTranscodedEnd to the end of the transcoded string.
      m_pchTranscodedEnd = m_pchTranscodedBegin;
      std::size_t cbTranscodedRemaining = cbTranscoded;
      abc::text::transcode(
         true, mc_enc, reinterpret_cast<void const **>(&pbSrc), &cbSrc, abc::text::encoding::host,
         reinterpret_cast<void **>(const_cast<char_t **>(&m_pchTranscodedEnd)),
         &cbTranscodedRemaining
      );
      m_cbSrcTranscoded = m_cbSrc - cbSrc;
   }
   m_pchTranscoded = m_pchTranscodedBegin;
   m_pchDst = m_psDst->data();
}

reader_read_helper::~reader_read_helper() {
   m_ptbbr->m_bEOF = m_bEOF;
   m_ptbbr->m_bDiscardNextLF = m_bDiscardNextLF;
}

std::size_t reader_read_helper::consume_used_bytes() {
   ABC_TRACE_FUNC(this);

   std::size_t cchUsed = static_cast<std::size_t>(m_pchTranscoded - m_pchTranscodedBegin);
   m_cchReadTotal += cchUsed;
   std::size_t cbUsed;
   if (mc_enc == abc::text::encoding::host) {
      cbUsed = sizeof(char_t) * cchUsed;
   } else {
      if (m_pchTranscoded == m_pchTranscodedEnd) {
         // We used all the bytes we transcoded.
         cbUsed = m_cbSrcTranscoded;
      } else {
         /* We didn’t consume all the characters we transcoded, repeat the transcoding capping the
         destination size to the consumed range of characters; this will yield the count of bytes to
         consume. */
         void const * pbSrc = m_pbSrc;
         std::size_t cbSrc = m_cbSrcTranscoded, cbTranscodedRemaining = sizeof(char_t) * cchUsed;
         abc::text::transcode(
            true, mc_enc, &pbSrc, &cbSrc, abc::text::encoding::host, nullptr, &cbTranscodedRemaining
         );
         ABC_ASSERT(cbTranscodedRemaining == 0, ABC_SL(
            "abc::text::transcode() didn’t transcode the expected count of characters"
         ));
         cbUsed = m_cbSrcTranscoded - cbSrc;
      }
   }
   m_ptbbr->m_pbbr->consume_bytes(cbUsed);
   /* Reset m_pchTranscoded to inhibit further calls to this method until more characters are
   consumed. */
   m_pchTranscoded = m_pchTranscodedBegin;
   return cbUsed;
}

void reader_read_helper::read_line() {
   ABC_TRACE_FUNC(this);

   m_cchLTerm = 0;
   // This condition is the inverse of the one controlling the while loop that follows.
   if (m_pchTranscoded == m_pchTranscodedEnd && !replenish_transcoded_buffer()) {
      return;
   }
   if (m_bDiscardNextLF) {
      /* This CR is part of a CR+LF line terminator we already presented as a LF, so make it
      disappear. */
      if (*m_pchTranscoded == '\n') {
         ++m_pchTranscoded;
      }
      m_bDiscardNextLF = false;
   }
   /* This (inner) loop copies characters from the transcoded buffer into *m_psDst until it gets to
   the appropriate line terminator, which gets translated on the fly if necessary. */
   bool bLineEndsOnCRLFAndFoundCR = false;
   while (
      m_cchLTerm == 0 && (m_pchTranscoded != m_pchTranscodedEnd || replenish_transcoded_buffer())
   ) {
      char_t ch = *m_pchTranscoded++;
      *m_pchDst++ = ch;
      if (ch == '\r') {
         if (m_bLineEndsOnCROrAny) {
            if (m_ptbbr->m_lterm != abc::text::line_terminator::cr) {
               // Make sure we discard a possible following LF.
               m_bDiscardNextLF = true;
               if (m_ptbbr->m_lterm == abc::text::line_terminator::convert_any_to_lf) {
                  // Convert this CR (possibly followed by LF) into a LF.
                  *(m_pchDst - 1) = '\n';
               }
            }
            m_cchLTerm = 1;
         } else if (m_ptbbr->m_lterm == abc::text::line_terminator::cr_lf) {
            // Only consider this CR if followed by a LF.
            bLineEndsOnCRLFAndFoundCR = true;
         }
      } else if (ch == '\n') {
         if (m_bLineEndsOnLFOrAny) {
            m_cchLTerm = 1;
         } else if (bLineEndsOnCRLFAndFoundCR) {
            m_cchLTerm = 2;
         }
      }
   }
}

bool reader_read_helper::replenish_peek_buffer() {
   ABC_TRACE_FUNC(this);

   /* If we didn’t consume some bytes because they don’t make a complete code point, we’ll ask for
   at least one more byte, in an attempt to complete the code point. */
   std::size_t cbConsumed = consume_used_bytes();
   _std::tie(m_pbSrc, m_cbSrc) = m_ptbbr->m_pbbr->peek<std::uint8_t>(cbConsumed + 1);
   if (m_cbSrc > 0) {
      return true;
   } else {
      // Reached EOF. bEOF will break the outer loop after we break out of the inner one.
      /* TODO: if mc_enc != abc::text::encoding::host, we might have an incomplete character that
      couldn’t be transcoded: do something about it. */
      m_bEOF = true;
      return false;
   }
}

bool reader_read_helper::replenish_transcoded_buffer() {
   ABC_TRACE_FUNC(this);

   // Calculate sizes from the current pointers, since the string buffer might be reallocated.
   std::size_t cchDstUsed = static_cast<std::size_t>(m_pchDst - m_psDst->data());
   /* If we already transcoded all the peeked bytes (and we’re here, so we used all the transcoded
   characters), get more. */
   if (m_cbSrcTranscoded == m_cbSrc) {
      if (!replenish_peek_buffer()) {
         return false;
      }
   }

   if (mc_enc == abc::text::encoding::host) {
      // Cap m_cbSrc and round it down to the previous char_t.
      m_cbSrcTranscoded = std::min(m_cbSrc, smc_cbTranscodeMax) & ~(sizeof(char_t) - 1);
      m_pchTranscodedBegin = reinterpret_cast<char_t const *>(m_pbSrc);
      m_pchTranscodedEnd = reinterpret_cast<char_t const *>(m_pbSrc + m_cbSrcTranscoded);
      // Validate the characters in the peek buffer before we start appending them to *psDst.
      /* TODO: FIXME: this is not forgiving of partially-read code points, but it should be. Maybe
      only do the validation later, at the end of each line? */
      /* TODO: intercept exceptions if the reader’s “error mode” (TODO) mandates that errors be
      converted into a special character, in which case we switch to forcing a transcoding read
      mode, since abc::text:transcode can fix errors if told so (this will need a separate variable
      to track the switch, instead of always comparing mc_enc == abc::text::encoding::host). */
      /* TODO: improve this so we don’t re-validate the entire string on each read; validate only
      the portion that was just read. */
      abc::text::str_traits::validate(m_pchTranscodedBegin, m_pchTranscodedEnd, true);
      m_psDst->set_capacity(m_cbSrcTranscoded / sizeof(char_t), true);
   } else {
      std::uint8_t const * pbSrc = m_pbSrc;
      std::size_t cbSrc = m_cbSrc;
      std::size_t cbTranscoded = abc::text::transcode(
         true, mc_enc, reinterpret_cast<void const **>(&pbSrc), &cbSrc, abc::text::encoding::host
      );
      m_cbSrcTranscoded = m_cbSrc - cbSrc;
      m_psDst->set_capacity(cchDstUsed + cbTranscoded / sizeof(char_t), true);
      // Use the part of the string beyond cchDstUsed as the transcoding destination buffer.
      m_pchTranscodedBegin = m_psDst->data() + cchDstUsed;
      // Transcode *pbSrc. This will move m_pchTranscodedEnd to the end of the transcoded string.
      m_pchTranscodedEnd = m_pchTranscodedBegin;
      std::size_t cbTranscodedRemaining = cbTranscoded;
      abc::text::transcode(
         true, mc_enc, reinterpret_cast<void const **>(&pbSrc), &cbSrc, abc::text::encoding::host,
         const_cast<void **>(reinterpret_cast<void const **>(&m_pchTranscodedEnd)),
         &cbTranscodedRemaining
      );
   }
   m_pchTranscoded = m_pchTranscodedBegin;
   // Rebase this pointer onto the (possibly) newly-reallocated string.
   m_pchDst = m_psDst->data() + cchDstUsed;
   return true;
}

bool reader_read_helper::run() {
   ABC_TRACE_FUNC(this);

   // This (outer) loop restarts the inner one if we’re not just reading a single line.
   do {
      read_line();
   } while (!m_bOneLine && !m_bEOF);
   if (m_bOneLine) {
      // If the line read includes a line terminator, strip it off.
      m_pchDst -= m_cchLTerm;
   }
   // Calculate the length of the string and truncate it to that.
   std::size_t cchDstTotal = static_cast<std::size_t>(m_pchDst - m_psDst->data());
   m_psDst->set_size_in_chars(cchDstTotal);
   if (m_pchTranscoded != m_pchTranscodedBegin) {
      // Calculate how many bytes were used from the last buffer peek and consume them.
      consume_used_bytes();
   }
   return m_cchReadTotal > 0;
}

}}}} //namespace abc::io::text::detail

namespace abc { namespace io { namespace text {

binbuf_reader::binbuf_reader(
   _std::shared_ptr<binary::buffered_reader> pbbr,
   abc::text::encoding enc /*= abc::text::encoding::unknown*/
) :
   base(),
   binbuf_base(enc),
   reader(),
   m_pbbr(_std::move(pbbr)),
   m_bEOF(false),
   m_bDiscardNextLF(false) {
}

/*virtual*/ binbuf_reader::~binbuf_reader() {
}

/*virtual*/ _std::shared_ptr<binary::buffered_base> binbuf_reader::_binary_buffered_base(
) const /*override*/ {
   ABC_TRACE_FUNC(this);

   return m_pbbr;
}

std::size_t binbuf_reader::detect_encoding(std::uint8_t const * pb, std::size_t cb) {
   ABC_TRACE_FUNC(this, pb, cb);

   std::size_t cbFile, cbBom;
   if (auto psb = _std::dynamic_pointer_cast<binary::sized>(m_pbbr->unbuffered())) {
      /* This special value prevents guess_encoding() from dismissing UTF-16/32 as impossible just
      because the need to clip cbFile to a std::size_t resulted in an odd count of bytes. */
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
   return cbBom;
}

/*virtual*/ bool binbuf_reader::read_line_or_all(str * psDst, bool bOneLine) /*override*/ {
   ABC_TRACE_FUNC(this, psDst, bOneLine);

   // Only continue if we didn’t reach EOF in the past.
   if (m_bEOF) {
      return false;
   }

   // Attempt to read at least a single byte.
   std::uint8_t const * pbSrc;
   std::size_t cbSrc;
   _std::tie(pbSrc, cbSrc) = m_pbbr->peek<std::uint8_t>(1);
   if (cbSrc == 0) {
      // If nothing was read, this is the end of the data.
      m_bEOF = true;
      return false;
   }

   // If the encoding is still undefined, try to guess it now.
   if (m_enc == abc::text::encoding::unknown) {
      std::size_t cbBom = detect_encoding(pbSrc, cbSrc);
      // If a BOM was read, consume it.
      if (cbBom) {
         m_pbbr->consume<std::uint8_t>(cbBom);
         pbSrc += cbBom;
         cbSrc -= cbBom;
      }
   }

   detail::reader_read_helper rrh(this, pbSrc, cbSrc, psDst, bOneLine);
   return rrh.run();
}

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

binbuf_writer::binbuf_writer(
   _std::shared_ptr<binary::buffered_writer> pbbw,
   abc::text::encoding enc /*= abc::text::encoding::unknown*/
) :
   base(),
   binbuf_base(enc),
   writer(),
   m_pbbw(_std::move(pbbw)) {
}

/*virtual*/ binbuf_writer::~binbuf_writer() {
   // Let m_pbbw detect whether finalize() was not called.
}

/*virtual*/ _std::shared_ptr<binary::buffered_base> binbuf_writer::_binary_buffered_base(
) const /*override*/ {
   ABC_TRACE_FUNC(this);

   return m_pbbw;
}

/*virtual*/ void binbuf_writer::finalize() /*override*/ {
   ABC_TRACE_FUNC(this);

   m_pbbw->finalize();
}

/*virtual*/ void binbuf_writer::flush() /*override*/ {
   ABC_TRACE_FUNC(this);

   m_pbbw->flush();
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
   if (cbSrc == 0) {
      return;
   }
   std::int8_t * pbDst;
   std::size_t cbDst;
   _std::tie(pbDst, cbDst) = m_pbbw->get_buffer<std::int8_t>(cbSrc);
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

}}} //namespace abc::io::text
