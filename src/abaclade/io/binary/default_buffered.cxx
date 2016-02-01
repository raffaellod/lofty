/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2016 Raffaello D. Di Napoli

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
#include <abaclade/bitmanip.hxx>
#include <abaclade/destructing_unfinalized_object.hxx>
#include <abaclade/io/binary.hxx>
#include "default_buffered.hxx"
#include "file-subclasses.hxx"


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary { namespace _pvt {

buffer::buffer(std::size_t cb) :
   m_p(memory::alloc_bytes_unique(cb)),
   m_cb(cb),
   m_ibUsedOffset(0),
   m_ibAvailableOffset(0) {
}
buffer::buffer(buffer && buf) :
   m_p(_std::move(buf.m_p)),
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

   m_p = _std::move(buf.m_p);
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

   memory::realloc_unique(&m_p, cb);
   m_cb = cb;
}

void buffer::make_unused_available() {
   ABC_TRACE_FUNC(this);

   memory::move(static_cast<std::int8_t *>(m_p.get()), get_used(), used_size());
   m_ibAvailableOffset -= m_ibUsedOffset;
   m_ibUsedOffset = 0;
}

}}}} //namespace abc::io::binary::_pvt

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

default_buffered_istream::default_buffered_istream(_std::shared_ptr<istream> pbis) :
   m_pbis(_std::move(pbis)) {
}

/*virtual*/ default_buffered_istream::~default_buffered_istream() {
}

/*virtual*/ void default_buffered_istream::consume_bytes(std::size_t cb) /*override*/ {
   ABC_TRACE_FUNC(this, cb);

   if (cb > m_bufRead.used_size()) {
      // Can’t consume more bytes than are available in the read buffer.
      // TODO: use a better exception class.
      ABC_THROW(argument_error, ());
   }
   // Shift the “used window” of the read buffer by cb bytes.
   m_bufRead.mark_as_unused(cb);
}

/*virtual*/ _std::tuple<void const *, std::size_t> default_buffered_istream::peek_bytes(
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
      std::size_t cbRead = m_pbis->read(m_bufRead.get_available(), m_bufRead.available_size());
      // Account for the additional data read.
      m_bufRead.mark_as_used(cbRead);
   }
   // Return the “used window” of the buffer.
   return _std::make_tuple(m_bufRead.get_used(), m_bufRead.used_size());
}

/*virtual*/ _std::shared_ptr<stream> default_buffered_istream::_unbuffered_stream(
) const /*override*/ {
   ABC_TRACE_FUNC(this);

   return _std::static_pointer_cast<stream>(m_pbis);
}

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

default_buffered_ostream::default_buffered_ostream(_std::shared_ptr<ostream> pbos) :
   m_pbos(_std::move(pbos)),
   // Disable buffering for console (interactive) files.
   m_bFlushAfterCommit(_std::dynamic_pointer_cast<tty_ostream>(m_pbos) != nullptr) {
}

/*virtual*/ default_buffered_ostream::~default_buffered_ostream() {
   /* Verify that the write buffer is empty. If that’s not the case, the caller neglected to verify
   that m_bufWrite and the OS write buffer were flushed successfully. */
   if (m_bufWrite.used_size()) {
      // This will cause a call to std::terminate().
      ABC_THROW(destructing_unfinalized_object, (this));
   }
}

/*virtual*/ void default_buffered_ostream::commit_bytes(std::size_t cb) /*override*/ {
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

/*virtual*/ void default_buffered_ostream::finalize() /*override*/ {
   ABC_TRACE_FUNC(this);

   // Flush both the write buffer and any lower-level buffers.
   try {
      flush_buffer();
   } catch (...) {
      // Consider the buffer contents as lost.
      m_bufWrite.mark_as_unused(m_bufWrite.used_size());
      /* If this throws a nested exception, std::terminate() will be called; otherwise, we’ll have
      successfully prevented m_pbos’s destructor from throwing due to a missed finalize(). */
      m_pbos->finalize();
      throw;
   }
   m_pbos->finalize();
}

/*virtual*/ void default_buffered_ostream::flush() /*override*/ {
   ABC_TRACE_FUNC(this);

   // Flush both the write buffer and any lower-level buffers.
   flush_buffer();
   m_pbos->flush();
}

void default_buffered_ostream::flush_buffer() {
   ABC_TRACE_FUNC(this);

   if (std::size_t cbBufUsed = m_bufWrite.used_size()) {
      /* TODO: if *m_pbos expects writes of an integer multiple of its block size but the buffer is
      not 100% full, do something – maybe truncate m_pbos afterwards if possible? */
      std::size_t cbWritten = m_pbos->write(m_bufWrite.get_used(), cbBufUsed);
      ABC_ASSERT(cbWritten == cbBufUsed, ABC_SL("the entire buffer must have been written"));
      m_bufWrite.mark_as_unused(cbWritten);
   }
}

/*virtual*/ _std::tuple<void *, std::size_t> default_buffered_ostream::get_buffer_bytes(
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
   return _std::make_tuple(m_bufWrite.get_available(), m_bufWrite.available_size());
}

/*virtual*/ _std::shared_ptr<stream> default_buffered_ostream::_unbuffered_stream(
) const /*override*/ {
   ABC_TRACE_FUNC(this);

   return _std::static_pointer_cast<stream>(m_pbos);
}

}}} //namespace abc::io::binary
