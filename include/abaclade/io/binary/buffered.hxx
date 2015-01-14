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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary globals

namespace abc {
namespace io {
namespace binary {

// Forward declarations.
class buffered_base;
class buffered_reader;
class buffered_writer;

/*! Creates and returns a buffered wrapper for the specified binary I/O object.

@param pbb
   Pointer to a binary I/O object.
@return
   Pointer to a buffered wrapper for *pbb.
*/
ABACLADE_SYM std::shared_ptr<buffered_base> buffer(std::shared_ptr<base> pbb);

/*! Creates and returns a buffered reader wrapper for the specified unbuffered binary reader.

@param pbr
   Pointer to an unbuffered binary reader.
@return
   Pointer to a buffered wrapper for *pbr.
*/
inline std::shared_ptr<buffered_reader> buffer_reader(std::shared_ptr<reader> pbr) {
   return std::dynamic_pointer_cast<buffered_reader>(buffer(std::move(pbr)));
}

/*! Creates and returns a buffered writer wrapper for the specified unbuffered binary writer.

@param pbw
   Pointer to an unbuffered binary writer.
@return
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

//! Interface for buffering objects that wrap binary::* instances.
class ABACLADE_SYM buffered_base : public virtual base {
public:
   /*! Returns a pointer to the wrapped unbuffered binary I/O object.

   @return
      Pointer to a unbuffered binary I/O object.
   */
   std::shared_ptr<base> unbuffered() const {
      return _unbuffered_base();
   }

protected:
   /*! Implementation of unbuffered(). This enables unbuffered() to be non-virtual, which in turn
   allows derived classes to override it changing its return type to be more specific.

   @return
      Pointer to a unbuffered binary I/O object.
   */
   virtual std::shared_ptr<base> _unbuffered_base() const = 0;
};

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::buffered_reader

namespace abc {
namespace io {
namespace binary {

//! Interface for buffering objects that wrap binary::reader instances.
class ABACLADE_SYM buffered_reader : public virtual buffered_base, public reader {
public:
   /*! Marks the specified amount of bytes as read, so that they won’t be presented again on the
   next peek() call.

   @param c
      Count of elements to mark as read.
   */
   template <typename T>
   void consume(std::size_t c) {
      return consume_bytes(sizeof(T) * c);
   }

   /*! Non-template implementation of consume(). See consume().

   @param cb
      Count of bytes to mark as read.
   */
   virtual void consume_bytes(std::size_t cb) = 0;

   /*! Returns a view of the internal read buffer, performing at most one read from the underlying
   binary reader.

   TODO: change to return a read-only, non-shareable ivector<T>.

   @param c
      Count of items to peek. If greater than the size of the read buffer’s contents, an additional
      read from the underlying binary reader will be made, adding to the contents of the read
      buffer; if the internal buffer is not large enough to hold the cumulative data, it will be
      enlarged.
   @return
      Pair containing:
      •  A pointer to the portion of the internal buffer that holds the read data;
      •  Count of bytes read. May be less than the cb argument if EOF is reached, or greater than cb
         if the buffer was filled more than requested. For non-zero values of cb, a return value of
         0 indicates that no more data is available (EOF).
   */
   template <typename T>
   std::pair<T const *, std::size_t> peek(std::size_t c = 1) {
      auto ret(peek_bytes(sizeof(T) * c));
      // Repack the tuple, changing pointer type.
      return std::make_pair(static_cast<T const *>(ret.first), ret.second);
   }

   /*! Non-template implementation of peek(). See peek().

   @param cb
      Count of bytes to peek.
   */
   virtual std::pair<void const *, std::size_t> peek_bytes(std::size_t cb) = 0;

   /*! See binary::reader::read(). Using peek()/consume() or peek_bytes()/consume_bytes() is
   preferred to calling this method, because it will spare the caller from having to allocate an
   intermediate buffer. */
   virtual std::size_t read(void * p, std::size_t cbMax) override;

