/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_IO_BINARY_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_IO_BINARY_HXX
#endif

#ifndef _LOFTY_IO_BINARY_HXX_NOPUB
#define _LOFTY_IO_BINARY_HXX_NOPUB

#include <lofty/io.hxx>
#include <lofty/noncopyable.hxx>
#include <lofty/_std/memory.hxx>
#include <lofty/_std/type_traits.hxx>
#include <lofty/_std/utility.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io {

//! Classes and functions to perform I/O in binary mode (raw bytes).
namespace binary {}

}} //namespace lofty::io

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

//! Holds a reference to a range in a buffer.
template <typename T>
struct buffer_range {
   //! Pointer to the first element in the range.
   T * ptr;
   //! Size of the array pointed to by ptr.
   std::size_t size;

   /*! Constructor.

   @param ptr_
      Pointer to the first element in the range.
   @param size_
      Size of the array pointed to by ptr_.
   */
   buffer_range(T * ptr_, std::size_t size_) :
      ptr(ptr_),
      size(size_) {
   }

   /*! Constructor converting from a raw bytes range.

   @param bytes
      Range, in bytes.
   */
   buffer_range(buffer_range<
      typename _std::_LOFTY_PUBNS conditional<_std::_LOFTY_PUBNS is_const<T>::value, void const, void>::type
   > const & bytes) :
      ptr(static_cast<T *>(bytes.ptr)),
      size(bytes.size / sizeof(
         typename _std::_pub::conditional<_std::_pub::is_void<T>::value, std::int8_t, T>::type
      )) {
   }
};

_LOFTY_PUBNS_END
}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

//! Base interface for binary (non-text) streams.
class LOFTY_SYM stream {
public:
   //! Destructor. Needed to make the class polymorphic (have a vtable).
   virtual ~stream();

protected:
   //! Default constructor.
   stream();
};

_LOFTY_PUBNS_END
}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

//! Interface for binary (non-text) input streams.
class LOFTY_SYM istream : public virtual stream {
public:
   //! Destructor.
   virtual ~istream();

   /*! Reads one or more elements.

   @param dst
      Address of the destination array.
   @param dst_max
      Maximum number of elements that can be stored in the destination array.
   @return
      Count of elements read. For non-zero values of dst_max, a return value of 0 indicates that the end of the
      data (EOF) was reached.
   */
   template <typename T>
   std::size_t read(T * dst, std::size_t dst_max = 1) {
      return read_bytes(dst, sizeof(T) * dst_max) / sizeof(T);
   }

   /*! Reads at most dst_max bytes.

   @param dst
      Address of the destination buffer.
   @param dst_max
      Size of the destination buffer, in bytes.
   @return
      Count of bytes read. For non-zero values of dst_max, a return value of 0 indicates that the end of the
      data (EOF) was reached.
   */
   virtual std::size_t read_bytes(void * dst, std::size_t dst_max) = 0;

protected:
   //! Default constructor.
   istream();
};

_LOFTY_PUBNS_END
}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

//! Interface for binary (non-text) output streams.
class LOFTY_SYM ostream : public virtual stream {
public:
   //! Destructor.
   virtual ~ostream();

   //! Forces writing any data in the write buffer.
   virtual void flush() = 0;

   /*! Writes an element.

   @param src
      Address of the source element.
   */
   template <typename T>
   void write(T const & src) {
      write(&src, 1);
   }

   /*! Writes an array of elements.

   @param src
      Address of the source array.
   @param src_size
      Count of elements in the source array.
   @return
      Count of elements written.
   */
   template <typename T>
   std::size_t write(T const * src, std::size_t src_size) {
      return write_bytes(src, sizeof(T) * src_size) / sizeof(T);
   }

   /*! Writes an array of bytes.

   @param src
      Address of the source buffer.
   @param src_size
      Size of the source buffer, in bytes.
   @return
      Count of bytes written.
   */
   virtual std::size_t write_bytes(void const * src, std::size_t src_size) = 0;

protected:
   //! Default constructor.
   ostream();
};

_LOFTY_PUBNS_END
}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

