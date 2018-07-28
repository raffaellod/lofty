/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_IO_BINARY__PVT_FILE_INIT_DATA_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_IO_BINARY__PVT_FILE_INIT_DATA_HXX
#endif

#ifndef _LOFTY_IO_BINARY__PVT_FILE_INIT_DATA_HXX_NOPUB
#define _LOFTY_IO_BINARY__PVT_FILE_INIT_DATA_HXX_NOPUB

#include <lofty/io.hxx>
#if LOFTY_HOST_API_POSIX
   #include <sys/stat.h> // stat
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace binary { namespace _pvt {

struct file_init_data {
#if LOFTY_HOST_API_POSIX
   //! Set by _construct().
   struct ::stat stat;
#endif
   //! See file_stream::m_fd. To be set before calling _construct().
   io::_LOFTY_PUBNS filedesc fd;
   //! Determines what type of stream will be instantiated. To be set before calling _construct().
   io::_LOFTY_PUBNS access_mode mode;
   /*! If true, causes the file to be opened with flags to the effect of disabling OS cache for the file. To
   be set before calling _construct(). */
   bool bypass_cache:1;
};

}}}} //namespace lofty::io::binary::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_IO_BINARY__PVT_FILE_INIT_DATA_HXX_NOPUB

#ifdef _LOFTY_IO_BINARY__PVT_FILE_INIT_DATA_HXX
   #undef _LOFTY_NOPUB

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_IO_BINARY__PVT_FILE_INIT_DATA_HXX