   //! See buffered_base::unbuffered().
   std::shared_ptr<reader> unbuffered() const {
      return std::dynamic_pointer_cast<reader>(_unbuffered_base());
   }
};

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::buffered_writer

namespace abc {
namespace io {
namespace binary {

//! Interface for buffering objects that wrap binary::writer instances.
class ABACLADE_SYM buffered_writer : public virtual buffered_base, public writer {
public:
   /*! Commits (writes) any pending buffer blocks returned by get_buffer().

   @param c
      Count of elements to commit.
   */
   template <typename T>
   void commit(std::size_t c) {
      commit_bytes(sizeof(T) * c);
   }

   /*! Non-template, byte-oriented implementation of commit(). See commit().

   @param cb
      Count of bytes to commit.
   */
   virtual void commit_bytes(std::size_t cb) = 0;

   /*! Returns a buffer large enough to store up to c items.

   @param c
      Count of items to create buffer space for.
   @return
      Pair containing:
      •  A pointer to the portion of the internal buffer that the caller can write to;
      •  Size of the portion of internal buffer, in bytes.
   */
   template <typename T>
   std::pair<T *, std::size_t> get_buffer(std::size_t c) {
      auto ret(get_buffer_bytes(sizeof(T) * c));
      // Repack the tuple, changing pointer type.
      return std::make_pair(static_cast<T *>(ret.first), ret.second);
   }

   /*! Byte-oriented implementation of get_buffer(). See get_buffer().

   @param cb
      Count of bytes to create buffer space for.
   */
   virtual std::pair<void *, std::size_t> get_buffer_bytes(std::size_t cb) = 0;

   //! See buffered_base::unbuffered().
   std::shared_ptr<writer> unbuffered() const {
      return std::dynamic_pointer_cast<writer>(_unbuffered_base());
   }

   /*! See binary::writer::write(). Using get_buffer()/commit() or get_buffer_bytes()/commit_bytes()
   is preferred to calling this method, because it will spare the caller from having to allocate an
   intermediate buffer. */
   virtual std::size_t write(void const * p, std::size_t cb) override;
};

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::default_buffered_reader

namespace abc {
namespace io {
namespace binary {

//! Provides buffering on top of a binary::reader instance.
class ABACLADE_SYM default_buffered_reader : public buffered_reader, public noncopyable {
public:
   /*! Constructor.

   @param pbr
      Pointer to a buffered reader to wrap.
   */
   default_buffered_reader(std::shared_ptr<reader> pbr);

   //! Destructor.
   virtual ~default_buffered_reader();

   //! See buffered_reader::async_join().
   virtual std::size_t async_join() override;

   //! See buffered_reader::async_pending().
   virtual bool async_pending() const override;

   //! See buffered_reader::consume_bytes().
   virtual void consume_bytes(std::size_t cb) override;

   //! See buffered_reader::peek_bytes().
   virtual std::pair<void const *, std::size_t> peek_bytes(std::size_t cb) override;

protected:
   //! See buffered_reader::_unbuffered_base().
   virtual std::shared_ptr<base> _unbuffered_base() const override;

protected:
   //! Wrapped binary reader.
   std::shared_ptr<reader> m_pbr;
   //! Read buffer.
   std::unique_ptr<std::int8_t[], memory::freeing_deleter<std::int8_t[]>> m_pbReadBuf;
   //! Size of m_pbReadBuf.
   std::size_t m_cbReadBuf;
   //! Offset of the first used byte in m_pbReadBuf.
   std::size_t m_ibReadBufUsed;
   //! Number of bytes used in m_pbReadBuf.
   std::size_t m_cbReadBufUsed;
   //! Default/increment size of m_pbReadBuf.
   // TODO: tune this value.
   static std::size_t const smc_cbReadBufDefault = 0x1000;
};

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::default_buffered_writer

namespace abc {
namespace io {
namespace binary {

/*! Provides buffering on top of a binary::writer instance. If the underlying binary::writer is
synchronous, a single buffer will be used; if asynchronous, multiple buffers may be used if the rate
of writes to buffer exceeds the rate at which the binary::writer can complete buffer flushes. */
class ABACLADE_SYM default_buffered_writer : public buffered_writer, public noncopyable {
protected:
   //! Single buffer.
   class ABACLADE_SYM buffer : public noncopyable {
   public:
      /*! Constructor.

      @param cb
         Size of the buffer to allocate, in bytes.
      @param buf
         Source object.
      */
      buffer(std::size_t cb);
      buffer(buffer && buf) :
         m_p(buf.m_p),
         m_cb(buf.m_cb),
         m_cbUsed(buf.m_cbUsed) {
         buf.m_p = nullptr;
         buf.m_cb = 0;
         buf.m_cbUsed = 0;
      }

