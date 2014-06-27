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
// abc::io::text::str_base


namespace abc {
namespace io {
namespace text {

str_base::str_base(abc::text::line_terminator lterm /*= abc::text::line_terminator::host*/) :
   base(lterm),
   m_ichOffset(0) {
}


/*virtual*/ str_base::~str_base() {
}


/*virtual*/ abc::text::encoding str_base::encoding() const {
   return abc::text::encoding::host;
}


/*virtual*/ abc::text::line_terminator str_base::line_terminator() const {
   ABC_TRACE_FUNC(this);

   return m_lterm;
}

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::str_reader


namespace abc {
namespace io {
namespace text {

str_reader::str_reader(
   istr const & s, abc::text::line_terminator lterm /*= abc::text::line_terminator::host*/
) :
   base(lterm),
   str_base(lterm),
   reader(lterm),
   m_psReadBuf(&m_sReadBuf),
   m_sReadBuf(s) {
}
str_reader::str_reader(
   istr && s, abc::text::line_terminator lterm /*= abc::text::line_terminator::host*/
) :
   base(lterm),
   str_base(lterm),
   reader(lterm),
   m_psReadBuf(&m_sReadBuf),
   m_sReadBuf(std::move(s)) {
}
str_reader::str_reader(
   mstr && s, abc::text::line_terminator lterm /*= abc::text::line_terminator::host*/
) :
   base(lterm),
   str_base(lterm),
   reader(lterm),
   m_psReadBuf(&m_sReadBuf),
   m_sReadBuf(std::move(s)) {
}


/*virtual*/ bool str_reader::read_while(mstr * ps, std::function<
   istr::const_iterator (istr const & sRead, istr::const_iterator itLastReadBegin)
> fnGetConsumeEnd) {
   ABC_TRACE_FUNC(this, ps/*, fnGetConsumeEnd*/);

   ABC_UNUSED_ARG(fnGetConsumeEnd);
   // TODO: implement this.
   return false;
}

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::str_writer


namespace abc {
namespace io {
namespace text {

str_writer::str_writer(
   mstr * psBuf /*= nullptr*/,
   abc::text::line_terminator lterm /*= abc::text::line_terminator::host*/
) :
   base(lterm),
   str_base(lterm),
   writer(lterm),
   m_psWriteBuf(psBuf ? psBuf : &m_sDefaultWriteBuf) {
}


void str_writer::clear() {
   ABC_TRACE_FUNC(this);

   m_psWriteBuf->set_size_in_chars(0);
   m_ichOffset = 0;
}


dmstr str_writer::release_content() {
   ABC_TRACE_FUNC(this);

   m_ichOffset = 0;
   return std::move(*m_psWriteBuf);
}


/*virtual*/ void str_writer::write_binary(void const * p, size_t cb, abc::text::encoding enc) {
   ABC_TRACE_FUNC(this, p, cb, enc);

   if (!cb) {
      // Nothing to do.
      return;
   }
   ABC_ASSERT(enc != abc::text::encoding::unknown, SL("cannot write data with unknown encoding"));
   if (enc == abc::text::encoding::host) {
      // Optimal case: no transcoding necessary.
      size_t cch(cb / sizeof(char_t));
      // Enlarge the string as necessary, then overwrite any character in the affected range.
      m_psWriteBuf->set_capacity(m_ichOffset + cch, true);
      memory::copy(m_psWriteBuf->begin().base() + m_ichOffset, static_cast<char_t const *>(p), cch);
      m_ichOffset += cch;
   } else {
      // Calculate the additional buffer size required.
      size_t cbBuf(abc::text::transcode(std::nothrow, enc, &p, &cb, abc::text::encoding::host));
      m_psWriteBuf->set_capacity(m_ichOffset + cbBuf / sizeof(char_t), true);
      // Transcode the source into the string buffer and advance m_ichOffset accordingly.
      void * pBuf(m_psWriteBuf->begin().base() + m_ichOffset);
      m_ichOffset += abc::text::transcode(
         std::nothrow, enc, &p, &cb, abc::text::encoding::host, &pBuf, &cbBuf
      ) / sizeof(char_t);
   }
   // Truncate the string.
   m_psWriteBuf->set_size_in_chars(m_ichOffset);
}

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

