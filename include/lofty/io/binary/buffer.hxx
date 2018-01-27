/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_IO_BINARY_BUFFER_HXX
#define _LOFTY_IO_BINARY_BUFFER_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {

/*! Self-managed, partitioned buffer.

A buffer is divided in three portions that change in size as the buffer is filled and consumed: unused, used
and available.

The buffer is initially empty, which means that it’s completely available (for filling):
   @verbatim
   ┌──────────────────────────────────────┐
   │available                             │ used_offset = available_offset = 0, size > 0
   └──────────────────────────────────────┘
   @endverbatim

As the buffer is written to, the used portion grows at expense of the available portion:
   @verbatim
   ┌──────────────────┬───────────────────┐
   │used              │available          │ 0 = used_offset < available_offset < size
   └──────────────────┴───────────────────┘
   @endverbatim

Consuming (reading) bytes from the buffer reduces the used size and increases the unused portion:
   @verbatim
   ┌────────┬─────────┬───────────────────┐
   │unused  │used     │available          │ 0 < used_offset < available_offset < size
   └────────┴─────────┴───────────────────┘
   @endverbatim

Eventually no bytes are usable:
   @verbatim
   ┌──────────────────┬───────────────────┐
   │unused            │available          │ 0 < used_offset = available_offset
   └──────────────────┴───────────────────┘
   @endverbatim

More bytes are then loaded in the buffer, eventually consuming most of the available space:
   @verbatim
   ┌──────────────────┬────────────┬──────┐
   │unused            │used        │avail.│ 0 < used_offset < available_offset < size
   └──────────────────┴────────────┴──────┘
   @endverbatim

And again, eventually most used bytes are consumed, resulting in insufficient usable bytes:
   @verbatim
   ┌─────────────────────────────┬─┬──────┐
   │unused                       │u│avail.│ 0 < used_offset < available_offset < size
   └─────────────────────────────┴─┴──────┘
   @endverbatim

If more available bytes are needed to fulfill the next request, the buffer is recompacted by a call to
make_unused_available():
   @verbatim
   ┌─┬────────────────────────────────────┐
   │u│available                           │ 0 = used_offset < available_offset < size
   └─┴────────────────────────────────────┘
   @endverbatim

And more bytes are read into the buffer, repeating the cycle.
   @verbatim
   ┌──────────────────────┬───────────────┐
   │used                  │available      │ 0 = used_offset < available_offset < size
   └──────────────────────┴───────────────┘
   @endverbatim
*/
class LOFTY_SYM buffer : public noncopyable {
public:
   //! Default constructor.
   buffer() :
      size_(0),
      used_offset_(0),
      available_offset_(0) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   buffer(buffer && src);

   /*! Constructor.

   @param size
      Size of the buffer to allocate, in bytes.
   */
   explicit buffer(std::size_t size);

   //! Destructor.
   ~buffer();

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   buffer & operator=(buffer && src);

   /*! Returns the number of used bytes. Only bytes following the used portion are reported as available
   (writable).

   @return
      Number of used bytes.
   */
   std::size_t available_offset() const {
      return available_offset_;
   }

   /*! Returns the amount of available buffer space.

   @return
      Size of available buffer space, in bytes.
   */
   std::size_t available_size() const {
      return size_ - available_offset_;
   }

   /*! Increases the size of the buffer.

   @param new_size
      New size of the buffer, in bytes.
   */
   void expand_to(std::size_t new_size);

   /*! Returns a pointer to the available portion of the buffer.

   @return
      Pointer to the available portion of the buffer.
   */
   std::int8_t * get_available() const {
      return static_cast<std::int8_t *>(ptr.get()) + available_offset_;
   }

   /*! Returns a pointer to the used portion of the buffer.

   @return
      Pointer to the used portion of the buffer.
   */
   std::int8_t * get_used() const {
      return static_cast<std::int8_t *>(ptr.get()) + used_offset_;
   }

   /*! Shifts the used portion of the buffer to completely obliterate the unused portion, resulting in an
   increase in available space. */
   void make_unused_available();

   /*! Increases the unused bytes count, reducing the used bytes count.

   @param unused_size
      Bytes to count as unused.
   */
   void mark_as_unused(std::size_t unused_size_) {
      used_offset_ += unused_size_;
   }

   /*! Increases the used bytes count, reducing the available bytes count.

   @param used_size
      Bytes to count as used.
   */
   void mark_as_used(std::size_t used_size_) {
      available_offset_ += used_size_;
   }

   //! Marks the unused (already read) portion of the buffer back as used (to be read).
   void mark_unused_as_used() {
      used_offset_ = 0;
   }

   //! Reduces the size of the buffer, making it just large enough to contain all used bytes.
   void shrink_to_fit();

   /*! Returns the size of the buffer.

   @return
      Size of the buffer space, in bytes.
   */
   std::size_t size() const {
      return size_;
   }

   /*! Returns the amount of unused buffer space.

   @return
      Size of the unused buffer space, in bytes.
   */
   std::size_t unused_size() const {
      return used_offset_;
   }

   /*! Returns the offset of the used portion of the buffer. Only bytes following the used portion are
   reported as used (readable).

   @return
      Offset of the used portion of the buffer, in bytes.
   */
   std::size_t used_offset() const {
      return used_offset_;
   }

   /*! Returns the amount of used buffer space.

   @return
      Size of the used buffer space, in bytes.
   */
   std::size_t used_size() const {
      return available_offset_ - used_offset_;
   }

private:
   //! Pointer to the allocated memory block.
   _std::unique_ptr<void, memory::freeing_deleter> ptr;
   //! Size of *ptr.
   std::size_t size_;
   /*! Offset of the used portion of the buffer. Only bytes following the used portion are reported as used
   (readable). */
   std::size_t used_offset_;
   //! Count of used bytes. Only bytes following the used portion are reported as available (writable).
   std::size_t available_offset_;
};

}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_IO_BINARY_BUFFER_HXX
