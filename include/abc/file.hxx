/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

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
class filedesc :
	public support_explicit_operator_bool<filedesc> {

	ABC_CLASS_PREVENT_COPYING(filedesc)

public:

	/** Constructor.

	TODO: comment signature.
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

	TODO: comment signature.
	*/
	filedesc & operator=(filedesc_t fd);
	filedesc & operator=(filedesc && fd);


	/** Safe bool operator.

	TODO: comment signature.
	*/
	explicit_operator_bool() const {
		return m_fd != smc_fdNull;
	}


	/** Returns the wrapped file descriptor.

	TODO: comment signature.
	*/
	filedesc_t get() const {
		return m_fd;
	}


	/** Yields ownership over the wrapped file descriptor, returning it.

	TODO: comment signature.
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

/** Wrapper for the OS’s native file API.
*/
class file {

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

	TODO: comment signature.
	*/
	explicit file(filedesc && fd);
	file(file_path const & fp, access_mode fam, bool bBuffered = true);


	/** Ensures that no write buffers contain any data.
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


	/** Reads at most cbMax bytes from the file.

	p
		Address of the destination buffer.
	cbMax
		Size of the destination buffer, in bytes.
	return
		Count of bytes read. For non-zero values of cb, a return value of 0 indicates that the end of
		the file was reached.
	*/
	size_t read(void * p, size_t cbMax);


	/** Returns the physical alignment for unbuffered/direct disk access.

	return
		Alignment boundary, in bytes.
	*/
	unsigned physical_alignment() const {
		return m_cbPhysAlign;
	}


	/** Returns the computed size of the file, if applicable, or 0 otherwise.

	return
		Size of the file, in bytes.
	*/
	fileint_t size() const {
		return m_cb;
	}


	/** Writes an array of bytes to the file.

	p
		Address of the source buffer.
	cb
		Size of the source buffer, in bytes.
	return
		Count of bytes written.
	*/
	size_t write(void const * p, size_t cb);


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


private:

	/** Initializes a standard file.

	TODO: comment signature.
	*/
	static void _construct_std_file(filedesc_t fd, std::shared_ptr<file> ** pppf);


	/** Opens a file, with the desired access mode. It touches member variables, but not m_fd.

	TODO: comment signature.
	*/
	filedesc _open(file_path const & fp, access_mode fam);


	/** Performs initialization to be done after having obtained a valid file descriptor. It touches
	member variables, but not m_fd.

	TODO: comment signature.
	*/
	filedesc _post_open(filedesc && fd);


	/** Releases any objects constructed by _construct_std_file().
	*/
	static void ABC_STL_CALLCONV _release_std_files();


private:

	/** Computed size of the file. */
	fileint_t m_cb;
	/** Physical alignment for unbuffered/direct disk access. */
	unsigned m_cbPhysAlign;
	/** If true, the file has a defined size, which is stored in m_cb. */
	bool m_bHasSize:1;
	/** If true, the OS is buffering reads/writes to m_fd; otherwise, use of m_cbPhysAlign is
	enforced. */
	bool m_bBuffered:1;
#if ABC_HOST_API_WIN32
	/** If true, write() will emulate POSIX’s O_APPEND. */
	bool m_bAppend:1;
#endif
	/** Descriptor of the underlying file. */
	filedesc m_fd;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_FILE_HXX

