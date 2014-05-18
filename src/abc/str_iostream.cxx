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



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::str_istream


namespace abc {

namespace io {

str_istream::str_istream(istr const & s) :
   istream(),
   m_sBuf(s),
   m_ibRead(0) {
}
str_istream::str_istream(istr && s) :
   istream(),
   m_sBuf(std::move(s)),
   m_ibRead(0) {
}
str_istream::str_istream(mstr && s) :
   istream(),
   m_sBuf(std::move(s)),
   m_ibRead(0) {
}
str_istream::str_istream(dmstr && s) :
   istream(),
   m_sBuf(std::move(s)),
   m_ibRead(0) {
}


/*virtual*/ str_istream::~str_istream() {
}


/*virtual*/ istream & str_istream::read_line(mstr * ps) {
   ABC_UNUSED_ARG(ps);
   return *this;
}


size_t str_istream::read_raw(void * p, size_t cbMax) {
   ABC_UNUSED_ARG(p);
   ABC_UNUSED_ARG(cbMax);
   return 0;
}


/*virtual*/ void str_istream::unread_raw(void const * p, size_t cb) {
   ABC_UNUSED_ARG(p);
   ABC_UNUSED_ARG(cb);
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::str_ostream


namespace abc {

namespace io {

str_ostream::str_ostream() :
   ostream(),
   m_ibWrite(0) {
   m_enc = text::encoding::host;
}


/*virtual*/ str_ostream::~str_ostream() {
}


str_ostream::str_type str_ostream::release_content() {
   m_ibWrite = 0;
   return std::move(m_sBuf);
}


/*virtual*/ void str_ostream::write_raw(void const * p, size_t cb, text::encoding enc) {
   ABC_TRACE_FN((this, p, cb, enc));

   if (!cb) {
      // Nothing to do.
      return;
   }
   ABC_ASSERT(enc != text::encoding::unknown, SL("cannot write data with unknown encoding"));

   size_t cbChar(sizeof(str_type::value_type));
   if (enc == m_enc) {
      // Optimal case: no transcoding necessary.
      // Enlarge the string as necessary, then overwrite any character in the affected range.
      m_sBuf.set_capacity((m_ibWrite + cb) / cbChar, true);
      memory::copy(
         reinterpret_cast<int8_t *>(m_sBuf.begin().base()) + m_ibWrite,
         static_cast<int8_t const *>(p),
         cb
      );
      m_ibWrite += cb;
   } else {
      do {
         // Calculate the additional size required, and enlarge the string.
         size_t cbDstEst(text::estimate_transcoded_size(enc, p, cb, m_enc));
         // Add cbChar - 1 to avoid rounding down and losing one character.
         m_sBuf.set_capacity((m_ibWrite + cbDstEst + cbChar - 1) / cbChar, true);
         // Get the resulting buffer and its actual size.
         void * pBuf(reinterpret_cast<int8_t *>(m_sBuf.begin().base()) + m_ibWrite);
         size_t cbBuf(cbChar * m_sBuf.capacity() - m_ibWrite);
         // Fill as much of the buffer as possible, and advance m_ibWrite accordingly.
         m_ibWrite += text::transcode(std::nothrow, enc, &p, &cb, m_enc, &pBuf, &cbBuf);
      } while (cb);
   }
   // Ensure the string knows its own length.
   m_sBuf.set_size(m_ibWrite / cbChar);
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

