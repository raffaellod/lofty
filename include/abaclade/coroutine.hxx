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

#if ABC_HOST_API_LINUX
   #include <ucontext.h>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::coroutine

namespace abc {

/*! Subroutine for use in non-preemptive multitasking, enabling asynchronous I/O in most abc::io
classes. */
class ABACLADE_SYM coroutine : public noncopyable {
private:
   friend class coroutine_scheduler;

   //! OS-dependent execution context for the coroutine.
   class context;

   class ABACLADE_SYM shared_data : public noncopyable {
   public:
      shared_data(std::function<void ()> fnMain) :
         m_fnInnerMain(std::move(fnMain)) {
      }

      //! See shared_data::inner_main().
      void inner_main() {
         m_fnInnerMain();
      }

   private:
      // TODO: copy abc::thread callback pattern.
      std::function<void ()> m_fnInnerMain;
   };

public:
   coroutine() {
   }
   template <typename F>
   explicit coroutine(F fnMain) :
      m_pctx(create_context(std::unique_ptr<shared_data>(new shared_data(std::move(fnMain))))) {
   }
   coroutine(coroutine && coro) :
      m_pctx(std::move(coro.m_pctx)) {
   }

   //! Destructor.
   ~coroutine();

private:
   static std::shared_ptr<context> create_context(std::unique_ptr<shared_data> psd);

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
   ~coroutine_scheduler();

   void add_coroutine(coroutine const & coro) {
      // Add the coroutine to those ready to start.
      m_listStartingCoros.push_back(coro.m_pctx);
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
   std::shared_ptr<coroutine::context> find_coroutine_to_activate();

private:
   // Pointer to the active (current) coroutine, if not the main one (the thread’s original code).
   std::shared_ptr<coroutine::context> m_pcoroctxActive;
   //! List of coroutines that have been scheduled, but have not been started yet.
   collections::list<std::shared_ptr<coroutine::context>> m_listStartingCoros;
   //! Pointer to the coroutine_scheduler for the current thread.
   static thread_local_value<std::shared_ptr<coroutine_scheduler>> sm_pcorosched;

   // TODO: move the following members to an “impl” subclass.

   //! File descriptor of the internal epoll.
   io::filedesc m_fdEpoll;
   //! Coroutines that are blocked on a fd wait.
   collections::map<int, std::shared_ptr<coroutine::context>> m_mapBlockedCoros;
   // Coroutine context that every coroutine eventually returns to.
   ::ucontext_t m_uctxReturn;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COROUTINE_HXX
