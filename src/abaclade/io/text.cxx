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
#include <abaclade/io/binary/file.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::base


namespace abc {
namespace io {
namespace text {

base::base() :
   m_lterm(abc::text::line_terminator::convert_any_to_lf) {
}


/*virtual*/ base::~base() {
}


} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::reader


namespace abc {
namespace io {
namespace text {

reader::reader() :
   base() {
}


void reader::read_all(mstr * psDst) {
   ABC_TRACE_FUNC(this, psDst);

   // Pass in a null function to indicate we’ll just consume everything.
   read_while(psDst, std::function<istr::const_iterator (istr const &, istr::const_iterator)>());
}


bool reader::read_line(mstr * psDst) {
   ABC_TRACE_FUNC(this, psDst);

   std::size_t cchLTerm(0);
   std::function<istr::const_iterator (istr const &, istr::const_iterator)> fnGetConsumeEnd;
   if (
      m_lterm == abc::text::line_terminator::any ||
      m_lterm == abc::text::line_terminator::convert_any_to_lf
   ) {
      fnGetConsumeEnd = [this, &cchLTerm] (
         istr const & sRead, istr::const_iterator itLastReadBegin
      ) -> istr::const_iterator {
         ABC_TRACE_FUNC(this, sRead, itLastReadBegin);

         /* Take advantage of the fact that read_while() will only send one line at a time and that
         each line will end in a single line terminator or won’t have any (because reading stopped
         at EOF): we mark trailing ‘\n’ and ‘\r’ characters to be trimmed (later) from the read
         string, but report to read_while() the whole string as consumed. */
         char_t const * pch(sRead.chars_end()), * pchBegin(sRead.chars_begin());
         while (--pch != pchBegin && (*pch == '\n' || *pch == '\r')) {
            ++cchLTerm;
         }
         return sRead.cend();
      };
   } else {
      fnGetConsumeEnd = [this, &cchLTerm] (
         istr const & sRead, istr::const_iterator itLastReadBegin
      ) -> istr::const_iterator {
         ABC_TRACE_FUNC(this, sRead, itLastReadBegin);

         // Pick the appropriate line terminator string and search for it.
         istr sLTerm(abc::text::get_line_terminator_str(m_lterm));
         /* Since line terminators can be more than one character long, back up one character first
         (if we have at least one), to avoid missing the line terminator due to searching for it one
         character beyond its start. */
         auto itLineEnd(sRead.find(sLTerm, itLastReadBegin - (
            sLTerm.size_in_chars() > 1 && itLastReadBegin > sRead.cbegin() ? 1 : 0
         )));
         // If the line terminator was not found, consume the entire string and ask for more
         // characters.
         if (itLineEnd == sRead.cend()) {
            return itLineEnd;
         }
         // Consume the string up to and including the line terminator; after read_while() we’ll
         // strip the line terminator (cchLTerm characters) from the string.
         cchLTerm = sLTerm.size_in_chars();
         return itLineEnd + static_cast<std::ptrdiff_t>(cchLTerm);
      };
   }
   bool bEOF(read_while(psDst, fnGetConsumeEnd));
   // Remove the line terminator from the end of the string, if we read one.
   if (cchLTerm) {
      psDst->set_size_in_chars(psDst->size_in_chars() - cchLTerm);
   }
   return bEOF;
}

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::writer


namespace abc {
namespace io {
namespace text {

writer::writer() :
   base() {
}


void writer::write_line(istr const & s) {
   ABC_TRACE_FUNC(this, s);

   to_str_backend<istr> tsb;
   tsb.write(s, this);
   abc::text::line_terminator lterm;
   // If no line terminator sequence has been explicitly set, use the platform’s default.
   if (m_lterm == abc::text::line_terminator::any) {
      lterm = abc::text::line_terminator::host;
   } else {
      lterm = m_lterm;
   }
   tsb.write(get_line_terminator_str(lterm), this);
}

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::detail::writer_print_helper


namespace abc {
namespace io {
namespace text {
namespace detail {

writer_print_helper_impl::writer_print_helper_impl(writer * ptw, istr const & sFormat) :
   m_ptw(ptw),
   // write_format_up_to_next_repl() will increment this to 0 or set it to a non-negative number.
   m_iSubstArg(static_cast<unsigned>(-1)),
   m_sFormat(sFormat),
   m_itFormatToWriteBegin(sFormat.cbegin()) {
}


void writer_print_helper_impl::run() {
   // Since this specialization has no replacements, verify that the format string doesn’t specify
   // any either.
   if (write_format_up_to_next_repl()) {
      ABC_THROW(index_error, (static_cast<std::ptrdiff_t>(m_iSubstArg)));
   }
}


void writer_print_helper_impl::throw_index_error() {
   ABC_THROW(index_error, (static_cast<std::ptrdiff_t>(m_iSubstArg)));
}


bool writer_print_helper_impl::write_format_up_to_next_repl() {
   ABC_TRACE_FUNC(this);

   // Search for the next replacement, if any.
   istr::const_iterator it(m_itFormatToWriteBegin), itReplFieldBegin, itEnd(m_sFormat.cend());
   char32_t ch;
   for (;;) {
      if (it >= itEnd) {
         // The format string is over; write any characters not yet written.
         write_format_up_to(itEnd);
         // Report that no more replacement fields were found.
         return false;
      }
      ch = *it++;
      if (ch == '{' || ch == '}') {
         if (ch == '{') {
            // Mark the beginning of the replacement field.
            itReplFieldBegin = it - 1;
            if (it >= itEnd) {
               throw_syntax_error(ABC_SL("unmatched '{' in format string"), itReplFieldBegin);
            }
            ch = *it;
            if (ch != '{') {
               // We found the beginning of a replacement field.
               break;
            }
         } else if (ch == '}') {
            if (it >= itEnd || *it != '}') {
               throw_syntax_error(ABC_SL("single '}' encountered in format string"), it - 1);
            }
         }
         // Convert “{{” into “{” or “}}” into “}”.
         // Write up to and including the first brace.
         write_format_up_to(it);
         // The next call to write_format_up_to() will skip the second brace.
         m_itFormatToWriteBegin = ++it;
      }
   }

   // Check if we have an argument index.
   if (ch >= '0' && ch <= '9') {
      // Consume as many digits as there are, and convert them into the argument index.
      unsigned iArg(0);
      for (;;) {
         iArg += static_cast<unsigned>(ch - '0');
         if (++it >= itEnd) {
            throw_syntax_error(ABC_SL("unmatched '{' in format string"), itReplFieldBegin);
         }
         ch = *it;
         if (ch < '0' || ch > '9') {
            break;
         }
         iArg *= 10;
      }
      // Save this index as the last used one.
      m_iSubstArg = iArg;
   } else {
      // The argument index is missing, so just use the next one.
      ++m_iSubstArg;
   }

   // Check for a format specification.
   if (ch == ':') {
      if (++it >= itEnd) {
         throw_syntax_error(ABC_SL("expected format specification"), it);
      }
      m_pchReplFormatSpecBegin = it.base();
      // Find the end of the replacement field.
      it = m_sFormat.find('}', it);
      if (it == itEnd) {
         throw_syntax_error(ABC_SL("unmatched '{' in format string"), itReplFieldBegin);
      }
      m_pchReplFormatSpecEnd = it.base();
   } else {
      // If there’s no format specification, it must be the end of the replacement field.
      if (ch != '}') {
         throw_syntax_error(ABC_SL("unmatched '{' in format string"), itReplFieldBegin);
      }
      // Set the format specification to nothing.
      m_pchReplFormatSpecBegin = nullptr;
      m_pchReplFormatSpecEnd = nullptr;
   }

   // Write the format string characters up to the beginning of the replacement.
   write_format_up_to(itReplFieldBegin);
   // Update this, so the next call to write_format_up_to() will skip over this replacement field.
   m_itFormatToWriteBegin = it + 1 /*'}'*/;
   // Report that a substitution must be written.
   return true;
}


void writer_print_helper_impl::throw_syntax_error(
   istr const & sDescription, istr::const_iterator it
) const {
   // +1 because the first character is 1, to human beings.
   ABC_THROW(
      syntax_error, (sDescription, m_sFormat, static_cast<unsigned>(it - m_sFormat.cbegin() + 1))
   );
}


void writer_print_helper_impl::write_format_up_to(istr::const_iterator itUpTo) {
   ABC_TRACE_FUNC(this, itUpTo);

   if (itUpTo > m_itFormatToWriteBegin) {
      m_ptw->write_binary(
         m_itFormatToWriteBegin.base(),
         reinterpret_cast<std::size_t>(itUpTo.base()) -
            reinterpret_cast<std::size_t>(m_itFormatToWriteBegin.base()),
         abc::text::encoding::host
      );
      m_itFormatToWriteBegin = itUpTo;
   }
}

} //namespace detail
} //namespace text
} //namespace io
} //namespace abc


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


/*virtual*/ abc::text::encoding binbuf_base::get_encoding() const {
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


/*virtual*/ std::shared_ptr<binary::buffered_base> binbuf_reader::buffered_base() const {
   ABC_TRACE_FUNC(this);

   return m_pbbr;
}


/*static*/ char_t const * binbuf_reader::call_get_consume_end(
   char_t const * pchBegin, char_t const * pchOffset, std::size_t cch, std::function<
      istr::const_iterator (istr const & sRead, istr::const_iterator itLastReadBegin)
   > const & fnGetConsumeEnd
) {
   ABC_TRACE_FUNC(pchBegin, pchOffset, cch/*, fnGetConsumeEnd*/);

   istr sConsumableBuf(unsafe, pchBegin, cch);
   char_t const * pchConsumeEnd(fnGetConsumeEnd(
      sConsumableBuf, istr::const_iterator(pchOffset, &sConsumableBuf)
   ).base());
   if (pchConsumeEnd < pchOffset) {
      // The caller wants to not consume bytes that we already consumed in a previous call, which is
      // not possible.
      // TODO: provide more information in the exception.
      ABC_THROW(iterator_error, ());
   }
   return pchConsumeEnd;
}


/*virtual*/ bool binbuf_reader::read_while(mstr * psDst, std::function<
   istr::const_iterator (istr const & sRead, istr::const_iterator itLastReadBegin)
> const & fnGetConsumeEnd) /*override*/ {
   ABC_TRACE_FUNC(this, psDst/*, fnGetConsumeEnd*/);

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
      std::size_t cbFile, cbBom;
      if (auto psb = std::dynamic_pointer_cast<binary::sized>(m_pbbr->unbuffered())) {
         // This special value prevents guess_encoding() from dismissing char16/32_t as impossible
         // just because the need to clip cbFile to a std::size_t resulted in an odd count of bytes.
         static std::size_t const sc_cbAlignedMax(
            numeric::max<std::size_t>::value & ~sizeof(char32_t)
         );
         cbFile = static_cast<std::size_t>(
            std::min(psb->size(), static_cast<full_size_t>(sc_cbAlignedMax))
         );
      } else {
         cbFile = 0;
      }
      m_enc = abc::text::guess_encoding(pbSrc, pbSrc + cbSrc, cbFile, &cbBom);
      // Cannot continue if the encoding is still unknown.
      if (m_enc == abc::text::encoding::unknown) {
         // TODO: provide more information in the exception.
         ABC_THROW(abc::text::error, ());
      }
      // If a BOM was read, consume and discard it.
      if (cbBom) {
         m_pbbr->consume_bytes(cbBom);
         pbSrc += cbBom;
         cbSrc -= cbBom;
         if (!cbSrc) {
            std::tie(pbSrc, cbSrc) = m_pbbr->peek<std::int8_t>(abc::text::max_codepoint_length);
         }
      }
   }

   std::size_t cchReadTotal(0);
   if (m_enc == abc::text::encoding::host) {
      // Optimal case: no transcoding necessary.
      while (cbSrc) {
         // Enlarge the destination string and append the read buffer contents to it.
         char_t const * pchSrcBegin(reinterpret_cast<char_t const *>(pbSrc));
         char_t const * pchSrcEnd(reinterpret_cast<char_t const *>(pbSrc + cbSrc));
         std::size_t cchSrc(cbSrc / sizeof(char_t));
         psDst->set_capacity(cchReadTotal + cchSrc, true);
         char_t * pchDstBegin(psDst->chars_begin());
         char_t * pchDstOffset(pchDstBegin + cchReadTotal);
         // Validate the characters in the source buffer before appending them to *psDst.
         // TODO: intercept exceptions if the “error mode” (TODO) mandates that errors be converted
         // into a special character, in which case we switch to the else branch below
         // (abc::text:transcode can fix errors if told so).
         abc::text::str_traits::validate(pchSrcBegin, pchSrcEnd, true);

         std::size_t cchConsumed;
         if (
            m_lterm == abc::text::line_terminator::any ||
            m_lterm == abc::text::line_terminator::convert_any_to_lf
         ) {
            /* Apply line terminators translation into “\n”s. There are two options on how to do it:

            a. Translate all line terminators in each buffer returned by peek(), then call
               fnGetConsumeEnd to get the end of the consumed buffer, and then rescan the peek
               buffer to find out where that end falls in it, so we can consume the correct amount
               of bytes read;

            b. Find the first line terminator (any one), then call fnGetConsumeEnd to get the end of
               the consumed buffer, and if that’s the entire line just read, repeat this until the
               whole peek buffer has been consumed or fnGetConsumeEnd stops before the end of a
               line.

            It’s likely that fnGetConsumeEnd will only consume a small portion of the first peek
            buffer (as in done by read_line()), so a. would unnecessarily translate all the line
            terminators in the peek buffer (which could contain the entire file), while b. may end
            up calling fnGetConsumeEnd and peek() multiple times, but it’s not going to scan the
            entire peek buffer unless it really happens to be a single line, and in any case it
            won’t need to scan it a second time because it knows it encountered at most a single
            line terminator. This makes b. the chosen algorithm. */

            char_t const * pchSrc(pchSrcBegin);
            char_t * pchDst(pchDstOffset);
            char_t const * pchDstConsumeEnd;
            do {
               char_t * pchDstLineStart(pchDst);
               // If the first character is a ‘\n’ that’s part of a “\r\n” terminator we already
               // presented as a ‘\n’, make it disappear.
               if (m_bDiscardNextLF && *pchSrc == '\n') {
                  m_bDiscardNextLF = false;
                  ++pchSrc;
               }
               do {
                  char_t ch(*pchSrc++);
                  if (ch == '\r') {
                     // Make sure we discard a possible following ‘\n’.
                     m_bDiscardNextLF = true;
                     if (m_lterm == abc::text::line_terminator::convert_any_to_lf) {
                        // Convert this ‘\r’ or “\r\n” into a ‘\n’.
                        ch = '\n';
                     }
                     *pchDst++ = ch;
                     break;
                  }
                  *pchDst++ = ch;
                  if (ch == '\n') {
                     // Stop here; no translation is necessary even if m_lterm is
                     // line_terminator::convert_any_to_lf.
                     break;
                  }
               } while (pchSrc < pchSrcEnd);
               if (fnGetConsumeEnd) {
                  pchDstConsumeEnd = call_get_consume_end(
                     pchDstBegin, pchDstLineStart,
                     static_cast<std::size_t>(pchDst - pchDstLineStart), fnGetConsumeEnd
                  );
                  if (pchDstConsumeEnd != pchDst) {
                     // We ended up not consuming the line terminator, so we shouldn’t try to
                     // discard a ‘\n’.
                     m_bDiscardNextLF = true;
                     break;
                  }
               } else {
                  pchDstConsumeEnd = pchDst;
               }
            } while (pchSrc < pchSrcEnd);
            cchReadTotal += static_cast<std::size_t>(pchDstConsumeEnd - pchDstOffset);
            cchConsumed = static_cast<std::size_t>(pchSrc - pchSrcBegin);
         } else {
            // No line terminator translation is needed.
            memory::copy(reinterpret_cast<std::int8_t *>(pchDstOffset), pbSrc, cbSrc);

            // Consume as much of the string as fnGetConsumeEnd, if provided, says.
            if (fnGetConsumeEnd) {
               char_t const * pchDstConsumeEnd(call_get_consume_end(
                  pchDstBegin, pchDstOffset, cchReadTotal + cchSrc, fnGetConsumeEnd
               ));
               cchConsumed = static_cast<std::size_t>(pchDstConsumeEnd - pchDstOffset);
            } else {
               cchConsumed = cchSrc;
            }
            cchReadTotal += cchConsumed;
         }
         m_pbbr->consume<char_t>(cchConsumed);

         // Read some more bytes; see comment to this same line at the beginning of this method.
         std::tie(pbSrc, cbSrc) = m_pbbr->peek<std::int8_t>(abc::text::max_codepoint_length);
      }
   } else {
      /* Sub-optimal case: transcoding is needed.

      Since fnGetConsumeEnd can reject part of the string which we’d then need to avoid consuming
      (expensive: we’d need to calculate the buffer offset back from the string offset, and the only
      way to do so is to re-transcode the buffer capping the destination size – see below), only
      translate relatively small portions (of size sc_cbSrcChunkMax) of the buffer at a time.

      TODO: tune sc_cbSrcChunkMax – too small causes more repeated function calls, too large causes
      more work in abc::text::transcode() when we need to move characters back to the source. */

      static std::size_t const sc_cbSrcChunkMax(128);
      while (cbSrc) {
         if (cbSrc > sc_cbSrcChunkMax) {
            cbSrc = sc_cbSrcChunkMax;
         }
         void const * pSrc(pbSrc);
         std::size_t cbSrcRemaining(cbSrc);
         // Calculate the additional size required.
         std::size_t cbDst(abc::text::transcode(
            true, m_enc, &pSrc, &cbSrcRemaining, abc::text::encoding::host
         ));
         // Enlarge the destination string and get its begin/end pointers.
         psDst->set_capacity(cchReadTotal + cbDst / sizeof(char_t), true);
         char_t * pchDstBegin(psDst->chars_begin());
         char_t * pchDstOffset(pchDstBegin + cchReadTotal);
         char_t * pchDstEnd(pchDstOffset);
         // Transcode the buffer chunk and advance pchDstEnd accordingly.
         abc::text::transcode(
            true, m_enc, &pSrc, &cbSrcRemaining,
            abc::text::encoding::host, reinterpret_cast<void **>(&pchDstEnd), &cbDst
         );

         // Determine how much of the string is to be consumed.
         char_t const * pchDstConsumeEnd;
         if (fnGetConsumeEnd) {
            pchDstConsumeEnd = call_get_consume_end(
               pchDstBegin, pchDstOffset, static_cast<std::size_t>(pchDstEnd - pchDstBegin),
               fnGetConsumeEnd
            );
            /* If fnGetConsumeEnd rejected some of the characters, repeat the transcoding capping
            the destination size to the consumed range of characters; this will yield the count of
            bytes to consume. */
            if (pchDstConsumeEnd != pchDstEnd) {
               // Restore the arguments for transcode().
               pSrc = pbSrc;
               cbSrcRemaining = cbSrc;
               pchDstEnd = pchDstOffset;
               cbDst = reinterpret_cast<std::size_t>(pchDstConsumeEnd) -
                  reinterpret_cast<std::size_t>(pchDstOffset);
               abc::text::transcode(
                  true, m_enc, &pSrc, &cbSrcRemaining,
                  abc::text::encoding::host, reinterpret_cast<void **>(&pchDstEnd), &cbDst
               );
               ABC_ASSERT(
                  pchDstEnd == pchDstConsumeEnd,
                  ABC_SL("abc::text::transcode() didn’t transcode the expected count of characters")
               );
            }
         } else {
            pchDstConsumeEnd = pchDstEnd;
         }
         // Consume as much of the string as fnGetConsumeEnd, if provided, said.
         cchReadTotal += static_cast<std::size_t>(pchDstConsumeEnd - pchDstOffset);
         m_pbbr->consume_bytes(cbSrc - cbSrcRemaining);

         // Read some more bytes; see comment to this same line at the beginning of this method.
         std::tie(pbSrc, cbSrc) = m_pbbr->peek<std::int8_t>(abc::text::max_codepoint_length);
      }
   }

   // Truncate the string.
   psDst->set_size_in_chars(cchReadTotal);
   // If the loop terminated because it run out of data and read no data at all, we reached the end
   // of the data; otherwise, return true.
   return cbSrc || cchReadTotal;
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


/*virtual*/ std::shared_ptr<binary::buffered_base> binbuf_writer::buffered_base() const {
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

