/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2016 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#include <abaclade/io/binary.hxx>
#include <abaclade/io/text.hxx>
#include <abaclade/numeric.hxx>
#include <abaclade/text.hxx>

#include <algorithm> // std::min()


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

binbuf_stream::binbuf_stream(abc::text::encoding enc) :
   stream(),
   m_enc(enc) {
}

/*virtual*/ binbuf_stream::~binbuf_stream() {
}

/*virtual*/ abc::text::encoding binbuf_stream::get_encoding() const /*override*/ {
   ABC_TRACE_FUNC(this);

   return m_enc;
}

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

binbuf_istream::binbuf_istream(
   _std::shared_ptr<binary::buffered_istream> pbbis,
   abc::text::encoding enc /*= abc::text::encoding::unknown*/
) :
   stream(),
   binbuf_stream(enc),
   istream(),
   m_pbbis(_std::move(pbbis)),
   m_ichPeekBufOffset(0),
   m_bEOF(false) {
}

/*virtual*/ binbuf_istream::~binbuf_istream() {
}

/*virtual*/ _std::shared_ptr<binary::buffered_stream> binbuf_istream::_binary_buffered_stream(
) const /*override*/ {
   ABC_TRACE_FUNC(this);

   return m_pbbis;
}

/*virtual*/ void binbuf_istream::consume_chars(std::size_t cch) /*override*/ {
   ABC_TRACE_FUNC(this, cch);

   if (cch > m_sPeekBuf.size_in_chars() - m_ichPeekBufOffset) {
      // TODO: use a better exception class.
      ABC_THROW(argument_error, ());
   }
   m_ichPeekBufOffset += cch;
}

