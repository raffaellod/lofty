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
// abc::io::binary::detail::buffer

namespace abc {
namespace io {
namespace binary {
namespace detail {

/*! Self-managed, partitioned file buffer.

A buffer is divided in three portions that change in size as the buffer is filled and consumed:
unused, used and available.

The buffer is initially empty, which means that it’s completely available (for filling):
   ┌──────────────────────────────────────┐
   │available                             │ m_ibUsedOffet = m_ibAvailableOffset = 0, m_cb > 0
   └──────────────────────────────────────┘

As the buffer is read into, the used portion grows at expense of the available portion:
   ┌──────────────────┬───────────────────┐
   │used              │available          │ 0 = m_ibUsedOffet < m_ibAvailableOffset < m_cb
   └──────────────────┴───────────────────┘

Consuming (using) bytes from the buffer reduces the used size and increases the unused portion:
   ┌────────┬─────────┬───────────────────┐
   │unused  │used     │available          │ 0 < m_ibUsedOffet < m_ibAvailableOffset < m_cb
   └────────┴─────────┴───────────────────┘

Eventually no bytes are usable:
   ┌──────────────────┬───────────────────┐
   │unused            │available          │ 0 < m_ibUsedOffet = m_ibAvailableOffset
   └──────────────────┴───────────────────┘

More bytes are then loaded in the buffer, eventually consuming most of the available space:
   ┌──────────────────┬────────────┬──────┐
   │unused            │used        │avail.│ 0 < m_ibUsedOffet < m_ibAvailableOffset < m_cb
   └──────────────────┴────────────┴──────┘

And again, eventually most used bytes are consumed, resulting in insufficient usable bytes:
   ┌─────────────────────────────┬─┬──────┐
   │unused                       │u│avail.│ 0 < m_ibUsedOffet < m_ibAvailableOffset < m_cb
   └─────────────────────────────┴─┴──────┘

If more available bytes are needed to fulfill the next request, the buffer is recompacted by a call
to make_unused_available():
   ┌─┬────────────────────────────────────┐
   │u│available                           │ 0 = m_ibUsedOffet < m_ibAvailableOffset < m_cb
   └─┴────────────────────────────────────┘

And more bytes are read into the buffer, repeating the cycle.
   ┌──────────────────────┬───────────────┐
   │used                  │available      │ 0 = m_ibUsedOffet < m_ibAvailableOffset < m_cb
   └──────────────────────┴───────────────┘
*/
class ABACLADE_SYM buffer : public noncopyable {
public:
   /*! Constructor.

   @param cb
      Size of the buffer to allocate, in bytes.
   @param buf
      Source object.
   */
   buffer() :
      m_cb(0),
      m_ibUsedOffset(0),
      m_ibAvailableOffset(0) {
   }
   buffer(std::size_t cb);
   buffer(buffer && buf);

   //! Destructor.
   ~buffer();

   /*! Assignment operator.

   @param buf
      Source object.
   @return
      *this.
   */
   buffer & operator=(buffer && buf);

   /*! Returns the amount of available buffer space.

   @return
      Size of available buffer space, in bytes.
   */
   std::size_t available_size() const {
      return m_cb - m_ibAvailableOffset;
   }

   /*! Increases the size of the buffer.

   @param cb
      New size of the buffer, in bytes.
   */
   void expand(std::size_t cb);

   /*! Returns a pointer to the available portion of the buffer.

   @return
      Pointer to the available portion of the buffer.
   */
   std::int8_t * get_available() const {
      return static_cast<std::int8_t *>(m_p.get()) + m_ibAvailableOffset;
   }

   /*! Returns a pointer to the used portion of the buffer.

   @return
      Pointer to the used portion of the buffer.
   */
   std::int8_t * get_used() const {
      return static_cast<std::int8_t *>(m_p.get()) + m_ibUsedOffset;
   }

   /*! Shifts the used portion of the buffer to completely obliterate the unused portion, resulting
   in an increase in available space. */
   void make_unused_available();

   /*! Increases the unused bytes count, reducing the used bytes count.

   @param cb
      Bytes to count as unused.
   */
   void mark_as_unused(std::size_t cb) {
      m_ibUsedOffset += cb;
   }

   /*! Increases the used bytes count, reducing the available bytes count.

   @param cb
      Bytes to count as used.
   */
   void mark_as_used(std::size_t cb) {
      m_ibAvailableOffset += cb;
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
      Size of the used buffer space, in bytes.
   */
   std::size_t used_size() const {
      return m_ibAvailableOffset - m_ibUsedOffset;
   }

   /*! Returns the amount of unused buffer space.

   @return
      Size of the unused buffer space, in bytes.
   */
   std::size_t unused_size() const {
      return m_ibUsedOffset;
   }

private:
   //! Pointer to the allocated memory block.
   std::unique_ptr<void, memory::freeing_deleter> m_p;
   //! Size of *m_p.
   std::size_t m_cb;
   /*! Offset of the used portion of the buffer. Only bytes following the used portion are reported
   as available. */
   std::size_t m_ibUsedOffset;
   //! Count of used bytes.
   std::size_t m_ibAvailableOffset;
};

} //namespace detail
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
   //! Main read buffer.
   detail::buffer m_bufRead;
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

//! Provides buffering on top of a binary::writer instance.
class ABACLADE_SYM default_buffered_writer : public buffered_writer, public noncopyable {
public:
   /*! Constructor.

   @param pbw
      Pointer to a buffered writer to wrap.
   */
   default_buffered_writer(std::shared_ptr<writer> pbw);

   //! Destructor.
   virtual ~default_buffered_writer();

   //! See buffered_writer::commit_bytes().
   virtual void commit_bytes(std::size_t cb) override;

   //! See buffered_writer::flush().
   virtual void flush() override;

   //! See buffered_writer::get_buffer_bytes().
   virtual std::pair<void *, std::size_t> get_buffer_bytes(std::size_t cb) override;

protected:
   //! Flushes the internal write buffer.
   void flush_buffer();

   //! See buffered_writer::_unbuffered_base().
   virtual std::shared_ptr<base> _unbuffered_base() const override;

protected:
   //! Wrapped binary writer.
   std::shared_ptr<writer> m_pbw;
   //! Write buffer.
   detail::buffer m_bufWrite;
   //! If true, every commit_bytes() call will flush the buffer.
   bool m_bFlushAfterCommit:1;
   //! Default/increment size of m_pbWriteBuf.
   // TODO: tune this value.
   static std::size_t const smc_cbWriteBufDefault = 0x1000;
};

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
