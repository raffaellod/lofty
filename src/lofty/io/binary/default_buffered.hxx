/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_IO_BINARY_DEFAULT_BUFFERED_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_IO_BINARY_DEFAULT_BUFFERED_HXX
#endif

#ifndef _LOFTY_IO_BINARY_DEFAULT_BUFFERED_HXX_NOPUB
#define _LOFTY_IO_BINARY_DEFAULT_BUFFERED_HXX_NOPUB

#include <lofty/io/binary.hxx>
#include <lofty/io/binary/buffer.hxx>
#include <lofty/noncopyable.hxx>
#include <lofty/_std/memory.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary { namespace _pub {

//! Provides buffering on top of a binary::istream instance.
class LOFTY_SYM default_buffered_istream : public buffered_istream, public lofty::_LOFTY_PUBNS noncopyable {
public:
   /*! Constructor.

   @param bin_istream
      Pointer to a buffered istream to wrap.
   */
   default_buffered_istream(_std::_LOFTY_PUBNS shared_ptr<istream> bin_istream);

   //! Destructor.
   virtual ~default_buffered_istream();

   //! See buffered_istream::consume_bytes().
   virtual void consume_bytes(std::size_t count) override;

   //! See buffered_istream::peek_bytes().
   virtual buffer_range<void const> peek_bytes(std::size_t count) override;

protected:
   //! See buffered_istream::_unbuffered_stream().
   virtual _std::_LOFTY_PUBNS shared_ptr<stream> _unbuffered_stream() const override;

protected:
   //! Wrapped binary istream.
   _std::_LOFTY_PUBNS shared_ptr<istream> bin_istream;
   //! Main read buffer.
   buffer read_buf;
   //! Default/increment size of read_buf.
   // TODO: tune this value.
   static std::size_t const read_buf_default_size = 0x1000;
};

}}}} //namespace lofty::io::binary::_pub

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary { namespace _pub {

//! Provides buffering on top of a binary::ostream instance.
class LOFTY_SYM default_buffered_ostream : public buffered_ostream, public lofty::_LOFTY_PUBNS noncopyable {
public:
   /*! Constructor.

   @param bin_ostream
      Pointer to a buffered output stream to wrap.
   */
   default_buffered_ostream(_std::_LOFTY_PUBNS shared_ptr<ostream> bin_ostream);

   //! Destructor.
   virtual ~default_buffered_ostream();

   //! See buffered_ostream::close().
   virtual void close() override;

   //! See buffered_ostream::commit_bytes().
   virtual void commit_bytes(std::size_t count) override;

   //! See buffered_ostream::flush().
   virtual void flush() override;

   //! See buffered_ostream::get_buffer_bytes().
   virtual buffer_range<void> get_buffer_bytes(std::size_t count) override;

protected:
   //! Flushes the internal write buffer.
   void flush_buffer();

   //! See buffered_ostream::_unbuffered_stream().
   virtual _std::_LOFTY_PUBNS shared_ptr<stream> _unbuffered_stream() const override;

protected:
   //! Wrapped binary ostream.
   _std::_LOFTY_PUBNS shared_ptr<ostream> bin_ostream;
   //! Write buffer.
   buffer write_buf;
   //! If true, every commit_bytes() call will flush the buffer.
   bool flush_after_commit:1;
   //! Default/increment size of write_buf.
   // TODO: tune this value.
   static std::size_t const write_buf_default_size = 0x1000;
};

}}}} //namespace lofty::io::binary::_pub

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_IO_BINARY_DEFAULT_BUFFERED_HXX_NOPUB

#ifdef _LOFTY_IO_BINARY_DEFAULT_BUFFERED_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace io { namespace binary {

   using _pub::default_buffered_istream;
   using _pub::default_buffered_ostream;

   }}}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_IO_BINARY_DEFAULT_BUFFERED_HXX