std::size_t binbuf_istream::detect_encoding(std::uint8_t const * pb, std::size_t cb) {
   ABC_TRACE_FUNC(this, pb, cb);

   std::size_t cbFile, cbBom;
   if (auto psb = _std::dynamic_pointer_cast<binary::sized>(m_pbbis->unbuffered())) {
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

/*virtual*/ str binbuf_istream::peek_chars(std::size_t cchMin) /*override*/ {
   ABC_TRACE_FUNC(this, cchMin);

   // The peek buffer might already contain enough characters.
   std::size_t cchPeekBuf = m_sPeekBuf.size_in_chars() - m_ichPeekBufOffset;
   if (cchPeekBuf < cchMin && !m_bEOF) {
      /* Ensure the peek buffer is large enough to hold the requested count of characters or an
      arbitrary minimum (chosen for efficiency). */
      if (cchMin > m_sPeekBuf.capacity() - m_ichPeekBufOffset) {
         // If there’s any unused space in m_sPeekBuf, recover it now.
         /* TODO: might use a different strategy to decide if it’s more convenient to just allocate
         a bigger buffer based on m_sPeekBuf.capacity() vs. cchPeekBuf, i.e. the cost of
         memory::realloc() vs. the cost of memory::move(). */
         if (m_ichPeekBufOffset > 0) {
            if (cchPeekBuf > 0) {
               memory::move(m_sPeekBuf.data(), m_sPeekBuf.data() + m_ichPeekBufOffset, cchPeekBuf);
            }
            m_ichPeekBufOffset = 0;
            m_sPeekBuf.set_size_in_chars(cchPeekBuf, false /*don’t clear*/);
         }
         static std::size_t const sc_cchPeekBufMin = 128;
         std::size_t cchNeededCapacity = std::max(cchMin, sc_cchPeekBufMin);
         if (cchNeededCapacity > m_sPeekBuf.capacity()) {
            m_sPeekBuf.set_capacity(cchNeededCapacity, true /*preserve*/);
         }
      }
      char_t * pchPeekBufBegin = m_sPeekBuf.data() + m_ichPeekBufOffset;
      std::size_t cbPeekBufCapacity = (m_sPeekBuf.capacity() - m_ichPeekBufOffset) * sizeof(char_t);
      void * pPeekBufEnd = pchPeekBufBegin;

      std::size_t cbPeekMin = 1;
      do {
         std::uint8_t const * pbSrc;
         std::size_t cbSrc;
         _std::tie(pbSrc, cbSrc) = m_pbbis->peek<std::uint8_t>(cbPeekMin);
         if (cbSrc == 0) {
            m_bEOF = true;
            break;
         }

         // If the encoding is still undefined, try to guess it now.
         if (m_enc == abc::text::encoding::unknown) {
            if (std::size_t cbBom = detect_encoding(pbSrc, cbSrc)) {
               // Consume the BOM that was read.
               m_pbbis->consume<std::uint8_t>(cbBom);
               pbSrc += cbBom;
               cbSrc -= cbBom;
            }
         }

         // Transcode the binary peek buffer into m_sPeekBuf at m_ichPeekBufOffset + cchPeekBuf.
         std::size_t cbSrcRemaining = cbSrc;
         std::size_t cbPeekBufTranscoded = abc::text::transcode(
            true, m_enc, reinterpret_cast<void const **>(&pbSrc), &cbSrcRemaining,
            abc::text::encoding::host, &pPeekBufEnd, &cbPeekBufCapacity
         );
         if (cbPeekBufTranscoded == 0) {
            // Couldn’t transcode even a single code point; get more bytes and try again.
            ++cbPeekMin;
            continue;
         }
         // If this was changed, ensure it’s reset now that we successfully transcoded something.
         cbPeekMin = 1;

         // Permanently remove the transcoded bytes from the binary buffer.
         m_pbbis->consume<std::uint8_t>(cbSrc - cbSrcRemaining);
         // Account for the characters just transcoded.
         cchPeekBuf += cbPeekBufTranscoded / sizeof(char_t);
         m_sPeekBuf.set_size_in_chars(m_ichPeekBufOffset + cchPeekBuf, false /*don’t clear*/);
      } while (cchPeekBuf < cchMin);
   }
   // Return a view of m_sPeekBuf to avoid copying it.
   return str(
      external_buffer,
      m_sPeekBuf.data() + m_ichPeekBufOffset,
      m_sPeekBuf.size_in_chars() - m_ichPeekBufOffset
   );
}

/*virtual*/ bool binbuf_istream::read_line(str * psDst) /*override*/ {
   ABC_TRACE_FUNC(this, psDst);

   if (m_bEOF) {
      psDst->clear();
      return false;
   } else {
      // This will result in calls to peek_chars(), which will set m_bEOF as necessary.
      istream::read_line(psDst);
      return true;
   }
}

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

binbuf_ostream::binbuf_ostream(
   _std::shared_ptr<binary::buffered_ostream> pbbos,
   abc::text::encoding enc /*= abc::text::encoding::unknown*/
) :
   stream(),
   binbuf_stream(enc),
   ostream(),
   m_pbbos(_std::move(pbbos)) {
}

/*virtual*/ binbuf_ostream::~binbuf_ostream() {
   // Let m_pbbos detect whether finalize() was not called.
}

/*virtual*/ _std::shared_ptr<binary::buffered_stream> binbuf_ostream::_binary_buffered_stream(
) const /*override*/ {
   ABC_TRACE_FUNC(this);

   return m_pbbos;
}

/*virtual*/ void binbuf_ostream::finalize() /*override*/ {
   ABC_TRACE_FUNC(this);

   m_pbbos->finalize();
}

/*virtual*/ void binbuf_ostream::flush() /*override*/ {
   ABC_TRACE_FUNC(this);

   m_pbbos->flush();
}

/*virtual*/ void binbuf_ostream::write_binary(
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
   _std::tie(pbDst, cbDst) = m_pbbos->get_buffer<std::int8_t>(cbSrc);
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
   m_pbbos->commit_bytes(cbDst);
}

}}} //namespace abc::io::text
