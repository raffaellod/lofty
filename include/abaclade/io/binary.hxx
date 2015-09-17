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

#ifndef _ABACLADE_IO_BINARY_HXX
#define _ABACLADE_IO_BINARY_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/io.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Base interface for binary (non-text) I/O.
class ABACLADE_SYM base {
public:
   //! Destructor. Needed to make the class polymorphic (have a vtable).
   virtual ~base();

protected:
   //! Default constructor.
   base();
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Interface for binary (non-text) input.
class ABACLADE_SYM reader : public virtual base {
public:
   //! Destructor.
   virtual ~reader();

   /*! Reads at most cbMax bytes.

   @param p
      Address of the destination buffer.
   @param cbMax
      Size of the destination buffer, in bytes.
   @return
      Count of bytes read. For non-zero values of cbMax, a return value of 0 indicates that the end
      of the data (EOF) was reached.
   */
   virtual std::size_t read(void * p, std::size_t cbMax) = 0;

protected:
   //! Default constructor.
   reader();
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Interface for binary (non-text) output.
class ABACLADE_SYM writer : public virtual base {
public:
   //! Destructor.
   virtual ~writer();

   /*! Flushes the write buffer and closes the underlying backend, ensuring that no error conditions
   remain possible in the destructor. */
   virtual void finalize() = 0;

   //! Forces writing any data in the write buffer.
   virtual void flush() = 0;

   /*! Writes an array of bytes.

   @param p
      Address of the source buffer.
   @param cb
      Size of the source buffer, in bytes.
   @return
      Count of bytes written.
   */
   virtual std::size_t write(void const * p, std::size_t cb) = 0;

protected:
   //! Default constructor.
   writer();
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Interface for binary I/O classes that allow random access (e.g. seek/tell operations).
class ABACLADE_SYM seekable {
public:
   /*! Changes the current read/write position.

   @param iOffset
      New position, relative to sfWhence.
   @param sfWhence
      Indicates what position iOffset is relative to.
   @return
      Resulting position.
   */
   virtual offset_t seek(offset_t ibOffset, seek_from sfWhence) = 0;

   /*! Returns the current read/write position.

   @return
      Current position.
   */
   virtual offset_t tell() const = 0;
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Interface for binary I/O classes that access data with a known size.
class ABACLADE_SYM sized {
public:
   /*! Returns the size of the data.

   @return
      Data size, in bytes.
   */
   virtual full_size_t size() const = 0;
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Interface for buffering objects that wrap binary::* instances.
class ABACLADE_SYM buffered_base : public virtual base {
public:
   //! Destructor.
   virtual ~buffered_base();

   /*! Returns a pointer to the wrapped unbuffered binary I/O object.

   @return
      Pointer to a unbuffered binary I/O object.
   */
   _std::shared_ptr<base> unbuffered() const {
      return _unbuffered_base();
   }

protected:
   //! Default constructor.
   buffered_base();

   /*! Implementation of unbuffered(). This enables unbuffered() to be non-virtual, which in turn
   allows derived classes to override it changing its return type to be more specific.

   @return
      Pointer to a unbuffered binary I/O object.
   */
   virtual _std::shared_ptr<base> _unbuffered_base() const = 0;
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Interface for buffering objects that wrap binary::reader instances.
class ABACLADE_SYM buffered_reader : public virtual buffered_base, public reader {
public:
   //! Destructor.
   virtual ~buffered_reader();

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

   @param c
      Count of items to peek. If greater than the size of the read buffer’s contents, an additional
      read from the underlying binary reader will be made, adding to the contents of the read
      buffer; if the internal buffer is not large enough to hold the cumulative data, it will be
      enlarged.
   @return
      Pair containing a pointer to the portion of the internal buffer that holds the read data, and
      the count of bytes read. The latter may be less than the cb argument if EOF is reached, or
      greater than cb if the buffer was filled more than requested. For non-zero values of cb, a
      count of 0 bytes read indicates that no more data is available (EOF).
   */
   template <typename T>
   _std::tuple<T const *, std::size_t> peek(std::size_t c = 1) {
      auto ret(peek_bytes(sizeof(T) * c));
      // Repack the tuple, changing pointer type.
      return _std::make_tuple(static_cast<T const *>(_std::get<0>(ret)), _std::get<1>(ret));
   }

   /*! Non-template implementation of peek(). See peek().

   @param cb
      Count of bytes to peek.
   */
   virtual _std::tuple<void const *, std::size_t> peek_bytes(std::size_t cb) = 0;

   /*! See binary::reader::read(). Using peek()/consume() or peek_bytes()/consume_bytes() is
   preferred to calling this method, because it will spare the caller from having to allocate an
   intermediate buffer. */
   virtual std::size_t read(void * p, std::size_t cbMax) override;

