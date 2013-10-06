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
	#include <unistd.h> // *_FILENO ssize_t close() open() read() write()
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



file::file(filedesc && fd) :
	m_bBuffered(true),
	m_fd(_post_open(std::move(fd))) {
}
file::file(file_path const & fp, access_mode fam, bool bBuffered /*= true*/) :
	m_bBuffered(bBuffered),
	m_fd(_post_open(_open(fp, fam))) {
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


size_t file::read(void * p, size_t cbMax) {
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


size_t file::write(void const * p, size_t cb) {
	abc_trace_fn((this, p, cb));

	int8_t const * pb(static_cast<int8_t const *>(p));
#if ABC_HOST_API_WIN32
	// Emulating O_APPEND in Win32 requires a little more code: we have to manually seek to EOF, then
	// write-protect the bytes we’re going to add, and then release the write protection.

	/// Win32 ::LockFile() / ::UnlockFile() helper.
	// TODO: this will probably find use somewhere else as well, so move it to file.hxx.
	//
	class file_lock {
	public:
	
		/// Constructor.
		//
		file_lock() :
			m_fd(INVALID_HANDLE_VALUE) {
		}


		/// Destructor.
		//
		~file_lock() {
			if (m_fd != INVALID_HANDLE_VALUE) {
				unlock();
			}
		}


		/// Attempts to lock a range of bytes for the specified file. Returns true if a lock was
		// acquired, false if it was not because of any or all of the requested bytes being locked by
		// another process, or throws an exception for any other error.
		//
		bool lock(filedesc_t fd, fileint_t ibOffset, fileint_t cb) {
			assert(m_fd == INVALID_FILE_HANDLE);
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


		void unlock() {
			assert(m_fd != INVALID_FILE_HANDLE);
			if (!::UnlockFile(
				m_fd, m_ibOffset.LowPart, DWORD(m_ibOffset.HighPart), m_cb.LowPart, DWORD(m_cb.HighPart)
			)) {
				throw_os_error();
			}
		}


	private:

		/// Locked file.
		filedesc_t m_fd;
		/// Start of the locked byte range.
		LARGE_INTEGER m_ibOffset;
		/// Length of the locked byte range.
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
#endif

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


/*static*/ void file::_construct_std_file(filedesc_t fd, std::shared_ptr<file> ** pppf) {
	abc_trace_fn((fd, pppf));

	// TODO: mutex!
	assert(!*pppf);
	// TODO: reduce the number of dynamic allocations.

	std::unique_ptr<std::shared_ptr<file>> ppf(new std::shared_ptr<file>());
	*ppf.get() = std::make_shared<file>(filedesc(fd, false));
	// If we’re still here, everything succeeded.

	// If this is the first standard file being constructed, register the releaser.
	if (!g_ppfileStdErr && !g_ppfileStdIn && !g_ppfileStdOut) {
		::atexit(_release_std_files);
	}
	// Return the allocated shared pointer.
	*pppf = ppf.release();
}


filedesc file::_open(file_path const & fp, access_mode fam) {
	abc_trace_fn((this, fp, fam));

	filedesc fd;
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
	if (!m_bBuffered) {
		fi |= O_DIRECT;
	}
	fd = ::open(fp.data(), fi, 0666);
#elif ABC_HOST_API_WIN32
	DWORD fiAccess, fiShareMode, iAction, fi(FILE_ATTRIBUTE_NORMAL);
	m_bAppend = false;
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
			m_bAppend = true;
			break;
		no_default;
	}
	if (!m_bBuffered) {
		fi |= FILE_FLAG_NO_BUFFERING;
	} else if (fiAccess & GENERIC_READ) {
		fi |= FILE_FLAG_SEQUENTIAL_SCAN;
	}
	fd = ::CreateFile(fp.data(), fiAccess, fiShareMode, NULL, iAction, fi, NULL);
#else
	#error TODO-PORT: HOST_API
#endif
	if (!fd) {
		throw_os_error();
	}
	return std::move(fd);
}


filedesc file::_post_open(filedesc && fd) {
	abc_trace_fn((this/*, fd*/));

#if ABC_HOST_API_POSIX

	struct ::stat statFile;
	if (::fstat(fd.get(), &statFile)) {
		throw_os_error();
	}
	m_bHasSize = S_ISREG(statFile.st_mode);
	if (m_bHasSize) {
		m_cb = size_t(statFile.st_size);
	}
	if (!m_bBuffered) {
		// For unbuffered access, use the filesystem-suggested I/O size increment.
		m_cbPhysAlign = unsigned(statFile.st_blksize);
	}

#elif ABC_HOST_API_WIN32

	m_bHasSize = (::GetFileType(fd.get()) == FILE_TYPE_DISK);
	if (m_bHasSize) {
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
	}
	if (!m_bBuffered) {
		// Should really use ::DeviceIoCtl(IOCTL_STORAGE_QUERY_PROPERTY) on the disk containing this
		// file. For now, use 4 KiB alignment, since that’s the most recent commonly used physical
		// sector size.
		m_cbPhysAlign = 4096;
	}

#else //elif ABC_HOST_API_WIN32
	#error TODO-PORT: HOST_API
#endif
	return std::move(fd);
}


/*static*/ void file::_release_std_files() {
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

