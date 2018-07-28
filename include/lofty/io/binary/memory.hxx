/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_IO_BINARY_MEMORY_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_IO_BINARY_MEMORY_HXX
#endif

#ifndef _LOFTY_IO_BINARY_MEMORY_HXX_NOPUB
#define _LOFTY_IO_BINARY_MEMORY_HXX_NOPUB

#include <lofty/io/binary.hxx>
#include <lofty/io/binary/buffer.hxx>
#include <lofty/noncopyable.hxx>
#include <lofty/_std/memory.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

//! Implementation of an input/output stream backed by an in-memory buffer.
class LOFTY_SYM memory_stream :
   public virtual buffered_istream,
   public virtual buffered_ostream,
   public seekable,
   public sized,
   public _std::_LOFTY_PUBNS enable_shared_from_this<memory_stream>,
   public lofty::_LOFTY_PUBNS noncopyable {
public:
   //! Default constructor.
   memory_stream();

   /*! Move constructor.

   @param src
      Source object.
   */
   memory_stream(memory_stream && src);

   /*! Constructs a stream for a given buffer, taking ownership of it.

   @param buf
      Buffer to take control of.
   */
   explicit memory_stream(buffer && buf);

   //! Destructor.
   virtual ~memory_stream();

   //! See buffered_ostream::commit_bytes().
   virtual void commit_bytes(std::size_t count) override;

   //! See buffered_istream::consume_bytes().
   virtual void consume_bytes(std::size_t count) override;

   //! See buffered_ostream::get_buffer_bytes().
   virtual buffer_range<void> get_buffer_bytes(std::size_t count) override;

   //! See buffered_istream::peek_bytes().
   virtual buffer_range<void const> peek_bytes(std::size_t count) override;

   //! Makes the next read operation start from the first byte.
   void rewind();

   //! See seekable::seek().
   virtual io::_LOFTY_PUBNS offset_t seek(
      io::_LOFTY_PUBNS offset_t offset, io::_LOFTY_PUBNS seek_from whence
   ) override;

   //! See sized::size().
   virtual io::_LOFTY_PUBNS full_size_t size() const override;

   //! See seekable::tell().
   virtual io::_LOFTY_PUBNS offset_t tell() const override;

protected:
   //! Not used in this implementation; see buffered_ostream::close().
   virtual void close() override;

   //! Not used in this implementation; see buffered_ostream::flush().
   virtual void flush() override;

   //! See buffered_istream::_unbuffered_stream().
   virtual _std::_LOFTY_PUBNS shared_ptr<stream> _unbuffered_stream() const override;

protected:
   //! Main buffer.
   buffer buf;
   //! Default/increment size of buf.
   // TODO: tune this value.
   static std::size_t const buf_default_size = 0x400;
};

_LOFTY_PUBNS_END
}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_IO_BINARY_MEMORY_HXX_NOPUB

#ifdef _LOFTY_IO_BINARY_MEMORY_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace io { namespace binary {

   using _pub::memory_stream;

   }}}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_IO_BINARY_MEMORY_HXX
