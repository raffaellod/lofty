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

#ifndef _ABACLADE_OS_HXX
#define _ABACLADE_OS_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/os/path.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! Provides facilities to interact with the underlying OS.
namespace os {}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace os {

//! A path failed validation. Path validation is typically file system- or OS-dependent.
class ABACLADE_SYM invalid_path : public generic_error {
public:
   /*! Constructor.

   @param opInvalid
      Path that failed validation.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit invalid_path(abc::os::path const & opInvalid, errint_t err = 0);

   /*! Copy onstructor.

   @param x
      Source object.
   */
   invalid_path(invalid_path const & x);

   //! Destructor.
   virtual ~invalid_path() ABC_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   invalid_path & operator=(invalid_path const & x);

   /*! Returns the path that couldn’t be found.

   @return
      Path that couldn’t be found at the moment it was accessed.
   */
   os::path const & path() const {
      return m_opInvalid;
   }

private:
   //! Path that caused the error.
   os::path m_opInvalid;
};

}} //namespace abc::os

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace os {

//! A path could not be found on the file system.
class ABACLADE_SYM path_not_found : public generic_error {
public:
   /*! Constructor.

   @param opNotFound
      Path that couldn’t be found.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit path_not_found(abc::os::path const & opNotFound, errint_t err = 0);

   /*! Copy onstructor.

   @param x
      Source object.
   */
   path_not_found(path_not_found const & x);

   //! Destructor.
   virtual ~path_not_found() ABC_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   path_not_found & operator=(path_not_found const & x);

   /*! Returns the path that couldn’t be found.

   @return
      Path that couldn’t be found at the moment it was accessed.
   */
   os::path const & path() const {
      return m_opNotFound;
   }

private:
   //! Path that caused the error.
   os::path m_opNotFound;
};

}} //namespace abc::os

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace os {

#if ABC_HOST_API_WIN32

/*! Determines whether the process is running under Windows NT or an older, non-NT version of
Windows.

@return
   true if running under Windows NT, or false otherwise.
*/
ABACLADE_SYM bool is_nt();

/*! Returns a Windows Registry value.

@param hkeyParent
   Parent Registry key, or an HKEY_* constant.
@param sKey
   Path to the key, relative to hkeyParent.
@param sName
   Name of the value to retrieve.
@param psRet
   Destination where to store the value retrieved from the Registry.
@return bool
   true if the value was found, or false otherwise.
*/
ABACLADE_SYM bool get_registry_value(
   ::HKEY hkeyParent, str const & sKey, str const & sName, str * psRet
);

/*! Returns the Windows version that’s running the process.

@return
   Windows version in the format 0xMMmmbbbb, where MM is the major version, mm is the minor version,
   and bbbb is the build number.
*/
ABACLADE_SYM std::uint32_t version();

#endif

}} //namespace abc::os

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_OS_HXX
