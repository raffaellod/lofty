﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

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

#include <abaclade.hxx>

#if ABC_HOST_API_POSIX
   #include <unistd.h> // close()
#endif


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
   #error "TODO: HOST_API"
#endif

filedesc::filedesc(filedesc && fd) :
   m_fd(fd.m_fd),
   m_bOwn(fd.m_bOwn) {
   ABC_TRACE_FUNC(this);

   fd.m_fd = smc_fdNull;
   fd.m_bOwn = false;
}

filedesc::~filedesc() {
   ABC_TRACE_FUNC(this);

   if (m_bOwn && m_fd != smc_fdNull) {
#if ABC_HOST_API_POSIX
      ::close(m_fd);
#elif ABC_HOST_API_WIN32
      ::CloseHandle(m_fd);
#else
   #error "TODO: HOST_API"
#endif
   }
}

filedesc & filedesc::operator=(filedesc_t fd) {
   ABC_TRACE_FUNC(this, fd);

   if (fd != m_fd) {
      this->~filedesc();
   }
   m_fd = fd;
   m_bOwn = true;
   return *this;
}
filedesc & filedesc::operator=(filedesc && fd) {
   ABC_TRACE_FUNC(this);

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