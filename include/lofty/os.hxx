/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_OS_HXX
#define _LOFTY_OS_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/os/path.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Provides facilities to interact with the underlying OS.
namespace os {}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace os {

//! A path failed validation. Path validation is typically file system- or OS-dependent.
class LOFTY_SYM invalid_path : public generic_error {
public:
   /*! Constructor.

   @param path
      Path that failed validation.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit invalid_path(lofty::os::path const & path, errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   invalid_path(invalid_path const & src);

   //! Destructor.
   virtual ~invalid_path() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   invalid_path & operator=(invalid_path const & src);

   /*! Returns the path that couldn’t be found.

   @return
      Path that couldn’t be found at the moment it was accessed.
   */
   os::path const & path() const {
      return path_;
   }

private:
   //! Path that caused the error.
   os::path path_;
};

}} //namespace lofty::os

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace os {

//! A path could not be found on the file system.
class LOFTY_SYM path_not_found : public generic_error {
public:
   /*! Constructor.

   @param path
      Path that couldn’t be found.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit path_not_found(lofty::os::path const & path, errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   path_not_found(path_not_found const & src);

   //! Destructor.
   virtual ~path_not_found() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   path_not_found & operator=(path_not_found const & src);

   /*! Returns the path that couldn’t be found.

   @return
      Path that couldn’t be found at the moment it was accessed.
   */
   os::path const & path() const {
      return path_;
   }

private:
   //! Path that caused the error.
   os::path path_;
};

}} //namespace lofty::os

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace os {

#if LOFTY_HOST_API_WIN32

/*! Returns a Windows Registry value.

@param parent_hkey
   Parent Registry key, or an HKEY_* constant.
@param key_path
   Path to the key, relative to parent_hkey.
@param name
   Name of the value to retrieve.
@param out
   Destination where to store the value retrieved from the Registry.
@return bool
   true if the value was found, or false otherwise.
*/
LOFTY_SYM bool get_registry_value(::HKEY parent_hkey, str const & key_path, str const & name, str * out);

#endif

}} //namespace lofty::os

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_OS_HXX
