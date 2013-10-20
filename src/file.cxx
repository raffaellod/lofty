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

#include <abc/core.hxx>
#include <abc/file.hxx>
#include <abc/numeric.hxx>
#include <abc/trace.hxx>
#include <algorithm>
#if ABC_HOST_API_POSIX
	#include <unistd.h> // *_FILENO ssize_t close() isatty() open() read() write()
	#include <stdlib.h> // atexit()
	#include <fcntl.h> // O_*
	#include <sys/stat.h> // S_*, stat()
//	#include <sys/mman.h> // mmap(), munmap(), PROT_*, MAP_*
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::filedesc


namespace abc {

filedesc_t const filedesc::smc_fdNull =
#if ABC_HOST_API_POSIX
	-1;
#elif ABC_HOST_API_WIN32
	INVALID_HANDLE_VALUE;
#else
	#error TODO-PORT: HOST_API
#endif


filedesc::filedesc(filedesc && fd) :
	m_fd(fd.m_fd),
	m_bOwn(fd.m_bOwn) {
	fd.m_fd = smc_fdNull;
	fd.m_bOwn = false;
}


filedesc::~filedesc() {
	if (m_bOwn && m_fd != smc_fdNull) {
#if ABC_HOST_API_POSIX
		::close(m_fd);
#elif ABC_HOST_API_WIN32
		::CloseHandle(m_fd);
#else
	#error TODO-PORT: HOST_API
#endif
	}
}


filedesc & filedesc::operator=(filedesc_t fd) {
	if (fd != m_fd) {
		this->~filedesc();
	}
	m_fd = fd;
	m_bOwn = true;
	return *this;
}
filedesc & filedesc::operator=(filedesc && fd) {
	if (fd.m_fd != m_fd) {
		this->~filedesc();
		m_fd = fd.m_fd;
		m_bOwn = fd.m_bOwn;
		fd.m_fd = smc_fdNull;
	}
	return *this;
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::file


namespace abc {

// These should be members of file, but since the file_?stream counterparts couldn’t be members of
// file_stream_base, they are here.
static std::shared_ptr<file> * g_ppfileStdErr(NULL);
static std::shared_ptr<file> * g_ppfileStdIn(NULL);
static std::shared_ptr<file> * g_ppfileStdOut(NULL);


struct _file_init_data {
#if ABC_HOST_API_POSIX
	/** Set by file::_construct_matching_type(). */
	struct ::stat statFile;
#endif
	/** See file::m_fd. To be set before calling file::_construct_matching_type(). */
	filedesc fd;
	/** See file::m_bBuffered. To be set before calling file::_construct_matching_type(). */
	bool bBuffered:1;
#if ABC_HOST_API_WIN32
	/** See regular_file::m_bAppend. To be set before calling file::_construct_matching_type(). */
	bool bAppend:1;
#endif
};


file::file(_file_init_data * pfid) :
	m_fd(std::move(pfid->fd)),
	m_bHasSize(false),
	m_bBuffered(pfid->bBuffered) {
}


/*virtual*/ file::~file() {
}


/*static*/ std::shared_ptr<file> file::attach(filedesc && fd) {
	abc_trace_fn((/*fd*/));

	_file_init_data fid;
	fid.fd = std::move(fd);
	// Since this method is mostly used for standard descriptors, assume that OS buffering is on.
	fid.bBuffered = true;
#if ABC_HOST_API_WIN32
	// Append-mode emulation is specific to ABC, so a file opened by other code cannot have been
	// opened in append mode.
	fid.bAppend = false;
#endif
	return _construct_matching_type(&fid);
}


void file::flush() {
	abc_trace_fn((this));

#if ABC_HOST_API_POSIX
	// TODO: investigate fdatasync().
	if (::fsync(m_fd.get())) {
		throw_os_error();
	}
#elif ABC_HOST_API_WIN32
	if (!::FlushFileBuffers(m_fd.get())) {
		throw_os_error();
	}
#else
	#error TODO-PORT: HOST_API
#endif
}


/*static*/ std::shared_ptr<file> file::open(
	file_path const & fp, access_mode fam, bool bBuffered /*= true*/
) {
	abc_trace_fn((fp, fam, bBuffered));

	_file_init_data fid;
	fid.bBuffered = bBuffered;
#if ABC_HOST_API_POSIX
	int fi;
	switch (fam.base()) {
		default:
		case access_mode::read:
			fi = O_RDONLY;
			break;
		case access_mode::write:
			fi = O_WRONLY | O_CREAT | O_TRUNC;
			break;
		case access_mode::read_write:
			fi = O_RDWR | O_CREAT;
			break;
		case access_mode::append:
			fi = O_APPEND;
			break;
	}
	if (!fid.bBuffered) {
		fi |= O_DIRECT;
	}
	fid.fd = ::open(fp.data(), fi, 0666);
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
	DWORD fiAccess, fiShareMode, iAction, fi(FILE_ATTRIBUTE_NORMAL);
	fid.bAppend = false;
	switch (fam.base()) {
		case access_mode::read:
			fiAccess = GENERIC_READ;
			fiShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
			iAction = OPEN_EXISTING;
			break;
		case access_mode::write:
			fiAccess = GENERIC_WRITE;
			fiShareMode = FILE_SHARE_READ;
			iAction = CREATE_ALWAYS;
			break;
		case access_mode::read_write:
			fiAccess = GENERIC_READ | GENERIC_WRITE;
			fiShareMode = FILE_SHARE_READ;
			iAction = OPEN_ALWAYS;
			break;
		case access_mode::append:
			// This combination is FILE_GENERIC_WRITE & ~FILE_WRITE_DATA; MSDN states that “for local
			// files, write operations will not overwrite existing data”. Requiring fewer permissions,
			// this also allows ::CreateFile() to succeed on files with stricter ACLs.
			fiAccess = FILE_APPEND_DATA | FILE_WRITE_ATTRIBUTES | STANDARD_RIGHTS_WRITE | SYNCHRONIZE;
			fiShareMode = FILE_SHARE_READ;
			iAction = OPEN_ALWAYS;
			fid.bAppend = true;
			break;
		no_default;
	}
	if (!m_bBuffered) {
		fi |= FILE_FLAG_NO_BUFFERING;
	} else if (fiAccess & GENERIC_READ) {
		fi |= FILE_FLAG_SEQUENTIAL_SCAN;
	}
	fid.fd = ::CreateFile(fp.data(), fiAccess, fiShareMode, NULL, iAction, fi, NULL);
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
	#error TODO-PORT: HOST_API
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
	if (!fid.fd) {
		throw_os_error();
	}
	return _construct_matching_type(&fid);
}


/*virtual*/ unsigned file::physical_alignment() const {
	return 0;
}


/*virtual*/ size_t file::read(void * p, size_t cbMax) {
	abc_trace_fn((this, p, cbMax));

	int8_t * pb(static_cast<int8_t *>(p));
	// The top half of this loop is OS-specific; the rest is generalized. As a guideline, the OS
	// read()-equivalent function is invoked at least once, so we give it a chance to report any
	// errors, instead of masking them by skipping the call (e.g. due to cbMax == 0 on input).
	do {
#if ABC_HOST_API_POSIX
		// This will be repeated at most three times, just to break a size_t-sized block down into
		// ssize_t-sized blocks.
		ssize_t cbLastRead(::read(
			m_fd.get(), pb, std::min<size_t>(cbMax, numeric::max<ssize_t>::value)
		));
		if (cbLastRead == 0) {
			// EOF.
			break;
		}
		if (cbLastRead < 0) {
			throw_os_error();
		}
#elif ABC_HOST_API_WIN32
		// This will be repeated at least once, and as long as we still have some bytes to read, and
		// reading them does not fail.
		DWORD cbLastRead;
		if (!::ReadFile(
			m_fd.get(), pb,
			DWORD(std::min<size_t>(cbMax, numeric::max<DWORD>::value)), &cbLastRead, NULL
		)) {
			DWORD iErr(::GetLastError());
			if (iErr == ERROR_HANDLE_EOF) {
				break;
			}
			throw_os_error(iErr);
		}
#else
	#error TODO-PORT: HOST_API
#endif
		// Some bytes were read; prepare for the next attempt.
		pb += cbLastRead;
		cbMax -= size_t(cbLastRead);
	} while (cbMax);

	return size_t(pb - static_cast<int8_t *>(p));
}


/*virtual*/ fileint_t file::size() const {
	return 0;
}


/*static*/ std::shared_ptr<file> const & file::stderr() {
	abc_trace_fn(());

	if (!g_ppfileStdErr) {
		_construct_std_file(
#if ABC_HOST_API_POSIX
			STDERR_FILENO,
#elif ABC_HOST_API_WIN32
			::GetStdHandle(STD_ERROR_HANDLE),
#else
	#error TODO-PORT: HOST_API
#endif
			&g_ppfileStdErr
		);
	}
	return *g_ppfileStdErr;
}


/*static*/ std::shared_ptr<file> const & file::stdin() {
	abc_trace_fn(());

	if (!g_ppfileStdIn) {
		_construct_std_file(
#if ABC_HOST_API_POSIX
			STDIN_FILENO,
#elif ABC_HOST_API_WIN32
			::GetStdHandle(STD_INPUT_HANDLE),
#else
	#error TODO-PORT: HOST_API
#endif
			&g_ppfileStdIn
		);
	}
	return *g_ppfileStdIn;
}


/*static*/ std::shared_ptr<file> const & file::stdout() {
	abc_trace_fn(());

	if (!g_ppfileStdOut) {
		_construct_std_file(
#if ABC_HOST_API_POSIX
			STDOUT_FILENO,
#elif ABC_HOST_API_WIN32
			::GetStdHandle(STD_OUTPUT_HANDLE),
#else
	#error TODO-PORT: HOST_API
#endif
			&g_ppfileStdOut
		);
	}
	return *g_ppfileStdOut;
}


/*virtual*/ size_t file::write(void const * p, size_t cb) {
	abc_trace_fn((this, p, cb));

	int8_t const * pb(static_cast<int8_t const *>(p));

	// The top half of this loop is OS-specific; the rest is generalized. As a guideline, the OS
	// write()-equivalent function is invoked at least once, so we give it a chance to report any
	// errors, instead of masking them by skipping the call (e.g. due to cb == 0 on input).
	do {
#if ABC_HOST_API_POSIX
		// This will be repeated at most three times, just to break a size_t-sized block down into
		// ssize_t-sized blocks.
		ssize_t cbLastWritten(::write(
			m_fd.get(), pb, std::min<size_t>(cb, numeric::max<ssize_t>::value)
		));
		if (cbLastWritten < 0) {
			throw_os_error();
		}
#elif ABC_HOST_API_WIN32
		// This will be repeated at least once, and as long as we still have some bytes to write, and
		// writing them does not fail.
		DWORD cbLastWritten;
		if (!::WriteFile(
			m_fd.get(), pb,
			DWORD(std::min<size_t>(cb, numeric::max<DWORD>::value)), &cbLastWritten, NULL
		)) {
			throw_os_error();
		}
#else
	#error TODO-PORT: HOST_API
#endif
		// Some bytes were written; prepare for the next attempt.
		pb += cbLastWritten;
		cb -= size_t(cbLastWritten);
	} while (cb);

	return size_t(pb - static_cast<int8_t const *>(p));
}


/*static*/ std::shared_ptr<file> file::_construct_matching_type(_file_init_data * pfid) {
	abc_trace_fn((pfid));

#if ABC_HOST_API_POSIX
	if (::fstat(pfid->fd.get(), &pfid->statFile)) {
		throw_os_error();
	}
	if (S_ISREG(pfid->statFile.st_mode)) {
		return std::make_shared<regular_file>(pfid);
	}
	if (S_ISCHR(pfid->statFile.st_mode) && ::isatty(pfid->fd.get())) {
		return std::make_shared<console_file>(pfid);
	}
	if (S_ISFIFO(pfid->statFile.st_mode) || S_ISSOCK(pfid->statFile.st_mode)) {
		return std::make_shared<pipe_file>(pfid);
	}
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
	switch (::GetFileType(pfid->fd.get())) {
		case FILE_TYPE_CHAR:
			// Serial line or console.
			// Using ::GetConsoleMode() to detect a console handle requires GENERIC_READ access rights,
			// which could be a problem with stdout/stderr because we don’t ask for that permission for
			// these handles; however, “The handles returned by CreateFile, CreateConsoleScreenBuffer,
			// and GetStdHandle have the GENERIC_READ and GENERIC_WRITE access rights”, so we can trust
			// this to succeed for console handles.
			DWORD iConsoleMode;
			if (::GetConsoleMode(pfid->fd.get(), iConsoleMode)) {
				return std::make_shared<console_file>(pfid);
			}
			break;
		case FILE_TYPE_DISK:
			// Regular file.
			return std::make_shared<regular_file>(pfid);
		case FILE_TYPE_PIPE:
			// Socket or pipe.
			return std::make_shared<pipe_file>(pfid);
		case FILE_TYPE_UNKNOWN:
			// Unknown or error.
			DWORD iErr(::GetLastError());
			if (iErr != ERROR_SUCCESS) {
				throw_os_error(iErr);
			}
			break;
	}
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
	#error TODO-PORT: HOST_API
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else

	// If a file object was not returned in the code above, return a basic file.
	return std::make_shared<file>(pfid);
}


/*static*/ void file::_construct_std_file(filedesc_t fd, std::shared_ptr<file> ** pppf) {
	abc_trace_fn((fd, pppf));

	// TODO: under Win32, GUI subsystem programs will get NULL when calling ::GetStdHandle(). This
	// needs to be handled here, with two options:
	// a. Return a NULL std::shared_ptr. This means that all callers will need additional checks to
	//	   detect this condition; further downstream, some code will need to use alternative means of
	//    output (a message box?).
	// b. Dynamically create a console to write to. This is not very Win32-like, but it allows to
	//    output larger amounts of data that would be unsightly in a message box.
	//
	// Note that this is not an issue for POSIX programs, because when a standard file handle is
	// redirected to /dev/null, it’s still a valid file handle, so no errors occur when reading/
	// writing to it.

	// TODO: mutex!
	ABC_ASSERT(!*pppf);
	// TODO: reduce the number of dynamic allocations.

	std::unique_ptr<std::shared_ptr<file>> ppf(new std::shared_ptr<file>());
	*ppf.get() = attach(filedesc(fd, false));
	// If we’re still here, everything succeeded.

	// If this is the first standard file being constructed, register the releaser.
	if (!g_ppfileStdErr && !g_ppfileStdIn && !g_ppfileStdOut) {
		::atexit(_release_std_files);
	}
	// Return the allocated shared pointer.
	*pppf = ppf.release();
}


/*static*/ void ABC_STL_CALLCONV file::_release_std_files() {
	abc_trace_fn(());

	// TODO: mutex!
	// Destruct the shared pointers, which will allow the files to be released if they were the last
	// strong references to them.
	if (g_ppfileStdErr) {
		delete g_ppfileStdErr;
		g_ppfileStdErr = NULL;
	}
	if (g_ppfileStdIn) {
		delete g_ppfileStdIn;
		g_ppfileStdIn = NULL;
	}
	if (g_ppfileStdOut) {
		delete g_ppfileStdOut;
		g_ppfileStdOut = NULL;
	}
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::console_file


namespace abc {

console_file::console_file(_file_init_data * pfid) :
	file(pfid) {
}


#if ABC_HOST_API_WIN32

/*virtual*/ size_t console_file::read(void * p, size_t cbMax) {
	abc_trace_fn((this, p, cbMax));

	int8_t * pb(static_cast<int8_t *>(p));
	// ::ReadConsole() is invoked at least once, so we give it a chance to report any errors, instead
	// of masking them by skipping the call (e.g. due to cbMax == 0 on input).
	do {
		// This will be repeated at least once, and as long as we still have some bytes to read, and
		// reading them does not fail.
		DWORD cbLastRead;
		if (!::ReadConsole(
			m_fd.get(), pb,
			DWORD(std::min<size_t>(cbMax, numeric::max<DWORD>::value)), &cbLastRead, NULL
		)) {
			DWORD iErr(::GetLastError());
			if (iErr == ERROR_HANDLE_EOF) {
				break;
			}
			throw_os_error(iErr);
		}
		// Some bytes were read; prepare for the next attempt.
		pb += cbLastRead;
		cbMax -= size_t(cbLastRead);
	} while (cbMax);

	return size_t(pb - static_cast<int8_t *>(p));
}


/*virtual*/ size_t console_file::write(void const * p, size_t cb) {
	abc_trace_fn((this, p, cb));

	int8_t const * pb(static_cast<int8_t const *>(p));

	// ::WriteConsole() is invoked at least once, so we give it a chance to report any errors,
	// instead of masking them by skipping the call (e.g. due to cb == 0 on input).
	do {
		// This will be repeated at least once, and as long as we still have some bytes to write, and
		// writing them does not fail.
		DWORD cbLastWritten;
		if (!::WriteFile(
			m_fd.get(), pb,
			DWORD(std::min<size_t>(cb, numeric::max<DWORD>::value)), &cbLastWritten, NULL
		)) {
			throw_os_error();
		}
		// Some bytes were written; prepare for the next attempt.
		pb += cbLastWritten;
		cb -= size_t(cbLastWritten);
	} while (cb);

	return size_t(pb - static_cast<int8_t const *>(p));
}

#endif //if ABC_HOST_API_WIN32

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::pipe_file


namespace abc {

pipe_file::pipe_file(_file_init_data * pfid) :
	file(pfid) {
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::regular_file


namespace abc {

regular_file::regular_file(_file_init_data * pfid) :
	file(pfid)  {
	abc_trace_fn((this, pfid));

	m_bHasSize = true;

#if ABC_HOST_API_POSIX

//	struct ::stat statFile;
	m_cb = size_t(pfid->statFile.st_size);
	if (!m_bBuffered) {
		// For unbuffered access, use the filesystem-suggested I/O size increment.
		m_cbPhysAlign = unsigned(pfid->statFile.st_blksize);
	}

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX

#if _WIN32_WINNT >= 0x0500
	static_assert(
			sizeof(m_cb) == sizeof(LARGE_INTEGER),
			"fileint_t must be the same size as LARGE_INTEGER"
		);
	if (!::GetFileSizeEx(fd.get(), reinterpret_cast<LARGE_INTEGER *>(&m_cb))) {
		throw_os_error();
	}
#else //if _WIN32_WINNT >= 0x0500
	DWORD cbHigh, cbLow(::GetFileSize(fd.get(), &cbHigh));
	if (cbLow == INVALID_FILE_SIZE) {
		DWORD iErr(::GetLastError());
		if (iErr != ERROR_SUCCESS) {
			throw_os_error(iErr);
		}
	}
	m_cb = (fileint_t(cbHigh) << sizeof(cbLow) * CHAR_BIT) | cbLow;
#endif //if _WIN32_WINNT >= 0x0500 … else
	if (!m_bBuffered) {
		// Should really use ::DeviceIoCtl(IOCTL_STORAGE_QUERY_PROPERTY) on the disk containing this
		// file. For now, use 4 KiB alignment, since that’s the most recent commonly used physical
		// sector size.
		m_cbPhysAlign = 4096;
	}
	m_bAppend = pfid->bAppend;

#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
	#error TODO-PORT: HOST_API
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
}


/*virtual*/ unsigned regular_file::physical_alignment() const {
	abc_trace_fn((this));

	return m_cbPhysAlign;
}


/*virtual*/ fileint_t regular_file::size() const {
	abc_trace_fn((this));

	return m_cb;
}


#if ABC_HOST_API_WIN32

/*virtual*/ size_t regular_file::write(void const * p, size_t cb) {
	abc_trace_fn((this, p, cb));

	// Emulating O_APPEND in Win32 requires a little more code: we have to manually seek to EOF, then
	// write-protect the bytes we’re going to add, and then release the write protection.

	/** Win32 ::LockFile() / ::UnlockFile() helper.

	TODO: this will probably find use somewhere else as well, so move it to file.hxx.
	*/
	class file_lock {
	public:

		/** Constructor.
		*/
		file_lock() :
			m_fd(INVALID_HANDLE_VALUE) {
		}


		/** Destructor.
		*/
		~file_lock() {
			if (m_fd != INVALID_HANDLE_VALUE) {
				unlock();
			}
		}


		/** Attempts to lock a range of bytes for the specified file. Returns true if a lock was
		acquired, false if it was not because of any or all of the requested bytes being locked by
		another process, or throws an exception for any other error.

		fd
			Open file to lock.
		ibOffset
			Offset of the first byte to lock.
		cb
			Count of bytes to lock, starting from ibOffset.
		return
			true if the specified range could be locked, or false if the range has already been locked.
		*/
		bool lock(filedesc_t fd, fileint_t ibOffset, fileint_t cb) {
			if (m_fd != INVALID_HANDLE_VALUE) {
				unlock();
			}
			m_fd = fd;
			m_ibOffset.QuadPart = LONGLONG(ibOffset);
			m_cb.QuadPart = LONGLONG(cb);
			if (!::LockFile(
				m_fd, m_ibOffset.LowPart, DWORD(m_ibOffset.HighPart), m_cb.LowPart, DWORD(m_cb.HighPart)
			)) {
				DWORD iErr(::GetLastError());
				if (iErr == ERROR_LOCK_VIOLATION) {
					return false;
				}
				throw_os_error(iErr);
			}
			return true;
		}


		/** Releases the lock acquired by lock().
		*/
		void unlock() {
			if (!::UnlockFile(
				m_fd, m_ibOffset.LowPart, DWORD(m_ibOffset.HighPart), m_cb.LowPart, DWORD(m_cb.HighPart)
			)) {
				throw_os_error();
			}
		}


	private:

		/** Locked file. */
		filedesc_t m_fd;
		/** Start of the locked byte range. */
		LARGE_INTEGER m_ibOffset;
		/** Length of the locked byte range. */
		LARGE_INTEGER m_cb;
	};

	// The file_lock has to be in this scope, so it will unlock after the write is performed.
	file_lock flAppend;
	if (m_bAppend) {
		// In this loop, we’ll seek to EOF and try to lock the not-yet-existing bytes that we want to
		// write to; if the latter fails, we’ll assume that somebody else is doing the same, so we’ll
		// retry from the seek.
		// TODO: guarantee of termination? Maybe the foreign locker won’t release the lock, ever. This
		// is too easy to fool.
		LARGE_INTEGER ibEOF;
		do {
			// TODO: this should really be moved to a file::seek() method.
#if _WIN32_WINNT >= 0x0500
			LARGE_INTEGER ibZero;
			ibZero.QuadPart = 0;
			if (!::SetFilePointerEx(m_fd.get(), ibZero, &ibEOF, FILE_END)) {
				throw_os_error();
			}
#else //if _WIN32_WINNT >= 0x0500
			ibEOF.LowPart = ::SetFilePointer(m_fd.get(), 0, &ibEOF.HighPart, FILE_END);
			if (ibEOF.LowPart == INVALID_SET_FILE_POINTER) {
				DWORD iErr(::GetLastError());
				if (iErr != ERROR_SUCCESS) {
					throw_os_error(iErr);
				}
			}
#endif //if _WIN32_WINNT >= 0x0500 … else
		} while (!flAppend.lock(m_fd.get(), fileint_t(ibEOF.QuadPart), cb));
		// Now the write can occur; the lock will be released automatically at the end.
	}

	return file::write(p, cb);
}

#endif //if ABC_HOST_API_WIN32

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

