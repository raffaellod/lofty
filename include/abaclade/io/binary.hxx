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
// abc::io::binary globals

namespace abc {
namespace io {
namespace binary {

namespace detail {

/*! Data collected by open() used to construct a file instance. This is only defined in file.cxx,
after the necessary header files have been included. */
struct file_init_data;

} //namespace detail

// Forward declarations.
class file_base;
class file_reader;
class file_writer;

/*! Returns the binary writer associated to the standard error output file (stderr).

@return
   Standard error file.
*/
ABACLADE_SYM std::shared_ptr<file_writer> stderr();

/*! Returns the binary reader associated to the standard input file (stdin).

@return
   Standard input file.
*/
ABACLADE_SYM std::shared_ptr<file_reader> stdin();

/*! Returns the binary writer associated to the standard output file (stdout).

@return
   Standard output file.
*/
ABACLADE_SYM std::shared_ptr<file_writer> stdout();

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
std::shared_ptr<file_base> open(os::path const & op, access_mode am, bool bBypassCache = false);

/*! Opens a file for binary reading.

@param op
   Path to the file.
@param bBypassCache
   If true, the OS will not cache any portion of the file; if false, accesses to the file will be
   backed by the OS file cache subsystem.
@return
   Pointer to a binary reader for the file.
*/
inline std::shared_ptr<file_reader> open_reader(os::path const & op, bool bBypassCache = false) {
   return std::dynamic_pointer_cast<file_reader>(open(op, access_mode::read, bBypassCache));
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
inline std::shared_ptr<file_writer> open_writer(os::path const & op, bool bBypassCache = false) {
   return std::dynamic_pointer_cast<file_writer>(open(op, access_mode::write, bBypassCache));
}

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::base

namespace abc {
namespace io {
namespace binary {

//! Base interface for binary (non-text) I/O.
class ABACLADE_SYM base {
public:
   //! Destructor. Also needed to make the class polymorphic (have a vtable).
   virtual ~base();
};

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::reader

namespace abc {
namespace io {
namespace binary {

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
      of the data was reached.
   */
   virtual std::size_t read(void * p, std::size_t cbMax) = 0;
};

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::writer

namespace abc {
namespace io {
namespace binary {

//! Interface for binary (non-text) output.
class ABACLADE_SYM writer : public virtual base {
public:
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

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::seekable

namespace abc {
namespace io {
namespace binary {

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

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::sized

namespace abc {
namespace io {
namespace binary {

//! Interface for binary I/O classes that access data with a known size.
class ABACLADE_SYM sized {
public:
   /*! Returns the size of the data.

   @return
      Data size, in bytes.
   */
   virtual full_size_t size() const = 0;
};

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::file_base

namespace abc {
namespace io {
namespace binary {

//! Base for file binary I/O classes.
class ABACLADE_SYM file_base : public virtual base, public noncopyable {
public:
   //! Destructor.
   virtual ~file_base();

protected:
   /*! Constructor.

   @param pfid
      Data used to initialize the object, as set by abc::io::open() and other functions.
   */
   file_base(detail::file_init_data * pfid);

protected:
   //! Descriptor of the underlying file.
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

//! Binary file input.
class ABACLADE_SYM file_reader : public virtual file_base, public reader {
public:
   //! See file_base::file_base().
   file_reader(detail::file_init_data * pfid);

   //! Destructor.
   virtual ~file_reader();

   //! See reader::read().
   virtual std::size_t read(void * p, std::size_t cbMax) override;

#if ABC_HOST_API_WIN32
   // Under Win32 there are major differences in detection of EOF depending on the file type.

   /*! Detects EOF conditions and real errors.

   @param cchRead
      Count of bytes read by ::ReadFile().
   @param iErr
      Value returned by ::GetLastError() if ::ReadFile() returned false, or ERROR_SUCCESS otherwise.
   @return
      true if ::ReadFile() indicated that EOF was reached, or false otherwise. Exceptions are
      thrown for all non-EOF error conditions.
   */
   virtual bool readfile_returned_eof(DWORD cchRead, DWORD iErr) const;
#endif //if ABC_HOST_API_WIN32
};

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::file_writer

namespace abc {
namespace io {
namespace binary {

//! Binary file output.
class ABACLADE_SYM file_writer : public virtual file_base, public writer {
public:
   //! See writer::writer().
   file_writer(detail::file_init_data * pfid);

   //! Destructor.
   virtual ~file_writer();

   //! See writer::flush().
   virtual void flush() override;

   //! See writer::write().
   virtual std::size_t write(void const * p, std::size_t cb) override;
};

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::console_file_base

namespace abc {
namespace io {
namespace binary {

//! Base for console/terminal binary I/O classes.
class ABACLADE_SYM console_file_base : public virtual file_base {
public:
   //! Destructor.
   virtual ~console_file_base();

protected:
   //! See file_base::file_base().
   console_file_base(detail::file_init_data * pfid);
};

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::console_reader

namespace abc {
namespace io {
namespace binary {

//! Console/terminal input pseudo-file.
class ABACLADE_SYM console_reader : public virtual console_file_base, public file_reader {
public:
   //! See file_reader::file_reader().
   console_reader(detail::file_init_data * pfid);

   //! Destructor.
   virtual ~console_reader();

#if ABC_HOST_API_WIN32
   /* Under Win32, console files must use a dedicated API in order to support the native character
   type. */

   //! See file_reader::read().
   virtual std::size_t read(void * p, std::size_t cbMax) override;
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

//! Console/terminal output pseudo-file.
class ABACLADE_SYM console_writer :
   public virtual console_file_base,
   public file_writer
#if ABC_HOST_API_WIN32
   // Under Win32, ANSI escape sequences parsing is up to us.
   , private abc::text::parsers::ansi_escape_sequences
#endif
   {
public:
   //! See file_writer::file_writer().
   console_writer(detail::file_init_data * pfid);

   //! Destructor.
   virtual ~console_writer();

#if ABC_HOST_API_WIN32
   /* Under Win32, console files must use a dedicated API in order to support the native character
   type. */

   //! See file_writer::write().
   virtual std::size_t write(void const * p, std::size_t cb) override;

private:
   //! See abc::text::parsers::ansi_escape_sequences::clear_display_area().
   virtual void clear_display_area(std::int16_t iRow, std::int16_t iCol, std::size_t cch) override;

   //! See abc::text::parsers::ansi_escape_sequences::get_cursor_pos_and_display_size().
   virtual void get_cursor_pos_and_display_size(
      std::int16_t * piRow, std::int16_t * piCol, std::int16_t * pcRows, std::int16_t * pcCols
   ) override;

   /* Determines whether output processing is enabled for the console pseudo-file.

   @return
      true if the bytes written are to be parsed for special characters, or false otherwise.
   */
   bool processing_enabled() const;

   //! See abc::text::parsers::ansi_escape_sequences::scroll_text().
   virtual void scroll_text(std::int16_t cRows, std::int16_t cCols) override;

   //! See abc::text::parsers::ansi_escape_sequences::set_char_attributes().
   virtual void set_char_attributes() override;

   //! See abc::text::parsers::ansi_escape_sequences::set_cursor_pos().
   virtual void set_cursor_pos(std::int16_t iRow, std::int16_t iCol) override;

   //! See abc::text::parsers::ansi_escape_sequences::set_cursor_visibility().
   virtual void set_cursor_visibility(bool bVisible) override;

   //! See abc::text::parsers::ansi_escape_sequences::set_window_title().
   virtual void set_window_title(istr const & sTitle) override;

   /*! Writes a range of characters directly to the console, without any parsing.

   @param pchBegin
      Start of the character array to write.
   @param pchEnd
      End of the character array to write.
   */
   void write_range(char_t const * pchBegin, char_t const * pchEnd) const;

private:
   //! Mapping table from ANSI terminal colors to Win32 console background colors.
   static WORD const smc_aiAnsiColorToBackgroundColor[];
   //! Mapping table from ANSI terminal colors to Win32 console foreground colors.
   static WORD const smc_aiAnsiColorToForegroundColor[];
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

//! Binary reader for the output end of a pipe.
class ABACLADE_SYM pipe_reader : public file_reader {
public:
   //! See file_reader::file_reader().
   pipe_reader(detail::file_init_data * pfid);

   //! Destructor.
   virtual ~pipe_reader();

#if ABC_HOST_API_WIN32
   /*! See file_reader::readfile_returned_eof(). Pipes report EOF in a completely different way than
   regular files. */
   virtual bool readfile_returned_eof(DWORD cchRead, DWORD iErr) const override;
#endif
};

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::pipe_writer

namespace abc {
namespace io {
namespace binary {

//! Binary writer for the input end of a pipe.
class ABACLADE_SYM pipe_writer : public file_writer {
public:
   //! See file_writer::file_writer().
   pipe_writer(detail::file_init_data * pfid);

   //! Destructor.
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

//! Base for binary I/O classes for regular disk files.
class ABACLADE_SYM regular_file_base : public virtual file_base, public seekable, public sized {
public:
   //! Destructor.
   virtual ~regular_file_base();

   //! See seekable::seek().
   virtual offset_t seek(offset_t ibOffset, seek_from sfWhence) override;

   //! See sized::size().
   virtual full_size_t size() const override;

   //! See seekable::tell().
   virtual offset_t tell() const override;

protected:
   //! See file_base::file_base().
   regular_file_base(detail::file_init_data * pfid);

protected:
   //! Size of the file.
   full_size_t m_cb;
#if 0
   //! Physical alignment for unbuffered/direct disk access.
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

//! Binary reader for regular disk files.
class ABACLADE_SYM regular_file_reader : public virtual regular_file_base, public file_reader {
public:
   //! See regular_file_base().
   regular_file_reader(detail::file_init_data * pfid);

   //! Destructor.
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

//! Binary writer for regular disk files.
class ABACLADE_SYM regular_file_writer : public virtual regular_file_base, public file_writer {
public:
   //! See regular_file_base().
   regular_file_writer(detail::file_init_data * pfid);

   //! Destructor.
   virtual ~regular_file_writer();

#if ABC_HOST_API_WIN32
   //! See file_writer::write(). This override is necessary to emulate O_APPEND under Win32.
   virtual std::size_t write(void const * p, std::size_t cb) override;

protected:
   //! If true, write() will emulate POSIX’s O_APPEND in platforms that don’t support it.
   bool m_bAppend:1;
#endif
};

} //namespace binary
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
