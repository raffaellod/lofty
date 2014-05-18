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

#ifndef _ABC_IO_BUFFERED_BINARY_HXX
#define _ABC_IO_BUFFERED_BINARY_HXX

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abc/io/binary.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary globals

namespace abc {

namespace io {

namespace binary {

// Forward declarations.
class buffered_base;
class buffered_reader;
class buffered_writer;


/** Creates and returns a buffered wrapper for the specified binary I/O object.

pbb
   Pointer to a binary I/O object.
return
   Pointer to a buffered wrapper for *pbb.
*/
std::shared_ptr<buffered_base> buffer(std::shared_ptr<base> pbb);


/** Creates and returns a buffered reader wrapper for the specified unbuffered binary reader.

pbr
   Pointer to an unbuffered binary reader.
return
   Pointer to a buffered wrapper for *pbr.
*/
inline std::shared_ptr<buffered_reader> buffer_reader(std::shared_ptr<reader> pbr) {
   return std::dynamic_pointer_cast<buffered_reader>(buffer(std::move(pbr)));
}


/** Creates and returns a buffered writer wrapper for the specified unbuffered binary writer.

pbw
   Pointer to an unbuffered binary writer.
return
   Pointer to a buffered wrapper for *pbw.
*/
inline std::shared_ptr<buffered_writer> buffer_writer(std::shared_ptr<writer> pbw) {
   return std::dynamic_pointer_cast<buffered_writer>(buffer(std::move(pbw)));
}

} //namespace binary

} //namespace io

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::buffered_base


namespace abc {

namespace io {

namespace binary {

/** Interface for buffering objects that wrap binary::* instances.
*/
class ABCAPI buffered_base :
   public virtual base {
public:

   /** Returns a pointer to the wrapper unbuffered binary I/O object.
   */
   virtual std::shared_ptr<base> unbuffered() const = 0;
};

} //namespace binary

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::buffered_reader


namespace abc {

namespace io {

namespace binary {

/** Interface for buffering objects that wrap binary::reader instances.
*/
class ABCAPI buffered_reader :
   public virtual buffered_base,
   public reader {
public:

   /** Marks the specified amount of bytes as read, so that they won’t be presented again on the
   next peek() call.

   cb
      Count of bytes to mark as read.
   */
   virtual void consume(size_t cb) = 0;


   /** Returns a view of the internal read buffer, performing at most one read from the underlying
   binary reader.

   TODO: change to return a read-only, non-shareable ivector<T>.

   cb
      If greater than the size of the read buffer’s contents, an additional read from the
      underlying binary reader will be made, adding to the contents of the read buffer; if the
      internal buffer is not large enough to hold the cumulative data, it will be enlarged.
   return
      Pair containing:
      •  A pointer to the portion of the internal buffer that holds the read data;
      •  Count of bytes read. May be less than the cb argument if EOF is reached, or greater than cb
         if the buffer was filled more than requested. For non-zero values of cb, a return value of
         0 indicates that no more data is available (EOF).
   */
   template <typename T>
   std::pair<T const *, size_t> peek(size_t cb = 1) {
      auto ret(_peek_void(cb));
      // Repack the tuple, changing pointer type.
      return std::make_pair(static_cast<T const *>(ret.first), ret.second);
   }


   /** See binary::reader::read(). Using peek()/consume() is preferred to calling this method,
   because it will spare the caller from allocating an intermediate buffer.
   */
   virtual size_t read(void * p, size_t cbMax);


protected:

   /** Non-template implementation of peek(). See peek().
   */
   virtual std::pair<void const *, size_t> _peek_void(size_t cb) = 0;
};

} //namespace binary

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::buffered_writer


namespace abc {

namespace io {

namespace binary {

/** Interface for buffering objects that wrap binary::writer instances.
*/
class ABCAPI buffered_writer :
   public virtual buffered_base,
   public writer {
};

} //namespace binary

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::default_buffered_reader


namespace abc {

namespace io {

namespace binary {

/** Provides buffering on top of a binary::reader instance.
*/
class ABCAPI default_buffered_reader :
   public buffered_reader {
public:

   /** Constructor.

   pbr
      Pointer to a buffered reader to wrap.
   */
   default_buffered_reader(std::shared_ptr<reader> pbr);


   /** Destructor.
   */
   virtual ~default_buffered_reader();


   /** See buffered_reader::consume().
   */
   virtual void consume(size_t cb);


   /** See buffered_reader::unbuffered().
   */
   virtual std::shared_ptr<base> unbuffered() const;


protected:

   /** See buffered_reader::_peek_void().
   */
   virtual std::pair<void const *, size_t> _peek_void(size_t cb);


protected:

   /** Wrapped binary reader. */
   std::shared_ptr<reader> m_pbr;
   /** Read buffer. */
   std::unique_ptr<int8_t[], memory::freeing_deleter<int8_t[]>> m_pbReadBuf;
   /** Size of m_pbReadBuf. */
   size_t m_cbReadBuf;
   /** Offset of the first used byte in m_pbReadBuf. */
   size_t m_ibReadBufUsed;
   /** Number of bytes used in m_pbReadBuf. */
   size_t m_cbReadBufUsed;
   /** Default/increment size of m_pbReadBuf. */
   // TODO: tune this value.
   static size_t const smc_cbReadBufDefault = 0x1000;
};

} //namespace binary

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::default_buffered_writer


namespace abc {

namespace io {

namespace binary {

/** Provides buffering on top of a binary::writer instance.
*/
class ABCAPI default_buffered_writer :
   public buffered_writer {
public:

   /** Constructor.

   pbw
      Pointer to a buffered writer to wrap.
   */
   default_buffered_writer(std::shared_ptr<writer> pbw);


   /** Destructor.
   */
   virtual ~default_buffered_writer();


   /** See buffered_writer::flush().
   */
   virtual void flush();


   /** See buffered_writer::unbuffered().
   */
   virtual std::shared_ptr<base> unbuffered() const;


   /** See buffered_writer::write().
   */
   virtual size_t write(void const * p, size_t cb);


protected:

   /** Flushes the internal write buffer.
   */
   void flush_buffer();


protected:

   /** Wrapped binary writer. */
   std::shared_ptr<writer> m_pbw;
   /** Write buffer. */
   std::unique_ptr<int8_t[], memory::freeing_deleter<int8_t[]>> m_pbWriteBuf;
   /** Size of m_pbWriteBuf. */
   size_t m_cbWriteBuf;
   /** Number of bytes used in m_pbWriteBuf. */
   size_t m_cbWriteBufUsed;
   /** Default/increment size of m_pbWriteBuf. */
   // TODO: tune this value.
   static size_t const smc_cbWriteBufDefault = 0x1000;
};

} //namespace binary

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef _ABC_IO_BUFFERED_BINARY_HXX

