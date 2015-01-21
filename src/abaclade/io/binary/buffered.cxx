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
// abc::io::binary::detail::buffer

namespace abc {
namespace io {
namespace binary {
namespace detail {

buffer::buffer(std::size_t cb) :
   m_p(memory::_raw_alloc(cb)),
   m_cb(cb),
   m_ibAvailableOffset(0),
   m_ibUsedOffset(0) {
}
buffer::buffer(buffer && buf) :
   m_p(buf.m_p),
   m_cb(buf.m_cb),
   m_ibAvailableOffset(buf.m_ibAvailableOffset),
   m_ibUsedOffset(buf.m_ibUsedOffset) {
   buf.m_p = nullptr;
   buf.m_cb = 0;
   buf.m_ibAvailableOffset = 0;
   buf.m_ibUsedOffset = 0;
}

buffer::~buffer() {
   // TODO: pool the buffer memory blocks.
   if (m_p) {
      memory::_raw_free(m_p);
   }
}

buffer & buffer::operator=(buffer && buf) {
   ABC_TRACE_FUNC(this/*, buf*/);

   // TODO: pool the buffer memory blocks.
   if (m_p) {
      memory::_raw_free(m_p);
   }

   m_p = buf.m_p;
   buf.m_p = nullptr;
   m_cb = buf.m_cb;
   buf.m_cb = 0;
   m_ibAvailableOffset = buf.m_ibAvailableOffset;
   buf.m_ibAvailableOffset = 0;
   m_ibUsedOffset = buf.m_ibUsedOffset;
   buf.m_ibUsedOffset = 0;
   return *this;
}

void buffer::make_unused_available() {
   ABC_TRACE_FUNC(this);

   memory::move(static_cast<std::int8_t *>(m_p), get_used(), used_size());
   m_ibAvailableOffset = 0;
}

} //namespace detail
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
      /* Advance the pointer and decrease the count of bytes to read, so that the next call will
      attempt to fill in the remaining buffer space. */
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
   m_pbr(std::move(pbr)) {
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

   std::size_t cbRead = m_pbr->async_join();
   m_bufReadMain.mark_as_used(cbRead);
   return cbRead;
}

/*virtual*/ bool default_buffered_reader::async_pending() /*override*/ {
   ABC_TRACE_FUNC(this);

   return m_pbr->async_pending();
}

/*virtual*/ void default_buffered_reader::consume_bytes(std::size_t cb) /*override*/ {
   ABC_TRACE_FUNC(this, cb);

   if (cb > m_bufReadMain.used_size()) {
      // Can’t consume more bytes than are available in the read buffer.
      // TODO: use a better exception class.
      ABC_THROW(argument_error, ());
   }
   // Shift the “used window” of the read buffer by cb bytes.
   m_bufReadMain.mark_as_unused(cb);
}

/*virtual*/ std::pair<void const *, std::size_t> default_buffered_reader::peek_bytes(
   std::size_t cb
) /*override*/ {
   ABC_TRACE_FUNC(this, cb);

   if (cb > m_bufReadMain.used_size()) {
      if (m_pbr->async_pending()) {
         // TODO: decide whether pending I/O should return (nullptr, 0) or tuple<(nullptr, 0), bool>.
         return std::make_pair(nullptr, 0);
      }
      // The caller wants more data than what’s currently in the buffer: try to load more.
      std::size_t cbReadMin = cb - m_bufReadMain.used_size();
      if (cbReadMin > m_bufReadMain.available_size()) {
         /* The buffer does’t have enough available space to hold the data that needs to be read;
         see if compacting it would create enough room. */
         if (m_bufReadMain.unused_size() + m_bufReadMain.available_size() >= cbReadMin) {
            m_bufReadMain.make_unused_available();
         } else {
            // Not enough room; create a new buffer, making sure it’s large enough.
            std::size_t cbReadBuf = bitmanip::ceiling_to_pow2_multiple(cb, smc_cbReadBufDefault);
            m_bufReadMain = detail::buffer(cbReadBuf);
         }
      }
      // Try to fill the available part of the buffer.
      std::size_t cbRead = m_pbr->read(
         m_bufReadMain.get_available(), m_bufReadMain.available_size()
      );
      // TODO: decide whether pending I/O should return (nullptr, 0) or tuple<(nullptr, 0), bool>.
      if (m_pbr->async_pending()) {
         return std::make_pair(nullptr, 0);
      }
      // Account for the additional data read.
      m_bufReadMain.mark_as_used(cbRead);
   }
   // Return the “used window” of the buffer.
   return std::make_pair(m_bufReadMain.get_used(), m_bufReadMain.used_size());
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

default_buffered_writer::default_buffered_writer(std::shared_ptr<writer> pbw) :
   m_pbw(std::move(pbw)) {
}

/*virtual*/ default_buffered_writer::~default_buffered_writer() {
   ABC_TRACE_FUNC(this);

   if (m_pbw->async_pending()) {
      // TODO: warn that this is a bug because I/O errors are not being checked for.
   }
   flush_all_buffers();
}

/*virtual*/ std::size_t default_buffered_writer::async_join() /*override*/ {
   ABC_TRACE_FUNC(this);

   if (m_lbufWriteBufs.empty()) {
      // No buffers, so certainly no pending I/O.
      return 0;
   } else {
      std::size_t cbWrittenTotal;
      // If there was a pending I/O operation, get its result now.
      if (std::size_t cbWritten = m_pbw->async_join()) {
         buffer_write_complete(cbWritten);
         cbWrittenTotal = cbWritten;
      } else {
         cbWrittenTotal = 0;
      }
      // Now that no I/O is pending, see if we can flush more buffers.
      cbWrittenTotal += flush_nonblocking_full_buffers();
      return cbWrittenTotal;
   }
}

/*virtual*/ bool default_buffered_writer::async_pending() /*override*/ {
   ABC_TRACE_FUNC(this);

   return m_pbw->async_pending();
}

void default_buffered_writer::buffer_write_complete(std::size_t cbWritten) {
   ABC_TRACE_FUNC(this, cbWritten);

   detail::buffer * pbuf = &m_lbufWriteBufs.back();
   pbuf->mark_as_unused(cbWritten);
   if (!pbuf->used_size()) {
      // Discard this now-unnecessary extra buffer.
      // TODO: recycle buffers by using a class-level buffer pool.
      m_lbufWriteBufs.remove_back();
   }
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
   detail::buffer * pbuf = &m_lbufWriteBufs.front();
   if (cb > pbuf->available_size()) {
      // Can’t commit more bytes than are available in the write buffer.
      // TODO: use a better exception class.
      ABC_THROW(argument_error, ());
   }
   pbuf->mark_as_used(cb);
   flush_nonblocking_full_buffers();
}

/*virtual*/ void default_buffered_writer::flush() /*override*/ {
   ABC_TRACE_FUNC(this);

   flush_all_buffers();
   m_pbw->flush();
}

void default_buffered_writer::flush_all_buffers() {
   ABC_TRACE_FUNC(this);

   // If there was a pending I/O operation, get its result now.
   if (std::size_t cbWritten = m_pbw->async_join()) {
      buffer_write_complete(cbWritten);
   }
   // Flush any remaining buffers.
   while (!m_lbufWriteBufs.empty()) {
      detail::buffer * pbuf = &m_lbufWriteBufs.back();
      // *pbuf must have used_size() > 0, otherwise it wouldn’t exist.
      /* TODO: if *m_pbw expects writes of an integer multiple of its block size but the buffer is
      not 100% full, do something ‒ maybe truncate m_pbw afterwards if possible? */
      std::size_t cbWritten = m_pbw->write(pbuf->get_used(), pbuf->used_size());
      // TODO: decide whether pending I/O should return 0 or tuple<size_t, bool>.
      if (m_pbw->async_pending()) {
         cbWritten = m_pbw->async_join();
      }
      buffer_write_complete(cbWritten);
   }
}

std::size_t default_buffered_writer::flush_nonblocking_full_buffers() {
   ABC_TRACE_FUNC(this);

   if (m_pbw->async_pending()) {
      return 0;
   }
   std::size_t cbWrittenTotal = 0;
   while (!m_lbufWriteBufs.empty()) {
      detail::buffer * pbuf = &m_lbufWriteBufs.back();
      // *pbuf must have used_size() > 0, otherwise it wouldn’t exist.
      /* TODO: if *m_pbw expects writes of an integer multiple of its block size and there’s no
      following buffer than can be partially moved into *pbuf to make *pbuf full, stop here; this
      method doesn’t have to flush all buffers. */
      std::size_t cbWritten = m_pbw->write(pbuf->get_used(), pbuf->used_size());
      // TODO: decide whether pending I/O should return 0 or tuple<size_t, bool>.
      if (m_pbw->async_pending()) {
         break;
      }
      buffer_write_complete(cbWritten);
      cbWrittenTotal += cbWritten;
   }
   return cbWrittenTotal;
}

/*virtual*/ std::pair<void *, std::size_t> default_buffered_writer::get_buffer_bytes(
   std::size_t cb
) /*override*/ {
   ABC_TRACE_FUNC(this, cb);

   flush_nonblocking_full_buffers();

   // Use the front() buffer if it has enough space available, or add a new buffer.
   detail::buffer * pbuf;
   if (m_lbufWriteBufs.empty()) {
      pbuf = nullptr;
   } else {
      pbuf = &m_lbufWriteBufs.front();
      if (cb > pbuf->available_size()) {
         /* If the buffer is not being used for an asynchronous write, see if compacting it would
         create enough room. */
         // TODO: does async_pending() imply that *pbuf is in use? Could it be a different buffer?
         if (!m_pbw->async_pending() && pbuf->unused_size() + pbuf->available_size() >= cb) {
            pbuf->make_unused_available();
         } else {
            pbuf = nullptr;
         }
      }
   }
   if (!pbuf) {
      std::size_t cbWriteBuf = bitmanip::ceiling_to_pow2_multiple(cb, smc_cbWriteBufDefault);
      m_lbufWriteBufs.push_front(detail::buffer(cbWriteBuf));
      pbuf = &m_lbufWriteBufs.front();
   }
   // Return the available portion of the buffer.
   return std::make_pair(pbuf->get_available(), pbuf->available_size());
}

/*virtual*/ std::shared_ptr<base> default_buffered_writer::_unbuffered_base() const /*override*/ {
   ABC_TRACE_FUNC(this);

   return std::static_pointer_cast<base>(m_pbw);
}

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
