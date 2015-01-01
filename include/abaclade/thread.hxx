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

/*! Models a thread of execution. Replacement for std::thread supporting cooperation with
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

   //! Pointer to a thread. Unlike abc::thread, instances of this class may be copied.
   class ABACLADE_SYM const_pointer;

   //! Pointer to a non-const thread.
   class ABACLADE_SYM pointer;

public:
   //! Constructor.
   thread();

   //! Destructor.
   ~thread();

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

   /*! Returns a pointer to the thread.

   @return
      Thread pointer.
   */
   pointer ptr();
   const_pointer ptr() const;

private:
   //! OS-dependent ID/handle.
   native_handle_type m_h;
#if ABC_HOST_API_POSIX
   /*! Since there’s no “uninitialized” pthread_t value, this tracks whether m_h is a valid
   pthread_t. */
   bool m_bJoinable;
#endif
};

// Now these can be defined.

//! Pointer to a thread. Unlike abc::thread, instances of this class may be copied.
class ABACLADE_SYM thread::const_pointer {
public:
   /*! Constructor.

   @param pthr
      Thread pointer to wrap.
   */
   const_pointer() {
   }
   const_pointer(thread const * pthr) {
      if (pthr) {
         m_thr.m_h = pthr->m_h;
      }
   }

   //! Destructor.
   ~const_pointer() {
      // Ensure that m_thr is not joinable, so its destructor won’t abort this process.
#if ABC_HOST_API_POSIX
      m_thr.m_bJoinable = false;
#elif ABC_HOST_API_WIN32
      m_thr.m_h = nullptr;
#else
   #error "TODO: HOST_API"
#endif
   }

   /*! Dereferencing member access operator.

   @return
      Raw pointer to the thread.
   */
   thread const * operator->() const {
      return &m_thr;
   }

   bool operator==(const_pointer const & p) const
#if ABC_HOST_API_POSIX
   {
      return ::pthread_equal(m_thr.m_h, p.m_thr.m_h);
   }
#elif ABC_HOST_API_WIN32
   // Defined in the .cxx file.
   ;
#else
   #error "TODO: HOST_API"
#endif

   bool operator!=(const_pointer const & p) const {
      return !operator==(p);
   }

   /*! Returns the underlying ID/handle type.

   @return
      OS-dependent ID/handle.
   */
   native_handle_type native() const {
      return m_thr.m_h;
   }

protected:
   //! Thread wrapper.
   thread mutable m_thr;
};

class ABACLADE_SYM thread::pointer : public const_pointer {
public:
   /*! Constructor.
   */
   pointer() {
   }
   pointer(thread * pthr) :
      const_pointer(pthr) {
   }

   /*! Dereferencing member access operator.

   @return
      Raw pointer to the thread.
   */
   thread * operator->() const {
      return &m_thr;
   }
};

// Now these can be implemented.

inline thread::pointer thread::ptr() {
   return pointer(this);
}
inline thread::const_pointer thread::ptr() const {
   return const_pointer(this);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_THREAD_HXX
