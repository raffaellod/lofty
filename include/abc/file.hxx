// -*- coding: utf-8; mode: c++; tab-width: 3 -*-
//--------------------------------------------------------------------------------------------------
// Application-Building Components
// Copyright 2010-2013 Raffaello D. Di Napoli
//--------------------------------------------------------------------------------------------------
// This file is part of Application-Building Components (henceforth referred to as ABC).
//
// ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ABC. If not, see
// <http://www.gnu.org/licenses/>.
//--------------------------------------------------------------------------------------------------

#ifndef ABC_FILE_HXX
#define ABC_FILE_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/file_path.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// Declarations

namespace abc {

/// List of standard (OS-provided) files.
ABC_ENUM(stdfile, \
	/** Internal identifier for stdin. */ \
	(stdin,  0), \
	/** Internal identifier for stdout. */ \
	(stdout, 1), \
	/** Internal identifier for stderr. */ \
	(stderr, 2) \
);


/// Native OS file descriptor/handle.
#if ABC_HOST_API_POSIX
	typedef int filedesc_t;
#elif ABC_HOST_API_WIN32
	typedef HANDLE filedesc_t;
#else
	#error TODO-PORT: HOST_API
#endif

/// Integer wide enough to express any valid file offset.
#if ABC_HOST_API_POSIX
	typedef uint64_t fileint_t;
#elif ABC_HOST_API_WIN32
	typedef uint64_t fileint_t;
#else
	#error TODO-PORT: HOST_API
#endif

/// Wrapper for filedesc_t, to implement RAII. Similar in concept to std::unique_ptr, except it
// doesn’t always own the wrapped filedesc_t (e.g. for standard files).
class filedesc;

/// Wrapper for the OS’s native file API.
class file;

} //namespace abc




////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::filedesc


namespace abc {

class filedesc :
	public support_explicit_operator_bool<filedesc> {

	ABC_CLASS_PREVENT_COPYING(filedesc)

public:

	/// Constructor.
	//
	filedesc() :
		m_fd(smc_fdNull), m_bOwn(false) {
	}
	filedesc(filedesc_t fd, bool bOwn = true) :
		m_fd(fd), m_bOwn(bOwn) {
	}
	filedesc(filedesc && fd);

	/// Destructor.
	~filedesc();

	/// Assignment operator.
	filedesc & operator=(filedesc_t fd);
	filedesc & operator=(filedesc && fd);


	/// Safe bool operator.
	//
	explicit_operator_bool() const {
		return m_fd != smc_fdNull;
	}


	/// Returns the wrapped file descriptor.
	//
	filedesc_t get() const {
		return m_fd;
	}


	/// Yields ownership over the wrapped file descriptor, returning it.
	//
	filedesc_t release() {
		filedesc_t fd(m_fd);
		m_fd = smc_fdNull;
		return fd;
	}


private:

	/// The actual descriptor.
	filedesc_t m_fd;
	/// If true, the wrapper will close the file on destruction.
	bool m_bOwn;

	/// Logically null file descriptor.
	static filedesc_t const smc_fdNull =
#if ABC_HOST_API_POSIX
		-1;
#elif ABC_HOST_API_WIN32
		INVALID_HANDLE_VALUE;
#else
	#error TODO-PORT: HOST_API
#endif
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::file


namespace abc {

class file {

	ABC_CLASS_PREVENT_COPYING(file)

public:

	/// File access modes.
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

	/// Constructor.
	explicit file(filedesc && fd);
	file(file_path const & fp, access_mode fam, bool bBuffered = true);

	/// Ensures that no write buffers contain any data.
	void flush();


	/// Returns true if the file has a defined size, which is stored in m_cb.
	//
	bool get_has_size() const {
		return m_bHasSize;
	}


	/// Returns true if the OS is buffering reads/writes to m_fd.
	//
	bool get_buffered() const {
		return m_bBuffered;
	}


	/// Returns the physical alignment for unbuffered/direct disk access.
	//
	unsigned get_physical_alignment() const {
		return m_cbPhysAlign;
	}


	/// Returns the computed size of the file, if applicable, or 0 otherwise.
	//
	fileint_t get_size() const {
		return m_cb;
	}


	/// Reads at most cbMax bytes from the file.
	size_t read(void * p, size_t cbMax);

	/// Writes an array of bytes to the file.
	size_t write(void const * p, size_t cb);

	/// Returns the file associated to the standard error output (stderr).
	static std::shared_ptr<file> const & get_stderr();

	/// Returns the file associated to the standard input (stdin).
	static std::shared_ptr<file> const & get_stdin();

	/// Returns the file associated to the standard output (stdout).
	static std::shared_ptr<file> const & get_stdout();


private:

	/// Initializes a standard file.
	static void _construct_std_file(filedesc_t fd, std::shared_ptr<file> ** pppf);

	/// Opens a file, with the desired access mode. It touches member variables, but not m_fd.
	filedesc _open(file_path const & fp, access_mode fam);

	/// Performs initialization to be done after having obtained a valid file descriptor. It touches
	// member variables, but not m_fd.
	filedesc _post_open(filedesc && fd);

	/// Releases any objects constructed by _construct_std_file().
	static void _release_std_files();


private:

	/// Computed size of the file.
	fileint_t m_cb;
	/// Physical alignment for unbuffered/direct disk access.
	unsigned m_cbPhysAlign;
	/// If true, the file has a defined size, which is stored in m_cb.
	bool m_bHasSize:1;
	/// If true, the OS is buffering reads/writes to m_fd; otherwise, use of m_cbPhysAlign is
	// enforced.
	bool m_bBuffered:1;
#if ABC_HOST_API_WIN32
	/// If true, write() will emulate POSIX’s O_APPEND.
	bool m_bAppend:1;
#endif
	/// Descriptor of the underlying file.
	filedesc m_fd;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_FILE_HXX