//! Interface for binary streams that allow random access (e.g. seek/tell operations).
class LOFTY_SYM seekable {
public:
   /*! Changes the current read/write position.

   @param offset
      New position, relative to whence.
   @param whence
      Indicates what position offset is relative to.
   @return
      Resulting position.
   */
   virtual io::_LOFTY_PUBNS offset_t seek(
      io::_LOFTY_PUBNS offset_t offset, io::_LOFTY_PUBNS seek_from whence
   ) = 0;

   /*! Returns the current read/write position.

   @return
      Current position.
   */
   virtual io::_LOFTY_PUBNS offset_t tell() const = 0;
};

_LOFTY_PUBNS_END
}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

//! Interface for binary streams that access data with a known size.
class LOFTY_SYM sized {
public:
   /*! Returns the size of the data.

   @return
      Data size, in bytes.
   */
   virtual io::_LOFTY_PUBNS full_size_t size() const = 0;
};

_LOFTY_PUBNS_END
}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

//! Interface for buffered streams that wrap binary streams.
class LOFTY_SYM buffered_stream : public virtual stream {
public:
   //! Destructor.
   virtual ~buffered_stream();

   /*! Returns a pointer to the wrapped unbuffered binary stream.

   @return
      Pointer to a unbuffered binary stream.
   */
   _std::_LOFTY_PUBNS shared_ptr<stream> unbuffered() const {
      return _unbuffered_stream();
   }

protected:
   //! Default constructor.
   buffered_stream();

   /*! Implementation of unbuffered(). This enables unbuffered() to be non-virtual, which in turn allows
   derived classes to override it changing its return type to be more specific.

   @return
      Pointer to a unbuffered binary stream.
   */
   virtual _std::_LOFTY_PUBNS shared_ptr<stream> _unbuffered_stream() const = 0;
};

_LOFTY_PUBNS_END
}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

//! Interface for buffered input streams that wrap binary input streams.
class LOFTY_SYM buffered_istream : public virtual buffered_stream, public istream {
public:
   //! Destructor.
   virtual ~buffered_istream();

   /*! Marks the specified amount of bytes as read, so that they won’t be presented again on the next peek()
   call.

   @param count
      Count of elements to mark as read.
   */
   template <typename T>
   void consume(std::size_t count) {
      return consume_bytes(sizeof(T) * count);
   }

   /*! Non-template implementation of consume(). See consume().

   @param count
      Count of bytes to mark as read.
   */
   virtual void consume_bytes(std::size_t count) = 0;

   /*! Returns a view of the internal read buffer, performing zero or more reads from the underlying binary
   input stream.

   @param count
      Count of items to peek. If greater than the size of the read buffer’s contents, additional reads from
      the underlying binary stream will be made, adding to the contents of the read buffer; if the internal
      buffer is not large enough to hold the cumulative data, it will be enlarged.
   @return
      Object that points to the portion of internal buffer that holds the read data, and the count of elements
      read. The latter may be less than the count argument if EOF is reached, or greater than the count
      argument if the buffer was filled more than requested. For non-zero values of the count argument,
      return.size == 0 elements indicates that no more data is available (EOF).
   */
   template <typename T>
   buffer_range<T const> peek(std::size_t count = 1) {
      return buffer_range<T const>(peek_bytes(sizeof(T) * count));
   }

   /*! Non-template implementation of peek(). See peek().

   @param count
      Count of bytes to peek.
   @return
      Peeked data.
   */
   virtual buffer_range<void const> peek_bytes(std::size_t count) = 0;

   /*! Reads at most dst_max bytes. Using peek()/consume() or peek_bytes()/consume_bytes() is preferred to
   calling this method, because it will spare the caller from having to allocate an intermediate buffer.

   @param dst
      Address of the destination buffer.
   @param dst_max
      Size of the destination buffer, in bytes.
   @return
      Count of bytes read. For non-zero values of dst_max, a return value of 0 indicates that the end of the
      data (EOF) was reached.
   */
   virtual std::size_t read_bytes(void * dst, std::size_t dst_max) override;

