/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2016 Raffaello D. Di Napoli

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

namespace abc { namespace io {

//! Classes and functions to perform I/O in binary mode (raw bytes).
namespace binary {}

}} //namespace abc::io

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Base interface for binary (non-text) streams.
class ABACLADE_SYM stream {
public:
   //! Destructor. Needed to make the class polymorphic (have a vtable).
   virtual ~stream();

protected:
   //! Default constructor.
   stream();
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Interface for binary (non-text) input streams.
class ABACLADE_SYM istream : public virtual stream {
public:
   //! Destructor.
   virtual ~istream();

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
   istream();
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Interface for binary (non-text) output streams.
class ABACLADE_SYM ostream : public virtual stream {
public:
   //! Destructor.
   virtual ~ostream();

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
   ostream();
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Interface for binary streams that allow random access (e.g. seek/tell operations).
class ABACLADE_SYM seekable {
public:
   /*! Changes the current read/write position.

   @param ibOffset
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

//! Interface for binary streams that access data with a known size.
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

//! Interface for buffered streams that wrap binary streams.
class ABACLADE_SYM buffered_stream : public virtual stream {
public:
   //! Destructor.
   virtual ~buffered_stream();

   /*! Returns a pointer to the wrapped unbuffered binary stream.

   @return
      Pointer to a unbuffered binary stream.
   */
   _std::shared_ptr<stream> unbuffered() const {
      return _unbuffered_stream();
   }

protected:
   //! Default constructor.
   buffered_stream();

   /*! Implementation of unbuffered(). This enables unbuffered() to be non-virtual, which in turn
   allows derived classes to override it changing its return type to be more specific.

   @return
      Pointer to a unbuffered binary stream.
   */
   virtual _std::shared_ptr<stream> _unbuffered_stream() const = 0;
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Interface for buffered input streams that wrap binary input streams.
class ABACLADE_SYM buffered_istream : public virtual buffered_stream, public istream {
public:
   //! Destructor.
   virtual ~buffered_istream();

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
   binary input stream.

   @param c
      Count of items to peek. If greater than the size of the read buffer’s contents, an additional
      read from the underlying binary stream will be made, adding to the contents of the read
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

   /*! Reads at most cbMax bytes. Using peek()/consume() or peek_bytes()/consume_bytes() is
   preferred to calling this method, because it will spare the caller from having to allocate an
   intermediate buffer.

   @param p
      Address of the destination buffer.
   @param cbMax
      Size of the destination buffer, in bytes.
   @return
      Count of bytes read. For non-zero values of cbMax, a return value of 0 indicates that the end
      of the data (EOF) was reached.
   */
   virtual std::size_t read(void * p, std::size_t cbMax) override;

   //! See buffered_stream::unbuffered().
   _std::shared_ptr<istream> unbuffered() const {
      return _std::dynamic_pointer_cast<istream>(_unbuffered_stream());
   }

protected:
   //! Default constructor.
   buffered_istream();
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Interface for buffered output streams that wrap binary output streams.
class ABACLADE_SYM buffered_ostream : public virtual buffered_stream, public ostream {
public:
   //! Destructor.
   virtual ~buffered_ostream();

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

   //! See buffered_stream::unbuffered().
   _std::shared_ptr<ostream> unbuffered() const {
      return _std::dynamic_pointer_cast<ostream>(_unbuffered_stream());
   }

   /*! Writes an array of bytes. Using get_buffer()/commit() or get_buffer_bytes()/commit_bytes() is
   preferred to calling this method, because it will spare the caller from having to allocate an
   intermediate buffer.

   @param p
      Address of the source buffer.
   @param cb
      Size of the source buffer, in bytes.
   @return
      Count of bytes written.
   */
   virtual std::size_t write(void const * p, std::size_t cb) override;

protected:
   //! Default constructor.
   buffered_ostream();
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary { namespace _pvt {

/*! Data collected by open() used to construct a file instance. This is only defined in file.cxx,
after the necessary header files have been included. */
struct file_init_data;

}}}} //namespace abc::io::binary::_pvt

namespace abc { namespace io { namespace binary {

//! Base for file binary streams.
class ABACLADE_SYM file_stream : public virtual stream, public noncopyable {
public:
   //! Destructor.
   virtual ~file_stream();

protected:
   /*! Constructor.

   @param pfid
      Data used to initialize the object, as set by abc::io::binary::open() and other functions.
   */
   file_stream(_pvt::file_init_data * pfid);

protected:
   //! Descriptor of the underlying file.
   filedesc m_fd;
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Binary file input stream.
class ABACLADE_SYM file_istream : public virtual file_stream, public istream {
public:
   //! See file_stream::file_stream().
   file_istream(_pvt::file_init_data * pfid);

   //! Destructor.
   virtual ~file_istream();

   //! See istream::read().
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

//! Binary file output stream.
class ABACLADE_SYM file_ostream : public virtual file_stream, public ostream {
public:
   //! See ostream::ostream().
   file_ostream(_pvt::file_init_data * pfid);

   //! Destructor.
   virtual ~file_ostream();

   //! See ostream::finalize().
   virtual void finalize() override;

   //! See ostream::flush().
   virtual void flush() override;

   //! See ostream::write().
   virtual std::size_t write(void const * p, std::size_t cb) override;
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Bidirectional binary file stream.
class ABACLADE_SYM file_iostream : public virtual file_istream, public virtual file_ostream {
public:
   //! See file_istream::file_istream() and file_ostream::file_ostream().
   file_iostream(_pvt::file_init_data * pfid);

