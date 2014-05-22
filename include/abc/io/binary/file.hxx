/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#ifndef _ABC_IO_BINARY_FILE_HXX
#define _ABC_IO_BINARY_FILE_HXX

#ifndef _ABC_HXX
   #error Please #include <abc.hxx> before this file
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abc/file_path.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io globals

namespace abc {
namespace io {

/** Native OS file descriptor/handle. */
#if ABC_HOST_API_POSIX
   typedef int filedesc_t;
#elif ABC_HOST_API_WIN32
   typedef HANDLE filedesc_t;
#else
   #error HOST_API
#endif


namespace binary {

/** Data collected by open() used to construct a file instance. This is only defined in file.cxx,
after the necessary header files have been included.
*/
struct _file_init_data;

// Forward declarations.
class file_base;
class file_reader;
class file_writer;


/** Returns the binary writer associated to the standard error output file (stderr).

return
   Standard error file.
*/
ABCAPI std::shared_ptr<file_writer> stderr();


/** Returns the binary reader associated to the standard input file (stdin).

return
   Standard input file.
*/
ABCAPI std::shared_ptr<file_reader> stdin();


/** Returns the binary writer associated to the standard output file (stdout).

return
   Standard output file.
*/
ABCAPI std::shared_ptr<file_writer> stdout();


/** Opens a file for binary access.

fp
   Path to the file.
am
   Desired access mode.
bBuffered
   If true, access to the file will be buffered by the OS, if false, access to the file will be
   unbuffered.
return
   Pointer to a binary I/O object for the file.
*/
std::shared_ptr<file_base> open(file_path const & fp, access_mode am, bool bBuffered = true);


/** Opens a file for binary reading.

fp
   Path to the file.
bBuffered
   If true, access to the file will be buffered by the OS, if false, access to the file will be
   unbuffered.
return
   Pointer to a binary reader for the file.
*/
inline std::shared_ptr<file_reader> open_reader(file_path const & fp, bool bBuffered = true) {
   return std::dynamic_pointer_cast<file_reader>(open(fp, access_mode::read, bBuffered));
}


/** Opens a file for binary writing.

fp
   Path to the file.
bBuffered
   If true, access to the file will be buffered by the OS, if false, access to the file will be
   unbuffered.
return
   Pointer to a binary writer for the file.
*/
inline std::shared_ptr<file_writer> open_writer(file_path const & fp, bool bBuffered = true) {
   return std::dynamic_pointer_cast<file_writer>(open(fp, access_mode::write, bBuffered));
}

} //namespace binary
} //namespace io
} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::filedesc


