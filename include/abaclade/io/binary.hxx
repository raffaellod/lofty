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

#if ABC_HOST_API_WIN32
   #include <abaclade/text/parsers/ansi_escape_sequences.hxx>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

// Forward declarations.
class pipe_reader;
class pipe_writer;

//! Contains the two ends of a pipe.
struct pipe_ends {
   //! Reader end.
   _std::shared_ptr<pipe_reader> reader;
   //! Writer end.
   _std::shared_ptr<pipe_writer> writer;

   /*! Constructor.

   @param pbprReader
      Reader end.
   @param pbpwWriter
      Writer end.
   */
   pipe_ends(_std::shared_ptr<pipe_reader> pbprReader, _std::shared_ptr<pipe_writer> pbpwWriter) :
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

namespace abc { namespace io { namespace binary {

// Forward declarations.
class file_base;
class file_reader;
class file_writer;
class file_readwriter;
class reader;
class writer;

/*! Creates and returns a binary reader for the specified file descriptor.

@param fd
   File descriptor.
@return
   Pointer to a binary reader for the file descriptor.
*/
ABACLADE_SYM _std::shared_ptr<file_reader> make_reader(io::filedesc && fd);

/*! Creates and returns a binary writer for the specified file descriptor.

@param fd
   File descriptor.
@return
   Pointer to a binary writer for the file descriptor.
*/
ABACLADE_SYM _std::shared_ptr<file_writer> make_writer(io::filedesc && fd);

/*! Creates and returns a binary reader/writer for the specified file descriptor.

@param fd
   File descriptor.
@return
   Pointer to a binary reader/writer for the file descriptor.
*/
ABACLADE_SYM _std::shared_ptr<file_readwriter> make_readwriter(io::filedesc && fd);

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

namespace abc { namespace io { namespace binary {

//! Base interface for binary (non-text) I/O.
class ABACLADE_SYM base {
public:
   //! Destructor. Also needed to make the class polymorphic (have a vtable).
   virtual ~base();
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Interface for binary (non-text) input.
class ABACLADE_SYM reader : public virtual base {
public:
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
};

}}} //namespace abc::io::binary

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary {

//! Interface for binary (non-text) output.
class ABACLADE_SYM writer : public virtual base {
public:
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
