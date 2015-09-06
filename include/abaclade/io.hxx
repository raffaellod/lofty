﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
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

namespace abc { namespace io {

//! Unsigned integer wide enough to express an I/O-related size.
#if ABC_HOST_API_POSIX || ABC_HOST_API_WIN32
   typedef std::uint64_t full_size_t;
#else
   #error "TODO: HOST_API"
#endif

//! Integer wide enough to express an I/O-related offset.
#if ABC_HOST_API_POSIX || ABC_HOST_API_WIN32
   typedef std::int64_t offset_t;
#else
   #error "TODO: HOST_API"
#endif

//! Native OS file descriptor/handle.
#if ABC_HOST_API_POSIX
   typedef int filedesc_t;
#elif ABC_HOST_API_WIN32
   typedef ::HANDLE filedesc_t;
#else
   #error "TODO: HOST_API"
#endif

//! File access modes.
ABC_ENUM_AUTO_VALUES(access_mode,
   read,        //! Read-only access.
   read_write,  //! Read/write access.
   write,       //! Write-only access.
   write_append //! Append-only access.
);

//! Position indicators to which offsets may be relative.
ABC_ENUM_AUTO_VALUES(seek_from,
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
ABC_ENUM_AUTO_VALUES(stdfile,
   stdin,  //! Internal identifier for stdin.
   stdout, //! Internal identifier for stdout.
   stderr  //! Internal identifier for stderr.
);

}} //namespace abc::io

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io {

//! Wrapper for filedesc_t, to implement RAII; similar to std::unique_ptr.
class ABACLADE_SYM filedesc : public support_explicit_operator_bool<filedesc>, public noncopyable {
public:
   //! Default constructor.
   filedesc() :
      m_fd(smc_fdNull)
#if ABC_HOST_API_WIN32
      , m_fdIocp(smc_fdNull)
#endif
   {
   }

   /*! Move constructor.

   @param fd
      Source object.
   */
   filedesc(filedesc && fd) :
      m_fd(fd.m_fd)
#if ABC_HOST_API_WIN32
      , m_fdIocp(fd.m_fdIocp)
#endif
   {
      fd.m_fd = smc_fdNull;
#if ABC_HOST_API_WIN32
      fd.m_fdIocp = smc_fdNull;
#endif
   }

   /*! Constructor.

   @param fd
      Source file descriptor that *this will take ownership of, releasing it when appropriate.
   */
   explicit filedesc(filedesc_t fd) :
      m_fd(fd)
#if ABC_HOST_API_WIN32
      , m_fdIocp(smc_fdNull)
#endif
   {
   }

   //! Destructor.
   ~filedesc();

   /*! Move-assignment operator.

   @param fd
      Source file descriptor.
   @return
      *this.
   */
   filedesc & operator=(filedesc && fd);

   /*! Boolean evaluation operator.

   @return
      true if the object has a valid file descriptor, or false otherwise.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return m_fd != smc_fdNull;
   }

#if ABC_HOST_API_WIN32
   /*! Associates the file descriptor to the IOCP of the coroutine::scheduler for the current
   thread, blocking attempts to associate a file descriptor to more than one IOCP. */
   void bind_to_this_coroutine_scheduler_iocp();
#endif

   /*! Returns the wrapped raw file descriptor.

   @return
      Wrapped raw file descriptor.
   */
   filedesc_t get() const {
      return m_fd;
   }

   /*! Yields ownership over the wrapped file descriptor, returning it.

   @return
      Unowned raw file descriptor.
   */
   filedesc_t release() {
      filedesc_t fd = m_fd;
      m_fd = smc_fdNull;
      return fd;
   }

   /*! Closes the file descriptor, ensuring that no error conditions remain possible in the
   destructor. */
   void safe_close();

#if ABC_HOST_API_POSIX
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
   filedesc_t m_fd;
#if ABC_HOST_API_WIN32
   //! Handle to the IOCP this file has been associated to, if any.
   filedesc_t m_fdIocp;
#endif
   //! Logically null file descriptor.
   static filedesc_t const smc_fdNull;
};

}} //namespace abc::io

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

template <>
class to_str_backend<io::filedesc> : public to_str_backend<io::filedesc_t> {
public:
   //! See to_str_backend<io::filedesc_t>::write().
   void write(io::filedesc const & fd, io::text::writer * ptwOut) {
      to_str_backend<io::filedesc_t>::write(fd.get(), ptwOut);
   }
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#if ABC_HOST_API_WIN32
namespace abc { namespace io {

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

}} //namespace abc::io
#endif //if ABC_HOST_API_WIN32

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io {

//! An I/O operation failed for an I/O-related reason.
class ABACLADE_SYM error : public virtual generic_error {
public:
   //! Default constructor.
   error();

   /*! Copy constructor.

   @param x
      Source object.
   */
   error(error const & x);

   //! Destructor.
   virtual ~error();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   error & operator=(error const & x);

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

}} //namespace abc::io

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io {

//! An I/O operation failed for a network-related reason.
class ABACLADE_SYM network_error : public virtual error, public virtual abc::network_error {
public:
   //! Default constructor.
   network_error();

   /*! Copy constructor.

   @param x
      Source object.
   */
   network_error(network_error const & x);

   //! Destructor.
   virtual ~network_error();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   network_error & operator=(network_error const & x);

   //! See abc::error::init() and abc::network_error::init().
   void init(errint_t err = 0);
};

}} //namespace abc::io
