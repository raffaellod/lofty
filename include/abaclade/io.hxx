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

//! Unsigned integer wide enough to express an I/O-related size.
#if ABC_HOST_API_POSIX || ABC_HOST_API_WIN32
   typedef std::uint64_t full_size_t;
#else
   #error HOST_API
#endif

//! Integer wide enough to express an I/O-related offset.
#if ABC_HOST_API_POSIX || ABC_HOST_API_WIN32
   typedef std::int64_t offset_t;
#else
   #error HOST_API
#endif

//! File access modes.
ABC_ENUM_AUTO_VALUES(access_mode,
   read,       //! Read-only access.
   write,      //! Write-only access.
   read_write, //! Read/write access.
   append      //! Append-only access.
);

//! Position indicators to which offsets may be relative.
ABC_ENUM_AUTO_VALUES(seek_from,
   start,   //! The offset is relative to the start of the data (absolute seek).
   current, //! The offset is relative to the current offset (incremental seek).
   end      //! The offset is relative to the end of the data and presumably negative.
);

// Some C libraries (such as MS CRT) define these as macros.
#ifdef stdin
   #undef stdin
   #undef stdout
   #undef stderr
#endif

//! List of standard (OS-provided) files.
ABC_ENUM_AUTO_VALUES(stdfile,
   stdin,  //! Internal identifier for stdin.
   stdout, //! Internal identifier for stdout.
   stderr  //! Internal identifier for stderr.
);

} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