namespace abc {
namespace io {

/** Wrapper for filedesc_t, to implement RAII. Similar in concept to std::unique_ptr, except it
doesn’t always own the wrapped filedesc_t (e.g. for standard files).
*/
class ABCAPI filedesc :
   public support_explicit_operator_bool<filedesc>,
   public noncopyable {
public:

   /** Constructor.

   fd
      Source file descriptor.
   bOwn
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

} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::file_base


namespace abc {
namespace io {
namespace binary {

/** Base for file binary I/O classes.
*/
class ABCAPI file_base :
   public virtual base {
public:

   /** Destructor.
   */
   virtual ~file_base();


protected:

   /** Constructor.

   pfid
      Data used to initialize the object, as set by abc::io::open() and other functions.
   */
   file_base(_file_init_data * pfid);


protected:

   /** Descriptor of the underlying file. */
   filedesc m_fd;
};

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::file_reader


namespace abc {
namespace io {
namespace binary {

/** Binary file input.
*/
class ABCAPI file_reader :
   public virtual file_base,
   public reader {
public:

   /** See file_base::file_base().
   */
   file_reader(_file_init_data * pfid);


   /** Destructor.
   */
   virtual ~file_reader();


   /** See reader::read().
   */
   virtual size_t read(void * p, size_t cbMax);
};

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::file_writer


namespace abc {
namespace io {
namespace binary {

/** Binary file output.
*/
class ABCAPI file_writer :
   public virtual file_base,
   public writer {
public:

   /** See writer::writer().
   */
   file_writer(_file_init_data * pfid);


   /** Destructor.
   */
   virtual ~file_writer();


   /** See writer::flush().
   */
   virtual void flush();


   /** See writer::write().
   */
   virtual size_t write(void const * p, size_t cb);
};

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::console_reader


namespace abc {
namespace io {
namespace binary {

/** Console/terminal input pseudo-file.
*/
class ABCAPI console_reader :
   public file_reader {
public:

   /** See file_reader::file_reader().
   */
   console_reader(_file_init_data * pfid);


   /** Destructor.
   */
   virtual ~console_reader();


#if ABC_HOST_API_WIN32

   // Under Win32, console files must use a dedicated API in order to support the native character
   // type.

   /** See file_reader::read().
   */
   virtual size_t read(void * p, size_t cbMax);

#endif //if ABC_HOST_API_WIN32
};

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::console_writer


namespace abc {
namespace io {
namespace binary {

/** Console/terminal output pseudo-file.
*/
class ABCAPI console_writer :
   public file_writer {
public:

   /** See file_writer::file_writer().
   */
   console_writer(_file_init_data * pfid);


   /** Destructor.
   */
   virtual ~console_writer();


#if ABC_HOST_API_WIN32

   // Under Win32, console files must use a dedicated API in order to support the native character
   // type.

   /** See file_writer::write().
   */
   virtual size_t write(void const * p, size_t cb);

#endif //if ABC_HOST_API_WIN32
};

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::pipe_reader


namespace abc {
namespace io {
namespace binary {

/** Binary reader for the output end of a pipe.
*/
class ABCAPI pipe_reader :
   public file_reader {
public:

   /** See file_reader::file_reader().
   */
   pipe_reader(_file_init_data * pfid);


   /** Destructor.
   */
   virtual ~pipe_reader();
};

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::pipe_writer


namespace abc {
namespace io {
namespace binary {

/** Binary writer for the input end of a pipe.
*/
class ABCAPI pipe_writer :
   public file_writer {
public:

   /** See file_writer::file_writer().
   */
   pipe_writer(_file_init_data * pfid);


   /** Destructor.
   */
   virtual ~pipe_writer();
};

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::regular_file_base


namespace abc {
namespace io {
namespace binary {

/** Base for binary I/O classes for regular disk files.
*/
class ABCAPI regular_file_base :
   public virtual file_base,
   public seekable,
   public sized {
public:

   /** Destructor.
   */
   virtual ~regular_file_base();


   /** See seekable::seek().
   */
   virtual offset_t seek(offset_t ibOffset, seek_from sfWhence);


   /** See sized::size().
   */
   virtual full_size_t size() const;


   /** See seekable::tell().
   */
   virtual offset_t tell() const;


protected:

   /** See file_base::file_base().
   */
   regular_file_base(_file_init_data * pfid);


protected:

   /** Size of the file. */
   full_size_t m_cb;
#if 0
   /** Physical alignment for unbuffered/direct disk access. */
   unsigned m_cbPhysAlign;
#endif
};

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::regular_file_reader


namespace abc {
namespace io {
namespace binary {

/** Binary reader for regular disk files.
*/
class ABCAPI regular_file_reader :
   public virtual regular_file_base,
   public file_reader {
public:

   /** See regular_file_base().
   */
   regular_file_reader(_file_init_data * pfid);


   /** Destructor.
   */
   virtual ~regular_file_reader();
};

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::regular_file_writer


namespace abc {
namespace io {
namespace binary {

/** Binary writer for regular disk files.
*/
class ABCAPI regular_file_writer :
   public virtual regular_file_base,
   public file_writer {
public:

   /** See regular_file_base().
   */
   regular_file_writer(_file_init_data * pfid);


   /** Destructor.
   */
   virtual ~regular_file_writer();


#if ABC_HOST_API_WIN32

   /** See file_writer::write(). This override is necessary to emulate O_APPEND under Win32.
   */
   virtual size_t write(void const * p, size_t cb);

#endif //if ABC_HOST_API_WIN32


protected:

#if ABC_HOST_API_WIN32
   /** If true, write() will emulate POSIX’s O_APPEND in platforms that don’t support it. */
   bool m_bAppend:1;
#endif
};

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef _ABC_IO_BINARY_FILE_HXX

