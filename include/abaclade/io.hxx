/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io globals

namespace abc {
namespace io {

/*! Unsigned integer wide enough to express an I/O-related size. */
#if ABC_HOST_API_POSIX || ABC_HOST_API_WIN32
   typedef uint64_t full_size_t;
#else
   #error HOST_API
#endif


/*! Integer wide enough to express an I/O-related offset. */
#if ABC_HOST_API_POSIX || ABC_HOST_API_WIN32
   typedef int64_t offset_t;
#else
   #error HOST_API
#endif


/*! File access modes.
*/
ABC_ENUM(access_mode,
   /*! Read-only access. */
   (read,       1),
   /*! Write-only access. */
   (write,      2),
   /*! Read/write access. */
   (read_write, 3),
   /*! Append-only access. */
   (append,     4)
);


/*! Position indicators to which offsets may be relative.
*/
ABC_ENUM(seek_from,
   /*! The offset is relative to the start of the data (absolute seek). */
   (start,   0),
   /*! The offset is relative to the current offset (incremental seek). */
   (current, 1),
   /*! The offset is relative to the end of the data and presumably negative. */
   (end,     2)
);


// Some C libraries (such as MS CRT) define these as macros.
#ifdef stdin
   #undef stdin
   #undef stdout
   #undef stderr
#endif

/*! List of standard (OS-provided) files.
*/
ABC_ENUM(stdfile,
   /*! Internal identifier for stdin. */
   (stdin,  0),
   /*! Internal identifier for stdout. */
   (stdout, 1),
   /*! Internal identifier for stderr. */
   (stderr, 2)
);

} //namespace io
} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////

