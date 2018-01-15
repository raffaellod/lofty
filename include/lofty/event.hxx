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

//! Event that can be waited for by a thread or coroutine.
class LOFTY_SYM event : public noncopyable {
public:
   //! Opaque id type.
   typedef std::uintptr_t id_type;

public:
   //! Default constructor.
   event();

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

   //! Triggers the event, unblocking any threads/coroutines waiting for it.
   void trigger();

   /*! Waits for the event to be triggered.

   @param timeout_millisecs
      Time after which the wait will be interrupted by an exception of type lofty::io::timeout.
   */
   void wait(unsigned timeout_millisecs = 0);

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
