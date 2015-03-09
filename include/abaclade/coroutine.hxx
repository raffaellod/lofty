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

#ifndef _ABACLADE_COROUTINE_HXX
#define _ABACLADE_COROUTINE_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/collections/map.hxx>

#if ABC_HOST_API_POSIX
   #include <errno.h> // EINTR errno
   #if ABC_HOST_API_BSD
      #include <sys/event.h>
      #include <sys/time.h>
      #include <sys/types.h>
   #elif ABC_HOST_API_LINUX
      #include <sys/epoll.h>
      #include <ucontext.h>
   #endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::timer

namespace abc {

class ABACLADE_SYM coroutine : public noncopyable {
private:
   friend class coroutine_scheduler;

   class ABACLADE_SYM shared_data : public noncopyable {
   public:
      shared_data(::ucontext_t * puctxReturn, std::function<void ()> fnMain);

   private:
      static void outer_main(void * p);

   public:
      ::ucontext_t m_uctx;
   private:
      // TODO: use MINSIGSTKSZ.
      abc::max_align_t m_aiStack[1024];
      // TODO: copy abc::thread callback patterh.
      std::function<void ()> m_fnInnerMain;
   };
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
   ~coroutine_scheduler();

   void add_coroutine(std::function<void ()> fnMain) {
      std::unique_ptr<coroutine::shared_data> pcorod(
         new coroutine::shared_data(&m_uctxReturn, std::move(fnMain))
      );
      // Add the coroutine to those ready to start.
      m_listStartingCoros.push_back(std::move(pcorod));
   }

   static coroutine_scheduler & attach_to_current_thread(
      std::shared_ptr<coroutine_scheduler> pcorosched = nullptr
   );

   static std::shared_ptr<coroutine_scheduler> const & get_for_current_thread() {
      return sm_pcorosched;
   }

   void run();

   void yield_while_async_pending(io::filedesc const & fd, bool bWrite);

private:
   std::unique_ptr<coroutine::shared_data> find_coroutine_to_activate();

private:
   // Pointer to the active (current) coroutine, if not the main one (the thread’s original code).
   std::unique_ptr<coroutine::shared_data> m_pcorodActive;
   //! List of coroutines that have been scheduled, but have not been started yet.
   collections::list<std::unique_ptr<coroutine::shared_data>> m_listStartingCoros;
   //! Pointer to the coroutine_scheduler for the current thread.
   static thread_local_value<std::shared_ptr<coroutine_scheduler>> sm_pcorosched;

   // TODO: move the following members to an “impl” subclass.

   //! File descriptor of the internal epoll.
   io::filedesc m_fdEpoll;
   //! Coroutines that are blocked on a fd wait.
   collections::map<int, std::unique_ptr<coroutine::shared_data>> m_mapBlockedCoros;
   // Coroutine context that every coroutine eventually returns to.
   ::ucontext_t m_uctxReturn;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COROUTINE_HXX
