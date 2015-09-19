/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015 Raffaello D. Di Napoli

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


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

str_base::str_base() :
   base(),
   m_ichOffset(0) {
}
str_base::str_base(str_base && sb) :
   base(_std::move(sb)),
   m_ichOffset(sb.m_ichOffset) {
}

/*virtual*/ str_base::~str_base() {
}

/*virtual*/ abc::text::encoding str_base::get_encoding() const /*override*/ {
   return abc::text::encoding::host;
}

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

str_reader::str_reader(str_reader && sr) :
   base(_std::move(sr)),
   str_base(_std::move(sr)),
   reader(_std::move(sr)),
   m_psReadBuf(sr.m_psReadBuf != &sr.m_sReadBuf ? sr.m_psReadBuf : &m_sReadBuf),
   m_sReadBuf(_std::move(sr.m_sReadBuf)) {
   sr.m_psReadBuf = &sr.m_sReadBuf;
}

str_reader::str_reader(str const & s) :
   base(),
   str_base(),
   reader(),
   m_psReadBuf(&m_sReadBuf),
   m_sReadBuf(s) {
}
str_reader::str_reader(str && s) :
   base(),
   str_base(),
   reader(),
   m_psReadBuf(&m_sReadBuf),
   m_sReadBuf(_std::move(s)) {
}
str_reader::str_reader(external_buffer_t const &, str const * ps) :
   base(),
   str_base(),
   reader(),
   m_psReadBuf(ps) {
}

/*virtual*/ str_reader::~str_reader() {
}

/*virtual*/ bool str_reader::read_line_or_all(str * psDst, bool bOneLine) /*override*/ {
   ABC_TRACE_FUNC(this, psDst, bOneLine);

   // TODO: implement this.
   return false;
}

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

str_writer::str_writer() :
   base(),
   str_base(),
   writer(),
   m_psWriteBuf(&m_sDefaultWriteBuf) {
}
str_writer::str_writer(str_writer && sw) :
   base(_std::move(sw)),
   str_base(_std::move(sw)),
   writer(_std::move(sw)),
   m_psWriteBuf(sw.m_psWriteBuf != &sw.m_sDefaultWriteBuf ? sw.m_psWriteBuf : &m_sDefaultWriteBuf),
   m_sDefaultWriteBuf(_std::move(sw.m_sDefaultWriteBuf)) {
   sw.m_psWriteBuf = &sw.m_sDefaultWriteBuf;
}
str_writer::str_writer(external_buffer_t const &, str * psBuf) :
   base(),
   str_base(),
   writer(),
   m_psWriteBuf(psBuf) {
}

/*virtual*/ str_writer::~str_writer() {
}

void str_writer::clear() {
   ABC_TRACE_FUNC(this);

   m_psWriteBuf->set_size_in_chars(0);
   m_ichOffset = 0;
}

/*virtual*/ void str_writer::finalize() /*override*/ {
   // Nothing to do.
}

/*virtual*/ void str_writer::flush() /*override*/ {
   // Nothing to do.
}

str str_writer::release_content() {
   ABC_TRACE_FUNC(this);

   m_ichOffset = 0;
   return _std::move(*m_psWriteBuf);
}

/*virtual*/ void str_writer::write_binary(
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
