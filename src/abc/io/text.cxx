/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abc/core.hxx>
#include <abc/io/binary/file.hxx>



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
   ABC_TRACE_FN((this, ps));

   read_while(ps, [] (
      char_t const * pchBegin, char_t const * pchLastReadBegin, char_t const * pchEnd
   ) -> char_t const * {
      ABC_UNUSED_ARG(pchBegin);
      ABC_UNUSED_ARG(pchLastReadBegin);

      // Unconditionally consume the entire string and ask for more characters.
      return pchEnd;
   });
}


bool reader::read_line(mstr * ps) {
   ABC_TRACE_FN((this, ps));

   size_t cchLTerm(0);
   bool bEOF(read_while(ps, [this, &cchLTerm] (
      char_t const * pchBegin, char_t const * pchLastReadBegin, char_t const * pchEnd
   ) -> char_t const * {
      ABC_TRACE_FN((this, pchBegin, pchLastReadBegin, pchEnd));

      // Since line terminators can be more than one character long, back up one character first (if
      // we have at least one), to avoid missing the line terminator due to searching for it one
      // character beyond its start.
      char_t const * pchBeforeLastReadBegin(std::max(pchBegin, pchLastReadBegin - 1));

      // If the line terminator isn’t known yet, try to detect it now.
      if (m_lterm == abc::text::line_terminator::unknown) {
         m_lterm = abc::text::guess_line_terminator(pchBeforeLastReadBegin, pchEnd);
         // If no line terminator was detected, consume the entire string and ask for more
         // characters.
         if (m_lterm == abc::text::line_terminator::unknown) {
            return pchEnd;
         }
      }

      // Pick the appropriate line terminator string…
      istr sLTerm(get_line_terminator_str(m_lterm));
      // …and search for it.
      char_t const * pchLineEnd(abc::text::utf_traits<>::str_str(
         pchBeforeLastReadBegin, pchEnd, sLTerm.cbegin().base(), sLTerm.cend().base()
      ));
      // If the line terminator was not found, consume the entire string and ask for more
      // characters.
      if (pchLineEnd == pchEnd) {
         return pchEnd;
      }
      // Consume the string up to and including the line terminator; after read_while() we’ll strip
      // the line terminator (cchLTerm characters) from the string.
      cchLTerm = sLTerm.size();
      return pchLineEnd + cchLTerm;
   }));
   // Remove the line terminator from the end of the string.
   if (cchLTerm) {
      ps->set_size(ps->size() - cchLTerm);
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
   ABC_TRACE_FN((this, s));

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
   m_iSubstArg(unsigned(-1)),
   m_sFormat(sFormat),
   m_itFormatToWriteBegin(sFormat.cbegin()) {
}


void _writer_print_helper_impl::run() {
   // Since this specialization has no replacements, verify that the format string doesn’t specify
   // any either.
   if (write_format_up_to_next_repl()) {
      ABC_THROW(index_error, (m_iSubstArg));
   }
}


void _writer_print_helper_impl::throw_index_error() {
   ABC_THROW(index_error, (m_iSubstArg));
}


bool _writer_print_helper_impl::write_format_up_to_next_repl() {
   ABC_TRACE_FN((this));

   // Search for the next replacement, if any.
   istr::const_iterator it(m_itFormatToWriteBegin), itReplFieldBegin, itEnd(m_sFormat.cend());
   char_t ch;
   for (;;) {
      if (it >= itEnd) {
         // The format string is over; write any characters not yet written.
         write_format_up_to(itEnd);
         // Report that no more replacement fields were found.
         return false;
      }
      ch = *it++;
      if (ch == CL('{') || ch == CL('}')) {
         if (ch == CL('{')) {
            // Mark the beginning of the replacement field.
            itReplFieldBegin = it - 1;
            if (it >= itEnd) {
               throw_syntax_error(SL("unmatched '{' in format string"), itReplFieldBegin);
            }
            ch = *it;
            if (ch != CL('{')) {
               // We found the beginning of a replacement field.
               break;
            }
         } else if (ch == CL('}')) {
            if (it >= itEnd || *it != CL('}')) {
               throw_syntax_error(SL("single '}' encountered in format string"), it - 1);
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
   if (ch >= CL('0') && ch <= CL('9')) {
      // Consume as many digits as there are, and convert them into the argument index.
      unsigned iArg(0);
      do {
         iArg *= 10;
         iArg += unsigned(ch - CL('0'));
      } while (++it < itEnd && (ch = *it, ch >= CL('0') && ch <= CL('9')));
      if (it >= itEnd) {
         throw_syntax_error(SL("unmatched '{' in format string"), itReplFieldBegin);
      }
      // Save this index as the last used one.
      m_iSubstArg = iArg;
   } else {
      // The argument index is missing, so just use the next one.
      ++m_iSubstArg;
   }

   // Check for a conversion specifier; defaults to string.
   char_t chConversion(CL('s'));
   if (ch == CL('!')) {
      if (++it >= itEnd) {
         throw_syntax_error(SL("expected conversion specifier"), it);
      }
      ch = *it;
      switch (ch) {
         case CL('s'):
// TODO: case CL('r'):
// TODO: case CL('a'):
            chConversion = ch;
            ABC_UNUSED_ARG(chConversion);
            break;
         default:
            throw_syntax_error(SL("unknown conversion specifier"), it);
      }
      if (++it >= itEnd) {
         throw_syntax_error(SL("unmatched '{' in format string"), itReplFieldBegin);
      }
      ch = *it;
   }

   // Check for a format specification.
   if (ch == CL(':')) {
      if (++it >= itEnd) {
         throw_syntax_error(SL("expected format specification"), it);
      }
      m_pchReplFormatSpecBegin = it.base();
      // Find the end of the replacement field.
      it = m_sFormat.find(U32CL('}'), it);
      if (it == m_sFormat.cend()) {
         throw_syntax_error(SL("unmatched '{' in format string"), itReplFieldBegin);
      }
      m_pchReplFormatSpecEnd = it.base();
   } else {
      // If there’s no format specification, it must be the end of the replacement field.
      if (ch != CL('}')) {
         throw_syntax_error(SL("unmatched '{' in format string"), itReplFieldBegin);
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
   ABC_THROW(syntax_error, (sDescription, m_sFormat, unsigned(it - m_sFormat.cbegin() + 1)));
}


void _writer_print_helper_impl::write_format_up_to(istr::const_iterator itUpTo) {
   ABC_TRACE_FN((this, itUpTo));

   if (itUpTo > m_itFormatToWriteBegin) {
      m_ptw->write_binary(m_itFormatToWriteBegin.base(), size_t(
         reinterpret_cast<int8_t const *>(itUpTo.base()) -
         reinterpret_cast<int8_t const *>(m_itFormatToWriteBegin.base())
      ), abc::text::encoding::host);
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
   ABC_TRACE_FN((this));

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
   abc::text::encoding enc /* = abc::text::encoding::unknown*/,
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
   ABC_TRACE_FN((this));

   return m_pbbr;
}


/*virtual*/ bool binbuf_reader::read_while(mstr * ps, std::function<
   char_t const * (char_t const * pchBegin, char_t const * pchLastReadBegin, char_t const * pchEnd)
> fnGetConsumeEnd) {
   ABC_TRACE_FN((this, ps/*, fnGetConsumeEnd*/));

   // Start with trying to read enough bytes to have the certainty we can decode even the longest
   // code point.
   // This doesn’t necessarily mean that we’ll read as many, and this is fine because we just want
   // to make sure that the following loops don’t get stuck, never being able to consume a whole
   // code point.
   // This also doesn’t mean that we’ll only read as few, because the buffered reader will probably
   // read many more than this.
   int8_t const * pbBuf;
   size_t cbBuf;
   std::tie(pbBuf, cbBuf) = m_pbbr->peek<int8_t>(abc::text::max_codepoint_length);
   if (!cbBuf) {
      // If nothing was read, this is the end of the data.
      return false;
   }

   // If the encoding is still undefined, try to guess it now.
   if (m_enc == abc::text::encoding::unknown) {
      size_t cbFile, cbBom;
      /*if (binary::sized const * psb = dynamic_cast<binary::sized *>(m_pfile.get())) {
         cbFile = size_t(std::min<full_size_t>(psb->size(), smc_cbAlignedMax));
      } else {*/
         cbFile = 0;
      //}
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
            std::tie(pbBuf, cbBuf) = m_pbbr->peek<int8_t>(abc::text::max_codepoint_length);
         }
      }
   }

   size_t cchReadTotal(0);
   if (m_enc == abc::text::encoding::host) {
      // Optimal case: no transcoding necessary.
      while (cbBuf) {
         // Enlarge the destination string and append the read buffer contents to it.
         size_t cchBuf(cbBuf / sizeof(char_t));
         ps->set_capacity(cchReadTotal + cchBuf, true);
         char_t * pchDstBegin(ps->begin().base());
         char_t * pchDstOffset(pchDstBegin + cchReadTotal);
         memory::copy(reinterpret_cast<int8_t *>(pchDstOffset), pbBuf, cbBuf);

         // Consume as much of the string as fnGetConsumeEnd says.
         char_t const * pchDstConsumeEnd(
            fnGetConsumeEnd(pchDstBegin, pchDstOffset, pchDstOffset + cchBuf)
         );
         size_t cchConsumed(size_t(pchDstConsumeEnd - pchDstOffset));
         cchReadTotal += cchConsumed;
         m_pbbr->consume<char_t>(cchConsumed);

         // Read some more bytes; the rationale behind this count is explained above.
         std::tie(pbBuf, cbBuf) = m_pbbr->peek<int8_t>(abc::text::max_codepoint_length);
      }
   } else {
      // Sub-optimal case: transcoding is needed.

      // Since fnGetConsumeEnd can reject part of the string which we’d then need to avoid
      // consuming (expensive: we’d need to calculate the buffer offset back from the string
      // offset, and the only way to do so is to re-transcode the buffer capping the destination
      // size – see below), only translate relatively small portions (of size sc_cbBufChunkMax)
      // of the buffer at a time.
      // TODO: tune this value – too small causes more repeated function calls, too large causes
      // more work in abc::text::transcode().
      static size_t const sc_cbBufChunkMax(128);
      while (cbBuf) {
         if (cbBuf > sc_cbBufChunkMax) {
            cbBuf = sc_cbBufChunkMax;
         }
         // Calculate the additional size required, ceiling it to sizeof(char_t).
         size_t cchDstEst((abc::text::estimate_transcoded_size(
            m_enc, pbBuf, cbBuf, abc::text::encoding::host
         ) + sizeof(char_t) - 1) / sizeof(char_t));
         // Enlarge the destination string and get its begin/end pointers.
         ps->set_capacity(cchReadTotal + cchDstEst, true);
         char_t * pchDstBegin(ps->begin().base());
         char_t * pchDstOffset(pchDstBegin + cchReadTotal);
         char_t * pchDstEnd(pchDstOffset);
         // Transcode as much of the buffer chunk as possible, and advance pchDstEnd accordingly.
         int8_t const * pbSrc(pbBuf);
         size_t cbSrcConsumed(cbBuf);
         size_t cbDst(sizeof(char_t) * (ps->capacity() - cchReadTotal));
         abc::text::transcode(
            std::nothrow, m_enc, reinterpret_cast<void const **>(&pbSrc), &cbSrcConsumed,
            abc::text::encoding::host, reinterpret_cast<void **>(&pchDstEnd), &cbDst
         );

         // Determine how much of the string is to be consumed.
         char_t const * pchDstConsumeEnd(
            fnGetConsumeEnd(pchDstBegin, pchDstOffset, pchDstEnd)
         );
         // If fnGetConsumeEnd rejected some of the characters, repeat the transcoding capping the
         // destination size to the consumed range of characters; this will yield the count of bytes
         // to consume.
         if (pchDstConsumeEnd != pchDstEnd) {
            // Restore the arguments for transcode().
            pbSrc = pbBuf;
            cbSrcConsumed = cbBuf;
            pchDstEnd = pchDstOffset;
            cbDst = size_t(reinterpret_cast<int8_t const *>(pchDstConsumeEnd) -
               reinterpret_cast<int8_t *>(pchDstOffset));
            abc::text::transcode(
               std::nothrow, m_enc, reinterpret_cast<void const **>(&pbSrc), &cbSrcConsumed,
               abc::text::encoding::host, reinterpret_cast<void **>(&pchDstEnd), &cbDst
            );
            ABC_ASSERT(
               pchDstEnd == pchDstConsumeEnd,
               SL("abc::text::transcode() did not transcode the expected count of characters")
            );
         }

         // Consume as much of the buffer as fnGetConsumeEnd said.
         cchReadTotal += size_t(pchDstConsumeEnd - pchDstOffset);
         m_pbbr->consume_bytes(cbSrcConsumed);

         // Read some more bytes; the rationale behind this count is explained above.
         std::tie(pbBuf, cbBuf) = m_pbbr->peek<int8_t>(abc::text::max_codepoint_length);
      }
   }

   // Truncate the string.
   ps->set_size(cchReadTotal);
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
   abc::text::encoding enc /* = abc::text::encoding::unknown*/,
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
   ABC_TRACE_FN((this));

   return m_pbbw;
}


/*virtual*/ void binbuf_writer::write_binary(void const * p, size_t cb, abc::text::encoding enc) {
   ABC_TRACE_FN((this, p, cb, enc));

   ABC_ASSERT(enc != abc::text::encoding::unknown, SL("cannot write data with unknown encoding"));

   // If no encoding has been set yet, default to UTF-8 if we’re writing to a regular file, or the
   // host platform’s default in all other cases.
   if (m_enc == abc::text::encoding::unknown) {
      if (std::dynamic_pointer_cast<binary::regular_file_base>(m_pbbw->unbuffered())) {
         m_enc = abc::text::encoding::utf8;
      } else {
         m_enc = abc::text::encoding::host;
      }
   }

   // Trivial case.
   if (!cb) {
      return;
   }
   int8_t * pbBuf;
   size_t cbBuf;
   std::tie(pbBuf, cbBuf) = m_pbbw->get_buffer<int8_t>(cb);
   if (enc == m_enc) {
      // Optimal case: no transcoding necessary.
      memory::copy(pbBuf, static_cast<int8_t const *>(p), cb);
      p = static_cast<int8_t const *>(p) + cbBuf;
      cbBuf = cb;
   } else {
      // Sub-optimal case: transcoding is needed.
      cbBuf = abc::text::transcode(
         std::nothrow, enc, &p, &cb, m_enc, reinterpret_cast<void **>(&pbBuf), &cbBuf
      );
   }
   m_pbbw->commit_bytes(cbBuf);
}

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

