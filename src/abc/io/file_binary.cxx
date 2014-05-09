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

#include <abc/core.hxx>
#include <abc/io/file_binary.hxx>
#include <abc/numeric.hxx>
#include <abc/trace.hxx>
#include <algorithm>
#if ABC_HOST_API_POSIX
   #include <unistd.h> // *_FILENO ssize_t close() isatty() open() read() write()
   #include <stdlib.h> // atexit()
   #include <fcntl.h> // O_*
   #include <sys/stat.h> // S_*, stat()
// #include <sys/mman.h> // mmap(), munmap(), PROT_*, MAP_*
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io globals


namespace abc {

namespace io {

static std::shared_ptr<file_binary_writer> g_pfbwStdErr;
static std::shared_ptr<file_binary_reader> g_pfbrStdIn;
static std::shared_ptr<file_binary_writer> g_pfbwStdOut;


struct _file_init_data {
#if ABC_HOST_API_POSIX
   /** Set by _construct_binary(). */
   struct ::stat statFile;
#endif
   /** See file_binary_base::m_fd. To be set before calling _construct_binary(). */
   filedesc fd;
   /** Determines what type of I/O object will be instantiated. To be set before calling
   _construct_binary(). */
   access_mode am;
   /** See file_binary_base::m_bBuffered. To be set before calling _construct_binary(). */
   bool bBuffered:1;
};


/** Instantiates a binary_base specialization appropriate for the descriptor in *pfid, returning a
shared pointer to it.

pfid
   Data that will be passed to the constructor of the file object.
return
   Shared pointer to the newly created object.
*/
static std::shared_ptr<file_binary_base> _construct_binary(_file_init_data * pfid) {
   ABC_TRACE_FN((pfid));

#if ABC_HOST_API_POSIX
   if (::fstat(pfid->fd.get(), &pfid->statFile)) {
      throw_os_error();
   }
   if (S_ISREG(pfid->statFile.st_mode)) {
      switch (pfid->am.base()) {
         case access_mode::read:
            return std::make_shared<regular_file_binary_reader>(pfid);
         case access_mode::write:
         case access_mode::append:
            return std::make_shared<regular_file_binary_writer>(pfid);
         case access_mode::read_write:
         // default is here just to silence compiler warnings.
         default:
            // TODO: regular_file_binary_random
            break;
      }
   }
   if (S_ISCHR(pfid->statFile.st_mode) && ::isatty(pfid->fd.get())) {
      switch (pfid->am.base()) {
         case access_mode::read:
            return std::make_shared<console_binary_reader>(pfid);
         case access_mode::write:
            return std::make_shared<console_binary_writer>(pfid);
         case access_mode::append:
         case access_mode::read_write:
         // default is here just to silence compiler warnings.
         default:
            // TODO: use a better exception class.
            ABC_THROW(argument_error, ());
      }
   }
   if (S_ISFIFO(pfid->statFile.st_mode) || S_ISSOCK(pfid->statFile.st_mode)) {
      switch (pfid->am.base()) {
         case access_mode::read:
            return std::make_shared<pipe_binary_reader>(pfid);
         case access_mode::write:
            return std::make_shared<pipe_binary_writer>(pfid);
         case access_mode::append:
         case access_mode::read_write:
         // default is here just to silence compiler warnings.
         default:
            // TODO: use a better exception class.
            ABC_THROW(argument_error, ());
      }
   }
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   switch (::GetFileType(pfid->fd.get())) {
      case FILE_TYPE_CHAR:
         // Serial line or console.
         // Using ::GetConsoleMode() to detect a console handle requires GENERIC_READ access rights,
         // which could be a problem with stdout/stderr because we don’t ask for that permission for
         // these handles; however, for consoles, “The handles returned by CreateFile,
         // CreateConsoleScreenBuffer, and GetStdHandle have the GENERIC_READ and GENERIC_WRITE
         // access rights”, so we can trust this to succeed for console handles.
         DWORD iConsoleMode;
         if (::GetConsoleMode(pfid->fd.get(), &iConsoleMode)) {
            switch (pfid->am.base()) {
               case access_mode::read:
                  return std::make_shared<console_binary_reader>(pfid);
               case access_mode::write:
                  return std::make_shared<console_binary_writer>(pfid);
               case access_mode::append:
               case access_mode::read_write:
               // default is here just to silence compiler warnings.
               default:
                  // TODO: use a better exception class.
                  ABC_THROW(argument_error, ());
            }
         }
         break;

      case FILE_TYPE_DISK:
         // Regular file.
         switch (pfid->am.base()) {
            case access_mode::read:
               return std::make_shared<regular_file_binary_reader>(pfid);
            case access_mode::write:
            case access_mode::append:
               return std::make_shared<regular_file_binary_writer>(pfid);
            case access_mode::read_write:
            // default is here just to silence compiler warnings.
            default:
               // TODO: regular_file_binary_random
               break;
         }
         break;

      case FILE_TYPE_PIPE:
         // Socket or pipe.
         switch (pfid->am.base()) {
            case access_mode::read:
               return std::make_shared<pipe_binary_reader>(pfid);
            case access_mode::write:
               return std::make_shared<pipe_binary_writer>(pfid);
            case access_mode::append:
            case access_mode::read_write:
            // default is here just to silence compiler warnings.
            default:
               // TODO: use a better exception class.
               ABC_THROW(argument_error, ());
         }
         break;

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

   // If a file object was not returned in the code above, return a generic file.
   switch (pfid->am.base()) {
      case access_mode::read:
         return std::make_shared<file_binary_reader>(pfid);
      case access_mode::write:
         return std::make_shared<file_binary_writer>(pfid);
      case access_mode::append:
      case access_mode::read_write:
      // default is here just to silence compiler warnings.
      default:
         // TODO: use a better exception class.
         ABC_THROW(argument_error, ());
   }
}


/** Returns new binary I/O object controlling the specified file descriptor.

fd
   File descriptor to take ownership of.
am
   Desired access mode.
return
   Pointer to a binary I/O object controlling fd.
*/
static std::shared_ptr<file_binary_base> _attach_binary(filedesc && fd, access_mode am) {
   ABC_TRACE_FN((/*fd*/));

   _file_init_data fid;
   fid.fd = std::move(fd);
   fid.am = am;
   // Since this method is supposed to be used only for standard descriptors, assume that OS
   // buffering is on.
   fid.bBuffered = true;
   return _construct_binary(&fid);
}


std::shared_ptr<file_binary_writer> binary_stderr() {
   ABC_TRACE_FN(());

   // TODO: under Win32, GUI subsystem programs will get nullptr when calling ::GetStdHandle(). This
   // needs to be handled here, with two options:
   // a. Return a nullptr std::shared_ptr. This means that all callers will need additional checks
   //    to detect this condition; further downstream, some code will need to use alternative means
   //    of output (a message box?).
   // b. Dynamically create a console to write to. This is not very Win32-like, but it allows to
   //    output larger amounts of data that would be unsightly in a message box.
   //
   // Note that this is not an issue for POSIX programs, because when a standard file handle is
   // redirected to /dev/null, it’s still a valid file handle, so no errors occur when reading/
   // writing to it.

   // TODO: mutex!
   if (!g_pfbwStdErr) {
      g_pfbwStdErr = std::dynamic_pointer_cast<file_binary_writer>(_attach_binary(filedesc(
#if ABC_HOST_API_POSIX
         STDERR_FILENO,
#elif ABC_HOST_API_WIN32
         ::GetStdHandle(STD_ERROR_HANDLE),
#else
   #error TODO-PORT: HOST_API
#endif
         false
      ), access_mode::write));
   }
   return g_pfbwStdErr;
}


std::shared_ptr<file_binary_reader> binary_stdin() {
   ABC_TRACE_FN(());

   // TODO: under Win32, GUI subsystem programs will get nullptr when calling ::GetStdHandle(). This
   // needs to be handled here, with two options:
   // a. Return a nullptr std::shared_ptr. This means that all callers will need additional checks
   //    to detect this condition; further downstream, some code will need to use alternative means
   //    of output (a message box?).
   // b. Dynamically create a console to write to. This is not very Win32-like, but it allows to
   //    output larger amounts of data that would be unsightly in a message box.
   //
   // Note that this is not an issue for POSIX programs, because when a standard file handle is
   // redirected to /dev/null, it’s still a valid file handle, so no errors occur when reading/
   // writing to it.

   // TODO: mutex!
   if (!g_pfbrStdIn) {
      g_pfbrStdIn = std::dynamic_pointer_cast<file_binary_reader>(_attach_binary(filedesc(
#if ABC_HOST_API_POSIX
         STDIN_FILENO,
#elif ABC_HOST_API_WIN32
         ::GetStdHandle(STD_INPUT_HANDLE),
#else
   #error TODO-PORT: HOST_API
#endif
         false
      ), access_mode::read));
   }
   return g_pfbrStdIn;
}


std::shared_ptr<file_binary_writer> binary_stdout() {
   ABC_TRACE_FN(());

   // TODO: under Win32, GUI subsystem programs will get nullptr when calling ::GetStdHandle(). This
   // needs to be handled here, with two options:
   // a. Return a nullptr std::shared_ptr. This means that all callers will need additional checks
   //    to detect this condition; further downstream, some code will need to use alternative means
   //    of output (a message box?).
   // b. Dynamically create a console to write to. This is not very Win32-like, but it allows to
   //    output larger amounts of data that would be unsightly in a message box.
   //
   // Note that this is not an issue for POSIX programs, because when a standard file handle is
   // redirected to /dev/null, it’s still a valid file handle, so no errors occur when reading/
   // writing to it.

   // TODO: mutex!
   if (!g_pfbwStdOut) {
      g_pfbwStdOut = std::dynamic_pointer_cast<file_binary_writer>(_attach_binary(filedesc(
#if ABC_HOST_API_POSIX
         STDOUT_FILENO,
#elif ABC_HOST_API_WIN32
         ::GetStdHandle(STD_OUTPUT_HANDLE),
#else
   #error TODO-PORT: HOST_API
#endif
         false
      ), access_mode::write));
   }
   return g_pfbwStdOut;
}


std::shared_ptr<file_binary_base> open_binary(
   file_path const & fp, access_mode am, bool bBuffered /*= true*/
) {
   ABC_TRACE_FN((fp, am, bBuffered));

   _file_init_data fid;
#if ABC_HOST_API_POSIX
   int fi;
   switch (am.base()) {
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
   if (!bBuffered) {
      fi |= O_DIRECT;
   }
   fid.fd = ::open(fp.os_str().c_str().get(), fi, 0666);
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   DWORD fiAccess, fiShareMode, iAction, fi(FILE_ATTRIBUTE_NORMAL);
   switch (am.base()) {
      default:
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
         break;
   }
   if (!bBuffered) {
      fi |= FILE_FLAG_NO_BUFFERING;
   } else if (fiAccess & GENERIC_READ) {
      fi |= FILE_FLAG_SEQUENTIAL_SCAN;
   }
   fid.fd = ::CreateFile(
      fp.os_str().c_str().get(), fiAccess, fiShareMode, nullptr, iAction, fi, nullptr
   );
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error TODO-PORT: HOST_API
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
   if (!fid.fd) {
      throw_os_error();
   }
   fid.am = am;
   fid.bBuffered = bBuffered;
   return _construct_binary(&fid);
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::filedesc


namespace abc {

namespace io {

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

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::file_binary_base


namespace abc {

namespace io {

file_binary_base::file_binary_base(_file_init_data * pfid) :
   m_fd(std::move(pfid->fd)) {
}


/*virtual*/ file_binary_base::~file_binary_base() {
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::file_binary_reader


namespace abc {

namespace io {

file_binary_reader::file_binary_reader(_file_init_data * pfid) :
   file_binary_base(pfid) {
}


/*virtual*/ file_binary_reader::~file_binary_reader() {
}


/*virtual*/ size_t file_binary_reader::read(void * p, size_t cbMax) {
   ABC_TRACE_FN((this, p, cbMax));

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
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
      // This will be repeated at least once, and as long as we still have some bytes to read, and
      // reading them does not fail.
      DWORD cbLastRead;
      if (!::ReadFile(
         m_fd.get(), pb,
         DWORD(std::min<size_t>(cbMax, numeric::max<DWORD>::value)), &cbLastRead, nullptr
      )) {
         DWORD iErr(::GetLastError());
         if (iErr == ERROR_HANDLE_EOF) {
            break;
         }
         throw_os_error(iErr);
      }
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error TODO-PORT: HOST_API
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
      // Some bytes were read; prepare for the next attempt.
      pb += cbLastRead;
      cbMax -= size_t(cbLastRead);
   } while (cbMax);

   return size_t(pb - static_cast<int8_t *>(p));
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::file_binary_writer


namespace abc {

namespace io {

file_binary_writer::file_binary_writer(_file_init_data * pfid) :
   file_binary_base(pfid) {
}


/*virtual*/ file_binary_writer::~file_binary_writer() {
}


/*virtual*/ void file_binary_writer::flush() {
   ABC_TRACE_FN((this));

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


/*virtual*/ size_t file_binary_writer::write(void const * p, size_t cb) {
   ABC_TRACE_FN((this, p, cb));

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
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
      // This will be repeated at least once, and as long as we still have some bytes to write, and
      // writing them does not fail.
      DWORD cbLastWritten;
      if (!::WriteFile(
         m_fd.get(), pb,
         DWORD(std::min<size_t>(cb, numeric::max<DWORD>::value)), &cbLastWritten, nullptr
      )) {
         throw_os_error();
      }
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error TODO-PORT: HOST_API
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
      // Some bytes were written; prepare for the next attempt.
      pb += cbLastWritten;
      cb -= size_t(cbLastWritten);
   } while (cb);

   return size_t(pb - static_cast<int8_t const *>(p));
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::console_binary_reader


namespace abc {

namespace io {

console_binary_reader::console_binary_reader(_file_init_data * pfid) :
   file_binary_base(pfid),
   file_binary_reader(pfid) {
}


/*virtual*/ console_binary_reader::~console_binary_reader() {
}


#if ABC_HOST_API_WIN32

/*virtual*/ size_t console_binary_reader::read(void * p, size_t cbMax) {
   ABC_TRACE_FN((this, p, cbMax));

   // Note: ::WriteConsole() expects character counts in place of byte counts, so everything must be
   // divided by sizeof(char_t).
   size_t cchMax(cbMax / sizeof(char_t));

   int8_t * pb(static_cast<int8_t *>(p));
   // ::ReadConsole() is invoked at least once, so we give it a chance to report any errors, instead
   // of masking them by skipping the call (e.g. due to cchMax == 0 on input).
   do {
      // This will be repeated at least once, and as long as we still have some bytes to read, and
      // reading them does not fail.
      DWORD cchLastRead;
      if (!::ReadConsole(
         m_fd.get(), pb,
         DWORD(std::min<size_t>(cchMax, numeric::max<DWORD>::value)), &cchLastRead, nullptr
      )) {
         DWORD iErr(::GetLastError());
         if (iErr == ERROR_HANDLE_EOF) {
            break;
         }
         throw_os_error(iErr);
      }
      // Some bytes were read; prepare for the next attempt.
      pb += cchLastRead * sizeof(char_t);
      cchMax -= size_t(cchLastRead);
   } while (cchMax);

   return size_t(pb - static_cast<int8_t *>(p)) * sizeof(char_t);
}

#endif //if ABC_HOST_API_WIN32

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::console_binary_writer


namespace abc {

namespace io {

console_binary_writer::console_binary_writer(_file_init_data * pfid) :
   file_binary_base(pfid),
   file_binary_writer(pfid) {
}


/*virtual*/ console_binary_writer::~console_binary_writer() {
}


#if ABC_HOST_API_WIN32

/*virtual*/ size_t console_binary_writer::write(void const * p, size_t cb) {
   ABC_TRACE_FN((this, p, cb));

   // TODO: verify that ::WriteConsole() is able to properly display UTF-16 surrogates.

   // Note: ::WriteConsole() expects character counts in place of byte counts, so everything must be
   // divided by sizeof(char_t).
   size_t cch(cb / sizeof(char_t));

   int8_t const * pb(static_cast<int8_t const *>(p));
   // ::WriteConsole() is invoked at least once, so we give it a chance to report any errors,
   // instead of masking them by skipping the call (e.g. due to cch == 0 on input).
   do {
      // This will be repeated at least once, and as long as we still have some bytes to write, and
      // writing them does not fail.
      DWORD cchLastWritten;
      if (!::WriteConsole(
         m_fd.get(), pb,
         DWORD(std::min<size_t>(cch, numeric::max<DWORD>::value)), &cchLastWritten, nullptr
      )) {
         throw_os_error();
      }
      // Some bytes were written; prepare for the next attempt.
      pb += cchLastWritten * sizeof(char_t);
      cch -= size_t(cchLastWritten);
   } while (cch);

   return size_t(pb - static_cast<int8_t const *>(p)) * sizeof(char_t);
}

#endif //if ABC_HOST_API_WIN32

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::pipe_binary_reader


namespace abc {

namespace io {

pipe_binary_reader::pipe_binary_reader(_file_init_data * pfid) :
   file_binary_base(pfid),
   file_binary_reader(pfid) {
}


/*virtual*/ pipe_binary_reader::~pipe_binary_reader() {
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::pipe_binary_writer


namespace abc {

namespace io {

pipe_binary_writer::pipe_binary_writer(_file_init_data * pfid) :
   file_binary_base(pfid),
   file_binary_writer(pfid) {
}


/*virtual*/ pipe_binary_writer::~pipe_binary_writer() {
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::regular_file_binary_base


namespace abc {

namespace io {

regular_file_binary_base::regular_file_binary_base(_file_init_data * pfid) :
   file_binary_base(pfid) {
   ABC_TRACE_FN((this, pfid));

#if ABC_HOST_API_POSIX

   m_cb = size_t(pfid->statFile.st_size);
#if 0
   if (!m_bBuffered) {
      // For unbuffered access, use the filesystem-suggested I/O size increment.
      m_cbPhysAlign = unsigned(pfid->statFile.st_blksize);
   }
#endif

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX

#if _WIN32_WINNT >= 0x0500
   static_assert(
      sizeof(m_cb) == sizeof(LARGE_INTEGER),
      "abc::io::full_size_t must be the same size as LARGE_INTEGER"
   );
   if (!::GetFileSizeEx(m_fd.get(), reinterpret_cast<LARGE_INTEGER *>(&m_cb))) {
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
   m_cb = (full_size_t(cbHigh) << sizeof(cbLow) * CHAR_BIT) | cbLow;
#endif //if _WIN32_WINNT >= 0x0500 … else
#if 0
   if (!m_bBuffered) {
      // Should really use ::DeviceIoCtl(IOCTL_STORAGE_QUERY_PROPERTY) on the disk containing this
      // file. For now, use 4 KiB alignment, since that’s the most recent commonly used physical
      // sector size.
      m_cbPhysAlign = 4096;
   }
#endif

#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error TODO-PORT: HOST_API
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
}


/*virtual*/ regular_file_binary_base::~regular_file_binary_base() {
}


/*virtual*/ offset_t regular_file_binary_base::seek(offset_t ibOffset, seek_from sfWhence) {
   ABC_TRACE_FN((this, ibOffset, sfWhence));

#if ABC_HOST_API_POSIX

   int iWhence;
   switch (sfWhence.base()) {
      // default is here just to silence compiler warnings.
      default:
      case seek_from::start:
         iWhence = SEEK_SET;
         break;
      case seek_from::current:
         iWhence = SEEK_CUR;
         break;
      case seek_from::end:
         iWhence = SEEK_END;
         break;
   }
   offset_t ibNewOffset(::lseek(m_fd.get(), ibOffset, iWhence));
   if (ibNewOffset == -1) {
      throw_os_error();
   }
   return ibNewOffset;

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX

   static_assert(
      sizeof(iOffset) == sizeof(LARGE_INTEGER),
      "abc::io::offset_t must be the same size as LARGE_INTEGER"
   );
   int iWhence;
   switch (sfWhence.base()) {
      // default is here just to silence compiler warnings.
      default:
      case seek_from::start:
         iWhence = FILE_BEGIN;
         break;
      case seek_from::current:
         iWhence = FILE_CURRENT;
         break;
      case seek_from::end:
         iWhence = FILE_END;
         break;
   }
   LARGE_INTEGER ibNewOffset;
   ibNewOffset.QuadPart = ibOffset;
#if _WIN32_WINNT >= 0x0500
   if (!::SetFilePointerEx(m_fd.get(), ibNewOffset, &ibNewOffset, iWhence)) {
      throw_os_error();
   }
#else //if _WIN32_WINNT >= 0x0500
   ibNewOffset.LowPart = ::SetFilePointer(
      m_fd.get(), ibNewOffset.LowPart, &ibNewOffset.HighPart, iWhence
   );
   if (ibNewOffset.LowPart == INVALID_SET_FILE_POINTER) {
      DWORD iErr(::GetLastError());
      if (iErr != ERROR_SUCCESS) {
         throw_os_error(iErr);
      }
   }
#endif //if _WIN32_WINNT >= 0x0500 … else
   return ibNewOffset.QuadPart;

#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error TODO-PORT: HOST_API
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
}


/*virtual*/ full_size_t regular_file_binary_base::size() const {
   ABC_TRACE_FN((this));

   return m_cb;
}


/*virtual*/ offset_t regular_file_binary_base::tell() const {
   ABC_TRACE_FN((this));

#if ABC_HOST_API_POSIX || ABC_HOST_API_WIN32
   // Seeking 0 bytes from the current position won’t change the internal status of the file
   // descriptor, so casting the const-ness away is not semantically wrong.
   return const_cast<regular_file_binary_base *>(this)->seek(0, seek_from::current);
#else
   #error TODO-PORT: HOST_API
#endif
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::regular_file_binary_reader


namespace abc {

namespace io {

regular_file_binary_reader::regular_file_binary_reader(_file_init_data * pfid) :
   file_binary_base(pfid),
   regular_file_binary_base(pfid),
   file_binary_reader(pfid) {
   ABC_TRACE_FN((this, pfid));
}


/*virtual*/ regular_file_binary_reader::~regular_file_binary_reader() {
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::regular_file_binary_writer


namespace abc {

namespace io {

regular_file_binary_writer::regular_file_binary_writer(_file_init_data * pfid) :
   file_binary_base(pfid),
   regular_file_binary_base(pfid),
   file_binary_writer(pfid) {
   ABC_TRACE_FN((this, pfid));

#if ABC_HOST_API_WIN32
   m_bAppend = (pfid->am == access_mode::append);
#endif
}


/*virtual*/ regular_file_binary_writer::~regular_file_binary_writer() {
}


#if ABC_HOST_API_WIN32

/*virtual*/ size_t regular_file_binary_writer::write(void const * p, size_t cb) {
   ABC_TRACE_FN((this, p, cb));

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
      bool lock(filedesc_t fd, offset_t ibOffset, full_size_t cb) {
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
      } while (!flAppend.lock(m_fd.get(), offset_t(ibEOF.QuadPart), cb));
      // Now the write can occur; the lock will be released automatically at the end.
   }

   return file_binary_writer::write(p, cb);
}

#endif //if ABC_HOST_API_WIN32

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////