   //! See buffered_base::unbuffered().
   _std::shared_ptr<reader> unbuffered() const {
      return _std::dynamic_pointer_cast<reader>(_unbuffered_base());
   }

protected:
   //! Default constructor.
   buffered_reader();
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Interface for buffering objects that wrap binary::writer instances.
class ABACLADE_SYM buffered_writer : public virtual buffered_base, public writer {
public:
   //! Destructor.
   virtual ~buffered_writer();

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
   _std::tuple<T *, std::size_t> get_buffer(std::size_t c) {
      auto ret(get_buffer_bytes(sizeof(T) * c));
      // Repack the tuple, changing pointer type.
      return _std::make_tuple(static_cast<T *>(_std::get<0>(ret)), _std::get<1>(ret));
   }

   /*! Byte-oriented implementation of get_buffer(). See get_buffer().

   @param cb
      Count of bytes to create buffer space for.
   */
   virtual _std::tuple<void *, std::size_t> get_buffer_bytes(std::size_t cb) = 0;

   //! See buffered_base::unbuffered().
   _std::shared_ptr<writer> unbuffered() const {
      return _std::dynamic_pointer_cast<writer>(_unbuffered_base());
   }

   /*! See binary::writer::write(). Using get_buffer()/commit() or get_buffer_bytes()/commit_bytes()
   is preferred to calling this method, because it will spare the caller from having to allocate an
   intermediate buffer. */
   virtual std::size_t write(void const * p, std::size_t cb) override;

protected:
   //! Default constructor.
   buffered_writer();
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary { namespace detail {

/*! Data collected by open() used to construct a file instance. This is only defined in file.cxx,
after the necessary header files have been included. */
struct file_init_data;

}}}} //namespace abc::io::binary::detail

namespace abc { namespace io { namespace binary {

//! Base for file binary I/O classes.
class ABACLADE_SYM file_base : public virtual base, public noncopyable {
public:
   //! Destructor.
   virtual ~file_base();

protected:
   /*! Constructor.

   @param pfid
      Data used to initialize the object, as set by abc::io::binary::open() and other functions.
   */
   file_base(detail::file_init_data * pfid);

protected:
   //! Descriptor of the underlying file.
   filedesc m_fd;
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Binary file input.
class ABACLADE_SYM file_reader : public virtual file_base, public reader {
public:
   //! See file_base::file_base().
   file_reader(detail::file_init_data * pfid);

   //! Destructor.
   virtual ~file_reader();

   //! See reader::read().
   virtual std::size_t read(void * p, std::size_t cbMax) override;

protected:
#if ABC_HOST_API_WIN32
   /*! Detects EOF conditions and real errors. Necessary because under Win32 there are major
   differences in detection of EOF depending on the file type.

   @param cbRead
      Count of bytes read by ::ReadFile().
   @param iErr
      Value returned by ::GetLastError() if ::ReadFile() returned false, or ERROR_SUCCESS otherwise.
   @return
      true if ::ReadFile() indicated that EOF was reached, or false otherwise. Exceptions are
      thrown for all non-EOF error conditions.
   */
   virtual bool check_if_eof_or_throw_os_error(::DWORD cbRead, ::DWORD iErr) const;
#endif
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Binary file output.
class ABACLADE_SYM file_writer : public virtual file_base, public writer {
public:
   //! See writer::writer().
   file_writer(detail::file_init_data * pfid);

   //! Destructor.
   virtual ~file_writer();

   //! See writer::finalize().
   virtual void finalize() override;

   //! See writer::flush().
   virtual void flush() override;

   //! See writer::write().
   virtual std::size_t write(void const * p, std::size_t cb) override;
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Bidirectional binary file.
class ABACLADE_SYM file_readwriter : public virtual file_reader, public virtual file_writer {
public:
   //! See file_reader::file_reader() and file_writer::file_writer().
   file_readwriter(detail::file_init_data * pfid);

   //! Destructor.
   virtual ~file_readwriter();
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Contains the two ends of a pipe.
struct pipe_ends {
   //! Reader end.
   _std::shared_ptr<file_reader> reader;
   //! Writer end.
   _std::shared_ptr<file_writer> writer;

   /*! Constructor.

   @param pbprReader
      Reader end.
   @param pbpwWriter
      Writer end.
   */
   pipe_ends(_std::shared_ptr<file_reader> pbprReader, _std::shared_ptr<file_writer> pbpwWriter) :
      reader(_std::move(pbprReader)),
      writer(_std::move(pbpwWriter)) {
   }

   /*! Move constructor.

   @param pe
      Source object.
   */
   pipe_ends(pipe_ends && pe) :
      reader(_std::move(pe.reader)),
      writer(_std::move(pe.writer)) {
   }

