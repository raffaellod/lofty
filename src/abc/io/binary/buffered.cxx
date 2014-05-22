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
#include <abc/bitmanip.hxx>
#include <algorithm>
#if ABC_HOST_API_POSIX
   #include <unistd.h> // *_FILENO ssize_t close() isatty() open() read() write()
   #include <fcntl.h> // O_*
   #include <sys/stat.h> // S_*, stat()
// #include <sys/mman.h> // mmap(), munmap(), PROT_*, MAP_*
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary globals


namespace abc {
namespace io {
namespace binary {

std::shared_ptr<buffered_base> buffer(std::shared_ptr<base> pbb) {
   ABC_TRACE_FN((/*pbb*/));

   auto pbr(std::dynamic_pointer_cast<reader>(pbb));
   auto pbw(std::dynamic_pointer_cast<writer>(pbb));
   if (pbr) {
      return std::make_shared<default_buffered_reader>(std::move(pbr));
   }
   if (pbw) {
      return std::make_shared<default_buffered_writer>(std::move(pbw));
   }
   // TODO: use a better exception class.
   ABC_THROW(argument_error, ());
}

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::buffered_reader


namespace abc {
namespace io {
namespace binary {

/*virtual*/ size_t buffered_reader::read(void * p, size_t cbMax) {
   ABC_TRACE_FN((this, p, cbMax));

   size_t cbReadTotal(0);
   while (cbMax) {
      // Attempt to read at least the count of bytes requested by the caller.
      int8_t const * pbBuf;
      size_t cbBuf;
      std::tie(pbBuf, cbBuf) = peek<int8_t>(cbMax);
      if (!cbBuf) {
         // No more data available.
         break;
      }
      // Copy whatever was read into the caller-supplied buffer.
      memory::copy(static_cast<int8_t *>(p), pbBuf, cbBuf);
      cbReadTotal += cbBuf;
      // Advance the pointer and decrease the count of bytes to read, so that the next call will
      // attempt to fill in the remaining buffer space.
      p = static_cast<int8_t *>(p) + cbBuf;
      cbMax -= cbBuf;
   }
   return cbReadTotal;
}

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::buffered_writer


namespace abc {
namespace io {
namespace binary {

/*virtual*/ size_t buffered_writer::write(void const * p, size_t cb) {
   ABC_TRACE_FN((this, p, cb));

   // Obtain a buffer large enough.
   int8_t * pbBuf;
   size_t cbBuf;
   std::tie(pbBuf, cbBuf) = get_buffer<int8_t>(cb);
   // Copy the source data into the buffer.
   memory::copy(pbBuf, static_cast<int8_t const *>(p), cb);
   return cb;
}

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::default_buffered_reader


namespace abc {
namespace io {
namespace binary {

default_buffered_reader::default_buffered_reader(std::shared_ptr<reader> pbr) :
   m_pbr(std::move(pbr)),
   m_cbReadBuf(0),
   m_ibReadBufUsed(0),
   m_cbReadBufUsed(0) {
}


/*virtual*/ default_buffered_reader::~default_buffered_reader() {
}


/*virtual*/ void default_buffered_reader::consume_bytes(size_t cb) {
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


/*virtual*/ std::pair<void const *, size_t> default_buffered_reader::peek_bytes(size_t cb) {
   ABC_TRACE_FN((this, cb));

   if (cb > m_cbReadBufUsed) {
      // The caller wants more data than what’s currently in the buffer: try to load more.
      if (m_ibReadBufUsed + m_cbReadBufUsed == m_cbReadBuf) {
         // No more room in the buffer. If the “used window” is at an offset (m_ibReadBufUsed > 0),
         // shift it backwards to offset 0, and we’ll use the resulting free space (m_ibReadBufUsed
         // bytes); otherwise just enlarge the buffer.
         if (m_ibReadBufUsed > 0) {
            if (m_cbReadBufUsed) {
               memory::move(
                  m_pbReadBuf.get(), m_pbReadBuf.get() + m_ibReadBufUsed, m_cbReadBufUsed
               );
            }
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


/*virtual*/ std::shared_ptr<base> default_buffered_reader::unbuffered() const {
   ABC_TRACE_FN((this));

   return std::dynamic_pointer_cast<base>(m_pbr);
}

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::default_buffered_writer


namespace abc {
namespace io {
namespace binary {

default_buffered_writer::default_buffered_writer(std::shared_ptr<writer> pbw) :
   m_pbw(std::move(pbw)),
   m_cbWriteBuf(0),
   m_cbWriteBufUsed(0) {
}


/*virtual*/ default_buffered_writer::~default_buffered_writer() {
   ABC_TRACE_FN((this));

   flush_buffer();
}


/*virtual*/ void default_buffered_writer::flush() {
   ABC_TRACE_FN((this));

   // Flush both the write buffer and any lower-level buffers.
   flush_buffer();
   m_pbw->flush();
}


/*virtual*/ void default_buffered_writer::commit_bytes(size_t cb) {
   ABC_TRACE_FN((this, cb));

   if (cb > m_cbWriteBuf - m_cbWriteBufUsed) {
      // Can’t commit more bytes than are available in the write buffer.
      // TODO: use a better exception class.
      ABC_THROW(argument_error, ());
   }
   // Increase the count of used bytes in the buffer. If that makes the buffer full, flush it.
   m_cbWriteBufUsed += cb;
   if (m_cbWriteBufUsed == m_cbWriteBuf) {
      flush_buffer();
   }
}


void default_buffered_writer::flush_buffer() {
   ABC_TRACE_FN((this));

   if (m_cbWriteBufUsed) {
      size_t cbWritten(m_pbw->write(m_pbWriteBuf.get(), m_cbWriteBufUsed));
      ABC_ASSERT(cbWritten == m_cbWriteBufUsed, SL("the entire buffer must have been written"));
      m_cbWriteBufUsed = 0;
   }
}


/*virtual*/ std::pair<void *, size_t> default_buffered_writer::get_buffer_bytes(size_t cb) {
   ABC_TRACE_FN((this, cb));

   size_t cbWriteBufAvail(m_cbWriteBuf - m_cbWriteBufUsed);
   // If the requested buffer size is more that is currently available, flush the buffer.
   if (cb > cbWriteBufAvail) {
      flush_buffer();
      // If the buffer is still too small, enlarge it.
      if (cb > m_cbWriteBuf) {
         size_t cbWriteBufNew(bitmanip::ceiling_to_pow2_multiple(cb, smc_cbWriteBufDefault));
         memory::realloc(&m_pbWriteBuf, cbWriteBufNew);
         m_cbWriteBuf = cbWriteBufNew;
      }
      cbWriteBufAvail = m_cbWriteBuf;
   }
   // Return the available portion of the buffer.
   return std::pair<void *, size_t>(m_pbWriteBuf.get() + m_cbWriteBufUsed, cbWriteBufAvail);
}


/*virtual*/ std::shared_ptr<base> default_buffered_writer::unbuffered() const {
   ABC_TRACE_FN((this));

   return std::dynamic_pointer_cast<base>(m_pbw);
}

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

