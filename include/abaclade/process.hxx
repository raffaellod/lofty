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

/*! Models a process (“task” on some platforms), typically a child process. Supports cooperation
with abc::event_loop. */
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

   //! Pointer to a process.
   class ABACLADE_SYM const_pointer;

   //! Pointer to a non-const process.
   class ABACLADE_SYM pointer;

public:
   //! Constructor.
   process();

   //! Destructor.
   ~process();

   /*! Returns a system-wide unique ID for the process handle.

   @return
      Unique ID representing the process.
   */
   id_type id() const
#if ABC_HOST_API_POSIX
   {
      // ID == “pointer”.
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

   /*! Returns a pointer to the process.

   @return
      Process pointer.
   */
   pointer ptr();
   const_pointer ptr() const;

private:
   //! OS-dependent ID/handle.
   native_handle_type m_h;
};

// Now these can be defined.

class ABACLADE_SYM process::const_pointer {
public:
   /*! Constructor.

   @param pthr
      Thread pointer to wrap.
   */
   const_pointer() {
   }
   const_pointer(process const * pproc) {
      if (pproc) {
         m_proc.m_h = pproc->m_h;
      }
   }

   /*! Dereferencing member access operator.

   @return
      Raw pointer to the process.
   */
   process const * operator->() const {
      return &m_proc;
   }

   bool operator==(const_pointer const & p) const {
      /* Make both processes const to avoid using the non-const id() which requires pointer to be
      defined ‒ it isn’t yet at this point. */
      return const_cast<process const &>(m_proc).id() == const_cast<process const &>(p.m_proc).id();
   }

   bool operator!=(const_pointer const & p) const {
      return !operator==(p);
   }

   /*! Returns the underlying ID/handle type.

   @return
      OS-dependent ID/handle.
   */
   native_handle_type native() const {
      return m_proc.m_h;
   }

protected:
   //! Process wrapper.
   process mutable m_proc;
};

//! Pointer to a non-const process.
class ABACLADE_SYM process::pointer : public const_pointer {
public:
   /*! Constructor.
   */
   pointer() {
   }
   pointer(process * pproc) :
      const_pointer(pproc) {
   }

   /*! Dereferencing member access operator.

   @return
      Raw pointer to the process.
   */
   process * operator->() const {
      return &m_proc;
   }
};

// Now these can be implemented.

inline process::pointer process::ptr() {
   return pointer(this);
}
inline process::const_pointer process::ptr() const {
   return const_pointer(this);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_PROCESS_HXX
