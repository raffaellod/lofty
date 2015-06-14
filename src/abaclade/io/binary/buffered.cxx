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
   m_p(memory::alloc<void>(cb)),
   m_cb(cb),
   m_ibUsedOffset(0),
   m_ibAvailableOffset(0) {
}
buffer::buffer(buffer && buf) :
   m_p(std::move(buf.m_p)),
   m_cb(buf.m_cb),
   m_ibUsedOffset(buf.m_ibUsedOffset),
   m_ibAvailableOffset(buf.m_ibAvailableOffset) {
   buf.m_cb = 0;
   buf.m_ibUsedOffset = 0;
   buf.m_ibAvailableOffset = 0;
}

buffer::~buffer() {
}

buffer & buffer::operator=(buffer && buf) {
   ABC_TRACE_FUNC(this/*, buf*/);

   m_p = std::move(buf.m_p);
   m_cb = buf.m_cb;
   buf.m_cb = 0;
   m_ibUsedOffset = buf.m_ibUsedOffset;
   buf.m_ibUsedOffset = 0;
   m_ibAvailableOffset = buf.m_ibAvailableOffset;
   buf.m_ibAvailableOffset = 0;
   return *this;
}

void buffer::expand(std::size_t cb) {
   ABC_TRACE_FUNC(this, cb);

   memory::realloc(&m_p, cb);
   m_cb = cb;
}

void buffer::make_unused_available() {
   ABC_TRACE_FUNC(this);

   memory::move(static_cast<std::int8_t *>(m_p.get()), get_used(), used_size());
   m_ibAvailableOffset -= m_ibUsedOffset;
   m_ibUsedOffset = 0;
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
}

/*virtual*/ void default_buffered_reader::consume_bytes(std::size_t cb) /*override*/ {
   ABC_TRACE_FUNC(this, cb);

   if (cb > m_bufRead.used_size()) {
      // Can’t consume more bytes than are available in the read buffer.
      // TODO: use a better exception class.
      ABC_THROW(argument_error, ());
   }
   // Shift the “used window” of the read buffer by cb bytes.
   m_bufRead.mark_as_unused(cb);
}

/*virtual*/ std::pair<void const *, std::size_t> default_buffered_reader::peek_bytes(
   std::size_t cb
) /*override*/ {
   ABC_TRACE_FUNC(this, cb);

   if (cb > m_bufRead.used_size()) {
      // The caller wants more data than what’s currently in the buffer: try to load more.
      std::size_t cbReadMin = cb - m_bufRead.used_size();
      if (cbReadMin > m_bufRead.available_size()) {
         /* The buffer doesn’t have enough available space to hold the data that needs to be read;
         see if compacting it would create enough room. */
         if (m_bufRead.unused_size() + m_bufRead.available_size() >= cbReadMin) {
            m_bufRead.make_unused_available();
         } else {
            // Not enough room; the buffer needs to be enlarged.
            std::size_t cbReadBuf = bitmanip::ceiling_to_pow2_multiple(cb, smc_cbReadBufDefault);
            m_bufRead.expand(cbReadBuf);
         }
      }
      // Try to fill the available part of the buffer.
      std::size_t cbRead = m_pbr->read(m_bufRead.get_available(), m_bufRead.available_size());
      // Account for the additional data read.
      m_bufRead.mark_as_used(cbRead);
   }
   // Return the “used window” of the buffer.
   return std::make_pair(m_bufRead.get_used(), m_bufRead.used_size());
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
   m_pbw(std::move(pbw)),
   // Disable buffering for console (interactive) files.
   m_bFlushAfterCommit(std::dynamic_pointer_cast<console_writer>(m_pbw) != nullptr) {
}

/*virtual*/ default_buffered_writer::~default_buffered_writer() {
   /* Verify that the write buffer is empty. If that’s not the case, the caller neglected to verify
   that m_bufWrite and the OS write buffer were flushed successfully. */
   if (m_bufWrite.used_size()) {
      // This will cause a call to std::terminate().
      ABC_THROW(destructing_unfinalized_object, (this));
   }
}

/*virtual*/ void default_buffered_writer::commit_bytes(std::size_t cb) /*override*/ {
   ABC_TRACE_FUNC(this, cb);

   if (cb > m_bufWrite.available_size()) {
      // Can’t commit more bytes than are available in the write buffer.
      // TODO: use a better exception class.
      ABC_THROW(argument_error, ());
   }
   // Increase the count of used bytes in the buffer; if that makes the buffer full, flush it.
   m_bufWrite.mark_as_used(cb);
   if (m_bFlushAfterCommit || !m_bufWrite.available_size()) {
      flush_buffer();
   }
}

/*virtual*/ void default_buffered_writer::finalize() /*override*/ {
   ABC_TRACE_FUNC(this);

   // Flush both the write buffer and any lower-level buffers.
   try {
      flush_buffer();
   } catch (...) {
      // Consider the buffer contents as lost.
      m_bufWrite.mark_as_unused(m_bufWrite.used_size());
      /* If this throws a nested exception, std::terminate() will be called; otherwise, we’ll have
      successfully prevented m_pbw’s destructor from throwing due to a missed finalize(). */
      m_pbw->finalize();
      throw;
   }
   m_pbw->finalize();
}

/*virtual*/ void default_buffered_writer::flush() /*override*/ {
   ABC_TRACE_FUNC(this);

   // Flush both the write buffer and any lower-level buffers.
   flush_buffer();
   m_pbw->flush();
}

void default_buffered_writer::flush_buffer() {
   ABC_TRACE_FUNC(this);

   if (std::size_t cbBufUsed = m_bufWrite.used_size()) {
      /* TODO: if *m_pbw expects writes of an integer multiple of its block size but the buffer is
      not 100% full, do something – maybe truncate m_pbw afterwards if possible? */
      std::size_t cbWritten = m_pbw->write(m_bufWrite.get_used(), cbBufUsed);
      ABC_ASSERT(cbWritten == cbBufUsed, ABC_SL("the entire buffer must have been written"));
      m_bufWrite.mark_as_unused(cbWritten);
   }
}

/*virtual*/ std::pair<void *, std::size_t> default_buffered_writer::get_buffer_bytes(
   std::size_t cb
) /*override*/ {
   ABC_TRACE_FUNC(this, cb);

   /* If the requested size is more than what can fit in the buffer, compact it, flush it, or
   enlarge it. */
   if (cb > m_bufWrite.available_size()) {
      // See if compacting the buffer would create enough room.
      if (m_bufWrite.unused_size() + m_bufWrite.available_size() >= cb) {
         m_bufWrite.make_unused_available();
      } else {
         // If the buffer is still too small, enlarge it.
         flush_buffer();
         m_bufWrite.make_unused_available();
         if (cb > m_bufWrite.available_size()) {
            std::size_t cbWriteBuf = bitmanip::ceiling_to_pow2_multiple(cb, smc_cbWriteBufDefault);
            m_bufWrite.expand(cbWriteBuf);
         }
      }
   }
   // Return the available portion of the buffer.
   return std::make_pair(m_bufWrite.get_available(), m_bufWrite.available_size());
}

/*virtual*/ std::shared_ptr<base> default_buffered_writer::_unbuffered_base() const /*override*/ {
   ABC_TRACE_FUNC(this);

   return std::static_pointer_cast<base>(m_pbw);
}

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