      //! Destructor.
      ~buffer();

      /*! Returns the amount of available buffer space.

      @return
         Size of available buffer space, in bytes.
      */
      std::size_t available_size() const {
         return m_cb - m_cbUsed;
      }

      /*! Returns a pointer to the buffer memory.

      @return
         Pointer to the buffer memory block.
      */
      std::int8_t * get() const {
         return static_cast<std::int8_t *>(m_p);
      }

      /*! Decreases the used bytes count, shifting the used part of the buffer towards the start.

      @param cb
         Bytes to count as used.
      */
      void mark_as_available(std::size_t cb);

      /*! Increases the used bytes count.

      @param cb
         Bytes to count as used.
      */
      void mark_as_used(std::size_t cb) {
         m_cbUsed += cb;
      }

      /*! Returns the size of the buffer.

      @return
         Size of the buffer space, in bytes.
      */
      std::size_t size() const {
         return m_cb;
      }

      /*! Returns the amount of used buffer space.

      @return
         Size of used buffer space, in bytes.
      */
      std::size_t used_size() const {
         return m_cbUsed;
      }

   private:
      //! Pointer to the allocated memory block.
      void * m_p;
      //! Size of *m_p.
      std::size_t m_cb;
      //! Count of used bytes.
      std::size_t m_cbUsed;
   };

public:
   /*! Constructor.

   @param pbw
      Pointer to a buffered writer to wrap.
   */
   default_buffered_writer(std::shared_ptr<writer> pbw);

   //! Destructor.
   virtual ~default_buffered_writer();

   //! See buffered_writer::async_join().
   virtual std::size_t async_join() override;

   //! See buffered_writer::async_pending().
   virtual bool async_pending() const override;

   //! See buffered_writer::commit_bytes().
   virtual void commit_bytes(std::size_t cb) override;

   //! See buffered_writer::flush().
   virtual void flush() override;

   //! See buffered_writer::get_buffer_bytes().
   virtual std::pair<void *, std::size_t> get_buffer_bytes(std::size_t cb) override;

protected:
   /*! Returns true if there’s any buffered data that may be flushed.

   @return
      true if flush_buffer() can be called, or false otherwise.
   */
   bool any_buffered_data() const;

   /*! Returns the size of the available space in the first buffer.

   @return
      Available space in the first buffer, or 0 if no buffers have been allocated.
   */
   std::size_t available_buffer() const;

   /*! Flushes the internal write buffer. If an asynchronous write was already in progress, this
   will block until it’s able to begin a new write.

   @return
      true if the write completed immediately, or false if it will be completed asynchronously.
   */
   bool flush_buffer();

   //! See buffered_writer::_unbuffered_base().
   virtual std::shared_ptr<base> _unbuffered_base() const override;

protected:
   //! Wrapped binary writer.
   std::shared_ptr<writer> m_pbw;
   /*! List of write buffers. May contain more than one in case *m_pbw is asynchronous and slow, in
   which case front() is the one to be used first, and the others are held until their respective
   I/O async operations complete. */
   list<buffer> m_lbufWriteBufs;
   //! Default/increment size of m_pbWriteBuf.
   // TODO: tune this value.
   static std::size_t const smc_cbWriteBufDefault = 0x1000;
};

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
