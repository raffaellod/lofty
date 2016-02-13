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
   m_psBuf(&m_sDefaultBuf),
   m_ichOffset(0) {
}

str_stream::str_stream(str_stream && ss) :
   stream(_std::move(ss)),
   m_psBuf(ss.m_psBuf != &ss.m_sDefaultBuf ? ss.m_psBuf : &m_sDefaultBuf),
   m_sDefaultBuf(_std::move(ss.m_sDefaultBuf)),
   m_ichOffset(ss.m_ichOffset) {
   // Make ss use its own internal buffer.
   ss.m_psBuf = &ss.m_sDefaultBuf;
   ss.m_ichOffset = 0;
}

/*explicit*/ str_stream::str_stream(str const & s) :
   stream(),
   m_psBuf(&m_sDefaultBuf),
   m_sDefaultBuf(s),
   m_ichOffset(0) {
}

/*explicit*/ str_stream::str_stream(str && s) :
   stream(),
   m_psBuf(&m_sDefaultBuf),
   m_sDefaultBuf(_std::move(s)),
   m_ichOffset(0) {
}

/*explicit*/ str_stream::str_stream(external_buffer_t const &, str const * ps) :
   stream(),
   m_psBuf(const_cast<str *>(ps)),
   m_ichOffset(0) {
}

/*virtual*/ str_stream::~str_stream() {
}

/*virtual*/ abc::text::encoding str_stream::get_encoding() const /*override*/ {
   return abc::text::encoding::host;
}

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

str_istream::str_istream(str const & s) :
   stream(),
   str_stream(s),
   istream() {
}

str_istream::str_istream(str && s) :
   stream(),
   str_stream(_std::move(s)),
   istream() {
}

str_istream::str_istream(external_buffer_t const & eb, str const * ps) :
   stream(),
   str_stream(eb, ps),
   istream() {
}

/*virtual*/ str_istream::~str_istream() {
}

/*virtual*/ void str_istream::consume_chars(std::size_t cch) /*override*/ {
   ABC_TRACE_FUNC(this, cch);

   if (cch > remaining_size_in_chars()) {
      // TODO: use a better exception class.
      ABC_THROW(argument_error, ());
   }
   m_ichOffset += cch;
}

/*virtual*/ str str_istream::peek_chars(std::size_t cchMin) /*override*/ {
   ABC_TRACE_FUNC(this, cchMin);

   // Always return the whole string buffer after m_ichOffset, ignoring cchMin.
   ABC_UNUSED_ARG(cchMin);
   return str(external_buffer, m_psBuf->data() + m_ichOffset, remaining_size_in_chars());
}

/*virtual*/ void str_istream::read_all(str * psDst) /*override*/ {
   ABC_TRACE_FUNC(this, psDst);

   *psDst = _std::move(*m_psBuf);
   m_psBuf = &m_sDefaultBuf;
   m_ichOffset = 0;
}

/*virtual*/ void str_istream::unconsume_chars(str const & s) /*override*/ {
   ABC_TRACE_FUNC(this, s);

   std::size_t cch = s.size_in_chars();
   if (cch > m_ichOffset) {
      // TODO: use a better exception class.
      ABC_THROW(argument_error, ());
   }
   m_ichOffset -= cch;
}

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

str_ostream::str_ostream() :
   stream(),
   str_stream(),
   ostream() {
}

str_ostream::str_ostream(str_ostream && sos) :
   stream(_std::move(sos)),
   str_stream(_std::move(sos)),
   ostream(_std::move(sos)) {
}

str_ostream::str_ostream(external_buffer_t const & eb, str * psBuf) :
   stream(),
   str_stream(eb, psBuf),
   ostream() {
}

/*virtual*/ str_ostream::~str_ostream() {
}

void str_ostream::clear() {
   ABC_TRACE_FUNC(this);

   m_psBuf->set_size_in_chars(0);
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
   return _std::move(*m_psBuf);
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
      m_psBuf->set_capacity(m_ichOffset + cch, true);
      memory::copy(m_psBuf->data() + m_ichOffset, static_cast<char_t const *>(pSrc), cch);
      m_ichOffset += cch;
   } else {
      // Calculate the additional buffer size required.
      std::size_t cbBuf = abc::text::transcode(true, enc, &pSrc, &cbSrc, abc::text::encoding::host);
      m_psBuf->set_capacity(m_ichOffset + cbBuf / sizeof(char_t), true);
      // Transcode the source into the string buffer and advance m_ichOffset accordingly.
      void * pBuf = m_psBuf->data() + m_ichOffset;
      m_ichOffset += abc::text::transcode(
         true, enc, &pSrc, &cbSrc, abc::text::encoding::host, &pBuf, &cbBuf
      ) / sizeof(char_t);
   }
   // Truncate the string.
   m_psBuf->set_size_in_chars(m_ichOffset);
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