   //! Destructor.
   virtual ~file_iostream();
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Unidirectional pipe (FIFO), consisting in a read end stream and a write end stream.
class ABACLADE_SYM pipe {
public:
   //! Default constructor.
   pipe();

   /*! Move constructor.

   @param pp
      Source object.
   */
   pipe(pipe && pp) :
      read_end(_std::move(pp.read_end)),
      write_end(_std::move(pp.write_end)) {
   }

   /*! Move-assignment operator.

   @param pp
      Source object.
   @return
      *this.
   */
   pipe & operator=(pipe && pp) {
      read_end = _std::move(pp.read_end);
      write_end = _std::move(pp.write_end);
      return *this;
   }

public:
   //! Read end.
   _std::shared_ptr<file_istream> read_end;
   //! Write end.
   _std::shared_ptr<file_ostream> write_end;
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declaration.
namespace abc { namespace os {

class path;

}} //namespace abc::os

namespace abc { namespace io { namespace binary {

/*! Creates and returns a buffered input stream for the specified unbuffered binary input stream.

@param pbis
   Pointer to an unbuffered binary stream.
@return
   Pointer to a buffered wrapper for *pbis.
*/
ABACLADE_SYM _std::shared_ptr<buffered_istream> buffer_istream(_std::shared_ptr<istream> pbis);

/*! Creates and returns a buffered output stream for the specified unbuffered binary output stream.

@param pbos
   Pointer to an unbuffered binary stream.
@return
   Pointer to a buffered wrapper for *pbos.
*/
ABACLADE_SYM _std::shared_ptr<buffered_ostream> buffer_ostream(_std::shared_ptr<ostream> pbos);

/*! Creates and returns a binary input stream for the specified file descriptor.

@param fd
   File descriptor.
@return
   Pointer to a binary input stream for the file descriptor.
*/
ABACLADE_SYM _std::shared_ptr<file_istream> make_istream(io::filedesc && fd);

/*! Creates and returns a binary input/output stream for the specified file descriptor.

@param fd
   File descriptor.
@return
   Pointer to a binary input/output stream for the file descriptor.
*/
ABACLADE_SYM _std::shared_ptr<file_iostream> make_iostream(io::filedesc && fd);

/*! Creates and returns a binary output stream for the specified file descriptor.

@param fd
   File descriptor.
@return
   Pointer to a binary output stream for the file descriptor.
*/
ABACLADE_SYM _std::shared_ptr<file_ostream> make_ostream(io::filedesc && fd);

/*! Opens a file for binary access.

@param op
   Path to the file.
@param am
   Desired access mode.
@param bBypassCache
   If true, the OS will not cache any portion of the file; if false, accesses to the file will be
   backed by the OS file cache subsystem.
@return
   Pointer to a binary stream for the file.
*/
ABACLADE_SYM _std::shared_ptr<file_stream> open(
   os::path const & op, access_mode am, bool bBypassCache = false
);

/*! Opens a file for binary reading.

@param op
   Path to the file.
@param bBypassCache
   If true, the OS will not cache any portion of the file; if false, accesses to the file will be
   backed by the OS file cache subsystem.
@return
   Pointer to a binary input stream for the file.
*/
inline _std::shared_ptr<file_istream> open_istream(os::path const & op, bool bBypassCache = false) {
   return _std::dynamic_pointer_cast<file_istream>(open(op, access_mode::read, bBypassCache));
}

/*! Opens a file for binary writing.

@param op
   Path to the file.
@param bBypassCache
   If true, the OS will not cache any portion of the file; if false, accesses to the file will be
   backed by the OS file cache subsystem.
@return
   Pointer to a binary output stream for the file.
*/
inline _std::shared_ptr<file_ostream> open_ostream(os::path const & op, bool bBypassCache = false) {
   return _std::dynamic_pointer_cast<file_ostream>(open(op, access_mode::write, bBypassCache));
}

/*! Opens a file for binary reading and writing.

@param op
   Path to the file.
@param bBypassCache
   If true, the OS will not cache any portion of the file; if false, accesses to the file will be
   backed by the OS file cache subsystem.
@return
   Pointer to a binary input/output stream for the file.
*/
inline _std::shared_ptr<file_iostream> open_iostream(
   os::path const & op, bool bBypassCache = false
) {
   return _std::dynamic_pointer_cast<file_iostream>(open(op, access_mode::write, bBypassCache));
}

//! Binary stream associated to the standard error output file.
extern ABACLADE_SYM _std::shared_ptr<ostream> stderr;
//! Binary stream associated to the standard input file.
extern ABACLADE_SYM _std::shared_ptr<istream> stdin;
//! Binary stream associated to the standard output file.
extern ABACLADE_SYM _std::shared_ptr<ostream> stdout;

}}} //namespace abc::io::binary

namespace abc { namespace io { namespace binary { namespace _pvt {

/*! Creates and returns a binary stream associated to the standard error output file (stderr).

@return
   Standard error file.
*/
ABACLADE_SYM _std::shared_ptr<ostream> make_stderr();

/*! Creates and returns a binary stream associated to the standard input file (stdin).

@return
   Standard input file.
*/
ABACLADE_SYM _std::shared_ptr<istream> make_stdin();

/*! Creates and returns a binary stream associated to the standard output file (stdout).

@return
   Standard output file.
*/
ABACLADE_SYM _std::shared_ptr<ostream> make_stdout();

}}}} //namespace abc::io::binary::_pvt

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_IO_BINARY_HXX
