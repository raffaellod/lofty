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

#ifndef _ABACLADE_THREAD_HXX
#define _ABACLADE_THREAD_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#if ABC_HOST_API_POSIX
   #include <pthread.h>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::thread

namespace abc {

/*! Thread of program execution. Replacement for std::thread supporting cooperation with
abc::event_loop. */
class ABACLADE_SYM thread {
public:
   //! Underlying OS-dependent ID/handle type.
#if ABC_HOST_API_POSIX
   typedef pthread_t native_handle_type;
#elif ABC_HOST_API_WIN32
   typedef HANDLE native_handle_type;
#else
   #error "TODO: HOST_API"
#endif

public:
   //! Constructor.
   thread();

   //! Destructor.
   ~thread();

   /*! Equality relational operator.

   @param thr
      Object to compare to *this.
   @return
      true if *this refers to the same thread as thr, or false otherwise.
   */
   bool operator==(thread const & thr) const
#if ABC_HOST_API_POSIX
   {
      return ::pthread_equal(m_h, thr.m_h);
   }
#elif ABC_HOST_API_WIN32
   // Defined in the .cxx file.
   ;
#else
   #error "TODO: HOST_API"
#endif

   /*! Inequality relational operator.

   @param thr
      Object to compare to *this.
   @return
      true if *this refers to a different thread than thr, or false otherwise.
   */
   bool operator!=(thread const & thr) const {
      return !operator==(thr);
   }

   void join();

   /*! Returns true if calling join() on the object is allowed.

   @return
      true if the object is in a joinable state, or false otherwise.
   */
   bool joinable() const {
#if ABC_HOST_API_POSIX
      return m_bJoinable;
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
#if ABC_HOST_API_POSIX
   /*! Since there’s no “uninitialized” pthread_t value, this tracks whether m_h is a valid
   pthread_t. */
   bool m_bJoinable;
#endif
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_THREAD_HXX
