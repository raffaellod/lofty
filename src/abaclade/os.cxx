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

#include <abaclade.hxx>
#include <abaclade/defer_to_scope_end.hxx>
#include <abaclade/os.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace os {

/*explicit*/ invalid_path::invalid_path(os::path const & opInvalid, errint_t err /*= 0*/) :
   generic_error(err ? err :
#if ABC_HOST_API_WIN32
      ERROR_BAD_PATHNAME
#else
      0
#endif
   ),
   m_opInvalid(opInvalid) {
   what_writer().print(ABC_SL("not a valid path=\"{}\""), m_opInvalid);
}

invalid_path::invalid_path(invalid_path const & x) :
   generic_error(x),
   m_opInvalid(x.m_opInvalid) {
}

/*virtual*/ invalid_path::~invalid_path() ABC_STL_NOEXCEPT_TRUE() {
}

invalid_path & invalid_path::operator=(invalid_path const & x) {
   generic_error::operator=(x);
   m_opInvalid = x.m_opInvalid;
   return *this;
}

}} //namespace abc::os

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace os {

/*explicit*/ path_not_found::path_not_found(os::path const & opNotFound, errint_t err /*= 0*/) :
   generic_error(err ? err :
#if ABC_HOST_API_POSIX
      ENOENT
#elif ABC_HOST_API_WIN32
      ERROR_PATH_NOT_FOUND
#else
      0
#endif
   ),
   m_opNotFound(opNotFound) {
   what_writer().print(ABC_SL("path not found=\"{}\""), m_opNotFound);
}

path_not_found::path_not_found(path_not_found const & x) :
   generic_error(x),
   m_opNotFound(x.m_opNotFound) {
}

/*virtual*/ path_not_found::~path_not_found() ABC_STL_NOEXCEPT_TRUE() {
}

path_not_found & path_not_found::operator=(path_not_found const & x) {
   generic_error::operator=(x);
   m_opNotFound = x.m_opNotFound;
   return *this;
}

}} //namespace abc::os

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace os {

#if ABC_HOST_API_WIN32

//! Value returned by abc::os::is_nt() under Windows.
static bool g_bIsNt = false;
//! Value returned by abc::os::version() under Windows.
static std::uint32_t g_iVer = 0;

//! Initializes g_bIsNt and g_iVer.
static void get_is_nt_and_version() {
   std::uint32_t iVer = ::GetVersion();
   std::uint8_t iMajor = static_cast<std::uint8_t>(iVer);
   // Extract the build number.
   std::uint16_t iBuild;
   if (iMajor != 4) {
      // Windows NT or Win32s.
      iBuild = static_cast<std::uint16_t>((iVer & 0x7fff0000) >> 16);
   } else {
      // Windows 9x.
      iBuild =  0;
   }
   /* No need to use a mutex here, since these writes are atomic and the values written to them
   will be equal for all threads in the process. */
   g_bIsNt = ((iVer & 0x80000000) == 0);
   g_iVer =
      (static_cast<std::uint32_t>(iMajor) << 24) |
      (static_cast<std::uint32_t>(iVer & 0x0000ff00) << 8) |
         static_cast<std::uint32_t>(iBuild);
}

static ::HKEY open_registry_key(::HKEY hkeyParent, str const & sName) {
   ::HKEY hkeyRet;
   if (long iRet = ::RegOpenKeyEx(hkeyParent, sName.c_str(), 0, KEY_QUERY_VALUE, &hkeyRet)) {
      if (iRet == ERROR_FILE_NOT_FOUND) {
         return nullptr;
      }
      exception::throw_os_error(static_cast<errint_t>(iRet));
   }
   return hkeyRet;
}

static bool get_registry_value_raw(
   ::HKEY hkey, char_t const * pszName, ::DWORD * piType, void * pValue, ::DWORD * pcbValue
) {
   if (long iRet = ::RegQueryValueEx(
      hkey, pszName, nullptr, piType, static_cast< ::BYTE *>(pValue), pcbValue
   )) {
      if (iRet == ERROR_FILE_NOT_FOUND) {
         return false;
      } else {
         exception::throw_os_error(static_cast<errint_t>(iRet));
      }
   }
   return true;
}

bool get_registry_value(::HKEY hkeyParent, str const & sKey, str const & sName, str * psRet) {
   ::HKEY hkey = open_registry_key(hkeyParent, sKey);
   if (!hkey) {
      psRet->clear();
      return false;
   }
   auto deferred1(defer_to_scope_end([hkey] () {
      ::RegCloseKey(hkey);
   }));
   // TODO: use Nt* functions to avoid the limitation of NUL termination.
   auto csName(sName.c_str());
   ::DWORD iTypeProbe, cbValueProbe;
   if (!get_registry_value_raw(hkey, csName, &iTypeProbe, nullptr, &cbValueProbe)) {
      psRet->clear();
      return false;
   }
   for (;;) {
      ::DWORD iTypeFinal, cbValueFinal = cbValueProbe;
      std::size_t cchValue = cbValueProbe / sizeof(char_t);
      switch (iTypeProbe) {
         case REG_SZ:
            psRet->set_size_in_chars(cchValue, false);
            if (!get_registry_value_raw(hkey, csName, &iTypeFinal, psRet->data(), &cbValueFinal)) {
               // Race condition detected: somebody else deleted the value between our queries.
               psRet->clear();
               return false;
            }
            if (iTypeFinal != iTypeProbe || cbValueFinal != cbValueProbe) {
               // Race condition detected: somebody else changed the value between our queries.
               break;
            }
            /* If *psRet ended up including a NUL terminator because the value did, strip it; str
            doesn’t need it. */
            // TODO: use ends_with(char_t) when it becomes avaiable.
            if (psRet->ends_with(ABC_SL("\0"))) {
               psRet->set_size_in_chars(cchValue - 1, false);
            }
            return true;
         case REG_EXPAND_SZ: {
            text::sstr<256> sUnexpanded;
            sUnexpanded.set_size_in_chars(cchValue, false);
            if (!get_registry_value_raw(
               hkey, csName, &iTypeFinal, sUnexpanded.data(), &cbValueFinal
            )) {
               // Race condition detected: somebody else deleted the value between our queries.
               psRet->clear();
               return false;
            }
            if (iTypeFinal != iTypeProbe || cbValueFinal != cbValueProbe) {
               // Race condition detected: somebody else changed the value between our queries.
               break;
            }
            /* If sUnexpanded ended up including a NUL terminator because the value did, strip it;
            str doesn’t need it. */
            // TODO: use ends_with(char_t) when it becomes avaiable.
            if (sUnexpanded.ends_with(ABC_SL("\0"))) {
               sUnexpanded.set_size_in_chars(cchValue - 1, false);
            }
            // Expand any environment variables.
            auto csUnexpanded(sUnexpanded.c_str());
            psRet->set_from([&csUnexpanded] (char_t * pch, std::size_t cchMax) -> std::size_t {
               ::DWORD cchExpanded = ::ExpandEnvironmentStrings(
                  csUnexpanded, pch, static_cast< ::DWORD>(cchMax)
               );
               if (!cchExpanded) {
                  exception::throw_os_error();
               }
               return cchExpanded;
            });
            return true;
         }
         default:
            // TODO: use a better exception class.
            ABC_THROW(generic_error, ());
      }
      // Start over by switching to the new type and size.
      iTypeProbe = iTypeFinal;
      cbValueProbe = cbValueFinal;
   }
}

bool is_nt() {
   if (g_iVer == 0) {
      get_is_nt_and_version();
   }
   return g_bIsNt;
}

std::uint32_t version() {
   if (g_iVer == 0) {
      get_is_nt_and_version();
   }
   return g_iVer;
}

#endif

}} //namespace abc::os
