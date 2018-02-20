/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_EVENT_HXX
#define _LOFTY_EVENT_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/coroutine.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! Event that can be waited for by a thread or coroutine (exclusive “or”).

An event can be waited for or triggered only after calling create() on it; failure to do so will result in an
exception being thrown by trigger() and wait().

If a coroutine scheduler is attached to the thread that calls create(), the event will become a coroutine
event, meaning it can only be waited on by a coroutine. If no coroutine scheduler is present, the event will
become a thread event, meaning it can only be waited on by a thread (not running coroutines). */
class LOFTY_SYM event : public support_explicit_operator_bool<event>, public noncopyable {
public:
   //! Opaque id type.
   typedef std::uintptr_t id_type;

   //! Type of manual_create.
   struct manual_create_t {
      //! Default constructor. Required to instantiate a const instance.
      manual_create_t() {
      }
   };

public:
   //! Default constructor; automatically creates the event.
   event() :
      id(0) {
      create();
   }

   //! Constructor that does not automatically create the event. Call the create() method to do so.
   explicit event(manual_create_t const &) :
      id(0) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   event(event && src);

   //! Destructor.
   ~event();

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   event & operator=(event && src);

   /*! Boolean evaluation operator.

   @return
      true if create() has been invoked, or false otherwise.
   */
   LOFTY_EXPLICIT_OPERATOR_BOOL() const {
      return id != 0;
   }

   //! Creates the event, allowing for trigger() and wait() to be invoked on it.
   event & create();

   //! Triggers the event, unblocking any threads/coroutines waiting for it.
   void trigger();

   /*! Waits for the event to be triggered.

   @param timeout_millisecs
      Time after which the wait will be interrupted by an exception of type lofty::io::timeout.
   */
   void wait(unsigned timeout_millisecs = 0);

private:
   /*! Returns true if the event is using a coroutine scheduler, i.e. it can only be waited on by a coroutine,
   or false if it’s not, i.e. it can only be waited on by a thread not running coroutines.

   @return
      false if coro_sched_w is nullptr, or true otherwise.
   */
   bool using_coro_sched() const {
      _std::weak_ptr<coroutine::scheduler> null_coro_sched;
      return coro_sched_w.owner_before(null_coro_sched) || null_coro_sched.owner_before(coro_sched_w);
   }

public:
   /*! If provided as a constructor argument, it causes the event to not be automatically created. In order to
   use the event, its create() method will need to be called manually. */
   static manual_create_t const manual_create;

private:
   /*! Scheduler that owns the event id. Stored for performance (avoid thread-local storage) and to allow one
   thread/scheduler to unblock a coroutine in a different thread/scheduler. */
   _std::weak_ptr<coroutine::scheduler> coro_sched_w;
   //! Event id.
   id_type id;
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_EVENT_HXX