   /*! Move-assignment operator.

   @param pe
      Source object.
   */
   pipe_ends & operator=(pipe_ends && pe) {
      reader = _std::move(pe.reader);
      writer = _std::move(pe.writer);
      return *this;
   }
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declaration.
namespace abc { namespace os {

class path;

}} //namespace abc::os

namespace abc { namespace io { namespace binary {

/*! Creates and returns a buffered reader wrapper for the specified unbuffered binary reader.

@param pbr
   Pointer to an unbuffered binary reader.
@return
   Pointer to a buffered wrapper for *pbr.
*/
ABACLADE_SYM _std::shared_ptr<buffered_reader> buffer_reader(_std::shared_ptr<reader> pbr);

/*! Creates and returns a buffered writer wrapper for the specified unbuffered binary writer.

@param pbw
   Pointer to an unbuffered binary writer.
@return
   Pointer to a buffered wrapper for *pbw.
*/
ABACLADE_SYM _std::shared_ptr<buffered_writer> buffer_writer(_std::shared_ptr<writer> pbw);

/*! Creates and returns a binary reader for the specified file descriptor.

@param fd
   File descriptor.
@return
   Pointer to a binary reader for the file descriptor.
*/
ABACLADE_SYM _std::shared_ptr<file_reader> make_reader(io::filedesc && fd);

/*! Creates and returns a binary reader/writer for the specified file descriptor.

@param fd
   File descriptor.
@return
   Pointer to a binary reader/writer for the file descriptor.
*/
ABACLADE_SYM _std::shared_ptr<file_readwriter> make_readwriter(io::filedesc && fd);

/*! Creates and returns a binary writer for the specified file descriptor.

@param fd
   File descriptor.
@return
   Pointer to a binary writer for the file descriptor.
*/
ABACLADE_SYM _std::shared_ptr<file_writer> make_writer(io::filedesc && fd);

/*! Opens a file for binary access.

@param op
   Path to the file.
@param am
   Desired access mode.
@param bBypassCache
   If true, the OS will not cache any portion of the file; if false, accesses to the file will be
   backed by the OS file cache subsystem.
@return
   Pointer to a binary I/O object for the file.
*/
ABACLADE_SYM _std::shared_ptr<file_base> open(
   os::path const & op, access_mode am, bool bBypassCache = false
);

/*! Opens a file for binary reading.

@param op
   Path to the file.
@param bBypassCache
   If true, the OS will not cache any portion of the file; if false, accesses to the file will be
   backed by the OS file cache subsystem.
@return
   Pointer to a binary reader for the file.
*/
inline _std::shared_ptr<file_reader> open_reader(os::path const & op, bool bBypassCache = false) {
   return _std::dynamic_pointer_cast<file_reader>(open(op, access_mode::read, bBypassCache));
}

/*! Opens a file for binary writing.

@param op
   Path to the file.
@param bBypassCache
   If true, the OS will not cache any portion of the file; if false, accesses to the file will be
   backed by the OS file cache subsystem.
@return
   Pointer to a binary writer for the file.
*/
inline _std::shared_ptr<file_writer> open_writer(os::path const & op, bool bBypassCache = false) {
   return _std::dynamic_pointer_cast<file_writer>(open(op, access_mode::write, bBypassCache));
}

/*! Opens a file for binary reading and writing.

@param op
   Path to the file.
@param bBypassCache
   If true, the OS will not cache any portion of the file; if false, accesses to the file will be
   backed by the OS file cache subsystem.
@return
   Pointer to a binary reader/writer for the file.
*/
inline _std::shared_ptr<file_readwriter> open_readwriter(
   os::path const & op, bool bBypassCache = false
) {
   return _std::dynamic_pointer_cast<file_readwriter>(open(op, access_mode::write, bBypassCache));
}

/*! Creates a unidirectional pipe (FIFO), returning a reader and a writer connected to its ends.

@return
   An object containing the reader end and the writer end of the pipe.
*/
ABACLADE_SYM pipe_ends pipe();

//! Binary writer associated to the standard error output file.
extern ABACLADE_SYM _std::shared_ptr<writer> stderr;
//! Binary reader associated to the standard input file.
extern ABACLADE_SYM _std::shared_ptr<reader> stdin;
//! Binary writer associated to the standard output file.
extern ABACLADE_SYM _std::shared_ptr<writer> stdout;

}}} //namespace abc::io::binary

namespace abc { namespace io { namespace binary { namespace detail {

/*! Creates and returns a binary writer associated to the standard error output file (stderr).

@return
   Standard error file.
*/
ABACLADE_SYM _std::shared_ptr<writer> make_stderr();

/*! Creates and returns a binary reader associated to the standard input file (stdin).

@return
   Standard input file.
*/
ABACLADE_SYM _std::shared_ptr<reader> make_stdin();

/*! Creates and returns a binary writer associated to the standard output file (stdout).

@return
   Standard output file.
*/
ABACLADE_SYM _std::shared_ptr<writer> make_stdout();

}}}} //namespace abc::io::binary::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_IO_BINARY_HXX
