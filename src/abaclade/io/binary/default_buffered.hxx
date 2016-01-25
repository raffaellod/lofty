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

#ifndef _ABACLADE_IO_BINARY_DEFAULT_BUFFERED_HXX
#define _ABACLADE_IO_BINARY_DEFAULT_BUFFERED_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/io/binary.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary { namespace detail {

/*! Self-managed, partitioned file buffer.

A buffer is divided in three portions that change in size as the buffer is filled and consumed:
unused, used and available.

The buffer is initially empty, which means that it’s completely available (for filling):
   @verbatim
   ┌──────────────────────────────────────┐
   │available                             │ m_ibUsedOffet = m_ibAvailableOffset = 0, m_cb > 0
   └──────────────────────────────────────┘
   @endverbatim

As the buffer is read into, the used portion grows at expense of the available portion:
   @verbatim
   ┌──────────────────┬───────────────────┐
   │used              │available          │ 0 = m_ibUsedOffet < m_ibAvailableOffset < m_cb
   └──────────────────┴───────────────────┘
   @endverbatim

Consuming (using) bytes from the buffer reduces the used size and increases the unused portion:
   @verbatim
   ┌────────┬─────────┬───────────────────┐
   │unused  │used     │available          │ 0 < m_ibUsedOffet < m_ibAvailableOffset < m_cb
   └────────┴─────────┴───────────────────┘
   @endverbatim

Eventually no bytes are usable:
   @verbatim
   ┌──────────────────┬───────────────────┐
   │unused            │available          │ 0 < m_ibUsedOffet = m_ibAvailableOffset
   └──────────────────┴───────────────────┘
   @endverbatim

More bytes are then loaded in the buffer, eventually consuming most of the available space:
   @verbatim
   ┌──────────────────┬────────────┬──────┐
   │unused            │used        │avail.│ 0 < m_ibUsedOffet < m_ibAvailableOffset < m_cb
   └──────────────────┴────────────┴──────┘
   @endverbatim

And again, eventually most used bytes are consumed, resulting in insufficient usable bytes:
   @verbatim
   ┌─────────────────────────────┬─┬──────┐
   │unused                       │u│avail.│ 0 < m_ibUsedOffet < m_ibAvailableOffset < m_cb
   └─────────────────────────────┴─┴──────┘
   @endverbatim

If more available bytes are needed to fulfill the next request, the buffer is recompacted by a call
to make_unused_available():
   @verbatim
   ┌─┬────────────────────────────────────┐
   │u│available                           │ 0 = m_ibUsedOffet < m_ibAvailableOffset < m_cb
   └─┴────────────────────────────────────┘
   @endverbatim

And more bytes are read into the buffer, repeating the cycle.
   @verbatim
   ┌──────────────────────┬───────────────┐
   │used                  │available      │ 0 = m_ibUsedOffet < m_ibAvailableOffset < m_cb
   └──────────────────────┴───────────────┘
   @endverbatim
*/
class ABACLADE_SYM buffer : public noncopyable {
public:
   //! Default constructor.
   buffer() :
      m_cb(0),
      m_ibUsedOffset(0),
      m_ibAvailableOffset(0) {
   }

   /*! Move constructor.

   @param buf
      Source object.
   */
   buffer(buffer && buf);

   /*! Constructor.

   @param cb
      Size of the buffer to allocate, in bytes.
   */
   buffer(std::size_t cb);

   //! Destructor.
   ~buffer();

   /*! Move-assignment operator.

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
   _std::unique_ptr<void, memory::freeing_deleter> m_p;
   //! Size of *m_p.
   std::size_t m_cb;
   /*! Offset of the used portion of the buffer. Only bytes following the used portion are reported
   as available. */
   std::size_t m_ibUsedOffset;
   //! Count of used bytes.
   std::size_t m_ibAvailableOffset;
};

}}}} //namespace abc::io::binary::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Provides buffering on top of a binary::istream instance.
class ABACLADE_SYM default_buffered_istream : public buffered_istream, public noncopyable {
public:
   /*! Constructor.

   @param pbr
      Pointer to a buffered istream to wrap.
   */
   default_buffered_istream(_std::shared_ptr<istream> pbis);

   //! Destructor.
   virtual ~default_buffered_istream();

   //! See buffered_istream::consume_bytes().
   virtual void consume_bytes(std::size_t cb) override;

   //! See buffered_istream::peek_bytes().
   virtual _std::tuple<void const *, std::size_t> peek_bytes(std::size_t cb) override;

protected:
   //! See buffered_istream::_unbuffered_stream().
   virtual _std::shared_ptr<stream> _unbuffered_stream() const override;

protected:
   //! Wrapped binary istream.
   _std::shared_ptr<istream> m_pbis;
   //! Main read buffer.
   detail::buffer m_bufRead;
   //! Default/increment size of m_pbReadBuf.
   // TODO: tune this value.
   static std::size_t const smc_cbReadBufDefault = 0x1000;
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Provides buffering on top of a binary::ostream instance.
class ABACLADE_SYM default_buffered_ostream : public buffered_ostream, public noncopyable {
public:
   /*! Constructor.

   @param pbw
      Pointer to a buffered output stream to wrap.
   */
   default_buffered_ostream(_std::shared_ptr<ostream> pbos);

   //! Destructor.
   virtual ~default_buffered_ostream();

   //! See buffered_ostream::commit_bytes().
   virtual void commit_bytes(std::size_t cb) override;

   //! See buffered_ostream::finalize().
   virtual void finalize() override;

   //! See buffered_ostream::flush().
   virtual void flush() override;

   //! See buffered_ostream::get_buffer_bytes().
   virtual _std::tuple<void *, std::size_t> get_buffer_bytes(std::size_t cb) override;

protected:
   //! Flushes the internal write buffer.
   void flush_buffer();

   //! See buffered_ostream::_unbuffered_stream().
   virtual _std::shared_ptr<stream> _unbuffered_stream() const override;

protected:
   //! Wrapped binary ostream.
   _std::shared_ptr<ostream> m_pbos;
   //! Write buffer.
   detail::buffer m_bufWrite;
   //! If true, every commit_bytes() call will flush the buffer.
   bool m_bFlushAfterCommit:1;
   //! Default/increment size of m_pbWriteBuf.
   // TODO: tune this value.
   static std::size_t const smc_cbWriteBufDefault = 0x1000;
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_IO_BINARY_DEFAULT_BUFFERED_HXX
