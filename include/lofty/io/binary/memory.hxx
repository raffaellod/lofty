/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_IO_BINARY_MEMORY_HXX
#define _LOFTY_IO_BINARY_MEMORY_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/io/binary.hxx>
#include <lofty/io/binary/buffer.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

//! Implementation of an input/output stream backed by an in-memory buffer.
class LOFTY_SYM memory_stream :
   public virtual buffered_istream,
   public virtual buffered_ostream,
   public _std::enable_shared_from_this<memory_stream>,
   public noncopyable {
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
   virtual _std::tuple<void *, std::size_t> get_buffer_bytes(std::size_t count) override;

   //! See buffered_istream::peek_bytes().
   virtual _std::tuple<void const *, std::size_t> peek_bytes(std::size_t count) override;

protected:
   //! Not used in this implementation; see buffered_ostream::finalize().
   virtual void finalize() override;

   //! Not used in this implementation; see buffered_ostream::flush().
   virtual void flush() override;

   //! See buffered_istream::_unbuffered_stream().
   virtual _std::shared_ptr<stream> _unbuffered_stream() const override;

protected:
   //! Main buffer.
   buffer buf;
   //! Default/increment size of buf.
   // TODO: tune this value.
   static std::size_t const buf_default_size = 0x400;
};

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_IO_BINARY_MEMORY_HXX
