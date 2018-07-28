/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_MUTEX_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_MUTEX_HXX
#endif

#ifndef _LOFTY_MUTEX_HXX_NOPUB
#define _LOFTY_MUTEX_HXX_NOPUB

#include <lofty/coroutine.hxx>
#include <lofty/explicit_operator_bool.hxx>
#include <lofty/_std/memory.hxx>
#include <lofty/_std/mutex.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {
_LOFTY_PUBNS_BEGIN

/*! Mutex that can be locked/unlocked by a thread or coroutine (exclusive “or”).

A mutex can be locked and unlocked only after calling create() on it; failure to do so will result in an
exception being thrown by lock(), try_lock() and unlock().

If a coroutine scheduler is attached to the thread that calls create(), the mutex will become a coroutine
mutex, meaning it can only be locked/unlocked by a coroutine. If no coroutine scheduler is present, the mutex
will become a thread mutex, meaning it can only be locked/unlocked by a thread (not running coroutines). */
class LOFTY_SYM mutex : public support_explicit_operator_bool<mutex>, public noncopyable {
public:
   //! Coroutine mode implementation data.
   struct coro_mode_t;

   //! Type of manual_create.
   struct manual_create_t {
      //! Default constructor. Required to instantiate a const instance.
      manual_create_t() {
      }
   };

public:
   //! Default constructor; automatically creates the mutex.
   mutex();

   //! Constructor that does not automatically create the mutex. Call the create() method to do so.
   explicit mutex(manual_create_t const &);

   /*! Move constructor.

   @param src
      Source object.
   */
   mutex(mutex && src);

   //! Destructor.
   ~mutex();

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   mutex & operator=(mutex && src);

   /*! Boolean evaluation operator.

   @return
      true if create() has been invoked, or false otherwise.
   */
   LOFTY_EXPLICIT_OPERATOR_BOOL() const {
      return thread_mutex != nullptr;
   }

   //! Creates the mutex, allowing for lock(), try_lock() and unlock() to be invoked on it.
   mutex & create();

   //! Acquires the mutex, blocking if necessary.
   void lock();

   /*! Attempts to acquire the mutex, returning immediately if that’s not possible.

   @return
      true if the mutex was locked and is now owned by the caller, or false otherwise.
   */
   bool try_lock();

   //! Releases the mutex.
   void unlock();

public:
   /*! If provided as a constructor argument, it causes the mutex to not be automatically created. In order to
   use the mutex, its create() method will need to be called manually. */
   static manual_create_t const manual_create;

private:
   //! Underlying mutex for thread mode, or controls access to coro_mode in coroutine mode.
   _std::_LOFTY_PUBNS unique_ptr<_std::_LOFTY_PUBNS mutex> thread_mutex;
   //! Pointer to the implementation instance.
   _std::_LOFTY_PUBNS unique_ptr<coro_mode_t> coro_mode;
};

_LOFTY_PUBNS_END
} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_MUTEX_HXX_NOPUB

#ifdef _LOFTY_MUTEX_HXX
   #undef _LOFTY_NOPUB

   namespace lofty {

   using _pub::mutex;

   }

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_MUTEX_HXX
