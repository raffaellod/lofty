/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_IO_HXX
#define _LOFTY_IO_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! I/O classes and functions. For an overview of the class/namespace hierarchy, see
doc/IO_class_hierarchy.fodg . */
namespace io {}

} //namespace lofty

namespace lofty { namespace io {

//! Unsigned integer wide enough to express an I/O-related size.
#if LOFTY_HOST_API_POSIX || LOFTY_HOST_API_WIN32
   typedef std::uint64_t full_size_t;
#else
   #error "TODO: HOST_API"
#endif

//! Integer wide enough to express an I/O-related offset.
#if LOFTY_HOST_API_POSIX || LOFTY_HOST_API_WIN32
   typedef std::int64_t offset_t;
#else
   #error "TODO: HOST_API"
#endif

//! Native OS file descriptor/handle.
#if LOFTY_HOST_API_POSIX
   typedef int filedesc_t;
#elif LOFTY_HOST_API_WIN32
   typedef ::HANDLE filedesc_t;
#else
   #error "TODO: HOST_API"
#endif

//! File access modes.
LOFTY_ENUM_AUTO_VALUES(access_mode,
   read,        //! Read-only access.
   read_write,  //! Read/write access.
   write,       //! Write-only access.
   write_append //! Append-only access.
);

//! Position indicators to which offsets may be relative.
LOFTY_ENUM_AUTO_VALUES(seek_from,
   start,   //! The offset is relative to the start of the data (absolute seek).
   current, //! The offset is relative to the current offset (incremental seek).
   end      //! The offset is relative to the end of the data and presumably negative.
);

// Some C libraries (such as MS CRT) define these as macros.
#ifdef stdin
   #undef stdin
   #undef stdout
   #undef stderr
#endif

//! List of standard (OS-provided) files.
LOFTY_ENUM_AUTO_VALUES(stdfile,
   stdin,  //! Internal identifier for stdin.
   stdout, //! Internal identifier for stdout.
   stderr  //! Internal identifier for stderr.
);

}} //namespace lofty::io

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io {

//! Wrapper for filedesc_t, to implement RAII; similar to std::unique_ptr.
class LOFTY_SYM filedesc : public support_explicit_operator_bool<filedesc>, public noncopyable {
public:
   //! Default constructor.
   filedesc() :
      fd(null_fd)
#if LOFTY_HOST_API_WIN32
      , iocp_fd(null_fd)
#endif
   {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   filedesc(filedesc && src) :
      fd(src.fd)
#if LOFTY_HOST_API_WIN32
      , iocp_fd(src.iocp_fd)
#endif
   {
      src.fd = null_fd;
#if LOFTY_HOST_API_WIN32
      src.iocp_fd = null_fd;
#endif
   }

   /*! Constructor.

   @param fd_
      Source file descriptor that *this will take ownership of, releasing it when appropriate.
   */
   explicit filedesc(filedesc_t fd_) :
      fd(fd_)
#if LOFTY_HOST_API_WIN32
      , iocp_fd(null_fd)
#endif
   {
   }

   //! Destructor.
   ~filedesc();

   /*! Move-assignment operator.

   @param src
      Source file descriptor.
   @return
      *this.
   */
   filedesc & operator=(filedesc && src);

   /*! Boolean evaluation operator.

   @return
      true if the object has a valid file descriptor, or false otherwise.
   */
   LOFTY_EXPLICIT_OPERATOR_BOOL() const {
      return fd != null_fd;
   }

#if LOFTY_HOST_API_WIN32
   /*! Associates the file descriptor to the IOCP of the coroutine::scheduler for the current thread, blocking
   attempts to associate a file descriptor to more than one IOCP. */
   void bind_to_this_coroutine_scheduler_iocp();
#endif

   /*! Returns the wrapped raw file descriptor.

   @return
      Wrapped raw file descriptor.
   */
   filedesc_t get() const {
      return fd;
   }

   /*! Yields ownership over the wrapped file descriptor, returning it.

   @return
      Unowned raw file descriptor.
   */
   filedesc_t release() {
      auto old_fd = fd;
      fd = null_fd;
      return old_fd;
   }

   //! Closes the file descriptor, ensuring that no error conditions remain possible in the destructor.
   void safe_close();

#if LOFTY_HOST_API_POSIX
   /*! Sets the CLOEXEC flag.

   @param b
      If true, CLOEXEC will be set; if false, it will be unset.
   */
   void set_close_on_exec(bool b);

   /*! Sets the NONBLOCK flag.

   @param b
      If true, NONBLOCK will be set; if false, it will be unset.
   */
   void set_nonblocking(bool b);
#endif

private:
   //! The actual descriptor.
   filedesc_t fd;
#if LOFTY_HOST_API_WIN32
   //! Handle to the IOCP this file has been associated to, if any.
   filedesc_t iocp_fd;
#endif
   //! Logically null file descriptor.
   static filedesc_t const null_fd;
};

}} //namespace lofty::io

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <>
class to_text_ostream<io::filedesc> : public to_text_ostream<io::filedesc_t> {
public:
   //! See to_text_ostream<io::filedesc_t>::write().
   void write(io::filedesc const & src, io::text::ostream * dst) {
      to_text_ostream<io::filedesc_t>::write(src.get(), dst);
   }
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_API_WIN32
namespace lofty { namespace io {

//! Extends OVERLAPPED with more information.
struct overlapped : public ::OVERLAPPED {
   //! Default constructor.
   overlapped() {
      hEvent = nullptr;
   }

   /*! Returns the status of the overlapped I/O operation.

   @return
      Status of the overlapped operation.
   */
   ::DWORD status() const {
      return static_cast< ::DWORD>(Internal);
   }

   /*! Returns the count of transferred bytes.

   @return
      Size of transferred data, in bytes.
   */
   ::DWORD transferred_size() const {
      return static_cast< ::DWORD>(InternalHigh);
   }

   /*! Calls GetOverlappedResult() to retrieve information about the I/O operation.

   @return
      Error status of the I/O operation.
   */
   ::DWORD get_result();
};

}} //namespace lofty::io
#endif //if LOFTY_HOST_API_WIN32

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io {

//! An I/O operation failed for an I/O-related reason.
class LOFTY_SYM error : public generic_error {
public:
   /*! Constructor.

   @param err
      OS-defined error number associated to the exception.
   */
   explicit error(errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   error(error const & src);

   //! Destructor.
   virtual ~error() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   error & operator=(error const & src);
};

}} //namespace lofty::io

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_IO_HXX
