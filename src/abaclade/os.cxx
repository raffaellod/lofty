/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015 Raffaello D. Di Napoli

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

#include <abaclade.hxx>
#include <abaclade/os.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace os {

invalid_path::invalid_path() {
   m_pszWhat = "abc::invalid_path";
}
invalid_path::invalid_path(invalid_path const & x) :
   generic_error(x),
   m_opInvalid(x.m_opInvalid) {
}

/*virtual*/ invalid_path::~invalid_path() {
}

invalid_path & invalid_path::operator=(invalid_path const & x) {
   generic_error::operator=(x);
   m_opInvalid = x.m_opInvalid;
   return *this;
}

void invalid_path::init(os::path const & opInvalid, errint_t err /*= 0*/) {
   generic_error::init(err ? err :
#if ABC_HOST_API_WIN32
      ERROR_BAD_PATHNAME
#else
      0
#endif
   );
   m_opInvalid = opInvalid;
}

/*virtual*/ void invalid_path::write_extended_info(io::text::writer * ptwOut) const /*override*/ {
   generic_error::write_extended_info(ptwOut);
   ptwOut->print(ABC_SL("not a valid path: \"{}\""), m_opInvalid);
}

}} //namespace abc::os

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace os {

path_not_found::path_not_found() {
   m_pszWhat = "abc::path_not_found";
}

path_not_found::path_not_found(path_not_found const & x) :
   generic_error(x),
   m_opNotFound(x.m_opNotFound) {
}

/*virtual*/ path_not_found::~path_not_found() {
}

path_not_found & path_not_found::operator=(path_not_found const & x) {
   generic_error::operator=(x);
   m_opNotFound = x.m_opNotFound;
   return *this;
}

void path_not_found::init(os::path const & opNotFound, errint_t err /*= 0*/) {
   generic_error::init(err ? err :
#if ABC_HOST_API_POSIX
      ENOENT
#elif ABC_HOST_API_WIN32
      ERROR_PATH_NOT_FOUND
#else
      0
#endif
   );
   m_opNotFound = opNotFound;
}

/*virtual*/ void path_not_found::write_extended_info(io::text::writer * ptwOut) const /*override*/ {
   generic_error::write_extended_info(ptwOut);
   ptwOut->print(ABC_SL("path not found: \"{}\""), m_opNotFound);
}

}} //namespace abc::os
