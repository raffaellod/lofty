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
#include <abaclade/text.hxx>

#include <algorithm> // std::min()


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

str_stream::str_stream() :
   stream(),
   m_ichOffset(0) {
}

str_stream::str_stream(str_stream && ss) :
   stream(_std::move(ss)),
   m_ichOffset(ss.m_ichOffset) {
}

/*virtual*/ str_stream::~str_stream() {
}

/*virtual*/ abc::text::encoding str_stream::get_encoding() const /*override*/ {
   return abc::text::encoding::host;
}

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

str_istream::str_istream(str_istream && sis) :
   stream(_std::move(sis)),
   str_stream(_std::move(sis)),
   istream(_std::move(sis)),
   m_psReadBuf(sis.m_psReadBuf != &sis.m_sReadBuf ? sis.m_psReadBuf : &m_sReadBuf),
   m_sReadBuf(_std::move(sis.m_sReadBuf)) {
   // Make sis use its own internal read buffer.
   sis.m_psReadBuf = &sis.m_sReadBuf;
}

str_istream::str_istream(str const & s) :
   stream(),
   str_stream(),
   istream(),
   m_psReadBuf(&m_sReadBuf),
   m_sReadBuf(s) {
}

str_istream::str_istream(str && s) :
   stream(),
   str_stream(),
   istream(),
   m_psReadBuf(&m_sReadBuf),
   m_sReadBuf(_std::move(s)) {
}

str_istream::str_istream(external_buffer_t const &, str const * ps) :
   stream(),
   str_stream(),
   istream(),
   m_psReadBuf(ps) {
}

/*virtual*/ str_istream::~str_istream() {
}

/*virtual*/ bool str_istream::read_line_or_all(str * psDst, bool bOneLine) /*override*/ {
   ABC_TRACE_FUNC(this, psDst, bOneLine);

   // TODO: implement this.
   return false;
}

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

str_ostream::str_ostream() :
   stream(),
   str_stream(),
   ostream(),
   m_psWriteBuf(&m_sDefaultWriteBuf) {
}

str_ostream::str_ostream(str_ostream && sos) :
   stream(_std::move(sos)),
   str_stream(_std::move(sos)),
   ostream(_std::move(sos)),
   m_psWriteBuf(
      sos.m_psWriteBuf != &sos.m_sDefaultWriteBuf ? sos.m_psWriteBuf : &m_sDefaultWriteBuf
   ),
   m_sDefaultWriteBuf(_std::move(sos.m_sDefaultWriteBuf)) {
   //  Make sos use its own internal buffer.
   sos.m_psWriteBuf = &sos.m_sDefaultWriteBuf;
}

str_ostream::str_ostream(external_buffer_t const &, str * psBuf) :
   stream(),
   str_stream(),
   ostream(),
   m_psWriteBuf(psBuf) {
}

/*virtual*/ str_ostream::~str_ostream() {
}

void str_ostream::clear() {
   ABC_TRACE_FUNC(this);

   m_psWriteBuf->set_size_in_chars(0);
   m_ichOffset = 0;
}

/*virtual*/ void str_ostream::finalize() /*override*/ {
   // Nothing to do.
}

/*virtual*/ void str_ostream::flush() /*override*/ {
   // Nothing to do.
}

str str_ostream::release_content() {
   ABC_TRACE_FUNC(this);

   m_ichOffset = 0;
   return _std::move(*m_psWriteBuf);
}

/*virtual*/ void str_ostream::write_binary(
   void const * pSrc, std::size_t cbSrc, abc::text::encoding enc
) /*override*/ {
   ABC_TRACE_FUNC(this, pSrc, cbSrc, enc);

   if (cbSrc == 0) {
      // Nothing to do.
      return;
   }
   ABC_ASSERT(
      enc != abc::text::encoding::unknown, ABC_SL("cannot write data with unknown encoding")
   );
   if (enc == abc::text::encoding::host) {
      // Optimal case: no transcoding necessary.
      std::size_t cch = cbSrc / sizeof(char_t);
      // Enlarge the string as necessary, then overwrite any character in the affected range.
      m_psWriteBuf->set_capacity(m_ichOffset + cch, true);
      memory::copy(m_psWriteBuf->data() + m_ichOffset, static_cast<char_t const *>(pSrc), cch);
      m_ichOffset += cch;
   } else {
      // Calculate the additional buffer size required.
      std::size_t cbBuf = abc::text::transcode(true, enc, &pSrc, &cbSrc, abc::text::encoding::host);
      m_psWriteBuf->set_capacity(m_ichOffset + cbBuf / sizeof(char_t), true);
      // Transcode the source into the string buffer and advance m_ichOffset accordingly.
      void * pBuf = m_psWriteBuf->data() + m_ichOffset;
      m_ichOffset += abc::text::transcode(
         true, enc, &pSrc, &cbSrc, abc::text::encoding::host, &pBuf, &cbBuf
      ) / sizeof(char_t);
   }
   // Truncate the string.
   m_psWriteBuf->set_size_in_chars(m_ichOffset);
}

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

char_ptr_ostream::char_ptr_ostream(char * pchBuf, std::size_t * pcchBufAvailable) :
   m_pchWriteBuf(pchBuf),
   m_pcchWriteBufAvailable(pcchBufAvailable) {
}

char_ptr_ostream::char_ptr_ostream(char_ptr_ostream && cpos) :
   ostream(_std::move(cpos)),
   m_pchWriteBuf(cpos.m_pchWriteBuf),
   m_pcchWriteBufAvailable(cpos.m_pcchWriteBufAvailable) {
   cpos.m_pchWriteBuf = nullptr;
}

/*virtual*/ char_ptr_ostream::~char_ptr_ostream() {
   if (m_pchWriteBuf) {
      /* NUL-terminate the string. Always safe, since *m_pcchWriteBufAvailable doesn’t include space
      for the NUL terminator. */
      *m_pchWriteBuf = '\0';
   }
}

/*virtual*/ void char_ptr_ostream::finalize() /*override*/ {
   // Nothing to do.
}

/*virtual*/ void char_ptr_ostream::flush() /*override*/ {
   // Nothing to do.
}

/*virtual*/ abc::text::encoding char_ptr_ostream::get_encoding() const /*override*/ {
   // Assume char is always UTF-8.
   return abc::text::encoding::utf8;
}

/*virtual*/ void char_ptr_ostream::write_binary(
   void const * pSrc, std::size_t cbSrc, abc::text::encoding enc
) /*override*/ {
   ABC_TRACE_FUNC(this, pSrc, cbSrc, enc);

   if (cbSrc == 0) {
      // Nothing to do.
      return;
   }
   ABC_ASSERT(
      enc != abc::text::encoding::unknown, ABC_SL("cannot write data with unknown encoding")
   );
   if (enc == abc::text::encoding::utf8) {
      // Optimal case: no transcoding necessary.
      std::size_t cch = std::min(*m_pcchWriteBufAvailable, cbSrc / sizeof(char));
      memory::copy(m_pchWriteBuf, static_cast<char const *>(pSrc), cch);
      m_pchWriteBuf += cch;
      *m_pcchWriteBufAvailable -= cch;
   } else {
      // Transcode the source into the string buffer.
      abc::text::transcode(
         true, enc, &pSrc, &cbSrc, abc::text::encoding::utf8,
         reinterpret_cast<void **>(&m_pchWriteBuf), m_pcchWriteBufAvailable
      );
   }
}

}}} //namespace abc::io::text
