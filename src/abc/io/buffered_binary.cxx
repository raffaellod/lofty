/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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
#include <abc/io/buffered_binary.hxx>
#include <abc/numeric.hxx>
#include <abc/trace.hxx>
#include <algorithm>
#if ABC_HOST_API_POSIX
   #include <unistd.h> // *_FILENO ssize_t close() isatty() open() read() write()
   #include <fcntl.h> // O_*
   #include <sys/stat.h> // S_*, stat()
// #include <sys/mman.h> // mmap(), munmap(), PROT_*, MAP_*
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io globals


namespace abc {

namespace io {

std::shared_ptr<buffered_binary_base> buffer_binary(std::shared_ptr<binary_base> pbb) {
   ABC_TRACE_FN((/*pbb*/));

   auto pbr(std::dynamic_pointer_cast<binary_reader>(pbb));
   auto pbw(std::dynamic_pointer_cast<binary_writer>(pbb));
   if (pbr) {
      return std::make_shared<default_buffered_binary_reader>(std::move(pbr));
   }
   if (pbw) {
      return std::make_shared<default_buffered_binary_writer>(std::move(pbw));
   }
   // TODO: use a better exception class.
   ABC_THROW(argument_error, ());
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::buffered_binary_reader


namespace abc {

namespace io {

/*virtual*/ size_t buffered_binary_reader::read(void * p, size_t cbMax) {
   ABC_TRACE_FN((this, p, cbMax));

   size_t cbReadTotal(0);
   while (cbMax > 0) {
      int8_t const * pbReadBuf;
      size_t cbRead;
      // Attempt to read at least the count of bytes requested by the caller.
      std::tie(pbReadBuf, cbRead) = peek<int8_t>(cbMax);
      if (!cbRead) {
         // No more data available.
         break;
      }
      // Copy whatever was read into the caller-supplied buffer.
      memory::copy<void>(p, pbReadBuf, cbRead);
      cbReadTotal += cbRead;
      // Advance the pointer and decrease the count of bytes to read, so that the next call will
      // attempt to fill in the remaining bytes
      p = static_cast<int8_t *>(p) + cbRead;
      cbMax -= cbRead;
   }
   return cbReadTotal;
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::default_buffered_binary_reader


namespace abc {

namespace io {

default_buffered_binary_reader::default_buffered_binary_reader(std::shared_ptr<binary_reader> pbr) :
   m_pbr(std::move(pbr)),
   m_cbReadBuf(0),
   m_ibReadBufUsed(0),
   m_cbReadBufUsed(0) {
}


/*virtual*/ default_buffered_binary_reader::~default_buffered_binary_reader() {
}


/*virtual*/ void default_buffered_binary_reader::consume(size_t cb) {
   ABC_TRACE_FN((this, cb));

   if (cb > m_cbReadBufUsed) {
      // Can’t consume more bytes than are available in the read buffer.
      // TODO: use a better exception class.
      ABC_THROW(argument_error, ());
   }
   // Shift the “used window” of the read buffer by cb bytes.
   m_ibReadBufUsed += cb;
   m_cbReadBufUsed -= cb;
}


/*virtual*/ std::pair<void const *, size_t> default_buffered_binary_reader::_peek_void(size_t cb) {
   ABC_TRACE_FN((this, cb));

   if (cb > m_cbReadBufUsed) {
      // The caller wants more data than what’s currently in the buffer: try to load more.
      if (m_ibReadBufUsed + m_cbReadBufUsed == m_cbReadBuf) {
         // No more room in the buffer. If the “used window” is at an offset (m_ibReadBufUsed > 0),
         // shift it backwards to offset 0, and we’ll use the resulting free space (m_ibReadBufUsed
         // bytes); otherwise just enlarge the buffer.
         if (m_ibReadBufUsed > 0) {
            memory::move(m_pbReadBuf.get(), m_pbReadBuf.get() + m_ibReadBufUsed, m_cbReadBufUsed);
            m_ibReadBufUsed = 0;
         } else {
            size_t cbReadBufNew(m_cbReadBuf + smc_cbReadBufDefault);
            // Check for overflow.
            if (cbReadBufNew < m_cbReadBuf) {
               cbReadBufNew = numeric::max<size_t>::value;
            }
            memory::realloc(&m_pbReadBuf, cbReadBufNew);
            m_cbReadBuf = cbReadBufNew;
         }
      }
      // Try to fill the buffer.
      size_t cbRead(m_pbr->read(
         m_pbReadBuf.get(), m_cbReadBuf - (m_ibReadBufUsed + m_cbReadBufUsed)
      ));
      // Account for the additional data.
      m_cbReadBufUsed += cbRead;
   }
   // Return the “used window” of the buffer.
   return std::make_pair(m_pbReadBuf.get() + m_ibReadBufUsed, m_cbReadBufUsed);
}


/*virtual*/ std::shared_ptr<binary_base> default_buffered_binary_reader::unbuffered() const {
   ABC_TRACE_FN((this));

   return std::dynamic_pointer_cast<binary_base>(m_pbr);
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::default_buffered_binary_writer


namespace abc {

namespace io {

default_buffered_binary_writer::default_buffered_binary_writer(std::shared_ptr<binary_writer> pbw) :
   m_pbw(std::move(pbw)) {
}


/*virtual*/ default_buffered_binary_writer::~default_buffered_binary_writer() {
}


/*virtual*/ void default_buffered_binary_writer::flush() {
   ABC_TRACE_FN((this));

   m_pbw->flush();
}


/*virtual*/ std::shared_ptr<binary_base> default_buffered_binary_writer::unbuffered() const {
   ABC_TRACE_FN((this));

   return std::dynamic_pointer_cast<binary_base>(m_pbw);
}


/*virtual*/ size_t default_buffered_binary_writer::write(void const * p, size_t cb) {
   ABC_TRACE_FN((this, p, cb));

   return m_pbw->write(p, cb);
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