   //! See buffered_stream::unbuffered().
   _std::_LOFTY_PUBNS shared_ptr<istream> unbuffered() const {
      return _std::_pub::dynamic_pointer_cast<istream>(_unbuffered_stream());
   }

protected:
   //! Default constructor.
   buffered_istream();
};

_LOFTY_PUBNS_END
}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

//! Interface for buffered output streams that wrap binary output streams.
class LOFTY_SYM buffered_ostream :
   public virtual buffered_stream,
   public ostream,
   public io::_LOFTY_PUBNS closeable {
public:
   //! Destructor.
   virtual ~buffered_ostream();

   /*! Commits (writes) any pending buffer blocks returned by get_buffer().

   @param count
      Count of elements to commit.
   */
   template <typename T>
   void commit(std::size_t count) {
      commit_bytes(sizeof(T) * count);
   }

   /*! Non-template, byte-oriented implementation of commit(). See commit().

   @param count
      Count of bytes to commit.
   */
   virtual void commit_bytes(std::size_t count) = 0;

   /*! Returns a buffer large enough to store at least the specified number of elements.

   @param count
      Count of elements to create buffer space for.
   @return
      An object containing a pointer to the portion of internal buffer that the caller can write to, and the
      count of elements contained in the portion of internal buffer; the latter may be more than the requested
      number.
   */
   template <typename T>
   buffer_range<T> get_buffer(std::size_t count) {
      return buffer_range<T>(get_buffer_bytes(sizeof(T) * count));
   }

   /*! Byte-oriented implementation of get_buffer(). See get_buffer().

   @param count
      Count of bytes to create buffer space for.
   @return
      Available buffer range.
   */
   virtual buffer_range<void> get_buffer_bytes(std::size_t count) = 0;

   //! See buffered_stream::unbuffered().
   _std::_LOFTY_PUBNS shared_ptr<ostream> unbuffered() const {
      return _std::_pub::dynamic_pointer_cast<ostream>(_unbuffered_stream());
   }

   /*! Writes an array of bytes. Using get_buffer()/commit() or get_buffer_bytes()/commit_bytes() is preferred
   to calling this method, because it will spare the caller from having to allocate an intermediate buffer.

   @param src
      Address of the source buffer.
   @param src_size
      Size of the source buffer, in bytes.
   @return
      Count of bytes written.
   */
   virtual std::size_t write_bytes(void const * src, std::size_t src_size) override;

protected:
   //! Default constructor.
   buffered_ostream();
};

_LOFTY_PUBNS_END
}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary { namespace _pvt {

/*! Data collected by open() used to construct a file instance. Only defined in the private header of the same
name. */
struct file_init_data;

}}}}

namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

//! Base for file binary streams.
class LOFTY_SYM file_stream : public virtual stream, public lofty::_LOFTY_PUBNS noncopyable {
public:
   //! Destructor.
   virtual ~file_stream();

protected:
   /*! Constructor.

   @param init_data
      Data used to initialize the object, as set by lofty::io::binary::open() and other functions.
   */
   file_stream(_pvt::file_init_data * init_data);

protected:
   //! Descriptor of the underlying file.
   io::_LOFTY_PUBNS filedesc fd;
};

_LOFTY_PUBNS_END
}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

//! Binary file input stream.
class LOFTY_SYM file_istream : public virtual file_stream, public istream {
public:
   //! See file_stream::file_stream().
   file_istream(_pvt::file_init_data * init_data);

   //! Destructor.
   virtual ~file_istream();

   //! See istream::read_bytes().
   virtual std::size_t read_bytes(void * dst, std::size_t dst_max) override;

protected:
#if LOFTY_HOST_API_WIN32
   /*! Detects EOF conditions and real errors. Necessary because under Win32 there are major differences in
   detection of EOF depending on the file type.

   @param bytes_read
      Count of bytes read by ::ReadFile().
   @param err
      Value returned by ::GetLastError() if ::ReadFile() returned false, or ERROR_SUCCESS otherwise.
   @return
      true if ::ReadFile() indicated that EOF was reached, or false otherwise. Exceptions are thrown for all
      non-EOF error conditions.
   */
   virtual bool check_if_eof_or_throw_os_error(::DWORD bytes_read, ::DWORD err) const;
#endif
};

