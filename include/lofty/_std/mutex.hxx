/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_STD_MUTEX_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_STD_MUTEX_HXX
#endif

#ifndef _LOFTY_STD_MUTEX_HXX_NOPUB
#define _LOFTY_STD_MUTEX_HXX_NOPUB

#include <lofty/_pvt/lofty.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_STL_LOFTY || LOFTY_HOST_STL_MSVCRT == 1600

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

//! Non-recursive mutex with exlusive ownership semantics (C++11 § 30.4.1.2.1 “Class mutex”).
class mutex {
public:
   //! Default constructor (C++11 § 30.4.1.2.1 “Class mutex”).
   mutex() {
#if LOFTY_HOST_API_WIN32
      ::InitializeCriticalSection(&cs);
#else
   #error "TODO: HOST_API"
#endif
   }

   //! Destructor (C++11 § 30.4.1.2.1 “Class mutex”).
   ~mutex() {
#if LOFTY_HOST_API_WIN32
      ::DeleteCriticalSection(&cs);
#else
   #error "TODO: HOST_API"
#endif
   }

   //! Acquires a lock on the mutex (C++11 § 30.4.1.2.1 “Class mutex”).
   void lock() {
#if LOFTY_HOST_API_WIN32
      ::EnterCriticalSection(&cs);
#else
   #error "TODO: HOST_API"
#endif
   }

   //! Releases the lock on the mutex (C++11 § 30.4.1.2.1 “Class mutex”).
   void unlock() {
#if LOFTY_HOST_API_WIN32
      ::LeaveCriticalSection(&cs);
#else
   #error "TODO: HOST_API"
#endif
   }

private:
#if LOFTY_HOST_API_WIN32
   //! Win32 implementation of a mutex.
   ::CRITICAL_SECTION cs;
#else
   #error "TODO: HOST_API"
#endif
};

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

//! Fully-automatic mutex lock (C++11 § 30.4.2.1 “Class template lock_guard”).
template <typename TMutex>
class lock_guard : public noncopyable {
public:
   /*! Constructor.

   @param mtx_
      Mutex to lock.
   */
   explicit lock_guard(TMutex & mtx_) :
      mtx(&mtx_) {
      mtx->lock();
   }

   //! Destructor.
   ~lock_guard() {
      mtx->unlock();
   }

private:
   //! Pointer to the locked mutex.
   TMutex * mtx;
};

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

//! Mutex lock with guaranteed release (C++11 § 30.4.2.2 “Class template unique_lock”).
template <typename TMutex>
class unique_lock : public noncopyable {
public:
   /*! Constructor (C++11 § 30.4.2.2.1 “construct/copy/destroy”).

   @param mtx_
      Mutex to lock.
   */
   explicit unique_lock(TMutex & mtx_) :
      mtx(&mtx_),
      owns_lock_(true) {
      mtx->lock();
   }

   //! Destructor (C++11 § 30.4.2.2.1 “construct/copy/destroy”).
   ~unique_lock() {
      if (owns_lock_) {
         mtx->lock();
      }
   }

   /*! Returns true if *this currently holds a lock on the associated mutex (C++11 § 30.4.2.2.4 “observers”).

   @return
      true if the mutex is locked by *this, or false otherwise.
   */
   bool owns_lock() const {
      return owns_lock_;
   }

   //! Acquires a lock on the associated mutex (C++11 § 30.4.2.2.2 “locking”).
   void lock() {
      if (owns_lock_) {
         // TODO: else throw std::system_error with code std::errc::operation_not_permitted.
         return;
      }
      mtx->lock();
   }

   //! Releases the lock on the associated mutex (C++11 § 30.4.2.2.2 “locking”).
   void unlock() {
      if (!owns_lock_) {
         // TODO: else throw std::system_error with code std::errc::operation_not_permitted.
         return;
      }
      mtx->unlock();
   }

private:
   //! Pointer to the locked mutex.
   TMutex * mtx;
   //! true if *mtx is currently locked by *this, or false otherwise.
   bool owns_lock_;
};

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#else //if LOFTY_HOST_STL_LOFTY || LOFTY_HOST_STL_MSVCRT == 1600
   #include <mutex>

   namespace lofty { namespace _std { namespace _pub {

   using ::std::lock_guard;
   using ::std::mutex;
   using ::std::unique_lock;

   }}}
#endif //if LOFTY_HOST_STL_LOFTY || LOFTY_HOST_STL_MSVCRT == 1600 … else

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_STD_MUTEX_HXX_NOPUB

#ifdef _LOFTY_STD_MUTEX_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace _std {

   using _pub::lock_guard;
   using _pub::mutex;
   using _pub::unique_lock;

   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_STD_MUTEX_HXX
