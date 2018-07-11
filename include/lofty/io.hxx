/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
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

//! Logically null file descriptor.
extern LOFTY_SYM filedesc_t const filedesc_t_null;

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

/*! Wrapper for filedesc_t, to implement RAII; similar to std::unique_ptr.

While this class guarantees that a file descriptor is closed by the time its destructor is executed, at that
point any write errors will result in throwing in a destructor, which is an automatic call to
std::terminate().

In order to properly close a file descriptor opened for writing while checking for errors, close() must be
called explicitly. This may be done using LOFTY_TRY_FINALLY():

   @verbatim
   filedesc fd1(…);
   LOFTY_TRY_FINALLY({
      // Use fd1.
   }, {
      fd1.close();
   })
   @endverbatim

Note that Lofty’s classes that operate on file descriptors take care of this internally, implementing
lofty::io::closeable and requesting that the owner of each instance calls close() on it.
*/
class LOFTY_SYM filedesc : public support_explicit_operator_bool<filedesc>, public noncopyable {
public:
   //! Default constructor.
   filedesc() :
      fd(filedesc_t_null)
#if LOFTY_HOST_API_WIN32
      , iocp_fd(filedesc_t_null)
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
      src.fd = filedesc_t_null;
#if LOFTY_HOST_API_WIN32
      src.iocp_fd = filedesc_t_null;
#endif
   }

   /*! Constructor.

   @param fd_
      Source file descriptor that *this will take ownership of, releasing it when appropriate.
   */
   explicit filedesc(filedesc_t fd_) :
      fd(fd_)
#if LOFTY_HOST_API_WIN32
      , iocp_fd(filedesc_t_null)
#endif
   {
   }

   /*! Destructor. Automatically closes the file descriptor if still open, but errors in doing so will
   automatically terminate the process. */
   ~filedesc();

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   filedesc & operator=(filedesc && src);

   /*! Boolean evaluation operator.

   @return
      true if the object has a valid file descriptor, or false otherwise.
   */
   LOFTY_EXPLICIT_OPERATOR_BOOL() const {
      return fd != filedesc_t_null;
   }

#if LOFTY_HOST_API_WIN32
   /*! Associates the file descriptor to the IOCP of the coroutine::scheduler for the current thread, blocking
   attempts to associate a file descriptor to more than one IOCP. */
   void bind_to_this_coroutine_scheduler_iocp();
#endif

   /*! Closes the file descriptor. This is also called by the destructor; however, should any errors cause an
   exception to be thrown at that point, the process will be terminated per C++ standard, so it’s best to
   explicitly call this before the file descriptor is destructed. */
   void close();

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
      fd = filedesc_t_null;
      return old_fd;
   }

#if LOFTY_HOST_API_POSIX
   /*! Sets the NONBLOCK flag.

   @param b
      If true, NONBLOCK will be set; if false, it will be unset.
   */
   void set_nonblocking(bool b);
#endif

   /*! Allows or disallows child processes to use the file descriptor. Under POSIX, this clears or sets the
   CLOEXEC bit; under Win32, it sets or clears the HANDLE_FLAG_INHERIT bit.

   @param b
      If true, CLOEXEC will be set; if false, it will be unset.
   */
   void share_with_subprocesses(bool share);

private:
   //! The actual descriptor.
   filedesc_t fd;
#if LOFTY_HOST_API_WIN32
   //! Handle to the IOCP this file has been associated to, if any.
   filedesc_t iocp_fd;
#endif
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

//! Interface for (writable) streams that need to be manually closed before being destructed.
class LOFTY_SYM closeable {
public:
   /*! Flushes any write buffers and closes the stream, ensuring that no error conditions remain possible in
   the destructor. Calling any methods on a closed object results in undefined behavior. */
   virtual void close() = 0;
};

}} //namespace lofty::io

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

namespace lofty { namespace io {

//! An I/O operation failed due to a timeout.
class LOFTY_SYM timeout : public error {
public:
   /*! Constructor.

   @param err
      OS-defined error number associated to the exception.
   */
   explicit timeout(errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   timeout(timeout const & src);

   //! Destructor.
   virtual ~timeout() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   timeout & operator=(timeout const & src);
};

}} //namespace lofty::io

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_IO_HXX
