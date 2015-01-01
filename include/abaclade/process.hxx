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
class ABACLADE_SYM process : public noncopyable {
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
   /*! Constructor.

   @param id
      ID of a running process to associate this abc::process instance with.
   @param proc
      Source process.
   */
   process() :
      m_h(smc_hNull) {
   }
   explicit process(id_type id);
   process(process && proc) :
      m_h(proc.m_h) {
      proc.m_h = smc_hNull;
   }

   //! Destructor.
   ~process();

   /*! Assignment operator.

   @param proc
      Source process.
   @return
      *this.
   */
   process & operator=(process && proc) {
      ABC_TRACE_FUNC(this/*, proc*/);

      native_handle_type h(proc.m_h);
      detach();
      m_h = h;
      proc.m_h = smc_hNull;
      return *this;
   }

   /*! Equality relational operator.

   @param proc
      Object to compare to *this.
   @return
      true if *this has the same id() as proc, or false otherwise.
   */
   bool operator==(process const & proc) const {
      ABC_TRACE_FUNC(this/*, proc*/);

      return id() == proc.id();
   }

   /*! Inequality relational operator.

   @param proc
      Object to compare to *this.
   @return
      true if *this has a different id() than proc, or false otherwise.
   */
   bool operator!=(process const & proc) const {
      ABC_TRACE_FUNC(this/*, proc*/);

      return !operator==(proc);
   }

   /*! Releases the OS-dependent ID/handle, making *this reference no process and invalidating the
   value returned by native_handle(). */
   void detach();

   /*! Returns a system-wide unique ID for the process handle.

   @return
      Unique ID representing the process.
   */
   id_type id() const;

   //! Waits for the process to terminate.
   void join();

   /*! Returns true if calling join() on the object is allowed.

   @return
      true if the object is in a joinable state, or false otherwise.
   */
   bool joinable() const;

   /*! Returns the underlying ID/handle type.

   @return
      OS-dependent ID/handle.
   */
   native_handle_type native_handle() const {
      ABC_TRACE_FUNC(this);

      return m_h;
   }

private:
   //! OS-dependent ID/handle.
   native_handle_type m_h;
   //! Logically null ID/handle.
   static native_handle_type const smc_hNull;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_PROCESS_HXX
