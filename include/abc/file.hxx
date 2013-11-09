/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013
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

#ifndef ABC_FILE_HXX
#define ABC_FILE_HXX

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abc/file_path.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals

namespace abc {

// Some C libraries (such as MS CRT) define these as macros.
#ifdef stdin
   #undef stdin
   #undef stdout
   #undef stderr
#endif

/** List of standard (OS-provided) files.
*/
ABC_ENUM(stdfile, \
   /** Internal identifier for stdin. */ \
   (stdin,  0), \
   /** Internal identifier for stdout. */ \
   (stdout, 1), \
   /** Internal identifier for stderr. */ \
   (stderr, 2) \
);


/** Native OS file descriptor/handle. */
#if ABC_HOST_API_POSIX
   typedef int filedesc_t;
#elif ABC_HOST_API_WIN32
   typedef HANDLE filedesc_t;
#else
   #error TODO-PORT: HOST_API
#endif

/** Integer wide enough to express any valid file offset. */
#if ABC_HOST_API_POSIX
   typedef uint64_t fileint_t;
#elif ABC_HOST_API_WIN32
   typedef uint64_t fileint_t;
#else
   #error TODO-PORT: HOST_API
#endif

} //namespace abc




////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::filedesc


namespace abc {

/** Wrapper for filedesc_t, to implement RAII. Similar in concept to std::unique_ptr, except it
doesn’t always own the wrapped filedesc_t (e.g. for standard files).
*/
class ABCAPI filedesc :
   public support_explicit_operator_bool<filedesc> {

   ABC_CLASS_PREVENT_COPYING(filedesc)

public:

   /** Constructor.

   fd
      Source file descriptor.
   [bOwn]
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


   /** Destructor.
   */
   ~filedesc();


   /** Assignment operator.

   fd
      Source file descriptor.
   return
      *this.
   */
   filedesc & operator=(filedesc_t fd);
   filedesc & operator=(filedesc && fd);


   /** Safe bool operator.

   return
      true if the object has a valid file descriptor, or false otherwise.
   */
   explicit_operator_bool() const {
      return m_fd != smc_fdNull;
   }


   /** Returns the wrapped raw file descriptor.

   return
      Wrapped raw file descriptor.
   */
   filedesc_t get() const {
      return m_fd;
   }


   /** Yields ownership over the wrapped file descriptor, returning it.

   return
      Unowned raw file descriptor.
   */
   filedesc_t release() {
      filedesc_t fd(m_fd);
      m_fd = smc_fdNull;
      return fd;
   }


private:

   /** The actual descriptor. */
   filedesc_t m_fd;
   /** If true, the wrapper will close the file on destruction. */
   bool m_bOwn;

   /** Logically null file descriptor. */
   static filedesc_t const smc_fdNull;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::file


namespace abc {

// Forward declaration. This is only defined in file.cxx, after the necessary header files have been
// included.
struct _file_init_data;


/** OS-native file (regular or pseudo).
*/
class ABCAPI file {

   ABC_CLASS_PREVENT_COPYING(file)

public:

   /** File access modes.
   */
   ABC_ENUM(access_mode, \
      /** Read-only access. */ \
      (read,       1), \
      /** Write-only access. */ \
      (write,      2), \
      /** Read/write access. */ \
      (read_write, 3), \
      /** Append-only access. */ \
      (append,     4) \
   );


public:

   /** Constructor.

   pfid
      Data used to initialize the object, as set by open() and other methods.
   */
   file(_file_init_data * pfid);


   /** Destructor.
   */
   virtual ~file();


   /** Returns new file object controlling the specified file descriptor.

   fd
      File descriptor to take ownership of.
   return
      Pointer to a new file controlling fd.
   */
   static std::shared_ptr<file> attach(filedesc && fd);


   /** Writes to the file any data being buffered.
   */
   void flush();


   /** Returns true if the file has a defined size, which is stored in m_cb.

   return
      true if the file has a size, or false otherwise.
   */
   bool has_size() const {
      return m_bHasSize;
   }


   /** Returns true if the OS is buffering reads/writes to m_fd.

   return
      true if the file is buffered by the OS, or false otherwise.
   */
   bool is_buffered() const {
      return m_bBuffered;
   }


   /** Opens a file, returning a new file object with the desired access to the specified file.

   fp
      Path to the file.
   fam
      Desired access mode.
   [bBuffered]
      If true, access to the file will be buffered by the OS, if false, access to the file will be
      unbuffered.
   return
      Pointer to a new file object for fp.
   */
   static std::shared_ptr<file> open(file_path const & fp, access_mode fam, bool bBuffered = true);


   /** Returns the physical alignment for unbuffered/direct disk access.

   return
      Alignment boundary, in bytes.
   */
   virtual unsigned physical_alignment() const;


   /** Reads at most cbMax bytes from the file.

   p
      Address of the destination buffer.
   cbMax
      Size of the destination buffer, in bytes.
   return
      Count of bytes read. For non-zero values of cb, a return value of 0 indicates that the end of
      the file was reached.
   */
   virtual size_t read(void * p, size_t cbMax);


   /** Returns the computed size of the file if applicable, or 0 otherwise.

   return
      Size of the file, in bytes, or 0 if not applicable.
   */
   virtual fileint_t size() const;


   /** Returns the file associated to the standard error output (stderr).

   return
      Standard error file.
   */
   static std::shared_ptr<file> const & stderr();


   /** Returns the file associated to the standard input (stdin).

   return
      Standard input file.
   */
   static std::shared_ptr<file> const & stdin();


   /** Returns the file associated to the standard output (stdout).

   return
      Standard output file.
   */
   static std::shared_ptr<file> const & stdout();


   /** Writes an array of bytes to the file.

   p
      Address of the source buffer.
   cb
      Size of the source buffer, in bytes.
   return
      Count of bytes written.
   */
   virtual size_t write(void const * p, size_t cb);


private:

   /** Instantiates a file of the appropriate type for the descriptor in *pfid, returning a shared
   pointer to it.

   pfid
      Data that will be passed to the constructor of the file object.
   return
      Shared pointer to the newly created file object.
   */
   static std::shared_ptr<file> _construct_matching_type(_file_init_data * pfid);


   /** Initializes a standard file object.

   fd
      Standard file descriptor to create a file object for.
   pppf
      Second-level pointer to a shared pointer; the shared pointer will be dynamically allocated,
      and the file object it will point to will also be dynamically allocated.
   */
   static void _construct_std_file(filedesc_t fd, std::shared_ptr<file> ** pppf);


   /** Releases any objects constructed by _construct_std_file().
   */
   static void ABC_STL_CALLCONV _release_std_files();


protected:

   /** Descriptor of the underlying file. */
   filedesc m_fd;
   /** If true, the file has a defined size, otherwise size() will always return 0. */
   bool m_bHasSize:1;
   /** If true, the OS will buffer reads/writes to m_fd. */
   bool m_bBuffered:1;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::console_file


namespace abc {

/** Console/terminal pseudo-file.
*/
class ABCAPI console_file :
   public file {

   ABC_CLASS_PREVENT_COPYING(console_file)

public:

   /** Constructor. See abc::file::file().
   */
   console_file(_file_init_data * pfid);


#if ABC_HOST_API_WIN32

   // In Win32, console files must use a dedicated API in order to support the native character
   // type.

   /** See abc::file::read().
   */
   virtual size_t read(void * p, size_t cbMax);


   /** See abc::file::write().
   */
   virtual size_t write(void const * p, size_t cb);

#endif //if ABC_HOST_API_WIN32
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::pipe_file


namespace abc {

/** Pipe file.
*/
class ABCAPI pipe_file :
   public file {

   ABC_CLASS_PREVENT_COPYING(pipe_file)

public:

   /** Constructor. See abc::file::file().
   */
   pipe_file(_file_init_data * pfid);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::regular_file


namespace abc {

/** File that behaves like a regular file on disk.
*/
class ABCAPI regular_file :
   public file {

   ABC_CLASS_PREVENT_COPYING(regular_file)

public:

   /** Constructor. See abc::file::file().
   */
   regular_file(_file_init_data * pfid);


   /** See abc::file::physical_alignment().
   */
   virtual unsigned physical_alignment() const;


   /** See abc::file::size().
   */
   virtual fileint_t size() const;


#if ABC_HOST_API_WIN32

   /** See abc::file::write(). This override is necessary to emulate O_APPEND in Win32.
   */
   virtual size_t write(void const * p, size_t cb);

#endif //if ABC_HOST_API_WIN32


protected:

   /** Computed size of the file. */
   fileint_t m_cb;
   /** Physical alignment for unbuffered/direct disk access. */
   unsigned m_cbPhysAlign;
#if ABC_HOST_API_WIN32
   /** If true, write() will emulate POSIX’s O_APPEND in platforms that don’t support it. */
   bool m_bAppend:1;
#endif
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_FILE_HXX

