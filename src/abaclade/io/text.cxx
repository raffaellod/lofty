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

base::base(abc::text::line_terminator lterm) :
   m_lterm(lterm) {
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

reader::reader(abc::text::line_terminator lterm) :
   base(lterm) {
}


void reader::read_all(mstr * ps) {
   ABC_TRACE_FUNC(this, ps);

   read_while(ps, [] (
      istr const & sRead, istr::const_iterator itLastReadBegin
   ) -> istr::const_iterator {
      ABC_UNUSED_ARG(itLastReadBegin);

      // Unconditionally consume the entire string and ask for more characters.
      return sRead.cend();
   });
}


bool reader::read_line(mstr * ps) {
   ABC_TRACE_FUNC(this, ps);

   std::size_t cchLTerm(0);
   bool bEOF(read_while(ps, [this, &cchLTerm] (
      istr const & sRead, istr::const_iterator itLastReadBegin
   ) -> istr::const_iterator {
      ABC_TRACE_FUNC(this, sRead, itLastReadBegin);

      // Since line terminators can be more than one character long, back up one character first (if
      // we have at least one), to avoid missing the line terminator due to searching for it one
      // character beyond its start.
      istr::const_iterator itBeforeLastReadBegin(std::max(sRead.cbegin(), itLastReadBegin - 1));

      // If the line terminator isn’t known yet, try to detect it now.
      if (m_lterm == abc::text::line_terminator::unknown) {
         m_lterm = abc::text::guess_line_terminator(
            itBeforeLastReadBegin.base(), sRead.chars_end()
         );
         // If no line terminator was detected, consume the entire string and ask for more
         // characters.
         if (m_lterm == abc::text::line_terminator::unknown) {
            return sRead.cend();
         }
      }

      // Pick the appropriate line terminator string…
      istr sLTerm(abc::text::get_line_terminator_str(m_lterm));
      // …and search for it.
      auto itLineEnd(sRead.find(sLTerm, itBeforeLastReadBegin));
      // If the line terminator was not found, consume the entire string and ask for more
      // characters.
      if (itLineEnd == sRead.cend()) {
         return itLineEnd;
      }
      // Consume the string up to and including the line terminator; after read_while() we’ll strip
      // the line terminator (cchLTerm characters) from the string.
      cchLTerm = sLTerm.size_in_chars();
      return itLineEnd + static_cast<std::ptrdiff_t>(cchLTerm);
   }));
   // Remove the line terminator from the end of the string.
   if (cchLTerm) {
      ps->set_size_in_chars(ps->size_in_chars() - cchLTerm);
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

writer::writer(abc::text::line_terminator lterm) :
   base(lterm) {
}


void writer::write_line(istr const & s) {
   ABC_TRACE_FUNC(this, s);

   to_str_backend<istr> tsb;
   tsb.write(s, this);
   abc::text::line_terminator lterm(m_lterm);
   // If at this point we still haven’t picked a line terminator, use the platform’s default.
   if (lterm == abc::text::line_terminator::unknown) {
      lterm = abc::text::line_terminator::host;
   }
   tsb.write(get_line_terminator_str(lterm), this);
}


_writer_print_helper_impl::_writer_print_helper_impl(writer * ptw, istr const & sFormat) :
   m_ptw(ptw),
   // write_format_up_to_next_repl() will increment this to 0 or set it to a non-negative number.
   m_iSubstArg(static_cast<unsigned>(-1)),
   m_sFormat(sFormat),
   m_itFormatToWriteBegin(sFormat.cbegin()) {
}


void _writer_print_helper_impl::run() {
   // Since this specialization has no replacements, verify that the format string doesn’t specify
   // any either.
   if (write_format_up_to_next_repl()) {
      ABC_THROW(index_error, (static_cast<std::ptrdiff_t>(m_iSubstArg)));
   }
}


void _writer_print_helper_impl::throw_index_error() {
   ABC_THROW(index_error, (static_cast<std::ptrdiff_t>(m_iSubstArg)));
}


bool _writer_print_helper_impl::write_format_up_to_next_repl() {
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
      do {
         iArg *= 10;
         iArg += static_cast<unsigned>(ch - '0');
      } while (++it < itEnd && (ch = *it, ch >= '0' && ch <= '9'));
      if (it >= itEnd) {
         throw_syntax_error(ABC_SL("unmatched '{' in format string"), itReplFieldBegin);
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
      if (it == m_sFormat.cend()) {
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


void _writer_print_helper_impl::throw_syntax_error(
   istr const & sDescription, istr::const_iterator it
) const {
   // +1 because the first character is 1, to human beings.
   ABC_THROW(
      syntax_error, (sDescription, m_sFormat, static_cast<unsigned>(it - m_sFormat.cbegin() + 1))
   );
}


void _writer_print_helper_impl::write_format_up_to(istr::const_iterator itUpTo) {
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

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::binbuf_base


namespace abc {
namespace io {
namespace text {

binbuf_base::binbuf_base(abc::text::encoding enc, abc::text::line_terminator lterm) :
   base(lterm),
   m_enc(enc) {
}


/*virtual*/ binbuf_base::~binbuf_base() {
}


/*virtual*/ abc::text::encoding binbuf_base::encoding() const {
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
   abc::text::encoding enc /*= abc::text::encoding::unknown*/,
   abc::text::line_terminator lterm /*= abc::text::line_terminator::unknown*/
) :
   base(lterm),
   binbuf_base(enc, lterm),
   reader(lterm),
   m_pbbr(std::move(pbbr)) {
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
      // The caller wants to not consume bytes that we already consumed in a previous call, which
      // is not possible.
      // TODO: provide more information in the exception.
      ABC_THROW(iterator_error, ());
   }
   return pchConsumeEnd;
}


/*virtual*/ bool binbuf_reader::read_while(mstr * ps, std::function<
   istr::const_iterator (istr const & sRead, istr::const_iterator itLastReadBegin)
> const & fnGetConsumeEnd) {
   ABC_TRACE_FUNC(this, ps/*, fnGetConsumeEnd*/);

   // Start with trying to read enough bytes to have the certainty we can decode even the longest
   // code point.
   // This doesn’t necessarily mean that we’ll read as many, and this is fine because we just want
   // to make sure that the following loops don’t get stuck, never being able to consume a whole
   // code point.
   // This also doesn’t mean that we’ll only read as few, because the buffered reader will probably
   // read many more than this.
   std::int8_t const * pbBuf;
   std::size_t cbBuf;
   std::tie(pbBuf, cbBuf) = m_pbbr->peek<std::int8_t>(abc::text::max_codepoint_length);
   if (!cbBuf) {
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
      m_enc = abc::text::guess_encoding(pbBuf, pbBuf + cbBuf, cbFile, &cbBom);
      // Cannot continue if the encoding is still unknown.
      if (m_enc == abc::text::encoding::unknown) {
         // TODO: provide more information in the exception.
         ABC_THROW(abc::text::error, ());
      }
      // If a BOM was read, consume and discard it.
      if (cbBom) {
         m_pbbr->consume_bytes(cbBom);
         pbBuf += cbBom;
         cbBuf -= cbBom;
         if (!cbBuf) {
            std::tie(pbBuf, cbBuf) = m_pbbr->peek<std::int8_t>(abc::text::max_codepoint_length);
         }
      }
   }

   std::size_t cchReadTotal(0);
   if (m_enc == abc::text::encoding::host) {
      // Optimal case: no transcoding necessary.
      while (cbBuf) {
         // Enlarge the destination string and append the read buffer contents to it.
         std::size_t cchBuf(cbBuf / sizeof(char_t));
         ps->set_capacity(cchReadTotal + cchBuf, true);
         char_t * pchDstBegin(ps->chars_begin());
         char_t * pchDstOffset(pchDstBegin + cchReadTotal);
         // Validate the characters in the source buffer before appending them to *ps.
         // TODO: intercept exceptions if the “error mode” (TODO) mandates that errors be converted
         // into a special character, in which case we switch to the else branch below
         // (abc::text:transcode can fix errors if told so).
         abc::text::str_traits::validate(
            reinterpret_cast<char_t const *>(pbBuf),
            reinterpret_cast<char_t const *>(pbBuf + cbBuf), true
         );
         memory::copy(reinterpret_cast<std::int8_t *>(pchDstOffset), pbBuf, cbBuf);

         // Consume as much of the string as fnGetConsumeEnd says.
         char_t const * pchDstConsumeEnd(call_get_consume_end(
            pchDstBegin, pchDstOffset, cchReadTotal + cchBuf, fnGetConsumeEnd
         ));
         std::size_t cchConsumed(static_cast<std::size_t>(pchDstConsumeEnd - pchDstOffset));
         cchReadTotal += cchConsumed;
         m_pbbr->consume<char_t>(cchConsumed);

         // Read some more bytes; see comment to this same line at the beginning of this method.
         std::tie(pbBuf, cbBuf) = m_pbbr->peek<std::int8_t>(abc::text::max_codepoint_length);
      }
   } else {
      // Sub-optimal case: transcoding is needed.
      // Since fnGetConsumeEnd can reject part of the string which we’d then need to avoid consuming
      // (expensive: we’d need to calculate the buffer offset back from the string offset, and the
      // only way to do so is to re-transcode the buffer capping the destination size – see below),
      // only translate relatively small portions (of size sc_cbBufChunkMax) of the buffer at a
      // time.

      // TODO: tune sc_cbBufChunkMax – too small causes more repeated function calls, too large
      // causes more work in abc::text::transcode() when we need to move characters back to the
      // source.
      static std::size_t const sc_cbBufChunkMax(128);
      while (cbBuf) {
         if (cbBuf > sc_cbBufChunkMax) {
            cbBuf = sc_cbBufChunkMax;
         }
         void const * pSrc(pbBuf);
         std::size_t cbSrcRemaining(cbBuf);
         // Calculate the additional size required.
         std::size_t cbDst(abc::text::transcode(
            true, m_enc, &pSrc, &cbSrcRemaining, abc::text::encoding::host
         ));
         // Enlarge the destination string and get its begin/end pointers.
         ps->set_capacity(cchReadTotal + cbDst / sizeof(char_t), true);
         char_t * pchDstBegin(ps->chars_begin());
         char_t * pchDstOffset(pchDstBegin + cchReadTotal);
         char_t * pchDstEnd(pchDstOffset);
         // Transcode the buffer chunk and advance pchDstEnd accordingly.
         abc::text::transcode(
            true, m_enc, &pSrc, &cbSrcRemaining,
            abc::text::encoding::host, reinterpret_cast<void **>(&pchDstEnd), &cbDst
         );

         // Determine how much of the string is to be consumed.
         char_t const * pchDstConsumeEnd(call_get_consume_end(
            pchDstBegin, pchDstOffset, static_cast<std::size_t>(pchDstEnd - pchDstBegin),
            fnGetConsumeEnd
         ));
         // If fnGetConsumeEnd rejected some of the characters, repeat the transcoding capping the
         // destination size to the consumed range of characters; this will yield the count of bytes
         // to consume.
         if (pchDstConsumeEnd != pchDstEnd) {
            // Restore the arguments for transcode().
            pSrc = pbBuf;
            cbSrcRemaining = cbBuf;
            pchDstEnd = pchDstOffset;
            cbDst = reinterpret_cast<std::size_t>(pchDstConsumeEnd) -
               reinterpret_cast<std::size_t>(pchDstOffset);
            abc::text::transcode(
               true, m_enc, &pSrc, &cbSrcRemaining,
               abc::text::encoding::host, reinterpret_cast<void **>(&pchDstEnd), &cbDst
            );
            ABC_ASSERT(
               pchDstEnd == pchDstConsumeEnd,
               ABC_SL("abc::text::transcode() did not transcode the expected count of characters")
            );
         }

         // Consume as much of the buffer as fnGetConsumeEnd said.
         cchReadTotal += static_cast<std::size_t>(pchDstConsumeEnd - pchDstOffset);
         m_pbbr->consume_bytes(cbBuf - cbSrcRemaining);

         // Read some more bytes; see comment to this same line at the beginning of this method.
         std::tie(pbBuf, cbBuf) = m_pbbr->peek<std::int8_t>(abc::text::max_codepoint_length);
      }
   }

   // Truncate the string.
   ps->set_size_in_chars(cchReadTotal);
   // If the loop terminated because it run out of data and read no data at all, we reached the end
   // of the data; otherwise, return true.
   return cbBuf || cchReadTotal;
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
   abc::text::encoding enc /*= abc::text::encoding::unknown*/,
   abc::text::line_terminator lterm /*= abc::text::line_terminator::unknown*/
) :
   base(lterm),
   binbuf_base(enc, lterm),
   writer(lterm),
   m_pbbw(std::move(pbbw)) {
}


/*virtual*/ binbuf_writer::~binbuf_writer() {
}


/*virtual*/ std::shared_ptr<binary::buffered_base> binbuf_writer::buffered_base() const {
   ABC_TRACE_FUNC(this);

   return m_pbbw;
}


/*virtual*/ void binbuf_writer::write_binary(
   void const * p, std::size_t cb, abc::text::encoding enc
) {
   ABC_TRACE_FUNC(this, p, cb, enc);

   ABC_ASSERT(
      enc != abc::text::encoding::unknown, ABC_SL("cannot write data with unknown encoding")
   );

   // If no encoding has been set yet, default to UTF-8.
   if (m_enc == abc::text::encoding::unknown) {
      m_enc = abc::text::encoding::utf8;
   }

   // Trivial case.
   if (!cb) {
      return;
   }
   std::int8_t * pbBuf;
   std::size_t cbBuf;
   std::tie(pbBuf, cbBuf) = m_pbbw->get_buffer<std::int8_t>(cb);
   if (enc == m_enc) {
      // Optimal case: no transcoding necessary.
      memory::copy(pbBuf, static_cast<std::int8_t const *>(p), cb);
      cbBuf = cb;
   } else {
      // Sub-optimal case: transcoding is needed.
      cbBuf = abc::text::transcode(
         true, enc, &p, &cb, m_enc, reinterpret_cast<void **>(&pbBuf), &cbBuf
      );
   }
   m_pbbw->commit_bytes(cbBuf);
}

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