_LOFTY_PUBNS_END
}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

//! Binary file output stream.
class LOFTY_SYM file_ostream : public virtual file_stream, public ostream, public io::_LOFTY_PUBNS closeable {
public:
   //! See ostream::ostream().
   file_ostream(_pvt::file_init_data * init_data);

   //! Destructor.
   virtual ~file_ostream();

   //! See closeable::close().
   virtual void close() override;

   //! See ostream::flush().
   virtual void flush() override;

   //! See ostream::write_bytes().
   virtual std::size_t write_bytes(void const * src, std::size_t src_size) override;
};

_LOFTY_PUBNS_END
}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

//! Bidirectional binary file stream.
class LOFTY_SYM file_iostream : public virtual file_istream, public virtual file_ostream {
public:
   //! See file_istream::file_istream() and file_ostream::file_ostream().
   file_iostream(_pvt::file_init_data * init_data);

   //! Destructor.
   virtual ~file_iostream();
};

_LOFTY_PUBNS_END
}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

//! Unidirectional pipe (FIFO), consisting in a read end stream and a write end stream.
class LOFTY_SYM pipe {
public:
   //! Default constructor.
   pipe();

   /*! Move constructor.

   @param src
      Source object.
   */
   pipe(pipe && src) :
      read_end(_std::_pub::move(src.read_end)),
      write_end(_std::_pub::move(src.write_end)) {
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   pipe & operator=(pipe && src) {
      read_end = _std::_pub::move(src.read_end);
      write_end = _std::_pub::move(src.write_end);
      return *this;
   }

public:
   //! Read end.
   _std::_LOFTY_PUBNS shared_ptr<file_istream> read_end;
   //! Write end.
   _std::_LOFTY_PUBNS shared_ptr<file_ostream> write_end;
};

_LOFTY_PUBNS_END
}}} //namespace lofty::io::binary

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declaration.
namespace lofty { namespace os {
_LOFTY_PUBNS_BEGIN

class path;

_LOFTY_PUBNS_END
}}

