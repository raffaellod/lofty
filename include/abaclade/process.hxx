/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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

#ifndef _ABACLADE_PROCESS_HXX
#define _ABACLADE_PROCESS_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::process

namespace abc {

//! Process (“task” on some platforms), typically a child spawned by the current process.
class ABACLADE_SYM process {
public:
   //! Underlying OS-dependent ID/handle type.
#if ABC_HOST_API_POSIX
   typedef int native_handle_type;
#elif ABC_HOST_API_WIN32
   typedef HANDLE native_handle_type;
#else
   #error "TODO: HOST_API"
#endif

   //! OS-dependent type for unique process IDs.
#if ABC_HOST_API_POSIX
   typedef int id_type;
#elif ABC_HOST_API_WIN32
   typedef DWORD id_type;
#else
   #error "TODO: HOST_API"
#endif

public:
   //! Constructor.
   process();

   //! Destructor.
   ~process();

   /*! Equality relational operator.

   @param proc
      Object to compare to *this.
   @return
      true if *this has the same id() as proc, or false otherwise.
   */
   bool operator==(process const & proc) const {
      return id() == proc.id();
   }

   /*! Inequality relational operator.

   @param proc
      Object to compare to *this.
   @return
      true if *this has a different id() than proc, or false otherwise.
   */
   bool operator!=(process const & proc) const {
      return !operator==(proc);
   }

   /*! Returns a system-wide unique ID for the process handle.

   @return
      Unique ID representing the process.
   */
   id_type id() const
#if ABC_HOST_API_POSIX
   {
      // ID == native handle.
      return m_h;
   }
#elif ABC_HOST_API_WIN32
   // Defined in the .cxx file.
   ;
#else
   #error "TODO: HOST_API"
#endif

   void join();

   /*! Returns true if calling join() on the object is allowed.

   @return
      true if the object is in a joinable state, or false otherwise.
   */
   bool joinable() const {
#if ABC_HOST_API_POSIX
      return m_h != 0;
#elif ABC_HOST_API_WIN32
      return m_h != nullptr;
#else
   #error "TODO: HOST_API"
#endif
   }

   /*! Returns the underlying ID/handle type.

   @return
      OS-dependent ID/handle.
   */
   native_handle_type native_handle() const {
      return m_h;
   }

private:
   //! OS-dependent ID/handle.
   native_handle_type m_h;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_PROCESS_HXX
