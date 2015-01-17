/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

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
// abc::io globals

namespace abc {
namespace io {

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
   typedef HANDLE filedesc_t;
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

} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::filedesc

namespace abc {
namespace io {

/*! Wrapper for filedesc_t, to implement RAII. Similar in concept to std::unique_ptr, except it
doesn’t always own the wrapped filedesc_t (e.g. for standard files). */
class ABACLADE_SYM filedesc : public support_explicit_operator_bool<filedesc>, public noncopyable {
public:
   /*! Constructor.

   @param fd
      Source file descriptor.
   @param bOwn
      If true, the filedesc object will take ownership of the raw descriptor (i.e. it will release
      it whenever appropriate); if false, the raw descriptor will never be closed by this instance.
   */
   filedesc() :
      m_fd(smc_fdNull), m_bOwn(false) {
   }
   filedesc(filedesc_t fd, bool bOwn = true) :
      m_fd(fd), m_bOwn(bOwn) {
   }
   filedesc(filedesc && fd);

   //! Destructor.
   ~filedesc();

   /*! Assignment operator.

   @param fd
      Source file descriptor.
   @return
      *this.
   */
   filedesc & operator=(filedesc_t fd);
   filedesc & operator=(filedesc && fd);

   /*! Safe bool operator.

   @return
      true if the object has a valid file descriptor, or false otherwise.
   */
   explicit_operator_bool() const {
      return m_fd != smc_fdNull;
   }

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

private:
   //! The actual descriptor.
   filedesc_t m_fd;
   //! If true, the wrapper will close the file on destruction.
   bool m_bOwn;
   //! Logically null file descriptor.
   static filedesc_t const smc_fdNull;
};

} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::async

namespace abc {
namespace io {

//! Interface to operate I/O classes asynchronously.
class ABACLADE_SYM async {
public:
   /*! Waits for the completion of any pending I/O.

   @return
      Size of the transferred data, in bytes.
   */
   virtual std::size_t async_join() = 0;

   /*! Returns true if the object has any pending I/O operations.

   @return
      true if there are pending I/O operations on the object, or false otherwise.
   */
   virtual bool async_pending() = 0;
};

} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
