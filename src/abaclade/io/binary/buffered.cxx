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
   if (m_pbr->async_pending()) {
      // Block to avoid segfaults due to the buffer being deallocated while in use by the kernel.
      m_pbr->async_join();
      /* TODO: warn that this is a bug because I/O errors are not being checked for. Also, the read
      bytes will be discarded, but that may be intentional. */
   }
}

/*virtual*/ std::size_t default_buffered_reader::async_join() /*override*/ {
   ABC_TRACE_FUNC(this);

   if (m_pbr->async_pending()) {
      m_pbr->async_join();
      // TODO: under Win32, do this:
      //m_cbReadBufUsed += cbRead;
   }
}

/*virtual*/ bool default_buffered_reader::async_pending() const /*override*/ {
   ABC_TRACE_FUNC(this);

   return m_pbr->async_pending();
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
      // Account for the additional data read.
      // TODO: decide whether pending I/O should return 0 or tuple<size_t, bool>.
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
   m_cbUsed -= cb;
   memory::move(get(), get() + cb, m_cbUsed);
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
      bAsyncPending = m_pbw->async_pending();
   }
   if (bAsyncPending) {
      // Block to avoid segfaults due to the buffer being deallocated while in use by the kernel.
      m_pbw->async_join();
      // TODO: warn that this is a bug because I/O errors are not being checked for.
   }
}

bool default_buffered_writer::any_buffered_data() const {
   ABC_TRACE_FUNC(this);

   return !m_lbufWriteBufs.empty() && m_lbufWriteBufs.front().used_size() > 0;
}

/*virtual*/ std::size_t default_buffered_writer::async_join() /*override*/ {
   ABC_TRACE_FUNC(this);

   if (m_lbufWriteBufs.empty()) {
      // No buffer, so certainly no pending I/O.
      return 0;
   }
   // Flush all buffers.
   std::size_t cbWrittenTotal = 0;
   buffer * pbufBack = &m_lbufWriteBufs.back();
   // If there was a pending I/O operation, get its result now.
   if (std::size_t cbWritten = m_pbw->async_join()) {
      pbufBack->mark_as_available(cbWritten);
      cbWrittenTotal += cbWritten;
   }
   do {
      if (!pbufBack->used_size()) {
         // The buffer is empty; if it’s the only buffer, keep it but quit the loop.
         if (m_lbufWriteBufs.size() == 1) {
            break;
         }
         // Discard this now-unnecessary extra buffer, and move to the next one.
         // TODO: recycle buffers by using a class-level buffer pool.
         m_lbufWriteBufs.remove_back();
         pbufBack = &m_lbufWriteBufs.back();
         // *pbufBack must have used_size() > 0, otherwise it wouldn’t exist.
      }
      /* TODO: flushing before the buffer is full (used_size() == size()) is a mistake if *m_pbw
      wants writes of an integer multiple of its block size. The only fix is implementing
      aligned_buffered_writer that regroups writes in blocks of the correct size, without trying to
      solve too many issues here ‒ divide et impera. */
      std::size_t cbWritten = m_pbw->write(pbufBack->get(), pbufBack->used_size());
      // cbWritten may be 0.
      // TODO: decide whether pending I/O should return 0 or tuple<size_t, bool>.
      pbufFront->mark_as_available(cbWritten);
      cbWrittenTotal += cbWritten;
   } while (!m_lbufWriteBufs.empty() && !m_pbw->async_pending());
   return cbWrittenTotal;
}

/*virtual*/ bool default_buffered_writer::async_pending() const /*override*/ {
   ABC_TRACE_FUNC(this);

   return m_pbw->async_pending();
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

// Flushes all buffers, blocking as necessary, then proceeds to flushing the underlying I/O object.
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
      /* If still waiting on an earlier flush, complete it now. back() could be the same as front(),
      but that doesn’t matter; what’s important is that if *pbufBack shifts its contents as part of
      mark_as_available(), it does it before we pass a portion of the same buffer (via *pbufFront)
      to m_pbw->write(). */
      buffer * pbufBack = &m_lbufWriteBufs.back();
      if (std::size_t cbWritten = m_pbw->async_join()) {
         pbufBack->mark_as_available(cbWritten);
      }

      buffer * pbufFront = &m_lbufWriteBufs.front();
      // Check only at this point, since the above mark_as_available() may have emptied the buffer.
      if (pbufFront->used_size()) {
         std::size_t cbWritten = m_pbw->write(pbufFront->get(), pbufFront->used_size());
         if (m_pbw->async_pending()) {
            // Flushing is still in progress.
            return false;
         }
         pbufFront->mark_as_available(cbWritten);
      }
   }
   return true;
}

/*virtual*/ std::pair<void *, std::size_t> default_buffered_writer::get_buffer_bytes(
   std::size_t cb
) /*override*/ {
   ABC_TRACE_FUNC(this, cb);

   // Flush buffer(s) as long as flushing doesn’t block.
   if (!m_lbufWriteBufs.empty() && !m_pbw->async_pending()) {
      buffer * pbufBack = &m_lbufWriteBufs.back();
      // If there was a pending I/O operation, get its result now.
      if (std::size_t cbWritten = m_pbw->async_join()) {
         pbufBack->mark_as_available(cbWritten);
      }
      do {
         if (!pbufBack->used_size()) {
            // The buffer is empty; if it’s the only buffer, keep it but quit the loop.
            if (m_lbufWriteBufs.size() == 1) {
               break;
            }
            // Discard this now-unnecessary extra buffer, and move to the next one.
            // TODO: recycle buffers by using a class-level buffer pool.
            m_lbufWriteBufs.remove_back();
            pbufBack = &m_lbufWriteBufs.back();
            // *pbufBack must have used_size() > 0, otherwise it wouldn’t exist.
         }
         /* TODO: flushing before the buffer is full (used_size() == size()) is a mistake if *m_pbw
         wants writes of an integer multiple of its block size. The only fix is implementing
         aligned_buffered_writer that regroups writes in blocks of the correct size, without trying
         to solve too many issues here ‒ divide et impera. */
         std::size_t cbWritten = m_pbw->write(pbufBack->get(), pbufBack->used_size());
         // cbWritten may be 0.
         // TODO: decide whether pending I/O should return 0 or tuple<size_t, bool>.
         pbufFront->mark_as_available(cbWritten);
      } while (!m_lbufWriteBufs.empty() && !m_pbw->async_pending());
   }

   // Use the front() buffer if it has enough space available, or add a new buffer.
   buffer * pbuf = nullptr;
   if (m_lbufWriteBufs.empty()) {
      pbuf = nullptr;
   } else {
      pbuf = &m_lbufWriteBufs.front();
      if (cb > pbuf->available_size()) {
         pbuf = nullptr;
      }
   }
   if (!pbuf) {
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
