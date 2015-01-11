/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014, 2015
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
#include <abaclade/bitmanip.hxx>

#include <algorithm>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary globals

namespace abc {
namespace io {
namespace binary {

std::shared_ptr<buffered_base> buffer(std::shared_ptr<base> pbb) {
   ABC_TRACE_FUNC(pbb);

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

/*virtual*/ std::size_t buffered_reader::read(void * p, std::size_t cbMax) /*override*/ {
   ABC_TRACE_FUNC(this, p, cbMax);

   std::size_t cbReadTotal(0);
   while (cbMax) {
      // Attempt to read at least the count of bytes requested by the caller.
      std::int8_t const * pbBuf;
      std::size_t cbBuf;
      std::tie(pbBuf, cbBuf) = peek<std::int8_t>(cbMax);
      if (!cbBuf) {
         // No more data available.
         break;
      }
      // Copy whatever was read into the caller-supplied buffer.
      memory::copy(static_cast<std::int8_t *>(p), pbBuf, cbBuf);
      cbReadTotal += cbBuf;
      // Advance the pointer and decrease the count of bytes to read, so that the next call will
      // attempt to fill in the remaining buffer space.
      p = static_cast<std::int8_t *>(p) + cbBuf;
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

/*virtual*/ std::size_t buffered_writer::write(void const * p, std::size_t cb) /*override*/ {
   ABC_TRACE_FUNC(this, p, cb);

   // Obtain a buffer large enough.
   std::int8_t * pbBuf;
   std::size_t cbBuf;
   std::tie(pbBuf, cbBuf) = get_buffer<std::int8_t>(cb);
   // Copy the source data into the buffer.
   memory::copy(pbBuf, static_cast<std::int8_t const *>(p), cb);
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
   /* TODO: if async read in progress, block to avoid segfault (from the buffer going away) and warn
   that this is a bug because read errors are not being checked for (the read bytes will be
   discarded too, but that may be on purpose). */
}

/*virtual*/ void default_buffered_reader::consume_bytes(std::size_t cb) /*override*/ {
   ABC_TRACE_FUNC(this, cb);

   if (cb > m_cbReadBufUsed) {
      // Can’t consume more bytes than are available in the read buffer.
      // TODO: use a better exception class.
      ABC_THROW(argument_error, ());
   }
   // Shift the “used window” of the read buffer by cb bytes.
   m_ibReadBufUsed += cb;
   m_cbReadBufUsed -= cb;
}

/*virtual*/ std::pair<void const *, std::size_t> default_buffered_reader::peek_bytes(
   std::size_t cb
) /*override*/ {
   ABC_TRACE_FUNC(this, cb);

   if (cb > m_cbReadBufUsed) {
      // The caller wants more data than what’s currently in the buffer: try to load more.
      if (m_ibReadBufUsed + m_cbReadBufUsed == m_cbReadBuf) {
         /* No more room in the buffer. If the “used window” is at an offset (m_ibReadBufUsed > 0),
         shift it backwards to offset 0, and we’ll use the resulting free space (m_ibReadBufUsed
         bytes); otherwise just enlarge the buffer. */

         if (m_ibReadBufUsed > 0) {
            if (m_cbReadBufUsed) {
               memory::move(
                  m_pbReadBuf.get(), m_pbReadBuf.get() + m_ibReadBufUsed, m_cbReadBufUsed
               );
            }
            m_ibReadBufUsed = 0;
         } else {
            std::size_t cbReadBufNew = m_cbReadBuf + smc_cbReadBufDefault;
            // Check for overflow.
            if (cbReadBufNew < m_cbReadBuf) {
               cbReadBufNew = numeric::max<std::size_t>::value;
            }
            memory::realloc(&m_pbReadBuf, cbReadBufNew);
            m_cbReadBuf = cbReadBufNew;
         }
      }
      // Try to fill the buffer.
      std::size_t cbRead = m_pbr->read(
         m_pbReadBuf.get(), m_cbReadBuf - (m_ibReadBufUsed + m_cbReadBufUsed)
      );
      // Account for the additional data.
      // TODO: don’t do this if async read in progress.
      m_cbReadBufUsed += cbRead;
   }
   // Return the “used window” of the buffer.
   return std::make_pair(m_pbReadBuf.get() + m_ibReadBufUsed, m_cbReadBufUsed);
}

/*virtual*/ std::shared_ptr<base> default_buffered_reader::_unbuffered_base() const /*override*/ {
   ABC_TRACE_FUNC(this);

   return std::static_pointer_cast<base>(m_pbr);
}

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::default_buffered_writer

namespace abc {
namespace io {
namespace binary {

default_buffered_writer::buffer::buffer(std::size_t cb) :
   m_p(memory::_raw_alloc(cb)),
   m_cb(cb),
   m_cbUsed(0) {
}

default_buffered_writer::buffer::~buffer() {
   if (m_p) {
      memory::_raw_free(m_p);
   }
}

void default_buffered_writer::buffer::mark_as_available(std::size_t cb) {
   memory::move(get(), get() + cb, m_cbUsed);
   m_cbUsed -= cb;
}


default_buffered_writer::default_buffered_writer(std::shared_ptr<writer> pbw) :
   m_pbw(std::move(pbw)) {
}

/*virtual*/ default_buffered_writer::~default_buffered_writer() {
   ABC_TRACE_FUNC(this);

   bool bAsyncPending;
   if (any_buffered_data()) {
      bAsyncPending = flush_buffer();
   } else {
      bAsyncPending = false /*TODO: m_pbw->async_pending()*/;
   }
   if (bAsyncPending) {
      /* TODO: block to avoid segfault (from the buffer going away) and warn that this is a bug
      because write errors are not being checked for. */
   }
}

bool default_buffered_writer::any_buffered_data() const {
   ABC_TRACE_FUNC(this);

   return !m_lbufWriteBufs.empty() && m_lbufWriteBufs.front().used_size() > 0;
}

/*virtual*/ void default_buffered_writer::commit_bytes(std::size_t cb) /*override*/ {
   ABC_TRACE_FUNC(this, cb);

   if (!cb) {
      return;
   }
   if (m_lbufWriteBufs.empty()) {
      // Can’t commit any bytes without a write buffer.
      // TODO: use a better exception class.
      ABC_THROW(argument_error, ());
   }
   buffer & buf = m_lbufWriteBufs.front();
   if (cb > buf.available_size()) {
      // Can’t commit more bytes than are available in the write buffer.
      // TODO: use a better exception class.
      ABC_THROW(argument_error, ());
   }
   // Increase the count of used bytes in the buffer. If that makes the buffer full, flush it.
   buf.mark_as_used(cb);
   if (!buf.available_size()) {
      flush_buffer();
   }
}

/*virtual*/ void default_buffered_writer::flush() /*override*/ {
   ABC_TRACE_FUNC(this);

   /* Flush any buffered bytes in m_lbufWriteBufs.front(). This will block if a previous write is
   still pending. */
   flush_buffer();
   /* Flush any lower-level buffers. This will block if flush_buffer() resulted in a new pending
   write, or if there was already one and flush_buffer() didn’t need to wait for it. */
   m_pbw->flush();
}

bool default_buffered_writer::flush_buffer() {
   ABC_TRACE_FUNC(this);

   if (!m_lbufWriteBufs.empty()) {
      buffer & buf = m_lbufWriteBufs.front();
      if (buf.used_size()) {
         // TODO: block if an earlier async write is still in progress.
         std::size_t cbWritten = m_pbw->write(buf.get(), buf.used_size());
         if (false /* TODO: async write in progress */) {
            return false;
         }
         buf.mark_as_available(cbWritten);
      }
   }
   return true;
}

/*virtual*/ std::pair<void *, std::size_t> default_buffered_writer::get_buffer_bytes(
   std::size_t cb
) /*override*/ {
   ABC_TRACE_FUNC(this, cb);

   bool bCreateNewBuffer = false;
   buffer * pbuf;
   if (m_lbufWriteBufs.empty()) {
      // No buffer available: create one of sufficient size.
      bCreateNewBuffer = true;
   } else {
      if (false /*TODO: m_pbw->async_pending()*/) {
         /* This is a good time to discard any buffers that have been written asynchronously and are
         now idle, since only one at a time may be in the process of being written asynchronously.
         TODO: discard old buffers in more spots (and move to a separate method). */
         while (m_lbufWriteBufs.size() > 1) {
            m_lbufWriteBufs.remove_back();
         }
      }
      pbuf = &m_lbufWriteBufs.front();
      // If the requested size is more than is currently available in *pbuf, flush the buffer.
      if (cb > pbuf->available_size()) {
         /* TODO: flushing before the buffer is full is a mistake if *m_pbw wants writes of an
         integer multiple of its block size. The only fix is implementing aligned_buffered_writer
         that regroups writes in blocks of the correct size, without trying to solve too many issues
         here ‒ divide et impera. */
         if (flush_buffer()) {
            if (cb > pbuf->size()) {
               /* Flushing completed synchronously but the buffer is too small anyway, so throw it
               away and create a new one. */
               m_lbufWriteBufs.remove_front();
               bCreateNewBuffer = true;
            }
         } else {
            // Flushing did not complete synchronously, need a new buffer.
            bCreateNewBuffer = true;
         }
      }
   }
   if (bCreateNewBuffer) {
      std::size_t cbWriteBuf = bitmanip::ceiling_to_pow2_multiple(cb, smc_cbWriteBufDefault);
      m_lbufWriteBufs.push_front(buffer(cbWriteBuf));
      pbuf = &m_lbufWriteBufs.front();
   }
   // Return the available portion of the buffer.
   return std::make_pair(pbuf->get() + pbuf->used_size(), pbuf->available_size());
}

/*virtual*/ std::shared_ptr<base> default_buffered_writer::_unbuffered_base() const /*override*/ {
   ABC_TRACE_FUNC(this);

   return std::static_pointer_cast<base>(m_pbw);
}

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
