﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

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

#ifndef _ABACLADE_COROUTINE_HXX
#define _ABACLADE_COROUTINE_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/collections/map.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::coroutine

namespace abc {

/*! Subroutine for use in non-preemptive multitasking, enabling asynchronous I/O in most abc::io
classes. */
class ABACLADE_SYM coroutine : public noncopyable {
private:
   friend class coroutine_scheduler;

public:
   //! OS-dependent execution context for the coroutine.
   class context;

public:
   coroutine();
   explicit coroutine(std::function<void ()> fnMain);
   coroutine(coroutine && coro) :
      m_pctx(std::move(coro.m_pctx)) {
   }

   //! Destructor.
   ~coroutine();

private:
   //! Pointer to the coroutine’s execution context.
   std::shared_ptr<context> m_pctx;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::coroutine_scheduler

namespace abc {

class ABACLADE_SYM coroutine_scheduler : public noncopyable {
public:
   //! Constructor.
   coroutine_scheduler();

   //! Destructor.
   virtual ~coroutine_scheduler();

   void add_coroutine(coroutine const & coro);

   static coroutine_scheduler & attach_to_current_thread(
      std::shared_ptr<coroutine_scheduler> pcorosched = nullptr
   );

   static std::shared_ptr<coroutine_scheduler> const & get_for_current_thread() {
      return sm_pcorosched;
   }

   virtual void run() = 0;

   virtual void yield_while_async_pending(io::filedesc const & fd, bool bWrite) = 0;

protected:
   //! Pointer to the active (current) coroutine, or nullptr if none is active.
   std::shared_ptr<coroutine::context> m_pcoroctxActive;
   //! List of coroutines that have been scheduled, but have not been started yet.
   collections::list<std::shared_ptr<coroutine::context>> m_listStartingCoros;
   //! Pointer to the coroutine_scheduler for the current thread.
   static thread_local_value<std::shared_ptr<coroutine_scheduler>> sm_pcorosched;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COROUTINE_HXX