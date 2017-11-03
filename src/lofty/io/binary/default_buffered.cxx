/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/bitmanip.hxx>
#include <lofty/destructing_unfinalized_object.hxx>
#include <lofty/io/binary.hxx>
#include "default_buffered.hxx"
#include "file-subclasses.hxx"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary { namespace _pvt {

buffer::buffer(std::size_t size__) :
   ptr(memory::alloc_bytes_unique(size__)),
   size_(size__),
   used_offset(0),
   available_offset(0) {
}
buffer::buffer(buffer && src) :
   ptr(_std::move(src.ptr)),
   size_(src.size_),
   used_offset(src.used_offset),
   available_offset(src.available_offset) {
   src.size_ = 0;
   src.used_offset = 0;
   src.available_offset = 0;
}

buffer::~buffer() {
}

buffer & buffer::operator=(buffer && src) {
   ptr = _std::move(src.ptr);
   size_ = src.size_;
   src.size_ = 0;
   used_offset = src.used_offset;
   src.used_offset = 0;
   available_offset = src.available_offset;
   src.available_offset = 0;
   return *this;
}

void buffer::expand_to(std::size_t new_size) {
   memory::realloc_unique(&ptr, new_size);
   size_ = new_size;
}

void buffer::make_unused_available() {
   memory::move(static_cast<std::int8_t *>(ptr.get()), get_used(), used_size());
   available_offset -= used_offset;
   used_offset = 0;
}

}}}} //namespace lofty::io::binary::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

default_buffered_istream::default_buffered_istream(_std::shared_ptr<istream> bin_istream_) :
   bin_istream(_std::move(bin_istream_)) {
}

/*virtual*/ default_buffered_istream::~default_buffered_istream() {
}

/*virtual*/ void default_buffered_istream::consume_bytes(std::size_t count) /*override*/ {
   if (count > read_buf.used_size()) {
      // Can’t consume more bytes than are available in the read buffer.
      // TODO: use a better exception class.
      LOFTY_THROW(argument_error, ());
   }
   // Shift the “used window” of the read buffer by count bytes.
   read_buf.mark_as_unused(count);
}

/*virtual*/ _std::tuple<void const *, std::size_t> default_buffered_istream::peek_bytes(
   std::size_t count
) /*override*/ {
   while (count > read_buf.used_size()) {
      // The caller wants more data than what’s currently in the buffer: try to load more.
      std::size_t read_byte_size_min = count - read_buf.used_size();
      if (read_byte_size_min > read_buf.available_size()) {
         /* The buffer doesn’t have enough available space to hold the data that needs to be read; see if
         compacting it would create enough room. */
         if (read_buf.unused_size() + read_buf.available_size() >= read_byte_size_min) {
            read_buf.make_unused_available();
         } else {
            // Not enough room; the buffer needs to be enlarged.
            std::size_t read_buf_size = bitmanip::ceiling_to_pow2_multiple(count, read_buf_default_size);
            read_buf.expand_to(read_buf_size);
         }
      }
      // Try to fill the available part of the buffer.
      std::size_t read_bytes = bin_istream->read(read_buf.get_available(), read_buf.available_size());
      if (read_bytes == 0) {
         // No more data available (EOF).
         break;
      }
      // Account for the additional data read.
      read_buf.mark_as_used(read_bytes);
   }
   // Return the “used window” of the buffer.
   return _std::make_tuple(read_buf.get_used(), read_buf.used_size());
}

/*virtual*/ _std::shared_ptr<stream> default_buffered_istream::_unbuffered_stream() const /*override*/ {
   return _std::static_pointer_cast<stream>(bin_istream);
}

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

default_buffered_ostream::default_buffered_ostream(_std::shared_ptr<ostream> bin_ostream_) :
   bin_ostream(_std::move(bin_ostream_)),
   // Disable buffering for console (interactive) files.
   flush_after_commit(_std::dynamic_pointer_cast<tty_ostream>(bin_ostream) != nullptr) {
}

/*virtual*/ default_buffered_ostream::~default_buffered_ostream() {
   /* Verify that the write buffer is empty. If that’s not the case, the caller neglected to verify that
   write_buf and the OS write buffer were flushed successfully. */
   if (write_buf.used_size()) {
      // This will cause a call to std::terminate().
      LOFTY_THROW(destructing_unfinalized_object, (this));
   }
}

/*virtual*/ void default_buffered_ostream::commit_bytes(std::size_t count) /*override*/ {
   if (count > write_buf.available_size()) {
      // Can’t commit more bytes than are available in the write buffer.
      // TODO: use a better exception class.
      LOFTY_THROW(argument_error, ());
   }
   // Increase the count of used bytes in the buffer; if that makes the buffer full, flush it.
   write_buf.mark_as_used(count);
   if (flush_after_commit || !write_buf.available_size()) {
      flush_buffer();
   }
}

/*virtual*/ void default_buffered_ostream::finalize() /*override*/ {
   // Flush both the write buffer and any lower-level buffers.
   try {
      flush_buffer();
   } catch (...) {
      // Consider the buffer contents as lost.
      write_buf.mark_as_unused(write_buf.used_size());
      /* If this throws a nested exception, std::terminate() will be called; otherwise, we’ll have
      successfully prevented bin_ostream’s destructor from throwing due to a missed finalize(). */
      bin_ostream->finalize();
      throw;
   }
   bin_ostream->finalize();
}

/*virtual*/ void default_buffered_ostream::flush() /*override*/ {
   // Flush both the write buffer and any lower-level buffers.
   flush_buffer();
   bin_ostream->flush();
}

void default_buffered_ostream::flush_buffer() {
   if (std::size_t buf_used_size = write_buf.used_size()) {
      /* TODO: if *bin_ostream expects writes of an integer multiple of its block size but the buffer is not
      100% full, do something – maybe truncate bin_ostream afterwards if possible? */
      std::size_t written_size = bin_ostream->write(write_buf.get_used(), buf_used_size);
      LOFTY_ASSERT(written_size == buf_used_size, LOFTY_SL("the entire buffer must have been written"));
      write_buf.mark_as_unused(written_size);
   }
}

/*virtual*/ _std::tuple<void *, std::size_t> default_buffered_ostream::get_buffer_bytes(
   std::size_t count
) /*override*/ {
   // If the requested size is more than what can fit in the buffer, compact it, flush it, or enlarge it.
   if (count > write_buf.available_size()) {
      // See if compacting the buffer would create enough room.
      if (write_buf.unused_size() + write_buf.available_size() >= count) {
         write_buf.make_unused_available();
      } else {
         // If the buffer is still too small, enlarge it.
         flush_buffer();
         write_buf.make_unused_available();
         if (count > write_buf.available_size()) {
            std::size_t write_buf_size = bitmanip::ceiling_to_pow2_multiple(count, write_buf_default_size);
            write_buf.expand_to(write_buf_size);
         }
      }
   }
   // Return the available portion of the buffer.
   return _std::make_tuple(write_buf.get_available(), write_buf.available_size());
}

/*virtual*/ _std::shared_ptr<stream> default_buffered_ostream::_unbuffered_stream() const /*override*/ {
   return _std::static_pointer_cast<stream>(bin_ostream);
}

}}} //namespace lofty::io::binary
