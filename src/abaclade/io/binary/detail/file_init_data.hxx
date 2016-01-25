/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2016 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABACLADE_IO_BINARY_DETAIL_FILE_INIT_DATA_HXX
#define _ABACLADE_IO_BINARY_DETAIL_FILE_INIT_DATA_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#if ABC_HOST_API_POSIX
   #include <sys/stat.h> // stat
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace binary { namespace detail {

struct file_init_data {
#if ABC_HOST_API_POSIX
   //! Set by _construct().
   struct ::stat statFile;
#endif
   //! See file_stream::m_fd. To be set before calling _construct().
   filedesc fd;
   //! Determines what type of stream will be instantiated. To be set before calling _construct().
   access_mode am;
   /*! If true, causes the file to be opened with flags to the effect of disabling OS cache for the
   file. To be set before calling _construct(). */
   bool bBypassCache:1;
};

}}}} //namespace abc::io::binary::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_IO_BINARY_DETAIL_FILE_INIT_DATA_HXX