namespace lofty { namespace io { namespace binary {
_LOFTY_PUBNS_BEGIN

/*! Creates and returns a buffered input stream for the specified unbuffered binary input stream.

@param bin_istream
   Pointer to an unbuffered binary stream.
@return
   Pointer to a buffered wrapper for *bin_istream.
*/
LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<buffered_istream> buffer_istream(
   _std::_LOFTY_PUBNS shared_ptr<istream> bin_istream
);

/*! Creates and returns a buffered output stream for the specified unbuffered binary output stream.

@param bin_ostream
   Pointer to an unbuffered binary stream.
@return
   Pointer to a buffered wrapper for *bin_ostream.
*/
LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<buffered_ostream> buffer_ostream(
   _std::_LOFTY_PUBNS shared_ptr<ostream> bin_ostream
);

/*! Creates and returns a binary input stream for the specified file descriptor.

@param fd
   File descriptor.
@return
   Pointer to a binary input stream for the file descriptor.
*/
LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<file_istream> make_istream(io::_LOFTY_PUBNS filedesc && fd);

/*! Creates and returns a binary input/output stream for the specified file descriptor.

@param fd
   File descriptor.
@return
   Pointer to a binary input/output stream for the file descriptor.
*/
LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<file_iostream> make_iostream(io::_LOFTY_PUBNS filedesc && fd);

/*! Creates and returns a binary output stream for the specified file descriptor.

@param fd
   File descriptor.
@return
   Pointer to a binary output stream for the file descriptor.
*/
LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<file_ostream> make_ostream(io::_LOFTY_PUBNS filedesc && fd);

/*! Opens a file for binary access.

@param path
   Path to the file.
@param mode
   Desired access mode.
@param bypass_cache
   If true, the OS will not cache any portion of the file; if false, accesses to the file will be backed by
   the OS file cache subsystem.
@return
   Pointer to a binary stream for the file.
*/
LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<file_stream> open(
   os::_LOFTY_PUBNS path const & path, io::_LOFTY_PUBNS access_mode mode, bool bypass_cache = false
);

/*! Opens a file for binary reading.

@param path
   Path to the file.
@param bypass_cache
   If true, the OS will not cache any portion of the file; if false, accesses to the file will be backed by
   the OS file cache subsystem.
@return
   Pointer to a binary input stream for the file.
*/
inline _std::_LOFTY_PUBNS shared_ptr<file_istream> open_istream(
   os::_LOFTY_PUBNS path const & path, bool bypass_cache = false
) {
   return _std::_pub::dynamic_pointer_cast<file_istream>(
      open(path, io::_LOFTY_PUBNS access_mode::read, bypass_cache)
   );
}

/*! Opens a file for binary writing.

@param path
   Path to the file.
@param bypass_cache
   If true, the OS will not cache any portion of the file; if false, accesses to the file will be backed by
   the OS file cache subsystem.
@return
   Pointer to a binary output stream for the file.
*/
inline _std::_LOFTY_PUBNS shared_ptr<file_ostream> open_ostream(
   os::_LOFTY_PUBNS path const & path, bool bypass_cache = false
) {
   return _std::_pub::dynamic_pointer_cast<file_ostream>(
      open(path, io::_LOFTY_PUBNS access_mode::write, bypass_cache)
   );
}

/*! Opens a file for binary reading and writing.

@param path
   Path to the file.
@param bypass_cache
   If true, the OS will not cache any portion of the file; if false, accesses to the file will be backed by
   the OS file cache subsystem.
@return
   Pointer to a binary input/output stream for the file.
*/
inline _std::_LOFTY_PUBNS shared_ptr<file_iostream> open_iostream(
   os::_LOFTY_PUBNS path const & path, bool bypass_cache = false
) {
   return _std::_pub::dynamic_pointer_cast<file_iostream>(
      open(path, io::_LOFTY_PUBNS access_mode::read_write, bypass_cache)
   );
}

//! Binary stream associated to the standard error output file.
extern LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<ostream> stderr;
//! Binary stream associated to the standard input file.
extern LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<istream> stdin;
//! Binary stream associated to the standard output file.
extern LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<ostream> stdout;

_LOFTY_PUBNS_END
}}} //namespace lofty::io::binary

namespace lofty { namespace io { namespace binary { namespace _pvt {

/*! Creates and returns a binary stream associated to the standard error output file (stderr).

@return
   Standard error file.
*/
LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<_LOFTY_PUBNS ostream> make_stderr();

/*! Creates and returns a binary stream associated to the standard input file (stdin).

@return
   Standard input file.
*/
LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<_LOFTY_PUBNS istream> make_stdin();

/*! Creates and returns a binary stream associated to the standard output file (stdout).

@return
   Standard output file.
*/
LOFTY_SYM _std::_LOFTY_PUBNS shared_ptr<_LOFTY_PUBNS ostream> make_stdout();

}}}} //namespace lofty::io::binary::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_IO_BINARY_HXX_NOPUB

#ifdef _LOFTY_IO_BINARY_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace io { namespace binary {

   using _pub::buffer_istream;
   using _pub::buffer_ostream;
   using _pub::buffer_range;
   using _pub::buffered_istream;
   using _pub::buffered_ostream;
   using _pub::buffered_stream;
   using _pub::file_iostream;
   using _pub::file_istream;
   using _pub::file_ostream;
   using _pub::file_stream;
   using _pub::istream;
   using _pub::make_istream;
   using _pub::make_iostream;
   using _pub::make_ostream;
   using _pub::open;
   using _pub::open_istream;
   using _pub::open_ostream;
   using _pub::open_iostream;
   using _pub::ostream;
   using _pub::pipe;
   using _pub::seekable;
   using _pub::sized;
   using _pub::stderr;
   using _pub::stdin;
   using _pub::stdout;
   using _pub::stream;

   }}}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_IO_BINARY_HXX
