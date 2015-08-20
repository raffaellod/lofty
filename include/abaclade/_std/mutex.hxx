/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015
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

#ifndef _ABACLADE_STD_MUTEX_HXX
#define _ABACLADE_STD_MUTEX_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std {

//! Non-recursive mutex with exlusive ownership semantics (C++11 § 30.4.1.2.1 “Class mutex”).
class mutex {
public:
   //! Default constructor (C++11 § 30.4.1.2.1 “Class mutex”).
   mutex() {
      ::InitializeCriticalSection(&m_cs);
   }

   //! Destructor (C++11 § 30.4.1.2.1 “Class mutex”).
   ~mutex() {
      ::DeleteCriticalSection(&m_cs);
   }

   //! Acquires a lock on the mutex (C++11 § 30.4.1.2.1 “Class mutex”).
   void lock() {
      ::EnterCriticalSection(&m_cs);
   }

   //! Releases the lock on the mutex (C++11 § 30.4.1.2.1 “Class mutex”).
   void unlock() {
      ::LeaveCriticalSection(&m_cs);
   }

private:
#if ABC_HOST_API_WIN32
   //! Win32 implementation of a mutex.
   ::CRITICAL_SECTION m_cs;
#else
   #error "TODO: HOST_API"
#endif
};

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std {

//! Fully-automatic mutex lock (C++11 § 30.4.2.1 “Class template lock_guard”).
template <typename TMutex>
class lock_guard : public noncopyable {
public:
   /*! Constructor.

   @param mtx
      Mutex to lock.
   */
   explicit lock_guard(TMutex & mtx) :
      m_pmtx(&mtx) {
      m_pmtx->lock();
   }

   //! Destructor.
   ~lock_guard() {
      m_pmtx->unlock();
   }

private:
   //! Pointer to the locked mutex.
   TMutex * m_pmtx;
};

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std {

//! Mutex lock with guaranteed release (C++11 § 30.4.2.2 “Class template unique_lock”).
template <typename TMutex>
class unique_lock : public noncopyable {
public:
   /*! Constructor (C++11 § 30.4.2.2.1 “construct/copy/destroy”).

   @param mtx
      Mutex to lock.
   */
   explicit unique_lock(TMutex & mtx) :
      m_mtx(&mtx),
      m_bOwnsLock(true) {
      m_mtx->lock();
   }

   //! Destructor (C++11 § 30.4.2.2.1 “construct/copy/destroy”).
   ~unique_lock() {
      if (m_bOwnsLock) {
         m_mtx->lock();
      }
   }

   /*! Returns true if *this currently holds a lock on the associated mutex (C++11 § 30.4.2.2.4
   “observers”).

   @return
      true if the mutex is locked by *this, or false otherwise.
   */
   bool owns_lock() const {
      return m_bOwnsLock;
   }

   //! Acquires a lock on the associated mutex (C++11 § 30.4.2.2.2 “locking”).
   void lock() {
      if (m_bOwnsLock) {
         // TODO: else throw std::system_error with code std::errc::operation_not_permitted.
         return;
      }
      m_mtx->lock();
   }

   //! Releases the lock on the associated mutex (C++11 § 30.4.2.2.2 “locking”).
   void unlock() {
      if (!m_bOwnsLock) {
         // TODO: else throw std::system_error with code std::errc::operation_not_permitted.
         return;
      }
      m_mtx->unlock();
   }

private:
   //! Pointer to the locked mutex.
   TMutex * m_mtx;
   //! true if *m_mtx is currently locked by *this, or false otherwise.
   bool m_bOwnsLock;
};

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_STD_MUTEX_HXX
